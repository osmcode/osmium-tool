#ifndef COMMAND_FILEINFO_HPP
#define COMMAND_FILEINFO_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013, 2014  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <string>

#include "osmc.hpp"

class CommandFileinfo : public Command {

    std::string m_input_filename = "-"; // default: stdin
    bool m_extended = false;
    std::string m_input_format;
    osmium::io::File m_input_file;

public:

    CommandFileinfo() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    bool run() override final;

}; // class CommandFileinfo


#endif // COMMAND_FILEINFO_HPP
