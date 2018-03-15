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

#include "command_fileinfo.hpp"
#include "exception.hpp"
#include "util.hpp"

#include <osmium/handler.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/crc.hpp>
#include <osmium/osm/metadata_options.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/minmax.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#pragma GCC diagnostic pop

#include <boost/crc.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef _MSC_VER
# include <unistd.h>
#endif

/*************************************************************************/

struct InfoHandler : public osmium::handler::Handler {

    osmium::Box bounds;

    uint64_t changesets = 0;
    uint64_t nodes      = 0;
    uint64_t ways       = 0;
    uint64_t relations  = 0;

    osmium::max_op<osmium::object_id_type> largest_changeset_id{0};
    osmium::max_op<osmium::object_id_type> largest_node_id{0};
    osmium::max_op<osmium::object_id_type> largest_way_id{0};
    osmium::max_op<osmium::object_id_type> largest_relation_id{0};
    osmium::metadata_options metadata_all_objects{"all"};
    osmium::metadata_options metadata_some_objects{"none"};

    osmium::min_op<osmium::Timestamp> first_timestamp;
    osmium::max_op<osmium::Timestamp> last_timestamp;

    osmium::CRC<boost::crc_32_type> crc32;

    bool ordered = true;
    bool multiple_versions = false;

    osmium::item_type last_type = osmium::item_type::undefined;
    osmium::object_id_type last_id = 0;

    void changeset(const osmium::Changeset& changeset) {
        if (last_type == osmium::item_type::changeset) {
            if (last_id > changeset.id()) {
                ordered = false;
            }
        } else {
            last_type = osmium::item_type::changeset;
        }

        last_id = changeset.id();
        crc32.update(changeset);
        ++changesets;

        largest_changeset_id.update(changeset.id());
    }

    void osm_object(const osmium::OSMObject& object) {
        first_timestamp.update(object.timestamp());
        last_timestamp.update(object.timestamp());

        metadata_all_objects &= osmium::detect_available_metadata(object);
        metadata_some_objects |= osmium::detect_available_metadata(object);

        if (last_type == object.type()) {
            if (last_id == object.id()) {
                multiple_versions = true;
            }
            if (osmium::id_order{}(object.id(), last_id)) {
                ordered = false;
            }
        } else if (last_type != osmium::item_type::changeset && last_type > object.type()) {
            ordered = false;
        }

        last_type = object.type();
        last_id = object.id();
    }

    void node(const osmium::Node& node) {
        crc32.update(node);
        bounds.extend(node.location());
        ++nodes;

        largest_node_id.update(node.id());
    }

    void way(const osmium::Way& way) {
        crc32.update(way);
        ++ways;

        largest_way_id.update(way.id());
    }

    void relation(const osmium::Relation& relation) {
        crc32.update(relation);
        ++relations;

        largest_relation_id.update(relation.id());
    }

}; // struct InfoHandler

class Output {

public:

    Output() noexcept = default;

    virtual ~Output() noexcept = default;

    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;

    Output(Output&&) noexcept = delete;
    Output& operator=(Output&&) noexcept = delete;

    virtual void file(const std::string& filename, const osmium::io::File& input_file) = 0;
    virtual void header(const osmium::io::Header& header) = 0;
    virtual void data(const osmium::io::Header& header, const InfoHandler& info_handler) = 0;

    virtual void output() {
    }

}; // class Output

class HumanReadableOutput : public Output {

public:

    void file(const std::string& input_filename, const osmium::io::File& input_file) final {
        std::cout << "File:\n";
        std::cout << "  Name: " << input_filename << "\n";
        std::cout << "  Format: " << input_file.format() << "\n";
        std::cout << "  Compression: " << input_file.compression() << "\n";

        if (!input_file.filename().empty()) {
            std::cout << "  Size: " << osmium::util::file_size(input_file.filename()) << "\n";
        }
    }

    void header(const osmium::io::Header& header) final {
        std::cout << "Header:\n";

        std::cout << "  Bounding boxes:\n";
        for (const auto& box : header.boxes()) {
            std::cout << "    " << box << "\n";
        }
        std::cout << "  With history: " << yes_no(header.has_multiple_object_versions());

        std::cout << "  Options:\n";
        for (const auto& option : header) {
            std::cout << "    " << option.first << "=" << option.second << "\n";
        }
    }

    void data(const osmium::io::Header& header, const InfoHandler& info_handler) final {
        std::cout << "Data:\n";
        std::cout << "  Bounding box: " << info_handler.bounds << "\n";

        if (info_handler.first_timestamp() != osmium::end_of_time()) {
            std::cout << "  Timestamps:\n";
            std::cout << "    First: " << info_handler.first_timestamp() << "\n";
            std::cout << "    Last: " << info_handler.last_timestamp() << "\n";
        }

        std::cout << "  Objects ordered (by type and id): " << yes_no(info_handler.ordered);

        std::cout << "  Multiple versions of same object: ";
        if (info_handler.ordered) {
            std::cout << yes_no(info_handler.multiple_versions);
            if (info_handler.multiple_versions != header.has_multiple_object_versions()) {
                std::cout << "    WARNING! This is different from the setting in the header.\n";
            }
        } else {
            std::cout << "unknown (because objects in file are unordered)\n";
        }

        std::cout << "  CRC32: " << std::hex << info_handler.crc32().checksum() << std::dec << "\n";

        std::cout << "  Number of changesets: " << info_handler.changesets << "\n";
        std::cout << "  Number of nodes: "      << info_handler.nodes      << "\n";
        std::cout << "  Number of ways: "       << info_handler.ways       << "\n";
        std::cout << "  Number of relations: "  << info_handler.relations  << "\n";

        std::cout << "  Largest changeset ID: " << info_handler.largest_changeset_id() << "\n";
        std::cout << "  Largest node ID: "      << info_handler.largest_node_id()      << "\n";
        std::cout << "  Largest way ID: "       << info_handler.largest_way_id()       << "\n";
        std::cout << "  Largest relation ID: "  << info_handler.largest_relation_id()  << "\n";

        std::cout << "  All objects have following metadata attributes: " << info_handler.metadata_all_objects  << "\n";
        std::cout << "  Some objects have following metadata attributes: " << info_handler.metadata_some_objects  << "\n";
    }

}; // class HumanReadableOutput


class JSONOutput : public Output {

    using writer_type = rapidjson::PrettyWriter<rapidjson::StringBuffer>;

    rapidjson::StringBuffer m_stream;
    writer_type m_writer;

public:

    JSONOutput() :
        m_writer(m_stream) {
        m_writer.StartObject();
    }

    void file(const std::string& input_filename, const osmium::io::File& input_file) final {
        m_writer.String("file");
        m_writer.StartObject();

        m_writer.String("name");
        m_writer.String(input_filename.c_str());

        m_writer.String("format");
        m_writer.String(osmium::io::as_string(input_file.format()));

        m_writer.String("compression");
        m_writer.String(osmium::io::as_string(input_file.compression()));

        if (!input_file.filename().empty()) {
            m_writer.String("size");
            m_writer.Int64(osmium::util::file_size(input_file.filename()));
        }

        m_writer.EndObject();
    }

    void add_bbox(const osmium::Box& box) {
        m_writer.StartArray();
        m_writer.Double(box.bottom_left().lon());
        m_writer.Double(box.bottom_left().lat());
        m_writer.Double(box.top_right().lon());
        m_writer.Double(box.top_right().lat());
        m_writer.EndArray();
    }

    void header(const osmium::io::Header& header) final {
        m_writer.String("header");
        m_writer.StartObject();

        m_writer.String("boxes");
        m_writer.StartArray();
        for (const auto& box : header.boxes()) {
            add_bbox(box);
        }
        m_writer.EndArray();

        m_writer.String("with_history");
        m_writer.Bool(header.has_multiple_object_versions());

        m_writer.String("option");
        m_writer.StartObject();
        for (const auto& option : header) {
            m_writer.String(option.first.c_str());
            m_writer.String(option.second.c_str());
        }
        m_writer.EndObject();

        m_writer.EndObject();
    }

    void data(const osmium::io::Header& /*header*/, const InfoHandler& info_handler) final {
        m_writer.String("data");
        m_writer.StartObject();

        m_writer.String("bbox");
        add_bbox(info_handler.bounds);

        if (info_handler.first_timestamp() != osmium::end_of_time()) {
            m_writer.String("timestamp");
            m_writer.StartObject();

            m_writer.String("first");
            std::string s = info_handler.first_timestamp().to_iso();
            m_writer.String(s.c_str());
            m_writer.String("last");
            s = info_handler.last_timestamp().to_iso();
            m_writer.String(s.c_str());

            m_writer.EndObject();
        }

        m_writer.String("objects_ordered");
        m_writer.Bool(info_handler.ordered);

        if (info_handler.ordered) {
            m_writer.String("multiple_versions");
            m_writer.Bool(info_handler.multiple_versions);
        }

        m_writer.String("crc32");
        std::stringstream ss;
        ss << std::hex << info_handler.crc32().checksum() << std::dec;
        m_writer.String(ss.str().c_str());

        m_writer.String("count");
        m_writer.StartObject();
        m_writer.String("changesets");
        m_writer.Int64(info_handler.changesets);
        m_writer.String("nodes");
        m_writer.Int64(info_handler.nodes);
        m_writer.String("ways");
        m_writer.Int64(info_handler.ways);
        m_writer.String("relations");
        m_writer.Int64(info_handler.relations);
        m_writer.EndObject();

        m_writer.String("maxid");
        m_writer.StartObject();
        m_writer.String("changesets");
        m_writer.Int64(info_handler.largest_changeset_id());
        m_writer.String("nodes");
        m_writer.Int64(info_handler.largest_node_id());
        m_writer.String("ways");
        m_writer.Int64(info_handler.largest_way_id());
        m_writer.String("relations");
        m_writer.Int64(info_handler.largest_relation_id());
        m_writer.EndObject();

        m_writer.String("metadata");
        m_writer.StartObject();
        m_writer.String("all_objects");
        m_writer.StartObject();
        m_writer.String("version");
        m_writer.Bool(info_handler.metadata_all_objects.version());
        m_writer.String("timestamp");
        m_writer.Bool(info_handler.metadata_all_objects.timestamp());
        m_writer.String("changeset");
        m_writer.Bool(info_handler.metadata_all_objects.changeset());
        m_writer.String("user");
        m_writer.Bool(info_handler.metadata_all_objects.user());
        m_writer.String("uid");
        m_writer.Bool(info_handler.metadata_all_objects.uid());
        m_writer.EndObject();
        m_writer.String("some_objects");
        m_writer.StartObject();
        m_writer.String("version");
        m_writer.Bool(info_handler.metadata_some_objects.version());
        m_writer.String("timestamp");
        m_writer.Bool(info_handler.metadata_some_objects.timestamp());
        m_writer.String("changeset");
        m_writer.Bool(info_handler.metadata_some_objects.changeset());
        m_writer.String("user");
        m_writer.Bool(info_handler.metadata_some_objects.user());
        m_writer.String("uid");
        m_writer.Bool(info_handler.metadata_some_objects.uid());
        m_writer.EndObject();
        m_writer.EndObject();

        m_writer.EndObject();
    }

    void output() final {
        m_writer.EndObject();
        std::cout << m_stream.GetString() << "\n";
    }

}; // class JSONOutput

class SimpleOutput : public Output {

    std::string m_get_value;

public:

    explicit SimpleOutput(std::string get_value) :
        m_get_value(std::move(get_value)) {
    }

    void file(const std::string& input_filename, const osmium::io::File& input_file) final {
        if (m_get_value == "file.name") {
            std::cout << input_filename << "\n";
        }
        if (m_get_value == "file.format") {
            std::cout << input_file.format() << "\n";
        }
        if (m_get_value == "file.compression") {
            std::cout << input_file.compression() << "\n";
        }
        if (m_get_value == "file.size") {
            if (input_file.filename().empty()) {
                std::cout << 0 << "\n";
            } else {
                std::cout << osmium::util::file_size(input_file.filename()) << "\n";
            }
        }
    }

    void header(const osmium::io::Header& header) final {
        if (m_get_value == "header.with_history") {
            std::cout << yes_no(header.has_multiple_object_versions());
        }

        for (const auto& option : header) {
            std::string value_name{"header.option."};
            value_name.append(option.first);
            if (m_get_value == value_name) {
                std::cout << option.second << "\n";
            }
        }
    }

    void data(const osmium::io::Header& /*header*/, const InfoHandler& info_handler) final {
        if (m_get_value == "data.bbox") {
            std::cout << info_handler.bounds << "\n";
        }

        if (m_get_value == "data.timestamp.first") {
            if (info_handler.first_timestamp() == osmium::end_of_time()) {
                std::cout << "\n";
            } else {
                std::cout << info_handler.first_timestamp() << "\n";
            }
        }

        if (m_get_value == "data.timestamp.last") {
            if (info_handler.first_timestamp() == osmium::end_of_time()) {
                std::cout << "\n";
            } else {
                std::cout << info_handler.last_timestamp() << "\n";
            }
        }

        if (m_get_value == "data.objects_ordered") {
            std::cout << (info_handler.ordered ? "yes\n" : "no\n");
        }

        if (m_get_value == "data.multiple_versions") {
            if (info_handler.ordered) {
                std::cout << (info_handler.multiple_versions ? "yes\n" : "no\n");
            } else {
                std::cout << "unknown\n";
            }
        }

        if (m_get_value == "data.crc32") {
            std::cout << std::hex << info_handler.crc32().checksum() << std::dec << "\n";
        }

        if (m_get_value == "data.count.changesets") {
            std::cout << info_handler.changesets << "\n";
        }
        if (m_get_value == "data.count.nodes") {
            std::cout << info_handler.nodes << "\n";
        }
        if (m_get_value == "data.count.ways") {
            std::cout << info_handler.ways << "\n";
        }
        if (m_get_value == "data.count.relations") {
            std::cout << info_handler.relations << "\n";
        }

        if (m_get_value == "data.maxid.changesets") {
            std::cout << info_handler.largest_changeset_id() << "\n";
        }
        if (m_get_value == "data.maxid.nodes") {
            std::cout << info_handler.largest_node_id() << "\n";
        }
        if (m_get_value == "data.maxid.ways") {
            std::cout << info_handler.largest_way_id() << "\n";
        }
        if (m_get_value == "data.maxid.relations") {
            std::cout << info_handler.largest_relation_id() << "\n";
        }
        if (m_get_value == "metadata.all_objects.version") {
            std::cout << (info_handler.metadata_all_objects.version() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.all_objects.timestamp") {
            std::cout << (info_handler.metadata_all_objects.timestamp() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.all_objects.changeset") {
            std::cout << (info_handler.metadata_all_objects.changeset() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.all_objects.uid") {
            std::cout << (info_handler.metadata_all_objects.uid() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.all_objects.user") {
            std::cout << (info_handler.metadata_all_objects.user() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.some_objects.version") {
            std::cout << (info_handler.metadata_some_objects.version() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.some_objects.timestamp") {
            std::cout << (info_handler.metadata_some_objects.timestamp() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.some_objects.changeset") {
            std::cout << (info_handler.metadata_some_objects.changeset() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.some_objects.uid") {
            std::cout << (info_handler.metadata_some_objects.uid() ? "yes\n" : "no\n");
        }
        if (m_get_value == "metadata.some_objects.user") {
            std::cout << (info_handler.metadata_some_objects.user() ? "yes\n" : "no\n");
        }
    }

}; // class SimpleOutput


/*************************************************************************/

bool CommandFileinfo::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("extended,e", "Extended output")
    ("get,g", po::value<std::string>(), "Get value")
    ("show-variables,G", "Show variables for --get option")
    ("json,j", "JSON output")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
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

    if (vm.count("extended")) {
        m_extended = true;
    }

    if (vm.count("json")) {
        m_json_output = true;
    }

    std::vector<std::string> known_values = {
        "file.name",
        "file.format",
        "file.compression",
        "file.size",
        "header.with_history",
        "header.option.generator",
        "header.option.osmosis_replication_base_url",
        "header.option.osmosis_replication_sequence_number",
        "header.option.osmosis_replication_timestamp",
        "header.option.pbf_dense_nodes",
        "header.option.timestamp",
        "header.option.version",
        "data.bbox",
        "data.timestamp.first",
        "data.timestamp.last",
        "data.objects_ordered",
        "data.multiple_versions",
        "data.crc32",
        "data.count.nodes",
        "data.count.ways",
        "data.count.relations",
        "data.count.changesets",
        "data.maxid.nodes",
        "data.maxid.ways",
        "data.maxid.relations",
        "data.maxid.changesets",
        "metadata.all_objects.version",
        "metadata.all_objects.timestamp",
        "metadata.all_objects.changeset",
        "metadata.all_objects.uid",
        "metadata.all_objects.user",
        "metadata.some_objects.version",
        "metadata.some_objects.timestamp",
        "metadata.some_objects.changeset",
        "metadata.some_objects.uid",
        "metadata.some_objects.user"
    };

    if (vm.count("show-variables")) {
        std::copy(known_values.cbegin(), known_values.cend(), std::ostream_iterator<std::string>(std::cout, "\n"));
        return false;
    }

    setup_input_file(vm);

    if (vm.count("get")) {
        m_get_value = vm["get"].as<std::string>();
        if (m_get_value.substr(0, 14) != "header.option.") {
            const auto& f = std::find(known_values.cbegin(), known_values.cend(), m_get_value);
            if (f == known_values.cend()) {
                throw argument_error{std::string{"Unknown value for --get/-g option '"} + m_get_value + "'. Use --show-variables/-G to see list of known values."};
            }
        }
        if (m_get_value.substr(0, 5) == "data." && !m_extended) {
            throw argument_error{"You need to set --extended/-e for any 'data.*' variables to be available."};
        }
    }

    if (vm.count("get") && vm.count("json")) {
        throw argument_error{"You can not use --get/-g and --json/-j together."};
    }

    return true;
}

void CommandFileinfo::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    extended output: " << (m_extended ? "yes\n" : "no\n");
}

bool CommandFileinfo::run() {
    std::unique_ptr<Output> output;
    if (m_json_output) {
        output.reset(new JSONOutput{});
    } else if (m_get_value.empty()) {
        output.reset(new HumanReadableOutput{});
    } else {
        output.reset(new SimpleOutput{m_get_value});
    }

    output->file(m_input_filename, m_input_file);

    osmium::io::Reader reader{m_input_file, m_extended ? osmium::osm_entity_bits::all : osmium::osm_entity_bits::nothing};
    osmium::io::Header header{reader.header()};
    output->header(header);

    if (m_extended) {
        InfoHandler info_handler;
        osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            osmium::apply(buffer, info_handler);
        }
        progress_bar.done();
        output->data(header, info_handler);
    }

    reader.close();
    output->output();

    m_vout << "Done.\n";

    return true;
}

