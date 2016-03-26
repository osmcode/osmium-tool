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

#include <osmium/util/memory.hpp>

#include "cmd.hpp"

po::options_description Command::add_common_options() {
    po::options_description options("COMMON OPTIONS");

    options.add_options()
    ("help,h", "Show usage help")
    ("verbose,v", "Set verbose mode")
    ;

    return options;
}

void Command::setup_common(const boost::program_options::variables_map& vm, const po::options_description& desc) {
    if (vm.count("help")) {
        std::cout << "Usage: " << synopsis() << "\n\n"
                  << CommandFactory::get_description(name()) << "\n"
                  << desc
                  << "\nUse 'osmium help " << name() << "' to display the manual page.\n";
        exit(0);
    }
    if (vm.count("verbose")) {
        m_vout.verbose(true);
    }
}

void Command::print_arguments(const std::string& command) {
    if (m_vout.verbose()) {
        m_vout << "Started osmium " << command << "\n";
        m_vout << "Command line options and default settings:\n";
        show_arguments();
    }
}

void Command::show_memory_used() {
    osmium::MemoryUsage mem;
    if (mem.current() > 0) {
        m_vout << "Peak memory used: " << mem.peak() << " MBytes\n";
    }
}

