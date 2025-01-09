#ifndef COMMAND_EXPORT_HPP
#define COMMAND_EXPORT_HPP

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
#include "export/options.hpp"
#include "export/ruleset.hpp"

#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/all.hpp>
#include <osmium/io/writer_options.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

class CommandExport : public CommandWithSingleOSMInput {

    using index_type = osmium::index::map::Map<osmium::unsigned_object_id_type, osmium::Location>;
    using location_handler_type = osmium::handler::NodeLocationsForWays<index_type, index_type>;

    options_type m_options{};

    Ruleset m_linear_ruleset;
    Ruleset m_area_ruleset;

    std::vector<std::string> m_include_tags;
    std::vector<std::string> m_exclude_tags;

    std::string m_config_file_name;
    std::string m_index_type_name;
    std::string m_output_filename;
    std::string m_output_format;

    geometry_types m_geometry_types;

    osmium::io::overwrite m_output_overwrite = osmium::io::overwrite::no;
    osmium::io::fsync m_fsync = osmium::io::fsync::no;

    bool m_show_errors = false;
    bool m_stop_on_error = false;

    void canonicalize_output_format();
    void parse_attributes(const nlohmann::json& attributes);
    void parse_format_options(const nlohmann::json& options);
    void parse_config_file();

public:

    explicit CommandExport(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

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
