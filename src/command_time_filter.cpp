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

#include "command_time_filter.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/diff_iterator.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/osm/diff_object.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <string>
#include <vector>

bool CommandTimeFilter::setup(const std::vector<std::string>& arguments) {
    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("time-from", po::value<std::string>(), "Start of time range")
    ("time-to", po::value<std::string>(), "End of time range")
    ;

    po::options_description desc;
    desc.add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("time-from", 1);
    positional.add("time-to", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    m_from = osmium::Timestamp{std::time(nullptr)};
    m_to = m_from;

    if (vm.count("time-from")) {
        const auto ts = vm["time-from"].as<std::string>();
        try {
            m_from = osmium::Timestamp{ts};
        } catch (const std::invalid_argument&) {
            throw argument_error{"Wrong format for (first) timestamp (use YYYY-MM-DDThh:mm:ssZ)."};
        }
        m_to = m_from;
    }

    if (vm.count("time-to")) {
        const auto ts = vm["time-to"].as<std::string>();
        try {
            m_to = osmium::Timestamp{ts};
        } catch (const std::invalid_argument&) {
            throw argument_error{"Wrong format for second timestamp (use YYYY-MM-DDThh:mm:ssZ)."};
        }
    }

    if (m_from > m_to) {
        throw argument_error{"Second timestamp is before first one."};
    }

    if (m_from == m_to) { // point in time
        if (m_output_file.has_multiple_object_versions()) {
            warning("You are writing to a file marked as having multiple object versions,\n"
                    "but there will be only a single version of each object.\n");
        }
    } else { // time range
        if (!m_output_file.has_multiple_object_versions()) {
            warning("You are writing to a file marked as having a single object version,\n"
                    "but there might be multiple versions of each object.\n");
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
    osmium::io::ReaderWithProgressBar reader{display_progress(), m_input_file, osmium::osm_entity_bits::object};

    m_vout << "Opening output file...\n";
    osmium::io::Header header{reader.header()};
    try {
        osmium::Timestamp const input_timestamp{header.get("osmosis_replication_timestamp")};
        if (input_timestamp.valid() && input_timestamp >= m_to) {
            auto ts = m_to;
            if (m_from != m_to) {
                ts -= 1;
            }
            m_vout << "Set output timestamp to " << ts.to_iso() << "\n";
            header.set("osmosis_replication_timestamp", ts.to_iso());
        }
    } catch (const std::invalid_argument&) { // NOLINT(bugprone-empty-catch)
        // Ignore unset or invalid timestamp from input file
    }
    setup_header(header);

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

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
            [this](const osmium::DiffObject& d) {
                return d.is_visible_at(m_from);
            });
    } else {
        std::copy_if(
            diff_begin,
            diff_end,
            out,
            [this](const osmium::DiffObject& d) {
                return d.is_between(m_from, m_to);
            });
    }

    m_vout << "Closing output file...\n";
    writer.close();

    m_vout << "Closing input file...\n";
    reader.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

