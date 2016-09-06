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
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/string.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/util/verbose_output.hpp>

#include "command_getid.hpp"
#include "exception.hpp"

std::set<osmium::object_id_type>& CommandGetId::ids(osmium::item_type type) noexcept {
    return m_ids[osmium::item_type_to_nwr_index(type)];
}

const std::set<osmium::object_id_type>& CommandGetId::ids(osmium::item_type type) const noexcept {
    return m_ids[osmium::item_type_to_nwr_index(type)];
}

void CommandGetId::parse_and_add_id(const std::string& s) {
    auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr, m_default_item_type);
    ids(p.first).insert(p.second);
}

void CommandGetId::read_id_file(std::istream& stream) {
    m_vout << "Reading ID file...\n";
    for (std::string line; std::getline(stream, line); ) {
        auto pos = line.find_first_of(" #");
        if (pos != std::string::npos) {
            line.erase(pos);
        }
        if (!line.empty()) {
            parse_and_add_id(line);
        }
    }
}

bool CommandGetId::no_ids() const {
    return ids(osmium::item_type::node).empty() &&
           ids(osmium::item_type::way).empty() &&
           ids(osmium::item_type::relation).empty();
}

size_t CommandGetId::count_ids() const {
    return ids(osmium::item_type::node).size() +
           ids(osmium::item_type::way).size() +
           ids(osmium::item_type::relation).size();
}

bool CommandGetId::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("default-type", po::value<std::string>()->default_value("node"), "Default item type")
    ("id-file,i", po::value<std::vector<std::string>>(), "Read OSM IDs from text file")
    ("id-osm-file,I", po::value<std::vector<std::string>>(), "Read OSM IDs from OSM file")
    ("history,H", "Make it work with history files")
    ("add-referenced,r", "Recursively add referenced objects")
    ("verbose-ids", "Print all requested and missing IDs")
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
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("add-referenced")) {
        if (m_input_filename == "-") {
            throw argument_error("Can not read OSM input from STDIN when --add-referenced/-r option is used.");
        }
        m_add_referenced_objects = true;
    }

    if (vm.count("history")) {
        m_work_with_history = true;
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
            throw argument_error(std::string("Unknown default type '") + t + "' (Allowed are 'node', 'way', and 'relation').");
        }
    }

    if (vm.count("verbose-ids")) {
        m_vout.verbose(true);
        m_verbose_ids = true;
    }

    if (vm.count("id-file")) {
        for (const std::string& filename : vm["id-file"].as<std::vector<std::string>>()) {
            if (filename == "-") {
                if (m_input_filename == "-") {
                    throw argument_error("Can not read OSM input and IDs both from STDIN.");
                }
                read_id_file(std::cin);
            } else {
                std::ifstream id_file{filename};
                if (!id_file.is_open()) {
                    throw argument_error("Could not open file '" + filename + "'");
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
        throw argument_error("Please specify IDs to look for on command line or with option --id-file/-i or --id-osm-file/-I.");
    }

    return true;
}

void CommandGetId::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    add referenced objects: " << yes_no(m_add_referenced_objects);
    m_vout << "    work with history files: " << yes_no(m_work_with_history);
    m_vout << "    default object type: " << osmium::item_type_to_name(m_default_item_type) << "\n";
    if (m_verbose_ids) {
        m_vout << "    looking for these ids:\n";
        m_vout << "      nodes:";
        for (osmium::object_id_type id : ids(osmium::item_type::node)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      ways:";
        for (osmium::object_id_type id : ids(osmium::item_type::way)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      relations:";
        for (osmium::object_id_type id : ids(osmium::item_type::relation)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
    } else {
        m_vout << "    looking for " << ids(osmium::item_type::node).size() << " node ID(s), "
                                     << ids(osmium::item_type::way).size() << " way ID(s), and "
                                     << ids(osmium::item_type::relation).size() << " relation ID(s)\n";
    }
}

osmium::osm_entity_bits::type CommandGetId::get_needed_types() const {
    osmium::osm_entity_bits::type types = osmium::osm_entity_bits::nothing;

    if (! ids(osmium::item_type::node).empty()) {
        types |= osmium::osm_entity_bits::node;
    }
    if (! ids(osmium::item_type::way).empty()) {
        types |= osmium::osm_entity_bits::way;
    }
    if (! ids(osmium::item_type::relation).empty()) {
        types |= osmium::osm_entity_bits::relation;
    }

    return types;
}

void CommandGetId::add_nodes(const osmium::Way& way) {
    for (const auto& nr : way.nodes()) {
        ids(osmium::item_type::node).insert(nr.ref());
    }
}

void CommandGetId::add_members(const osmium::Relation& relation) {
    for (const auto& member : relation.members()) {
        ids(member.type()).insert(member.ref());
    }
}

static void print_missing_ids(const char* type, std::set<osmium::object_id_type>& set) {
    if (set.empty()) {
        return;
    }
    std::cerr << "Missing " << type << " IDs:";
    for (const auto& id : set) {
        std::cerr << ' ' << id;
    }
    std::cerr << '\n';
}

void CommandGetId::read_id_osm_file(const std::string& file_name) {
    m_vout << "Reading OSM ID file...\n";
    osmium::io::Reader reader(file_name, osmium::osm_entity_bits::object);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            ids(object.type()).insert(object.id());
            if (object.type() == osmium::item_type::way) {
                add_nodes(static_cast<const osmium::Way&>(object));
            } else if (object.type() == osmium::item_type::relation) {
                add_members(static_cast<const osmium::Relation&>(object));
            }
        }
    }
    reader.close();
}

void CommandGetId::mark_rel_ids(const std::multimap<osmium::object_id_type, osmium::object_id_type>& rel_in_rel, osmium::object_id_type id) {
    auto range = rel_in_rel.equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
        if (ids(osmium::item_type::relation).count(it->second) == 0) {
            ids(osmium::item_type::relation).insert(it->second);
            mark_rel_ids(rel_in_rel, it->second);
        }
    }
}

bool CommandGetId::find_relations_in_relations() {
    m_vout << "  Reading input file to find relations in relations...\n";
    std::multimap<osmium::object_id_type, osmium::object_id_type> rel_in_rel;

    osmium::io::Reader reader(m_input_file, osmium::osm_entity_bits::relation);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& relation : buffer.select<osmium::Relation>()) {
            for (const auto& member : relation.members()) {
                if (member.type() == osmium::item_type::relation) {
                    rel_in_rel.emplace(relation.id(), member.ref());
                } else if (ids(osmium::item_type::relation).count(relation.id())) {
                    if (member.type() == osmium::item_type::node) {
                        ids(osmium::item_type::node).insert(member.ref());
                    } else if (member.type() == osmium::item_type::way) {
                        ids(osmium::item_type::way).insert(member.ref());
                    }
                }
            }
        }
    }
    reader.close();

    if (rel_in_rel.empty()) {
        return false;
    }

    for (const osmium::object_id_type id : ids(osmium::item_type::relation)) {
        mark_rel_ids(rel_in_rel, id);
    }

    return true;
}

void CommandGetId::find_nodes_and_ways_in_relations() {
    m_vout << "  Reading input file to find nodes/ways in relations...\n";

    osmium::io::Reader reader(m_input_file, osmium::osm_entity_bits::relation);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& relation : buffer.select<osmium::Relation>()) {
            if (ids(osmium::item_type::relation).count(relation.id())) {
                for (const auto& member : relation.members()) {
                    if (member.type() == osmium::item_type::node) {
                        ids(osmium::item_type::node).insert(member.ref());
                    } else if (member.type() == osmium::item_type::way) {
                        ids(osmium::item_type::way).insert(member.ref());
                    }
                }
            }
        }
    }
    reader.close();
}

void CommandGetId::find_nodes_in_ways() {
    m_vout << "  Reading input file to find nodes in ways...\n";

    osmium::io::Reader reader(m_input_file, osmium::osm_entity_bits::way);
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& way : buffer.select<osmium::Way>()) {
            if (ids(osmium::item_type::way).count(way.id())) {
                add_nodes(way);
            }
        }
    }
    reader.close();
}

void CommandGetId::find_referenced_objects() {
    m_vout << "Following references...\n";
    bool todo = !ids(osmium::item_type::relation).empty();
    if (todo) {
        todo = find_relations_in_relations();
    }

    if (todo) {
        find_nodes_and_ways_in_relations();
    }

    if (!ids(osmium::item_type::way).empty()) {
        find_nodes_in_ways();
    }
    m_vout << "Done following references.\n";
}

bool CommandGetId::run() {
    if (m_add_referenced_objects) {
        find_referenced_objects();
    }

    m_vout << "Opening input file...\n";
    osmium::io::Reader reader(m_input_file, get_needed_types());

    m_vout << "Opening output file...\n";
    osmium::io::Header header = reader.header();
    header.set("generator", m_generator);
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

    m_vout << "Copying from source to output file...\n";
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            auto& index = ids(object.type());
            if (index.count(object.id())) {
                if (!m_work_with_history) {
                    index.erase(object.id());
                }
                writer(object);
            }
        }
    }

    m_vout << "Closing output file...\n";
    writer.close();

    m_vout << "Closing input file...\n";
    reader.close();

    if (!m_work_with_history) {
        if (no_ids()) {
            m_vout << "Found all objects.\n";
        } else {
            m_vout << "Did not find " << count_ids() << " object(s).\n";
            if (m_verbose_ids) {
                print_missing_ids("node",     ids(osmium::item_type::node));
                print_missing_ids("way",      ids(osmium::item_type::way));
                print_missing_ids("relation", ids(osmium::item_type::relation));
            }
        }
    }

    show_memory_used();

    m_vout << "Done.\n";

    return m_work_with_history || no_ids();
}

namespace {

    const bool register_get_id_command = CommandFactory::add("getid", "Get objects with given ID from OSM file", []() {
        return new CommandGetId();
    });

}

