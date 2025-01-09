#ifndef EXPORT_EXPORT_FORMAT_JSON_HPP
#define EXPORT_EXPORT_FORMAT_JSON_HPP

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

#include "export_format.hpp"

#include <osmium/fwd.hpp>
#include <osmium/io/writer_options.hpp>

#include <nlohmann/json.hpp>

#include <string>

class ExportFormatJSON : public ExportFormat {

    int m_fd;
    osmium::io::fsync m_fsync;
    bool m_text_sequence_format;
    bool m_with_record_separator;
    std::string m_buffer;
    std::size_t m_committed_size = 0;

    void flush_to_output();

    void rollback_uncomitted();

    void add_option(const std::string& name);

    void start_feature(const std::string& prefix, osmium::object_id_type id);
    void add_attributes(const osmium::OSMObject& object);
    void finish_feature(const osmium::OSMObject& object);

    void create_coordinate(const osmium::Location& location);
    void create_coordinate_list(const osmium::NodeRefList& nrl);

    void create_point(const osmium::Node& node);
    void create_linestring(const osmium::Way& way);
    void create_multipolygon(const osmium::Area& area);

public:

    ExportFormatJSON(const std::string& output_format,
                     const std::string& output_filename,
                     osmium::io::overwrite overwrite,
                     osmium::io::fsync fsync,
                     const options_type& options);

    ~ExportFormatJSON() override {
        close();
    }

    void node(const osmium::Node& node) override;

    void way(const osmium::Way& way) override;

    void area(const osmium::Area& area) override;

    void close() override;

}; // class ExportFormatJSON

#endif // EXPORT_EXPORT_FORMAT_JSON_HPP
