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

#include "thread.h"

#include "math_util.h"

#include <linux/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <fstream>
#include <stdexcept>
#include <string>

namespace cornelich
{
namespace util
{

std::int32_t get_native_thread_id()
{
    thread_local std::int32_t tid = (pid_t) syscall(__NR_gettid);
    return tid;
}

std::int32_t thread_id_bits()
{
    std::ifstream is("/proc/sys/kernel/threads-max");
    if(!is)
        throw std::runtime_error("Cannot access /proc/sys/kernel/threads-max");
    unsigned int v;
    is >> v;
    return log2_bits(v - 1) + 1;
}

}
}
