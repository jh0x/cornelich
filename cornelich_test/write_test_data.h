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

#pragma once

#include <cornelich/excerpt_appender.h>
#include <cstdint>

using namespace cornelich;

auto do_nothing = [](){};

template<typename T = decltype(do_nothing)>
void write_test_data(excerpt_appender & appender, std::uint32_t id, std::uint32_t count, T post_single_append = do_nothing)
{
    for(std::uint32_t i = 0; i != count; ++i)
    {
        appender.start_excerpt(128);
        appender.write(id);
        appender.write(i);
        appender.write(0x0123456789ABCDEFLL);
        appender.write(1);
        appender.write(2);
        appender.write(3);
        appender.write(4);
        appender.write(5);
        appender.finish();
        post_single_append();
    }
}
