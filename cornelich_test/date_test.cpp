#include <cornelich/util/date.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <catch.hpp>

using namespace cornelich::util;

TEST_CASE( "util::format_date: Parses posix_time into std::string", "[util/date]" )
{
    REQUIRE(format_date(boost::posix_time::time_from_string("2002-01-20 23:59:59.000"), "%Y%m%d%H%M%S") == "20020120235959");
    REQUIRE(format_date(boost::posix_time::time_from_string("2002-01-20 23:59:59.000"), "%Y%m%d") == "20020120");
    REQUIRE(format_date(boost::posix_time::time_from_string("2002-01-20 23:59:59.000"), "ABC") == "ABC");
    REQUIRE(format_date(boost::posix_time::time_from_string("2002-01-20 23:59:59.000"), "") == "");
}

TEST_CASE( "util::parse_date: Parses std::string into posix_time", "[util/date]" )
{
    GIVEN("An empty ptime")
    {
        boost::posix_time::ptime result;

        WHEN("We parse a valid date with good format")
        {
            REQUIRE(parse_date("20020120235959", "%Y%m%d%H%M%S", result));
            REQUIRE(boost::posix_time::to_iso_string(result) == "20020120T235959");
        }

        WHEN("We parse another valid date with good format")
        {
            REQUIRE(parse_date("20020120", "%Y%m%d", result));
            REQUIRE(boost::posix_time::to_iso_string(result) == "20020120T000000");
        }

        WHEN("We parse a valid date with bad format")
        {
            REQUIRE(parse_date("20020120235959", "FOOBAR", result));
            REQUIRE(boost::posix_time::to_iso_string(result) == "14000101T000000");
        }

        WHEN("We parse a valid date with empty format")
        {
            REQUIRE(parse_date("20020120235959", "", result));
            REQUIRE(boost::posix_time::to_iso_string(result) == "14000101T000000");
        }

        WHEN("We parse a valid date with mismatched format")
        {
            REQUIRE(parse_date("200201", "%Y%m%d%H%M%S", result));
            REQUIRE(boost::posix_time::to_iso_string(result) == "20020101T000000");
        }

        WHEN("We parse an invalid date")
        {
            REQUIRE(!parse_date("1", "%Y%m%d%H%M%S", result));
        }
    }
}
