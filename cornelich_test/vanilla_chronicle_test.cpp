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
#include <cornelich/formatters.h>

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

            SECTION("Finish without start throws")
            {
                appender.start_excerpt(128);
                appender.finish();
                REQUIRE_THROWS_AS(appender.finish(), std::logic_error);
            }
        }

        SECTION("Can append with the same appender on different threads though not recommended")
        {
            auto appender = chronicle.create_appender();

            std::thread t([&appender]() {write_test_data(appender, 0, 1000);});
            t.join();
            write_test_data(appender, 1, 1000);

            auto tailer = chronicle.create_tailer();
            for(int id = 0; id != 2; ++id)
            {
                for(int i = 0; i != 1000; ++i)
                {
                    REQUIRE(tailer.next_index());
                    REQUIRE(tailer.limit()==36);
                    REQUIRE(tailer.read<std::int32_t>() == id);
                    REQUIRE(tailer.read<std::int32_t>() == i);
                }
            }

        }

        SECTION("Can append with one appender and then with another")
        {
            {
                auto appender = chronicle.create_appender();
                write_test_data(appender, 0, 1000);
            }
            {
                auto appender = chronicle.create_appender();
                write_test_data(appender, 1, 1000);
            }
            auto tailer = chronicle.create_tailer();
            for(int id = 0; id != 2; ++id)
            {
                for(int i = 0; i != 1000; ++i)
                {
                    REQUIRE(tailer.next_index());
                    REQUIRE(tailer.limit()==36);
                    REQUIRE(tailer.read<std::int32_t>() == id);
                    REQUIRE(tailer.read<std::int32_t>() == i);
                }
            }
        }

        SECTION("Can write with different threads, one after another")
        {
            std::thread t([&chronicle]()
            {
                auto appender = chronicle.create_appender();
                write_test_data(appender, 0, 1000);
            });
            t.join();
            {
                auto appender = chronicle.create_appender();
                write_test_data(appender, 1, 1000);
            }
            auto tailer = chronicle.create_tailer();
            for(int id = 0; id != 2; ++id)
            {
                for(int i = 0; i != 1000; ++i)
                {
                    REQUIRE(tailer.next_index());
                    REQUIRE(tailer.limit()==36);
                    REQUIRE(tailer.read<std::int32_t>() == id);
                    REQUIRE(tailer.read<std::int32_t>() == i);
                }
            }
        }

        SECTION("Can read and write simultaneously")
        {
            std::atomic_bool done(false);

            std::thread t([&chronicle, &done]()
            {
                auto appender = chronicle.create_appender();
                write_test_data(appender, 0, 1000);
                done.store(true);
            });

            int idx = 0;
            auto tailer = chronicle.create_tailer();
            while(!done)
            {
                while(tailer.next_index())
                {
                    REQUIRE(tailer.limit()==36);
                    REQUIRE(tailer.read<std::int32_t>() == 0);
                    REQUIRE(tailer.read<std::int32_t>() == idx);
                    ++idx;
                }
            }
            REQUIRE(idx == 1000);
            t.join();
        }

        SECTION("Can write with multiple writers")
        {
            constexpr auto THREAD_COUNT = 8u;
            constexpr auto ITER_COUNT = 10000u;
            std::vector<std::thread> threads;
            for(auto tid = 0u; tid != THREAD_COUNT; ++tid)
            {
                threads.push_back(std::thread([&chronicle,tid]()
                {
                    auto appender = chronicle.create_appender();
                    write_test_data(appender, tid, ITER_COUNT, [](){std::this_thread::sleep_for(std::chrono::microseconds(10));});
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
                REQUIRE(counts[i] == ITER_COUNT);
        }

    }
}

