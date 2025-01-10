
#include "test.hpp" // IWYU pragma: keep

#include "exception.hpp"
#include "geojson_file_parser.hpp"
#include "geometry_util.hpp"
#include "osm_file_parser.hpp"
#include "poly_file_parser.hpp"

#include <osmium/memory/buffer.hpp>

#include <vector>

TEST_CASE("Parse poly files") {
    osmium::memory::Buffer buffer{1024};

    SECTION("Missing file") {
        REQUIRE_THROWS(PolyFileParser(buffer, "test/extract/missing.poly")());
    }

    SECTION("Empty file") {
        PolyFileParser parser{buffer, "test/extract/empty.poly"};
        REQUIRE_THROWS_AS(parser(), poly_error);
    }

    SECTION("One line file") {
        PolyFileParser parser{buffer, "test/extract/one-line.poly"};
        REQUIRE_THROWS_AS(parser(), poly_error);
    }

    SECTION("Two line file") {
        PolyFileParser parser{buffer, "test/extract/two-line.poly"};
        REQUIRE_THROWS_AS(parser(), poly_error);
    }

    SECTION("Missing END ring") {
        PolyFileParser parser{buffer, "test/extract/missing-end-ring.poly"};
        REQUIRE_THROWS_AS(parser(), poly_error);
    }

    SECTION("Missing END polygon") {
        PolyFileParser parser{buffer, "test/extract/missing-end-polygon.poly"};
        REQUIRE_THROWS_AS(parser(), poly_error);
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
        REQUIRE_THROWS(OSMFileParser(buffer, "test/extract/missing.osm.opl")());
    }

    SECTION("Empty OSM file") {
        OSMFileParser parser{buffer, "test/extract/empty.osm.opl"};
        REQUIRE_THROWS(parser());
    }

    SECTION("OSM file without polygon") {
        OSMFileParser parser{buffer, "test/extract/no-polygon.osm.opl"};
        REQUIRE_THROWS(parser());
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

    SECTION("File with CRLF") {
        PolyFileParser parser{buffer, "test/extract/polygon-crlf.poly"};
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

}

TEST_CASE("Parse GeoJSON files") {
    osmium::memory::Buffer buffer{1024};

    SECTION("Missing GeoJSON file") {
        REQUIRE_THROWS(GeoJSONFileParser(buffer, "test/extract/missing.geojson")());
    }

    SECTION("Empty GeoJSON file") {
        GeoJSONFileParser parser{buffer, "test/extract/empty.geojson"};
        REQUIRE_THROWS(parser());
    }

    SECTION("Invalid GeoJSON file") {
        GeoJSONFileParser parser{buffer, "test/extract/invalid.geojson"};
        REQUIRE_THROWS_AS(parser(), geojson_error);
    }

    SECTION("Invalid GeoJSON file: Root not an object") {
        GeoJSONFileParser parser{buffer, "test/extract/invalid-root.geojson"};
        REQUIRE_THROWS_AS(parser(), geojson_error);
    }

    SECTION("Invalid GeoJSON file: Empty root object") {
        GeoJSONFileParser parser{buffer, "test/extract/empty-root.geojson"};
        REQUIRE_THROWS_AS(parser(), geojson_error);
    }

    SECTION("Invalid GeoJSON file: Wrong geometry type") {
        GeoJSONFileParser parser{buffer, "test/extract/wrong-geometry-type.geojson"};
        REQUIRE_THROWS_AS(parser(), geojson_error);
    }

}

TEST_CASE("Ring orientation (clockwise)") {
    using oc = osmium::geom::Coordinates;
    std::vector<oc> c = {oc{0, 0}, oc{0, 1}, oc{1, 1}, oc{1, 0}, oc{0, 0}};
    REQUIRE(calculate_double_area(c) == Approx(-2.0));
    REQUIRE_FALSE(is_ccw(c));
}

TEST_CASE("Ring orientation (counter-clockwise)") {
    using oc = osmium::geom::Coordinates;
    std::vector<oc> c = {oc{0, 0}, oc{1, 0}, oc{1, 1}, oc{0, 1}, oc{0, 0}};
    REQUIRE(calculate_double_area(c) == Approx(2.0));
    REQUIRE(is_ccw(c));
}

