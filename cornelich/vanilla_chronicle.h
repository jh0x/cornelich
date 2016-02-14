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

#include "vanilla_chronicle_settings.h"
#include "vanilla_index.h"
#include "vanilla_data.h"
#include "excerpt_appender.h"
#include "excerpt_tailer.h"

#include <atomic>
#include <cstdint>
#include <iosfwd>

namespace cornelich
{

class vanilla_chronicle
{
public:
    explicit vanilla_chronicle(const vanilla_chronicle_settings & settings);

    std::int64_t index_block_size_mask() const { return m_index_block_size_mask; }
    std::int64_t index_block_longs_mask() const { return m_index_block_longs_mask; }
    std::int64_t data_block_size_mask() const { return m_data_block_size_mask; }
    std::int64_t entries_for_cycle_mask() const { return m_entries_for_cycle_mask; }

    /**
      * This method returns the very last index in the chronicle (by looking at the underlying data)
      * @return The last index in the file
      */
    std::int64_t last_index();
    std::int64_t last_written_index() const { return m_last_written_index; }

    excerpt_appender create_appender();
    excerpt_tailer create_tailer();
private:
    friend class excerpt_appender;
    friend class excerpt_tailer;

    const vanilla_chronicle_settings m_settings;
    const std::int32_t m_index_block_size_bits;
    const std::int64_t m_index_block_size_mask;

    const std::int32_t m_index_block_longs_bits;
    const std::int64_t m_index_block_longs_mask;

    const std::int32_t m_data_block_size_bits;
    const std::int64_t m_data_block_size_mask;

    const std::int32_t m_entries_for_cycle_bits;
    const std::int64_t m_entries_for_cycle_mask;

    vanilla_index m_index;
    vanilla_data m_data;

    std::atomic_int_fast64_t m_last_written_index;
};

}
