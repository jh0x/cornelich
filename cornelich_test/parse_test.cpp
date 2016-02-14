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

#include <cornelich/util/parse.h>

#include <string>

#include <catch.hpp>

using namespace cornelich::util;

void test_parse(const std::string & text, int expected)
{
    INFO("text=" << text << ", expected=" << expected);
    int actual;
    REQUIRE(parse_number(text.begin(), text.end(), actual));
    REQUIRE(actual == expected);
}

void test_parse_fail(const std::string & text)
{
    int foo;
    bool result = parse_number(text.begin(), text.end(), foo);
    INFO("text=" << text << ", result=" << result << ", foo=" << foo);
    REQUIRE(!result);
}

TEST_CASE( "util::parse", "[util/parse]")
{
    test_parse("0", 0);
    test_parse("-213", -213);
    test_parse("-2147483648", -2147483648);
    test_parse("2147483647", 2147483647);
    test_parse("20BLABLA", 20);
    test_parse_fail("");
    test_parse_fail("-");
    test_parse_fail("-A");
    test_parse_fail("foo");
    test_parse_fail("bLA");
}
