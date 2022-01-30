#ifndef EXPORT_EXPORT_FORMAT_JSON_HPP
#define EXPORT_EXPORT_FORMAT_JSON_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2022  Jochen Topf <jochen@topf.org>

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
#include <osmium/geom/rapid_geojson.hpp>
#include <osmium/io/writer_options.hpp>

#ifndef RAPIDJSON_HAS_STDSTRING
# define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <string>

using writer_type = rapidjson::Writer<rapidjson::StringBuffer>;

class ExportFormatJSON : public ExportFormat {

    int m_fd;
    osmium::io::fsync m_fsync;
    bool m_text_sequence_format;
    bool m_with_record_separator;
    rapidjson::StringBuffer m_stream;
    std::size_t m_committed_size = 0;
    writer_type m_writer;
    osmium::geom::RapidGeoJSONFactory<writer_type> m_factory;

    void flush_to_output();

    void rollback_uncomitted();

    void start_feature(const std::string& prefix, osmium::object_id_type id);
    void add_attributes(const osmium::OSMObject& object);
    void finish_feature(const osmium::OSMObject& object);

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
