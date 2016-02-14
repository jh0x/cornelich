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

#include "write_test_data.h"

#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>
#include <cornelich/vanilla_date.h>

#include <array>
#include <cstdint>
#include <map>
#include <limits>
#include <chrono>
#include <thread>
#include <vector>

#include <catch.hpp>



TEST_CASE_METHOD(clean_up_fixture, "Using excerpt_appender to write data into the chronicle", "[vanilla_chronicle]")
{
    GIVEN("A non-existent chronicle")
    {
        vanilla_chronicle_settings settings(path().c_str());
        // Small values to exercise region rotation
        settings.data_block_size(1ULL << 20);
        // Small values to exercise region rotation
        settings.index_block_size(1ULL << 13);
        vanilla_chronicle chronicle(settings);
        REQUIRE(chronicle.last_index() == -1);

        SECTION("Tailer begin == end")
        {
            auto tailer = chronicle.create_tailer();
            auto end = tailer.to_end().index();
            auto start = tailer.to_start().index();
            REQUIRE(end == start);
        }

        GIVEN("An appender")
        {
            auto appender = chronicle.create_appender();
            REQUIRE(appender.index() == -1);

            SECTION("Can write data into an excerpt")
            {
                for(int i = 0; i != 2000000; ++i)
                {
                    appender.start_excerpt(1024);
                    appender.write(1LL);
                    appender.write(i);
                    appender.write(0x0123456789ABCDEFLL);
                    appender.write(1);
                    appender.write(2);
                    appender.write(3);
                    appender.write(4);
                    appender.write(5);
                    if(i % 5 == 0)
                        appender.write(0LL);
                    appender.finish();
                }

                int count = 0;
                auto tailer = chronicle.create_tailer();
                while(tailer.next_index())
                {
                    REQUIRE(tailer.read<std::int64_t>()== 1LL);
                    REQUIRE(tailer.read<std::int32_t>() == count);
                    REQUIRE(tailer.read<std::int64_t>() == 0x0123456789ABCDEFLL);
                    REQUIRE(tailer.read<std::int32_t>() == 1);
                    REQUIRE(tailer.read<std::int32_t>() == 2);
                    REQUIRE(tailer.read<std::int32_t>() == 3);
                    REQUIRE(tailer.read<std::int32_t>() == 4);
                    REQUIRE(tailer.read<std::int32_t>() == 5);
                    if(count % 5 == 0)
                        REQUIRE(tailer.read<std::int64_t>() == 0LL);
                    ++count;
                }
                {
                    // Random access
                    tailer.to_start();
                    REQUIRE(tailer.index(tailer.index() + 1000000) == true);
                    REQUIRE(tailer.read<std::int64_t>()== 1LL);
                    REQUIRE(tailer.read<std::int32_t>() == 999999);
                    REQUIRE(tailer.read<std::int64_t>() == 0x0123456789ABCDEFLL);
                    REQUIRE(tailer.read<std::int32_t>() == 1);
                    REQUIRE(tailer.read<std::int32_t>() == 2);
                    REQUIRE(tailer.read<std::int32_t>() == 3);
                    REQUIRE(tailer.read<std::int32_t>() == 4);
                    REQUIRE(tailer.read<std::int32_t>() == 5);
                }
            }
        }
    }
}


TEST_CASE_METHOD(clean_up_fixture, "Excercise cycle rolling", "[vanilla_chronicle]")
{
    GIVEN("A non-existent chronicle")
    {
        vanilla_chronicle_settings settings(path().c_str());
        // Small values to exercise region rotation
        settings.data_block_size(1ULL << 20);
        // Small values to exercise region rotation
        settings.index_block_size(1ULL << 13);
        // One minute cycle length
        settings.cycle_length(1000 * 60, std::make_shared<cycle_formatter_yyyymmddhhmm>(1000 * 60), false);
        // TODO:
        settings.entries_per_cycle(1 << 29);
        vanilla_chronicle chronicle(settings);
        REQUIRE(chronicle.last_index() == -1);

        auto appender = chronicle.create_appender();
        REQUIRE(appender.index() == -1);

        SECTION("Can write with multiple writers")
        {
            constexpr auto THREAD_COUNT = 1u;
            constexpr auto ITER_COUNT = 10000u;
            std::vector<std::thread> threads;
            for(auto tid = 0u; tid != THREAD_COUNT; ++tid)
            {
                threads.push_back(std::thread([&chronicle,tid]()
                {
                    auto a = chronicle.create_appender();
                    write_test_data(a, tid, ITER_COUNT, [](){std::this_thread::sleep_for(std::chrono::milliseconds(7));});
                }));
            }
            for(auto & thread : threads)
            {
                thread.join();
            }
            std::vector<uint> counts(THREAD_COUNT,0);
            auto tailer = chronicle.create_tailer();
            while(tailer.next_index())
            {
                auto idx = tailer.read<std::uint32_t>();
                auto val = tailer.read<std::uint32_t>();
                REQUIRE(counts[idx]++ == val);
            }
            for(std::uint32_t i = 0; i != THREAD_COUNT; ++i)
            {
                INFO("Verifying counts for thread #" << i);
                REQUIRE(counts[i] == ITER_COUNT);
            }
        }
    }
}
