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
#include <type_traits>

namespace cornelich
{
namespace util
{

/**
 * Perform unsigned right shift (i.e. always fill with zeros from the left)
 */
template<typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type right_shift(T x, int how_much)
{
    // Whatever we are shifting - should be made unsigned so that it gets filled with zros from the left
    using unsigned_type = typename std::make_unsigned<T>::type;
    // But then cast the result back to T
    return static_cast<T>(
                (static_cast<unsigned_type>(x) >> how_much)
                );
}

__attribute__ ((const))
inline int log2_bits(std::uint32_t v)
{
    return 32 - __builtin_clz(v) - 1;
}


}
}
