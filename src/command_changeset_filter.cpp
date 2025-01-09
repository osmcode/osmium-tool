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

#include "command_changeset_filter.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/geom/relations.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

bool CommandChangesetFilter::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("with-discussion,d", "Changesets with discussions (comments)")
    ("without-discussion,D", "Changesets without discussions (no comments)")
    ("with-changes,c", "Changesets with changes")
    ("without-changes,C", "Changesets without any changes")
    ("open", "Open changesets")
    ("closed", "Closed changesets")
    ("user,u", po::value<std::string>(), "Changesets by given user")
    ("uid,U", po::value<osmium::user_id_type>(), "Changesets by given user ID")
    ("after,a", po::value<std::string>(), "Changesets opened after this time")
    ("before,b", po::value<std::string>(), "Changesets closed before this time")
    ("bbox,B", po::value<std::string>(), "Changesets overlapping this bounding box")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("with-discussion")) {
        m_with_discussion = true;
    }

    if (vm.count("without-discussion")) {
        m_without_discussion = true;
    }

    if (vm.count("with-changes")) {
        m_with_changes = true;
    }

    if (vm.count("without-changes")) {
        m_without_changes = true;
    }

    if (vm.count("open")) {
        m_open = true;
    }

    if (vm.count("closed")) {
        m_closed = true;
    }

    if (vm.count("uid")) {
        m_uid = vm["uid"].as<osmium::user_id_type>();
    }

    if (vm.count("user")) {
        m_user = vm["user"].as<std::string>();
    }

    if (vm.count("after")) {
        auto ts = vm["after"].as<std::string>();
        try {
            m_after = osmium::Timestamp(ts);
        } catch (const std::invalid_argument&) {
            throw argument_error{"Wrong format for --after/-a timestamp (use YYYY-MM-DDThh:mm:ssZ)."};
        }
    }

    if (vm.count("before")) {
        auto ts = vm["before"].as<std::string>();
        try {
            m_before = osmium::Timestamp(ts);
        } catch (const std::invalid_argument&) {
            throw argument_error{"Wrong format for --before/-b timestamp (use YYYY-MM-DDThh:mm:ssZ)."};
        }
    }

    if (vm.count("bbox")) {
        m_box = parse_bbox(vm["bbox"].as<std::string>(), "--bbox/-B");
    }

    if (m_with_discussion && m_without_discussion) {
        throw argument_error{"You can not use --with-discussion/-d and --without-discussion/-D together."};
    }

    if (m_with_changes && m_without_changes) {
        throw argument_error{"You can not use --with-changes/-c and --without-changes/-C together."};
    }

    if (m_open && m_closed) {
        throw argument_error{"You can not use --open and --closed together."};
    }

    if (m_after > m_before) {
        throw argument_error{"Timestamp 'after' is after 'before'."};
    }

    return true;
}

void CommandChangesetFilter::show_arguments() {
    show_single_input_arguments(m_vout);
    show_output_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    changesets must\n";
    if (m_with_discussion) {
        m_vout << "      - have a discussion\n";
    }
    if (m_without_discussion) {
        m_vout << "      - not have a discussion\n";
    }
    if (m_with_changes) {
        m_vout << "      - have at least one change\n";
    }
    if (m_without_changes) {
        m_vout << "      - not have any changes\n";
    }
    if (m_open) {
        m_vout << "      - be open\n";
    }
    if (m_closed) {
        m_vout << "      - be closed\n";
    }
    if (m_uid != 0) {
        m_vout << "      - be from uid " << m_uid << "\n";
    }
    if (!m_user.empty()) {
        m_vout << "      - be from user '" << m_user << "'\n";
    }
    if (m_after > osmium::start_of_time()) {
        m_vout << "      - be closed after " << m_after.to_iso() << " or still open\n";
    }
    if (m_before < osmium::end_of_time()) {
        m_vout << "      - be created before " << m_before.to_iso() << "\n";
    }
}

namespace {

bool changeset_after(const osmium::Changeset& changeset, osmium::Timestamp time) {
    return changeset.open() || changeset.closed_at() >= time;
}

bool changeset_before(const osmium::Changeset& changeset, osmium::Timestamp time) {
    return changeset.created_at() <= time;
}

} // anonymous namespace

bool CommandChangesetFilter::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file, osmium::osm_entity_bits::changeset};

    auto input = osmium::io::make_input_iterator_range<osmium::Changeset>(reader);

    osmium::io::Header header{reader.header()};
    setup_header(header);

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};
    auto out = osmium::io::make_output_iterator(writer);

    m_vout << "Filtering data...\n";

    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};

    int count = 0;
    std::copy_if(input.begin(), input.end(), out,
        [this, &count, &progress_bar, &reader](const osmium::Changeset& changeset) {
            if (++count > 10000) {
                progress_bar.update(reader.offset());
                count = 0;
            }
            return (!m_with_discussion    || changeset.num_comments() > 0) &&
                   (!m_without_discussion || changeset.num_comments() == 0) &&
                   (!m_with_changes       || changeset.num_changes() > 0) &&
                   (!m_without_changes    || changeset.num_changes() == 0) &&
                   (!m_open               || changeset.open()) &&
                   (!m_closed             || changeset.closed()) &&
                   (m_uid == 0            || changeset.uid() == m_uid) &&
                   (m_user.empty()        || m_user == changeset.user()) &&
                   changeset_after(changeset, m_after) &&
                   changeset_before(changeset, m_before) &&
                   (!m_box.valid()        || (changeset.bounds().valid() && osmium::geom::overlaps(changeset.bounds(), m_box)));

    });

    progress_bar.done();

    writer.close();
    reader.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

