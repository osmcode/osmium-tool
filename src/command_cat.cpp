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

#include "command_cat.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <cassert>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

bool CommandCat::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation, changeset)")
    ("clean,c", po::value<std::vector<std::string>>(), "Clean attribute (version, changeset, timestamp, uid, user)")
    ("buffer-data", "Buffer all data in memory before writing it out")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_multiple_inputs_options()};
    const po::options_description opts_output{add_output_options()};

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

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_object_type_nwrc(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    m_clean.setup(vm);

    if (vm.count("buffer-data")) {
        m_buffer_data = true;
    }

    return true;
}

void CommandCat::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    show_object_types(m_vout);
    m_vout << "    attributes to clean: " << m_clean.to_string() << '\n';
}

void CommandCat::copy(osmium::ProgressBar& progress_bar, osmium::io::Reader& reader, osmium::io::Writer& writer) const {
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());

        m_clean.apply_to(buffer);

        writer(std::move(buffer));
    }
}

std::size_t CommandCat::read_buffers(osmium::ProgressBar& progress_bar, osmium::io::Reader& reader, std::vector<osmium::memory::Buffer>& buffers) {
    std::size_t size = 0;

    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());

        m_clean.apply_to(buffer);

        size += buffer.committed();

        buffers.emplace_back(std::move(buffer));
    }

    return size;
}

void CommandCat::write_buffers(osmium::ProgressBar& progress_bar, std::vector<osmium::memory::Buffer>& buffers, osmium::io::Writer& writer) {
    std::size_t size = 0;

    for (auto&& buffer : buffers) {
        size += buffer.committed();
        writer(std::move(buffer));
        progress_bar.update(size);
    }
}

namespace {

void report_filename(osmium::VerboseOutput* vout, const osmium::io::File& file, const osmium::io::Reader& reader) {
    assert(vout);

    const auto size = reader.file_size();
    const auto& name = file.filename();

    if (size == 0) {
        if (name.empty()) {
            *vout << "Reading from stdin...\n";
        } else {
            *vout << "Reading input file '" << name << "'...\n";
        }
    } else {
        *vout << "Reading input file '" << name << "' (" << size << " bytes)...\n";
    }
}

} // anonymous namespace

bool CommandCat::run() {
    std::size_t bytes_written = 0;

    if (m_input_files.size() == 1) { // single input file
        osmium::io::Reader reader{m_input_files[0], osm_entity_bits()};
        osmium::io::Header header{reader.header()};

        report_filename(&m_vout, m_input_files[0], reader);

        setup_header(header);
        osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

        if (m_buffer_data) {
            std::vector<osmium::memory::Buffer> buffers;
            osmium::ProgressBar progress_bar_reader{reader.file_size(), display_progress()};
            const std::size_t size = read_buffers(progress_bar_reader, reader, buffers);
            progress_bar_reader.done();
            m_vout << "All data read.\n";
            show_memory_used();
            m_vout << "Writing data...\n";
            osmium::ProgressBar progress_bar_writer{size, display_progress()};
            write_buffers(progress_bar_writer, buffers, writer);
            progress_bar_writer.done();
        } else {
            osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
            copy(progress_bar, reader, writer);
            progress_bar.done();
        }
        bytes_written = writer.close();
        reader.close();
    } else { // multiple input files
        osmium::io::Header header;
        setup_header(header);
        osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

        if (m_buffer_data) {
            std::vector<osmium::memory::Buffer> buffers;
            std::size_t size = 0;
            osmium::ProgressBar progress_bar_reader{file_size_sum(m_input_files), display_progress()};
            for (const auto& input_file : m_input_files) {
                progress_bar_reader.remove();
                osmium::io::Reader reader{input_file, osm_entity_bits()};
                report_filename(&m_vout, input_file, reader);
                size += read_buffers(progress_bar_reader, reader, buffers);
                progress_bar_reader.file_done(reader.file_size());
                reader.close();
            }
            progress_bar_reader.done();
            m_vout << "All data read.\n";
            show_memory_used();
            m_vout << "Writing data...\n";
            osmium::ProgressBar progress_bar_writer{size, display_progress()};
            write_buffers(progress_bar_writer, buffers, writer);
            bytes_written = writer.close();
            progress_bar_writer.done();
        } else {
            osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
            for (const auto& input_file : m_input_files) {
                progress_bar.remove();
                osmium::io::Reader reader{input_file, osm_entity_bits()};
                report_filename(&m_vout, input_file, reader);
                copy(progress_bar, reader, writer);
                progress_bar.file_done(reader.file_size());
                reader.close();
            }
            bytes_written = writer.close();
            progress_bar.done();
        }
    }

    if (bytes_written > 0) {
        m_vout << "Wrote " << bytes_written << " bytes.\n";
    }

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

