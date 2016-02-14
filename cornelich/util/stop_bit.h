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
#include <stdexcept>

namespace cornelich
{
namespace util
{
namespace stop_bit
{
/**
 * Read a stop-bit encoded integer from the buffer
 * @param buffer Buffer from which the value should be read
 * @param[in,out] offset Offset from which the reading should start.
 *                       It will be adjusted to point after the stop-bit-encoded number.
 * @return Value that has been read
 * @throw std::logic_error in case of overflow
 */
std::int64_t read(const std::uint8_t * buffer, std::int32_t & offset);

/**
 * Write a stop-bit encoded integer into the buffer
 * @param buffer Buffer into which the number should be written
 * @param offset[in,out] Offset from which the writing should start.
 *                       It will be adjusted to point after the stop-bit-encoded number
 * @param value Value to be written
 */
void write(std::uint8_t * buffer, std::int32_t & offset, std::int64_t value);
}
}
}
