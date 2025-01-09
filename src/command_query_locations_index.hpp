#ifndef COMMAND_QUERY_LOCATIONS_INDEX_HPP
#define COMMAND_QUERY_LOCATIONS_INDEX_HPP

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

#include <osmium/osm/types.hpp>

#include <string>
#include <vector>

class CommandQueryLocationsIndex : public Command, public with_osm_output {

    std::string m_index_file_name;
    osmium::object_id_type m_id = 0;
    bool m_dump = false;

public:

    explicit CommandQueryLocationsIndex(const CommandFactory& command_factory) :
        Command(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "query-locations-index";
    }

    const char* synopsis() const noexcept override final {
        return "osmium query-locations-index -i INDEX-FILE [OPTIONS] NODE-ID";
    }

}; // class CommandQueryLocationsIndex


#endif // COMMAND_QUERY_LOCATIONS_INDEX_HPP
