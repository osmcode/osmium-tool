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

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include "command_cat.hpp"

bool CommandCat::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation, changeset)")
    ;

    add_common_options(cmdline);
    add_multiple_inputs_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "Input files")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    if (vm.count("object-type")) {
        m_osm_entity_bits = osmium::osm_entity_bits::nothing;
        for (const auto& t : vm["object-type"].as<std::vector<std::string>>()) {
            if (t == "node") {
                m_osm_entity_bits |= osmium::osm_entity_bits::node;
            } else if (t == "way") {
                m_osm_entity_bits |= osmium::osm_entity_bits::way;
            } else if (t == "relation") {
                m_osm_entity_bits |= osmium::osm_entity_bits::relation;
            } else if (t == "changeset") {
                m_osm_entity_bits |= osmium::osm_entity_bits::changeset;
            } else {
                throw argument_error(std::string("Unknown object type '") + t + "' (Allowed are 'node', 'way', 'relation', and 'changeset').");
            }
        }
    }

    return true;
}

void CommandCat::show_arguments() {
    m_vout << "Started osmium cat\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input filenames: \n";
    for (const auto& fn : m_input_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  output filename: " << m_output_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  output format: " << m_output_format << "\n";
    m_vout << "  output header: \n";
    for (const auto& h : m_output_headers) {
        m_vout << "    " << h << "\n";
    }
    m_vout << "  object types:";
    if (m_osm_entity_bits & osmium::osm_entity_bits::node) {
        m_vout << " node";
    }
    if (m_osm_entity_bits & osmium::osm_entity_bits::way) {
        m_vout << " way";
    }
    if (m_osm_entity_bits & osmium::osm_entity_bits::relation) {
        m_vout << " relation";
    }
    if (m_osm_entity_bits & osmium::osm_entity_bits::changeset) {
        m_vout << " changeset";
    }
    m_vout << "\n";
}

void CommandCat::setup_header(osmium::io::Header& header) const {
    header.set("generator", m_generator);
    for (const auto& h : m_output_headers) {
        header.set(h);
    }
}

bool CommandCat::run() {
    if (m_input_files.size() == 1) { // single input file
        m_vout << "Copying input file '" << m_input_files[0].filename() << "'\n";
        osmium::io::Reader reader(m_input_files[0], m_osm_entity_bits);
        osmium::io::Header header = reader.header();
        setup_header(header);
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite);

        while (osmium::memory::Buffer buffer = reader.read()) {
            writer(std::move(buffer));
        }
        writer.close();
    } else { // multiple input files
        osmium::io::Header header;
        setup_header(header);
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite);

        for (const auto& input_file : m_input_files) {
            m_vout << "Copying input file '" << input_file.filename() << "'\n";
            osmium::io::Reader reader(input_file, m_osm_entity_bits);
            while (osmium::memory::Buffer buffer = reader.read()) {
                writer(std::move(buffer));
            }
        }
        writer.close();
    }

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_cat_command = CommandFactory::add("cat", "Concatenate OSM files and convert to different formats", []() {
        return new CommandCat();
    });

}

