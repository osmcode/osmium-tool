#ifndef COMMAND_TIME_FILTER_HPP
#define COMMAND_TIME_FILTER_HPP

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
#include <vector>

#include <osmium/io/file.hpp>
#include <osmium/io/overwrite.hpp>
#include <osmium/osm/timestamp.hpp>

#include "osmc.hpp"

class CommandTimeFilter : public Command {

    std::string m_input_filename = "-"; // default: stdin
    std::string m_output_filename = "-"; // default: stdout

    std::string m_input_format;
    std::string m_output_format;

    osmium::io::overwrite m_output_overwrite = osmium::io::overwrite::no;

    osmium::io::File m_input_file;
    osmium::io::File m_output_file;

    osmium::Timestamp m_from;
    osmium::Timestamp m_to;

public:

    CommandTimeFilter() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    bool run() override final;

}; // class CommandTimeFilter


#endif // COMMAND_TIME_FILTER_HPP
