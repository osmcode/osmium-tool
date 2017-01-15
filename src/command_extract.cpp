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

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <osmium/geom/coordinates.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>

#include "command_extract.hpp"
#include "exception.hpp"

#include "extract/osm_file_parser.hpp"
#include "extract/poly_file_parser.hpp"
#include "extract/strategy_simple.hpp"
#include "extract/strategy_complete_ways.hpp"
#include "extract/strategy_complete_ways_with_history.hpp"
#include "extract/strategy_smart.hpp"

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

namespace {

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

    std::size_t read_geojson_file(const std::string& file_name, osmium::memory::Buffer& buffer) {
        std::ifstream file{file_name};

        if (!file.is_open()) {
            throw config_error{std::string{"Could not open file '"} + file_name + "'"};
        }

        rapidjson::IStreamWrapper stream_wrapper{file};

        rapidjson::Document doc;
        if (doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(stream_wrapper).HasParseError()) {
            throw geojson_error{file_name, std::string{"JSON error at offset "} +
                                std::to_string(doc.GetErrorOffset()) +
                                " : " +
                                rapidjson::GetParseError_En(doc.GetParseError())
                                };
        }

        if (!doc.IsObject()) {
            throw geojson_error{file_name, "Top-level value must be an object"};
        }

        std::string type{get_value_as_string(doc, "type")};
        if (type != "Feature") {
            throw geojson_error{file_name, "Expect 'type' value to be 'Feature'"};
        }

        const auto json_geometry = doc.FindMember("geometry");
        if (json_geometry == doc.MemberEnd()) {
            throw geojson_error{file_name, "Missing 'geometry' member"};
        }

        if (!json_geometry->value.IsObject()) {
            throw geojson_error{file_name, "Expect 'geometry' value to be an object"};

        }

        type = get_value_as_string(json_geometry->value, "type");
        if (type != "Polygon" && type != "Multipolygon") {
            throw geojson_error{file_name, "Expect 'type' value to be 'Polygon' or 'Multipolygon'"};
        }

        const auto json_coordinates = json_geometry->value.FindMember("coordinates");
        if (json_coordinates == json_geometry->value.MemberEnd()) {
            throw geojson_error{file_name, "Missing 'coordinates' member"};
        }

        if (!json_coordinates->value.IsArray()) {
            throw geojson_error{file_name, "Expect 'coordinates' value to be an array"};
        }

        if (type == "Polygon") {
            return parse_polygon_array(json_coordinates->value, buffer);
        } else {
            return parse_multipolygon_array(json_coordinates->value, buffer);
        }

    }

    std::size_t parse_multipolygon_object(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
        std::string file_name{get_value_as_string(value, "file_name")};
        const std::string file_type{get_value_as_string(value, "file_type")};

        if (file_name.empty()) {
            throw config_error{"missing file_name"};
        }

        if (file_name[0] != '/') {
            // relative file name
            file_name = directory + file_name;
        }

        if (file_type == "osm") {
            OSMFileParser parser{buffer, file_name};
            return parser();
        } else if (file_type == "geojson") {
            return read_geojson_file(file_name, buffer);
        } else if (file_type == "poly") {
            std::ifstream file{file_name};
            if (!file.is_open()) {
                throw config_error{std::string{"Could not open file '"} + file_name + "'"};
            }
            std::stringstream sstr;
            sstr << file.rdbuf();
            PolyFileParser parser{buffer, file_name, sstr.str()};
            return parser();
        } else if (file_type == "") {
            throw config_error{"missing file_type"};
        }

        throw config_error{std::string{"unknown file type: '"} + file_type + "'"};
    }

    std::size_t parse_polygon(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
        if (value.IsArray()) {
            return parse_polygon_array(value, buffer);
        } else if (value.IsObject()) {
            return parse_multipolygon_object(directory, value, buffer);
        }

        throw config_error{"polygon must be an object or array"};
    }

    std::size_t parse_multipolygon(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
        if (value.IsArray()) {
            return parse_multipolygon_array(value, buffer);
        } else if (value.IsObject()) {
            return parse_multipolygon_object(directory, value, buffer);
        }

        throw config_error{"multipolygon must be an object or array"};
    }

} // anonymous namespace

void CommandExtract::set_directory(const std::string& directory) {
    m_output_directory = directory;
    if (m_output_directory.empty() || m_output_directory.back() != '/') {
        m_output_directory += '/';
    }
}

void CommandExtract::parse_config_file() {
    std::ifstream config_file{m_config_file_name};
    rapidjson::IStreamWrapper stream_wrapper{config_file};

    rapidjson::Document doc;
    if (doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(stream_wrapper).HasParseError()) {
        throw config_error{std::string{"JSON error at offset "} +
                           std::to_string(doc.GetErrorOffset()) +
                           " : " +
                           rapidjson::GetParseError_En(doc.GetParseError())
                          };
    }

    if (!doc.IsObject()) {
        throw config_error{"Top-level value must be an object"};
    }

    std::string directory{get_value_as_string(doc, "directory")};
    if (!directory.empty() && m_output_directory.empty()) {
        set_directory(directory);
    }

    const auto json_extracts = doc.FindMember("extracts");
    if (json_extracts == doc.MemberEnd()) {
        throw config_error{"Missing 'extracts' member in top-level object"};
    }

    if (!json_extracts->value.IsArray()) {
        throw config_error{"'extracts' member in top-level object must be array"};
    }

    int extract_num = 1;
    for (const auto& e : json_extracts->value.GetArray()) {
        if (!e.IsObject()) {
            throw config_error{extract_num, "Members in extracts array must be objects"};
        }

        std::string output{get_value_as_string(e, "output")};
        if (output.empty()) {
            throw config_error{extract_num, "Missing 'output' field for extract"};
        }

        std::string output_format{get_value_as_string(e, "output_format")};
        std::string description{get_value_as_string(e, "description")};

        const auto json_bbox         = e.FindMember("bbox");
        const auto json_polygon      = e.FindMember("polygon");
        const auto json_multipolygon = e.FindMember("multipolygon");

        try {
            if (json_bbox != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractBBox{m_output_directory + output, output_format, description, parse_bbox(json_bbox->value)});
            } else if (json_polygon != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractPolygon{m_output_directory + output, output_format, description, m_buffer, parse_polygon(m_config_directory, json_polygon->value, m_buffer)});
            } else if (json_multipolygon != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractPolygon{m_output_directory + output, output_format, description, m_buffer, parse_multipolygon(m_config_directory, json_multipolygon->value, m_buffer)});
            } else {
                throw config_error{"Missing geometry for extract"};
            }
        } catch (const config_error& e) {
            throw config_error{std::string{e.what()} + " (in extract " + std::to_string(extract_num) + ")"};
        } catch (const poly_error& e) {
            throw poly_error{std::string{e.what()} + " (in extract " + std::to_string(extract_num) + ")"};
        } catch (const geojson_error& e) {
            throw geojson_error{std::string{e.what()} + " (in extract " + std::to_string(extract_num) + ")"};
        }

        ++extract_num;
    }
}

std::unique_ptr<ExtractStrategy> CommandExtract::make_strategy(const std::string& name) {
    if (name == "simple") {
        if (m_with_history) {
            throw argument_error{"The 'simple' strategy is not supported for history files"};
        } else {
            return std::unique_ptr<ExtractStrategy>(new strategy_simple::Strategy{m_extracts, m_options});
        }
    } else if (name == "complete_ways") {
        if (m_with_history) {
            return std::unique_ptr<ExtractStrategy>(new strategy_complete_ways_with_history::Strategy{m_extracts, m_options});
        } else {
            return std::unique_ptr<ExtractStrategy>(new strategy_complete_ways::Strategy{m_extracts, m_options});
        }
    } else if (name == "smart") {
        if (m_with_history) {
            throw argument_error{"The 'smart' strategy is not supported for history files"};
        } else {
            return std::unique_ptr<ExtractStrategy>(new strategy_smart::Strategy{m_extracts, m_options});
        }
    }

    throw argument_error{std::string{"Unknown extract strategy: '"} + name + "'"};
}

bool CommandExtract::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("config,c", po::value<std::string>(), "Config file")
    ("directory,d", po::value<std::string>(), "Output directory (default: from config)")
    ("fsync", "Call fsync after writing file")
    ("generator", po::value<std::string>(), "Generator setting for file header")
    ("option,S", po::value<std::vector<std::string>>(), "Set strategy option")
    ("output-header", po::value<std::vector<std::string>>(), "Add output header")
    ("overwrite,O", "Allow existing output files to be overwritten")
    ("strategy,s", po::value<std::string>()->default_value("complete_ways"), "Use named extract strategy")
    ("with-history", "Input file and output files are history files")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_progress(vm);
    setup_input_file(vm);

    if (vm.count("generator")) {
        m_generator = vm["generator"].as<std::string>();
    }

    if (vm.count("output-header")) {
        m_output_headers = vm["output-header"].as<std::vector<std::string>>();
    }

    if (vm.count("overwrite")) {
        m_output_overwrite = osmium::io::overwrite::allow;
    }

    if (vm.count("fsync")) {
        m_fsync = osmium::io::fsync::yes;
    }

    if (vm.count("directory")) {
        set_directory(vm["directory"].as<std::string>());
    }

    if (vm.count("config")) {
        m_config_file_name = vm["config"].as<std::string>();
        auto slash = m_config_file_name.find_last_of('/');
        if (slash != std::string::npos) {
            m_config_directory = m_config_file_name;
            m_config_directory.resize(slash + 1);
        }

        parse_config_file();
    }

    if (vm.count("option")) {
        for (const auto& option : vm["option"].as<std::vector<std::string>>()) {
            m_options.set(option);
        }
    }

    if (vm.count("with-history")) {
        m_with_history = true;
    }

    if (vm.count("strategy")) {
        m_strategy = make_strategy(vm["strategy"].as<std::string>());
    }

    return true;
}

void CommandExtract::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  output options:\n";
    m_vout << "    generator: " << m_generator << "\n";
    m_vout << "    overwrite: " << yes_no(m_output_overwrite == osmium::io::overwrite::allow);
    m_vout << "    fsync: " << yes_no(m_fsync == osmium::io::fsync::yes);
    if (!m_output_headers.empty()) {
        m_vout << "    output header:\n";
        for (const auto& h : m_output_headers) {
            m_vout << "      " << h << "\n";
        }
    }

    m_vout << "  strategy options:\n";
    m_vout << "    strategy: " << m_strategy->name() << '\n';
    m_vout << "    with history: " << yes_no(m_with_history);
    m_strategy->show_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    config file: " << m_config_file_name << '\n';
    m_vout << "    output directory: " << m_output_directory << '\n';

    m_vout << '\n';
    m_vout << "Extracts:\n";

    int n = 1;
    for (const auto& e : m_extracts) {
        const char old_fill = std::cerr.fill();
        m_vout << "[" << std::setw(2) << std::setfill('0') << n
               << "] Output:      " << e->output() << '\n';
        std::cerr.fill(old_fill);
        m_vout << "     Format:      " << e->output_format()    << '\n';
        m_vout << "     Description: " << e->description()      << '\n';
        m_vout << "     Envelope:    " << e->envelope_as_text() << '\n';
        m_vout << "     Type:        " << e->geometry_type()    << '\n';
        m_vout << "     Geometry:    " << e->geometry_as_text() << '\n';
        ++n;
    }

    m_vout << '\n';
}

bool CommandExtract::run() {
    osmium::io::Header header;
    setup_header(header);

    for (const auto& extract : m_extracts) {
        extract->open_file(header, m_output_overwrite, m_fsync);
    }

    m_strategy->run(m_vout, display_progress(), m_input_file);

    for (const auto& extract : m_extracts) {
        extract->close_file();
    }

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_extract_command = CommandFactory::add("extract", "Create geographic extract", []() {
        return new CommandExtract();
    });

}

