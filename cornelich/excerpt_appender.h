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

#include "region.h"
#include "util/buffer_view.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>

namespace cornelich
{

class vanilla_chronicle;
using region_ptr = std::shared_ptr<region>;

class excerpt_appender
{
public:
    excerpt_appender(vanilla_chronicle & chronicle);

    std::int64_t index() const { return m_index; }
    std::int64_t last_written_index() const { return m_last_written_index; }

    void start_excerpt(std::int32_t capacity);

    void finish();

    util::buffer_view & buffer() { return m_buffer; }
    const util::buffer_view & buffer() const { return m_buffer; }

    template <typename T>
    void write(T val);
    template <typename T, typename WRITER>
    void write(T && val, WRITER && wrt);

private:
    void start_excerpt(std::int32_t capacity, std::int32_t cycle);
    std::int64_t index_from(std::int64_t cycle, std::int64_t index_count, std::int64_t index_position) const;
    void set_last_written_index(std::int64_t cycle, std::int64_t index_count, std::int64_t inde_position);

    vanilla_chronicle & m_chronicle;

    region_ptr m_index_region;
    region_ptr m_data_region;

    std::int64_t m_index;
    std::int32_t m_last_cycle;
    std::int32_t m_last_index_file_number;

    std::int32_t m_last_thread_id;
    std::int32_t m_last_data_file_number;

    bool m_finished;

    std::int64_t m_last_written_index = -1;
    util::buffer_view m_buffer;

};

template<typename T>
BOOST_FORCEINLINE void excerpt_appender::write(T val)
{
    static_assert(std::is_pod<T>::value, "excerpt_tailer::read(): POD expected for T");
    std::memcpy(m_buffer.data() + m_buffer.position(), &val, sizeof(T));
    m_buffer.position() += static_cast<std::int32_t>(sizeof(T));
}

template <typename T, typename WRITER>
BOOST_FORCEINLINE void excerpt_appender::write(T && val, WRITER && wrt)
{
    wrt(m_buffer.data(), m_buffer.position(), std::forward<T>(val));
}

}
