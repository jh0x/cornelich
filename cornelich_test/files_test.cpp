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
#include <cornelich/util/files.h>

#include <cstdint>

#include <boost/filesystem.hpp>

#include <catch.hpp>

using namespace cornelich::util;
namespace fs = boost::filesystem;

TEST_CASE_METHOD(clean_up_fixture, "File utils - create_mapping", "[util/files]")
{
    GIVEN("A temporary directory")
    {
        fs::create_directory(path());
        REQUIRE(exists());
        auto p = (path() / "foo");

        SECTION("We can create a mapping")
        {
            auto mapping = create_mapping(p.string(), 16384);
            REQUIRE(fs::exists(p));
            REQUIRE(fs::file_size(p) == 16384);
        }
    }
}

TEST_CASE_METHOD(clean_up_fixture, "File utils - touch & size", "[util/files]")
{
    GIVEN("A temporary directory")
    {
        fs::create_directory(path());
        REQUIRE(exists());
        auto p = (path() / "foo");

        SECTION("Touch and create a file")
        {
            touch(p.string());
            REQUIRE(fs::exists(p));
        }
    }
}
