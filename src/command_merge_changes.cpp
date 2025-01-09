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

#include "command_merge_changes.hpp"

#include "util.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

bool CommandMergeChanges::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("simplify,s", "Simplify change")
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
    setup_input_files(vm);
    setup_output_file(vm);

    if (vm.count("simplify")) {
        m_simplify_change = true;
    }

    return true;
}

void CommandMergeChanges::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);
}

bool CommandMergeChanges::run() {
    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    setup_header(header);

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};
    auto out = osmium::io::make_output_iterator(writer);

    // this will contain all the buffers with the input data
    std::vector<osmium::memory::Buffer> changes;

    osmium::ObjectPointerCollection objects;

    // read all input files, keep the buffers around and add pointer
    // to each object to objects collection.
    m_vout << "Reading change file contents...\n";
    osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
    for (const osmium::io::File& change_file : m_input_files) {
        osmium::io::Reader reader{change_file, osmium::osm_entity_bits::object};
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            osmium::apply(buffer, objects);
            changes.push_back(std::move(buffer));
        }
        progress_bar.file_done(reader.file_size());
        reader.close();
    }
    progress_bar.done();

    // Now we sort all objects and write them in order into the
    // output_buffer, flushing the output_buffer whenever it is full.
    if (m_simplify_change) {
        // If the --simplify option was given we sort with the
        // largest version of each object first and then only
        // copy this last version of any object to the output_buffer.
        m_vout << "Sorting change data...\n";

        // This is needed for a special case: When change files have been
        // created from extracts it is possible that they contain objects
        // with the same type, id, version, and timestamp. In that case we
        // still want to get the last object available. So we have to make
        // sure it appears first in the objects vector before doing the
        // stable sort.
        std::reverse(objects.ptr_begin(), objects.ptr_end());
        objects.sort(osmium::object_order_type_id_reverse_version());

        m_vout << "Writing last version of each object to output...\n";
        std::unique_copy(objects.cbegin(), objects.cend(), out, osmium::object_equal_type_id());
    } else {
        // If the --simplify option was not given, this
        // is a straightforward sort and copy.
        m_vout << "Sorting change data...\n";
        objects.sort(osmium::object_order_type_id_version());
        m_vout << "Writing all objects to output...\n";
        std::copy(objects.cbegin(), objects.cend(), out);
    }

    m_vout << "Closing output file...\n";
    writer.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

