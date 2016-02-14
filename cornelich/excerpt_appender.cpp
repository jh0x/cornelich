/*
Copyright 2015-2016 Joanna Hulboj <j@hulboj.org>
Copyright 2016 Milosz Hulboj <m@hulboj.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "excerpt_appender.h"

#include "vanilla_chronicle.h"
#include "vanilla_utils.h"

#include "util/math_util.h"
#include "util/streamer.h"
#include "util/thread.h"

#include <utility>

namespace cornelich
{

excerpt_appender::excerpt_appender(vanilla_chronicle & chronicle)
    : m_chronicle(chronicle)
    , m_index(-1)
    , m_last_cycle(-1)
    , m_last_index_file_number(-1)
    , m_last_thread_id(-1)
    , m_last_data_file_number(-1)
    , m_finished(true)
    , m_buffer(m_index)
{
}

void excerpt_appender::start_excerpt(std::int32_t capacity)
{
    start_excerpt(capacity, cycle_for_now(m_chronicle.m_settings.cycle_length()));
}

void excerpt_appender::start_excerpt(std::int32_t capacity, std::int32_t cycle)
{
    auto thread_id = util::get_native_thread_id();
    assert((thread_id & m_chronicle.m_settings.thread_id_mask()) == thread_id);

    if (cycle != m_last_cycle)
    {
        m_index_region.reset();

        m_data_region.reset();

        m_last_cycle = cycle;
        m_last_index_file_number = m_chronicle.m_index.last_index_file_number(m_last_cycle, 0);
        m_last_thread_id = thread_id;
    }
    else if (thread_id != m_last_thread_id)
    {
        m_data_region.reset();
        m_last_thread_id = thread_id;
    }

    if (!m_data_region)
    {
        m_last_data_file_number = m_chronicle.m_data.find_next_data_file_number(cycle, thread_id);
        m_data_region = m_chronicle.m_data.data_for(cycle,thread_id, m_last_data_file_number, true);
    }

    if (m_data_region->remaining() < static_cast<std::int32_t>(capacity) + 4) // +4 to store the size later on (see finish())
    {
        m_data_region = m_chronicle.m_data.data_for(cycle, thread_id, ++m_last_data_file_number, true);
    }

    m_buffer.reset(m_data_region->data() + m_data_region->position() + 4, 0, static_cast<std::int32_t>(capacity));
    __builtin_prefetch(m_data_region->data() + 64, 1);
    m_finished = false;
}

void excerpt_appender::finish()
{
    if(m_finished)
        throw std::logic_error("Not started");

    if(!m_data_region)
        return;

    const auto length = ~static_cast<std::int32_t>(m_buffer.position());

    m_data_region->write_ordered32(static_cast<std::int32_t>(m_buffer.data() - m_data_region->data() - 4), length);

    const auto data_offset = m_data_region->index() * m_chronicle.m_settings.data_block_size() + (m_buffer.data() - m_data_region->data());
    const auto index_value = (static_cast<std::int64_t>(m_last_thread_id) << m_chronicle.m_settings.index_data_offset_bits()) + data_offset;

    auto position = m_index_region ? vanilla_index::append(*m_index_region, index_value) : -1;
    if (position < 0)
    {
        if (m_index_region)
        {
            m_last_index_file_number = static_cast<std::int32_t>(m_index_region->index()) + 1;
            m_index_region.reset();
        }

        std::tie(m_index_region, position) = m_chronicle.m_index.append(m_last_cycle, index_value, m_last_index_file_number);
        m_last_index_file_number = m_index_region->index();
    }

    set_last_written_index(m_last_cycle, m_index_region->index(), position);

    m_index = m_last_written_index + 1;
    m_data_region->position(m_data_region->position() + m_buffer.position() + 4);
    m_data_region->align_position(4);
    m_finished = true;
}

std::int64_t excerpt_appender::index_from(std::int64_t cycle, std::int64_t index_count, std::int64_t index_position) const
{
    return (cycle << m_chronicle.m_entries_for_cycle_bits) + (index_count << m_chronicle.m_index_block_longs_bits) + (index_position >> 3);
}

void excerpt_appender::set_last_written_index(std::int64_t cycle, std::int64_t index_count, std::int64_t index_position)
{
    std::int64_t last_written_index = index_from(cycle, index_count, index_position);
    m_last_written_index = last_written_index;
    while(true)
    {
        std::int64_t lwi = m_chronicle.last_written_index();
        if(lwi >= last_written_index || m_chronicle.m_last_written_index.compare_exchange_weak(lwi, last_written_index))
            break;
    }
}

}
