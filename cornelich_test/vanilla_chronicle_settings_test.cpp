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

#include <catch.hpp>

#include <string>

using namespace cornelich;

TEST_CASE( "Default settings are set", "[vanilla_chronicle_settings]" )
{
    REQUIRE(min_cycle_length() == 3600000);
    REQUIRE(std::string(INDEX_FILE_NAME_PREFIX) == "index-");
    REQUIRE(std::string(DATA_FILE_NAME_PREFIX) == "data-");
    REQUIRE(DEFAULT_THREAD_ID_BITS == 16);

    GIVEN( "Default settings" )
    {
        vanilla_chronicle_settings settings("foo");
        REQUIRE(settings.path() == "foo");
        REQUIRE(settings.cycle_length() == 86400000ULL);
        REQUIRE(settings.entries_per_cycle() == 0x10000000000ULL);
        REQUIRE(settings.index_block_size() == 0x1000000ULL);
        REQUIRE(settings.data_block_size() == 0x4000000ULL);
        REQUIRE(settings.thread_id_mask() == 0xFFFF);
        REQUIRE(settings.index_data_offset_bits() == 48);
        REQUIRE(settings.index_data_offset_mask() == 0xFFFFFFFFFFFFLL);

    }
}
