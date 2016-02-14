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

#include "files.h"

#include "streamer.h"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace bip = boost::interprocess;
namespace fs = boost::filesystem;


namespace cornelich
{
namespace util
{

void touch(const std::string & path)
{
    std::ofstream f(path.c_str());
    if (!f)
    {
        throw std::runtime_error(util::streamer() << "Failed to create/open file " << path);
    }
}

bip::file_mapping create_mapping(const std::string & path, std::uint32_t size)
{
    if(!fs::exists(path))
    {
        touch(path);
    }
    if(fs::file_size(path) < size)
    {
        fs::resize_file(path, size);
    }
    auto mapping = bip::file_mapping(path.c_str(), bip::read_write);
    return mapping;
}


}
}
