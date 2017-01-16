
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/location.hpp>

#include "error.hpp"
#include "geojson_file_parser.hpp"

std::string get_value_as_string(const rapidjson::Value& object, const char* key) {
    assert(object.IsObject());

    const auto it = object.FindMember(key);
    if (it == object.MemberEnd()) {
        return "";
    }

    if (it->value.IsString()) {
        return it->value.GetString();
    } else {
        throw config_error{std::string{"Value for name '"} + key + "' is not a string"};
    }
}

osmium::Box parse_bbox(const rapidjson::Value& value) {
    if (value.IsArray()) {
        if (value.Size() == 4) {
            if (value[0].IsNumber() && value[1].IsNumber() && value[2].IsNumber() && value[3].IsNumber()) {
                const osmium::Location bottom_left{value[0].GetDouble(), value[1].GetDouble()};
                const osmium::Location top_right{value[2].GetDouble(), value[3].GetDouble()};

                if (bottom_left < top_right) {
                    return osmium::Box{bottom_left, top_right};
                }
                throw config_error{"'bbox' array elements must be in order: left, bottom, right, top"};
            }
            throw config_error{"'bbox' array elements must be numbers"};
        } else {
            throw config_error{"'bbox' member is not an array of length 4"};
        }
    } else if (value.IsObject()) {
        const auto left   = value.FindMember("left");
        const auto right  = value.FindMember("right");
        const auto top    = value.FindMember("top");
        const auto bottom = value.FindMember("bottom");

        if (left != value.MemberEnd() && right  != value.MemberEnd() &&
            top  != value.MemberEnd() && bottom != value.MemberEnd()) {
            if (left->value.IsNumber() && right->value.IsNumber() &&
                top->value.IsNumber()  && bottom->value.IsNumber()) {

                const osmium::Location bottom_left{left->value.GetDouble(), bottom->value.GetDouble()};
                const osmium::Location top_right{right->value.GetDouble(), top->value.GetDouble()};

                if (bottom_left < top_right) {
                    return osmium::Box{bottom_left, top_right};
                }

                throw config_error{"Need 'left' < 'right' and 'bottom' < 'top'"};
            }
        }

        throw config_error{"Need 'left', 'right', 'top', and 'bottom' members"};
    }

    throw config_error{"'bbox' member is not an array or object"};
}

// parse coordinate pair from JSON array
osmium::geom::Coordinates parse_coordinate(const rapidjson::Value& value) {
    if (!value.IsArray()) {
        throw config_error{"coordinates must be an array"};
    }

    const auto array = value.GetArray();
    if (array.Size() != 2) {
        throw config_error{"coordinates array must have size 2"};
    }

    if (array[0].IsNumber() && array[1].IsNumber()) {
        return osmium::geom::Coordinates{array[0].GetDouble(), array[1].GetDouble()};
    }

    throw config_error{"coordinates array must contain numbers"};
}

std::vector<osmium::geom::Coordinates> parse_ring(const rapidjson::Value& value) {
    if (!value.IsArray()) {
        throw config_error{"ring must be an array"};
    }

    const auto array = value.GetArray();
    if (array.Size() < 3) {
        throw config_error{"ring must contain at least three coordinates"};
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
    if (array.Size() < 1) {
        throw config_error{"polygon must contain at least one ring"};
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
    if (array.Size() < 1) {
        throw config_error{"multipolygon must contain at least one polygon"};
    }

    for (const auto& polygon : array) {
        if (!polygon.IsArray()) {
            throw config_error{"polygon must be an array"};
        }
        osmium::builder::AreaBuilder builder{buffer};
        parse_rings(polygon, builder);
    }

    return buffer.commit();
}

void GeoJSONFileParser::error(const std::string& message) {
    throw geojson_error{message + " in file '" + m_file_name + "'"};
}

GeoJSONFileParser::GeoJSONFileParser(osmium::memory::Buffer& buffer, const std::string& file_name) :
    m_buffer(buffer),
    m_file_name(file_name) {
}

std::size_t GeoJSONFileParser::operator()() {
    std::ifstream file{m_file_name};

    if (!file.is_open()) {
        throw config_error{std::string{"Could not open file '"} + m_file_name + "'"};
    }

    rapidjson::IStreamWrapper stream_wrapper{file};

    rapidjson::Document doc;
    if (doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(stream_wrapper).HasParseError()) {
        throw geojson_error{m_file_name, std::string{"JSON error at offset "} +
                            std::to_string(doc.GetErrorOffset()) +
                            " : " +
                            rapidjson::GetParseError_En(doc.GetParseError())
                            };
    }

    if (!doc.IsObject()) {
        error("Top-level value must be an object");
    }

    std::string type{get_value_as_string(doc, "type")};
    if (type != "Feature") {
        error("Expect 'type' value to be 'Feature'");
    }

    const auto json_geometry = doc.FindMember("geometry");
    if (json_geometry == doc.MemberEnd()) {
        error("Missing 'geometry' member");
    }

    if (!json_geometry->value.IsObject()) {
        error("Expect 'geometry' value to be an object");

    }

    type = get_value_as_string(json_geometry->value, "type");
    if (type != "Polygon" && type != "Multipolygon") {
        error("Expect 'type' value to be 'Polygon' or 'Multipolygon'");
    }

    const auto json_coordinates = json_geometry->value.FindMember("coordinates");
    if (json_coordinates == json_geometry->value.MemberEnd()) {
        error("Missing 'coordinates' member");
    }

    if (!json_coordinates->value.IsArray()) {
        error("Expect 'coordinates' value to be an array");
    }

    if (type == "Polygon") {
        return parse_polygon_array(json_coordinates->value, m_buffer);
    } else {
        return parse_multipolygon_array(json_coordinates->value, m_buffer);
    }
}

