/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2019  Jochen Topf <jochen@topf.org>

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

#include "../util.hpp"
#include "export_format_pg.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/string_util.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
# define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <limits>
#include <string>

enum {
    initial_buffer_size = 1024u * 1024u
};

enum {
    flush_buffer_size = 800u * 1024u
};

ExportFormatPg::ExportFormatPg(const std::string& /*output_format*/,
                               const std::string& output_filename,
                               osmium::io::overwrite overwrite,
                               osmium::io::fsync fsync,
                               const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync) {
    m_buffer.reserve(initial_buffer_size);
}

void ExportFormatPg::flush_to_output() {
    osmium::io::detail::reliable_write(m_fd, m_buffer.data(), m_buffer.size());
    m_buffer.clear();
    m_commit_size = 0;
}

void ExportFormatPg::start_feature(const char type, const osmium::object_id_type id) {
    m_buffer.resize(m_commit_size);
    if (options().unique_id == unique_id_type::counter) {
        m_buffer.append(std::to_string(m_count + 1));
        m_buffer += '\t';
    } else if (options().unique_id == unique_id_type::type_id) {
        m_buffer += type;
        m_buffer.append(std::to_string(id));
        m_buffer += '\t';
    }
}

void ExportFormatPg::append_pg_escaped(const char* str, std::size_t size = std::numeric_limits<std::size_t>::max()) {
    while (size-- > 0 && *str != '\0') {
        switch (*str) {
            case '\\':
                m_buffer += '\\';
                m_buffer += '\\';
                break;
            case '\n':
                m_buffer += '\\';
                m_buffer += 'n';
                break;
            case '\r':
                m_buffer += '\\';
                m_buffer += 'r';
                break;
            case '\t':
                m_buffer += '\\';
                m_buffer += 't';
                break;
            default:
                m_buffer += *str;
        }
        ++str;
    }
}

void ExportFormatPg::add_attributes(const osmium::OSMObject& object) {
    if (!options().type.empty()) {
        m_buffer.append(object_type_as_string(object));
        m_buffer += '\t';
    }

    if (!options().id.empty()) {
        m_buffer.append(std::to_string(object.type() == osmium::item_type::area ? osmium::area_id_to_object_id(object.id()) : object.id()));
        m_buffer += '\t';
    }

    if (!options().version.empty()) {
        m_buffer.append(std::to_string(object.version()));
        m_buffer += '\t';
    }

    if (!options().changeset.empty()) {
        m_buffer.append(std::to_string(object.changeset()));
        m_buffer += '\t';
    }

    if (!options().uid.empty()) {
        m_buffer.append(std::to_string(object.uid()));
        m_buffer += '\t';
    }

    if (!options().user.empty()) {
        append_pg_escaped(object.user());
        m_buffer += '\t';
    }

    if (!options().timestamp.empty()) {
        m_buffer.append(object.timestamp().to_iso());
        m_buffer += '\t';
    }

    if (!options().way_nodes.empty()) {
        if (object.type() == osmium::item_type::way) {
            m_buffer += '{';
            for (const auto& nr : static_cast<const osmium::Way&>(object).nodes()) {
                m_buffer.append(std::to_string(nr.ref()));
                m_buffer += ',';
            }
            if (m_buffer.back() == ',') {
                m_buffer.back() = '}';
            } else {
                m_buffer += '}';
            }
        } else {
            m_buffer += '\\';
            m_buffer += 'N';
        }
        m_buffer += '\t';
    }
}

bool ExportFormatPg::add_tags(const osmium::OSMObject& object) {
    bool has_tags = false;

    rapidjson::StringBuffer stream;
    rapidjson::Writer<rapidjson::StringBuffer> writer{stream};

    writer.StartObject();
    for (const auto& tag : object.tags()) {
        if (options().tags_filter(tag)) {
            has_tags = true;
            writer.Key(tag.key());
            writer.String(tag.value());
        }
    }
    writer.EndObject();

    append_pg_escaped(stream.GetString(), stream.GetSize());

    return has_tags;
}

void ExportFormatPg::finish_feature(const osmium::OSMObject& object) {
    m_buffer += '\t';
    add_attributes(object);

    if (add_tags(object) || options().keep_untagged) {
        m_buffer += '\n';

        m_commit_size = m_buffer.size();

        ++m_count;

        if (m_buffer.size() > flush_buffer_size) {
            flush_to_output();
        }
    }
}

void ExportFormatPg::node(const osmium::Node& node) {
    start_feature('n', node.id());
    m_buffer.append(m_factory.create_point(node));
    finish_feature(node);
}

void ExportFormatPg::way(const osmium::Way& way) {
    start_feature('w', way.id());
    m_buffer.append(m_factory.create_linestring(way));
    finish_feature(way);
}

void ExportFormatPg::area(const osmium::Area& area) {
    start_feature('a', area.id());
    m_buffer.append(m_factory.create_multipolygon(area));
    finish_feature(area);
}

void ExportFormatPg::close() {
    if (m_fd > 0) {
        flush_to_output();
        if (m_fsync == osmium::io::fsync::yes) {
            osmium::io::detail::reliable_fsync(m_fd);
        }
        ::close(m_fd);
        m_fd = -1;
    }
}

void ExportFormatPg::debug_output(osmium::VerboseOutput& out, const std::string& filename) {
    out << '\n';
    out << "Create table with something like this:\n";
    out << "CREATE TABLE osmdata (\n";

    if (options().unique_id == unique_id_type::counter) {
        out << "    id        BIGINT PRIMARY KEY,\n";
    } else if (options().unique_id == unique_id_type::type_id) {
        out << "    id        TEXT PRIMARY KEY,\n";
    }

    out << "    geom      GEOMETRY, -- or GEOGRAPHY\n";

    if (!options().type.empty()) {
        out << "    osm_type  TEXT,\n";
    }

    if (!options().id.empty()) {
        out << "    osm_id    BIGINT,\n";
    }

    if (!options().version.empty()) {
        out << "    version   INTEGER,\n";
    }

    if (!options().changeset.empty()) {
        out << "    changeset INTEGER,\n";
    }

    if (!options().uid.empty()) {
        out << "    uid       INTEGER,\n";
    }

    if (!options().user.empty()) {
        out << "    user      TEXT,\n";
    }

    if (!options().timestamp.empty()) {
        out << "    timestamp TIMESTAMP (0) WITH TIME ZONE,\n";
    }

    if (!options().way_nodes.empty()) {
        out << "    way_nodes BIGINT[],\n";
    }

    out << "    tags      JSONB -- or JSON, or TEXT\n";
    out << ");\n";
    out << "Then load data with something like this:\n";
    out << "\\copy osmdata FROM '" << filename << "'\n";
    out << '\n';
}

