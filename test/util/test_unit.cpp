
#include "test.hpp" // IWYU pragma: keep

#include "util.hpp"

TEST_CASE("Get suffix from filename") {
    REQUIRE(get_filename_suffix("foo.bar") == "bar");
}

TEST_CASE("Get suffixes from filename") {
    REQUIRE(get_filename_suffix("foo.bar.baz") == "bar.baz");
}

TEST_CASE("Get suffixes from file path") {
    REQUIRE(get_filename_suffix("/usr/local/foo.bar.baz") == "bar.baz");
}

TEST_CASE("Get suffixes from relative path") {
    REQUIRE(get_filename_suffix("../somewhere/foo.bar.baz") == "bar.baz");
}

TEST_CASE("Get suffixes from path with dots in the middle") {
    REQUIRE(get_filename_suffix("anything/../somewhere/foo.bar.baz") == "bar.baz");
}

