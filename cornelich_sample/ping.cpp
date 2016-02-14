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
#include <cornelich/vanilla_date.h>
#include <cornelich/formatters.h>

#include <cornelich/util/stop_bit.h>
#include <cornelich/util/math_util.h>

#include <boost/filesystem.hpp>

#include <cmdparser/cmdparser.hpp>

#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <string>

using namespace cornelich;

void configure_parser(cli::Parser & parser)
{
    parser.set_optional<std::string>("o", "output", "/tmp/__test/chr", "Output chronicle path");
    parser.set_optional<std::size_t>("n", "max-count", 10000000, "Number of entries written");
    parser.set_optional<std::size_t>("w", "writer-threads", 4, "Number of writer threads");
    parser.set_optional<bool>("x", "delete", false, "Delete the input chronicle at startup");
}

int main(int argc, char **argv)
{
    cli::Parser parser(argc, argv);
    configure_parser(parser);
    parser.run_and_exit_if_error();

    const auto path = parser.get<std::string>("o");

    if(parser.get<bool>("x"))
        boost::filesystem::remove_all(path);

    vanilla_chronicle_settings settings(path);
    settings.thread_id_bits(16);
    vanilla_chronicle chr(settings);

    std::vector<std::thread> threads;
    const auto writer_thread_count = parser.get<std::size_t>("w");
    const auto max_count = parser.get<std::size_t>("n");

    using std::chrono::steady_clock;
    auto t0 = steady_clock::now();


    for(auto tid = 0u; tid != writer_thread_count; ++tid)
    {
        threads.push_back(std::thread([&chr, tid, settings, max_count]()
        {
            auto appender = chr.create_appender();

            for(auto i = 0u; i != max_count; ++i)
            {
                appender.start_excerpt(8192);
                appender.write(tid);
                appender.write(i, util::stop_bit::write);
                appender.write(0x0badcafedeadbeef);
                appender.write("FooBar", writers::chars());
                appender.write("AnotherFooBar", writers::chars());
                appender.finish();
            }
            return 0;
        }));
    }
    for(auto & thread : threads)
    {
        thread.join();
    }
    auto t1 = steady_clock::now();
    auto diff = t1 - t0;
    std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;
}
