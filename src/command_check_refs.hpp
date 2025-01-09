#ifndef COMMAND_CHECK_REFS_HPP
#define COMMAND_CHECK_REFS_HPP

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

#include "cmd.hpp" // IWYU pragma: export

#include <string>
#include <vector>

class CommandCheckRefs : public CommandWithSingleOSMInput {

    bool m_show_ids = false;
    bool m_check_relations = false;

public:

    explicit CommandCheckRefs(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "check-refs";
    }

    const char* synopsis() const noexcept override final {
        return "osmium check-refs [OPTIONS] OSM-DATA-FILE";
    }

}; // class CommandCheckRefs


#endif // COMMAND_CHECK_REFS_HPP
