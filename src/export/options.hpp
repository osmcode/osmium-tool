#ifndef EXPORT_OPTIONS_HPP
#define EXPORT_OPTIONS_HPP

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

#include <osmium/tags/tags_filter.hpp>

enum class unique_id_type {
    none    = 0,
    counter = 1,
    type_id = 2
};

struct options_type {
    osmium::TagsFilter tags_filter{true};
    std::string type;
    std::string id;
    std::string version;
    std::string changeset;
    std::string timestamp;
    std::string uid;
    std::string user;
    std::string way_nodes;

    unique_id_type unique_id = unique_id_type::none;

    bool keep_untagged = false;
};

#endif // EXPORT_OPTIONS_HPP
