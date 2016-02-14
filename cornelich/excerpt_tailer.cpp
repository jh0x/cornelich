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

#include "excerpt_tailer.h"

#include "vanilla_chronicle.h"
#include "vanilla_utils.h"

#include "util/math_util.h"
#include "util/streamer.h"

namespace cornelich
{

excerpt_tailer::excerpt_tailer(vanilla_chronicle & chronicle)
    : m_chronicle(chronicle)
    , m_index(-1)
    , m_last_cycle(-1)
    , m_last_index_file_number(-1)
    , m_last_thread_id(-1)
    , m_last_data_file_number(-1)
    , m_buffer(m_index)
{
}

excerpt_tailer & excerpt_tailer::to_start()
{
    auto cycle = m_chronicle.m_index.find_first_cycle();
    if(cycle >= 0)
        m_index = (cycle * m_chronicle.m_settings.entries_per_cycle()) - 1;
    return *this;

}

excerpt_tailer & excerpt_tailer::to_end()
{
    auto last_index = m_chronicle.last_index();
    if(last_index >= 0)
        index(last_index);
    else
        return to_start();
    return *this;
}

bool excerpt_tailer::next_index()
{
    if(m_index < 0)
    {
        to_start();
        if(m_index < 0)
            return false;
    }

    auto next = m_index + 1;
    while(true)
    {
        auto found = index(next);
        if(found)
        {
            m_index = next;
            return true;
        }

        auto cycle = static_cast<std::int32_t>(next / m_chronicle.m_settings.entries_per_cycle());
        if(cycle > cycle_for_now(m_chronicle.m_settings.cycle_length()))
            return false;
        next = (cycle + 1) * m_chronicle.m_settings.entries_per_cycle();
    }
}

bool excerpt_tailer::index(std::int64_t index)
{
    auto cycle_for_index = static_cast<int32_t>(util::right_shift(index, m_chronicle.m_entries_for_cycle_bits));
    auto index_file_number = static_cast<int32_t>(util::right_shift(index & m_chronicle.m_entries_for_cycle_mask, m_chronicle.m_index_block_longs_bits));
    auto index_offset = index & m_chronicle.m_index_block_longs_mask;

    bool index_file_change = false;

    if(m_last_cycle != cycle_for_index || m_last_index_file_number != index_file_number || !m_index_region)
    {
        m_index_region = m_chronicle.m_index.index_for(cycle_for_index, index_file_number, false);
        if(!m_index_region)
            return false;
        index_file_change = true;
        m_last_cycle = cycle_for_index;
        m_last_index_file_number = index_file_number;
    }

    auto index_value = m_index_region->read_ordered64(static_cast<std::int32_t>(index_offset << 3));
    if(!index_value)
        return false;

    auto thread_id = static_cast<std::int32_t>(util::right_shift(index_value, m_chronicle.m_settings.index_data_offset_bits()));
    auto data_offset0 = index_value & m_chronicle.m_settings.index_data_offset_mask();
    auto data_file_number = static_cast<std::int32_t>(util::right_shift(data_offset0, m_chronicle.m_data_block_size_bits));
    auto data_offset = static_cast<std::int32_t>(data_offset0 & m_chronicle.m_data_block_size_mask);

    if(m_last_thread_id != thread_id || m_last_data_file_number != data_file_number || index_file_change)
        m_data_region.reset();

    if (!m_data_region)
    {
        m_data_region = m_chronicle.m_data.data_for(cycle_for_index, thread_id, data_file_number, false);
        m_last_thread_id = thread_id;
        m_last_data_file_number = data_file_number;
    }
    if (!m_data_region)
        return false;

    auto len = m_data_region->read_ordered32(data_offset - 4);
    if(len == 0)
        return false;

    auto len2 = ~len;
    // invalid if either the top two bits are set,
    if(util::right_shift(len2, 30))
        throw std::logic_error(util::streamer() << "Corrupted length 0x" << std::hex << len);

    m_buffer.reset(m_data_region->data() + data_offset, 0, len2);
    __builtin_prefetch(m_data_region->data() + 64, 0);
    m_index = index;

    return true;
}

bool excerpt_tailer::position(std::int32_t position)
{
    if(position > m_buffer.limit())
        return false;
    m_buffer.position() = position;
    return true;
}

}
