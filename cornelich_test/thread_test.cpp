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

#include <cornelich/util/thread.h>

#include <cmath>
#include <fstream>

#include <catch.hpp>

using namespace cornelich::util;

TEST_CASE( "util::thread_id_bits:", "[util/thread]" )
{
    std::ifstream is("/proc/sys/kernel/threads-max");
    if(!is)
        throw std::runtime_error("Cannot access /proc/sys/kernel/threads-max");
    unsigned int v;
    is >> v;
    REQUIRE(thread_id_bits() == (int)std::log2(v) + 1);
}
