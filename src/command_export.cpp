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

#include "command_export.hpp"

#include "exception.hpp"
#include "util.hpp"

#include "export/export_format_json.hpp"
#include "export/export_format_pg.hpp"
#include "export/export_format_text.hpp"
#include "export/export_handler.hpp"

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/handler/check_order.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/relations/manager_util.hpp>
#include <osmium/util/string.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <nlohmann/json.hpp>

#include <boost/program_options.hpp>

#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

std::string get_attr_string(const nlohmann::json& object, const char* key) {
    const auto it = object.find(key);
    if (it == object.end()) {
        return {};
    }

    if (it->is_string()) {
        return it->template get<std::string>();
    }

    if (it->is_boolean() && it->template get<bool>()) {
        return std::string{"@"} + key;
    }

    return {};
}

Ruleset parse_tags_ruleset(const nlohmann::json& object, const char* key) {
    Ruleset ruleset;

    const auto json = object.find(key);
    if (json == object.end() || json->is_null()) {
        // When this is not set, the default is "other". This is later
        // changed to "any" if both linear_tags and area_tags are missing.
        ruleset.set_rule_type(tags_filter_rule_type::other);
        return ruleset;
    }

    if (json->is_boolean()) {
        if (json->template get<bool>()) {
            ruleset.set_rule_type(tags_filter_rule_type::any);
        } else {
            ruleset.set_rule_type(tags_filter_rule_type::none);
        }
        return ruleset;
    }

    if (!json->is_array()) {
        throw config_error{std::string{"'"} + key + "' member in top-level object must be false, true, null, or an array."};
    }

    if (json->empty()) {
        std::cerr << "Warning! An empty array for 'linear_tags' or 'area_tags' matches any tags.\n"
                  << "         Please use 'true' instead of the array.\n";
        ruleset.set_rule_type(tags_filter_rule_type::any);
        return ruleset;
    }

    ruleset.set_rule_type(tags_filter_rule_type::list);

    for (const auto& value : *json) {
        if (!value.is_string()) {
            throw config_error{std::string{"Array elements in '"} + key + "' must be strings."};
        }

        const auto str = value.template get<std::string>();
        if (!str.empty()) {
            ruleset.add_rule(str);
        }
    }

    return ruleset;
}

bool parse_string_array(const nlohmann::json& object, const char* key, std::vector<std::string>* result) {
    const auto json = object.find(key);
    if (json == object.end()) {
        return false;
    }

    if (!json->is_array()) {
        throw config_error{std::string{"'"} + key + "' member in top-level object must be array."};
    }

    for (const auto& value : *json) {
        if (!value.is_string()) {
            throw config_error{std::string{"Array elements in '"} + key + "' must be strings."};
        }

        const auto str = value.template get<std::string>();
        if (!str.empty()) {
            result->emplace_back(str);
        }
    }

    return true;
}

} // anonymous namespace

void CommandExport::parse_attributes(const nlohmann::json& attributes) {
    if (!attributes.is_object()) {
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

void CommandExport::parse_format_options(const nlohmann::json& options) {
    if (!options.is_object()) {
        throw config_error{"'format_options' member must be an object."};
    }
    for (const auto &item : options.items()) {
        const auto type = item.value().type();
        const auto &key = item.key();
        switch (type) {
            case nlohmann::json::value_t::null:
                m_options.format_options.set(key, false);
                break;
            case nlohmann::json::value_t::boolean:
                m_options.format_options.set(key, item.value().template get<bool>());
                break;
            case nlohmann::json::value_t::object:
                throw config_error{"Option value for key '" + std::string(key) + "' can not be of type object."};
            case nlohmann::json::value_t::array:
                throw config_error{"Option value for key '" + std::string(key) + "' can not be an array."};
                break;
            case nlohmann::json::value_t::string:
                m_options.format_options.set(key, item.value().template get<std::string>());
                break;
            case nlohmann::json::value_t::number_integer:
                m_options.format_options.set(key, std::to_string(item.value().template get<int64_t>()));
                break;
            case nlohmann::json::value_t::number_unsigned:
                m_options.format_options.set(key, std::to_string(item.value().template get<uint64_t>()));
                break;
            case nlohmann::json::value_t::number_float:
                m_options.format_options.set(key, std::to_string(item.value().template get<double>()));
                break;
            default:
                throw config_error{"Unknown type"};
        }
    }
}

void CommandExport::parse_config_file() {
    std::ifstream config_file{m_config_file_name};
    nlohmann::json doc = nlohmann::json::parse(config_file);

    if (!doc.is_object()) {
        throw config_error{"Top-level value must be an object."};
    }

    const auto json_attr = doc.find("attributes");
    if (json_attr != doc.end()) {
        parse_attributes(*json_attr);
    }

    const auto json_opts = doc.find("format_options");
    if (json_opts != doc.end()) {
        parse_format_options(*json_opts);
    }

    m_linear_ruleset = parse_tags_ruleset(doc, "linear_tags");
    m_area_ruleset   = parse_tags_ruleset(doc, "area_tags");

    if (m_linear_ruleset.rule_type() == tags_filter_rule_type::other &&
        m_area_ruleset.rule_type() == tags_filter_rule_type::other) {
        m_linear_ruleset.set_rule_type(tags_filter_rule_type::any);
        m_area_ruleset.set_rule_type(tags_filter_rule_type::any);
    }

    parse_string_array(doc, "include_tags", &m_include_tags);
    parse_string_array(doc, "exclude_tags", &m_exclude_tags);
}

void CommandExport::canonicalize_output_format() {
    for (auto& c : m_output_format) {
        c = static_cast<char>(std::tolower(c));
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
    const std::string default_index_type{"flex_mem"};

    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("add-unique-id,u", po::value<std::string>(), "Add unique id to each feature ('counter' or 'type_id')")
    ("config,c", po::value<std::string>(), "Config file")
    ("format-option,x", po::value<std::vector<std::string>>(), "Output format options")
    ("fsync", "Call fsync after writing file")
    ("geometry-types", po::value<std::string>(), "Geometry types that should be written (default: 'point,linestring,polygon')")
    ("index-type,i", po::value<std::string>()->default_value(default_index_type), "Index type to use")
    ("keep-untagged,n", "Keep features that don't have any tags")
    ("output,o", po::value<std::string>(), "Output file (default: STDOUT)")
    ("output-format,f", po::value<std::string>(), "Output format (default depends on output file suffix)")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ("print-default-config,C", "Print default config on STDOUT")
    ("show-errors,e", "Output any geometry errors on STDOUT")
    ("stop-on-error,E", "Stop on the first error encountered")
    ("show-index-types,I", "Show available index types")
    ("attributes,a", po::value<std::string>(), "Comma-separated list of attributes to add to the output (default: none)")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};

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

    if (vm.count("print-default-config")) {
        std::cout << R"({
    "attributes": {
        "type":      false,
        "id":        false,
        "version":   false,
        "changeset": false,
        "timestamp": false,
        "uid":       false,
        "user":      false,
        "way_nodes": false
    },
    "format_options": {
    },
    "linear_tags":  true,
    "area_tags":    true,
    "exclude_tags": [],
    "include_tags": []
}
)";
        return false;
    }

    if (vm.count("show-index-types")) {
        const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
        for (const auto& map_type : map_factory.map_types()) {
            std::cout << map_type << '\n';
        }
        std::cout << "none\n";
        return false;
    }

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);

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

    if (m_output_format != "geojson" &&
        m_output_format != "geojsonseq" &&
        m_output_format != "pg" &&
        m_output_format != "text") {
        throw argument_error{"Set output format with --output-format or -f to 'geojson', 'geojsonseq', 'pg', or 'text'."};
    }

    // Set defaults for output format options depending on output format
    if (m_output_format == "geojsonseq") {
        m_options.format_options.set("print_record_separator", true);
    }
    if (m_output_format == "pg") {
        m_options.format_options.set("tags_type", "json");
    }

    if (vm.count("config")) {
        m_config_file_name = vm["config"].as<std::string>();

        try {
            parse_config_file();
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Error while reading config file '" << m_config_file_name << "':\n";
            throw config_error{std::string{"JSON error at offset "} +
                               std::to_string(e.byte) + ": " + e.what()};
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

    if (vm.count("geometry-types")) {
        m_geometry_types.clear();
        const auto types = osmium::split_string(vm["geometry-types"].as<std::string>(), ',');
        for (const auto& type : types) {
            if (type == "point") {
                m_geometry_types.point = true;
            } else if (type == "linestring") {
                m_geometry_types.linestring = true;
            } else if (type == "polygon") {
                m_geometry_types.polygon = true;
            } else if (type == "multipolygon") {
                m_geometry_types.polygon = true;
            } else {
                throw argument_error{"Unknown geometry type in --geometry-types option: " + type + "."};
            }
        }
        if (m_geometry_types.empty()) {
            throw argument_error{"No geometry types in --geometry-types option."};
        }
    }

    if (vm.count("attributes")) {
        const auto attrs = osmium::split_string(vm["attributes"].as<std::string>(), ',');
        for (const auto& attr : attrs) {
            if (attr == "type") {
                m_options.type = "@type";
            } else if (attr == "id") {
                m_options.id = "@id";
            } else if (attr == "version") {
                m_options.version = "@version";
            } else if (attr == "changeset") {
                m_options.changeset = "@changeset";
            } else if (attr == "timestamp") {
                m_options.timestamp = "@timestamp";
            } else if (attr == "uid") {
                m_options.uid = "@uid";
            } else if (attr == "user") {
                m_options.user = "@user";
            } else if (attr == "way_nodes") {
                m_options.way_nodes = "@way_nodes";
            } else {
                throw argument_error{"Unknown attribute in --attributes option: " + attr + "."};
            }
        }
    }

    if (vm.count("index-type")) {
        m_index_type_name = check_index_type(vm["index-type"].as<std::string>(), true);
    }

    if (vm.count("keep-untagged")) {
        m_options.keep_untagged = true;
    }

    if (vm.count("overwrite")) {
        m_output_overwrite = osmium::io::overwrite::allow;
    }

    if (vm.count("format-option")) {
        for (const auto& str : vm["format-option"].as<std::vector<std::string>>()) {
            m_options.format_options.set(str);
        }
    }

    if (vm.count("show-errors")) {
        m_show_errors = true;
    }

    if (vm.count("stop-on-error")) {
        m_show_errors = true;
        m_stop_on_error = true;
    }

    if (!m_include_tags.empty() && !m_exclude_tags.empty()) {
        throw config_error{"Setting both 'include_tags' and 'exclude_tags' is not allowed."};
    }

    if (!m_include_tags.empty()) {
        initialize_tags_filter(m_options.tags_filter, false, m_include_tags);
    } else if (!m_exclude_tags.empty()) {
        initialize_tags_filter(m_options.tags_filter, true, m_exclude_tags);
    }

    if (m_input_file.filename().empty()) {
        throw config_error{"Can not read from STDIN, because input file has to be read twice."};
    }

    return true;
}

namespace {

void print_taglist(osmium::VerboseOutput* vout, const std::vector<std::string>& strings) {
    assert(vout);
    for (const auto& str : strings) {
        *vout << "    " << str << '\n';
    }
}

void print_ruleset(osmium::VerboseOutput* vout, const Ruleset& ruleset) {
    assert(vout);
    switch (ruleset.rule_type()) {
        case tags_filter_rule_type::none:
            *vout << "none\n";
            break;
        case tags_filter_rule_type::any:
            *vout << "any\n";
            break;
        case tags_filter_rule_type::list:
            *vout << "one of the following:\n";
            print_taglist(vout, ruleset.tags());
            break;
        case tags_filter_rule_type::other:
            *vout << "if other tag list doesn't match\n";
            break;
    }
}

const char* print_unique_id_type(unique_id_type unique_id) {
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

std::unique_ptr<ExportFormat> create_handler(const std::string& output_format,
                                             const std::string& output_filename,
                                             osmium::io::overwrite overwrite,
                                             osmium::io::fsync fsync,
                                             const options_type& options) {
    if (output_format == "geojson" || output_format == "geojsonseq") {
        return std::make_unique<ExportFormatJSON>(output_format, output_filename, overwrite, fsync, options);
    }

    if (output_format == "pg") {
        return std::make_unique<ExportFormatPg>(output_format, output_filename, overwrite, fsync, options);
    }

    if (output_format == "text") {
        return std::make_unique<ExportFormatText>(output_format, output_filename, overwrite, fsync, options);
    }

    throw argument_error{"Unknown output format"};
}

} // anonymous namespace

void CommandExport::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  output options:\n";
    m_vout << "    file name: " << m_output_filename << '\n';
    m_vout << "    file format: " << m_output_format << '\n';
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

    if (!m_options.format_options.empty()) {
        m_vout << "  output format options:\n";
        for (const auto& option : m_options.format_options) {
            m_vout << "    " << option.first << " = " << option.second << '\n';
        }
    }

    m_vout << "  linear tags: ";
    print_ruleset(&m_vout, m_linear_ruleset);
    m_vout << "  area tags:   ";
    print_ruleset(&m_vout, m_area_ruleset);

    if (!m_include_tags.empty()) {
        m_vout << "  include only these tags:\n";
        print_taglist(&m_vout, m_include_tags);
    } else if (!m_exclude_tags.empty()) {
        m_vout << "  exclude these tags:\n";
        print_taglist(&m_vout, m_exclude_tags);
    }

    m_vout << "  other options:\n";
    m_vout << "    index type: " << m_index_type_name << '\n';
    m_vout << "    add unique IDs: " << print_unique_id_type(m_options.unique_id) << '\n';
    m_vout << "    keep untagged features: " << yes_no(m_options.keep_untagged);
}

bool CommandExport::run() {
    auto handler = create_handler(m_output_format, m_output_filename, m_output_overwrite, m_fsync, m_options);
    if (m_vout.verbose()) {
        handler->debug_output(m_vout, m_output_filename);
    }

    const osmium::area::Assembler::config_type assembler_config;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{assembler_config};

    m_vout << "First pass (of two) through input file (reading relations)...\n";
    osmium::relations::read_relations(m_input_file, mp_manager);
    m_vout << "First pass done.\n";

    m_vout << "Second pass (of two) through input file...\n";
    m_linear_ruleset.init_filter();
    m_area_ruleset.init_filter();

    ExportHandler export_handler{std::move(handler), m_linear_ruleset, m_area_ruleset, m_geometry_types, m_show_errors, m_stop_on_error};
    osmium::handler::CheckOrder check_order_handler;

    if (m_index_type_name == "none") {
        osmium::io::ReaderWithProgressBar reader{display_progress(), m_input_file};
        osmium::apply(reader, check_order_handler, export_handler, mp_manager.handler([&export_handler](const osmium::memory::Buffer& buffer) {
            osmium::apply(buffer, export_handler);
        }));
        reader.close();
    } else {
        const auto& map_factory = osmium::index::MapFactory<osmium::unsigned_object_id_type, osmium::Location>::instance();
        auto location_index_pos = map_factory.create_map(m_index_type_name);
        auto location_index_neg = map_factory.create_map(m_index_type_name);
        location_handler_type location_handler{*location_index_pos, *location_index_neg};
        if (!m_stop_on_error) {
            location_handler.ignore_errors();
        }

        osmium::io::ReaderWithProgressBar reader{display_progress(), m_input_filename};
        osmium::apply(reader, check_order_handler, location_handler, export_handler, mp_manager.handler([&export_handler](const osmium::memory::Buffer& buffer) {
            osmium::apply(buffer, export_handler);
        }));
        reader.close();
        m_vout << "About "
               << show_mbytes(location_index_pos->used_memory() + location_index_neg->used_memory())
               << " MBytes used for node location index (in main memory or on disk).\n";
    }

    if (m_stop_on_error) {
        const auto incomplete_relations = mp_manager.relations_database().count_relations();
        if (incomplete_relations > 0) {
            throw osmium::geometry_error{"Found " + std::to_string(incomplete_relations) + " incomplete relation(s)"};
        }
    }

    m_vout << "Second pass done.\n";
    export_handler.close();

    m_vout << "Wrote " << export_handler.count() << " features.\n";
    m_vout << "Encountered " << export_handler.error_count() << " errors.\n";

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

