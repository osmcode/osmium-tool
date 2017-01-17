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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <osmium/util/minmax.hpp>

#include "cmd.hpp"

CommandFactory& CommandFactory::instance() {
    static CommandFactory factory;
    return factory;
}

std::vector<std::pair<std::string, std::string>> CommandFactory::help() {
    std::vector<std::pair<std::string, std::string>> commands;
    for (const auto& cmd : instance().m_commands) {
        commands.push_back(std::make_pair(cmd.first, cmd.second.description));
    }
    return commands;
}

int CommandFactory::max_command_name_length() {
    osmium::max_op<int> max_width;

    for (const auto& cmd : instance().m_commands) {
        max_width.update(int(cmd.first.length()));
    }

    return max_width();
}

bool CommandFactory::add(const std::string& name, const std::string& description, create_command_type create_function) {
    return instance().register_command(name, description, create_function);
}

std::string CommandFactory::get_description(const std::string& name) {
    auto it = instance().m_commands.find(name);
    if (it == instance().m_commands.end()) {
        return "";
    }
    return it->second.description;
}

bool CommandFactory::register_command(const std::string& name, const std::string& description, create_command_type create_function) {
    command_info info {description, create_function};
    return m_commands.insert(std::make_pair(name, info)).second;
}

std::unique_ptr<Command> CommandFactory::create_command(const std::string& name) {
    const auto it = m_commands.find(name);

    if (it != m_commands.end()) {
        return std::unique_ptr<Command>((it->second.create)());
    }

    return std::unique_ptr<Command>();
}

