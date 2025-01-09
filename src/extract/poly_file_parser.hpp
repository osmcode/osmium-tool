#ifndef EXTRACT_POLY_FILE_PARSER_HPP
#define EXTRACT_POLY_FILE_PARSER_HPP

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

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/memory/buffer.hpp>

#include <memory>
#include <string>
#include <vector>

/**
 *  Thrown when there is a problem with parsing a poly file.
 */
struct poly_error : public std::runtime_error {

    explicit poly_error(const std::string& message) :
        std::runtime_error(message) {
    }

}; // struct poly_error

/**
 * Gets areas from .poly files.
 *
 * Format description:
 * https://wiki.openstreetmap.org/wiki/Osmosis/Polygon_Filter_File_Format
 */
class PolyFileParser {

    osmium::memory::Buffer& m_buffer;
    std::unique_ptr<osmium::builder::AreaBuilder> m_builder;
    std::string m_file_name;
    std::vector<std::string> m_data;
    std::size_t m_line = 0;

    void parse_ring();
    void parse_multipolygon();

    const std::string& line() const noexcept {
        return m_data[m_line];
    }

    [[noreturn]] void error(const std::string& message);

public:

    PolyFileParser(osmium::memory::Buffer& buffer, const std::string& file_name);

    std::size_t operator()();

}; // class PolyFileParser

#endif // EXTRACT_POLY_FILE_PARSER_HPP
