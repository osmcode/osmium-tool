
#include "test.hpp" // IWYU pragma: keep

#include "command_cat.hpp"

TEST_CASE("cat") {
    CommandFactory factory;
    CommandCat cmd{factory};

    SECTION("no parameters") {
        REQUIRE_THROWS_AS(cmd.setup({}), const argument_error&);
    }

    SECTION("stdin osm to stdout pbf") {
        cmd.setup({"-F", "osm", "-f", "pbf"});

        auto ifs = cmd.input_files();
        REQUIRE(ifs.size() == 1);
        REQUIRE(ifs.front().format() == osmium::io::file_format::xml);
        REQUIRE(ifs.front().compression() == osmium::io::file_compression::none);
        REQUIRE(ifs.front().filename() == "");

        auto of = cmd.output_file();
        REQUIRE(of.format() == osmium::io::file_format::pbf);
        REQUIRE(of.filename() == "");

        REQUIRE(cmd.output_overwrite() == osmium::io::overwrite::no);
        REQUIRE(cmd.osm_entity_bits() == osmium::osm_entity_bits::all);
    }

    SECTION("unknown object-type 1") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-o", "out.osm", "-t", "foo"}), const argument_error&);
    }

    SECTION("unknown object-type 2") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-o", "out.osm", "--object-type", "foo"}), const argument_error&);
    }

    SECTION("reading to files") {
        cmd.setup({"-o", "output", "-O", "-f", "opl", "-t", "way", "a.osm.bz2", "b.pbf"});

        auto ifs = cmd.input_files();
        REQUIRE(ifs.size() == 2);
        REQUIRE(ifs[0].format() == osmium::io::file_format::xml);
        REQUIRE(ifs[0].compression() == osmium::io::file_compression::bzip2);
        REQUIRE(ifs[0].filename() == "a.osm.bz2");
        REQUIRE(ifs[1].format() == osmium::io::file_format::pbf);
        REQUIRE(ifs[1].compression() == osmium::io::file_compression::none);
        REQUIRE(ifs[1].filename() == "b.pbf");

        auto of = cmd.output_file();
        REQUIRE(of.format() == osmium::io::file_format::opl);
        REQUIRE(of.filename() == "output");

        REQUIRE(cmd.output_overwrite() == osmium::io::overwrite::allow);
        REQUIRE(cmd.osm_entity_bits() == osmium::osm_entity_bits::way);
    }

    SECTION("unknown input format suffix") {
        REQUIRE_THROWS_AS(cmd.setup({"in.foo"}), const argument_error&);
    }

    SECTION("unknown input format option") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-F", "foo"}), const argument_error&);
    }

    SECTION("unknown output format suffix") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-o", "foo.bar"}), const std::runtime_error&);
    }

    SECTION("unknown output suffix and unknown format option") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-o", "foo.foo", "-f", "bar"}), const std::runtime_error&);
    }

    SECTION("unknown output format option") {
        REQUIRE_THROWS_AS(cmd.setup({"in.osm", "-o", "foo.pbf", "-f", "bar"}), const std::runtime_error&);
    }

}

