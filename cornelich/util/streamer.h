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

#include <sstream>
#include <string>

namespace cornelich
{
namespace util
{

/**
 * The streamer class for making custom exception strings easier
 * <pre>
 * throw std::invalid_argument(util::streamer() << "Expected positive value, but got " << x);
 * </pre>
 */
class streamer
{
public:
    streamer() {}
    streamer(const streamer &) = delete;
    streamer & operator = (streamer &) = delete;

    template <typename T>
    streamer & operator<<(T && value)
    {
        m_stream << std::forward<T>(value);
        return *this;
    }

    std::string str() const { return m_stream.str(); }
    operator std::string () const { return m_stream.str(); }

private:
    std::stringstream m_stream;
};

}
}
