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

#include "geojson_file_parser.hpp"

#include "../exception.hpp"
#include "geometry_util.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/location.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

std::string get_value_as_string(const nlohmann::json& object, const char* key) {
    assert(object.is_object());

    const auto it = object.find(key);
    if (it == object.end()) {
        return {};
    }

    if (it->is_string()) {
        return it->template get<std::string>();
    }

    throw config_error{std::string{"Value for name '"} + key + "' must be a string."};
}

namespace {

// parse coordinate pair from JSON array
osmium::geom::Coordinates parse_coordinate(const nlohmann::json& value) {
    if (!value.is_array()) {
        throw config_error{"Coordinates must be an array."};
    }

    if (value.size() != 2) {
        throw config_error{"Coordinates array must have exactly two elements."};
    }

    if (value[0].is_number() && value[1].is_number()) {
        return osmium::geom::Coordinates{value[0].template get<double>(),
                                         value[1].template get<double>()};
    }

    throw config_error{"Coordinates array must contain numbers."};
}

std::vector<osmium::geom::Coordinates> parse_ring(const nlohmann::json& value) {
    if (!value.is_array()) {
        throw config_error{"Ring must be an array."};
    }

    if (value.size() < 3) {
        throw config_error{"Ring must contain at least three coordinate pairs."};
    }

    std::vector<osmium::geom::Coordinates> coordinates;

    for (const nlohmann::json& item : value) {
        coordinates.push_back(parse_coordinate(item));
    }

    return coordinates;
}

void parse_rings(const nlohmann::json& value, osmium::builder::AreaBuilder* builder) {
    assert(value.is_array());
    if (value.empty()) {
        throw config_error{"Polygon must contain at least one ring."};
    }

    {
        auto outer_ring = parse_ring(value[0]);
        if (!is_ccw(outer_ring)) {
            std::reverse(outer_ring.begin(), outer_ring.end());
        }
        osmium::builder::OuterRingBuilder ring_builder{*builder};
        for (const auto& c : outer_ring) {
            const osmium::Location loc{c.x, c.y};
            if (loc.valid()) {
                ring_builder.add_node_ref(0, loc);
            } else {
                throw config_error{"Invalid location in boundary (multi)polygon: (" +
                                   std::to_string(c.x) +
                                   ", " +
                                   std::to_string(c.y) + ")."};
            }
        }
    }

    for (unsigned int i = 1; i < value.size(); ++i) {
        auto inner_ring = parse_ring(value[i]);
        if (is_ccw(inner_ring)) {
            std::reverse(inner_ring.begin(), inner_ring.end());
        }
        osmium::builder::InnerRingBuilder ring_builder{*builder};
        for (const auto& c : inner_ring) {
            const osmium::Location loc{c.x, c.y};
            if (loc.valid()) {
                ring_builder.add_node_ref(0, loc);
            } else {
                throw config_error{"Invalid location in boundary (multi)polygon: (" +
                                   std::to_string(c.x) +
                                   ", " +
                                   std::to_string(c.y) + ")."};
            }
        }
    }
}

} // anonymous namespace

std::size_t parse_polygon_array(const nlohmann::json& value, osmium::memory::Buffer* buffer) {
    {
        osmium::builder::AreaBuilder builder{*buffer};
        parse_rings(value, &builder);
    }

    return buffer->commit();
}

std::size_t parse_multipolygon_array(const nlohmann::json& value, osmium::memory::Buffer* buffer) {
    assert(value.is_array());
    if (value.empty()) {
        throw config_error{"Multipolygon must contain at least one polygon array."};
    }

    {
        osmium::builder::AreaBuilder builder{*buffer};
        for (const auto& polygon : value) {
            if (!polygon.is_array()) {
                throw config_error{"Polygon must be an array."};
            }
            parse_rings(polygon, &builder);
        }
    }

    return buffer->commit();
}

[[noreturn]] void GeoJSONFileParser::error(const std::string& message) {
    throw geojson_error{std::string{"In file '"} + m_file_name + "':\n" + message};
}

GeoJSONFileParser::GeoJSONFileParser(osmium::memory::Buffer& buffer, std::string file_name) :
    m_buffer(buffer),
    m_file_name(std::move(file_name)),
    m_file(m_file_name) {
    if (!m_file.is_open()) {
        throw config_error{std::string{"Could not open file '"} + m_file_name + "'."};
    }
}

const nlohmann::json& GeoJSONFileParser::get_coordinates(const nlohmann::json& json_geometry) {
    const auto json_coordinates = json_geometry.find("coordinates");
    if (json_coordinates == json_geometry.end()) {
        error("Missing 'coordinates' name in 'geometry' object.");
    }

    if (!json_coordinates->is_array()) {
        error("Expected 'geometry.coordinates' value to be an array.");
    }

    return *json_coordinates;
}

std::size_t GeoJSONFileParser::parse_top(const nlohmann::json& top) {
    const auto json_geometry = top.find("geometry");
    if (json_geometry == top.end()) {
        error("Missing 'geometry' name.");
    }

    if (!json_geometry->is_object()) {
        error("Expected 'geometry' value to be an object.");
    }

    const std::string geometry_type{get_value_as_string(*json_geometry, "type")};
    if (geometry_type.empty()) {
        error("Missing 'geometry.type'.");
    }
    if (geometry_type != "Polygon" && geometry_type != "MultiPolygon") {
        error("Expected 'geometry.type' value to be 'Polygon' or 'MultiPolygon'.");
    }

    const auto json_coordinates = get_coordinates(*json_geometry);

    if (geometry_type == "Polygon") {
        return parse_polygon_array(json_coordinates, &m_buffer);
    }

    return parse_multipolygon_array(json_coordinates, &m_buffer);
}

std::size_t GeoJSONFileParser::operator()() {
    try {
        const nlohmann::json doc = nlohmann::json::parse(m_file);

        if (!doc.is_object()) {
            error("Top-level value must be an object.");
        }

        const std::string type{get_value_as_string(doc, "type")};
        if (type.empty()) {
            error("Expected 'type' name with the value 'Feature', 'FeatureCollection', 'Polygon', or 'MultiPolygon'.");
        }

        if (type == "Polygon") {
            const auto json_coordinates = get_coordinates(doc);
            return parse_polygon_array(json_coordinates, &m_buffer);
        }

        if (type == "MultiPolygon") {
            const auto json_coordinates = get_coordinates(doc);
            return parse_multipolygon_array(json_coordinates, &m_buffer);
        }

        if (type == "Feature") {
            return parse_top(doc);
        }

        if (type == "FeatureCollection") {
            const auto json_features = doc.find("features");
            if (json_features == doc.end()) {
                error("Missing 'features' name.");
            }

            if (!json_features->is_array()) {
                error("Expected 'features' value to be an array.");
            }

            if (json_features->empty()) {
                throw config_error{"Features array must contain at least one polygon."};
            }

            const auto& json_first_feature = (*json_features)[0];
            if (!json_first_feature.is_object()) {
                error("Expected values of 'features' array to be a objects.");
            }

            const std::string feature_type{get_value_as_string(json_first_feature, "type")};

            if (feature_type != "Feature") {
                error("Expected 'type' value to be 'Feature'.");
            }

            return parse_top(json_first_feature);
        }

        error("Expected 'type' value to be 'Feature'.");
    } catch (const nlohmann::json::parse_error &e) {
        error(std::string{"JSON error at offset "} + std::to_string(e.byte) +
              " : " + e.what());
    }
}
