
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

TEST_CASE("Get object types") {
    REQUIRE(get_types("") == osmium::osm_entity_bits::nothing);
    REQUIRE(get_types("n") == osmium::osm_entity_bits::node);
    REQUIRE(get_types("w") == osmium::osm_entity_bits::way);
    REQUIRE(get_types("r") == osmium::osm_entity_bits::relation);
    REQUIRE(get_types("nw") == (osmium::osm_entity_bits::node | osmium::osm_entity_bits::way));
    REQUIRE(get_types("rw") == (osmium::osm_entity_bits::way | osmium::osm_entity_bits::relation));
    REQUIRE_THROWS_AS(get_types("x"), argument_error);
    REQUIRE_THROWS_AS(get_types("nwx"), argument_error);
}

