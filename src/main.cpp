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

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
# include <fcntl.h>
# include <io.h>
#endif

#include <osmium/version.hpp>

#include "cmd.hpp"

enum return_code : int {
    okay  = 0,
    error = 1,
    fatal = 2
};

int main(int argc, char *argv[]) {
#ifdef _WIN32
    _setmode(1, _O_BINARY);
#endif

    std::string command = argv[0];

    // remove path from command
    // (backslash for windows, slash for everybody else)
    if (command.find_last_of("/\\") != std::string::npos) {
        command = command.substr(command.find_last_of("/\\") + 1);
    }

    std::vector<std::string> arguments;

    for (int i = 1; i < argc; ++i) {
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
        std::cout << get_osmium_long_version() << '\n'
                  << get_libosmium_version() << '\n'
                  << "Copyright (C) 2013-2016  Jochen Topf <jochen@topf.org>\n"
                  << "License: GNU GENERAL PUBLIC LICENSE Version 3 <http://gnu.org/licenses/gpl.html>.\n"
                  << "This is free software: you are free to change and redistribute it.\n"
                  << "There is NO WARRANTY, to the extent permitted by law.\n";

        return return_code::okay;
    }

    std::unique_ptr<Command> cmd = CommandFactory::instance().create_command(command);

    if (!cmd) {
        std::cerr << "Unknown command or option '" << command << "'. Try 'osmium help'.\n";
        return return_code::fatal;
    }

    try {
        if (!cmd->setup(arguments)) {
            return return_code::okay;
        }
    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        return return_code::fatal;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return return_code::fatal;
    }

    cmd->print_arguments(command);

    try {
        if (cmd->run()) {
            return return_code::okay;
        }
    } catch (std::bad_alloc& e) {
        std::cerr << "Out of memory. Read the MEMORY USAGE section of the osmium(1) manpage.\n";
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return return_code::error;
}

