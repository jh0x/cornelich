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

#include "stop_bit.h"

namespace cornelich
{
namespace util
{

namespace stop_bit
{
std::int64_t read(const std::uint8_t * buffer, std::int32_t & offset)
{
    std::int64_t l;
    if ((l = (std::int8_t)buffer[offset++]) >= 0)
        return l;
    l &= 0x7FL;
    std::int64_t b;
    int count = 7;
    while ((b = (std::int8_t)buffer[offset++]) < 0)
    {
        l |= (b & 0x7FL) << count;
        count += 7;
    }
    if (b != 0)
    {
        if (count > 56)
            throw std::logic_error("Cannot read more than 9 stop bits of positive value");
        return l | (b << count);
    }
    else
    {
        if (count > 63)
            throw std::logic_error("Cannot read more than 10 stop bits of negative value");
        return ~l;
    }
}

void write(std::uint8_t * buffer, std::int32_t & offset, std::int64_t value)
{
    if((value & ~0x7F) == 0)
    {
        buffer[offset++] = value & 0x7f;
        return;
    }
    if((value & ~0x3FFF) == 0)
    {
        buffer[offset++] = static_cast<std::uint8_t>(value & 0x7f) | 0x80;
        buffer[offset++] = static_cast<std::uint8_t>(value >> 7);
        return;
    }
    auto neg = false;
    if(value < 0)
    {
        neg = true;
        value = ~value;
    }
    std::uint64_t n2;
    while ((n2 = static_cast<std::uint64_t>(value >> 7)) != 0)
    {
        buffer[offset++] = static_cast<std::uint8_t>(0x80L | value);
        value = static_cast<std::int64_t>(n2);
    }
    // final byte
    if (!neg)
        buffer[offset++] = static_cast<std::uint8_t>(value);
    else
    {
        buffer[offset++] = static_cast<std::uint8_t>(0x80 | value);
        buffer[offset++] = 0;
    }
}

}

}
}
