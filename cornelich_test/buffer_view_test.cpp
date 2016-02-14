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

#include <cornelich/util/buffer_view.h>

#include <array>
#include <cstdint>

#include <catch.hpp>

using namespace cornelich::util;


template<typename T>
void test_detached_view(T & view)
{
    REQUIRE(view.data() == nullptr);
    REQUIRE(view.limit() == 0);
    REQUIRE(view.position() == 0);
    REQUIRE(view.remaining() == 0);
}

TEST_CASE( "util::buffer_view", "[util/buffer_view]" )
{

    GIVEN("A detached buffer_view")
    {
        std::int64_t idx = -1;
        buffer_view view(idx);
        view.reset();

        test_detached_view(view);
        const auto & cview = view;
        test_detached_view(cview);
    }


    GIVEN("A chunk of memory and a buffer_view around it")
    {
        std::int64_t idx = -1;
        std::array<uint8_t, 1024> buffer;
        buffer[16] = 0xff;
        buffer[64] = 0xdd;

        auto view = buffer_view(idx);
        view.reset(buffer.data() + 16, 48, buffer.size() - 16);

        WHEN("We inspect the contents of the buffer")
        {
            REQUIRE(view.data() == buffer.data() + 16);
            REQUIRE(view.limit() == 1024 - 16);
            REQUIRE(view.position() == 48);
            REQUIRE(view.remaining() == 1024 - 48 - 16);
            REQUIRE(view.data()[0] == 0xff);
            REQUIRE(view.data()[48] == 0xdd);
            REQUIRE(view.index() == idx);
        }

        WHEN("We move the position")
        {
            view.position() = 256;
            REQUIRE(view.data() == buffer.data() + 16);
            REQUIRE(view.limit() == 1024 - 16);
            REQUIRE(view.position() == 256);
            REQUIRE(view.remaining() == 1024 - 256 - 16);
            REQUIRE(view.data()[0] == 0xff);
            REQUIRE(view.data()[48] == 0xdd);
            REQUIRE(view.index() == idx);
        }
    }
}
