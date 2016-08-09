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

#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <io.h>
#endif

#include <boost/program_options.hpp>

#include <osmium/index/index.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/memory_mapping.hpp>

#include "command_renumber.hpp"

osmium::object_id_type id_map::operator()(osmium::object_id_type id) {
    // Search for id in m_extra_ids and return if found.
    auto it = m_extra_ids.find(id);
    if (it != m_extra_ids.end()) {
        return it->second;
    }

    // New ID is larger than all existing IDs. Add it to end and return.
    if (m_ids.empty() || id > m_ids.back()) {
        m_ids.push_back(id);
        return m_ids.size();
    }

    const auto element = std::lower_bound(m_ids.cbegin(), m_ids.cend(), id);
    // Old ID not found in m_ids, add to m_extra_ids.
    if (element == m_ids.cend() || *element != id) {
        m_ids.push_back(m_ids.back());
        m_extra_ids[id] = m_ids.size();
        return m_ids.size();
    }

    // Old ID found in m_ids, return.
    return osmium::object_id_type(std::distance(m_ids.cbegin(), element) + 1);
}

void id_map::write(int fd) {
    for (const auto& m : m_extra_ids) {
        m_ids[m.second - 1] = m.first;
    }

    osmium::io::detail::reliable_write(
        fd,
        reinterpret_cast<const char*>(m_ids.data()),
        sizeof(osmium::object_id_type) * m_ids.size()
    );
}

void id_map::read(int fd, size_t file_size) {
    auto num_elements = file_size / sizeof(osmium::object_id_type);
    m_ids.reserve(num_elements);
    osmium::util::TypedMemoryMapping<osmium::object_id_type> mapping{num_elements, osmium::util::MemoryMapping::mapping_mode::readonly, fd};

    osmium::object_id_type last_id = 0;
    for (osmium::object_id_type id : mapping) {
        if (id > last_id) {
            m_ids.push_back(id);
            last_id = id;
        } else {
            m_ids.push_back(last_id);
            m_extra_ids[id] = m_ids.size();
        }
    }
}

bool CommandRenumber::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("index-directory,i", po::value<std::string>(), "Index directory")
    ("object-type,t", po::value<std::vector<std::string>>(), "Renumber only objects of given type (node, way, relation)")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};
    po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_object_type_nrw(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("index-directory")) {
        m_index_directory = vm["index-directory"].as<std::string>();
    }

    return true;
}

void CommandRenumber::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    index directory: " << m_index_directory << "\n";
    m_vout << "    object types that will be renumbered:";
    if (osm_entity_bits() & osmium::osm_entity_bits::node) {
        m_vout << " node";
    }
    if (osm_entity_bits() & osmium::osm_entity_bits::way) {
        m_vout << " way";
    }
    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << " relation";
    }
    m_vout << "\n";
}

void CommandRenumber::renumber(osmium::memory::Buffer& buffer) {
    for (auto& object : buffer.select<osmium::OSMObject>()) {
        switch (object.type()) {
            case osmium::item_type::node:
                if (osm_entity_bits() & osmium::osm_entity_bits::node) {
                    m_check_order.node(static_cast<const osmium::Node&>(object));
                    object.set_id(map(osmium::item_type::node)(object.id()));
                }
                break;
            case osmium::item_type::way:
                if (osm_entity_bits() & osmium::osm_entity_bits::way) {
                    m_check_order.way(static_cast<const osmium::Way&>(object));
                    object.set_id(map(osmium::item_type::way)(object.id()));
                }
                if (osm_entity_bits() & osmium::osm_entity_bits::node) {
                    for (auto& ref : static_cast<osmium::Way&>(object).nodes()) {
                        ref.set_ref(map(osmium::item_type::node)(ref.ref()));
                    }
                }
                break;
            case osmium::item_type::relation:
                if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
                    m_check_order.relation(static_cast<const osmium::Relation&>(object));
                    object.set_id(map(osmium::item_type::relation)(object.id()));
                }
                for (auto& member : static_cast<osmium::Relation&>(object).members()) {
                    if (osm_entity_bits() & osmium::osm_entity_bits::from_item_type(member.type())) {
                        member.set_ref(map(member.type())(member.ref()));
                    }
                }
                break;
            default:
                break;
        }
    }
}

std::string CommandRenumber::filename(const char* name) const {
    return m_index_directory + "/" + name + ".idx";
}

void CommandRenumber::read_index(osmium::item_type type) {
    std::string f { filename(osmium::item_type_to_name(type)) };
    int fd = ::open(f.c_str(), O_RDONLY);
    if (fd < 0) {
        // if the file is not there we don't have to read anything and can return
        if (errno == ENOENT) {
            return;
        }
        throw std::runtime_error(std::string("Can't open file '") + f + "': " + strerror(errno));
    }
#ifdef _WIN32
    _setmode(fd, _O_BINARY);
#endif

    size_t file_size = osmium::util::file_size(fd);

    if (file_size % sizeof(osmium::object_id_type) != 0) {
        throw std::runtime_error(std::string("index file '") + f + "' has wrong file size");
    }

    map(type).read(fd, file_size);

    close(fd);
}

void CommandRenumber::write_index(osmium::item_type type) {
    if (!(osm_entity_bits() & osmium::osm_entity_bits::from_item_type(type))) {
        return;
    }

    std::string f { filename(osmium::item_type_to_name(type)) };
    int fd = ::open(f.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error(std::string("Can't open file '") + f + "': " + strerror(errno));
    }
#ifdef _WIN32
    _setmode(fd, _O_BINARY);
#endif

    map(type).write(fd);

    close(fd);
}

void read_relations(const osmium::io::File& input_file, id_map& map) {
    osmium::io::Reader reader_pass1(input_file, osmium::osm_entity_bits::relation);

    auto input = osmium::io::make_input_iterator_range<osmium::Relation>(reader_pass1);
    for (const osmium::Relation& relation : input) {
        map(relation.id());
    }

    reader_pass1.close();
}

bool CommandRenumber::run() {
    if (!m_index_directory.empty()) {
        m_vout << "Reading index files...\n";
        read_index(osmium::item_type::node);
        read_index(osmium::item_type::way);
        read_index(osmium::item_type::relation);

        m_vout << "  Nodes     index contains " << map(osmium::item_type::node).size() << " items\n";
        m_vout << "  Ways      index contains " << map(osmium::item_type::way).size() << " items\n";
        m_vout << "  Relations index contains " << map(osmium::item_type::relation).size() << " items\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << "First pass through input file (reading relations)...\n";
        read_relations(m_input_file, map(osmium::item_type::relation));
    } else {
        m_vout << "No first pass through input file, because relation IDs are not mapped.\n";
    }

    m_vout << "Second pass through input file...\n";
    osmium::io::Reader reader_pass2(m_input_file);

    osmium::io::Header header = reader_pass2.header();
    header.set("generator", m_generator);
    header.set("xml_josm_upload", "false");
    for (const auto& h : m_output_headers) {
        header.set(h);
    }
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

    try {
        while (osmium::memory::Buffer buffer = reader_pass2.read()) {
            renumber(buffer);
            writer(std::move(buffer));
        }
    } catch (osmium::out_of_order_error& e) {
        std::cerr << e.what() << "\n";
        std::cerr << "This command expects the input file to be ordered: First nodes in order of ID,\n"
                  << "then ways in order of ID, then relations in order of ID.\n";
        exit(1);
    }
    reader_pass2.close();

    m_vout << "Closing output file...\n";
    writer.close();

    if (!m_index_directory.empty()) {
        m_vout << "Writing index files...\n";
        write_index(osmium::item_type::node);
        write_index(osmium::item_type::way);
        write_index(osmium::item_type::relation);
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::node) {
        m_vout << "Largest (referenced) node id: "     << map(osmium::item_type::node).size()     << "\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::way) {
        m_vout << "Largest (referenced) way id: "      << map(osmium::item_type::way).size()      << "\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << "Largest (referenced) relation id: " << map(osmium::item_type::relation).size() << "\n";
    }

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_renumber_command = CommandFactory::add("renumber", "Renumber IDs in OSM file", []() {
        return new CommandRenumber();
    });

}

