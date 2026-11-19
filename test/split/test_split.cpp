#include "test.hpp" // IWYU pragma: keep

#include "command_split.hpp"

#include <osmium/handler.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

#include <cstdio>
#include <string>

namespace {

struct CountHandler : public osmium::handler::Handler {
    size_t nodes = 0;
    size_t ways = 0;
    size_t relations = 0;
    size_t tags = 0;
    size_t valid_locations = 0;
    size_t way_nodes = 0;
    size_t rel_members = 0;

    void node(const osmium::Node& n) {
        ++nodes;
        tags += n.tags().size();
        if (n.location().valid()) {
            ++valid_locations;
        }
    }

    void way(const osmium::Way& w) {
        ++ways;
        tags += w.tags().size();
        way_nodes += w.nodes().size();
    }

    void relation(const osmium::Relation& r) {
        ++relations;
        tags += r.tags().size();
        rel_members += r.members().size();
    }
};

CountHandler inspect_file(const std::string& filename) {
    osmium::io::Reader reader{filename};
    CountHandler handler;
    osmium::apply(reader, handler);
    reader.close();
    return handler;
}

} // anonymous namespace

TEST_CASE("split") {
    const CommandFactory factory;
    const std::string input = "../test/split/input.osm";

    SECTION("3 files mode (by type)") {
        const std::string prefix = "test_out_split3";
        CommandSplit cmd{factory};
        REQUIRE(cmd.setup({"-O", "-f", "osm", "-p", prefix, input}));
        REQUIRE(cmd.run());

        auto n = inspect_file(prefix + "_nodes.osm");
        REQUIRE(n.nodes == 2);
        REQUIRE(n.ways == 0);
        REQUIRE(n.relations == 0);
        REQUIRE(n.tags == 1);
        REQUIRE(n.valid_locations == 2);

        auto w = inspect_file(prefix + "_ways.osm");
        REQUIRE(w.ways == 1);
        REQUIRE(w.way_nodes == 2);

        auto r = inspect_file(prefix + "_relations.osm");
        REQUIRE(r.relations == 1);
        REQUIRE(r.rel_members == 1);

        std::remove((prefix + "_nodes.osm").c_str());
        std::remove((prefix + "_ways.osm").c_str());
        std::remove((prefix + "_relations.osm").c_str());
    }

    SECTION("6 files mode (split details)") {
        const std::string prefix = "test_out_split6";
        CommandSplit cmd{factory};
        REQUIRE(cmd.setup({"-O", "-d", "-f", "osm", "-p", prefix, input}));
        REQUIRE(cmd.run());

        auto n_tags = inspect_file(prefix + "_nodes_tags.osm");
        REQUIRE(n_tags.nodes == 2);
        REQUIRE(n_tags.tags == 1);
        REQUIRE(n_tags.valid_locations == 0);

        auto n_geom = inspect_file(prefix + "_nodes_geom.osm");
        REQUIRE(n_geom.nodes == 2);
        REQUIRE(n_geom.tags == 0);
        REQUIRE(n_geom.valid_locations == 2);

        auto w_tags = inspect_file(prefix + "_ways_tags.osm");
        REQUIRE(w_tags.ways == 1);
        REQUIRE(w_tags.tags == 1);
        REQUIRE(w_tags.way_nodes == 0);

        auto w_geom = inspect_file(prefix + "_ways_geom.osm");
        REQUIRE(w_geom.ways == 1);
        REQUIRE(w_geom.tags == 0);
        REQUIRE(w_geom.way_nodes == 2);

        auto r_tags = inspect_file(prefix + "_relations_tags.osm");
        REQUIRE(r_tags.relations == 1);
        REQUIRE(r_tags.tags == 1);
        REQUIRE(r_tags.rel_members == 0);

        auto r_geom = inspect_file(prefix + "_relations_geom.osm");
        REQUIRE(r_geom.relations == 1);
        REQUIRE(r_geom.tags == 0);
        REQUIRE(r_geom.rel_members == 1);

        std::remove((prefix + "_nodes_tags.osm").c_str());
        std::remove((prefix + "_nodes_geom.osm").c_str());
        std::remove((prefix + "_ways_tags.osm").c_str());
        std::remove((prefix + "_ways_geom.osm").c_str());
        std::remove((prefix + "_relations_tags.osm").c_str());
        std::remove((prefix + "_relations_geom.osm").c_str());
    }

    SECTION("6 files mode with --skip-empty-tags") {
        const std::string prefix = "test_out_split6_skip";
        CommandSplit cmd{factory};
        REQUIRE(cmd.setup({"-O", "-d", "-e", "-f", "osm", "-p", prefix, input}));
        REQUIRE(cmd.run());

        auto n_tags = inspect_file(prefix + "_nodes_tags.osm");
        REQUIRE(n_tags.nodes == 1);
        REQUIRE(n_tags.tags == 1);

        auto n_geom = inspect_file(prefix + "_nodes_geom.osm");
        REQUIRE(n_geom.nodes == 2);

        std::remove((prefix + "_nodes_tags.osm").c_str());
        std::remove((prefix + "_nodes_geom.osm").c_str());
        std::remove((prefix + "_ways_tags.osm").c_str());
        std::remove((prefix + "_ways_geom.osm").c_str());
        std::remove((prefix + "_relations_tags.osm").c_str());
        std::remove((prefix + "_relations_geom.osm").c_str());
    }
}
