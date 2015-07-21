
#include "test.hpp"

#include "command_time_filter.hpp"

TEST_CASE("time-filter") {

    CommandTimeFilter cmd;

    SECTION("no parameters") {
        REQUIRE_THROWS_AS(cmd.setup({}), argument_error);
    }

}

