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

#include "extract_bbox.hpp"

#include <osmium/osm/location.hpp>

#include <iterator>
#include <string>

bool ExtractBBox::contains(const osmium::Location& location) const noexcept {
    return location.valid() && envelope().contains(location);
}

const char* ExtractBBox::geometry_type() const noexcept {
    return "bbox";
}

std::string ExtractBBox::geometry_as_text() const {
    std::string s{"BOX("};
    envelope().bottom_left().as_string(std::back_inserter(s), ' ');
    s += ',';
    envelope().top_right().as_string(std::back_inserter(s), ' ');
    s += ')';
    return s;
}

