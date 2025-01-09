#ifndef COMMAND_ADD_LOCATIONS_TO_WAYS_HPP
#define COMMAND_ADD_LOCATIONS_TO_WAYS_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2025  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "cmd.hpp" // IWYU pragma: export

#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/id_set.hpp>
#include <osmium/index/map/all.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/util/progress_bar.hpp>

#include <string>
#include <vector>

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type, index_type>;

class CommandAddLocationsToWays : public CommandWithMultipleOSMInputs, public with_osm_output {

    osmium::index::IdSetSmall<osmium::unsigned_object_id_type> m_member_node_ids;
    std::string m_index_type_name_pos;
    std::string m_index_type_name_neg;
    bool m_keep_untagged_nodes = false;
    bool m_keep_member_nodes = false;
    bool m_ignore_missing_nodes = false;

    void find_member_nodes();
    void copy_data(osmium::ProgressBar& progress_bar, osmium::io::Reader& reader, osmium::io::Writer& writer, location_handler_type& location_handler) const;

public:

    explicit CommandAddLocationsToWays(const CommandFactory& command_factory) :
        CommandWithMultipleOSMInputs(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "add-locations-to-ways";
    }

    const char* synopsis() const noexcept override final {
        return "osmium add-locations-to-ways [OPTIONS] OSM-FILE...";
    }

}; // class CommandAddLocationsToWays


#endif // COMMAND_ADD_LOCATIONS_TO_WAYS_HPP
