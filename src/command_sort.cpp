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

#include "command_sort.hpp"

#include "exception.hpp"
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
#include <cmath>
#include <string>
#include <utility>
#include <vector>

bool CommandSort::setup(const std::vector<std::string>& arguments) {
    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_multiple_inputs_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "OSM input files")
    ("strategy,s", po::value<std::string>(), "Strategy (default: simple)")
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

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    if (vm.count("strategy")) {
        m_strategy = vm["strategy"].as<std::string>();
        if (m_strategy != "simple" && m_strategy != "multipass") {
            throw argument_error{"Unknown strategy: " + m_strategy};
        }
    }

    if (m_strategy == "multipass") {
        if (std::any_of(m_input_filenames.cbegin(), m_input_filenames.cend(), [&](const std::string& name) {
            return name == "-";
        })) {
            throw argument_error{"Can not read from STDIN when using multipass strategy"};
        }
    }

    return true;
}

void CommandSort::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);

    m_vout << "  other options:\n";
    m_vout << "    strategy: " << m_strategy << "\n";
}

bool CommandSort::run_single_pass() {
    osmium::io::Writer writer{m_output_file, m_output_overwrite, m_fsync};

    std::vector<osmium::memory::Buffer> data;
    osmium::ObjectPointerCollection objects;

    osmium::Box bounding_box;

    uint64_t buffers_count = 0;
    uint64_t buffers_size = 0;
    uint64_t buffers_capacity = 0;

    m_vout << "Reading contents of input files...\n";
    osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
    for (const auto& file : m_input_files) {
        osmium::io::Reader reader{file, osmium::osm_entity_bits::object};
        const osmium::io::Header header{reader.header()};
        bounding_box.extend(header.joined_boxes());
        while (osmium::memory::Buffer buffer = reader.read()) {
            ++buffers_count;
            buffers_size += buffer.committed();
            buffers_capacity += buffer.capacity();
            progress_bar.update(reader.offset());
            osmium::apply(buffer, objects);
            data.push_back(std::move(buffer));
        }
        progress_bar.file_done(reader.file_size());
        reader.close();
    }
    progress_bar.done();

    m_vout << "Number of buffers: " << buffers_count << "\n";
    m_vout << "Sum of buffer sizes: " << buffers_size << " (" << show_gbytes(buffers_size) << " GB)\n";

    if (buffers_capacity != 0) {
        const auto fill_factor = std::round(100 * static_cast<double>(buffers_size) / static_cast<double>(buffers_capacity));
        m_vout << "Sum of buffer capacities: " << buffers_capacity << " (" << show_gbytes(buffers_capacity) << " GB, " << fill_factor << "% full)\n";
    } else {
        m_vout << "Sum of buffer capacities: 0 (0 GB)\n";
    }

    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    setup_header(header);
    header.set("sorting", "Type_then_ID");
    if (bounding_box) {
        header.add_box(bounding_box);
    }
    writer.set_header(header);

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

bool CommandSort::run_multi_pass() {
    osmium::io::Writer writer{m_output_file, m_output_overwrite, m_fsync};

    osmium::Box bounding_box;

    m_vout << "Reading input file headers...\n";
    for (const auto& file : m_input_files) {
        osmium::io::Reader reader{file, osmium::osm_entity_bits::nothing};
        const osmium::io::Header header{reader.header()};
        bounding_box.extend(header.joined_boxes());
        reader.close();
    }

    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    setup_header(header);
    if (bounding_box) {
        header.add_box(bounding_box);
    }
    writer.set_header(header);

    osmium::ProgressBar progress_bar{file_size_sum(m_input_files) * 3, display_progress()};

    int pass = 1;
    for (const auto entity : {osmium::osm_entity_bits::node, osmium::osm_entity_bits::way, osmium::osm_entity_bits::relation}) {
        std::vector<osmium::memory::Buffer> data;
        osmium::ObjectPointerCollection objects;

        uint64_t buffers_count = 0;
        uint64_t buffers_size = 0;
        uint64_t buffers_capacity = 0;

        m_vout << "Pass " << pass++ << "...\n";
        m_vout << "Reading contents of input files...\n";
        for (const auto& file : m_input_files) {
            osmium::io::Reader reader{file, entity};
            const osmium::io::Header read_header{reader.header()};
            bounding_box.extend(read_header.joined_boxes());
            while (osmium::memory::Buffer buffer = reader.read()) {
                ++buffers_count;
                buffers_size += buffer.committed();
                buffers_capacity += buffer.capacity();
                progress_bar.update(reader.offset());
                osmium::apply(buffer, objects);
                data.push_back(std::move(buffer));
            }
            progress_bar.file_done(reader.file_size());
            reader.close();
        }

        if (m_vout.verbose()) {
            progress_bar.remove();
        }

        m_vout << "Number of buffers: " << buffers_count << "\n";
        m_vout << "Sum of buffer sizes: " << buffers_size << " (" << show_gbytes(buffers_size) << " GB)\n";

        if (buffers_capacity != 0) {
            const auto fill_factor = std::round(100 * static_cast<double>(buffers_size) / static_cast<double>(buffers_capacity));
            m_vout << "Sum of buffer capacities: " << buffers_capacity
                   << " (" << show_gbytes(buffers_capacity) << " GB, " << fill_factor << "% full)\n";
        } else {
            m_vout << "Sum of buffer capacities: 0 (0 GB)\n";
        }

        m_vout << "Sorting data...\n";
        objects.sort(osmium::object_order_type_id_version());

        m_vout << "Writing out sorted data...\n";
        auto out = osmium::io::make_output_iterator(writer);
        std::copy(objects.begin(), objects.end(), out);
    }

    progress_bar.done();

    m_vout << "Closing output file...\n";
    writer.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

bool CommandSort::run() {
    if (m_strategy == "simple") {
        return run_single_pass();
    }
    return run_multi_pass();
}

