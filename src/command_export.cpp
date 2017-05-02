/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

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

#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <osmium/handler/check_order.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/string.hpp>
#include <osmium/util/verbose_output.hpp>

#include "command_export.hpp"
#include "exception.hpp"
#include "util.hpp"

#include "export/export_handler.hpp"
#include "export/export_format_json.hpp"
#include "export/export_format_text.hpp"

static std::string get_attr_string(const rapidjson::Value& object, const char* key) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd()) {
        return "";
    }

    if (it->value.IsString()) {
        return it->value.GetString();
    }

    if (it->value.IsBool()) {
        if (it->value.GetBool()) {
            return std::string{"@"} + key;
        }
    }

    return "";
}

void CommandExport::parse_options(const rapidjson::Value& attributes) {
    if (!attributes.IsObject()) {
        throw config_error{"'attributes' member must be an object."};
    }

    m_options.type      = get_attr_string(attributes, "type");
    m_options.id        = get_attr_string(attributes, "id");
    m_options.version   = get_attr_string(attributes, "version");
    m_options.changeset = get_attr_string(attributes, "changeset");
    m_options.timestamp = get_attr_string(attributes, "timestamp");
    m_options.uid       = get_attr_string(attributes, "uid");
    m_options.user      = get_attr_string(attributes, "user");
    m_options.way_nodes = get_attr_string(attributes, "way_nodes");
}

static bool parse_string_array(const rapidjson::Value& object, const char* key, std::vector<std::string>& result) {
    const auto json = object.FindMember(key);
    if (json == object.MemberEnd()) {
        return false;
    }

    if (!json->value.IsArray()) {
        throw config_error{std::string{"'"} + key + "' member in top-level object must be array."};
    }

    for (const auto& value : json->value.GetArray()) {
        if (!value.IsString()) {
            throw config_error{std::string{"Array elements in '"} + key + "' must be strings."};
        }

        if (value.GetString()[0] != '\0') {
            result.emplace_back(value.GetString());
        }
    }

    return true;
}

void CommandExport::parse_config_file() {
    std::ifstream config_file{m_config_file_name};
    rapidjson::IStreamWrapper stream_wrapper{config_file};

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

    const auto json_attr = doc.FindMember("attributes");
    if (json_attr != doc.MemberEnd()) {
        parse_options(json_attr->value);
    }

    parse_string_array(doc, "linear_tags", m_linear_tags);
    parse_string_array(doc, "area_tags", m_area_tags);

    parse_string_array(doc, "include_tags", m_include_tags);
    parse_string_array(doc, "exclude_tags", m_exclude_tags);
}

void CommandExport::canonicalize_output_format() {
    for (auto& c : m_output_format) {
        c = std::tolower(c);
    }

    if (m_output_format == "json") {
        m_output_format = "geojson";
        return;
    }

    if (m_output_format == "jsonseq") {
        m_output_format = "geojsonseq";
        return;
    }

    if (m_output_format == "txt") {
        m_output_format = "text";
        return;
    }
}

bool CommandExport::setup(const std::vector<std::string>& arguments) {
    const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
    std::string default_index_type{map_factory.has_map_type("sparse_mmap_array") ? "sparse_mmap_array" : "sparse_mem_array"};

    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("add-unique-id,u", po::value<std::string>(), "Add unique id to each feature ('counter' or 'type_id')")
    ("config,c", po::value<std::string>(), "Config file")
    ("fsync", "Call fsync after writing file")
    ("index-type,i", po::value<std::string>()->default_value(default_index_type), "Index type to use")
    ("keep-untagged,n", "Keep features that don't have any tags")
    ("output,o", po::value<std::string>(), "Output file (default: STDOUT)")
    ("output-format,f", po::value<std::string>(), "Output format (default depends on output file suffix)")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ("show-errors,e", "Output any geometry errors on STDOUT")
    ("show-index-types,I", "Show available index types")
    ("omit-rs,r", "Do not print RS (record separator) character when using JSON Text Sequences")
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

    if (vm.count("show-index-types")) {
        for (const auto& map_type : map_factory.map_types()) {
            std::cout << map_type << '\n';
        }
        std::cout << "none\n";
        return false;
    }

    setup_common(vm, desc);
    setup_input_file(vm);

    if (vm.count("config")) {
        m_config_file_name = vm["config"].as<std::string>();

        try {
            parse_config_file();
        } catch (const config_error&) {
            std::cerr << "Error while reading config file '" << m_config_file_name << "':\n";
            throw;
        }
    }

    if (vm.count("add-unique-id")) {
        const std::string value = vm["add-unique-id"].as<std::string>();
        if (value == "counter") {
            m_options.unique_id = unique_id_type::counter;
        } else if (value == "type_id") {
            m_options.unique_id = unique_id_type::type_id;
        } else {
            throw argument_error{"Unknown --add-unique-id, -u setting. Use 'counter' or 'type_id'."};
        }
    }

    if (vm.count("fsync")) {
        m_fsync = osmium::io::fsync::yes;
    }

    if (vm.count("index-type")) {
        m_index_type_name = vm["index-type"].as<std::string>();
        if (m_index_type_name != "none" && !map_factory.has_map_type(m_index_type_name)) {
            throw argument_error{std::string{"Unknown index type '"} + m_index_type_name + "'. Use --show-index-types or -I to get a list."};
        }
    }

    if (vm.count("keep-untagged")) {
        m_options.keep_untagged = true;
    }

    if (vm.count("output")) {
        m_output_filename = vm["output"].as<std::string>();

        const auto pos = m_output_filename.rfind('.');
        if (pos != std::string::npos) {
            m_output_format = m_output_filename.substr(pos + 1);
        }
    } else {
        m_output_filename = "-";
    }

    if (vm.count("output-format")) {
        m_output_format = vm["output-format"].as<std::string>();
    }

    canonicalize_output_format();

    if (m_output_format != "geojson" && m_output_format != "geojsonseq" && m_output_format != "text") {
        throw argument_error{"Set output format with --output-format or -f to 'geojson', 'geojsonseq', or 'text'."};
    }

    if (vm.count("overwrite")) {
        m_output_overwrite = osmium::io::overwrite::allow;
    }

    if (vm.count("omit-rs")) {
        m_options.print_record_separator = false;
        if (m_output_format != "geojsonseq") {
            warning("The --omit-rs/-r option only works for GeoJSON Text Sequence (geojsonseq) format. Ignored.\n");
        }
    }

    if (vm.count("show-errors")) {
        m_show_errors = true;
    }

    if (!m_include_tags.empty() && !m_exclude_tags.empty()) {
        throw config_error{"Setting both 'include_tags' and 'exclude_tags' is not allowed."};
    }

    if (!m_include_tags.empty()) {
        initialize_tags_filter(m_options.tags_filter, false, m_include_tags);
    } else if (!m_exclude_tags.empty()) {
        initialize_tags_filter(m_options.tags_filter, true, m_exclude_tags);
    }

    if (m_output_filename == "-" && m_show_errors) {
        throw argument_error{"Can't use --show-errors/-e when writing to STDOUT."};
    }

    return true;
}

static void print_taglist(osmium::util::VerboseOutput& vout, const std::vector<std::string>& strings) {
    for (const auto& str : strings) {
        vout << "    " << str << '\n';
    }
}

static const char* print_unique_id_type(unique_id_type unique_id) {
    switch (unique_id) {
        case unique_id_type::counter:
            return "counter";
        case unique_id_type::type_id:
            return "type and id";
        default:
            break;
    }

    return "no";
}

void CommandExport::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  output options:\n";
    m_vout << "    file name: " << m_output_filename << '\n';

    if (m_output_format == "geojsonseq") {
        m_vout << "    file format: geojsonseq (with" << (m_options.print_record_separator ? " RS)\n" : "out RS)\n");
    } else {
        m_vout << "    file format: " << m_output_format << '\n';
    }
    m_vout << "    overwrite: " << yes_no(m_output_overwrite == osmium::io::overwrite::allow);
    m_vout << "    fsync: " << yes_no(m_fsync == osmium::io::fsync::yes);
    m_vout << "  attributes:\n";
    m_vout << "    type:      " << (m_options.type.empty()      ? "(omitted)" : m_options.type)      << '\n';
    m_vout << "    id:        " << (m_options.id.empty()        ? "(omitted)" : m_options.id)        << '\n';
    m_vout << "    version:   " << (m_options.version.empty()   ? "(omitted)" : m_options.version)   << '\n';
    m_vout << "    changeset: " << (m_options.changeset.empty() ? "(omitted)" : m_options.changeset) << '\n';
    m_vout << "    timestamp: " << (m_options.timestamp.empty() ? "(omitted)" : m_options.timestamp) << '\n';
    m_vout << "    uid:       " << (m_options.uid.empty()       ? "(omitted)" : m_options.uid)       << '\n';
    m_vout << "    user:      " << (m_options.user.empty()      ? "(omitted)" : m_options.user)      << '\n';
    m_vout << "    way_nodes: " << (m_options.way_nodes.empty() ? "(omitted)" : m_options.way_nodes) << '\n';

    m_vout << "  linear tags:\n";
    print_taglist(m_vout, m_linear_tags);
    m_vout << "  area tags:\n";
    print_taglist(m_vout, m_area_tags);

    if (!m_include_tags.empty()) {
        m_vout << "  include only these tags:\n";
        print_taglist(m_vout, m_include_tags);
    } else if (!m_exclude_tags.empty()) {
        m_vout << "  exclude these tags:\n";
        print_taglist(m_vout, m_exclude_tags);
    }

    m_vout << "  other options:\n";
    m_vout << "    index type: " << m_index_type_name << '\n';
    m_vout << "    add unique IDs: " << print_unique_id_type(m_options.unique_id) << '\n';
    m_vout << "    keep untagged features: " << yes_no(m_options.keep_untagged);
}

static std::unique_ptr<ExportFormat> create_handler(const std::string& output_format,
                                                    const std::string& output_filename,
                                                    osmium::io::overwrite overwrite,
                                                    osmium::io::fsync fsync,
                                                    const options_type& options) {
    if (output_format == "geojson" || output_format == "geojsonseq") {
        return std::unique_ptr<ExportFormat>{new ExportFormatJSON{output_format, output_filename, overwrite, fsync, options}};
    }

    if (output_format == "text") {
        return std::unique_ptr<ExportFormat>{new ExportFormatText{output_format, output_filename, overwrite, fsync, options}};
    }

    throw argument_error{"Unknown output format"};
}

bool CommandExport::run() {
    osmium::area::Assembler::config_type assembler_config;
    osmium::area::MultipolygonCollector<osmium::area::Assembler> collector{assembler_config};

    m_vout << "First pass through input file (reading relations)...\n";
    {
        osmium::io::Reader reader{m_input_filename, osmium::osm_entity_bits::relation};
        collector.read_relations(reader);
        reader.close();
    }
    m_vout << "First pass done.\n";

    m_vout << "Second pass through input file...\n";

    auto handler = create_handler(m_output_format, m_output_filename, m_output_overwrite, m_fsync, m_options);
    ExportHandler export_handler{std::move(handler), m_linear_tags, m_area_tags, m_show_errors};
    osmium::handler::CheckOrder check_order_handler;

    if (m_index_type_name == "none") {
        osmium::io::Reader reader{m_input_filename};
        osmium::apply(reader, check_order_handler, export_handler, collector.handler([&export_handler](osmium::memory::Buffer&& buffer) {
            osmium::apply(buffer, export_handler);
        }));
        reader.close();
    } else {
        const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
        std::unique_ptr<index_type> index = map_factory.create_map(m_index_type_name);
        location_handler_type location_handler{*index};
        location_handler.ignore_errors();

        osmium::io::Reader reader{m_input_filename};
        osmium::apply(reader, check_order_handler, location_handler, export_handler, collector.handler([&export_handler](osmium::memory::Buffer&& buffer) {
            osmium::apply(buffer, export_handler);
        }));
        reader.close();
    }
    m_vout << "Second pass done.\n";
    export_handler.close();

    m_vout << "Wrote " << export_handler.count() << " features.\n";

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

