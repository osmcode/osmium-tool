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

#include "command_query_locations_index.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

bool CommandQueryLocationsIndex::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("index-file,i", po::value<std::string>(), "Index file name (required)")
    ("dump", "Dump all locations to STDOUT")
    ;

    const po::options_description opts_common{add_common_options(false)};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("node-id", po::value<std::string>(), "Node ID")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("node-id", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    init_output_file(vm);

    if (vm.count("index-file")) {
        m_index_file_name = vm["index-file"].as<std::string>();
    } else {
        throw argument_error{"Missing --index-file,-i option."};
    }

    if (vm.count("dump")) {
        m_dump = true;
        if ((m_output_filename.empty() || m_output_filename == "-") && m_output_format.empty()) {
            m_output_format = "opl,add_metadata=none";
        }
        if (m_output_format.empty()) {
            m_output_file = osmium::io::File{m_output_filename};
            m_output_file.set("add_metadata", "none");
        } else {
            m_output_file = osmium::io::File{m_output_filename, m_output_format};
        }
        m_output_file.check();
    }

    if (vm.count("node-id")) {
        if (m_dump) {
            throw argument_error{"Either use --dump or use node ID, not both."};
        }
        const auto id = vm["node-id"].as<std::string>();
        const auto r = osmium::string_to_object_id(id.c_str(), osmium::osm_entity_bits::node, osmium::item_type::node);
        m_id = r.second;
    } else if (!m_dump) {
        throw argument_error{"Missing node ID on command line."};
    }

    return true;
}

void CommandQueryLocationsIndex::show_arguments() {
    show_output_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    index file: " << m_index_file_name << '\n';
}

bool CommandQueryLocationsIndex::run() {
    const int fd = ::open(m_index_file_name.c_str(), O_RDWR);
    if (fd == -1) {
        throw std::system_error{errno, std::system_category(), std::string("Can not open index file '") + m_index_file_name + "'"};
    }

    const osmium::index::map::DenseFileArray<osmium::unsigned_object_id_type, osmium::Location> location_index{fd};

    if (m_dump) {
        const std::size_t max_buffer_size = 11UL * 1024UL * 1024UL;
        osmium::memory::Buffer buffer{max_buffer_size};

        m_vout << "Opening output file...\n";
        osmium::io::Header header{};
        setup_header(header);
        osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

        m_vout << "Dumping index as OSM file...\n";
        for (std::size_t i = 0; i < location_index.size(); ++i) {
            if (location_index.get_noexcept(i).valid()) {
                osmium::builder::NodeBuilder builder{buffer};
                builder.set_id(static_cast<osmium::object_id_type>(i));
                builder.set_location(location_index.get_noexcept(i));
            }
            buffer.commit();
            if (buffer.committed() > 10UL * 1024UL * 1024UL) {
                writer(std::move(buffer));
                buffer = osmium::memory::Buffer{max_buffer_size};
            }
        }
        if (buffer.committed() > 0) {
            writer(std::move(buffer));
        }
    } else {
        m_vout << "Looking up location in index...\n";
        const auto location = location_index.get(m_id);
        std::cout << location << '\n';
    }

    m_vout << "Done.\n";

    return true;
}

