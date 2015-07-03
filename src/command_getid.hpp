#ifndef COMMAND_GETID_HPP
#define COMMAND_GETID_HPP

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

#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>

#include "osmc.hpp"

class CommandGetId : public Command, with_single_osm_input, with_osm_output {

    std::vector<osmium::object_id_type> m_ids[3];

public:

    CommandGetId() = default;

    std::vector<osmium::object_id_type>& ids(osmium::item_type type) noexcept;

    void sort_unique(osmium::item_type type);

    bool setup(const std::vector<std::string>& arguments) override final;

    bool run() override final;

}; // class CommandGetId


#endif // COMMAND_GETID_HPP
