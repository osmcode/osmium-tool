
#include "test.hpp" // IWYU pragma: keep

#include "command_time_filter.hpp"

TEST_CASE("time-filter") {
    CommandFactory factory;
    CommandTimeFilter cmd{factory};

    SECTION("no parameters") {
        REQUIRE_THROWS_AS(cmd.setup({}), argument_error);
    }

}

