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

#include "vanilla_utils.h"

#include "util/streamer.h"

#include <boost/filesystem/operations.hpp>

#include <chrono>
#include <stdexcept>

namespace fs = boost::filesystem;

namespace cornelich
{

std::string make_file(const std::string & base_path,
                      const std::string & cycle_string,
                      const std::string & file_name,
                      bool append)
{
    const auto dir = fs::path(base_path) / cycle_string;
    if(!append && !fs::exists(dir / file_name))
    {
        return {};
    }

    if(!fs::is_directory(dir))
    {
        fs::create_directories(dir);
    }

    const auto file = dir / file_name;
    if(fs::exists(file))
    {
        // "File exists:" << file;
    }
    else if(append)
    {
        // "File does not exist: " << file;
    }
    else
    {
        return {};
    }
    return file.string();
}

std::int32_t cycle_for_now(std::int32_t cycle_length)
{
    using namespace std::chrono;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return static_cast<std::int32_t>(now.count() / cycle_length);
}

}
