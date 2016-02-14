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

#include "vanilla_chronicle.h"

#include "util/math_util.h"

#include <cmath>

namespace cornelich
{

vanilla_chronicle::vanilla_chronicle(const vanilla_chronicle_settings & settings)
    : m_settings(settings)
    , m_index_block_size_bits( (std::int32_t)std::log2(m_settings.index_block_size()) )
    , m_index_block_size_mask( ((std::int64_t)1 << m_index_block_size_bits) - 1 )
    , m_index_block_longs_bits( m_index_block_size_bits - 3 )
    , m_index_block_longs_mask( util::right_shift(m_index_block_size_mask, 3) )
    , m_data_block_size_bits( (std::int32_t)std::log2(m_settings.data_block_size()) )
    , m_data_block_size_mask( ((std::int64_t)1 << m_data_block_size_bits) - 1 )
    , m_entries_for_cycle_bits( (std::int32_t)std::log2(m_settings.entries_per_cycle()) )
    , m_entries_for_cycle_mask( ((std::int64_t)1 << m_entries_for_cycle_bits) - 1 )
    , m_index(m_settings, m_index_block_size_bits)
    , m_data(m_settings, m_data_block_size_bits)
    , m_last_written_index(-1)
{
}

std::int64_t vanilla_chronicle::last_index()
{
    const auto last_cycle = m_index.find_last_cycle();
    if(last_cycle == -1)
        return -1;

    const auto last_file = m_index.last_index_file_number(last_cycle, -1);
    if(last_file == -1)
        return -1;

    const auto region = m_index.index_for(last_cycle, last_file, false);
    if(!region)
        return -1;
    const auto count_entries = vanilla_index::count_index_entries(*region);
    const auto index_entry_number = (count_entries > 0) ? count_entries - 1 : 0;

    return (static_cast<std::int64_t>(last_cycle) <<  m_entries_for_cycle_bits) +
           (static_cast<std::int64_t>(last_file) << m_index_block_size_bits) +
           index_entry_number;
}

excerpt_appender vanilla_chronicle::create_appender()
{
    return excerpt_appender(*this);
}

excerpt_tailer vanilla_chronicle::create_tailer()
{
    return excerpt_tailer(*this);
}

}
