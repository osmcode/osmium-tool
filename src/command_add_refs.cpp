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

#include <fstream>

#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include "command_add_refs.hpp"
#include "exception.hpp"

void CommandAddRefs::parse_and_add_id(const std::string& s) {
    auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr);
    switch (p.first) {
        case osmium::item_type::undefined:
            /* fallthrough */
        case osmium::item_type::node:
            m_node_ids.insert(p.second);
            break;
        case osmium::item_type::way:
            m_way_ids.insert(p.second);
            break;
        case osmium::item_type::relation:
            m_relation_ids.insert(p.second);
            break;
        default:
            break;
    }
}

bool CommandAddRefs::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Available options");
    cmdline.add_options()
    ("id-file,i", po::value<std::string>(), "Read OSM IDs from given file")
    ("source,s", po::value<std::string>(), "Source file supplying the referenced objects")
    ("source-format", po::value<std::string>(), "Format of source file")
    ;

    add_common_options(cmdline);
    add_multiple_inputs_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "OSM input files")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: osmium add-refs [OPTIONS] -s OSM-DATA-FILE OSM-DATA-FILE...\n"
                  << "       osmium add-refs [OPTIONS] -s OSM-DATA-FILE -i ID-FILE\n\n";
        std::cout << cmdline << "\n";
        exit(0);
    }

    setup_common(vm);
    setup_input_files(vm, true);
    setup_output_file(vm);

    if (vm.count("id-file")) {
        std::string filename = vm["id-file"].as<std::string>();

        std::ifstream id_file{filename};
        if (!id_file.is_open()) {
            throw argument_error("Could not open file '" + filename + "'");
        }

        for (std::string line; std::getline(id_file, line); ) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            auto pos = line.find(' ');
            if (pos != std::string::npos) {
                line = line.erase(pos);
            }
            parse_and_add_id(line);
        }
    }

    if (vm.count("source")) {
        m_source_filename = vm["source"].as<std::string>();
    }

    if (vm.count("source-format")) {
        m_source_format = vm["source-format"].as<std::string>();
    }

    m_source_file = osmium::io::File{m_source_filename, m_source_format};

    return true;
}

void CommandAddRefs::show_arguments() {
    m_vout << "  source file name: " << m_source_filename << "\n";
    m_vout << "  source file format: " << m_source_format << "\n";
    m_vout << "  input file names: \n";
    for (const auto& fn : m_input_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  input format: " << m_input_format << "\n";
    show_output_arguments(m_vout);
    m_vout << "  looking for these ids:\n";
    m_vout << "    nodes:";
    for (osmium::object_id_type id : m_node_ids) {
        m_vout << " " << id;
    }
    m_vout << "\n";
    m_vout << "    ways:";
    for (osmium::object_id_type id : m_way_ids) {
        m_vout << " " << id;
    }
    m_vout << "\n";
    m_vout << "    relations:";
    for (osmium::object_id_type id : m_relation_ids) {
        m_vout << " " << id;
    }
    m_vout << "\n";
}

void CommandAddRefs::add_nodes(const osmium::Way& way) {
    for (const auto& nr : way.nodes()) {
        m_node_ids.insert(nr.ref());
    }
}

osmium::osm_entity_bits::type CommandAddRefs::add_members(const osmium::Relation& relation) {
    osmium::osm_entity_bits::type read_types = osmium::osm_entity_bits::nothing;

    for (const auto& member : relation.members()) {
        switch (member.type()) {
            case osmium::item_type::node:
                m_node_ids.insert(member.ref());
                break;
            case osmium::item_type::way:
                if (m_way_ids.count(member.ref()) == 0) {
                    m_way_ids.insert(member.ref());
                    read_types |= osmium::osm_entity_bits::way;
                }
                break;
            case osmium::item_type::relation:
                if (m_relation_ids.count(member.ref()) == 0) {
                    m_relation_ids.insert(member.ref());
                    read_types |= osmium::osm_entity_bits::relation;
                }
                break;
            default:
                break;
        }
    }

    return read_types;
}

void print_missing_ids(const char* type, std::set<osmium::object_id_type>& set) {
    if (set.empty()) {
        return;
    }
    std::cerr << "Missing " << type << " IDs:";
    for (const auto& id : set) {
        std::cerr << ' ' << id;
    }
    std::cerr << '\n';
}

void CommandAddRefs::read_input_files() {
    m_vout << "Reading input files...\n";
    for (const std::string& file_name : m_input_filenames) {
        osmium::io::Reader reader(file_name, osmium::osm_entity_bits::object);
        while (osmium::memory::Buffer buffer = reader.read()) {
            for (auto it = buffer.begin<osmium::OSMObject>(); it != buffer.end<osmium::OSMObject>(); ++it) {
                if (it->type() == osmium::item_type::node) {
                    m_node_ids.insert(it->id());
                } else if (it->type() == osmium::item_type::way) {
                    m_way_ids.insert(it->id());
                    add_nodes(static_cast<const osmium::Way&>(*it));
                } else if (it->type() == osmium::item_type::relation) {
                    m_relation_ids.insert(it->id());
                    add_members(static_cast<const osmium::Relation&>(*it));
                }
            }
        }
        reader.close();
    }
}

void CommandAddRefs::mark_rel_ids(const std::multimap<osmium::object_id_type, osmium::object_id_type>& rel_in_rel, osmium::object_id_type id) {
    auto range = rel_in_rel.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        if (m_relation_ids.count(it->second) == 0) {
            m_relation_ids.insert(it->second);
            mark_rel_ids(rel_in_rel, it->second);
        }
    }
}

bool CommandAddRefs::find_relations_in_relations() {
    m_vout << "Reading source file to find relations in relations...\n";
    std::multimap<osmium::object_id_type, osmium::object_id_type> rel_in_rel;

    osmium::io::Reader reader(m_source_file, osmium::osm_entity_bits::relation);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (auto it = buffer.begin<osmium::Relation>(); it != buffer.end<osmium::Relation>(); ++it) {
            for (const auto& member : it->members()) {
                if (member.type() == osmium::item_type::relation) {
                    rel_in_rel.emplace(it->id(), member.ref());
                } else if (m_relation_ids.count(it->id())) {
                    if (member.type() == osmium::item_type::node) {
                        m_node_ids.insert(member.ref());
                    } else if (member.type() == osmium::item_type::way) {
                        m_way_ids.insert(member.ref());
                    }
                }
            }
        }
    }
    reader.close();

    if (rel_in_rel.empty()) {
        return false;
    }

    for (const osmium::object_id_type id : m_relation_ids) {
        mark_rel_ids(rel_in_rel, id);
    }

    return true;
}

void CommandAddRefs::find_nodes_and_ways_in_relations() {
    m_vout << "Reading source file to find nodes/ways in relations...\n";

    osmium::io::Reader reader(m_source_file, osmium::osm_entity_bits::relation);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (auto it = buffer.begin<osmium::Relation>(); it != buffer.end<osmium::Relation>(); ++it) {
            if (m_relation_ids.count(it->id())) {
                for (const auto& member : it->members()) {
                    if (member.type() == osmium::item_type::node) {
                        m_node_ids.insert(member.ref());
                    } else if (member.type() == osmium::item_type::way) {
                        m_way_ids.insert(member.ref());
                    }
                }
            }
        }
    }
    reader.close();
}

void CommandAddRefs::find_nodes_in_ways() {
    m_vout << "Reading source file to find nodes in ways...\n";

    osmium::io::Reader reader(m_source_file, osmium::osm_entity_bits::way);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (auto it = buffer.begin<osmium::Way>(); it != buffer.end<osmium::Way>(); ++it) {
            if (m_way_ids.count(it->id())) {
                add_nodes(*it);
            }
        }
    }
    reader.close();
}

bool CommandAddRefs::run() {
    if (!m_input_files.empty()) {
        read_input_files();
    }

    bool todo = !m_relation_ids.empty();
    if (!m_relation_ids.empty()) {
        todo = find_relations_in_relations();
    }

    if (todo) {
        find_nodes_and_ways_in_relations();
    }

    if (!m_way_ids.empty()) {
        find_nodes_in_ways();
    }

    osmium::io::Reader reader(m_source_file, osmium::osm_entity_bits::object);
    osmium::io::Header header = reader.header();
    header.set("generator", m_generator);

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

    m_vout << "Copying from source to output file...\n";
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (auto it = buffer.begin<osmium::OSMObject>(); it != buffer.end<osmium::OSMObject>(); ++it) {
            switch (it->type()) {
                case osmium::item_type::node:
                    if (m_node_ids.count(it->id())) {
                        m_node_ids.erase(it->id());
                        writer(*it);
                    }
                    break;
                case osmium::item_type::way:
                    if (m_way_ids.count(it->id())) {
                        m_way_ids.erase(it->id());
                        writer(*it);
                    }
                    break;
                case osmium::item_type::relation:
                    if (m_relation_ids.count(it->id())) {
                        m_relation_ids.erase(it->id());
                        writer(*it);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    writer.close();
    reader.close();

    print_missing_ids("node", m_node_ids);
    print_missing_ids("way", m_way_ids);
    print_missing_ids("relation", m_relation_ids);

    show_memory_used();
    m_vout << "Done.\n";

    return m_node_ids.empty() && m_way_ids.empty() && m_relation_ids.empty();
}

namespace {

    const bool register_add_refs_command = CommandFactory::add("add-refs", "Add referenced objects to OSM data file", []() {
        return new CommandAddRefs();
    });

}

