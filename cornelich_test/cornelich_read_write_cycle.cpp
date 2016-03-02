/*
Copyright 2016 Joanna Hulboj <j@hulboj.org>
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
#include <cornelich/vanilla_date.h>

#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

#include <catch.hpp>

using namespace cornelich;

TEST_CASE_METHOD(clean_up_fixture, "Excerpt missed on cycle boundary")
{
    vanilla_chronicle_settings settings(path().c_str());
    settings.data_block_size(1ULL << 20);
    settings.index_block_size(1ULL << 13);
    settings.cycle_length(1000 * 60, std::make_shared<cycle_formatter_yyyymmddhhmm>(1000 * 60), false);
    settings.entries_per_cycle(1 << 29);
    vanilla_chronicle chronicle(settings);
    REQUIRE(chronicle.last_index() == -1);

    std::vector<int> read_by_reader;

    auto reader = std::thread([&read_by_reader, &chronicle]()
    {
        auto tailer = chronicle.create_tailer();
        while(read_by_reader.empty() || *--read_by_reader.end() != 100)
        {
            // next_index looks for new entries in the current index
            // it also looks to see if there are index files corresponding to higher cycle numbers and uses them if
            // they are present
            if(tailer.next_index())
            {
                auto i = tailer.read<int>();
                read_by_reader.push_back(i);
            }
        }
    });

    util::spin_lock spin;
    std::unique_lock<util::spin_lock> lock(spin);

    auto slow_writer = std::thread([&chronicle, &spin]()
    {
        auto appender = chronicle.create_appender();
        appender.start_excerpt(32);

        std::lock_guard<util::spin_lock> lk(spin);

        appender.write<int>(9999);
        appender.finish();
    });

    auto fast_writer = std::thread([&chronicle, &lock]()
    {
        auto appender = chronicle.create_appender();

        for(int i = 0; i != 100; ++i)
        {
            appender.start_excerpt(32);
            appender.write<int>(i + 1);
            appender.finish();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // After at most 60 seconds a new cycle should start. Some time later we can unblock the other appender.
            if(i == 80)
            {
                lock.unlock();
            }
        }
    });

    reader.join();
    slow_writer.join();
    fast_writer.join();

    std::vector<int> actual;
    auto tailer = chronicle.create_tailer();
    while(tailer.next_index())
        actual.push_back(tailer.read<int>());

    REQUIRE(read_by_reader == actual);
}
