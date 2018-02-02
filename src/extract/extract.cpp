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

#include "extract.hpp"

#include <osmium/io/writer_options.hpp>

#include <sstream>
#include <string>

namespace osmium {

    namespace io {
        class Header;
    } // namespace io

    namespace memory {
        class Item;
    } // namespace memory

} // namespace osmium

void Extract::open_file(const osmium::io::Header& header, osmium::io::overwrite output_overwrite, osmium::io::fsync sync) {
    m_writer.reset(new osmium::io::Writer{m_output_file, header, output_overwrite, sync});
}

void Extract::close_file() {
    if (m_writer) {
        m_writer->close();
    }
}

void Extract::write(const osmium::memory::Item& item) {
    (*m_writer)(item);
}

std::string Extract::envelope_as_text() const {
    std::stringstream ss;
    ss << m_envelope;
    return ss.str();
}

