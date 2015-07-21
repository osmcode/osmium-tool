/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2015  Jochen Topf <jochen@topf.org>

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

#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

#include <boost/program_options.hpp>

#include <osmium/index/index.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/memory_mapping.hpp>

#include "command_renumber.hpp"

bool CommandRenumber::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("index-directory,i", po::value<std::string>(), "Index directory")
    ;

    add_common_options(cmdline);
    add_single_input_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("index-directory")) {
        m_index_directory = vm["index-directory"].as<std::string>();
    }

    return true;
}

void CommandRenumber::show_arguments() {
    m_vout << "Started osmium renumber\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input filename: " << m_input_filename << "\n";
    m_vout << "  output filename: " << m_output_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  output format: " << m_output_format << "\n";
    m_vout << "  output header: \n";
    for (const auto& h : m_output_headers) {
        m_vout << "    " << h << "\n";
    }
    m_vout << "  index directory: " << m_index_directory << "\n";
}

osmium::object_id_type CommandRenumber::lookup(osmium::item_type type, osmium::object_id_type id) {
    try {
        return index(type).at(id);
    } catch (std::out_of_range&) {
        index(type)[id] = ++last_id(type);
        return last_id(type);
    }
}

void CommandRenumber::renumber(osmium::memory::Buffer& buffer) {
    for (auto it = buffer.begin<osmium::OSMObject>(); it != buffer.end<osmium::OSMObject>(); ++it) {
        switch (it->type()) {
            case osmium::item_type::node:
                it->set_id(lookup(osmium::item_type::node, it->id()));
                break;
            case osmium::item_type::way:
                it->set_id(lookup(osmium::item_type::way, it->id()));
                for (auto& ref : static_cast<osmium::Way&>(*it).nodes()) {
                    ref.set_ref(lookup(osmium::item_type::node, ref.ref()));
                }
                break;
            case osmium::item_type::relation:
                it->set_id(lookup(osmium::item_type::relation, it->id()));
                for (auto& member : static_cast<osmium::Relation&>(*it).members()) {
                    member.set_ref(lookup(member.type(), member.ref()));
                }
                break;
            default:
                break;
        }
    }
}

std::string CommandRenumber::filename(const std::string& name) {
    return m_index_directory + "/" + name + ".idx";
}

remap_index_type& CommandRenumber::index(osmium::item_type type) {
    return m_id_index[osmium::item_type_to_nwr_index(type)];
}

osmium::object_id_type& CommandRenumber::last_id(osmium::item_type type) {
    return m_last_id[osmium::item_type_to_nwr_index(type)];
}

void CommandRenumber::read_index(osmium::item_type type, const std::string& name) {
    std::string f { filename(name) };
    int fd = ::open(f.c_str(), O_RDONLY);
    if (fd < 0) {
        // if the file is not there we don't have to read anything and can return
        if (errno == ENOENT) {
            return;
        }
        throw std::runtime_error(std::string("Can't open file '") + f + "': " + strerror(errno));
    }

    size_t file_size = osmium::util::file_size(fd);
    if (file_size % sizeof(remap_index_type::value_type) != 0) {
        throw std::runtime_error(std::string("index file '") + f + "' has wrong file size");
    }

    {
        osmium::util::TypedMemoryMapping<remap_index_type::value_type> mapping(file_size / sizeof(remap_index_type::value_type), false, fd);
        std::copy(mapping.begin(), mapping.end(), std::inserter(index(type), index(type).begin()));

        last_id(type) = std::max_element(mapping.begin(),
                                         mapping.end(),
                                         [](const remap_index_type::value_type& a,
                                            const remap_index_type::value_type& b) {
                                             return a.second < b.second;
                                         }
                        )->second;
    }

    close(fd);
}

void CommandRenumber::write_index(osmium::item_type type, const std::string& name) {
    std::string f { filename(name) };
    int fd = ::open(f.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error(std::string("Can't open file '") + f + "': " + strerror(errno));
    }

    std::vector<remap_index_type::value_type> data;
    std::copy(index(type).begin(), index(type).end(), std::back_inserter(data));
    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(data.data()), sizeof(remap_index_type::value_type) * data.size());

    close(fd);
}

bool CommandRenumber::run() {
    try {
        if (!m_index_directory.empty()) {
            m_vout << "Reading index files...\n";
            read_index(osmium::item_type::node, "nodes");
            read_index(osmium::item_type::way, "ways");
            read_index(osmium::item_type::relation, "relations");

            m_vout << "  Nodes     index contains " << index(osmium::item_type::node).size()     << " items\n";
            m_vout << "  Ways      index contains " << index(osmium::item_type::way).size()      << " items\n";
            m_vout << "  Relations index contains " << index(osmium::item_type::relation).size() << " items\n";
        }

        m_vout << "First pass through input file (reading relations)...\n";
        osmium::io::Reader reader_pass1(m_input_file, osmium::osm_entity_bits::relation);

        osmium::io::Header header = reader_pass1.header();
        header.set("generator", m_generator);
        header.set("xml_josm_upload", "false");
        for (const auto& h : m_output_headers) {
            header.set(h);
        }
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite);

        osmium::io::InputIterator<osmium::io::Reader, osmium::Relation> it { reader_pass1 };
        osmium::io::InputIterator<osmium::io::Reader, osmium::Relation> end {};

        for (; it != end; ++it) {
            lookup(osmium::item_type::relation, it->id());
        }

        reader_pass1.close();

        m_vout << "Second pass through input file...\n";
        osmium::io::Reader reader_pass2(m_input_file);
        while (osmium::memory::Buffer buffer = reader_pass2.read()) {
            renumber(buffer);
            writer(std::move(buffer));
        }
        reader_pass2.close();

        writer.close();

        if (!m_index_directory.empty()) {
            m_vout << "Writing index files...\n";
            write_index(osmium::item_type::node, "nodes");
            write_index(osmium::item_type::way, "ways");
            write_index(osmium::item_type::relation, "relations");
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return false;
    }

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_renumber_command = CommandFactory::add("renumber", "Renumber IDs in OSM file", []() {
        return new CommandRenumber();
    });

}

