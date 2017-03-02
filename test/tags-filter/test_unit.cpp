
#include <sstream>

#include "test.hpp" // IWYU pragma: keep

#include "command_tags_filter.hpp"
#include "util.hpp"

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

osmium::StringMatcher get_matcher(std::string string);

void test_matcher(const char* string, const char* print_out) {
    std::stringstream ss;
    ss << get_matcher(string);
    REQUIRE(ss.str() == print_out);
}

TEST_CASE("get_matcher") {
    test_matcher("foo", "equal[foo]");
    test_matcher("", "equal[]");
    test_matcher("foo*", "prefix[foo]");
    test_matcher(" foo* ", "prefix[foo]");
    test_matcher("*foo", "substring[foo]");
    test_matcher("*foo*", "substring[foo]");
    test_matcher(" *foo* ", "substring[foo]");
    test_matcher("*", "always_true");
    test_matcher(" * ", "always_true");
    test_matcher("f*oo", "equal[f*oo]");
    test_matcher("foo,bar", "list[[foo][bar]]");
    test_matcher("foo,bar*,baz", "list[[foo][bar*][baz]]");
    test_matcher("*foo,bar", "substring[foo,bar]");
    test_matcher("foo ", "equal[foo]");
    test_matcher(" foo", "equal[foo]");
    test_matcher(" foo ", "equal[foo]");
    test_matcher("foo    ", "equal[foo]");
    test_matcher("    foo", "equal[foo]");
    test_matcher("  foo ", "equal[foo]");
    test_matcher("foo, bar, baz", "list[[foo][bar][baz]]");
    test_matcher("  foo, bar   ,baz   ", "list[[foo][bar][baz]]");
}

