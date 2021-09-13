/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2021  Jochen Topf <jochen@topf.org>

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

#include "command_extract.hpp"
#include "exception.hpp"

#include "extract/extract_bbox.hpp"
#include "extract/extract_polygon.hpp"
#include "extract/geojson_file_parser.hpp"
#include "extract/osm_file_parser.hpp"
#include "extract/poly_file_parser.hpp"
#include "extract/strategy_complete_ways.hpp"
#include "extract/strategy_complete_ways_with_history.hpp"
#include "extract/strategy_simple.hpp"
#include "extract/strategy_smart.hpp"
#include "util.hpp"

#include <osmium/geom/coordinates.hpp>
#include <osmium/handler/check_order.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/string.hpp>
#include <osmium/util/verbose_output.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN // Prevent winsock.h inclusion; avoid winsock2.h conflict
#  endif
# include <io.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif

static osmium::Box parse_bbox(const rapidjson::Value& value) {
    if (value.IsArray()) {
        if (value.Size() != 4) {
            throw config_error{"'bbox' must be an array with exactly four elements."};
        }

        if (!value[0].IsNumber() || !value[1].IsNumber() || !value[2].IsNumber() || !value[3].IsNumber()) {
            throw config_error{"'bbox' array elements must be numbers."};
        }

        const auto value0 = value[0].GetDouble();
        const auto value1 = value[1].GetDouble();
        const auto value2 = value[2].GetDouble();
        const auto value3 = value[3].GetDouble();

        if (value0 < -180.0 || value0 > 180.0) {
            throw config_error{"Invalid coordinate in bbox: " + std::to_string(value0) + "."};
        }

        if (value1 < -90.0 || value1 > 90.0) {
            throw config_error{"Invalid coordinate in bbox: " + std::to_string(value1) + "."};
        }

        if (value2 < -180.0 || value2 > 180.0) {
            throw config_error{"Invalid coordinate in bbox: " + std::to_string(value2) + "."};
        }

        if (value3 < -90.0 || value3 > 90.0) {
            throw config_error{"Invalid coordinate in bbox: " + std::to_string(value3) + "."};
        }

        const osmium::Location location1{value0, value1};
        const osmium::Location location2{value2, value3};

        osmium::Box box;
        box.extend(location1);
        box.extend(location2);

        return box;
    }

    if (value.IsObject()) {
        const auto left   = value.FindMember("left");
        const auto right  = value.FindMember("right");
        const auto top    = value.FindMember("top");
        const auto bottom = value.FindMember("bottom");

        if (left != value.MemberEnd() && right  != value.MemberEnd() &&
            top  != value.MemberEnd() && bottom != value.MemberEnd()) {
            if (left->value.IsNumber() && right->value.IsNumber() &&
                top->value.IsNumber()  && bottom->value.IsNumber()) {

                const auto left_value = left->value.GetDouble();
                const auto bottom_value = bottom->value.GetDouble();
                const auto right_value = right->value.GetDouble();
                const auto top_value = top->value.GetDouble();

                if (left_value < -180.0 || left_value > 180.0) {
                    throw config_error{"Invalid coordinate in bbox: " + std::to_string(left_value) + "."};
                }

                if (right_value < -180.0 || right_value > 180.0) {
                    throw config_error{"Invalid coordinate in bbox: " + std::to_string(right_value) + "."};
                }

                if (top_value < -90.0 || top_value > 90.0) {
                    throw config_error{"Invalid coordinate in bbox: " + std::to_string(top_value) + "."};
                }

                if (bottom_value < -90.0 || bottom_value > 90.0) {
                    throw config_error{"Invalid coordinate in bbox: " + std::to_string(bottom_value) + "."};
                }

                const osmium::Location bottom_left{left_value, bottom_value};
                const osmium::Location top_right{right_value, top_value};

                if (bottom_left.x() < top_right.x() &&
                    bottom_left.y() < top_right.y()) {
                    return osmium::Box{bottom_left, top_right};
                }

                throw config_error{"Need 'left' < 'right' and 'bottom' < 'top' in 'bbox' object."};
            }

            throw config_error{"Members in 'bbox' object must be numbers."};
        }

        throw config_error{"Need 'left', 'right', 'top', and 'bottom' members in 'bbox' object."};
    }

    throw config_error{"'bbox' member is not an array or object."};
}

static std::size_t parse_multipolygon_object(const std::string& directory, std::string file_name, std::string file_type, osmium::memory::Buffer& buffer) {
    if (file_name.empty()) {
        throw config_error{"Missing 'file_name' in '(multi)polygon' object."};
    }

    if (file_name[0] != '/') {
        // relative file name
        file_name = directory + file_name;
    }

    // If the file type is not set, try to deduce it from the file name
    // suffix.
    if (file_type.empty()) {
        if (ends_with(file_name, ".poly")) {
            file_type = "poly";
        } else if (ends_with(file_name, ".json") || ends_with(file_name, ".geojson")) {
            file_type = "geojson";
        } else {
            std::string suffix{get_filename_suffix(file_name)};
            osmium::io::File osmfile{"", suffix};
            if (osmfile.format() != osmium::io::file_format::unknown) {
                file_type = "osm";
            }
        }
    }

    if (file_type == "osm") {
        try {
            OSMFileParser parser{buffer, file_name};
            return parser();
        } catch (const std::system_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        } catch (const osmium::io_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        } catch (const osmium::out_of_order_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        }
    } else if (file_type == "geojson") {
        GeoJSONFileParser parser{buffer, file_name};
        try {
            return parser();
        } catch (const config_error& e) {
            throw geojson_error{e.what()};
        }
    } else if (file_type == "poly") {
        PolyFileParser parser{buffer, file_name};
        return parser();
    } else if (file_type.empty()) {
        throw config_error{"Could not autodetect file type in '(multi)polygon' object. Add a 'file_type'."};
    }

    throw config_error{std::string{"Unknown file type: '"} + file_type + "' in '(multi)polygon.file_type'"};
}

static std::size_t parse_multipolygon_object(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
    const std::string file_name{get_value_as_string(value, "file_name")};
    const std::string file_type{get_value_as_string(value, "file_type")};
    return parse_multipolygon_object(directory, file_name, file_type, buffer);
}

static std::size_t parse_polygon(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
    if (value.IsArray()) {
        return parse_polygon_array(value, buffer);
    }

    if (value.IsObject()) {
        return parse_multipolygon_object(directory, value, buffer);
    }

    throw config_error{"Polygon must be an object or array."};
}

std::size_t parse_multipolygon(const std::string& directory, const rapidjson::Value& value, osmium::memory::Buffer& buffer) {
    if (value.IsArray()) {
        return parse_multipolygon_array(value, buffer);
    }

    if (value.IsObject()) {
        return parse_multipolygon_object(directory, value, buffer);
    }

    throw config_error{"Multipolygon must be an object or array."};
}

static bool is_existing_directory(const char* name) {
#ifdef _MSC_VER
    // Windows implementation
    // https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
    struct _stat64 s{};
    if (::_stati64(name, &s) != 0) {
        return false;
    }
    return (s.st_mode & _S_IFDIR) != 0;
#else
    // Unix implementation
    struct stat s; // NOLINT clang-tidy
    if (::stat(name, &s) != 0) {
        return false;
    }
    return S_ISDIR(s.st_mode); // NOLINT(hicpp-signed-bitwise)
#endif
}

void CommandExtract::set_directory(const std::string& directory) {
    if (!is_existing_directory(directory.c_str())) {
        throw config_error{"Output directory is missing or not accessible: " + directory};
    }
    m_output_directory = directory;
    if (m_output_directory.empty() || m_output_directory.back() != '/') {
        m_output_directory += '/';
    }
}

void CommandExtract::parse_config_file() {
    std::ifstream config_file{m_config_file_name};
    rapidjson::IStreamWrapper stream_wrapper{(m_config_file_name == "-")? std::cin : config_file};

    rapidjson::Document doc;
    if (doc.ParseStream<(rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag)>(stream_wrapper).HasParseError()) {
        throw config_error{std::string{"JSON error at offset "} +
                           std::to_string(doc.GetErrorOffset()) +
                           ": " +
                           rapidjson::GetParseError_En(doc.GetParseError())
                          };
    }

    if (!doc.IsObject()) {
        throw config_error{"Top-level value must be an object."};
    }

    std::string directory{get_value_as_string(doc, "directory")};
    if (!directory.empty() && m_output_directory.empty()) {
        m_vout << "  Directory set to '" << directory << "'.\n";
        set_directory(directory);
    }

    const auto json_extracts = doc.FindMember("extracts");
    if (json_extracts == doc.MemberEnd()) {
        throw config_error{"Missing 'extracts' member in top-level object."};
    }

    if (!json_extracts->value.IsArray()) {
        throw config_error{"'extracts' member in top-level object must be array."};
    }

    m_vout << "  Reading extracts from config file...\n";
    int extract_num = 1;
    for (const auto& e : json_extracts->value.GetArray()) {
        std::string output;
        try {
            if (!e.IsObject()) {
                throw config_error{"Members in 'extracts' array must be objects."};
            }

            output = get_value_as_string(e, "output");
            if (output.empty()) {
                throw config_error{"Missing 'output' field for extract."};
            }

            m_vout << "    Looking at extract '" << output << "'...\n";

            std::string output_format{get_value_as_string(e, "output_format")};
            std::string description{get_value_as_string(e, "description")};

            const auto json_bbox         = e.FindMember("bbox");
            const auto json_polygon      = e.FindMember("polygon");
            const auto json_multipolygon = e.FindMember("multipolygon");

            osmium::io::File output_file{m_output_directory + output, output_format};
            if (m_with_history) {
                output_file.set_has_multiple_object_versions(true);
            } else if (output_file.has_multiple_object_versions()) {
                throw config_error{"Looks like you are trying to write a history file, but option --with-history is not set."};
            }

            if (json_bbox != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractBBox{output_file, description, parse_bbox(json_bbox->value)});
            } else if (json_polygon != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractPolygon{output_file, description, m_buffer, parse_polygon(m_config_directory, json_polygon->value, m_buffer)});
            } else if (json_multipolygon != e.MemberEnd()) {
                m_extracts.emplace_back(new ExtractPolygon{output_file, description, m_buffer, parse_multipolygon(m_config_directory, json_multipolygon->value, m_buffer)});
            } else {
                throw config_error{"Missing geometry for extract. Need 'bbox', 'polygon', or 'multipolygon'."};
            }

            Extract& extract = *m_extracts.back();
            const auto json_output_header = e.FindMember("output_header");
            if (json_output_header != e.MemberEnd()) {
                const auto& value = json_output_header->value;
                if (!value.IsObject()) {
                    throw config_error{"Optional 'output_header' field must be an object."};
                }
                for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
                    const auto& member_value = it->value;
                    if (member_value.IsNull()) {
                        extract.add_header_option(it->name.GetString());
                    } else {
                        if (!member_value.IsString()) {
                            throw config_error{"Values in 'output_header' object must be strings or null."};
                        }
                        extract.add_header_option(it->name.GetString(), member_value.GetString());
                    }
                }
            }
        } catch (const config_error& e) {
            std::string message{"In extract "};
            message += std::to_string(extract_num);
            message += ": ";
            message += e.what();
            throw config_error{message};
        } catch (const poly_error&) {
            std::cerr << "Error while reading poly file for extract " << extract_num << " (" << output << "):\n";
            throw;
        } catch (const geojson_error&) {
            std::cerr << "Error while reading GeoJSON file for extract " << extract_num << " (" << output << "):\n";
            throw;
        } catch (const std::system_error&) {
            std::cerr << "Error while reading OSM file for extract " << extract_num << " (" << output << "):\n";
            throw;
        } catch (const osmium::io_error&) {
            std::cerr << "Error while reading OSM file for extract " << extract_num << " (" << output << "):\n";
            throw;
        } catch (const osmium::out_of_order_error&) {
            std::cerr << "Error while reading OSM file for extract " << extract_num << " (" << output << "):\n";
            throw;
        }

        ++extract_num;
    }
    m_vout << '\n';
}

std::unique_ptr<ExtractStrategy> CommandExtract::make_strategy(const std::string& name) {
    if (name == "simple") {
        if (m_with_history) {
            throw argument_error{"The 'simple' strategy is not supported for history files."};
        }
        return std::unique_ptr<ExtractStrategy>(new strategy_simple::Strategy{m_extracts, m_options});
    }

    if (name == "complete_ways") {
        if (m_with_history) {
            return std::unique_ptr<ExtractStrategy>(new strategy_complete_ways_with_history::Strategy{m_extracts, m_options});
        }
        return std::unique_ptr<ExtractStrategy>(new strategy_complete_ways::Strategy{m_extracts, m_options});
    }

    if (name == "smart") {
        if (m_with_history) {
            throw argument_error{"The 'smart' strategy is not supported for history files."};
        }
        return std::unique_ptr<ExtractStrategy>(new strategy_smart::Strategy{m_extracts, m_options});
    }

    throw argument_error{std::string{"Unknown extract strategy: '"} + name + "'."};
}

bool CommandExtract::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("bbox,b", po::value<std::string>(), "Bounding box")
    ("config,c", po::value<std::string>(), "Config file")
    ("directory,d", po::value<std::string>(), "Output directory (default: from config)")
    ("option,S", po::value<std::vector<std::string>>(), "Set strategy option")
    ("polygon,p", po::value<std::string>(), "Polygon file")
    ("strategy,s", po::value<std::string>()->default_value("complete_ways"), "Use named extract strategy")
    ("with-history,H", "Input file and output files are history files")
    ("set-bounds", "Sets bounds (bounding box) in header")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};
    po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

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
    init_output_file(vm);

    if (vm.count("config") + vm.count("bbox") + vm.count("polygon") > 1) {
        throw argument_error{"Can only use one of --config/-c, --bbox/-b, or --polygon/-p."};
    }

    if (vm.count("with-history")) {
        m_with_history = true;
    }

    if (vm.count("config")) {
        if (vm.count("directory")) {
            set_directory(vm["directory"].as<std::string>());
        }
        if (vm.count("output")) {
            warning("Ignoring --output/-o option.\n");
        }
        if (vm.count("output-format")) {
            warning("Ignoring --output-format/-f option.\n");
        }
        m_config_file_name = vm["config"].as<std::string>();
        const auto slash = m_config_file_name.find_last_of('/');
        if (slash != std::string::npos) {
            m_config_directory = m_config_file_name;
            m_config_directory.resize(slash + 1);
        }
    }

    if (vm.count("bbox")) {
        if (vm.count("directory")) {
            warning("Ignoring --directory/-d option.\n");
        }
        check_output_file();
        if (m_with_history) {
            m_output_file.set_has_multiple_object_versions(true);
        }
        m_extracts.emplace_back(new ExtractBBox{m_output_file, "", parse_bbox(vm["bbox"].as<std::string>(), "--box/-b")});
    }

    if (vm.count("polygon")) {
        if (vm.count("directory")) {
            warning("Ignoring --directory/-d option.\n");
        }
        check_output_file();
        if (m_with_history) {
            m_output_file.set_has_multiple_object_versions(true);
        }
        m_extracts.emplace_back(new ExtractPolygon{m_output_file, "", m_buffer, parse_multipolygon_object("./", vm["polygon"].as<std::string>(), "", m_buffer)});
    }

    if (vm.count("option")) {
        for (const auto& option : vm["option"].as<std::vector<std::string>>()) {
            m_options.set(option);
        }
    }

    if (vm.count("set-bounds")) {
        m_set_bounds = true;
    }

    if (vm.count("strategy")) {
        m_strategy_name = vm["strategy"].as<std::string>();
    }

    return true;
}

void CommandExtract::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  strategy options:\n";
    m_vout << "    strategy: " << m_strategy_name << '\n';
    m_vout << "    with history: " << yes_no(m_with_history);

    m_vout << "  other options:\n";
    m_vout << "    config file: " << m_config_file_name << '\n';
    m_vout << "    output directory: " << m_output_directory << '\n';

    m_vout << '\n';
}

void CommandExtract::show_extracts() {
    m_vout << "Extracts:\n";

    int n = 1;
    for (const auto& e : m_extracts) {
        const char old_fill = std::cerr.fill();
        m_vout << "[" << std::setw(2) << std::setfill('0') << n
               << "] Output:      " << e->output() << '\n';
        std::cerr.fill(old_fill);
        m_vout << "     Format:      " << e->output_format()    << '\n';
        m_vout << "     Description: " << e->description()      << '\n';
        if (!e->header_options().empty()) {
            m_vout << "     Header opts: ";
            bool first = true;
            for (const auto& opt : e->header_options()) {
                if (first) {
                    first = false;
                } else {
                    m_vout << "                  ";
                }
                m_vout << opt << '\n';
            }
        }
        m_vout << "     Envelope:    " << e->envelope_as_text() << '\n';
        m_vout << "     Type:        " << e->geometry_type()    << '\n';
        m_vout << "     Geometry:    " << e->geometry_as_text() << '\n';
        ++n;
    }

    m_vout << '\n';
}

bool CommandExtract::run() {
    if (!m_config_file_name.empty()) {
        m_vout << "Reading config file...\n";
        try {
            parse_config_file();
        } catch (const config_error&) {
            std::cerr << "Error while reading config file '" << m_config_file_name << "':\n";
            throw;
        }
    }

    if (m_extracts.empty()) {
        throw config_error{"No extract specified in config file or on the command line."};
    }

    show_extracts();

    m_strategy = make_strategy(m_strategy_name);
    m_strategy->show_arguments(m_vout);

    osmium::io::Header header;
    osmium::io::Header input_header;
    if (m_input_file.filename().empty()) {
        setup_header(header);
    } else {
        osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::nothing};
        input_header = reader.header();
        setup_header(header, input_header);
        reader.close();
    }
    header.set("sorting", "Type_then_ID");
    if (m_with_history) {
        header.set_has_multiple_object_versions(true);
    }

    for (const auto& extract : m_extracts) {
        osmium::io::Header file_header{header};
        if (m_set_bounds) {
            file_header.add_box(extract->envelope());
        }
        init_header(file_header, input_header, extract->header_options());
        extract->open_file(file_header, m_output_overwrite, m_fsync);
    }

    m_strategy->run(m_vout, display_progress(), m_input_file);

    for (const auto& extract : m_extracts) {
        extract->close_file();
    }

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

