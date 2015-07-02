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

#include <string>
#include <vector>

#include <osmium/index/map/sparse_mem_map.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types.hpp>

#include "osmc.hpp"

typedef osmium::index::map::SparseMemMap<osmium::unsigned_object_id_type, osmium::unsigned_object_id_type> remap_index_type;

class CommandRenumber : public Command, with_single_osm_input, with_osm_output {

    std::vector<std::string> m_output_headers;

    remap_index_type m_id_index[3];
    osmium::object_id_type m_last_id[3] = {0, 0, 0};

public:

    CommandRenumber() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    osmium::object_id_type lookup(int n, osmium::object_id_type id);

    void renumber(osmium::memory::Buffer& buffer);

    bool run() override final;

}; // class CommandRenumber


#endif // COMMAND_RENUMBER_HPP
