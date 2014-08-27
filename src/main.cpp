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

#include <iostream>
#include <string>
#include <vector>

#include <osmpbf/osmpbf.h>

#include "osmc.hpp"

#include "command_apply_changes.hpp"
#include "command_cat.hpp"
#include "command_fileinfo.hpp"
#include "command_help.hpp"
#include "command_merge_changes.hpp"
#include "command_time_filter.hpp"

enum return_code : int {
    okay  = 0,
    error = 1,
    fatal = 2
};

int main(int argc, char *argv[]) {
    std::string command = argv[0];

    // remove path from command
    // (backslash for windows, slash for everybody else)
    if (command.find_last_of("/\\") != std::string::npos) {
        command = command.substr(command.find_last_of("/\\") + 1);
    }

    std::vector<std::string> arguments;

    for (int i=1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    if (command == "osmium" || command == "osmium.exe") {
        if (arguments.size() == 0) {
            command = "help";
        } else {
            if (arguments.front() == "--help" || arguments.front() == "-h") {
                command = "help";
            } else if (arguments.front() == "--version") {
                command = "version";
            } else {
                command = arguments.front();
            }
            arguments.erase(arguments.begin());
        }
    } else {
        if (command.substr(0, 7) == "osmium-") {
            command = command.substr(7);
        }
    }

    if (command == "version") {
        std::cout << "osmium version " << OSMIUM_VERSION << std::endl;
        return return_code::okay;
    }

    std::unique_ptr<Command> cmd = CommandFactory::instance().create_command(command);

    if (!cmd) {
        std::cerr << "Unknown command '" << command << "'. Try 'osmium help'." << std::endl;
        return return_code::fatal;
    }

    if (!cmd->setup(arguments)) {
        return return_code::fatal;
    }

    bool result = cmd->run();

    google::protobuf::ShutdownProtobufLibrary();

    if (result) {
        return return_code::okay;
    } else {
        return return_code::error;
    }
}

