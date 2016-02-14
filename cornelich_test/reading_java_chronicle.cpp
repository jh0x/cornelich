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


#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>

#include <map>
#include <catch.hpp>

#ifndef _CORNELICH_TEST_ABSOLUTE_PATH
#define _CORNELICH_TEST_ABSOLUTE_PATH
#endif

using namespace cornelich;

constexpr std::uint64_t LAST_INDEX_IN_SAMPLE_CHRONICLE = 0x41710000000f9f;

/*
Test chronicle: 4 threads doing this:
for(int i = 0; i != 1000; ++i) {
    appender.startExcerpt(128);
    appender.writeLong(id);
    appender.writeInt(i);
    appender.writeLong(0x0123456789ABCDEFL);
    appender.writeInt(1);
    appender.writeInt(2);
    appender.writeInt(3);
    appender.writeInt(4);
    appender.writeInt(5);
    appender.finish();
}
*/

TEST_CASE( "Reading data from a sample chronicle", "[vanilla_chronicle]" )
{
    GIVEN("A non-existent chronicle")
    {
        vanilla_chronicle_settings settings("foobar");
        vanilla_chronicle chronicle(settings);
        REQUIRE(chronicle.last_index() == -1);
    }

    GIVEN("An existing chronicle")
    {
        vanilla_chronicle_settings settings(_CORNELICH_TEST_ABSOLUTE_PATH "/SampleVanillaChronicle");
        settings.thread_id_bits(15);
        vanilla_chronicle chronicle(settings);
        REQUIRE(chronicle.last_index() == LAST_INDEX_IN_SAMPLE_CHRONICLE);

        GIVEN("A tailer")
        {
            auto tailer = chronicle.create_tailer();
            REQUIRE(tailer.index() == -1);

            SECTION("Tailer can go to the end")
            {
                tailer.to_end();
                REQUIRE(tailer.index() == LAST_INDEX_IN_SAMPLE_CHRONICLE);
            }

            SECTION("The number of elements in the chronicle is correct")
            {
                auto count = 0;
                while(tailer.next_index())
                    ++count;
                REQUIRE(count == 8000);
            }

            SECTION("Aggregates computed from the contents are correct")
            {
                for(int day = 0; day != 2; ++day)
                {
                    std::map<std::int64_t, int> counts;
                    for(int i = 0; i != 4000; ++i)
                    {
                        tailer.next_index();
                        auto id = tailer.read<std::int64_t>();
                        auto val = tailer.read<std::int32_t>();
                        REQUIRE(counts[id]++ == val);
                    }
                }
            }

            SECTION("Iterating through the chronicle checking the contents")
            {
                int count = 0;

                REQUIRE(tailer.next_index());
                ++count;
                {
                    REQUIRE(tailer.position() == 0);
                    REQUIRE(tailer.limit() == 40);
                    REQUIRE(tailer.read<std::int64_t>() == 3L);
                    REQUIRE(tailer.read<std::int32_t>() == 0);
                    REQUIRE(tailer.read<std::int64_t>() == 0x0123456789ABCDEFLL);
                    REQUIRE(tailer.read<std::int32_t>() == 1);
                    REQUIRE(tailer.read<std::int32_t>() == 2);
                    REQUIRE(tailer.read<std::int32_t>() == 3);
                    REQUIRE(tailer.read<std::int32_t>() == 4);
                    REQUIRE(tailer.read<std::int32_t>() == 5);
                    REQUIRE(tailer.position() == 40);

                    REQUIRE(tailer.position(0));
                    REQUIRE(tailer.read<std::int64_t>() == 3L);
                    REQUIRE(tailer.position() == 8);
                }
                for(int i = 0; i != 3456; ++i)
                {
                    tailer.next_index();
                    ++count;
                }
                {
                    REQUIRE(tailer.position() == 0);
                    REQUIRE(tailer.limit() == 40);
                    REQUIRE(tailer.read<std::int64_t>() == 1L);
                    REQUIRE(tailer.read<std::int32_t>() == 864);
                    REQUIRE(tailer.read<std::int64_t>() == 0x0123456789ABCDEFLL);
                    REQUIRE(tailer.read<std::int32_t>() == 1);
                    REQUIRE(tailer.read<std::int32_t>() == 2);
                    REQUIRE(tailer.read<std::int32_t>() == 3);
                    REQUIRE(tailer.read<std::int32_t>() == 4);
                    REQUIRE(tailer.read<std::int32_t>() == 5);
                    REQUIRE(tailer.position() == 40);
                }
                while(tailer.next_index())
                    ++count;
                REQUIRE(count == 8000);
            }
        }

    }
}
