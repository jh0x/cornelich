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

#include "vanilla_index.h"

#include "vanilla_chronicle_settings.h"
#include "vanilla_date.h"
#include "vanilla_utils.h"
#include "region.h"

#include "util/parse.h"
#include "util/streamer.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
namespace fs = boost::filesystem;

#include <boost/algorithm/string.hpp>
#include <memory>
#include <limits>
#include <mutex>

namespace cornelich
{

vanilla_index::vanilla_index(const vanilla_chronicle_settings & settings, std::int32_t index_block_size_bits)
    : m_settings(settings)
    , m_index_block_size_bits(index_block_size_bits)
    , m_cache(settings.index_cache_size(), region_ptr_validator())
{
}

std::int32_t vanilla_index::find_first_cycle() const
{
    auto first_cycle = std::numeric_limits<std::int32_t>::max();
    boost::system::error_code err;
    fs::directory_iterator begin(m_settings.path(), err);
    fs::directory_iterator end;

    // No directories -> empty
    if(begin == end)
        return -1;

    // Find the earliest directory
    for(const auto & entry : boost::make_iterator_range(begin, end))
    {
        if(!fs::is_directory(entry))
            continue;
        const auto date = m_settings.cycle_format().cycle_from_date(entry.path().filename().string());
        if (first_cycle > date)
            first_cycle = date;
    }

    // Did we find anything?
    return first_cycle == std::numeric_limits<std::int32_t>::max() ? -1 : first_cycle;
}

std::int32_t vanilla_index::find_last_cycle() const
{
    auto last_cycle = -1;
    boost::system::error_code err;
    fs::directory_iterator begin(m_settings.path(), err);
    fs::directory_iterator end;

    // No directories -> empty
    if(err || begin == end)
        return last_cycle;

    // Find the newest directory
    for(const auto & entry : boost::make_iterator_range(begin, end))
    {
        if(!fs::is_directory(entry))
            continue;
        const auto cycle = m_settings.cycle_format().cycle_from_date(entry.path().filename().string());
        if (last_cycle < cycle)
            last_cycle = cycle;
    }
    return last_cycle;
}

std::int32_t vanilla_index::last_index_file_number(std::int32_t cycle, std::int32_t default_cycle) const
{
    std::int32_t last_number = -1;
    const auto path = fs::path(m_settings.path()) / m_settings.cycle_format().date_from_cycle(cycle);

    boost::system::error_code err;
    fs::directory_iterator begin(path, err);
    fs::directory_iterator end;

    // Find the last index file in the specified cycle directory
    for(const auto & entry : boost::make_iterator_range(begin, end))
    {
        auto name = entry.path().filename().native();
        if(boost::algorithm::starts_with(name, INDEX_FILE_NAME_PREFIX))
        {
            std::int32_t index = -1;
            util::parse_number(name.begin() + static_cast<std::string::difference_type>(INDEX_FILE_NAME_PREFIX.size()) - 1, name.end(), index);
            if(last_number < index)
                last_number = index;
        }
    }
    return last_number == -1 ? default_cycle : last_number;
}

std::int64_t vanilla_index::count_index_entries(const region & region)
{
    std::int64_t indices = 0;
    for(auto offset = 0LL; offset != region.size(); offset += 8)
    {
        if(!*reinterpret_cast<const std::int64_t*>(region.data() + offset))
            break;
        ++indices;
    }
    return indices;
}

std::pair<region_ptr, std::int64_t> vanilla_index::append(std::int32_t cycle, std::int64_t index_value, std::int32_t file_number)
{
    for (int index_count = file_number; index_count < 10000; ++index_count)
    {
        auto region = index_for(cycle, index_count, true);
        if(!region)
            continue;
        auto position = append(*region, index_value);
        if (position >= 0) {
            return {region, position};
        }
    }

    throw std::logic_error((util::streamer()
                            << "Unable to write index" << index_value
                            << "on cycle " << cycle
                            << "(" <<  m_settings.cycle_format().date_from_cycle(cycle) << ")").str());
}

std::int64_t vanilla_index::append(region & region, std::int64_t index_value)
{
    // Initial values in the index are zero.
    // Try to CAS64 against the 'current' tail of the index.
    // If no other thread was 'faster' then we succeed, otherwise we'll try again

    bool eof = false;
    auto position = region.position();
    while (!eof)
    {
        eof = (region.limit() - position) < 8;
        if (!eof && region.cas64(position, 0L, index_value))
        {
            if(BOOST_UNLIKELY(!region.position(position + 8)))
            {
                throw std::logic_error(util::streamer() << "Position out of bounds: " << position + 8);
            }

            return position;
        }
        position += 8;
    }

    return -1;
}

region_ptr vanilla_index::index_for(std::int32_t cycle, std::int32_t file_number, bool append)
{
    std::lock_guard<mutex_t> lk(m_lock);
    auto key = std::make_pair(cycle, file_number);
    auto && creator = [this, append](const key_t & k)
    {
        auto cycle_ = std::get<0>(k);
        auto file_number_ = std::get<1>(k);
        auto && path = make_file(m_settings.path(),
                                 m_settings.cycle_format().date_from_cycle(cycle_),
                                 (util::streamer() << INDEX_FILE_NAME_PREFIX << file_number_).str(),
                                 append);
        return !path.empty()
                ? std::make_shared<region>(path, 1LL << m_index_block_size_bits, file_number_)
                : region_ptr();
    };

    return m_cache.get(key, creator);
}

}
