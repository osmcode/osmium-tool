#include "test.hpp" // IWYU pragma: keep

#include "command_sort.hpp"

#include <osmium/memory/buffer.hpp>
#include <osmium/builder/osm_object_builder.hpp>

TEST_CASE("sort locations on ways warning") {
    const CommandFactory factory;

    auto format = GENERATE("xml", "pbf", "opl");
    
    SECTION("detection logic with way containing locations - " + std::string(format)) {
        CommandSort cmd{factory};
        std::vector<std::string> args = {"test/sort/input-simple1.osm", "-f", format};
        cmd.setup(args);
        
        REQUIRE(cmd.found_locations_on_ways() == false);
        REQUIRE(cmd.warned_locations_on_ways() == false);
        
        osmium::memory::Buffer buffer{1024};
        osmium::builder::WayBuilder builder{buffer};
        builder.set_id(123);
        builder.set_version(1);
        builder.set_changeset(1);
        builder.set_timestamp(osmium::Timestamp{"2015-01-01T01:00:00Z"});
        builder.set_uid(1);
        builder.set_user("test");
        
        osmium::builder::WayNodeListBuilder wnl_builder{builder};
        wnl_builder.add_node_ref(10, osmium::Location{1.0, 1.0});
        wnl_builder.add_node_ref(11, osmium::Location{2.0, 1.0});
        
        const auto& way = buffer.get<osmium::Way>(0);
        cmd.check_for_locations_on_ways(way);
        
        REQUIRE(cmd.found_locations_on_ways() == true);
        REQUIRE(cmd.warned_locations_on_ways() == false);
    }

    SECTION("detection logic with way without locations - " + std::string(format)) {
        CommandSort cmd{factory};
        std::vector<std::string> args = {"test/sort/input-simple1.osm", "-f", format};
        cmd.setup(args);
        
        REQUIRE(cmd.found_locations_on_ways() == false);
        
        osmium::memory::Buffer buffer{1024};
        osmium::builder::WayBuilder builder{buffer};
        builder.set_id(123);
        builder.set_version(1);
        builder.set_changeset(1);
        builder.set_timestamp(osmium::Timestamp{"2015-01-01T01:00:00Z"});
        builder.set_uid(1);
        builder.set_user("test");
        
        osmium::builder::WayNodeListBuilder wnl_builder{builder};
        wnl_builder.add_node_ref(10);
        wnl_builder.add_node_ref(11);
        
        const auto& way = buffer.get<osmium::Way>(0);
        cmd.check_for_locations_on_ways(way);
        
        REQUIRE(cmd.found_locations_on_ways() == false);
    }

}