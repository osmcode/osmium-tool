#ifndef EXTRACT_GEOJSON_FILE_PARSER_HPP
#define EXTRACT_GEOJSON_FILE_PARSER_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

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

namespace osmium { namespace memory {
    class Buffer;
}}

extern std::string get_value_as_string(const rapidjson::Value& object, const char* key);
extern osmium::Box parse_bbox(const rapidjson::Value& value);
extern std::size_t parse_polygon_array(const rapidjson::Value& value, osmium::memory::Buffer& buffer);
extern std::size_t parse_multipolygon_array(const rapidjson::Value& value, osmium::memory::Buffer& buffer);

/**
 * Gets areas from OSM files.
 */
class GeoJSONFileParser {

    osmium::memory::Buffer& m_buffer;
    std::string m_file_name;

    void error(const std::string& message);

public:

    GeoJSONFileParser(osmium::memory::Buffer& buffer, const std::string& file_name);

    std::size_t operator()();

}; // class GeoJSONFileParser

#endif // EXTRACT_GEOJSON_FILE_PARSER_HPP
