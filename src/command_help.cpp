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

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <map>
#include <numeric>
#include <string>
#include <unistd.h>

#include "command_help.hpp"

bool CommandHelp::setup(const std::vector<std::string>& arguments) {
    m_topic = arguments.empty() ? "help" : arguments.front();
    return true;
}

bool CommandHelp::run() {
    if (m_topic == "help") {
        std::cout << "Usage: osmium [--version] [--help] <command> [<args>]\n\nCommands are:\n";

        auto commands = CommandFactory::help();

        // find the maximum length of all command names
        size_t max_width = std::accumulate(commands.begin(), commands.end(), 0, [](size_t max_so_far, std::pair<std::string, std::string> info) {
            return std::max(max_so_far, info.first.length());
        });

        // and print them out in a nice table
        for (const auto& cmd : commands) {
            std::cout << "  " << std::setw(max_width) << std::left << cmd.first << std::setw(0) << "  " << cmd.second << "\n";
        }

        std::cout << "\nSee 'osmium help <command>' for more information on a specific command." << std::endl;
        return true;
    }

    // show man page
    std::string manpage("osmium-");
    manpage += m_topic;
    ::execlp("man", "man", manpage.c_str(), nullptr);

    std::cerr << "Executing man command failed: " << std::strerror(errno) << std::endl;

    return false;
}

namespace {

    const bool register_help_command = CommandFactory::add("help", "Show osmium help", []() {
        return new CommandHelp();
    });

}

