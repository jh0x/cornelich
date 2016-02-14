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
#include <memory>
#include <string>

namespace cornelich
{

class cycle_formatter;

static constexpr std::int64_t min_cycle_length() { return 60 * 60 * 1000; }
static const std::string INDEX_FILE_NAME_PREFIX = "index-";
static const std::string DATA_FILE_NAME_PREFIX = "data-";
static constexpr std::int32_t DEFAULT_THREAD_ID_BITS = 16;

class vanilla_chronicle_settings
{
public:
    /// Create default settings with specified chronicle base path
    vanilla_chronicle_settings(const std::string & path);

    /// Return chronicle base path
    const std::string & path() const { return m_path; }

    /// Length of a cycle in milliseconds
    std::int32_t cycle_length() const { return m_cycle_length; }
    /// Set the length of a cycle in milliseconds. By default check for sane values of the parameter.
    vanilla_chronicle_settings & cycle_length(std::int32_t cycle_length, const std::shared_ptr<cycle_formatter> & formatter, bool check = true);

    /// Return the formatter used for converting cycles to date
    const cycle_formatter & cycle_format() const { return *m_cycle_format; }

    /// How many entries can be stored within one cycle
    std::int64_t entries_per_cycle() const { return m_entries_per_cycle; }
    /// Set the number of entries that can be stored in one cycle (must be a power of 2)
    vanilla_chronicle_settings & entries_per_cycle(std::int64_t entries_per_cycle) { m_entries_per_cycle = entries_per_cycle; return *this;}

    /// Size used for index regions
    std::int64_t index_block_size() const { return m_index_block_size; }
    /// Set the size used for storing index regions
    vanilla_chronicle_settings & index_block_size(std::int64_t index_block_size) { m_index_block_size = index_block_size; return *this;}

    /// Size used for data regions
    std::int64_t data_block_size() const { return m_data_block_size; }
    /// Set the size used for storing data regions
    vanilla_chronicle_settings & data_block_size(std::int64_t data_block_size) {m_data_block_size = data_block_size; return *this;}

    /// Number of bits used for storing the thread id in this chronicle
    std::int32_t thread_id_bits() const { return m_thread_id_bits; }
    /// Set the number of bits to use for storing the thread id
    vanilla_chronicle_settings & thread_id_bits(std::int32_t thread_id_bits) { m_thread_id_bits = thread_id_bits; return *this;}

    /// Mask used to validate that the thread id does not exceed the allocated number of bits.
    std::int64_t thread_id_mask() const { return (INT64_C(1) << m_thread_id_bits) - 1; }

    /// Number of least-significant bits used to hold the data offset info in index entries.
    std::int32_t index_data_offset_bits() const { return 64 - m_thread_id_bits; }

    /// Mask used to extract the data offset info from an index entry.
    std::int64_t index_data_offset_mask() const { return (INT64_C(1) << index_data_offset_bits()) - 1; }

    std::size_t index_cache_size() const { return m_index_cache_size; }
    vanilla_chronicle_settings & index_cache_size(std::size_t size) { m_index_cache_size = size; return *this; }

    std::size_t data_cache_size() const { return m_data_cache_size; }
    vanilla_chronicle_settings & data_cache_size(std::size_t size) { m_data_cache_size = size; return *this; }
private:
    friend std::ostream & operator<<(std::ostream &os, const vanilla_chronicle_settings & s);

    const std::string m_path;
    std::int32_t m_thread_id_bits;
    std::int32_t m_cycle_length;
    std::shared_ptr<cycle_formatter> m_cycle_format;
    std::int64_t m_entries_per_cycle;
    std::int64_t m_index_block_size;
    std::int64_t m_data_block_size;
    std::size_t m_index_cache_size;
    std::size_t m_data_cache_size;
};

std::ostream & operator<<(std::ostream & os, const vanilla_chronicle_settings & s);

}
