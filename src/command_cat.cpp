/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2018  Jochen Topf <jochen@topf.org>

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

#include "command_cat.hpp"
#include "util.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <string>
#include <utility>
#include <vector>

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
    setup_progress(vm);
    setup_object_type_nwrc(vm);
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

bool CommandCat::run() {
    if (m_input_files.size() == 1) { // single input file
        m_vout << "Copying input file '" << m_input_files[0].filename() << "'\n";
        osmium::io::Reader reader{m_input_files[0], osm_entity_bits()};
        osmium::io::Header header{reader.header()};
        setup_header(header);
        osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

        osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            writer(std::move(buffer));
        }
        progress_bar.done();

        writer.close();
        reader.close();
    } else { // multiple input files
        osmium::io::Header header;
        setup_header(header);
        osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

        osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
        for (const auto& input_file : m_input_files) {
            progress_bar.remove();
            m_vout << "Copying input file '" << input_file.filename() << "'\n";
            osmium::io::Reader reader{input_file, osm_entity_bits()};
            while (osmium::memory::Buffer buffer = reader.read()) {
                progress_bar.update(reader.offset());
                writer(std::move(buffer));
            }
            progress_bar.file_done(reader.file_size());
            reader.close();
        }
        writer.close();
        progress_bar.done();
    }

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

