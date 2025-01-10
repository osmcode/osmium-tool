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

#include "command_renumber.hpp"

#include "exception.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/memory_mapping.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/string.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
# include <io.h>
#endif

osmium::object_id_type id_map::add_offset_to_id(osmium::object_id_type id) const noexcept {
    if (m_start_id < 0) {
        return -id + m_start_id + 1;
    }
    return id + m_start_id - 1;
}

osmium::object_id_type id_map::operator()(osmium::object_id_type id) {
    // Search for id in m_extra_ids and return if found.
    const auto it = m_extra_ids.find(id);
    if (it != m_extra_ids.end()) {
        return add_offset_to_id(it->second);
    }

    // New ID is larger than all existing IDs. Add it to end and return.
    if (m_ids.empty() || osmium::id_order{}(m_ids.back(), id)) {
        m_ids.push_back(id);
        return add_offset_to_id(static_cast<osmium::object_id_type>(m_ids.size()));
    }

    const auto element = std::lower_bound(m_ids.cbegin(), m_ids.cend(), id, osmium::id_order{});
    // Old ID not found in m_ids, add to m_extra_ids.
    if (element == m_ids.cend() || *element != id) {
        m_ids.push_back(m_ids.back());
        m_extra_ids[id] = static_cast<osmium::object_id_type>(m_ids.size());
        return add_offset_to_id(static_cast<osmium::object_id_type>(m_ids.size()));
    }

    // Old ID found in m_ids, return.
    return add_offset_to_id(static_cast<osmium::object_id_type>(std::distance(m_ids.cbegin(), element) + 1));
}

void id_map::write(int fd) {
    for (const auto& m : m_extra_ids) {
        m_ids[m.second - 1] = m.first;
    }

    osmium::io::detail::reliable_write(
        fd,
        reinterpret_cast<const char*>(m_ids.data()), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        sizeof(osmium::object_id_type) * m_ids.size()
    );
}

void id_map::print(osmium::object_id_type new_id) {
    for (const auto& m : m_extra_ids) {
        m_ids[m.second - 1] = m.first;
    }

    for (const auto& id : m_ids) {
        std::cout << id << ' ' << new_id << '\n';
        if (new_id > 0) {
            ++new_id;
        } else {
            --new_id;
        }
    }
}

void id_map::read(int fd, std::size_t file_size) {
    const auto num_elements = file_size / sizeof(osmium::object_id_type);
    m_ids.reserve(num_elements);
    const osmium::TypedMemoryMapping<osmium::object_id_type> mapping{num_elements, osmium::MemoryMapping::mapping_mode::readonly, fd};

    osmium::object_id_type last_id = 0;
    for (const osmium::object_id_type id : mapping) {
        if (osmium::id_order{}(last_id, id)) {
            m_ids.push_back(id);
            last_id = id;
        } else {
            m_ids.push_back(last_id);
            m_extra_ids[id] = static_cast<osmium::object_id_type>(m_ids.size());
        }
    }
}

namespace {

osmium::object_id_type get_start_id(const std::string& s) {
    const auto id = osmium::string_to_object_id(s.c_str());
    if (id == 0) {
        return 1;
    }
    return id;
}

} // anonymous namespace

void CommandRenumber::set_start_ids(const std::string& str) {
    const auto start_ids = osmium::split_string(str, ',');
    if (start_ids.size() == 1) {
        const auto id = get_start_id(start_ids[0]);
        m_id_map(osmium::item_type::node).set_start_id(id);
        m_id_map(osmium::item_type::way).set_start_id(id);
        m_id_map(osmium::item_type::relation).set_start_id(id);
    } else if (start_ids.size() == 3) {
        m_id_map(osmium::item_type::node).set_start_id(get_start_id(start_ids[0]));
        m_id_map(osmium::item_type::way).set_start_id(get_start_id(start_ids[1]));
        m_id_map(osmium::item_type::relation).set_start_id(get_start_id(start_ids[2]));
    } else {
        throw argument_error{"The --start-id/s option must be followed by exactly 1 ID or 3 IDs separated by commas"};
    }
}

void CommandRenumber::show_index(const std::string& type) {
    auto t = osmium::item_type::undefined;

    if (type == "n" || type == "node") {
        t = osmium::item_type::node;
    } else if (type == "w" || type == "way") {
        t = osmium::item_type::way;
    } else if (type == "r" || type == "relation") {
        t = osmium::item_type::relation;
    } else {
        throw argument_error{"Invalid value for --show-index option. Allowed are 'node', 'way', or 'relation'"};
    }

    read_start_ids_file();
    read_index(t);
    m_id_map(t).print(m_id_map(t).start_id());
}

bool CommandRenumber::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("index-directory,i", po::value<std::string>(), "Index directory")
    ("object-type,t", po::value<std::vector<std::string>>(), "Renumber only objects of given type (node, way, relation)")
    ("show-index", po::value<std::string>(), "Show contents of index file")
    ("start-id,s", po::value<std::string>(), "Comma separated list of first node, way, and relation id to use (default: 1,1,1)")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

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

    if (vm.count("index-directory")) {
        m_index_directory = vm["index-directory"].as<std::string>();
    }

    if (vm.count("show-index")) {
        show_index(vm["show-index"].as<std::string>());
        return false;
    }

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_object_type_nwr(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("start-id")) {
        set_start_ids(vm["start-id"].as<std::string>());
    }

    return true;
}

void CommandRenumber::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    index directory: " << m_index_directory << "\n";
    m_vout << "    object types that will be renumbered and their start IDs:";
    if (osm_entity_bits() & osmium::osm_entity_bits::node) {
        m_vout << " node (" << m_id_map(osmium::item_type::node).start_id() << ')';
    }
    if (osm_entity_bits() & osmium::osm_entity_bits::way) {
        m_vout << " way (" << m_id_map(osmium::item_type::way).start_id() << ')';
    }
    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << " relation (" << m_id_map(osmium::item_type::relation).start_id() << ')';
    }
    m_vout << "\n";
}

void CommandRenumber::renumber(osmium::memory::Buffer& buffer) {
    for (auto& object : buffer.select<osmium::OSMObject>()) {
        switch (object.type()) {
            case osmium::item_type::node:
                if (osm_entity_bits() & osmium::osm_entity_bits::node) {
                    m_check_order.node(static_cast<const osmium::Node&>(object));
                    object.set_id(m_id_map(osmium::item_type::node)(object.id()));
                }
                break;
            case osmium::item_type::way:
                if (osm_entity_bits() & osmium::osm_entity_bits::way) {
                    m_check_order.way(static_cast<const osmium::Way&>(object));
                    object.set_id(m_id_map(osmium::item_type::way)(object.id()));
                }
                if (osm_entity_bits() & osmium::osm_entity_bits::node) {
                    for (auto& ref : static_cast<osmium::Way&>(object).nodes()) {
                        ref.set_ref(m_id_map(osmium::item_type::node)(ref.ref()));
                    }
                }
                break;
            case osmium::item_type::relation:
                if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
                    m_check_order.relation(static_cast<const osmium::Relation&>(object));
                    object.set_id(m_id_map(osmium::item_type::relation)(object.id()));
                }
                for (auto& member : static_cast<osmium::Relation&>(object).members()) {
                    if (osm_entity_bits() & osmium::osm_entity_bits::from_item_type(member.type())) {
                        member.set_ref(m_id_map(member.type())(member.ref()));
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
    const std::string f{filename(osmium::item_type_to_name(type))};
    const int fd = ::open(f.c_str(), O_RDWR);
    if (fd < 0) {
        // if the file is not there we don't have to read anything and can return
        if (errno == ENOENT) {
            return;
        }
        throw std::system_error{errno, std::system_category(), "Could not open file '" + f + "'"};
    }
#ifdef _WIN32
    _setmode(fd, _O_BINARY);
#endif

    const std::size_t file_size = osmium::file_size(fd);

    if (file_size % sizeof(osmium::object_id_type) != 0) {
        throw std::runtime_error{std::string{"Index file '"} + f + "' has wrong file size"};
    }

    m_id_map(type).read(fd, file_size);

    close(fd);
}

void CommandRenumber::write_index(osmium::item_type type) {
    if (!(osm_entity_bits() & osmium::osm_entity_bits::from_item_type(type))) {
        return;
    }

    const std::string f{filename(osmium::item_type_to_name(type))};
    const int fd = ::open(f.c_str(), O_WRONLY | O_CREAT, 0666); // NOLINT(hicpp-signed-bitwise)
    if (fd < 0) {
        throw std::system_error{errno, std::system_category(), "Could not open file '" + f + "'"};
    }
#ifdef _WIN32
    _setmode(fd, _O_BINARY);
#endif

    m_id_map(type).write(fd);

    close(fd);
}

namespace {

void read_relations(const osmium::io::File& input_file, id_map* map) {
    osmium::io::Reader reader{input_file, osmium::osm_entity_bits::relation};

    const auto input = osmium::io::make_input_iterator_range<osmium::Relation>(reader);
    for (const osmium::Relation& relation : input) {
        (*map)(relation.id());
    }

    reader.close();
}

} // anonymous namespace

void CommandRenumber::read_start_ids_file() {
    std::ifstream start_id_file{m_index_directory + "/start_ids"};
    if (start_id_file.is_open()) {
        std::string line;
        start_id_file >> line;
        start_id_file.close();
        set_start_ids(line);
    }
}

bool CommandRenumber::run() {
    if (!m_index_directory.empty()) {
        m_vout << "Reading index files...\n";
        read_start_ids_file();
        read_index(osmium::item_type::node);
        read_index(osmium::item_type::way);
        read_index(osmium::item_type::relation);

        m_vout << "  Nodes     index contains " << m_id_map(osmium::item_type::node).size() << " items\n";
        m_vout << "  Ways      index contains " << m_id_map(osmium::item_type::way).size() << " items\n";
        m_vout << "  Relations index contains " << m_id_map(osmium::item_type::relation).size() << " items\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << "First pass (of two) through input file (reading relations)...\n";
        read_relations(m_input_file, &m_id_map(osmium::item_type::relation));
        m_vout << "First pass done.\n";
        m_vout << "Second pass (of two) through input file...\n";
    } else {
        m_vout << "Single pass through input file (because relation IDs are not mapped)...\n";
    }

    osmium::io::Reader reader{m_input_file};

    osmium::io::Header header = reader.header();
    setup_header(header);
    header.set("xml_josm_upload", "false");
    header.set("sorting", "Type_then_ID");

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        renumber(buffer);
        writer(std::move(buffer));
    }
    progress_bar.done();
    reader.close();

    m_vout << "Pass done.\n";
    m_vout << "Closing output file...\n";
    writer.close();

    if (!m_index_directory.empty()) {
        m_vout << "Writing index files...\n";

        std::ofstream start_id_file{m_index_directory + "/start_ids"};
        start_id_file << m_id_map(osmium::item_type::node).start_id() << ','
                      << m_id_map(osmium::item_type::way).start_id() << ','
                      << m_id_map(osmium::item_type::relation).start_id() << '\n';
        start_id_file.close();

        write_index(osmium::item_type::node);
        write_index(osmium::item_type::way);
        write_index(osmium::item_type::relation);
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::node) {
        m_vout << "Largest (referenced) node id: "     << m_id_map(osmium::item_type::node).size()     << "\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::way) {
        m_vout << "Largest (referenced) way id: "      << m_id_map(osmium::item_type::way).size()      << "\n";
    }

    if (osm_entity_bits() & osmium::osm_entity_bits::relation) {
        m_vout << "Largest (referenced) relation id: " << m_id_map(osmium::item_type::relation).size() << "\n";
    }

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

