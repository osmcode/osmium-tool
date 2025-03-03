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

#include "export_format_json.hpp"

#include "../exception.hpp"
#include "../util.hpp"

#include <osmium/geom/coordinates.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/osm.hpp>

#include <array>
#include <cstdio>
#include <string>

static constexpr const std::size_t initial_buffer_size = 1024UL * 1024UL;
static constexpr const std::size_t flush_buffer_size   =  800UL * 1024UL;

ExportFormatJSON::ExportFormatJSON(const std::string& output_format,
                                   const std::string& output_filename,
                                   osmium::io::overwrite overwrite,
                                   osmium::io::fsync fsync,
                                   const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync),
    m_text_sequence_format(output_format == "geojsonseq"),
    m_with_record_separator(m_text_sequence_format && options.format_options.is_true("print_record_separator")) {
    m_buffer.reserve(initial_buffer_size);
    if (!m_text_sequence_format) {
        m_buffer += R"({"type":"FeatureCollection","features":[)";
        m_buffer += '\n';
    }
    m_committed_size = m_buffer.size();

    if (output_format == "geojsonseq") {
        const auto prs = options.format_options.get("print_record_separator");
        if (prs != "true" && prs != "false") {
            throw config_error{"Unknown value for print_record_separator option: '" + prs + "'."};
        }
    }
}

void ExportFormatJSON::flush_to_output() {
    osmium::io::detail::reliable_write(m_fd, m_buffer.data(), m_buffer.size());
    m_buffer.clear();
    m_committed_size = 0;
}

void ExportFormatJSON::start_feature(const std::string& prefix, osmium::object_id_type id) {
    rollback_uncomitted();

    if (m_count > 0) {
        if (!m_text_sequence_format) {
            m_buffer += ',';
        }
        m_buffer += '\n';
    }

    if (m_with_record_separator) {
        m_buffer += static_cast<char>(0x1e);
    }

    m_buffer += R"({"type":"Feature")";

    if (options().unique_id == unique_id_type::counter) {
        m_buffer += R"(,"id":)";
        m_buffer += std::to_string(m_count + 1);
    } else if (options().unique_id == unique_id_type::type_id) {
        m_buffer += R"(,"id":")";
        m_buffer += prefix;
        m_buffer += std::to_string(id);
        m_buffer += '"';
    }
}

void ExportFormatJSON::add_option(const std::string& name) {
    const nlohmann::json j = name;
    m_buffer += j.dump();
    m_buffer += ':';
}

void ExportFormatJSON::add_attributes(const osmium::OSMObject& object) {

    if (!options().type.empty()) {
        add_option(options().type);
        m_buffer += '"';
        m_buffer += object_type_as_string(object);
        m_buffer += '"';
        m_buffer += ',';
    }

    if (!options().id.empty()) {
        add_option(options().id);
        m_buffer += std::to_string(object.type() == osmium::item_type::area ? osmium::area_id_to_object_id(object.id()) : object.id());
        m_buffer += ',';
    }

    if (!options().version.empty()) {
        add_option(options().version);
        m_buffer += std::to_string(object.version());
        m_buffer += ',';
    }

    if (!options().changeset.empty()) {
        add_option(options().changeset);
        m_buffer += std::to_string(object.changeset());
        m_buffer += ',';
    }

    if (!options().uid.empty()) {
        add_option(options().uid);
        m_buffer += std::to_string(object.uid());
        m_buffer += ',';
    }

    if (!options().user.empty()) {
        add_option(options().user);
        const nlohmann::json j = object.user();
        m_buffer += j.dump();
        m_buffer += ',';
    }

    if (!options().timestamp.empty()) {
        add_option(options().timestamp);
        m_buffer += std::to_string(object.timestamp().seconds_since_epoch());
        m_buffer += ',';
    }

    if (!options().way_nodes.empty() && object.type() == osmium::item_type::way) {
        add_option(options().way_nodes);
        m_buffer += '[';
        for (const auto& nr : static_cast<const osmium::Way&>(object).nodes()) {
            m_buffer += std::to_string(nr.ref());
            m_buffer += ',';
        }

        if (m_buffer.back() == ',') {
            m_buffer.back() = ']';
        } else {
            m_buffer += ']';
        }
        m_buffer += ',';
    }
}

void ExportFormatJSON::finish_feature(const osmium::OSMObject& object) {
    m_buffer += R"(,"properties":{)";

    add_attributes(object);

    nlohmann::json j;
    const bool has_tags = add_tags(object, [&](const osmium::Tag& tag) {
        j = tag.key();
        m_buffer += j.dump();
        m_buffer += ':';
        j = tag.value();
        m_buffer += j.dump();
        m_buffer += ',';
    });

    if (has_tags || options().keep_untagged) {
        if (m_buffer.back() == ',') {
            m_buffer.back() = '}'; // end properties
        } else {
            m_buffer += '}'; // end properties
        }
        m_buffer += '}'; // end feature

        m_committed_size = m_buffer.size();
        ++m_count;

        if (m_buffer.size() > flush_buffer_size) {
            flush_to_output();
        }
    }
}

namespace {

void append_coordinate(std::string* buffer, double coord) {
    std::array<char, 20> tmp{};
    auto n = std::snprintf(&*tmp.begin(), 20, "%.7f", coord);

    // remove trailing zeros
    while (n >= 2 && tmp[n - 1] == '0' && tmp[n - 2] != '.') {
        --n;
    }

    buffer->append(&*tmp.begin(), n);
}

} // anonymous namespace

void ExportFormatJSON::create_coordinate(const osmium::Location& location) {
    std::string buffer;
    buffer.resize(20);
    m_buffer += '[';
    append_coordinate(&m_buffer, location.lon());
    m_buffer += ',';
    append_coordinate(&m_buffer, location.lat());
    m_buffer += ']';
}

void ExportFormatJSON::create_coordinate_list(const osmium::NodeRefList& nrl) {
    m_buffer += '[';
    for (auto const &nr : nrl) {
        create_coordinate(nr.location());
        m_buffer += ',';
    }

    if (m_buffer.back() == ',') {
        m_buffer.back() = ']';
    } else {
        m_buffer += ']';
    }
}

void ExportFormatJSON::create_point(const osmium::Node& node) {
    m_buffer += R"(,"geometry":{"type":"Point","coordinates":)";
    create_coordinate(node.location());
    m_buffer += '}';
}

void ExportFormatJSON::create_linestring(const osmium::Way& way) {
    m_buffer += R"(,"geometry":{"type":"LineString","coordinates":)";
    create_coordinate_list(way.nodes());
    m_buffer += '}';
}

void ExportFormatJSON::create_multipolygon(const osmium::Area& area) {
    m_buffer += R"(,"geometry":{"type":"MultiPolygon","coordinates":)";
    m_buffer += '[';
    for (const auto &outer_ring : area.outer_rings()) {
        m_buffer += '[';
        create_coordinate_list(outer_ring);
        for (const auto &inner_ring : area.inner_rings(outer_ring)) {
            m_buffer += ",";
            create_coordinate_list(inner_ring);
        }
        m_buffer += "],";
    }
    m_buffer.pop_back(); // remove trailing comma
    m_buffer += "]}";
}

void ExportFormatJSON::node(const osmium::Node& node) {
    start_feature("n", node.id());
    create_point(node);
    finish_feature(node);
}

void ExportFormatJSON::way(const osmium::Way& way) {
    start_feature("w", way.id());
    create_linestring(way);
    finish_feature(way);
}

void ExportFormatJSON::area(const osmium::Area& area) {
    start_feature("a", area.id());
    create_multipolygon(area);
    finish_feature(area);
}

void ExportFormatJSON::rollback_uncomitted() {
    m_buffer.resize(m_committed_size);
}

void ExportFormatJSON::close() {
    if (m_fd > 0) {
        rollback_uncomitted();

        m_buffer += '\n';
        if (!m_text_sequence_format) {
            m_buffer += "]}\n";
        }

        flush_to_output();
        if (m_fsync == osmium::io::fsync::yes) {
            osmium::io::detail::reliable_fsync(m_fd);
        }
        ::close(m_fd);
        m_fd = -1;
    }
}

