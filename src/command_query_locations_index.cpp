/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2019  Jochen Topf <jochen@topf.org>

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

#include "command_query_locations_index.hpp"
#include "exception.hpp"
#include "util.hpp"

#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

bool CommandQueryLocationsIndex::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("index-file,i", po::value<std::string>(), "Index file name (required)")
    ;

    po::options_description opts_common{add_common_options(false)};

    po::options_description hidden;
    hidden.add_options()
    ("node-id", po::value<osmium::object_id_type>(), "Node ID")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("node-id", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);

    if (vm.count("index-file")) {
        m_index_file_name = vm["index-file"].as<std::string>();
    } else {
        throw argument_error{"Missing --index-file,-i option."};
    }

    if (vm.count("node-id")) {
        m_id = vm["node-id"].as<osmium::object_id_type>();
    }

    return true;
}

void CommandQueryLocationsIndex::show_arguments() {
    m_vout << "  other options:\n";
    m_vout << "    index file: " << m_index_file_name << '\n';
}

bool CommandQueryLocationsIndex::run() {
    const int fd = ::open(m_index_file_name.c_str(), O_CREAT | O_RDWR, 0644); // NOLINT(hicpp-signed-bitwise)
    if (fd == -1) {
        throw std::runtime_error{std::string{"can't open file '"} + "filename" + "': " + std::strerror(errno)};
    }

    osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location> location_index{fd};

    const auto location = location_index.get(m_id);
    std::cout << location << "\n";

    m_vout << "Done.\n";

    return true;
}

