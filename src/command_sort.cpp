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

#include "command_sort.hpp"
#include "util.hpp"

#include <osmium/io/header.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/box.hpp>
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

bool CommandSort::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_multiple_inputs_options()};
    po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "OSM input files")
    ;

    po::options_description desc;
    desc.add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_input_files(vm);
    setup_output_file(vm);

    if (vm.count("input-filenames")) {
        m_filenames = vm["input-filenames"].as<std::vector<std::string>>();
    }

    return true;
}

void CommandSort::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);
}

bool CommandSort::run() {
    std::vector<osmium::memory::Buffer> data;
    osmium::ObjectPointerCollection objects;

    osmium::Box bounding_box;

    m_vout << "Reading contents of input files...\n";
    osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
    for (const std::string& file_name : m_filenames) {
        osmium::io::Reader reader{file_name, osmium::osm_entity_bits::object};
        osmium::io::Header header{reader.header()};
        bounding_box.extend(header.joined_boxes());
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            osmium::apply(buffer, objects);
            data.push_back(std::move(buffer));
        }
        progress_bar.file_done(reader.file_size());
        reader.close();
    }
    progress_bar.done();

    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    setup_header(header);
    if (bounding_box) {
        header.add_box(bounding_box);
    }

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

    m_vout << "Sorting data...\n";
    objects.sort(osmium::object_order_type_id_version());

    m_vout << "Writing out sorted data...\n";
    auto out = osmium::io::make_output_iterator(writer);
    std::copy(objects.begin(), objects.end(), out);

    m_vout << "Closing output file...\n";
    writer.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

