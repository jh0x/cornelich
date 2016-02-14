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

#include "vanilla_data.h"

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

vanilla_data::vanilla_data(const vanilla_chronicle_settings & settings, std::int32_t data_block_size_bits)
    : m_settings(settings)
    , m_data_block_size_bits(data_block_size_bits)
    , m_cache(settings.data_cache_size(), region_ptr_validator())
{
}

std::int32_t vanilla_data::find_next_data_file_number(std::int32_t cycle, std::int32_t thread_id) const
{
    std::int32_t last_number = -1;
    const auto path = fs::path(m_settings.path()) / m_settings.cycle_format().date_from_cycle(cycle);

    boost::system::error_code err;
    fs::directory_iterator begin(path, err);
    fs::directory_iterator end;

    const auto data_prefix = (util::streamer() << DATA_FILE_NAME_PREFIX << thread_id << '-').str();

    for(const auto & entry : boost::make_iterator_range(begin, end))
    {
        auto name = entry.path().filename().native();
        if(boost::algorithm::starts_with(name, data_prefix))
        {
            std::int32_t index = -1;
            util::parse_number(name.begin() + (std::int32_t)data_prefix.size(), name.end(), index);
            if(last_number < index)
                last_number = index;
        }
    }
    // Move to the next data file
    return last_number + 1;

}

region_ptr vanilla_data::data_for(std::int32_t cycle, std::int32_t thread_id, std::int32_t file_number, bool for_write)
{
    std::lock_guard<mutex_t> lk(m_lock);
    auto key = std::make_tuple(cycle, thread_id, file_number);
    auto && creator = [this, for_write](const key_t & k)
    {
        auto cycle_ = std::get<0>(k);
        auto thread_id_ = std::get<1>(k);
        auto file_number_ = std::get<2>(k);
        auto && path = make_file(m_settings.path(),
                                 m_settings.cycle_format().date_from_cycle(cycle_),
                                 (util::streamer() << DATA_FILE_NAME_PREFIX << thread_id_ << '-' << file_number_).str(),
                                 for_write);
        return !path.empty()
                ? std::make_shared<region>(path, 1LL << m_data_block_size_bits, file_number_)
                : region_ptr();
    };

    return m_cache.get(key, creator);
}


}
