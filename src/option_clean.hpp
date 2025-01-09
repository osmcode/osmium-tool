#ifndef OPTION_CLEAN_HPP
#define OPTION_CLEAN_HPP

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

#include <osmium/memory/buffer.hpp>

#include <boost/program_options.hpp>

#include <string>

class OptionClean {
    enum clean_options : uint8_t {
        clean_version   = 0x01,
        clean_changeset = 0x02,
        clean_timestamp = 0x04,
        clean_uid       = 0x08,
        clean_user      = 0x10
    };

    uint8_t m_clean_attrs = 0;

    void clean_buffer(osmium::memory::Buffer& buffer) const;

public:
    OptionClean() = default;

    void setup(const boost::program_options::variables_map& vm);

    void apply_to(osmium::memory::Buffer& buffer) const {
        if (m_clean_attrs) {
            clean_buffer(buffer);
        }
    }

    std::string to_string() const;
}; // class OptionClean

#endif // OPTION_CLEAN_HPP
