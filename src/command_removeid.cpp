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

#include "command_removeid.hpp"

#include "exception.hpp"
#include "id_file.hpp"
#include "util.hpp"

#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/string.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

bool CommandRemoveId::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("default-type", po::value<std::string>()->default_value("node"), "Default item type")
    ("id-file,i", po::value<std::vector<std::string>>(), "Read OSM IDs from text file")
    ("id-osm-file,I", po::value<std::vector<std::string>>(), "Read OSM IDs from OSM file")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("ids", po::value<std::vector<std::string>>(), "OSM IDs")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("ids", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("default-type")) {
        m_default_item_type = parse_item_type(vm["default-type"].as<std::string>());
    }

    if (vm.count("id-file")) {
        for (const std::string& filename : vm["id-file"].as<std::vector<std::string>>()) {
            if (filename == "-") {
                if (m_input_filename.empty() || m_input_filename == "-") {
                    throw argument_error{"Can not read OSM input and IDs both from STDIN."};
                }
                m_vout << "Reading IDs from STDIN...\n";
                read_id_file(std::cin, m_ids, m_default_item_type);
            } else {
                std::ifstream id_file{filename};
                if (!id_file.is_open()) {
                    throw argument_error{"Could not open file '" + filename + "'"};
                }
                m_vout << "Reading ID file...\n";
                read_id_file(id_file, m_ids, m_default_item_type);
            }
        }
    }

    if (vm.count("id-osm-file")) {
        for (const std::string& filename : vm["id-osm-file"].as<std::vector<std::string>>()) {
            m_vout << "Reading OSM ID file...\n";
            read_id_osm_file(filename, m_ids);
        }
    }

    if (vm.count("ids")) {
        std::string sids;
        for (const auto& s : vm["ids"].as<std::vector<std::string>>()) {
            sids += s + " ";
        }
        for (const auto& s : osmium::split_string(sids, "\t ;,/|", true)) {
            parse_and_add_id(s, m_ids, m_default_item_type);
        }
    }

    if (no_ids(m_ids)) {
        throw argument_error{"Please specify IDs to look for on command line or with option --id-file/-i or --id-osm-file/-I."};
    }

    return true;
}

void CommandRemoveId::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    default object type: " << osmium::item_type_to_name(m_default_item_type) << "\n";
    m_vout << "    removing " << m_ids(osmium::item_type::node).size() << " node ID(s), "
                              << m_ids(osmium::item_type::way).size() << " way ID(s), and "
                              << m_ids(osmium::item_type::relation).size() << " relation ID(s)\n";
}

bool CommandRemoveId::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::nwr};

    m_vout << "Opening output file...\n";
    osmium::io::Header header{reader.header()};
    setup_header(header);

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

    m_vout << "Copying non-matching objects to output file...\n";
    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        for (auto& object : buffer.select<osmium::OSMObject>()) {
            if (!m_ids(object.type()).get(object.positive_id())) {
                writer(object);
            }
        }
    }
    progress_bar.done();

    m_vout << "Closing output file...\n";
    writer.close();

    m_vout << "Closing input file...\n";
    reader.close();

    show_memory_used();

    m_vout << "Done.\n";

    return true;
}

