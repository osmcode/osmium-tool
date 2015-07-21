#ifndef COMMAND_RENUMBER_HPP
#define COMMAND_RENUMBER_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2015  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <map>
#include <string>
#include <vector>

#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types.hpp>

#include "osmium-tool.hpp"

typedef std::map<osmium::unsigned_object_id_type, osmium::unsigned_object_id_type> remap_index_type;
typedef remap_index_type::value_type remap_index_value_type;

class CommandRenumber : public Command, public with_single_osm_input, public with_osm_output {

    std::string m_index_directory;

    remap_index_type m_id_index[3];
    osmium::object_id_type m_last_id[3];

public:

    CommandRenumber() {
        // workaround for MSVC
        m_last_id[0] = 0;
        m_last_id[1] = 0;
        m_last_id[2] = 0;
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    osmium::object_id_type lookup(osmium::item_type type, osmium::object_id_type id);

    void renumber(osmium::memory::Buffer& buffer);

    std::string filename(const std::string& name);

    remap_index_type& index(osmium::item_type type);
    osmium::object_id_type& last_id(osmium::item_type type);

    void read_index(osmium::item_type type, const std::string& name);

    void write_index(osmium::item_type type, const std::string& name);

    bool run() override final;

}; // class CommandRenumber


#endif // COMMAND_RENUMBER_HPP
