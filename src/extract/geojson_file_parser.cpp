/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2018  Jochen Topf <jochen@topf.org>

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

#include "../exception.hpp"
#include "geojson_file_parser.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/location.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

std::string get_value_as_string(const rapidjson::Value& object, const char* key) {
    assert(object.IsObject());

    const auto it = object.FindMember(key);
    if (it == object.MemberEnd()) {
        return "";
    }

    if (it->value.IsString()) {
        return it->value.GetString();
    }

    throw config_error{std::string{"Value for name '"} + key + "' must be a string."};
}

// parse coordinate pair from JSON array
osmium::geom::Coordinates parse_coordinate(const rapidjson::Value& value) {
    if (!value.IsArray()) {
        throw config_error{"Coordinates must be an array."};
    }

    const auto array = value.GetArray();
    if (array.Size() != 2) {
        throw config_error{"Coordinates array must have exactly two elements."};
    }

    if (array[0].IsNumber() && array[1].IsNumber()) {
        return osmium::geom::Coordinates{array[0].GetDouble(), array[1].GetDouble()};
    }

    throw config_error{"Coordinates array must contain numbers."};
}

std::vector<osmium::geom::Coordinates> parse_ring(const rapidjson::Value& value) {
    if (!value.IsArray()) {
        throw config_error{"Ring must be an array."};
    }

    const auto array = value.GetArray();
    if (array.Size() < 3) {
        throw config_error{"Ring must contain at least three coordinate pairs."};
    }

    std::vector<osmium::geom::Coordinates> coordinates;

    for (const rapidjson::Value& item : array) {
        coordinates.push_back(parse_coordinate(item));
    }

    return coordinates;
}

void parse_rings(const rapidjson::Value& value, osmium::builder::AreaBuilder& builder) {
    assert(value.IsArray());
    const auto array = value.GetArray();
    if (array.Empty()) {
        throw config_error{"Polygon must contain at least one ring."};
    }

    {
        const auto outer_ring = parse_ring(array[0]);
        osmium::builder::OuterRingBuilder ring_builder{builder};
        for (const auto& c : outer_ring) {
            ring_builder.add_node_ref(0, osmium::Location{c.x, c.y});
        }
    }

    for (unsigned int i = 1; i < array.Size(); ++i) {
        const auto inner_ring = parse_ring(array[i]);
        osmium::builder::InnerRingBuilder ring_builder{builder};
        for (const auto& c : inner_ring) {
            ring_builder.add_node_ref(0, osmium::Location{c.x, c.y});
        }
    }
}

std::size_t parse_polygon_array(const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
    {
        osmium::builder::AreaBuilder builder{buffer};
        parse_rings(value, builder);
    }

    return buffer.commit();
}

std::size_t parse_multipolygon_array(const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
    assert(value.IsArray());
    const auto array = value.GetArray();
    if (array.Empty()) {
        throw config_error{"Multipolygon must contain at least one polygon array."};
    }

    {
        osmium::builder::AreaBuilder builder{buffer};
        for (const auto& polygon : array) {
            if (!polygon.IsArray()) {
                throw config_error{"Polygon must be an array."};
            }
            parse_rings(polygon, builder);
        }
    }

    return buffer.commit();
}

OSMIUM_NORETURN void GeoJSONFileParser::error(const std::string& message) {
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

std::size_t GeoJSONFileParser::parse_top(const rapidjson::Value& top) {
    const auto json_geometry = top.FindMember("geometry");
    if (json_geometry == top.MemberEnd()) {
        error("Missing 'geometry' name.");
    }

    if (!json_geometry->value.IsObject()) {
        error("Expected 'geometry' value to be an object.");
    }

    std::string geometry_type{get_value_as_string(json_geometry->value, "type")};
    if (geometry_type.empty()) {
        error("Missing 'geometry.type'.");
    }
    if (geometry_type != "Polygon" && geometry_type != "MultiPolygon") {
        error("Expected 'geometry.type' value to be 'Polygon' or 'MultiPolygon'.");
    }

    const auto json_coordinates = json_geometry->value.FindMember("coordinates");
    if (json_coordinates == json_geometry->value.MemberEnd()) {
        error("Missing 'coordinates' name in 'geometry' object.");
    }

    if (!json_coordinates->value.IsArray()) {
        error("Expected 'geometry.coordinates' value to be an array.");
    }

    if (geometry_type == "Polygon") {
        return parse_polygon_array(json_coordinates->value, m_buffer);
    }

    return parse_multipolygon_array(json_coordinates->value, m_buffer);
}

std::size_t GeoJSONFileParser::operator()() {
    rapidjson::IStreamWrapper stream_wrapper{m_file};

    rapidjson::Document doc;
    if (doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(stream_wrapper).HasParseError()) {
        error(std::string{"JSON error at offset "} +
              std::to_string(doc.GetErrorOffset()) +
              " : " +
              rapidjson::GetParseError_En(doc.GetParseError()));
    }

    if (!doc.IsObject()) {
        error("Top-level value must be an object.");
    }

    const std::string type{get_value_as_string(doc, "type")};
    if (type.empty()) {
        error("Expected 'type' name with the value 'Feature' or 'FeatureCollection'.");
    }

    if (type == "Feature") {
        return parse_top(doc);
    }

    if (type == "FeatureCollection") {
        const auto json_features = doc.FindMember("features");
        if (json_features == doc.MemberEnd()) {
            error("Missing 'features' name.");
        }

        if (!json_features->value.IsArray()) {
            error("Expected 'features' value to be an array.");
        }

        const auto json_features_array = json_features->value.GetArray();
        if (json_features_array.Empty()) {
            throw config_error{"Features array must contain at least one polygon."};
        }

        const auto& json_first_feature = json_features_array[0];
        if (!json_first_feature.IsObject()) {
            error("Expected values of 'features' array to be a objects.");
        }

        const std::string feature_type{get_value_as_string(json_first_feature, "type")};

        if (feature_type != "Feature") {
            error("Expected 'type' value to be 'Feature'.");
        }

        return parse_top(json_first_feature);
    }

    error("Expected 'type' value to be 'Feature'.");
}

