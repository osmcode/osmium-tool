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

#include <nlohmann/json.hpp>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <vector>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN // Prevent winsock.h inclusion; avoid winsock2.h conflict
#  endif
# include <io.h>
#endif

#ifndef _WIN32
# include <unistd.h>
#endif

namespace {

constexpr const std::size_t max_extracts = 500;

osmium::Box parse_bbox(const nlohmann::json& value) {
    if (value.is_array()) {
        if (value.size() != 4) {
            throw config_error{"'bbox' must be an array with exactly four elements."};
        }

        if (!value[0].is_number() || !value[1].is_number() || !value[2].is_number() || !value[3].is_number()) {
            throw config_error{"'bbox' array elements must be numbers."};
        }

        const auto value0 = value[0].template get<double>();
        const auto value1 = value[1].template get<double>();
        const auto value2 = value[2].template get<double>();
        const auto value3 = value[3].template get<double>();

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

    if (value.is_object()) {
        const auto left   = value.find("left");
        const auto right  = value.find("right");
        const auto top    = value.find("top");
        const auto bottom = value.find("bottom");

        if (left != value.end() && right  != value.end() &&
            top  != value.end() && bottom != value.end()) {
            if (left->is_number() && right->is_number() &&
                top->is_number()  && bottom->is_number()) {

                const auto left_value   = left->template get<double>();
                const auto right_value  = right->template get<double>();
                const auto top_value    = top->template get<double>();
                const auto bottom_value = bottom->template get<double>();

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

#ifdef _WIN32
bool is_valid_driver_char(const char value) noexcept {
    return ((value | 0x20) - 'a') <= ('z' - 'a');
}

bool is_path_rooted(const std::string& path) noexcept {
    const std::size_t len = path.size();

    return (len >= 1 && (path[0] == '\\' || path[0] == '/'))
        || (len >= 2 && is_valid_driver_char(path[0]) && path[1] == ':');
}
#endif

std::size_t parse_multipolygon_object(const std::string& directory, std::string file_name, std::string file_type, osmium::memory::Buffer* buffer) {
    assert(buffer);

    if (file_name.empty()) {
        throw config_error{"Missing 'file_name' in '(multi)polygon' object."};
    }

#ifdef _WIN32
    const bool is_relative = !is_path_rooted(file_name);
#else
    const bool is_relative = file_name[0] != '/';
#endif

    if (is_relative) {
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
            const std::string suffix{get_filename_suffix(file_name)};
            const osmium::io::File osmfile{"", suffix};
            if (osmfile.format() != osmium::io::file_format::unknown) {
                file_type = "osm";
            }
        }
    }

    if (file_type == "osm") {
        try {
            OSMFileParser parser{*buffer, file_name};
            return parser();
        } catch (const std::system_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        } catch (const osmium::io_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        } catch (const osmium::out_of_order_error& e) {
            throw osmium::io_error{std::string{"While reading file '"} + file_name + "':\n" + e.what()};
        }
    } else if (file_type == "geojson") {
        GeoJSONFileParser parser{*buffer, file_name};
        try {
            return parser();
        } catch (const config_error& e) {
            throw geojson_error{e.what()};
        }
    } else if (file_type == "poly") {
        PolyFileParser parser{*buffer, file_name};
        return parser();
    } else if (file_type.empty()) {
        throw config_error{"Could not autodetect file type in '(multi)polygon' object. Add a 'file_type'."};
    }

    throw config_error{std::string{"Unknown file type: '"} + file_type + "' in '(multi)polygon.file_type'"};
}

std::size_t parse_multipolygon_object(const std::string& directory, const nlohmann::json& value, osmium::memory::Buffer* buffer) {
    assert(buffer);

    const std::string file_name{get_value_as_string(value, "file_name")};
    const std::string file_type{get_value_as_string(value, "file_type")};
    return parse_multipolygon_object(directory, file_name, file_type, buffer);
}

std::size_t parse_polygon(const std::string& directory, const nlohmann::json& value, osmium::memory::Buffer* buffer) {
    assert(buffer);

    if (value.is_array()) {
        return parse_polygon_array(value, buffer);
    }

    if (value.is_object()) {
        return parse_multipolygon_object(directory, value, buffer);
    }

    throw config_error{"Polygon must be an object or array."};
}

std::size_t parse_multipolygon(const std::string& directory, const nlohmann::json& value, osmium::memory::Buffer* buffer) {
    assert(buffer);

    if (value.is_array()) {
        return parse_multipolygon_array(value, buffer);
    }

    if (value.is_object()) {
        return parse_multipolygon_object(directory, value, buffer);
    }

    throw config_error{"Multipolygon must be an object or array."};
}

bool is_existing_directory(const char* name) {
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

} // anonymous namespace

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
    nlohmann::json doc = nlohmann::json::parse(config_file);

    if (!doc.is_object()) {
        throw config_error{"Top-level value must be an object."};
    }

    const std::string directory{get_value_as_string(doc, "directory")};
    if (!directory.empty() && m_output_directory.empty()) {
        m_vout << "  Directory set to '" << directory << "'.\n";
        set_directory(directory);
    }

    const auto json_extracts = doc.find("extracts");
    if (json_extracts == doc.end()) {
        throw config_error{"Missing 'extracts' member in top-level object."};
    }

    if (!json_extracts->is_array()) {
        throw config_error{"'extracts' member in top-level object must be array."};
    }

    m_vout << "  Reading extracts from config file...\n";
    int extract_num = 1;
    for (const auto& item : *json_extracts) {
        std::string output;
        try {
            if (!item.is_object()) {
                throw config_error{"Members in 'extracts' array must be objects."};
            }

            output = get_value_as_string(item, "output");
            if (output.empty()) {
                throw config_error{"Missing 'output' field for extract."};
            }

            m_vout << "    Looking at extract '" << output << "'...\n";

            const std::string output_format{get_value_as_string(item, "output_format")};
            const std::string description{get_value_as_string(item, "description")};

            const auto json_bbox         = item.find("bbox");
            const auto json_polygon      = item.find("polygon");
            const auto json_multipolygon = item.find("multipolygon");

            osmium::io::File output_file{m_output_directory + output, output_format};
            if (m_with_history) {
                output_file.set_has_multiple_object_versions(true);
            } else if (output_file.has_multiple_object_versions()) {
                throw config_error{"Looks like you are trying to write a history file, but option --with-history is not set."};
            }

            if (json_bbox != item.end()) {
                m_extracts.push_back(std::make_unique<ExtractBBox>(output_file, description, parse_bbox(*json_bbox)));
            } else if (json_polygon != item.end()) {
                m_extracts.push_back(std::make_unique<ExtractPolygon>(output_file, description, m_buffer, parse_polygon(m_config_directory, *json_polygon, &m_buffer)));
            } else if (json_multipolygon != item.end()) {
                m_extracts.push_back(std::make_unique<ExtractPolygon>(output_file, description, m_buffer, parse_multipolygon(m_config_directory, *json_multipolygon, &m_buffer)));
            } else {
                throw config_error{"Missing geometry for extract. Need 'bbox', 'polygon', or 'multipolygon'."};
            }

            const auto json_output_header = item.find("output_header");
            if (json_output_header != item.end()) {
                if (!json_output_header->is_object()) {
                    throw config_error{"Optional 'output_header' field must be an object."};
                }
                Extract& extract = *m_extracts.back();
                for (auto const& header_item : json_output_header->items()) {
                    if (header_item.value().is_null()) {
                        extract.add_header_option(header_item.key());
                    } else {
                        if (!header_item.value().is_string()) {
                            throw config_error{"Values in 'output_header' object must be strings or null."};
                        }
                        extract.add_header_option(header_item.key(), header_item.value().template get<std::string>());
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
        return std::make_unique<strategy_simple::Strategy>(m_extracts, m_options);
    }

    if (name == "complete_ways") {
        if (m_with_history) {
            return std::make_unique<strategy_complete_ways_with_history::Strategy>(m_extracts, m_options);
        }
        return std::make_unique<strategy_complete_ways::Strategy>(m_extracts, m_options);
    }

    if (name == "smart") {
        if (m_with_history) {
            throw argument_error{"The 'smart' strategy is not supported for history files."};
        }
        return std::make_unique<strategy_smart::Strategy>(m_extracts, m_options);
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
    ("clean", po::value<std::vector<std::string>>(), "Clean attribute (version, changeset, timestamp, uid, user)")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

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

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);
    init_output_file(vm);

    m_clean.setup(vm);

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
        m_extracts.push_back(std::make_unique<ExtractBBox>(m_output_file, "", parse_bbox(vm["bbox"].as<std::string>(), "--box/-b")));
    }

    if (vm.count("polygon")) {
        if (vm.count("directory")) {
            warning("Ignoring --directory/-d option.\n");
        }
        check_output_file();
        if (m_with_history) {
            m_output_file.set_has_multiple_object_versions(true);
        }
        m_extracts.push_back(std::make_unique<ExtractPolygon>(m_output_file, "", m_buffer, parse_multipolygon_object("./", vm["polygon"].as<std::string>(), "", &m_buffer)));
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
    m_vout << "    attributes to clean: " << m_clean.to_string() << '\n';

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
        } catch (const nlohmann::json::parse_error &e) {
            throw geojson_error{std::string{"In file '"} + m_config_file_name +
                  "':\nJSON error at offset " + std::to_string(e.byte) +
                  " : " + e.what()};
        } catch (const config_error&) {
            std::cerr << "Error while reading config file '" << m_config_file_name << "':\n";
            throw;
        }
    }

    if (m_extracts.empty()) {
        throw config_error{"No extract specified in config file or on the command line."};
    }

    if (m_extracts.size() > max_extracts) {
        throw config_error{"Too many extracts specified in config file (Maximum: " + std::to_string(max_extracts) + ")."};
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
        extract->open_file(file_header, m_output_overwrite, m_fsync, &m_clean);
    }

    m_strategy->run(m_vout, display_progress(), m_input_file);

    for (const auto& extract : m_extracts) {
        extract->close_file();
    }

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

