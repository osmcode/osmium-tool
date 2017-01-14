#ifndef EXTRACT_EXTRACT_HPP
#define EXTRACT_EXTRACT_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

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

#include <memory>
#include <string>
#include <vector>

#include <osmium/osm/box.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/io/writer.hpp>

namespace osmium {
    class Area;
    class Box;
    class Location;
    class Segment;

    namespace io {
        class Header;
    }

    namespace memory {
        class Item;
    }
}

class Extract {

    std::string m_output;
    std::string m_output_format;
    std::string m_description;
    osmium::Box m_envelope;
    std::unique_ptr<osmium::io::Writer> m_writer;

public:

    Extract(const std::string& output, const std::string& output_format, const std::string& description, const osmium::Box& envelope) :
        m_output(output),
        m_output_format(output_format),
        m_description(description),
        m_envelope(envelope),
        m_writer(nullptr) {
    }

    virtual ~Extract() = default;

    const std::string& output() const noexcept {
        return m_output;
    }

    const std::string& output_format() const noexcept {
        return m_output_format;
    }

    const std::string& description() const noexcept {
        return m_description;
    }

    const osmium::Box& envelope() const noexcept {
        return m_envelope;
    }

    osmium::io::Writer& writer() {
        return *m_writer;
    }

    void open_file(const osmium::io::Header& header, osmium::io::overwrite output_overwrite, osmium::io::fsync sync);

    void close_file();

    void write(const osmium::memory::Item& item);

    std::string envelope_as_text() const;

    virtual bool contains(const osmium::Location& location) const noexcept = 0;

    virtual const char* geometry_type() const noexcept = 0;

    virtual std::string geometry_as_text() const = 0;

}; // class ExtractGeometry


class ExtractBBox : public Extract {

public:

    ExtractBBox(const std::string& output, const std::string& output_format, const std::string& description, const osmium::Box& box) :
        Extract(output, output_format, description, box) {
    }

    bool contains(const osmium::Location& location) const noexcept override final;

    const char* geometry_type() const noexcept override final;

    std::string geometry_as_text() const override final;

}; // class ExtractBBox

class ExtractPolygon : public Extract {

    const osmium::memory::Buffer& m_buffer;
    std::size_t m_offset;

    std::vector<osmium::Segment> m_segments;

    const osmium::Area& area() const noexcept;

public:

    ExtractPolygon(const std::string& output, const std::string& output_format, const std::string& description, const osmium::memory::Buffer& buffer, std::size_t offset);

    bool contains(const osmium::Location& location) const noexcept override final;

    const char* geometry_type() const noexcept override final;

    std::string geometry_as_text() const override final;

}; // class ExtractPolygon


#endif // EXTRACT_EXTRACT_HPP
