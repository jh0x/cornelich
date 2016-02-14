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

#include <boost/spirit/include/qi_parse_auto.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

namespace cornelich
{
namespace util
{

/** Extrant number from a range of character */
template<typename It>
bool parse_number(It begin, It end, int & target)
{
    return boost::spirit::qi::parse(begin, end, boost::spirit::qi::int_, target);
}

}
}
