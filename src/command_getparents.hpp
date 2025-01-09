#ifndef COMMAND_GETPARENTS_HPP
#define COMMAND_GETPARENTS_HPP

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

class CommandGetParents : public CommandWithSingleOSMInput, public with_osm_output {

    osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>> m_ids;

    osmium::item_type m_default_item_type = osmium::item_type::node;

    bool m_add_self = false;
    bool m_verbose_ids = false;

    osmium::osm_entity_bits::type get_needed_types() const;

    void add_nodes(const osmium::Way& way);
    void add_members(const osmium::Relation& relation);

public:

    explicit CommandGetParents(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "getparents";
    }

    const char* synopsis() const noexcept override final {
        return "osmium getparents [OPTIONS] OSM-FILE ID...\n"
               "       osmium getparents [OPTIONS] OSM-FILE -i ID-FILE\n"
               "       osmium getparents [OPTIONS] OSM-FILE -I ID-OSM-FILE";
    }

}; // class CommandGetParents


#endif // COMMAND_GETPARENTS_HPP
