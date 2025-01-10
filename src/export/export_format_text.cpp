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

#include "export_format_text.hpp"

#include "../util.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/string_util.hpp>

#include <cstdlib>
#include <string>

static constexpr const std::size_t initial_buffer_size = 1024UL * 1024UL;
static constexpr const std::size_t flush_buffer_size   =  800UL * 1024UL;

ExportFormatText::ExportFormatText(const std::string& /*output_format*/,
                                   const std::string& output_filename,
                                   osmium::io::overwrite overwrite,
                                   osmium::io::fsync fsync,
                                   const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync) {
    m_buffer.reserve(initial_buffer_size);
}

void ExportFormatText::flush_to_output() {
    osmium::io::detail::reliable_write(m_fd, m_buffer.data(), m_buffer.size());
    m_buffer.clear();
    m_commit_size = 0;
}

void ExportFormatText::start_feature(char type, osmium::object_id_type id) {
    m_buffer.resize(m_commit_size);
    if (options().unique_id == unique_id_type::counter) {
        m_buffer.append(std::to_string(m_count + 1));
        m_buffer.append(1, ' ');
    } else if (options().unique_id == unique_id_type::type_id) {
        m_buffer.append(1, type);
        m_buffer.append(std::to_string(id));
        m_buffer.append(1, ' ');
    }
}

void ExportFormatText::add_attributes(const osmium::OSMObject& object) {
    if (!options().type.empty()) {
        m_buffer.append(options().type);
        m_buffer.append(1, '=');
        m_buffer.append(object_type_as_string(object));
        m_buffer.append(1, ',');
    }

    if (!options().id.empty()) {
        m_buffer.append(options().id);
        m_buffer.append(1, '=');
        m_buffer.append(std::to_string(object.type() == osmium::item_type::area ? osmium::area_id_to_object_id(object.id()) : object.id()));
        m_buffer.append(1, ',');
    }

    if (!options().version.empty()) {
        m_buffer.append(options().version);
        m_buffer.append(1, '=');
        m_buffer.append(std::to_string(object.version()));
        m_buffer.append(1, ',');
    }

    if (!options().changeset.empty()) {
        m_buffer.append(options().changeset);
        m_buffer.append(1, '=');
        m_buffer.append(std::to_string(object.changeset()));
        m_buffer.append(1, ',');
    }

    if (!options().uid.empty()) {
        m_buffer.append(options().uid);
        m_buffer.append(1, '=');
        m_buffer.append(std::to_string(object.uid()));
        m_buffer.append(1, ',');
    }

    if (!options().user.empty()) {
        m_buffer.append(options().user);
        m_buffer.append(1, '=');
        m_buffer.append(object.user());
        m_buffer.append(1, ',');
    }

    if (!options().timestamp.empty()) {
        m_buffer.append(options().timestamp);
        m_buffer.append(1, '=');
        m_buffer.append(std::to_string(object.timestamp().seconds_since_epoch()));
        m_buffer.append(1, ',');
    }

    if (!options().way_nodes.empty() && object.type() == osmium::item_type::way) {
        m_buffer.append(options().way_nodes);
        m_buffer.append(1, '=');
        for (const auto& nr : static_cast<const osmium::Way&>(object).nodes()) {
            m_buffer.append(std::to_string(nr.ref()));
            m_buffer.append(1, '/');
        }
        if (m_buffer.back() == '/') {
            m_buffer.resize(m_buffer.size() - 1);
        }
    }
}

void ExportFormatText::finish_feature(const osmium::OSMObject& object) {
    m_buffer.append(1, ' ');

    add_attributes(object);

    const bool has_tags = add_tags(object, [&](const osmium::Tag& tag) {
        osmium::io::detail::append_utf8_encoded_string(m_buffer, tag.key());
        m_buffer.append(1, '=');
        osmium::io::detail::append_utf8_encoded_string(m_buffer, tag.value());
        m_buffer.append(1, ',');
    });

    if (has_tags || options().keep_untagged) {
        if (m_buffer.back() == ',') {
            m_buffer.back() = '\n';
        } else {
            m_buffer.append(1, '\n');
        }

        m_commit_size = m_buffer.size();

        ++m_count;

        if (m_buffer.size() > flush_buffer_size) {
            flush_to_output();
        }
    }
}

void ExportFormatText::node(const osmium::Node& node) {
    start_feature('n', node.id());
    m_buffer.append(m_factory.create_point(node));
    finish_feature(node);
}

void ExportFormatText::way(const osmium::Way& way) {
    start_feature('w', way.id());
    m_buffer.append(m_factory.create_linestring(way));
    finish_feature(way);
}

void ExportFormatText::area(const osmium::Area& area) {
    start_feature('a', area.id());
    m_buffer.append(m_factory.create_multipolygon(area));
    finish_feature(area);
}

void ExportFormatText::close() {
    if (m_fd > 0) {
        flush_to_output();
        if (m_fsync == osmium::io::fsync::yes) {
            osmium::io::detail::reliable_fsync(m_fd);
        }
        ::close(m_fd);
        m_fd = -1;
    }
}

