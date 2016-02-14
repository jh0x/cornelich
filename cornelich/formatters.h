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

#include "util/math_util.h"
#include "util/stop_bit.h"

#include <boost/utility/string_ref.hpp>

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

namespace cornelich
{
namespace writers
{

template<typename T>
struct raw
{
    void operator()(std::uint8_t * buffer, std::int32_t & offset, const T & val) const
    {
        std::memcpy(buffer + offset, &val, sizeof(T));
        offset += static_cast<std::int32_t>(sizeof(T));
    }
};


// OpenHFT boolean
struct open_hft_boolean
{
    void operator()(std::uint8_t * buffer, std::int32_t & offset, bool val) const
    {
        buffer[offset++] = val ? 'Y' : '\0';
    }
};


struct chars
{
    void operator()(std::uint8_t * buffer, std::int32_t & offset, boost::string_ref str) const
    {
        util::stop_bit::write(buffer, offset, static_cast<std::int64_t>(str.length()));
        for(auto c : str)
            buffer[offset++] = static_cast<std::uint8_t>(c);
    }
};

}

namespace readers
{

template<typename T>
struct raw
{
    const T & operator()(const std::uint8_t * buffer, std::int32_t & offset) const
    {
        auto && val = *reinterpret_cast<const T *>(buffer + offset);
        offset += static_cast<std::int32_t>(sizeof(T));
        return val;
    }
};

// OpenHFT boolean
struct open_hft_boolean
{
    bool operator()(const std::uint8_t * buffer, std::int32_t & offset) const
    {
        return buffer[offset++];
    }
};



struct chars
{
    boost::string_ref operator()(const std::uint8_t * buffer, std::int32_t & offset) const
    {
        auto len = util::stop_bit::read(buffer, offset);
        auto result = boost::string_ref(reinterpret_cast<const char *>(buffer + offset), static_cast<std::size_t>(len));
        offset += static_cast<std::int32_t>(len);
        return result;
    }
};

}
}
