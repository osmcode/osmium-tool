
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


static void test_filter(const std::string& expression, osmium::osm_entity_bits::type entities, const char* filter) {
    const auto p = get_filter_expression(expression);
    REQUIRE(p.first == entities);
    REQUIRE(p.second == filter);
}

TEST_CASE("Get tags filter expression") {
    test_filter("highway", osmium::osm_entity_bits::nwr, "highway");
    test_filter("/highway", osmium::osm_entity_bits::nwr, "highway");
    test_filter("n/highway", osmium::osm_entity_bits::node, "highway");
    test_filter("w/highway", osmium::osm_entity_bits::way, "highway");
    test_filter("r/highway", osmium::osm_entity_bits::relation, "highway");
    test_filter("nw/highway", osmium::osm_entity_bits::node | osmium::osm_entity_bits::way, "highway");
    test_filter("n/highway/foo", osmium::osm_entity_bits::node, "highway/foo");
    REQUIRE_THROWS_AS(get_filter_expression("highway/foo"), argument_error);
}

