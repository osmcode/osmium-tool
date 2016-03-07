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
#include <numeric>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/string.hpp>

#include "command_getid.hpp"
#include "exception.hpp"

std::vector<osmium::object_id_type>& CommandGetId::ids(osmium::item_type type) noexcept {
    return m_ids[osmium::item_type_to_nwr_index(type)];
}

const std::vector<osmium::object_id_type>& CommandGetId::ids(osmium::item_type type) const noexcept {
    return m_ids[osmium::item_type_to_nwr_index(type)];
}

void CommandGetId::sort_unique(osmium::item_type type) {
    auto& index = ids(type);
    std::sort(index.begin(), index.end());
    auto last = std::unique(index.begin(), index.end());
    index.erase(last, index.end());
}

void CommandGetId::parse_id(const std::string& s) {
    auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr);
    if (p.first == osmium::item_type::undefined) {
        p.first = osmium::item_type::node;
    }
    ids(p.first).push_back(p.second);
}

bool CommandGetId::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("id-file,i", po::value<std::string>(), "Read OSM IDs from given file")
    ;

    add_common_options(cmdline);
    add_single_input_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("ids", po::value<std::vector<std::string>>(), "OSM IDs")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("ids", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm);
    setup_input_file(vm);
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
            parse_id(line);
        }
    }

    if (vm.count("ids")) {
        std::string sids;
        for (const auto& s : vm["ids"].as<std::vector<std::string>>()) {
            sids += s + " ";
        }
        for (const auto& s : osmium::split_string(sids, "\t ;,/|", true)) {
            parse_id(s);
        }
    }

    if (ids(osmium::item_type::node).empty() &&
        ids(osmium::item_type::way).empty() &&
        ids(osmium::item_type::relation).empty()) {
        throw argument_error("Need at least one id to look for...");
    }

    sort_unique(osmium::item_type::node);
    sort_unique(osmium::item_type::way);
    sort_unique(osmium::item_type::relation);

    return true;
}

void CommandGetId::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
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

bool CommandGetId::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader(m_input_file, get_needed_types());

    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    header.set("generator", m_generator);
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

    m_vout << "Reading input file...\n";
    auto input = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader);

    osmium::memory::Buffer output_buffer(10240);

    size_t num_ids = ids(osmium::item_type::node).size() +
                     ids(osmium::item_type::way).size() +
                     ids(osmium::item_type::relation).size();

    for (const osmium::OSMObject& object : input) {
        const auto& index = ids(object.type());
        if (std::binary_search(index.begin(), index.end(), object.id())) {
            output_buffer.add_item(object);
            output_buffer.commit();
            --num_ids;
        }
    }

    m_vout << "Writing out results...\n";
    writer(std::move(output_buffer));

    m_vout << "Closing output file...\n";
    writer.close();

    if (num_ids == 0) {
        m_vout << "Found all objects.\n";
    } else {
        m_vout << "Did not find " << num_ids << " objects.\n";
    }

    m_vout << "Closing input file...\n";
    reader.close();

    show_memory_used();
    m_vout << "Done.\n";

    return num_ids == 0;
}

namespace {

    const bool register_get_id_command = CommandFactory::add("getid", "Get objects with given ID from OSM file", []() {
        return new CommandGetId();
    });

}

