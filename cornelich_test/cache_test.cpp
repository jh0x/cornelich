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

#include <cornelich/util/cache.h>
#include <cornelich/util/streamer.h>

#include <utility>
#include <string>
#include <catch.hpp>

using namespace cornelich;

using key_type = std::pair<int, int>;
using value_type = std::string;

struct provider
{
    provider(bool & c) : called(c) { called = false; }
    std::string operator()(const key_type & k) const
    {
        called = true;
        return util::streamer() << k.first << " " << k.second;
    }
    bool & called;
};

TEST_CASE( "util::cache", "[util/cache]")
{
    GIVEN("An empty cache with the default verifier")
    {
        util::cache<key_type, value_type> cache(4);
        bool called = false;
        REQUIRE(cache.size() == 0);
        REQUIRE(cache.capacity() == 4);

        SECTION("Caching")
        {
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);

            REQUIRE(cache.size() == 1);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == true);

            REQUIRE(cache.size() == 2);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == true);

            REQUIRE(cache.size() == 3);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == true);

            REQUIRE(cache.size() == 4);
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == false);

            REQUIRE(cache.size() == 4);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == false);

            REQUIRE(cache.get({0, 3}, provider(called)) == "0 3");
            REQUIRE(called == true);

            REQUIRE(cache.size() == 4);
        }

        SECTION("LRU behaviour")
        {
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == true);
            // 00 10 20 30

            REQUIRE(cache.get({4, 0}, provider(called)) == "4 0");
            REQUIRE(called == true);
            // We don't have 40, We evict 00:
            //    10 20 30 40

            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == false);
            // We have 10, 10 changes position
            //    20 30 40 10

            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);
            // We don't have 10, We evict 20
            //       30 40 10 00

            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == false);
            // We have 30, 30 changes position
            //          40 10 00 30

            REQUIRE(cache.get({5, 5}, provider(called)) == "5 5");
            REQUIRE(called == true);
            // We don't have 55, We evict 40
            //             10 00 30 55

            REQUIRE(cache.get({4, 0}, provider(called)) == "4 0");
            REQUIRE(called == true);
            // We don't have 40 We evict 10
            //                00 30 55 40
        }
    }

    GIVEN("An empty cache with a custom verifier")
    {
        auto validator = [](const value_type & v) {
            // Let's say 0 0 is not valid.
            return v != "0 0";
        };
        util::cache<key_type, value_type, decltype(validator)> cache(4, validator);
        bool called = false;
        REQUIRE(cache.size() == 0);
        REQUIRE(cache.capacity() == 4);

        SECTION("Not caching if not validated")
        {
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);
            REQUIRE(cache.size() == 0);
        }

        SECTION("Caching & LRU")
        {
            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);

            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == true);
            REQUIRE(cache.get({4, 0}, provider(called)) == "4 0");
            REQUIRE(called == true);

            // 10 20 30 40

            REQUIRE(cache.get({0, 0}, provider(called)) == "0 0");
            REQUIRE(called == true);

            REQUIRE(cache.get({4, 0}, provider(called)) == "4 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == false);

            // 40 30 20 10

            REQUIRE(cache.get({9, 9}, provider(called)) == "9 9");
            REQUIRE(called == true);

            //   30 20 10 99

            REQUIRE(cache.get({3, 0}, provider(called)) == "3 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({2, 0}, provider(called)) == "2 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({1, 0}, provider(called)) == "1 0");
            REQUIRE(called == false);
            REQUIRE(cache.get({9, 9}, provider(called)) == "9 9");
            REQUIRE(called == false);
        }
    }
}
