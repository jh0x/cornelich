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

#include <cornelich/region.h>

#include <cstdint>
#include <fstream>
#include <iterator>

#include <catch.hpp>

using namespace cornelich;

constexpr std::size_t SIZE = 65536;

TEST_CASE_METHOD(clean_up_fixture, "Memory mapped region", "[region]")
{
    GIVEN("A temporary directory")
    {
        fs::create_directory(path());
        REQUIRE(exists());
        auto p = path() / "foo";

        GIVEN("A region")
        {
            region r(p.string(), SIZE, 0);
            REQUIRE(fs::exists(p));

            SECTION("Basic properties are correct")
            {
                REQUIRE(r.size() == SIZE);
                REQUIRE(r.index() == 0);
                REQUIRE(r.position() == 0);
                REQUIRE(r.limit() == SIZE);
                REQUIRE(r.remaining() == SIZE);
            }

            SECTION("Position/size calculations are correct")
            {
                REQUIRE(r.position(512) == true);
                REQUIRE(r.size() == SIZE);
                REQUIRE(r.position() == 512);
                REQUIRE(r.limit() == SIZE);
                REQUIRE(r.remaining() == SIZE - 512);
            }

            SECTION("Cannot go beyond the region")
            {
                REQUIRE(r.position(1) == true);
                REQUIRE(r.position(SIZE + 1) == false);
                REQUIRE(r.position() == 1);
                REQUIRE(r.limit() == SIZE);
                REQUIRE(r.remaining() == SIZE - 1);
            }

            SECTION("Can IO region data")
            {
                REQUIRE(std::all_of(r.data(), r.data() + SIZE, [](std::uint8_t v){return v == 0;}));

                std::fill_n(r.data(), SIZE, 0x01);
                REQUIRE(std::all_of(r.data(), r.data() + SIZE, [](std::uint8_t v){return v == 0x01;}));
            }
        }

        SECTION("Region data is preserved after closing")
        {
            {
                region r(p.string(), SIZE, 0);
                std::fill_n(r.data(), SIZE, 0x01);
            }
            REQUIRE(fs::exists(p));
            std::ifstream is(p.string().c_str(), std::ios::binary);
            REQUIRE(std::all_of(std::istreambuf_iterator<char>(is),
                                std::istreambuf_iterator<char>(),
                                [](std::uint8_t v){return v == 0x01;}));
        }
    }
}
