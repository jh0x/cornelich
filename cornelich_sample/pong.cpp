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

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <string>

using namespace cornelich;

const std::string EXPECTED1 {"FooBar"};
const std::string EXPECTED2 {"AnotherFooBar"};

void configure_parser(cli::Parser & parser)
{
    parser.set_optional<std::string>("i", "input", "/tmp/__test/chr", "Input chronicle path");
    parser.set_optional<std::size_t>("n", "max-count", 10000000, "Number of entries written");
    parser.set_optional<std::size_t>("w", "writer-threads", 4, "Number of writer threads");
    parser.set_optional<std::size_t>("r", "reader-threads", 4, "Number of reader threads");
    parser.set_optional<bool>("x", "delete", false, "Delete the input chronicle at startup");
}

int main(int argc, char **argv)
{
    cli::Parser parser(argc, argv);
    configure_parser(parser);
    parser.run_and_exit_if_error();

    const auto path = parser.get<std::string>("i");

    if(parser.get<bool>("x"))
        boost::filesystem::remove_all(path);

    vanilla_chronicle_settings settings(path);
    settings.thread_id_bits(16);
    vanilla_chronicle chr(settings);

    std::vector<std::thread> threads;
    const auto reader_thread_count = parser.get<std::size_t>("r");
    const auto max_count = parser.get<std::size_t>("w") * parser.get<std::size_t>("n");
    std::cout << "About to start reading from " << path << '\n';
    for(auto tid = 0u; tid != reader_thread_count; ++tid)
    {
        threads.push_back(std::thread([&chr, tid, settings, max_count]()
        {
            //std::this_thread::sleep_for(std::chrono::milliseconds(10*tid));
            using std::chrono::steady_clock;
            auto t0 = steady_clock::now();

            std::map<int, int> counts;
            auto tailer = chr.create_tailer();

            auto count = 0u;
            while(count != max_count)
            {
                if(!tailer.next_index())
                    continue;
                const auto id = tailer.read<std::int32_t>();
                const auto val = tailer.read(util::stop_bit::read);
                const auto long_value = tailer.read<std::int64_t>();
                const auto text1 = tailer.read(readers::chars());
                const auto text2 = tailer.read(readers::chars());
                if(BOOST_UNLIKELY(
                        counts[id]++ != val
                            || long_value != 0x0badcafedeadbeef
                            || text1 != boost::string_ref(EXPECTED1)
                            || text2 != boost::string_ref(EXPECTED2) ))
                {
                    std::cerr << "Unexpected value\n";
                    return 1;
                }
                ++count;
                if(count % 1000000 == 0)
                {
                    auto t1 = steady_clock::now();
                    auto diff = t1 - t0;
                    std::cout << tid << " " << count << " " << std::chrono::duration<double, std::milli>(diff).count() << " ms\n";
                }
            }
            auto t1 = steady_clock::now();
            auto diff = t1 - t0;
            std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;
            return 0;
        }));
    }
    for(auto & thread : threads)
    {
        thread.join();
    }
}
