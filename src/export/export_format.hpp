#ifndef EXPORT_EXPORT_FORMAT_HPP
#define EXPORT_EXPORT_FORMAT_HPP

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

#include <cstdint>

#include <osmium/fwd.hpp>
#include <osmium/io/writer_options.hpp>

#include "options.hpp"

class ExportFormat {

    const options_type& m_options;

protected:

    std::uint64_t m_count;

    explicit ExportFormat(const options_type& options) :
        m_options(options),
        m_count(0) {
    }

public:

    const options_type& options() const noexcept {
         return m_options;
    }

    std::uint64_t count() const noexcept {
        return m_count;
    }

    virtual ~ExportFormat() = default;

    virtual void node(const osmium::Node&) = 0;

    virtual void way(const osmium::Way&) = 0;

    virtual void area(const osmium::Area&) = 0;

    virtual void close() = 0;

}; // class ExportFormat


#endif // EXPORT_EXPORT_FORMAT_HPP
