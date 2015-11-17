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

#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/diff_iterator.hpp>

#include "command_time_filter.hpp"
#include "exception.hpp"

bool CommandTimeFilter::setup(const std::vector<std::string>& arguments) {
    po::options_description cmdline("Allowed options");

    add_common_options(cmdline);
    add_single_input_options(cmdline);
    add_output_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("time-from", po::value<std::string>(), "Start of time range")
    ("time-to", po::value<std::string>(), "End of time range")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("time-from", 1);
    positional.add("time-to", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    m_from = osmium::Timestamp(time(0));
    m_to = m_from;

    if (vm.count("time-from")) {
        m_from = osmium::Timestamp(vm["time-from"].as<std::string>());
        m_to = m_from;
    }

    if (vm.count("time-to")) {
        m_to = osmium::Timestamp(vm["time-to"].as<std::string>());
        if (m_to < m_from) {
            throw argument_error("Second timestamp is before first one.");
        }
    }

    if (m_from == m_to) { // point in time
        if (m_output_file.has_multiple_object_versions()) {
            std::cerr << "Warning! You are writing to a file marked as having multiple object versions,\n";
            std::cerr << "but there will be only a single version of each object.\n";
        }
    } else { // time range
        if (!m_output_file.has_multiple_object_versions()) {
            std::cerr << "Warning! You are writing to a file marked as having a single object version,\n";
            std::cerr << "but there might be multiple versions of each object.\n";
        }
    }

    return true;
}

void CommandTimeFilter::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    Filtering from time " << m_from.to_iso() << " to " << m_to.to_iso() << "\n";
}

bool CommandTimeFilter::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader(m_input_file, osmium::osm_entity_bits::object);

    m_vout << "Opening output file...\n";
    osmium::io::Header header = reader.header();
    header.set("generator", m_generator);
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);

    m_vout << "Filter data while copying it from input to output...\n";
    auto input = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader);
    auto diff_begin = osmium::make_diff_iterator(input.begin(), input.end());
    auto diff_end   = osmium::make_diff_iterator(input.end(), input.end());
    auto out = osmium::io::make_output_iterator(writer);

    if (m_from == m_to) {
        std::copy_if(
            diff_begin,
            diff_end,
            out,
            [this](const osmium::DiffObject& d){
                return d.is_visible_at(m_from);
        });
    } else {
        std::copy_if(
            diff_begin,
            diff_end,
            out,
            [this](const osmium::DiffObject& d){
                return d.is_between(m_from, m_to);
        });
    }

    m_vout << "Closing output file...\n";
    writer.close();

    m_vout << "Closing input file...\n";
    reader.close();

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_time_filter_command = CommandFactory::add("time-filter", "Filter OSM data from a point in time or a time span out of a history file", []() {
        return new CommandTimeFilter();
    });

}

