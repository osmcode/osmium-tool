#ifndef COMMAND_ADD_LOCATIONS_TO_WAYS_HPP
#define COMMAND_ADD_LOCATIONS_TO_WAYS_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2017  Jochen Topf <jochen@topf.org>

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

#include <string>
#include <vector>

#include <osmium/index/map/dense_mem_array.hpp> // IWYU pragma: keep
#include <osmium/index/map/dense_mmap_array.hpp> // IWYU pragma: keep
#include <osmium/index/map/sparse_mem_array.hpp> // IWYU pragma: keep
#include <osmium/index/map/sparse_mmap_array.hpp> // IWYU pragma: keep
#include <osmium/handler/node_locations_for_ways.hpp>

namespace osmium { namespace io {
    class Header;
    class Reader;
    class Writer;
}}

#include "cmd.hpp" // IWYU pragma: export

using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

class CommandAddLocationsToWays : public Command, public with_multiple_osm_inputs, public with_osm_output {

    void copy_data(osmium::ProgressBar& progress_bar, osmium::io::Reader& reader, osmium::io::Writer& writer, location_handler_type& location_handler);

    std::string m_index_type_name;
    bool m_keep_untagged_nodes = false;
    bool m_ignore_missing_nodes = false;

public:

    CommandAddLocationsToWays() = default;

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
