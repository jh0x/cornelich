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
using weak_region_ptr = std::weak_ptr<region>;
class vanilla_chronicle_settings;

/// This class manages the chronicle data files (mmaped regions)
class vanilla_data
{
public:
    /// Create a data-region manager using the given settings and desired region size
    vanilla_data(const vanilla_chronicle_settings & settings, std::int32_t data_block_size_bits);

    /// Find the next free region number for a specific (cycle / thread_id) pair
    /// If we have /tmp/chron/20151114/data-9791-0 and we want a next number for a cycle corresponding
    /// to 2015114 and thread 9791 we shall get 1
    std::int32_t find_next_data_file_number(std::int32_t cycle, std::int32_t thread_id) const;

    /// Return a pointer to a specific (cycle, thread_id, file_number) region
    /// If there is no such region AND we set for_write to false an empty pointer shall be returned.
    region_ptr data_for(std::int32_t cycle, std::int32_t thread_id, std::int32_t file_number, bool for_write);

private:
    const vanilla_chronicle_settings & m_settings;
    const std::int32_t m_data_block_size_bits;
    using mutex_t = util::spin_lock;
    mutex_t m_lock;
    using key_t = std::tuple<std::int32_t, std::int32_t, std::int32_t>;
    util::cache<key_t, region_ptr, region_ptr_validator> m_cache;
};

}
