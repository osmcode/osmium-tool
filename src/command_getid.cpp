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

#include "command_getid.hpp"

#include "exception.hpp"
#include "id_file.hpp"
#include "util.hpp"

#include <osmium/index/relations_map.hpp>
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
#include <string>
#include <vector>

std::size_t CommandGetId::count_ids() const noexcept {
    return m_ids(osmium::item_type::node).size() +
           m_ids(osmium::item_type::way).size() +
           m_ids(osmium::item_type::relation).size();
}

bool CommandGetId::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("default-type", po::value<std::string>()->default_value("node"), "Default item type")
    ("id-file,i", po::value<std::vector<std::string>>(), "Read OSM IDs from text file")
    ("id-osm-file,I", po::value<std::vector<std::string>>(), "Read OSM IDs from OSM file")
    ("with-history,H", "Make it work with history files")
    ("add-referenced,r", "Recursively add referenced objects")
    ("remove-tags,t", "Remove tags from objects not explicitly requested")
    ("verbose-ids", "Print all requested and missing IDs")
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

    if (vm.count("add-referenced")) {
        if (m_input_filename.empty() || m_input_filename == "-") {
            throw argument_error{"Can not read OSM input from STDIN when --add-referenced/-r option is used."};
        }
        m_add_referenced_objects = true;
    }

    if (vm.count("with-history")) {
        m_work_with_history = true;
    }

    if (vm.count("default-type")) {
        m_default_item_type = parse_item_type(vm["default-type"].as<std::string>());
    }

    if (vm.count("remove-tags")) {
        m_remove_tags = true;
        if (!m_add_referenced_objects) {
            std::cerr << "Warning! Without -r/--add-referenced use of -t/--remove-tags isn't doing anything.\n";
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

    m_matching_ids = m_ids;

    return true;
}

void CommandGetId::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    add referenced objects: " << yes_no(m_add_referenced_objects);
    if (m_add_referenced_objects) {
        m_vout << "    remove tags on non-matching objects: " << yes_no(m_remove_tags);
    }
    m_vout << "    work with history files: " << yes_no(m_work_with_history);
    m_vout << "    default object type: " << osmium::item_type_to_name(m_default_item_type) << "\n";
    if (m_verbose_ids) {
        m_vout << "    looking for these ids:\n";
        m_vout << "      nodes:";
        for (const auto id : m_ids(osmium::item_type::node)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      ways:";
        for (const auto id : m_ids(osmium::item_type::way)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
        m_vout << "      relations:";
        for (const auto id : m_ids(osmium::item_type::relation)) {
            m_vout << " " << id;
        }
        m_vout << "\n";
    } else {
        m_vout << "    looking for " << m_ids(osmium::item_type::node).size() << " node ID(s), "
                                     << m_ids(osmium::item_type::way).size() << " way ID(s), and "
                                     << m_ids(osmium::item_type::relation).size() << " relation ID(s)\n";
    }
}

osmium::osm_entity_bits::type CommandGetId::get_needed_types() const {
    osmium::osm_entity_bits::type types = osmium::osm_entity_bits::nothing;

    if (!m_ids(osmium::item_type::node).empty()) {
        types |= osmium::osm_entity_bits::node;
    }
    if (!m_ids(osmium::item_type::way).empty()) {
        types |= osmium::osm_entity_bits::way;
    }
    if (!m_ids(osmium::item_type::relation).empty()) {
        types |= osmium::osm_entity_bits::relation;
    }

    return types;
}

namespace {

void print_missing_ids(const char* type, const osmium::index::IdSetDense<osmium::unsigned_object_id_type>& set) {
    if (set.empty()) {
        return;
    }
    std::cerr << "Missing " << type << " IDs:";
    for (const auto id : set) {
        std::cerr << ' ' << id;
    }
    std::cerr << '\n';
}

} // anonymous namespace

void CommandGetId::mark_rel_ids(const osmium::index::RelationsMapIndex& rel_in_rel, osmium::unsigned_object_id_type parent_id) {
    rel_in_rel.for_each(parent_id, [&](osmium::unsigned_object_id_type member_id) {
        if (m_ids(osmium::item_type::relation).check_and_set(member_id)) {
            mark_rel_ids(rel_in_rel, member_id);
        }
    });
}

bool CommandGetId::find_relations_in_relations() {
    m_vout << "  Reading input file to find relations in relations...\n";
    osmium::index::RelationsMapStash stash;

    osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::relation, osmium::io::read_meta::no};
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& relation : buffer.select<osmium::Relation>()) {
            for (const auto& member : relation.members()) {
                if (member.type() == osmium::item_type::relation) {
                    stash.add(member.ref(), relation.id());
                } else if (m_ids(osmium::item_type::relation).get(relation.positive_id())) {
                    if (member.type() == osmium::item_type::node) {
                        m_ids(osmium::item_type::node).set(member.positive_ref());
                    } else if (member.type() == osmium::item_type::way) {
                        m_ids(osmium::item_type::way).set(member.positive_ref());
                    }
                }
            }
        }
    }
    reader.close();

    if (stash.empty()) {
        return false;
    }

    const auto rel_in_rel = stash.build_parent_to_member_index();
    for (const auto id : m_ids(osmium::item_type::relation)) {
        mark_rel_ids(rel_in_rel, id);
    }

    return true;
}

void CommandGetId::find_nodes_and_ways_in_relations() {
    m_vout << "  Reading input file to find nodes/ways in relations...\n";

    osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::relation, osmium::io::read_meta::no};
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& relation : buffer.select<osmium::Relation>()) {
            if (m_ids(osmium::item_type::relation).get(relation.positive_id())) {
                for (const auto& member : relation.members()) {
                    if (member.type() == osmium::item_type::node) {
                        m_ids(osmium::item_type::node).set(member.positive_ref());
                    } else if (member.type() == osmium::item_type::way) {
                        m_ids(osmium::item_type::way).set(member.positive_ref());
                    }
                }
            }
        }
    }
    reader.close();
}

void CommandGetId::find_nodes_in_ways() {
    m_vout << "  Reading input file to find nodes in ways...\n";

    osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::way, osmium::io::read_meta::no};
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& way : buffer.select<osmium::Way>()) {
            if (m_ids(osmium::item_type::way).get(way.positive_id())) {
                add_nodes(way, m_ids);
            }
        }
    }
    reader.close();
}

void CommandGetId::find_referenced_objects() {
    m_vout << "Following references...\n";

    // If there are any relations we are looking for, we need to run
    // find_relations_in_relations() to get the member IDs of all types.
    bool todo = !m_ids(osmium::item_type::relation).empty();
    if (todo) {
        todo = find_relations_in_relations();
    }

    if (todo) {
        // If find_relations_in_relations() returned true, it means it found
        // relation members that were not in the original relations ID list.
        // This means we need to run find_nodes_and_ways_in_relations() to
        // make sure we have all node and way members of those relations, too.
        find_nodes_and_ways_in_relations();
    }

    if (!m_ids(osmium::item_type::way).empty()) {
        find_nodes_in_ways();
    }
    m_vout << "Done following references.\n";
}

bool CommandGetId::run() {
    m_vout << "Opening output file...\n";
    osmium::io::Writer writer{m_output_file, m_output_overwrite, m_fsync};

    if (m_add_referenced_objects) {
        find_referenced_objects();
    }

    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file, get_needed_types()};

    osmium::io::Header header{reader.header()};
    setup_header(header);
    writer.set_header(header);

    m_vout << "Copying matching objects to output file...\n";
    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        for (auto& object : buffer.select<osmium::OSMObject>()) {
            if (m_matching_ids(object.type()).get(object.positive_id())) {
                if (!m_work_with_history) {
                    m_ids(object.type()).unset(object.positive_id());
                }
                writer(object);
            } else if (m_ids(object.type()).get(object.positive_id())) {
                if (!m_work_with_history) {
                    m_ids(object.type()).unset(object.positive_id());
                }
                if (m_remove_tags) {
                    object.remove_tags();
                }
                writer(object);
            }
        }
    }
    progress_bar.done();

    m_vout << "Closing output file...\n";
    writer.close();

    m_vout << "Closing input file...\n";
    reader.close();

    if (!m_work_with_history) {
        if (no_ids(m_ids)) {
            m_vout << "Found all objects.\n";
        } else {
            m_vout << "Did not find " << count_ids() << " object(s).\n";
            if (m_verbose_ids) {
                print_missing_ids("node",     m_ids(osmium::item_type::node));
                print_missing_ids("way",      m_ids(osmium::item_type::way));
                print_missing_ids("relation", m_ids(osmium::item_type::relation));
            }
        }
    }

    show_memory_used();

    m_vout << "Done.\n";

    return m_work_with_history || no_ids(m_ids);
}

