#ifndef EXTRACT_EXTRACT_HPP
#define EXTRACT_EXTRACT_HPP

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

#include <osmium/osm/box.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/io/writer.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace osmium {

    class Box;
    class Location;

    namespace io {
        class Header;
    } // namespace io

    namespace memory {
        class Item;
    } // namespace memory

} // namespace osmium

class Extract {

    osmium::io::File m_output_file;
    std::string m_description;
    std::vector<std::pair<std::string, std::string>> m_header_options;
    osmium::Box m_envelope;
    std::unique_ptr<osmium::io::Writer> m_writer;

public:

    Extract(const osmium::io::File& output_file, const std::string& description, const osmium::Box& envelope) :
        m_output_file(output_file),
        m_description(description),
        m_envelope(envelope),
        m_writer(nullptr) {
    }

    virtual ~Extract() = default;

    const std::string& output() const noexcept {
        return m_output_file.filename();
    }

    const char* output_format() const noexcept {
        return osmium::io::as_string(m_output_file.format());
    }

    const std::string& description() const noexcept {
        return m_description;
    }

    const osmium::Box& envelope() const noexcept {
        return m_envelope;
    }

    void add_header_option(const std::string& name, const std::string& value) {
        m_header_options.emplace_back(name, value);
    }

    const std::vector<std::pair<std::string, std::string>>& header_options() const noexcept {
        return m_header_options;
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

}; // class Extract

#endif // EXTRACT_EXTRACT_HPP
