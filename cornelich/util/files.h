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

#include <boost/interprocess/file_mapping.hpp>

#include <string>

namespace cornelich
{
namespace util
{

/**
 * Create a file if doesn't exist, try to open for appending if already present.
 */
void touch(const std::string & path);

/**
 * Create an interprocess::file_mapping from a file with a given path
 */
boost::interprocess::file_mapping create_mapping(const std::string & path, std::uint32_t size);

}
}
