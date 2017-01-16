
#include "test.hpp" // IWYU pragma: keep

#include <osmium/memory/buffer.hpp>

#include "poly_file_parser.hpp"
#include "osm_file_parser.hpp"

TEST_CASE("Parse poly files") {
    osmium::memory::Buffer buffer{1024};

    SECTION("Empty file") {
        PolyFileParser parser{buffer, "test/extract/empty.poly"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("One line file") {
        PolyFileParser parser{buffer, "test/extract/one-line.poly"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Two line file") {
        PolyFileParser parser{buffer, "test/extract/two-line.poly"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Missing END ring") {
        PolyFileParser parser{buffer, "test/extract/missing-end-ring.poly"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Missing END polygon") {
        PolyFileParser parser{buffer, "test/extract/missing-end-polygon.poly"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("File with one polygon with one outer ring") {
        PolyFileParser parser{buffer, "test/extract/polygon-one-outer.poly"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 1);
        REQUIRE(nr.second == 0);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("File with one polygons with two outer rings") {
        PolyFileParser parser{buffer, "test/extract/polygon-two-outer.poly"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 2);
        REQUIRE(nr.second == 0);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        ++it;
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(20.0, 20.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("File with one polygon with outer and inner rings") {
        PolyFileParser parser{buffer, "test/extract/polygon-outer-inner.poly"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 1);
        REQUIRE(nr.second == 1);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        const auto& inner_ring = *area.inner_rings(*it).begin();
        REQUIRE(inner_ring.front().location() == osmium::Location(11.0, 11.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("Two concatenated files") {
        PolyFileParser parser{buffer, "test/extract/two-polygons.poly"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 2);
        REQUIRE(nr.second == 1);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        const auto& inner_ring = *area.inner_rings(*it).begin();
        REQUIRE(inner_ring.front().location() == osmium::Location(11.0, 11.0));
        ++it;
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(20.0, 20.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("Two concatenated files with empty line in between") {
        PolyFileParser parser{buffer, "test/extract/two-polygons-empty-line.poly"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 2);
        REQUIRE(nr.second == 1);
    }

}

TEST_CASE("Parse OSM files") {
    osmium::memory::Buffer buffer{1024};

    SECTION("Missing OSM file") {
        OSMFileParser parser{buffer, "test/extract/missing.osm.opl"};
        REQUIRE_THROWS({
            parser();
        });
    }

    SECTION("Empty OSM file") {
        OSMFileParser parser{buffer, "test/extract/empty.osm.opl"};
        REQUIRE_THROWS({
            parser();
        });
    }

    SECTION("OSM file without polygon") {
        OSMFileParser parser{buffer, "test/extract/no-polygon.osm.opl"};
        REQUIRE_THROWS({
            parser();
        });
    }

    SECTION("OSM file with simple polygon") {
        OSMFileParser parser{buffer, "test/extract/polygon-way.osm.opl"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 1);
        REQUIRE(nr.second == 0);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("OSM file with two simple polygons") {
        OSMFileParser parser{buffer, "test/extract/polygon-two-ways.osm.opl"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 2);
        REQUIRE(nr.second == 0);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        ++it;
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(20.0, 20.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

    SECTION("OSM file with multipolygon relation") {
        OSMFileParser parser{buffer, "test/extract/multipolygon.osm.opl"};
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        const auto nr = area.num_rings();
        REQUIRE(nr.first == 2);
        REQUIRE(nr.second == 1);

        auto it = area.outer_rings().begin();
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(10.0, 10.0));
        const auto& inner_ring = *area.inner_rings(*it).begin();
        REQUIRE(inner_ring.front().location() == osmium::Location(11.0, 11.0));
        ++it;
        REQUIRE(it != area.outer_rings().end());
        REQUIRE(it->front().location() == osmium::Location(20.0, 20.0));
        ++it;
        REQUIRE(it == area.outer_rings().end());
    }

}

