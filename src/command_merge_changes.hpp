#ifndef COMMAND_MERGE_CHANGES_HPP
#define COMMAND_MERGE_CHANGES_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2016  Jochen Topf <jochen@topf.org>

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

#include "cmd.hpp"

class CommandMergeChanges : public Command, public with_multiple_osm_inputs, public with_osm_output {

    bool m_simplify_change = false;

public:

    CommandMergeChanges() = default;

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

}; // class CommandMergeChanges


#endif // COMMAND_MERGE_CHANGES_HPP
