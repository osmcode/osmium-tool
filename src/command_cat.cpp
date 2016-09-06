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

#include <boost/program_options.hpp>

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/util/verbose_output.hpp>

#include "command_cat.hpp"

bool CommandCat::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation, changeset)")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_multiple_inputs_options()};
    po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "Input files")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_object_type_nrwc(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    return true;
}

void CommandCat::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    show_object_types(m_vout);
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
        osmium::io::Reader reader(m_input_files[0], osm_entity_bits());
        osmium::io::Header header = reader.header();
        setup_header(header);
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

        while (osmium::memory::Buffer buffer = reader.read()) {
            writer(std::move(buffer));
        }
        writer.close();
        reader.close();
    } else { // multiple input files
        osmium::io::Header header;
        setup_header(header);
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

        for (const auto& input_file : m_input_files) {
            m_vout << "Copying input file '" << input_file.filename() << "'\n";
            osmium::io::Reader reader(input_file, osm_entity_bits());
            while (osmium::memory::Buffer buffer = reader.read()) {
                writer(std::move(buffer));
            }
            reader.close();
        }
        writer.close();
    }

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_cat_command = CommandFactory::add("cat", "Concatenate OSM files and convert to different formats", []() {
        return new CommandCat();
    });

}

