#ifndef EXTRACT_ERROR_HPP
#define EXTRACT_ERROR_HPP

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

#include <stdexcept>
#include <string>

/**
 *  Thrown when there is a problem with parsing the JSON config file.
 */
struct config_error : std::runtime_error {

    explicit config_error(const char* message) :
        std::runtime_error(std::string{"Config file error: "} + message) {
    }

    explicit config_error(const std::string& message) :
        std::runtime_error(std::string{"Config file error: "} + message) {
    }

    explicit config_error(int n, const char* message) :
        std::runtime_error(std::string{"Config file error in extract "} + std::to_string(n) + ": " + message) {
    }

    explicit config_error(int n, const std::string& message) :
        std::runtime_error(std::string{"Config file error in extract "} + std::to_string(n) + ": " + message) {
    }

}; // struct config_error

/**
 *  Thrown when there is a problem with parsing a GeoJSON file.
 */
struct geojson_error : std::runtime_error {

    explicit geojson_error(const std::string& file_name, const char* message) :
        std::runtime_error(std::string{"Error in GeoJSON file '"} + file_name + "': " + message) {
    }

    explicit geojson_error(const std::string& file_name, const std::string& message) :
        std::runtime_error(std::string{"Error in GeoJSON file '"} + file_name + "': " + message) {
    }

    explicit geojson_error(const std::string& message) :
        std::runtime_error(message) {
    }

}; // struct geojson_error

#endif // EXTRACT_ERROR_HPP
