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

#include "vanilla_chronicle_settings.h"

#include <vanilla_date.h>

#include "util/streamer.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace cornelich
{

vanilla_chronicle_settings::vanilla_chronicle_settings(const std::string & path)
    : m_path(path)
    , m_thread_id_bits(DEFAULT_THREAD_ID_BITS)
    , m_cycle_length(24 * 60 * 60 * 1000) // MILLIS_PER_DAY
    , m_cycle_format(std::make_shared<cycle_formatter_yyyymmdd>(m_cycle_length))
    , m_entries_per_cycle(1ULL << 40)
    , m_index_block_size(16ULL << 20) // 16MB
    , m_data_block_size(64ULL << 20) // 64MB
    , m_index_cache_size(8)
    , m_data_cache_size(16)
{
}

vanilla_chronicle_settings & vanilla_chronicle_settings::cycle_length(const std::int32_t cycle_length, const std::shared_ptr<cycle_formatter> & formatter, bool check)
{
    if(check && cycle_length < min_cycle_length())
        throw std::invalid_argument(util::streamer() << "Cycle length [ms] can't be less than " << min_cycle_length() << " ms");
    m_cycle_length = cycle_length;
    m_cycle_format = formatter;
    return *this;
}

std::ostream & operator<<(std::ostream & os, const vanilla_chronicle_settings & s)
{
    os << "- path                   = " << s.path() << '\n'
       << "- cycle_length           = " << s.cycle_length() << '\n'
       << "- entries_per_cycle      = " << s.entries_per_cycle() << " [" << std::log2(s.entries_per_cycle()) << " bits]\n"
       << "- index_block_size       = " << s.index_block_size() << " [" << std::log2(s.index_block_size()) << " bits]\n"
       << "- data_block_size        = " << s.data_block_size() << " [" << std::log2(s.data_block_size()) << " bits]\n"
       << "- thread_id_bits         = " << s.thread_id_bits() << '\n'
       << "- thread_id_mask         = 0x" << std::hex << s.thread_id_mask() << std::dec << '\n'
       << "- index_data_offset_bits = " << s.index_data_offset_bits() << '\n'
       << "- index_data_offset_mask = 0x" << std::hex << s.index_data_offset_mask() << std::dec
       << "- index_cache_size       = " << s.index_cache_size() << '\n'
       << "- data_cache_size        = " << s.data_cache_size();
    return os;
}

}
