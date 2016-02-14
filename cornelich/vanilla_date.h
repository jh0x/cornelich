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
#include <string>

#include <array>
#include <memory>

namespace cornelich
{

class cycle_formatter
{
public:
    cycle_formatter(std::int32_t cycle_length) : m_cycle_length(cycle_length) {}
    std::int32_t cycle_from_date(const std::string & date) const;
    std::string date_from_cycle(std::int64_t cycle) const;
    virtual ~cycle_formatter() = default;
private:
    virtual std::int32_t cycle_from_date_impl(const std::string & date) const = 0;
    virtual std::string date_from_cycle_impl(std::int64_t cycle) const = 0;
protected:
    const std::int32_t m_cycle_length;

private:
    static constexpr auto CACHE_SIZE = 32;
    struct node_t
    {
        node_t(std::int64_t cycle, const std::string & data) : m_cycle(cycle), m_data(data) {}
        const std::int64_t m_cycle;
        const std::string m_data;
    };
    mutable std::array<std::shared_ptr<node_t>, CACHE_SIZE> m_cache;
};

struct cycle_formatter_yyyymmdd : public cycle_formatter
{
public:
    cycle_formatter_yyyymmdd(std::int32_t cycle_length) : cycle_formatter(cycle_length) {}
private:
    virtual std::int32_t cycle_from_date_impl(const std::string & date) const override;
    virtual std::string date_from_cycle_impl(std::int64_t cycle) const override;
};

struct cycle_formatter_yyyymmddhh : public cycle_formatter
{
public:
    cycle_formatter_yyyymmddhh(std::int32_t cycle_length) : cycle_formatter(cycle_length) {}
private:
    virtual std::int32_t cycle_from_date_impl(const std::string & date) const override;
    virtual std::string date_from_cycle_impl(std::int64_t cycle) const override;
};

// Not very practical, used in some tests
struct cycle_formatter_yyyymmddhhmm : public cycle_formatter
{
public:
    cycle_formatter_yyyymmddhhmm(std::int32_t cycle_length) : cycle_formatter(cycle_length) {}
private:
    virtual std::int32_t cycle_from_date_impl(const std::string & date) const override;
    virtual std::string date_from_cycle_impl(std::int64_t cycle) const override;
};

}
