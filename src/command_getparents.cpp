/*

Osmium -- OpenStreetMap data manipulation command line tool
https://osmcode.org/osmium-tool/

Copyright (C) 2013-2018  Jochen Topf <jochen@topf.org>

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

#include "command_getparents.hpp"
#include "exception.hpp"
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

#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void CommandGetParents::parse_and_add_id(const std::string& s) {
    auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr, m_default_item_type);
    if (p.second < 0) {
        throw std::runtime_error{"osmium-getid does not work with negative IDs"};
    }
    m_ids(p.first).set(p.second);
}

void CommandGetParents::read_id_file(std::istream& stream) {
    m_vout << "Reading ID file...\n";
    for (std::string line; std::getline(stream, line);) {
        strip_whitespace(line);
        const auto pos = line.find_first_of(" #");
        if (pos != std::string::npos) {
            line.erase(pos);
        }
        if (!line.empty()) {
            parse_and_add_id(line);
        }
    }
}

bool CommandGetParents::no_ids() const {
    return m_ids(osmium::item_type::node).empty() &&
           m_ids(osmium::item_type::way).empty() &&
           m_ids(osmium::item_type::relation).empty();
}

bool CommandGetParents::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("default-type", po::value<std::string>()->default_value("node"), "Default item type")
    ("id-file,i", po::value<std::vector<std::string>>(), "Read OSM IDs from text file")
    ("id-osm-file,I", po::value<std::vector<std::string>>(), "Read OSM IDs from OSM file")
    ("add-self,s", "Add objects with specified IDs themselves")
    ("verbose-ids", "Print all requested IDs")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};
    po::options_description opts_output{add_output_options()};

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

    setup_common(vm, desc);
    setup_progress(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("add-self")) {
        m_add_self = true;
    }

    if (vm.count("default-type")) {
        std::string t{vm["default-type"].as<std::string>()};

        if (t == "n" || t == "node") {
            m_default_item_type = osmium::item_type::node;
        } else if (t == "w" || t == "way") {
            m_default_item_type = osmium::item_type::way;
        } else if (t == "r" || t == "relation") {
            m_default_item_type = osmium::item_type::relation;
        } else {
            throw argument_error{std::string{"Unknown default type '"} + t + "' (Allowed are 'node', 'way', and 'relation')."};
        }
    }

    if (vm.count("verbose-ids")) {
        m_vout.verbose(true);
        m_verbose_ids = true;
    }

    if (vm.count("id-file")) {
        for (const std::string& filename : vm["id-file"].as<std::vector<std::string>>()) {
            if (filename == "-") {
                if (m_input_filename.empty() || m_input_filename == "-") {
                    throw argument_error{"Can not read OSM input and IDs both from STDIN."};
                }
                read_id_file(std::cin);
            } else {
                std::ifstream id_file{filename};
                if (!id_file.is_open()) {
                    throw argument_error{"Could not open file '" + filename + "'"};
                }
                read_id_file(id_file);
            }
        }
    }

    if (vm.count("id-osm-file")) {
        for (const std::string& filename : vm["id-osm-file"].as<std::vector<std::string>>()) {
            read_id_osm_file(filename);
        }
    }

    if (vm.count("ids")) {
        std::string sids;
        for (const auto& s : vm["ids"].as<std::vector<std::string>>()) {
            sids += s + " ";
        }
        for (const auto& s : osmium::split_string(sids, "\t ;,/|", true)) {
            parse_and_add_id(s);
        }
    }

    if (no_ids()) {
        throw argument_error{"Please specify IDs to look for on command line or with option --id-file/-i or --id-osm-file/-I."};
    }

    return true;
}

void CommandGetParents::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    add self: " << yes_no(m_add_self);
    m_vout << "    default object type: " << osmium::item_type_to_name(m_default_item_type) << "\n";
    if (m_verbose_ids) {
        m_vout << "    looking for these ids:\n";
        m_vout << "      nodes:";
        for (osmium::object_id_type id : m_ids(osmium::item_type::node)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      ways:";
        for (osmium::object_id_type id : m_ids(osmium::item_type::way)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      relations:";
        for (osmium::object_id_type id : m_ids(osmium::item_type::relation)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
    } else {
        m_vout << "    looking for " << m_ids(osmium::item_type::node).size() << " node ID(s), "
                                     << m_ids(osmium::item_type::way).size() << " way ID(s), and "
                                     << m_ids(osmium::item_type::relation).size() << " relation ID(s)\n";
    }
}

osmium::osm_entity_bits::type CommandGetParents::get_needed_types() const {
    osmium::osm_entity_bits::type types = osmium::osm_entity_bits::relation;

    if (!m_ids(osmium::item_type::node).empty()) {
        if (m_add_self) {
            types |= osmium::osm_entity_bits::node;
        }
        types |= osmium::osm_entity_bits::way;
    }
    if (!m_ids(osmium::item_type::way).empty()) {
        if (m_add_self) {
            types |= osmium::osm_entity_bits::way;
        }
    }

    return types;
}

void CommandGetParents::add_nodes(const osmium::Way& way) {
    for (const auto& nr : way.nodes()) {
        m_ids(osmium::item_type::node).set(nr.positive_ref());
    }
}

void CommandGetParents::add_members(const osmium::Relation& relation) {
    for (const auto& member : relation.members()) {
        m_ids(member.type()).set(member.positive_ref());
    }
}

void CommandGetParents::read_id_osm_file(const std::string& file_name) {
    m_vout << "Reading OSM ID file...\n";
    osmium::io::Reader reader{file_name, osmium::osm_entity_bits::object};
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            m_ids(object.type()).set(object.positive_id());
            if (object.type() == osmium::item_type::way) {
                add_nodes(static_cast<const osmium::Way&>(object));
            } else if (object.type() == osmium::item_type::relation) {
                add_members(static_cast<const osmium::Relation&>(object));
            }
        }
    }
    reader.close();
}

bool CommandGetParents::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file, get_needed_types()};

    m_vout << "Opening output file...\n";
    osmium::io::Header header = reader.header();
    setup_header(header);

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

    m_vout << "Copying matching objects to output file...\n";
    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            if (m_add_self && m_ids(object.type()).get(object.positive_id())) {
                writer(object);
                continue;
            }
            if (object.type() == osmium::item_type::way) {
                const auto& way = static_cast<const osmium::Way&>(object);
                for (const auto& nr : way.nodes()) {
                    if (m_ids(osmium::item_type::node).get(nr.positive_ref())) {
                        writer(object);
                        break;
                    }
                }
            } else if (object.type() == osmium::item_type::relation) {
                const auto& relation = static_cast<const osmium::Relation&>(object);
                for (const auto& member : relation.members()) {
                    if (m_ids(member.type()).get(member.positive_ref())) {
                        writer(object);
                        break;
                    }
                }
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

