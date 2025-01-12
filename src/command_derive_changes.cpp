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

#include "command_derive_changes.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/builder/attr.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <ctime>
#include <string>
#include <vector>

bool CommandDeriveChanges::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("increment-version", "Increment version of deleted objects")
    ("keep-details",      "Keep tags (and nodes of ways, members of relations) of deleted objects")
    ("update-timestamp",  "Set timestamp of deleted objects to current time")
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

    if (m_input_files.size() != 2) {
        throw argument_error{"You need exactly two input files for this command."};
    }

    if (vm.count("increment-version")) {
        m_increment_version = true;
    }

    if (vm.count("keep-details")) {
        m_keep_details = true;
    }

    if (vm.count("update-timestamp")) {
        m_update_timestamp = true;
    }

    return true;
}

void CommandDeriveChanges::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    on deleted objects:\n";
    m_vout << "      increment version: " << yes_no(m_increment_version);
    m_vout << "      keep details: "      << yes_no(m_keep_details);
    m_vout << "      update timestamp: "  << yes_no(m_update_timestamp);
}

void CommandDeriveChanges::write_deleted(osmium::io::Writer& writer, osmium::OSMObject& object) {
    if (m_increment_version) {
        object.set_version(object.version() + 1);
    }

    if (m_update_timestamp) {
        object.set_timestamp(std::time(nullptr));
    }

    if (m_keep_details) {
        object.set_visible(false);
        writer(object);
    } else {
        using namespace osmium::builder::attr; // NOLINT(google-build-using-namespace)
        if (object.type() == osmium::item_type::node) {
            osmium::builder::add_node(m_buffer,
                _deleted(),
                _id(object.id()),
                _version(object.version()),
                _timestamp(object.timestamp())
            );
        } else if (object.type() == osmium::item_type::way) {
            osmium::builder::add_way(m_buffer,
                 _deleted(),
                 _id(object.id()),
                 _version(object.version()),
                 _timestamp(object.timestamp())
             );
        } else if (object.type() == osmium::item_type::relation) {
            osmium::builder::add_relation(m_buffer,
                 _deleted(),
                 _id(object.id()),
                 _version(object.version()),
                 _timestamp(object.timestamp())
             );
        }
        writer(m_buffer.get<osmium::OSMObject>(0));
        m_buffer.clear();
    }
}

bool CommandDeriveChanges::run() {
    m_vout << "Opening output file...\n";
    if (m_output_file.format() != osmium::io::file_format::xml || !m_output_file.is_true("xml_change_format")) {
        warning("Output format chosen is not the XML change format. Use .osc(.gz|bz2) as suffix or -f option.\n");
    }
    osmium::io::Writer writer{m_output_file, m_output_overwrite, m_fsync};

    m_vout << "Opening input files...\n";
    osmium::io::Reader reader1{m_input_files[0], osmium::osm_entity_bits::object};
    osmium::io::ReaderWithProgressBar reader2{display_progress(), m_input_files[1], osmium::osm_entity_bits::object};
    auto in1 = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader1);
    auto in2 = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader2);
    auto it1 = in1.begin();
    auto it2 = in2.begin();
    auto end1 = in1.end();
    auto end2 = in2.end();

    osmium::io::Header header;
    setup_header(header);
    writer.set_header(header);

    reader2.progress_bar().remove();
    m_vout << "Deriving changes...\n";
    while (it1 != end1 || it2 != end2) {
        if (it2 == end2) {
            write_deleted(writer, *it1);
            ++it1;
        } else if (it1 == end1 || *it2 < *it1) {
            writer(*it2);
            ++it2;
        } else if (*it1 < *it2) {
            if (it2->id() != it1->id()) {
                write_deleted(writer, *it1);
            }
            ++it1;
        } else { /* *it1 == *it2 */
            ++it1;
            ++it2;
        }
    }

    writer.close();
    reader2.close();
    reader1.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

