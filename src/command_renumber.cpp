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

#include <iostream>
#include <iterator>

#include <boost/program_options.hpp>

#include <osmium/index/index.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include "command_renumber.hpp"

bool CommandRenumber::setup(const std::vector<std::string>& arguments) {
    namespace po = boost::program_options;
    po::variables_map vm;

    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("verbose,v", "Set verbose mode")
    ("output,o", po::value<std::string>(), "Output file")
    ("output-format,f", po::value<std::string>(), "Format of output file")
    ("input-format,F", po::value<std::string>(), "Format of input files")
    ("generator", po::value<std::string>(), "Generator setting for file header")
    ("output-header", po::value<std::vector<std::string>>(), "Add output header")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ;

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("verbose")) {
        m_vout.verbose(true);
    }

    if (vm.count("generator")) {
        m_generator = vm["generator"].as<std::string>();
    }

    if (vm.count("output-header")) {
        m_output_headers = vm["output-header"].as<std::vector<std::string>>();
    }

    setup_input_file(vm);
    setup_output_file(vm);

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

    return true;
}

osmium::object_id_type CommandRenumber::lookup(int n, osmium::object_id_type id) {
    osmium::object_id_type result;

    try {
        result = m_id_index[n].get(id);
    } catch (osmium::not_found& e) {
        m_id_index[n].set(id, ++m_last_id[n]);
        result = m_last_id[n];
    }

    return result;
}


void CommandRenumber::renumber(osmium::memory::Buffer& buffer) {
    for (auto it = buffer.begin<osmium::OSMObject>(); it != buffer.end<osmium::OSMObject>(); ++it) {
        switch (it->type()) {
            case osmium::item_type::node:
                m_id_index[0].set(it->id(), ++m_last_id[0]);
                it->set_id(m_last_id[0]);
                break;
            case osmium::item_type::way:
                m_id_index[1].set(it->id(), ++m_last_id[1]);
                it->set_id(m_last_id[1]);
                for (auto& ref : static_cast<osmium::Way&>(*it).nodes()) {
                    ref.set_ref(lookup(0, ref.ref()));
                }
                break;
            case osmium::item_type::relation:
                it->set_id(m_id_index[2].get(it->id()));
                for (auto& member : static_cast<osmium::Relation&>(*it).members()) {
                    int n = uint16_t(member.type()) - 1;
                    assert(n >= 0 && n <= 2);
                    member.set_ref(lookup(n, member.ref()));
                }
                break;
            default:
                break;
        }
    }
}

bool CommandRenumber::run() {
    try {
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
            m_id_index[2].set(it->id(), ++m_last_id[2]);
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

