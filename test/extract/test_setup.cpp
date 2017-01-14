
#include "test.hpp" // IWYU pragma: keep

#include <osmium/memory/buffer.hpp>

#include "poly_file_parser.hpp"

TEST_CASE("Parse poly files") {
    osmium::memory::Buffer buffer{1024};

    SECTION("Empty file") {
        PolyFileParser parser{buffer, "filename", ""};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("One line file") {
        PolyFileParser parser{buffer, "filename", "foo"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Two line file") {
        PolyFileParser parser{buffer, "filename", "foo\nbar"};
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Missing END ring") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
        };
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("Missing END polygon") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
        };
        REQUIRE_THROWS_AS({
            parser();
        }, poly_error);
    }

    SECTION("File with one outer polygon") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
            "END\n"
        };
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        REQUIRE(area.num_rings() == std::make_pair(1ul, 0ul));
    }

    SECTION("File with two outer polygons") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
            "2\n"
            "20.0 20.0\n"
            "29.0 20.0\n"
            "29.0 29.0\n"
            "20.0 29.0\n"
            "20.0 20.0\n"
            "END\n"
            "END\n"
        };
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        REQUIRE(area.num_rings() == std::make_pair(2ul, 0ul));
    }

    SECTION("File with outer and inner polygons") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
            "!1s\n"
            "11.0 11.0\n"
            "18.0 11.0\n"
            "18.0 18.0\n"
            "11.0 18.0\n"
            "11.0 11.0\n"
            "END\n"
            "END\n"
        };
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        REQUIRE(area.num_rings() == std::make_pair(1ul, 1ul));
    }

    SECTION("Two concatenated files") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
            "!1s\n"
            "11.0 11.0\n"
            "18.0 11.0\n"
            "18.0 18.0\n"
            "11.0 18.0\n"
            "11.0 11.0\n"
            "END\n"
            "END\n"
            "bar\n"
            "2\n"
            "20.0 20.0\n"
            "29.0 20.0\n"
            "29.0 29.0\n"
            "20.0 29.0\n"
            "20.0 20.0\n"
            "END\n"
            "END\n"
        };
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        REQUIRE(area.num_rings() == std::make_pair(2ul, 1ul));
    }

    SECTION("Two concatenated files with empty line in between") {
        PolyFileParser parser{buffer, "filename",
            "foo\n"
            "1\n"
            "10.0 10.0\n"
            "19.0 10.0\n"
            "19.0 19.0\n"
            "10.0 19.0\n"
            "10.0 10.0\n"
            "END\n"
            "!1s\n"
            "11.0 11.0\n"
            "18.0 11.0\n"
            "18.0 18.0\n"
            "11.0 18.0\n"
            "11.0 11.0\n"
            "END\n"
            "END\n"
            "\n"
            "bar\n"
            "2\n"
            "20.0 20.0\n"
            "29.0 20.0\n"
            "29.0 29.0\n"
            "20.0 29.0\n"
            "20.0 20.0\n"
            "END\n"
            "END\n"
        };
        REQUIRE(parser() == 0);
        const osmium::Area& area = buffer.get<osmium::Area>(0);
        REQUIRE(area.num_rings() == std::make_pair(2ul, 1ul));
    }

}


