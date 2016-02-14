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

#include <cornelich/util/test_helpers.h>

#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>
#include <cornelich/formatters.h>

#include <cornelich/util/stop_bit.h>

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include <catch.hpp>

using namespace cornelich;


void test_chars_encoding(const char * value, const std::vector<std::uint8_t> & expected)
{
    std::array<std::uint8_t, 1024> buffer;
    std::int32_t write_offset = 0;

    writers::chars()(buffer.data(), write_offset, value);
    INFO("value=" << value << ", write_offset=" << write_offset);
    REQUIRE(write_offset == expected.size());
    REQUIRE(std::equal(expected.begin(), expected.end(), buffer.begin()));

    std::int32_t read_offset = 0;
    auto read_back = readers::chars()(buffer.data(), read_offset);
    INFO("value=" << value << ", read_back=" << read_back << ", read_offset=" << read_offset);
    REQUIRE(write_offset == read_offset);
    REQUIRE(read_back == value);
}


TEST_CASE("Custom readers/writers in isolation", "[formatters]")
{
    SECTION("Chars encoder")
    {
        test_chars_encoding("", {0x00});
        test_chars_encoding("A", {0x01, 'A'});
        test_chars_encoding("Chars encoder", {0x0D, 'C','h','a','r','s',' ','e','n','c','o','d','e','r'});
    }
    SECTION("OpenHFT bool writer")
    {
        std::array<std::uint8_t, 1024> buffer;
        std::int32_t write_offset = 0;

        writers::open_hft_boolean()(buffer.data(), write_offset, true);
        REQUIRE(buffer[0] == 'Y');
        REQUIRE(write_offset == 1);
        writers::open_hft_boolean()(buffer.data(), write_offset, false);
        REQUIRE(buffer[1] == 0);
        REQUIRE(write_offset == 2);
        writers::open_hft_boolean()(buffer.data(), write_offset, true);
        REQUIRE(buffer[2] == 'Y');
        REQUIRE(write_offset == 3);
        writers::open_hft_boolean()(buffer.data(), write_offset, true);
        REQUIRE(buffer[3] == 'Y');
        REQUIRE(write_offset == 4);
        writers::open_hft_boolean()(buffer.data(), write_offset, false);
        REQUIRE(buffer[4] == 0);
        REQUIRE(write_offset == 5);
    }
    SECTION("OpenHFT bool reader")
    {
        std::array<std::uint8_t, 1024> buffer;
        std::int32_t read_offset = 0;

        buffer[0] = 0;
        buffer[1] = 'Y';
        buffer[2] = '2';
        buffer[3] = 0;

        REQUIRE(readers::open_hft_boolean()(buffer.data(), read_offset) == false);
        REQUIRE(read_offset == 1);
        REQUIRE(readers::open_hft_boolean()(buffer.data(), read_offset) == true);
        REQUIRE(read_offset == 2);
        REQUIRE(readers::open_hft_boolean()(buffer.data(), read_offset) == true);
        REQUIRE(read_offset == 3);
        REQUIRE(readers::open_hft_boolean()(buffer.data(), read_offset) == false);
        REQUIRE(read_offset == 4);
    }
    SECTION("Raw encoder")
    {
        struct foo
        {
            int a;
            int b;
            unsigned long long c;
            float d;
        };
        foo foo1 = {0x1234567, 0x10203040, 0xaabbccddeeff0011LL, 0.125};
        std::array<std::uint8_t, 1024> buffer;
        std::int32_t offset = 0;
        writers::raw<foo>()(buffer.data(), offset, foo1);
        REQUIRE(offset == sizeof(foo));

        offset = 0;
        auto foo2 = readers::raw<foo>()(buffer.data(), offset);
        REQUIRE(offset == sizeof(foo));
        REQUIRE(foo1.a == foo2.a);
        REQUIRE(foo1.b == foo2.b);
        REQUIRE(foo1.c == foo2.c);
        REQUIRE(foo1.d == Approx(foo2.d));
    }
}


TEST_CASE_METHOD(clean_up_fixture, "Custom readers/writers with a chronicle", "[formatters]")
{
    GIVEN("A non-existent chronicle")
    {
        vanilla_chronicle_settings settings(path().c_str());
        vanilla_chronicle chronicle(settings);

        SECTION("Can provide a custom inline formatter")
        {
            constexpr auto FIXED_STRING_SIZE = 10;
            auto appender = chronicle.create_appender();
            appender.start_excerpt(128);
            appender.write("QWERTYUIOP", [](std::uint8_t * buffer, std::int32_t & position, const char * val)
            {
                std::copy(val, val + FIXED_STRING_SIZE, buffer + position);
                position += 10;
            });
            appender.finish();

            auto tailer = chronicle.create_tailer();
            REQUIRE(tailer.next_index());
            auto && val = tailer.read([](const std::uint8_t * buffer, std::int32_t & position)
            {
                auto result = reinterpret_cast<const char *>(buffer) + position;
                position += FIXED_STRING_SIZE;
                return result;
            });
            REQUIRE(std::string(val) == "QWERTYUIOP");
        }

        SECTION("Handle stopbit encoding")
        {
            auto appender = chronicle.create_appender();
            appender.start_excerpt(8192);
            appender.write(0, util::stop_bit::write);
            appender.write(1, util::stop_bit::write);
            appender.write(-1, util::stop_bit::write);
            appender.write(std::numeric_limits<std::int8_t>::min(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int8_t>::max(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int16_t>::min(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int16_t>::max(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int32_t>::min(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int32_t>::max(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int64_t>::min(), util::stop_bit::write);
            appender.write(std::numeric_limits<std::int64_t>::max(), util::stop_bit::write);
            appender.finish();

            auto tailer = chronicle.create_tailer();
            REQUIRE(tailer.next_index());
            REQUIRE(tailer.read(util::stop_bit::read) == 0);
            REQUIRE(tailer.read(util::stop_bit::read) == 1);
            REQUIRE(tailer.read(util::stop_bit::read) == -1);
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int8_t>::min());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int8_t>::max());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int16_t>::min());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int16_t>::max());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int32_t>::min());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int32_t>::max());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int64_t>::min());
            REQUIRE(tailer.read(util::stop_bit::read) == std::numeric_limits<std::int64_t>::max());
        }

        SECTION("Handle chars encoding")
        {
            std::string long_string(1000,'c');

            auto appender = chronicle.create_appender();
            appender.start_excerpt(8192);
            appender.write("", writers::chars());
            appender.write("Some text", writers::chars());
            appender.write(long_string, writers::chars());
            appender.finish();

            auto tailer = chronicle.create_tailer();
            REQUIRE(tailer.next_index());
            REQUIRE(tailer.read(readers::chars()) == "");
            REQUIRE(tailer.read(readers::chars()) == "Some text");
            REQUIRE(tailer.read(readers::chars()) == long_string);
        }

        SECTION("Handle raw primitives")
        {
            double d = 2.3123142155;
            float f = 1.2312321312f;
            char c = 'a';
            short s  = 32000;
            int i = 100000000;
            long l  = 1000000000000000L;

            auto appender = chronicle.create_appender();
            appender.start_excerpt(8192);
            appender.write(d);
            appender.write(f);
            appender.write(c);
            appender.write(s);
            appender.write(i);
            appender.write(l);
            appender.finish();

            auto tailer = chronicle.create_tailer();
            REQUIRE(tailer.next_index());
            REQUIRE(tailer.read<double>() == Approx(d).epsilon(1e-9));
            REQUIRE(tailer.read<float>() == Approx(f).epsilon(1e-9));
            REQUIRE(tailer.read<char>() == c);
            REQUIRE(tailer.read<short>() == s);
            REQUIRE(tailer.read<int>() == i);
            REQUIRE(tailer.read<long>() == l);
        }
    }
}
