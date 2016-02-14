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

#include <boost/filesystem.hpp>

#include <iostream>
#include <chrono>

namespace fs = boost::filesystem;

class clean_up_fixture
{
public:
    clean_up_fixture()
        : m_path(fs::temp_directory_path() / fs::unique_path()) {}
    ~clean_up_fixture()
    {
        if(exists())
        {
            try
            {
                fs::remove_all(m_path);
            }
            catch (...) {
            }
        }
    }

    const fs::path & path() const { return m_path; }
    bool exists() const { return fs::exists(m_path); }
private:
    const fs::path m_path;
};

/** Primitive wrapper for measuring the execution time of a function */
template<typename TimeT = std::chrono::milliseconds>
struct measure
{
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F func, Args&&... args)
    {
        auto start = std::chrono::high_resolution_clock::now();

        func(std::forward<Args>(args)...);

        auto duration = std::chrono::duration_cast< TimeT>(std::chrono::high_resolution_clock::now() - start);

        return duration.count();
    }
};

inline char char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
inline void hex2bin(const char* src, std::uint8_t * target)
{
    while(*src && src[1])
    {
      *(target++) = static_cast<std::uint8_t>(char2int(*src)*16 + char2int(src[1]));
        src += 2;
    }
}
