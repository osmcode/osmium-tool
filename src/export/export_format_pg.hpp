#ifndef EXPORT_EXPORT_FORMAT_PG_HPP
#define EXPORT_EXPORT_FORMAT_PG_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

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
#include <osmium/geom/wkb.hpp>
#include <osmium/io/writer_options.hpp>

#include <string>

class ExportFormatPg : public ExportFormat {

    enum tags_output_format {
        json,
        hstore
    };

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::ewkb, osmium::geom::out_type::hex};
    std::string m_buffer;
    std::size_t m_commit_size = 0;
    int m_fd;
    osmium::io::fsync m_fsync;

    tags_output_format m_tags_type = tags_output_format::json;

    void flush_to_output();

    void start_feature(char type, osmium::object_id_type id);
    void add_attributes(const osmium::OSMObject& object);
    bool add_tags_json(const osmium::OSMObject& object);
    bool add_tags_hstore(const osmium::OSMObject& object);
    bool add_tags(const osmium::OSMObject& object);
    void finish_feature(const osmium::OSMObject& object);
    void append_pg_escaped(const char* str);

public:

    ExportFormatPg(const std::string& output_format,
                   const std::string& output_filename,
                   osmium::io::overwrite overwrite,
                   osmium::io::fsync fsync,
                   const options_type& options);

    ~ExportFormatPg() override {
        try {
            close();
        } catch (...) {
        }
    }

    void node(const osmium::Node& node) override;

    void way(const osmium::Way& way) override;

    void area(const osmium::Area& area) override;

    void close() override;

    void debug_output(osmium::VerboseOutput& out, const std::string& filename) override;

}; // class ExportFormatPg

#endif // EXPORT_EXPORT_FORMAT_PG_HPP
