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

#include <atomic>
#include <cstdint>

#include <boost/config.hpp>

namespace cornelich
{
namespace util
{

inline void wait(std::uint32_t delay)
{
    for(auto i=0u; i!=delay; ++i)
    {
        __asm__ __volatile__("pause;");
    }
}

template<std::size_t N>
class default_backoff
{
public:
    default_backoff() : m_count(1) {}
    
    void operator()()
    {
        if(BOOST_LIKELY((m_count & (1<<N)) == 0))
        {
            wait(m_count);
            m_count<<=1;
        }
        else
        {
            sched_yield();
        }
    }

private:
    std::uint_fast32_t m_count;
};

/** spin-lock to be used instead of mutexes when really needed */
class spin_lock
{
public:
    spin_lock() noexcept {}
    spin_lock(const spin_lock &) = delete;
    spin_lock & operator=(const spin_lock &) = delete;

    void lock() noexcept
    {
        lock(default_backoff<5>());
    }

    template<typename backoff_t>
    void lock(backoff_t backoff) noexcept
    {
        while(BOOST_UNLIKELY(m_flag.test_and_set(std::memory_order_acquire)))
            backoff();
    }

    void unlock() noexcept
    {
        m_flag.clear(std::memory_order_release);
    }
private:
    mutable std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};


}
}
