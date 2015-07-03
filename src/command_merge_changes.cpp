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

#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/object_comparisons.hpp>

#include "command_merge_changes.hpp"

bool CommandMergeChanges::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("simplify,s", "Simplify change")
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

    if (vm.count("simplify")) {
        m_simplify_change = true;
    }

    return true;
}

void CommandMergeChanges::show_arguments() {
    m_vout << "Started osmium merge-changes\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input change file names: \n";
    for (const auto& fn : m_input_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  output filename: " << m_output_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  output format: " << m_output_format << "\n";
}

bool CommandMergeChanges::run() {
    // this will contain all the buffers with the input data
    std::vector<osmium::memory::Buffer> changes;

    osmium::ObjectPointerCollection objects;

    // read all input files, keep the buffers around and add pointer
    // to each object to objects collection.
    m_vout << "Reading change file contents...\n";
    for (osmium::io::File& change_file : m_input_files) {
        osmium::io::Reader reader(change_file, osmium::osm_entity_bits::object);
        while (osmium::memory::Buffer buffer = reader.read()) {
            osmium::apply(buffer, objects);
            changes.push_back(std::move(buffer));
        }
    }

    osmium::io::Header header;
    header.set("generator", m_generator);

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite);
    osmium::io::OutputIterator<osmium::io::Writer> out(writer);

    // Now we sort all objects and write them in order into the
    // output_buffer, flushing the output_buffer whenever it is full.
    if (m_simplify_change) {
        // If the --simplify option was given we sort with the
        // largest version of each object first and then only
        // copy this last version of any object to the output_buffer.
        m_vout << "Sorting change data...\n";
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

    out.flush();
    writer.close();

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_merge_changes_command = CommandFactory::add("merge-changes", "Merge several OSM change files into one", []() {
        return new CommandMergeChanges();
    });

}

