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


#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>
#include <cornelich/formatters.h>

#include <iostream>

namespace chr = cornelich;

int main()
{
    chr::vanilla_chronicle_settings settings("/tmp/ëxample");
    chr::vanilla_chronicle chronicle(settings);

    auto tailer = chronicle.create_tailer();
    while(true)
    {
        if(!tailer.next_index())
            continue;
        std::cout << "int:  " << tailer.read<int>() << '\n';
        std::cout << "text: " << tailer.read(chr::readers::chars()) << '\n';
    }

}
