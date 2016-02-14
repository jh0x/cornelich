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

#pragma once

#include <cstdint>
#include <iosfwd>

namespace cornelich
{
namespace util
{

class buffer_view
{
public:
    buffer_view(const std::int64_t & index) : m_index(index) {}

    void reset()
    {
        m_data = nullptr;
        m_position = 0;
        m_limit = 0;
    }

    void reset(std::uint8_t * data, std::int32_t position, std::int32_t limit)
    {
        m_data = data;
        m_position = position;
        m_limit = limit;
    }

    std::uint8_t * data()
    {
        return m_data;
    }

    const std::uint8_t * data() const
    {
        return m_data;
    }


    std::int32_t limit() const
    {
        return m_limit;
    }

    std::int32_t & position()
    {
        return m_position;
    }

    std::int32_t position() const
    {
        return m_position;
    }

    std::int32_t remaining() const
    {
        return m_limit - m_position;
    }

    std::int64_t index() const {
        return m_index;
    }

private:
    const int64_t & m_index;
    std::int32_t m_position;
    std::int32_t m_limit;
    std::uint8_t * m_data;
};

std::ostream & operator<<(std::ostream & os, const buffer_view & b);

}
}
