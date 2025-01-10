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
#include <osmium/osm/crc_zlib.hpp>
#include <osmium/osm/metadata_options.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/minmax.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <nlohmann/json.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef _WIN32
# include <unistd.h>
#endif

// Use ordered_json type if available. Tests rely on ordering.
#if NLOHMANN_JSON_VERSION_MAJOR == 3 && NLOHMANN_JSON_VERSION_MINOR < 9
using json = nlohmann::json;
#else
using json = nlohmann::ordered_json;
#endif

/*************************************************************************/

struct InfoHandler : public osmium::handler::Handler {

    osmium::Box bounds;

    uint64_t changesets = 0;
    uint64_t nodes      = 0;
    uint64_t ways       = 0;
    uint64_t relations  = 0;

    uint64_t buffers_count    = 0;
    uint64_t buffers_size     = 0;
    uint64_t buffers_capacity = 0;

    osmium::min_op<osmium::object_id_type> smallest_changeset_id;
    osmium::min_op<osmium::object_id_type> smallest_node_id;
    osmium::min_op<osmium::object_id_type> smallest_way_id;
    osmium::min_op<osmium::object_id_type> smallest_relation_id;

    osmium::max_op<osmium::object_id_type> largest_changeset_id;
    osmium::max_op<osmium::object_id_type> largest_node_id;
    osmium::max_op<osmium::object_id_type> largest_way_id;
    osmium::max_op<osmium::object_id_type> largest_relation_id;

    osmium::metadata_options metadata_all_objects{"all"};
    osmium::metadata_options metadata_some_objects{"none"};

    osmium::min_op<osmium::Timestamp> first_timestamp;
    osmium::max_op<osmium::Timestamp> last_timestamp;

    osmium::CRC<osmium::CRC_zlib> crc32;

    bool ordered = true;
    bool multiple_versions = false;
    bool calculate_crc = false;

    osmium::item_type last_type = osmium::item_type::undefined;
    osmium::object_id_type last_id = 0;

    explicit InfoHandler(bool with_crc) :
        calculate_crc(with_crc) {
    }

    void changeset(const osmium::Changeset& changeset) {
        if (last_type == osmium::item_type::changeset) {
            if (last_id > changeset.id()) {
                ordered = false;
            }
        } else {
            last_type = osmium::item_type::changeset;
        }

        last_id = changeset.id();
        if (calculate_crc) {
            crc32.update(changeset);
        }
        ++changesets;

        smallest_changeset_id.update(changeset.id());
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
        if (calculate_crc) {
            crc32.update(node);
        }
        bounds.extend(node.location());
        ++nodes;

        smallest_node_id.update(node.id());
        largest_node_id.update(node.id());
    }

    void way(const osmium::Way& way) {
        if (calculate_crc) {
            crc32.update(way);
        }
        ++ways;

        smallest_way_id.update(way.id());
        largest_way_id.update(way.id());
    }

    void relation(const osmium::Relation& relation) {
        if (calculate_crc) {
            crc32.update(relation);
        }
        ++relations;

        smallest_relation_id.update(relation.id());
        largest_relation_id.update(relation.id());
    }

}; // struct InfoHandler

class Output {

    bool m_calculate_crc = false;

public:

    Output() noexcept = default;

    virtual ~Output() noexcept = default;

    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;

    Output(Output&&) noexcept = delete;
    Output& operator=(Output&&) noexcept = delete;

    bool calculate_crc() const noexcept {
        return m_calculate_crc;
    }

    void set_crc(bool with_crc) noexcept {
        m_calculate_crc = with_crc;
    }

    virtual void file(const std::string& filename, const osmium::io::File& input_file) = 0;
    virtual void header(const osmium::io::Header& header) = 0;
    virtual void data(const osmium::io::Header& header, const InfoHandler& info_handler) = 0;

    virtual void output() {
    }

}; // class Output

namespace {

osmium::object_id_type get_smallest(osmium::object_id_type id) noexcept {
    return id == osmium::min_op<osmium::object_id_type>{}() ? 0 : id;
}

osmium::object_id_type get_largest(osmium::object_id_type id) noexcept {
    return id == osmium::max_op<osmium::object_id_type>{}() ? 0 : id;
}

} // anonymous namespace

class HumanReadableOutput : public Output {

public:

    void file(const std::string& input_filename, const osmium::io::File& input_file) final {
        std::cout << "File:\n";
        std::cout << "  Name: " << input_filename << "\n";
        std::cout << "  Format: " << input_file.format() << "\n";
        std::cout << "  Compression: " << input_file.compression() << "\n";

        if (!input_file.filename().empty()) {
            std::cout << "  Size: " << file_size(input_file) << "\n";
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

        if (calculate_crc()) {
            std::cout << "  CRC32: " << std::hex << info_handler.crc32().checksum() << std::dec << "\n";
        } else {
            std::cout << "  CRC32: not calculated (use --crc/-c to enable)\n";
        }

        std::cout << "  Number of changesets: " << info_handler.changesets << "\n";
        std::cout << "  Number of nodes: "      << info_handler.nodes      << "\n";
        std::cout << "  Number of ways: "       << info_handler.ways       << "\n";
        std::cout << "  Number of relations: "  << info_handler.relations  << "\n";

        std::cout << "  Smallest changeset ID: " << get_smallest(info_handler.smallest_changeset_id()) << "\n";
        std::cout << "  Smallest node ID: "      << get_smallest(info_handler.smallest_node_id())      << "\n";
        std::cout << "  Smallest way ID: "       << get_smallest(info_handler.smallest_way_id())       << "\n";
        std::cout << "  Smallest relation ID: "  << get_smallest(info_handler.smallest_relation_id())  << "\n";

        std::cout << "  Largest changeset ID: " << get_largest(info_handler.largest_changeset_id()) << "\n";
        std::cout << "  Largest node ID: "      << get_largest(info_handler.largest_node_id())      << "\n";
        std::cout << "  Largest way ID: "       << get_largest(info_handler.largest_way_id())       << "\n";
        std::cout << "  Largest relation ID: "  << get_largest(info_handler.largest_relation_id())  << "\n";

        const auto num_objects = info_handler.changesets + info_handler.nodes + info_handler.ways + info_handler.relations;
        std::cout << "  Number of buffers: " << info_handler.buffers_count;
        if (num_objects != 0) {
            std::cout << " (avg " << (num_objects / info_handler.buffers_count) << " objects per buffer)\n";
        } else {
            std::cout << '\n';
        }

        std::cout << "  Sum of buffer sizes: " << info_handler.buffers_size
                  << " (" << show_gbytes(info_handler.buffers_size) << " GB)\n";

        if (info_handler.buffers_capacity != 0) {
            const auto fill_factor = std::round(100 * static_cast<double>(info_handler.buffers_size) / static_cast<double>(info_handler.buffers_capacity));
            std::cout << "  Sum of buffer capacities: " << info_handler.buffers_capacity
                      << " (" << show_gbytes(info_handler.buffers_capacity) << " GB, " << fill_factor << "% full)\n";
        } else {
            std::cout << "  Sum of buffer capacities: 0 (0 GB)\n";
        }

        std::cout << "Metadata:\n";
        std::cout << "  All objects have following metadata attributes: " << info_handler.metadata_all_objects  << "\n";
        std::cout << "  Some objects have following metadata attributes: " << info_handler.metadata_some_objects  << "\n";
    }

}; // class HumanReadableOutput

class JSONOutput : public Output {

    json m_json;

public:

    void file(const std::string& input_filename, const osmium::io::File& input_file) final {
        m_json["file"] = {
            {"name", input_filename},
            {"format", osmium::io::as_string(input_file.format())},
            {"compression", osmium::io::as_string(input_file.compression())}
        };

        if (!input_file.filename().empty()) {
            m_json["file"]["size"] = static_cast<int64_t>(file_size(input_file));
        }
    }

    void header(const osmium::io::Header& header) final {
        std::vector<std::vector<double>> boxes;
        for (const auto& box : header.boxes()) {
            boxes.emplace_back(std::vector<double>{
                box.bottom_left().lon(),
                box.bottom_left().lat(),
                box.top_right().lon(),
                box.top_right().lat()
            });
        }

        json json_options;
        for (const auto& option : header) {
            json_options[option.first] = option.second;
        }

        m_json["header"] = {
            {"boxes", boxes},
            {"with_history", header.has_multiple_object_versions()},
            {"option", json_options}
        };
    }

    void data(const osmium::io::Header& /*header*/, const InfoHandler& info_handler) final {
        json json_data = {
            {"bbox", { info_handler.bounds.bottom_left().lon(),
                       info_handler.bounds.bottom_left().lat(),
                       info_handler.bounds.top_right().lon(),
                       info_handler.bounds.top_right().lat() }}
        };

        if (info_handler.first_timestamp() != osmium::end_of_time()) {
            json_data["timestamp"] = {
                {"first", info_handler.first_timestamp().to_iso()},
                {"last", info_handler.last_timestamp().to_iso()}
            };
        }

        json_data["objects_ordered"] = info_handler.ordered;

        if (info_handler.ordered) {
            json_data["multiple_versions"] = info_handler.multiple_versions;
        }

        if (calculate_crc()) {
            std::stringstream ss;
            ss << std::hex << info_handler.crc32().checksum() << std::dec;
            json_data["crc32"] = ss.str().c_str();
        }

        json_data["count"] = {
            {"changesets", info_handler.changesets},
            {"nodes", info_handler.nodes},
            {"ways", info_handler.ways},
            {"relations", info_handler.relations}
        };

        json_data["minid"] = {
            {"changesets", static_cast<int64_t>(get_smallest(info_handler.smallest_changeset_id()))},
            {"nodes", static_cast<int64_t>(get_smallest(info_handler.smallest_node_id()))},
            {"ways", static_cast<int64_t>(get_smallest(info_handler.smallest_way_id()))},
            {"relations", static_cast<int64_t>(get_smallest(info_handler.smallest_relation_id()))}
        };

        json_data["maxid"] = {
            {"changesets", static_cast<int64_t>(get_largest(info_handler.largest_changeset_id()))},
            {"nodes", static_cast<int64_t>(get_largest(info_handler.largest_node_id()))},
            {"ways", static_cast<int64_t>(get_largest(info_handler.largest_way_id()))},
            {"relations", static_cast<int64_t>(get_largest(info_handler.largest_relation_id()))}
        };

        json_data["buffers"] = {
            {"count", get_largest(static_cast<int64_t>(info_handler.buffers_count))},
            {"size", get_largest(static_cast<int64_t>(info_handler.buffers_size))},
            {"capacity", get_largest(static_cast<int64_t>(info_handler.buffers_capacity))}
        };

        json_data["metadata"] = {
            {"all_objects", {
                {"version", info_handler.metadata_all_objects.version()},
                {"timestamp", info_handler.metadata_all_objects.timestamp()},
                {"changeset", info_handler.metadata_all_objects.changeset()},
                {"user", info_handler.metadata_all_objects.user()},
                {"uid", info_handler.metadata_all_objects.uid()}}
            },
            {"some_objects", {
                {"version", info_handler.metadata_some_objects.version()},
                {"timestamp", info_handler.metadata_some_objects.timestamp()},
                {"changeset", info_handler.metadata_some_objects.changeset()},
                {"user", info_handler.metadata_some_objects.user()},
                {"uid", info_handler.metadata_some_objects.uid()}}
            }
        };

        m_json["data"] = json_data;
    }

    void output() final {
        std::cout << std::setw(4) << m_json << "\n";
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
            return;
        }
        if (m_get_value == "file.format") {
            std::cout << input_file.format() << "\n";
            return;
        }
        if (m_get_value == "file.compression") {
            std::cout << input_file.compression() << "\n";
            return;
        }
        if (m_get_value == "file.size") {
            if (input_file.filename().empty()) {
                std::cout << 0 << "\n";
            } else {
                std::cout << file_size(input_file) << "\n";
            }
            return;
        }
    }

    void header(const osmium::io::Header& header) final {
        if (m_get_value == "header.boxes") {
            for (const auto& box : header.boxes()) {
                std::cout << box << "\n";
            }
        }

        if (m_get_value == "header.with_history") {
            std::cout << yes_no(header.has_multiple_object_versions());
            return;
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
            return;
        }

        if (m_get_value == "data.timestamp.first") {
            if (info_handler.first_timestamp() == osmium::end_of_time()) {
                std::cout << "\n";
            } else {
                std::cout << info_handler.first_timestamp() << "\n";
            }
            return;
        }

        if (m_get_value == "data.timestamp.last") {
            if (info_handler.first_timestamp() == osmium::end_of_time()) {
                std::cout << "\n";
            } else {
                std::cout << info_handler.last_timestamp() << "\n";
            }
            return;
        }

        if (m_get_value == "data.objects_ordered") {
            std::cout << (info_handler.ordered ? "yes\n" : "no\n");
            return;
        }

        if (m_get_value == "data.multiple_versions") {
            if (info_handler.ordered) {
                std::cout << (info_handler.multiple_versions ? "yes\n" : "no\n");
            } else {
                std::cout << "unknown\n";
            }
            return;
        }

        if (m_get_value == "data.crc32") {
            std::cout << std::hex << info_handler.crc32().checksum() << std::dec << "\n";
            return;
        }

        if (m_get_value == "data.count.changesets") {
            std::cout << info_handler.changesets << "\n";
            return;
        }
        if (m_get_value == "data.count.nodes") {
            std::cout << info_handler.nodes << "\n";
            return;
        }
        if (m_get_value == "data.count.ways") {
            std::cout << info_handler.ways << "\n";
            return;
        }
        if (m_get_value == "data.count.relations") {
            std::cout << info_handler.relations << "\n";
            return;
        }

        if (m_get_value == "data.minid.changesets") {
            std::cout << get_smallest(info_handler.smallest_changeset_id()) << "\n";
            return;
        }
        if (m_get_value == "data.minid.nodes") {
            std::cout << get_smallest(info_handler.smallest_node_id()) << "\n";
            return;
        }
        if (m_get_value == "data.minid.ways") {
            std::cout << get_smallest(info_handler.smallest_way_id()) << "\n";
            return;
        }
        if (m_get_value == "data.minid.relations") {
            std::cout << get_smallest(info_handler.smallest_relation_id()) << "\n";
            return;
        }

        if (m_get_value == "data.maxid.changesets") {
            std::cout << get_largest(info_handler.largest_changeset_id()) << "\n";
            return;
        }
        if (m_get_value == "data.maxid.nodes") {
            std::cout << get_largest(info_handler.largest_node_id()) << "\n";
            return;
        }
        if (m_get_value == "data.maxid.ways") {
            std::cout << get_largest(info_handler.largest_way_id()) << "\n";
            return;
        }
        if (m_get_value == "data.maxid.relations") {
            std::cout << get_largest(info_handler.largest_relation_id()) << "\n";
            return;
        }

        if (m_get_value == "data.buffers.count") {
            std::cout << info_handler.buffers_count << "\n";
            return;
        }
        if (m_get_value == "data.buffers.size") {
            std::cout << info_handler.buffers_size << "\n";
            return;
        }
        if (m_get_value == "data.buffers.capacity") {
            std::cout << info_handler.buffers_capacity << "\n";
            return;
        }

        if (m_get_value == "metadata.all_objects.version") {
            std::cout << (info_handler.metadata_all_objects.version() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.all_objects.timestamp") {
            std::cout << (info_handler.metadata_all_objects.timestamp() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.all_objects.changeset") {
            std::cout << (info_handler.metadata_all_objects.changeset() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.all_objects.uid") {
            std::cout << (info_handler.metadata_all_objects.uid() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.all_objects.user") {
            std::cout << (info_handler.metadata_all_objects.user() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.some_objects.version") {
            std::cout << (info_handler.metadata_some_objects.version() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.some_objects.timestamp") {
            std::cout << (info_handler.metadata_some_objects.timestamp() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.some_objects.changeset") {
            std::cout << (info_handler.metadata_some_objects.changeset() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.some_objects.uid") {
            std::cout << (info_handler.metadata_some_objects.uid() ? "yes\n" : "no\n");
            return;
        }
        if (m_get_value == "metadata.some_objects.user") {
            std::cout << (info_handler.metadata_some_objects.user() ? "yes\n" : "no\n");
            return;
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
    ("crc,c", "Calculate CRC")
    ("no-crc", "Do not calculate CRC")
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation, changeset)")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};

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

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_object_type_nwrc(vm);

    if (vm.count("extended")) {
        m_extended = true;
    }

    if (vm.count("json")) {
        m_json_output = true;
    }

    if (vm.count("crc") && vm.count("no-crc")) {
        throw argument_error{"Can not use --crc/-c option and --no-crc at the same time."};
    }

    if (m_extended && (vm.count("crc") || (m_json_output && !vm.count("no-crc")))) {
        m_calculate_crc = true;
    }

    const std::vector<std::string> known_values = {
        "file.name",
        "file.format",
        "file.compression",
        "file.size",
        "header.boxes",
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
        "data.minid.nodes",
        "data.minid.ways",
        "data.minid.relations",
        "data.minid.changesets",
        "data.maxid.nodes",
        "data.maxid.ways",
        "data.maxid.relations",
        "data.maxid.changesets",
        "data.buffers.count",
        "data.buffers.size",
        "data.buffers.capacity",
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
        m_calculate_crc = (m_get_value == "data.crc32");
    }

    if (vm.count("get") && vm.count("json")) {
        throw argument_error{"You can not use --get/-g and --json/-j together."};
    }

    return true;
}

void CommandFileinfo::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  other options:\n";
    show_object_types(m_vout);
    m_vout << "    extended output: " << (m_extended ? "yes\n" : "no\n");
    m_vout << "    calculate CRC: " << (m_calculate_crc ? "yes\n" : "no\n");
}

bool CommandFileinfo::run() {
    std::unique_ptr<Output> output;
    if (m_json_output) {
        output = std::make_unique<JSONOutput>();
    } else if (m_get_value.empty()) {
        output = std::make_unique<HumanReadableOutput>();
    } else {
        output = std::make_unique<SimpleOutput>(m_get_value);
    }

    output->set_crc(m_calculate_crc);
    output->file(m_input_filename, m_input_file);

    osmium::io::Reader reader{m_input_file, m_extended ? osm_entity_bits() : osmium::osm_entity_bits::nothing};
    const osmium::io::Header header{reader.header()};
    output->header(header);

    if (m_extended) {
        InfoHandler info_handler{m_calculate_crc};
        osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            ++info_handler.buffers_count;
            info_handler.buffers_size += buffer.committed();
            info_handler.buffers_capacity += buffer.capacity();
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

