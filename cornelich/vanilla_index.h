#pragma once

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

#include "region_utils.h"

#include "util/cache.h"
#include "util/spin_lock.h"

#include <cstdint>
#include <memory>
//#include <mutex>
#include <tuple>

namespace cornelich
{

class region;
using region_ptr = std::shared_ptr<region>;
class vanilla_chronicle_settings;

/// This class manages the chronicle index files (mmaped regions)
class vanilla_index
{
public:
    /// Create an index-region manager using the given settings and desired region size
    vanilla_index(const vanilla_chronicle_settings & settings, std::int32_t index_block_size_bits);

    std::int32_t find_first_cycle() const;
    std::int32_t find_last_cycle() const;

    /// Find the next free region number for a specific cycle
    /// If we have /tmp/chron/20151114/index-0 and we want a next number for a cycle corresponding
    /// to 2015114 we shall get 1
    /// default_cycle shall be returned if there is no index file at all for that cycle
    std::int32_t last_index_file_number(std::int32_t cycle, std::int32_t default_cycle) const;

    /// Return a pointer to a specific (cycle, file_number) region
    /// If there is no such region AND we set for_write to false an empty pointer shall be returned.
    region_ptr index_for(std::int32_t cycle, std::int32_t file_number, bool append);

    /// Append a new value (index_value) to the index for a given cycle.
    /// Will append to the index file with number >= file_number
    /// Return a pair: {ptr to the index region; position at which the value was stored}
    std::pair<region_ptr, std::int64_t> append(std::int32_t cycle, std::int64_t index_value, std::int32_t file_number);

    /// Return the number of (non-zero) index entries in the given region
    static std::int64_t count_index_entries(const region & region);

    /// Attempt to atomically append (CAS) a value in the index region
    /// Return offset at which the value was appended or -1 on failure (e.g. region full)
    static std::int64_t append(region & region, std::int64_t index_value);

private:
    const vanilla_chronicle_settings & m_settings;
    const std::int32_t m_index_block_size_bits;
    using mutex_t = util::spin_lock;
    mutex_t m_lock;
    using key_t = std::tuple<std::int32_t, std::int32_t>;
    util::cache<key_t, region_ptr, region_ptr_validator> m_cache;
};

}
