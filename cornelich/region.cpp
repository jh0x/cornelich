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

#include "region.h"

#include <util/files.h>

#include <cassert>
#include <stdexcept>

namespace bip = boost::interprocess;

namespace cornelich
{

region::region(const std::string & path, std::uint32_t size, std::int32_t index)
    : m_path(path)
    , m_index(index)
    , m_size(size)
    , m_region(util::create_mapping(path, size), bip::read_write, 0, static_cast<std::size_t>(size), 0)
    , m_start_offset(0)
    , m_limit_offset(size)
    //, m_capacity_offset(size)
    , m_position_offset(0)
{
    m_region.advise(bip::mapped_region::advice_willneed);
}

void region::align_position(int32_t value)
{
    assert( !(value == 0) && !(value & (value - 1)) );
    auto pos = (m_position_offset + value - 1) & ~(value - 1);
    position(pos);
}

bool region::position(std::int32_t position)
{
    if(BOOST_UNLIKELY(position > m_limit_offset || position < 0))
        return false;
    m_position_offset.store(m_start_offset + position, std::memory_order_relaxed);
    return true;
}

}
