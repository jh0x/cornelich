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

#include "vanilla_date.h"

#include "util/parse.h"

#include <boost/spirit/include/karma.hpp>

#include <contrib/date/date.h>
#include <chrono>
#include <functional>
#include <iterator>

namespace cornelich
{


std::int32_t cycle_formatter::cycle_from_date(const std::string & date) const
{
    return cycle_from_date_impl(date);
}

std::string cycle_formatter::date_from_cycle(std::int64_t cycle) const
{
    // TODO: Is it worth it...?
    auto hash = std::hash<std::int64_t>()(cycle) % (CACHE_SIZE - 1);
    auto cached = std::atomic_load(&m_cache[hash]);
    if(cached && cached->m_cycle == cycle)
    {
        // Cache hit - return the string
        return cached->m_data;
    }
    // Cache miss -> evaluate and cache the string
    auto node = std::make_shared<node_t>(cycle, date_from_cycle_impl(cycle));
    std::atomic_store(&m_cache[hash], node);
    return node->m_data;
}


using namespace date;
using namespace std::chrono;
using namespace boost::spirit;

static const day_point unix_epoch = day(1)/jan/1970;

bool parse_date(const std::string & date, day_point & dp)
{
    int y = 0, m = 0, d = 0;
    if(BOOST_UNLIKELY(
                !util::parse_number(date.begin(), date.begin() + 4, y) ||
                !util::parse_number(date.begin() + 4, date.begin() + 6, m) ||
                !util::parse_number(date.begin() + 6, date.begin() + 8, d)))
    {
        return false;
    }
    auto ymd = year(y)/m/d;
    dp = ymd;
    return ymd.ok();
}

std::int64_t get_epoch_seconds(const system_clock::time_point & ts)
{
    day_point today = time_point_cast<days>(ts);
    system_clock::time_point this_morning = today;
    days days_since_epoch = today - unix_epoch;
    auto s = ts - this_morning;
    std::int64_t unix_timestamp = (days_since_epoch + s) / seconds(1);
    return unix_timestamp;
}

std::int32_t cycle_formatter_yyyymmdd::cycle_from_date_impl(const std::string & date) const
{
    thread_local day_point dp;
    if(BOOST_UNLIKELY(date.length() != 8))
        return -1;
    if(BOOST_UNLIKELY(!parse_date(date, dp)))
        return -1;
    system_clock::time_point ts = dp;
    auto unix_timestamp = get_epoch_seconds(ts);

    return unix_timestamp * 1000 / m_cycle_length;
}

std::string cycle_formatter_yyyymmdd::date_from_cycle_impl(std::int64_t cycle) const
{
    const auto s = static_cast<std::time_t>(cycle * m_cycle_length / 1000);
    auto ts = system_clock::time_point(seconds(s));
    auto daypoint = floor<days>(ts);
    auto ymd = year_month_day(daypoint);

    using namespace boost::spirit;

    std::string result;
    std::back_insert_iterator<std::string> sink(result);
    karma::generate(sink,
                    karma::right_align(4, '0')[karma::int_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_],
            int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
    return result;
}

std::int32_t cycle_formatter_yyyymmddhh::cycle_from_date_impl(const std::string & date) const
{
    thread_local day_point dp;
    int h = 0;
    if(BOOST_UNLIKELY(date.length() != 10))
        return -1;
    if(BOOST_UNLIKELY(
                !parse_date(date, dp) ||
                !util::parse_number(date.begin() + 8, date.begin() + 10, h) || h > 24))
        return -1;
    system_clock::time_point ts = dp + hours(h);
    auto unix_timestamp = get_epoch_seconds(ts);

    return unix_timestamp * 1000 / m_cycle_length;
}

std::string cycle_formatter_yyyymmddhh::date_from_cycle_impl(std::int64_t cycle) const
{
    const auto s = static_cast<std::time_t>(cycle * m_cycle_length / 1000);
    auto ts = system_clock::time_point(seconds(s));
    auto daypoint = floor<days>(ts);
    auto ymd = year_month_day(daypoint);
    auto tod = make_time(ts - daypoint);

    using namespace boost::spirit;

    std::string result;
    std::back_insert_iterator<std::string> sink(result);
    karma::generate(sink,
                    karma::right_align(4, '0')[karma::int_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_],
            int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()), tod.hours().count());
    return result;
}

std::int32_t cycle_formatter_yyyymmddhhmm::cycle_from_date_impl(const std::string & date) const
{
    thread_local day_point dp;
    int h = 0, min = 0;
    if(BOOST_UNLIKELY(date.length() != 12))
        return -1;
    if(BOOST_UNLIKELY(
                !parse_date(date, dp) ||
                !util::parse_number(date.begin() + 8, date.begin() + 10, h) || h > 24 ||
                !util::parse_number(date.begin() + 10, date.begin() + 12, min) || min > 60
                ))
        return -1;

    system_clock::time_point ts = dp + hours(h) + minutes(min);
    auto unix_timestamp = get_epoch_seconds(ts);

    return unix_timestamp * 1000 / m_cycle_length;
}

std::string cycle_formatter_yyyymmddhhmm::date_from_cycle_impl(std::int64_t cycle) const
{
    const auto s = static_cast<std::time_t>(cycle * m_cycle_length / 1000);
    auto ts = system_clock::time_point(seconds(s));
    auto daypoint = floor<days>(ts);
    auto ymd = year_month_day(daypoint);
    auto tod = make_time(ts - daypoint);

    using namespace boost::spirit;

    std::string result;
    std::back_insert_iterator<std::string> sink(result);
    karma::generate(sink,
                    karma::right_align(4, '0')[karma::int_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_] << karma::right_align(2, '0')[karma::uint_],
            int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()), tod.hours().count(), tod.minutes().count());
    return result;
}

}
