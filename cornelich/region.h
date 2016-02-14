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

#include <atomic>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

#include <boost/interprocess/mapped_region.hpp>

namespace cornelich
{

/// A wrapper for mmaped file.
class region
{
public:
    /**
     * @brief Create a mapping. The file will be created if it does not exist.
     * @param path Location of the file
     * @param size Size of the region to map
     * @param index Index of the file
     */
    region(const std::string & path, std::uint32_t size, std::int32_t index);

    std::string path() const { return m_path; }

    /// Size of the mmaped region
    std::int64_t size() const { return m_size; }

    /// Index assigned during the creation
    std::int32_t index() const { return m_index; }

    /// Pointer to the beginning of the region
    const std::uint8_t * data() const { return static_cast<const std::uint8_t *>(m_region.get_address()); }
    /// Pointer to the beginning of the region
    std::uint8_t * data() { return static_cast<std::uint8_t *>(m_region.get_address()); }

    /// Perform an ordered read (4 bytes) from a given offset
    std::int32_t read_ordered32(std::int32_t offset) const;
    /// Perform an ordered read (8 bytes) from a given offset
    std::int64_t read_ordered64(std::int32_t offset) const;

    /// Perform an ordered write (4 bytes) to the given location
    void write_ordered32(std::int32_t offset, std::int32_t value);

    /// Perform an 8-byte CAS operation at a given offset
    bool cas64(std::int32_t offset, std::int64_t expected, std::int64_t x);

    /// Align the position in the region to a value divisible by the given parameter
    void align_position(std::int32_t value);

    /// Return the current position in the region
    std::int32_t position() const { return m_position_offset.load(std::memory_order_relaxed) - m_start_offset; }

    /// Attempt to set the position in the region. Return true if succeeded
    bool position(std::int32_t position);

    /// Return the capacity of the region
    std::int32_t limit() const { return m_limit_offset - m_start_offset; }

    /// Return the number of bytes remaining in the region according to the current position
    std::int32_t remaining() const { return m_limit_offset - m_position_offset; }
private:
    region(const region &) = delete;
    region & operator=(const region &) = delete;

    const std::string m_path;
    const std::int32_t m_index;
    const std::int64_t m_size;
    boost::interprocess::mapped_region m_region;

    const std::int32_t m_start_offset;
    const std::int32_t m_limit_offset;
    std::atomic<std::int32_t> m_position_offset;
};

BOOST_FORCEINLINE
std::int64_t region::read_ordered64(std::int32_t offset) const
{
    assert((offset & 7) == 0);
    auto * ptr  = reinterpret_cast<const volatile std::int64_t *>(data() + offset);
    //__asm__ __volatile__ ("" ::: "memory");
    //auto val = *ptr;
    auto val = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    return val;
}

BOOST_FORCEINLINE
std::int32_t region::read_ordered32(std::int32_t offset) const
{
    assert((offset & 3) == 0);
    auto * ptr  = reinterpret_cast<const volatile std::int32_t *>(data() + offset);
    //__asm__ __volatile__ ("" ::: "memory");
    //auto val = *ptr;
    auto val = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    return val;
}

BOOST_FORCEINLINE
void region::write_ordered32(std::int32_t offset, std::int32_t value)
{
    //__asm__ __volatile__ ("" ::: "memory");
    //*reinterpret_cast<volatile std::int32_t *>(data() + offset) = value;
    auto * ptr = reinterpret_cast<volatile std::int32_t *>(data() + offset);
    __atomic_store_n(ptr, value, __ATOMIC_RELAXED);
}

BOOST_FORCEINLINE
bool region::cas64(std::int32_t offset, std::int64_t expect, std::int64_t update)
{
    return __sync_bool_compare_and_swap(reinterpret_cast<volatile std::int64_t*>(data() + offset), expect, update);
}

}
