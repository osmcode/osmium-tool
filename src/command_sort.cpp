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

#include <boost/function_output_iterator.hpp>
#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/object_comparisons.hpp>

#include "command_sort.hpp"

bool CommandSort::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ;

    add_common_options(cmdline);
    add_multiple_inputs_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "OSM input files")
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

    if (vm.count("input-filenames")) {
        m_filenames = vm["input-filenames"].as<std::vector<std::string>>();
    }

    return true;
}

void CommandSort::show_arguments() {
    m_vout << "Started osmium sort\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input file names: \n";
    for (const auto& fn : m_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  output filename: " << m_output_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  output format: " << m_output_format << "\n";
}

bool CommandSort::run() {
    std::vector<osmium::memory::Buffer> data;
    osmium::ObjectPointerCollection objects;

    osmium::Box bounding_box;

    m_vout << "Reading file contents...\n";
    for (const std::string& file_name : m_filenames) {
        osmium::io::Reader reader(file_name, osmium::osm_entity_bits::object);
        auto header = reader.header();
        bounding_box.extend(header.joined_boxes());
        while (osmium::memory::Buffer buffer = reader.read()) {
            osmium::apply(buffer, objects);
            data.push_back(std::move(buffer));
        }
        reader.close();
    }

    osmium::io::Header header;
    if (bounding_box) {
        header.add_box(bounding_box);
    }
    header.set("generator", m_generator);

    m_vout << "Sorting data...\n";
    objects.sort(osmium::object_order_type_id_version());

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);
    auto out = osmium::io::make_output_iterator(writer);

    m_vout << "Writing out sorted data...\n";
    std::copy(objects.begin(), objects.end(), out);

    writer.close();

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_sort_command = CommandFactory::add("sort", "Sort OSM data files", []() {
        return new CommandSort();
    });

}

