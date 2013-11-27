#ifndef COMMAND_APPLY_CHANGES_HPP
#define COMMAND_APPLY_CHANGES_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013  Jochen Topf <jochen@topf.org>

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

#include "osmc.hpp"

class CommandApplyChanges : public Command {

    std::string m_input_filename = "-"; // default: stdin
    std::string m_output_filename = "-"; // default: stdout
    std::vector<std::string> m_change_filenames;

    std::string m_input_format;
    std::string m_output_format;

    bool m_output_overwrite = false;
    bool m_simplify_change = false;
    bool m_remove_deleted = false;

    osmium::io::File m_output_file;

public:

    CommandApplyChanges() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    bool run() override final;

}; // class CommandApplyChanges


#endif // COMMAND_APPLY_CHANGES_HPP
