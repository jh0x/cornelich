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
#include <memory>
#include <type_traits>

namespace cornelich
{

class vanilla_chronicle;
using region_ptr = std::shared_ptr<region>;

class excerpt_tailer
{
public:
    excerpt_tailer(vanilla_chronicle & chronicle);

    std::int64_t index() const { return m_index; }
    bool index(std::int64_t index);

    excerpt_tailer & to_start();
    excerpt_tailer & to_end();

    bool next_index();

    util::buffer_view & buffer() { return m_buffer; }
    const util::buffer_view & buffer() const { return m_buffer; }

    std::int32_t position() const { return m_buffer.position(); }
    bool position(std::int32_t position);
    std::int32_t limit() const { return m_buffer.limit(); }

    template <typename T>
    T read();
    template <typename READER>
    typename std::result_of<READER(std::uint8_t*, std::int32_t&)>::type read(READER && rdr);
private:
    vanilla_chronicle & m_chronicle;

    region_ptr m_index_region;
    region_ptr m_data_region;

    std::int64_t m_index;
    std::int32_t m_last_cycle;
    std::int32_t m_last_index_file_number;

    std::int32_t m_last_thread_id;
    std::int32_t m_last_data_file_number;

    util::buffer_view m_buffer;
};

template<typename T>
BOOST_FORCEINLINE T excerpt_tailer::read()
{
    static_assert(std::is_pod<T>::value, "excerpt_tailer::read(): POD expected for T");
    //auto && val = *reinterpret_cast<const T *>(m_buffer.data() + m_buffer.position());
    // memcpy the data to avoid alignment issues
    T val;
    std::memcpy(&val, m_buffer.data() + m_buffer.position(), sizeof(T));
    m_buffer.position() += static_cast<std::int32_t>(sizeof(T));
    return val;
}

template <typename READER>
BOOST_FORCEINLINE typename std::result_of<READER(std::uint8_t*, std::int32_t&)>::type excerpt_tailer::read(READER && rdr)
{
    return rdr(m_buffer.data(), m_buffer.position());
}

}
