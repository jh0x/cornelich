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

#include <cornelich/vanilla_date.h>

#include <limits>

#include <catch.hpp>

TEST_CASE( "Converting between cycles and date", "[vanilla_date]" )
{
    GIVEN("A YYYYMMDD formatter")
    {
        auto fmt = std::make_shared<cornelich::cycle_formatter_yyyymmdd>(24 * 60 * 60 * 1000);

        SECTION("Cycle into date")
        {
            REQUIRE(fmt->date_from_cycle(0) == "19700101");
            REQUIRE(fmt->date_from_cycle(1) == "19700102");
            REQUIRE(fmt->date_from_cycle(2) == "19700103");
            REQUIRE(fmt->date_from_cycle(31) == "19700201");
            REQUIRE(fmt->date_from_cycle(10000) == "19970519");
            REQUIRE(fmt->date_from_cycle(10957) == "20000101");
            REQUIRE(fmt->date_from_cycle(16801) == "20160101");

            REQUIRE(fmt->date_from_cycle(-1) == "19691231");
        }

        SECTION("Date into cycle")
        {
            REQUIRE(fmt->cycle_from_date("19700101") == 0);
            REQUIRE(fmt->cycle_from_date("19700102") == 1);
            REQUIRE(fmt->cycle_from_date("19700103") == 2);
            REQUIRE(fmt->cycle_from_date("19700201") == 31);
            REQUIRE(fmt->cycle_from_date("19970519") == 10000);
            REQUIRE(fmt->cycle_from_date("20000101") == 10957);
            REQUIRE(fmt->cycle_from_date("20160101") == 16801);

            REQUIRE(fmt->cycle_from_date("19691231") == -1);

            REQUIRE(fmt->cycle_from_date("") == -1);
            REQUIRE(fmt->cycle_from_date("YYYYMMDD") == -1);
            REQUIRE(fmt->cycle_from_date("1999MMDD") == -1);
            REQUIRE(fmt->cycle_from_date("199912DD") == -1);
            //REQUIRE(fmt->cycle_from_date("1999121D") == -1); // TODO

            REQUIRE(fmt->cycle_from_date("19990001") == -1);
            REQUIRE(fmt->cycle_from_date("19991301") == -1);
            REQUIRE(fmt->cycle_from_date("19990100") == -1);
            REQUIRE(fmt->cycle_from_date("19990132") == -1);
        }
    }

    GIVEN("A YYYYMMDDHH formatter")
    {
        auto fmt = std::make_shared<cornelich::cycle_formatter_yyyymmddhh>(60 * 60 * 1000);

        SECTION("Cycle into date")
        {
            REQUIRE(fmt->date_from_cycle(0) == "1970010100");
            REQUIRE(fmt->date_from_cycle(1) == "1970010101");
            REQUIRE(fmt->date_from_cycle(1*24) == "1970010200");
            REQUIRE(fmt->date_from_cycle(2*24+12) == "1970010312");
            REQUIRE(fmt->date_from_cycle(31*24+11) == "1970020111");
            REQUIRE(fmt->date_from_cycle(10000*24+3) == "1997051903");
            REQUIRE(fmt->date_from_cycle(10957*24+20) == "2000010120");
            REQUIRE(fmt->date_from_cycle(16801*24+19) == "2016010119");

            REQUIRE(fmt->date_from_cycle(-1) == "1969123123");
        }

        SECTION("Date into cycle")
        {
            REQUIRE(fmt->cycle_from_date("1970010100") == 0);
            REQUIRE(fmt->cycle_from_date("1970010101") == 1);
            REQUIRE(fmt->cycle_from_date("1970010200") == 1*24);
            REQUIRE(fmt->cycle_from_date("1970010312") == 2*24+12);
            REQUIRE(fmt->cycle_from_date("1970020111") == 31*24+11);
            REQUIRE(fmt->cycle_from_date("1997051903") == 10000*24+3);
            REQUIRE(fmt->cycle_from_date("2000010120") == 10957*24+20);
            REQUIRE(fmt->cycle_from_date("2016010119") == 16801*24+19);

            REQUIRE(fmt->cycle_from_date("1969123123") == -1);

            REQUIRE(fmt->cycle_from_date("") == -1);
            REQUIRE(fmt->cycle_from_date("YYYYMMDDHH") == -1);
            REQUIRE(fmt->cycle_from_date("1999MMDDHH") == -1);
            REQUIRE(fmt->cycle_from_date("199912DDHH") == -1);
            REQUIRE(fmt->cycle_from_date("19991211HH") == -1);
            //REQUIRE(fmt->cycle_from_date("199912112H") == -1); // TODO

            REQUIRE(fmt->cycle_from_date("1999000110") == -1);
            REQUIRE(fmt->cycle_from_date("1999130110") == -1);
            REQUIRE(fmt->cycle_from_date("1999010010") == -1);
            REQUIRE(fmt->cycle_from_date("1999013210") == -1);
            REQUIRE(fmt->cycle_from_date("1999010199") == -1);
        }
    }

    GIVEN("A YYYYMMDDHHMM formatter")
    {
        auto fmt = std::make_shared<cornelich::cycle_formatter_yyyymmddhhmm>(60 * 1000);

        SECTION("Cycle into date")
        {
            REQUIRE(fmt->date_from_cycle(0) == "197001010000");
            REQUIRE(fmt->date_from_cycle(1) == "197001010001");
            REQUIRE(fmt->date_from_cycle(1*24*60) == "197001020000");
            REQUIRE(fmt->date_from_cycle((2*24+12)*60+1) == "197001031201");
            REQUIRE(fmt->date_from_cycle((31*24+11)*60+10) == "197002011110");
            REQUIRE(fmt->date_from_cycle((10000*24+3)*60+40) == "199705190340");
            REQUIRE(fmt->date_from_cycle((10957*24+20)*60+50) == "200001012050");
            REQUIRE(fmt->date_from_cycle((16801*24+19)*60+55) == "201601011955");

            REQUIRE(fmt->date_from_cycle(-1) == "196912312359");
        }

        SECTION("Date into cycle")
        {
            REQUIRE(fmt->cycle_from_date("197001010000") == 0);
            REQUIRE(fmt->cycle_from_date("197001010001") == 1);
            REQUIRE(fmt->cycle_from_date("197001020000") == 1*24*60);
            REQUIRE(fmt->cycle_from_date("197001031201") == (2*24+12)*60+1);
            REQUIRE(fmt->cycle_from_date("197002011110") == (31*24+11)*60+10);
            REQUIRE(fmt->cycle_from_date("199705190340") == (10000*24+3)*60+40);
            REQUIRE(fmt->cycle_from_date("200001012050") == (10957*24+20)*60+50);
            REQUIRE(fmt->cycle_from_date("201601011955") == (16801*24+19)*60+55);

            REQUIRE(fmt->cycle_from_date("196912312359") == -1);

            REQUIRE(fmt->cycle_from_date("") == -1);
            REQUIRE(fmt->cycle_from_date("YYYYMMDDHH") == -1);
            REQUIRE(fmt->cycle_from_date("1999MMDDHH") == -1);
            REQUIRE(fmt->cycle_from_date("199912DDHH") == -1);
            REQUIRE(fmt->cycle_from_date("19991211HH") == -1);
            //REQUIRE(fmt->cycle_from_date("199912112H") == -1); // TODO

            REQUIRE(fmt->cycle_from_date("1999000110") == -1);
            REQUIRE(fmt->cycle_from_date("1999130110") == -1);
            REQUIRE(fmt->cycle_from_date("1999010010") == -1);
            REQUIRE(fmt->cycle_from_date("1999013210") == -1);
            REQUIRE(fmt->cycle_from_date("1999010199") == -1);
        }
    }
}
