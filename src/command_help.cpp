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

#include "command_help.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifndef _WIN32
# include <unistd.h>
#endif

bool CommandHelp::setup(const std::vector<std::string>& arguments) {
    m_topic = arguments.empty() ? "help" : arguments.front();
    return true;
}

namespace {

void show_help(const std::string& topic, const std::string& info) {
#ifndef _WIN32
    // show man page on non-Windows systems
    std::string manpage{"osmium-"};
    manpage += topic;
    ::execlp("man", "man", manpage.c_str(), nullptr);
    // if exec fails, fall thru
#endif
    // show info string and link on Windows systems
    std::cout << info << "\n";
    std::cout << "You'll find more documentation at https://osmcode.org/osmium-tool/\n";
}

} // anonymous namespace

bool CommandHelp::run() {
    const auto commands = m_command_factory.help();

    if (m_topic == "help") {
        std::cout << "Usage: " << synopsis()
                  << "\n\nCOMMANDS:\n";

        // print command names and descriptions in a nice table
        for (const auto& cmd : commands) {
            std::cout << "  "
                      << std::setw(m_command_factory.max_command_name_length())
                      << std::left
                      << cmd.first
                      << std::setw(0)
                      << "  "
                      << cmd.second
                      << "\n";
        }

        std::cout << "\nTOPICS:\n"
                     "  file-formats            File formats supported by Osmium\n"
                     "  index-types             Index types for storing node locations\n"
                     "  output-headers          Header options that can be set on output files\n";

        std::cout << "\nUse 'osmium COMMAND -h' for short usage information.\n"
                     "Use 'osmium help COMMAND' for detailed information on a specific command.\n"
                     "Use 'osmium help TOPIC' for detailed information on a specific topic.\n";
        return true;
    }

    const auto description = m_command_factory.get_description(m_topic);
    if (!description.empty()) {
        show_help(m_topic, std::string{"osmium "} + m_topic + ": " + description);
        return true;
    }

    if (m_topic == "file-formats") {
        show_help("file-formats", "osmium file-formats: Supported formats are 'xml', 'pbf', and 'opl'.");
        return true;
    }

    if (m_topic == "index-types") {
        show_help("index-types", "");
        return true;
    }

    if (m_topic == "output-headers") {
        show_help("output-headers", "");
        return true;
    }

    std::cerr << "Unknown help topic '" << m_topic << "'.\n";
    return false;
}

