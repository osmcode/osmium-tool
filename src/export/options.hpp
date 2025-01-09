#ifndef EXPORT_OPTIONS_HPP
#define EXPORT_OPTIONS_HPP

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

#include <osmium/tags/tags_filter.hpp>
#include <osmium/util/options.hpp>

#include <string>

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

    osmium::Options format_options;

    bool keep_untagged = false;
};

struct geometry_types {

    bool point = true;
    bool linestring = true;
    bool polygon = true;

    void clear() noexcept {
        point = false;
        linestring = false;
        polygon = false;
    }

    bool empty() const noexcept {
        return !point && !linestring && !polygon;
    }

}; // struct geometry_types

#endif // EXPORT_OPTIONS_HPP
