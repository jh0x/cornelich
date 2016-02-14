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

#include <cornelich/util/stop_bit.h>

#include <array>
#include <cstdint>

#include <catch.hpp>

using namespace cornelich::util;


void test_stop_bit_encoding(std::int64_t value, const std::vector<std::uint8_t> & expected)
{
    std::array<std::uint8_t, 1024> buffer;
    std::int32_t write_offset = 0;

    stop_bit::write(buffer.data(), write_offset, value);
    INFO("value=" << value << ", write_offset=" << write_offset);
    REQUIRE(write_offset == expected.size());
    REQUIRE(std::equal(expected.begin(), expected.end(), buffer.begin()));

    std::int32_t read_offset = 0;
    auto read_back = stop_bit::read(buffer.data(), read_offset);
    INFO("value=" << value << ", read_back=" << read_back << ", read_offset=" << read_offset);
    REQUIRE(write_offset == read_offset);
    REQUIRE(read_back == value);
}

TEST_CASE("stop-bit encoding", "[util/stop_bit]")
{
    SECTION("Stopbit encoder")
    {
        test_stop_bit_encoding(0b00000000, {0b00000000});
        test_stop_bit_encoding(0b01111111, {0b01111111});
        test_stop_bit_encoding(0b10000000, {0b10000000, 0b00000001});
        test_stop_bit_encoding(0b00000000, {0b00000000});
        test_stop_bit_encoding(0b00000000, {0b00000000});
        test_stop_bit_encoding(0b00000000, {0b00000000});
        test_stop_bit_encoding(0b0010111101011100111000010001111111100000110001101110000101111110,
                              {0b11111110, 0b11000010, 0b10011011, 0b10000110, 0b11111110, 0b10100011, 0b10111000, 0b10101110, 0b00101111});
        test_stop_bit_encoding(0x7FFFFFFFFFFFFFFF, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F});
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
        test_stop_bit_encoding(0x8000000000000000, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00});
        test_stop_bit_encoding(0xFFFFFFFFFFFFFF00, {0xFF, 0x81, 0x00});
        test_stop_bit_encoding(0xFFFFFFFFFFFFFFFF, {0x80, 0x00});
#pragma GCC diagnostic pop
    }
}
