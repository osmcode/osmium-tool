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

#include "extract.hpp"

#include <osmium/io/writer_options.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

void Extract::open_file(const osmium::io::Header& header, osmium::io::overwrite output_overwrite, osmium::io::fsync sync, OptionClean const* clean) {
    m_clean = clean;
    m_writer = std::make_unique<osmium::io::Writer>(m_output_file, header, output_overwrite, sync);
}

void Extract::close_file() {
    if (m_writer) {
        if (m_buffer.committed() > 0) {
            m_clean->apply_to(m_buffer);
            (*m_writer)(std::move(m_buffer));
        }
        m_writer->close();
    }
}

void Extract::write(const osmium::memory::Item& item) {
    if (m_buffer.capacity() - m_buffer.committed() < item.padded_size()) {
        m_clean->apply_to(m_buffer);
        (*m_writer)(std::move(m_buffer));
        m_buffer = osmium::memory::Buffer{buffer_size, osmium::memory::Buffer::auto_grow::no};
    }
    m_buffer.push_back(item);
}

std::string Extract::envelope_as_text() const {
    std::stringstream ss;
    ss << m_envelope;
    return ss.str();
}

