#ifndef COMMAND_EXPORT_HPP
#define COMMAND_EXPORT_HPP

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

#include <rapidjson/document.h>

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/handler/check_order.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/visitor.hpp>

// these must be included in this order
#include <osmium/index/map/all.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include "cmd.hpp" // IWYU pragma: export

#include "export/options.hpp"

class CommandExport : public Command, public with_single_osm_input {

    using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
    using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

    options_type m_options;

    std::vector<std::string> m_linear_tags;
    std::vector<std::string> m_area_tags;
    std::vector<std::string> m_include_tags;
    std::vector<std::string> m_exclude_tags;

    std::string m_config_file_name;
    std::string m_index_type_name;
    std::string m_output_filename;
    std::string m_output_format;

    osmium::io::overwrite m_output_overwrite = osmium::io::overwrite::no;
    osmium::io::fsync m_fsync = osmium::io::fsync::no;

    bool m_show_errors = false;

    void canonicalize_output_format();
    void parse_options(const rapidjson::Value& attributes);
    void parse_config_file();

public:

    CommandExport() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "export";
    }

    const char* synopsis() const noexcept override final {
        return "osmium export [OPTIONS] OSM-FILE";
    }

}; // class CommandExport


#endif // COMMAND_EXPORT_HPP
