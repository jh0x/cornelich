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

#include <cstdint>
#include <string>

namespace cornelich
{

/**
 * Return a path to a file in the chronicle folder (located under base_path/cycle_string/file_name).
 * Depending on the append parameter value:
 * * if false:
 * *********** Return the path if the file_name exists at the given path
 * *********** Return false if any of the path elements is missing
 * * if true:
 * *********** Return the path if the file_name exists at the given path
 * *********** Return the path to the file and create the necessary directories if missing (no files get created here)
 */
std::string make_file(const std::string & base_path,
                      const std::string & cycle_string,
                      const std::string & file_name,
                      bool append);


/// Return a cycle number corresponding to the current time
std::int32_t cycle_for_now(std::int32_t cycle_length);

}
