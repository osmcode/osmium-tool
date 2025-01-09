#ifndef EXTRACT_GEOMETRY_UTIL_HPP
#define EXTRACT_GEOMETRY_UTIL_HPP

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

#include <osmium/geom/coordinates.hpp>

#include <vector>

double calculate_double_area(const std::vector<osmium::geom::Coordinates>& coordinates);

double calculate_double_area(const std::vector<osmium::Location>& coordinates);

/// Is the ring defined by the coordinates counter-clockwise?
template <typename T>
bool is_ccw(T& coordinates) {
    return calculate_double_area(coordinates) > 0;
}

#endif // EXTRACT_GEOMETRY_UTIL_HPP
