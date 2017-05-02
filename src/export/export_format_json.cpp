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

#include <osmium/io/detail/read_write.hpp>

#include "export_format_json.hpp"

static constexpr const std::size_t initial_buffer_size = 1024 * 1024;
static constexpr const std::size_t flush_buffer_size   =  800 * 1024;

static void add_to_stream(rapidjson::StringBuffer& stream, const char* s) {
    while (*s) {
        stream.Put(*s++);
    }
}

ExportFormatJSON::ExportFormatJSON(const std::string& output_format,
                                   const std::string& output_filename,
                                   osmium::io::overwrite overwrite,
                                   osmium::io::fsync fsync,
                                   const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync),
    m_text_sequence_format(output_format == "geojsonseq"),
    m_with_record_separator(m_text_sequence_format && options.print_record_separator),
    m_stream(),
    m_committed_size(0),
    m_writer(m_stream),
    m_factory(m_writer) {
    m_stream.Reserve(initial_buffer_size);
    if (!m_text_sequence_format) {
        add_to_stream(m_stream, "{\"type\":\"FeatureCollection\",\"features\":[\n");
    }
    m_committed_size = m_stream.GetSize();
}

void ExportFormatJSON::flush_to_output() {
    osmium::io::detail::reliable_write(m_fd, m_stream.GetString(), m_stream.GetSize());
    m_stream.Clear();
    m_committed_size = 0;
}

void ExportFormatJSON::start_feature(const std::string& prefix, osmium::object_id_type id) {
    rollback_uncomitted();

    if (m_count > 0) {
        if (!m_text_sequence_format) {
            m_stream.Put(',');
        }
        m_stream.Put('\n');
    }
    m_writer.Reset(m_stream);

    if (m_with_record_separator) {
        m_stream.Put(0x1e);
    }
    m_writer.StartObject(); // start feature
    m_writer.Key("type");
    m_writer.String("Feature");

    if (options().unique_id == unique_id_type::counter) {
        m_writer.Key("id");
        m_writer.Int64(m_count + 1);
    } else if (options().unique_id == unique_id_type::type_id) {
        m_writer.Key("id");
        m_writer.String(prefix + std::to_string(id));
    }
}

void ExportFormatJSON::add_attributes(const osmium::OSMObject& object) {
    if (!options().type.empty()) {
        m_writer.String(options().type);
        if (object.type() == osmium::item_type::area) {
            if (static_cast<const osmium::Area&>(object).from_way()) {
                m_writer.String("way");
            } else {
                m_writer.String("relation");
            }
        } else {
            m_writer.String(osmium::item_type_to_name(object.type()));
        }
    }

    if (!options().id.empty()) {
        m_writer.String(options().id);
        m_writer.Int64(object.type() == osmium::item_type::area ? osmium::area_id_to_object_id(object.id()) : object.id());
    }

    if (!options().version.empty()) {
        m_writer.String(options().version);
        m_writer.Int(object.version());
    }

    if (!options().changeset.empty()) {
        m_writer.String(options().changeset);
        m_writer.Int(object.changeset());
    }

    if (!options().uid.empty()) {
        m_writer.String(options().uid);
        m_writer.Int(object.uid());
    }

    if (!options().user.empty()) {
        m_writer.String(options().user);
        m_writer.String(object.user());
    }

    if (!options().timestamp.empty()) {
        m_writer.String(options().timestamp);
        m_writer.Int(object.timestamp().seconds_since_epoch());
    }

    if (!options().way_nodes.empty() && object.type() == osmium::item_type::way) {
        m_writer.String(options().way_nodes);
        m_writer.StartArray();
        for (const auto& nr : static_cast<const osmium::Way&>(object).nodes()) {
            m_writer.Int64(nr.ref());
        }
        m_writer.EndArray();
    }
}

bool ExportFormatJSON::add_tags(const osmium::OSMObject& object) {
    bool has_tags = false;

    for (const auto& tag : object.tags()) {
        if (options().tags_filter(tag)) {
            has_tags = true;
            m_writer.String(tag.key());
            m_writer.String(tag.value());
        }
    }

    return has_tags;
}

void ExportFormatJSON::finish_feature(const osmium::OSMObject& object) {
    m_writer.Key("properties");
    m_writer.StartObject(); // start properties

    add_attributes(object);

    if (add_tags(object) || options().keep_untagged) {
        m_writer.EndObject(); // end properties
        m_writer.EndObject(); // end feature

        m_committed_size = m_stream.GetSize();
        ++m_count;

        if (m_stream.GetSize() > flush_buffer_size) {
            flush_to_output();
        }
    }
}

void ExportFormatJSON::node(const osmium::Node& node) {
    start_feature("n", node.id());
    m_factory.create_point(node);
    finish_feature(node);
}

void ExportFormatJSON::way(const osmium::Way& way) {
    start_feature("w", way.id());
    m_factory.create_linestring(way);
    finish_feature(way);
}

void ExportFormatJSON::area(const osmium::Area& area) {
    start_feature("a", area.id());
    m_factory.create_multipolygon(area);
    finish_feature(area);
}

void ExportFormatJSON::rollback_uncomitted() {
    const auto uncommitted_size = m_stream.GetSize() - m_committed_size;
    if (uncommitted_size != 0) {
        m_stream.Pop(uncommitted_size);
    }
}

void ExportFormatJSON::close() {
    if (m_fd > 0) {
        rollback_uncomitted();

        add_to_stream(m_stream, "\n");
        if (!m_text_sequence_format) {
            add_to_stream(m_stream, "]}\n");
        }

        flush_to_output();
        if (m_fsync == osmium::io::fsync::yes) {
            osmium::io::detail::reliable_fsync(m_fd);
        }
        ::close(m_fd);
        m_fd = -1;
    }
}

