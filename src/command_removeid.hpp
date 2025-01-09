#ifndef COMMAND_REMOVEID_HPP
#define COMMAND_REMOVEID_HPP

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

#include <osmium/fwd.hpp>
#include <osmium/index/id_set.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/index/relations_map.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

class CommandRemoveId : public CommandWithSingleOSMInput, public with_osm_output {

    osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>> m_ids;

    osmium::item_type m_default_item_type = osmium::item_type::node;

public:

    explicit CommandRemoveId(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "removeid";
    }

    const char* synopsis() const noexcept override final {
        return "osmium removeid [OPTIONS] OSM-FILE ID...\n"
               "       osmium removeid [OPTIONS] OSM-FILE -i ID-FILE\n"
               "       osmium removeid [OPTIONS] OSM-FILE -I ID-OSM-FILE";
    }

}; // class CommandRemoveId


#endif // COMMAND_REMOVEID_HPP
