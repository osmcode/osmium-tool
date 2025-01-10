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

#include "cmd.hpp"

#include <osmium/util/minmax.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

bool CommandFactory::register_command(const std::string& name, const std::string& description, create_command_type&& create_function) {
    const command_info info{description, std::move(create_function)};
    return m_commands.emplace(name, info).second;
}

std::vector<std::pair<std::string, std::string>> CommandFactory::help() const {
    std::vector<std::pair<std::string, std::string>> commands;
    commands.reserve(m_commands.size());

    for (const auto& cmd : m_commands) {
        commands.emplace_back(cmd.first, cmd.second.description);
    }

    return commands;
}

int CommandFactory::max_command_name_length() const {
    osmium::max_op<int> max_width;

    for (const auto& cmd : m_commands) {
        max_width.update(static_cast<int>(cmd.first.length()));
    }

    return max_width();
}

std::string CommandFactory::get_description(const std::string& name) const {
    const auto it = m_commands.find(name);

    if (it == m_commands.end()) {
        return "";
    }

    return it->second.description;
}

std::unique_ptr<Command> CommandFactory::create_command(const std::string& name) const {
    const auto it = m_commands.find(name);

    if (it == m_commands.end()) {
        return std::unique_ptr<Command>{};
    }

    return std::unique_ptr<Command>{(it->second.create)()};
}

