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

#include "poly_file_parser.hpp"

#include "../exception.hpp"
#include "geometry_util.hpp"

#include <osmium/geom/coordinates.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/util/string.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

[[noreturn]] void PolyFileParser::error(const std::string& message) {
    throw poly_error{std::string{"In file '"} + m_file_name + "' on line " + std::to_string(m_line + 1) + ":\n" + message};
}

PolyFileParser::PolyFileParser(osmium::memory::Buffer& buffer, const std::string& file_name) :
    m_buffer(buffer),
    m_builder(nullptr),
    m_file_name(file_name) {
    std::ifstream file{file_name};
    if (!file.is_open()) {
        throw config_error{std::string{"Could not open file '"} + file_name + "'"};
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    m_data = osmium::split_string(sstr.str(), '\n', true);

    // remove CR at end of lines
    for (auto& line : m_data) {
        if (line.back() == '\r') {
            line.resize(line.size() - 1);
        }
    }
}

void PolyFileParser::parse_ring() {
    const bool is_inner_ring = line()[0] == '!';
    ++m_line;

    std::vector<osmium::Location> coordinates;
    while (m_line < m_data.size()) {
        if (line() == "END") {
            if (coordinates.size() < 3) {
                error("Expected at least three lines with coordinates.");
            }

            if (coordinates.front() != coordinates.back()) {
                coordinates.push_back(coordinates.front());
            }

            if (is_inner_ring) {
                if (is_ccw(coordinates)) {
                    std::reverse(coordinates.begin(), coordinates.end());
                }
                osmium::builder::InnerRingBuilder ring_builder{*m_builder};
                for (const auto& location : coordinates) {
                    ring_builder.add_node_ref(0, location);
                }
            } else {
                if (!is_ccw(coordinates)) {
                    std::reverse(coordinates.begin(), coordinates.end());
                }
                osmium::builder::OuterRingBuilder ring_builder{*m_builder};
                for (const auto& location : coordinates) {
                    ring_builder.add_node_ref(0, location);
                }
            }

            ++m_line;
            return;
        }

        std::istringstream sstr{line()};
        double lon = NAN;
        double lat = NAN;
        if (!(sstr >> lon >> lat)) {
            error("Expected coordinates or 'END' to end the ring.");
        }
        coordinates.emplace_back(lon, lat);

        if (!coordinates.back().valid()) {
            throw config_error{"Invalid location in boundary (multi)polygon: (" + std::to_string(lon) + ", " + std::to_string(lat) + ")."};
        }

        ++m_line;
    }
}

void PolyFileParser::parse_multipolygon() {
    ++m_line; // ignore first line

    while (m_line < m_data.size()) {
        if (line() == "END") {
            ++m_line;
            if (m_line == 2) {
                error("Need at least one ring in (multi)polygon.");
            }
            return;
        }
        parse_ring();
    }

    --m_line;
    error("Expected 'END' for end of (multi)polygon.");
}

std::size_t PolyFileParser::operator()() {
    if (m_data.empty()) {
        throw poly_error{std::string{"File '"} + m_file_name + "' is empty."};
    }

    m_builder = std::make_unique<osmium::builder::AreaBuilder>(m_buffer);
    while (m_line < m_data.size()) {
        parse_multipolygon();
    }
    m_builder.reset();

    return m_buffer.commit();
}

