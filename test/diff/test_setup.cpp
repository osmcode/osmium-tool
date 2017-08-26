
#include "test.hpp"

#include "command_diff.hpp"

TEST_CASE("diff") {

    CommandDiff cmd;

    SECTION("no arguments - need exactly two arguments") {
        REQUIRE_THROWS_AS(cmd.setup({}), const argument_error&);
    }

    SECTION("one argument - need exactly two arguments") {
        REQUIRE_THROWS_AS(cmd.setup({"x"}), const argument_error&);
    }

    SECTION("three arguments - need exactly two arguments") {
        REQUIRE_THROWS_AS(cmd.setup({"x", "y", "z"}), const argument_error&);
    }

    SECTION("quiet with output parameter -o") {
        REQUIRE_THROWS_AS(cmd.setup({"-q", "-o", "file"}), const argument_error&);
    }

    SECTION("quiet with output parameter -f") {
        REQUIRE_THROWS_AS(cmd.setup({"-q", "-f", "opl"}), const argument_error&);
    }

    SECTION("parameter --fsync") {
        REQUIRE_THROWS_AS(cmd.setup({"--fsync"}), const boost::program_options::unknown_option&);
    }

    SECTION("quiet with output parameter -O") {
        REQUIRE_THROWS_AS(cmd.setup({"-q", "-O"}), const argument_error&);
    }

}

