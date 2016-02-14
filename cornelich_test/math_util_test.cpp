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

#include <cornelich/util/math_util.h>

#include <cstdint>
#include <catch.hpp>

using namespace cornelich::util;

TEST_CASE( "util::right_shift: unsigned shift right", "[util/math]" )
{
    REQUIRE(right_shift((std::int8_t)0b11111111, 0) == (std::int8_t)0b11111111);
    REQUIRE(right_shift((std::int8_t)0b11111111, 3) == (std::int8_t)0b00011111);
    REQUIRE(right_shift((std::int8_t)0b11111111, 7) == (std::int8_t)0b00000001);
    REQUIRE(right_shift((std::int8_t)0b11111111, 8) == (std::int8_t)0b00000000);
    REQUIRE(right_shift((std::int8_t)0b11111111, 9) == (std::int8_t)0b00000000);

    REQUIRE(right_shift((std::uint8_t)0b11111111, 0) == (std::uint8_t)0b11111111);
    REQUIRE(right_shift((std::uint8_t)0b11111111, 3) == (std::uint8_t)0b00011111);
    REQUIRE(right_shift((std::uint8_t)0b11111111, 7) == (std::uint8_t)0b00000001);
    REQUIRE(right_shift((std::uint8_t)0b11111111, 8) == (std::uint8_t)0b00000000);
    REQUIRE(right_shift((std::uint8_t)0b11111111, 9) == (std::uint8_t)0b00000000);

    REQUIRE(right_shift((std::int64_t)-1,  0) == (std::int64_t)-1);
    REQUIRE(right_shift((std::int64_t)-1, 32) == (std::int64_t)0x00000000FFFFFFFF);

    REQUIRE(right_shift((std::uint64_t)-1,  0) == (std::uint64_t)-1);
    REQUIRE(right_shift((std::uint64_t)-1, 48) == (std::uint64_t)0x000000000000FFFF);
}

TEST_CASE( "util::log2_bits:", "[util/math]" )
{
    REQUIRE(log2_bits(1) == 0);
    REQUIRE(log2_bits(2) == 1);
    REQUIRE(log2_bits(3) == 1);
    REQUIRE(log2_bits(4) == 2);
    REQUIRE(log2_bits(1023) == 9);
    REQUIRE(log2_bits(1024) == 10);
    REQUIRE(log2_bits((1<<20) - 1) == 19);
    REQUIRE(log2_bits(1<<20) == 20);
    REQUIRE(log2_bits((1<<20) + 1) == 20);
}
