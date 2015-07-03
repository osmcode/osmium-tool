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

#include <numeric>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/util/string.hpp>

#include "command_getid.hpp"

std::vector<osmium::object_id_type>& CommandGetId::ids(osmium::item_type type) noexcept {
    return m_ids[osmium::item_type_to_nwr_index(type)];
}

void CommandGetId::sort_unique(osmium::item_type type) {
    std::sort(ids(type).begin(), ids(type).end());
    auto last = std::unique(ids(type).begin(), ids(type).end());
    ids(type).erase(last, ids(type).end());
}

bool CommandGetId::setup(const std::vector<std::string>& arguments) {
    namespace po = boost::program_options;
    po::variables_map vm;

    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("verbose,v", "Set verbose mode")
    ("output,o", po::value<std::string>(), "Output file")
    ("output-format,f", po::value<std::string>(), "Format of output file")
    ("input-format,F", po::value<std::string>(), "Format of input file")
    ("generator", po::value<std::string>(), "Generator setting for file header")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ;

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("ids", po::value<std::vector<std::string>>(), "OSM ids")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("ids", -1);

    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("ids")) {
        std::string sids;
        for (const auto& s : vm["ids"].as<std::vector<std::string>>()) {
            sids += s + " ";
        }
        auto vids = osmium::split_string(sids, "\t ;,/|", true);
        for (const auto& s : vids) {
            auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr);
            if (p.first == osmium::item_type::undefined) {
                p.first = osmium::item_type::node;
            }
            ids(p.first).push_back(p.second);
        }

        sort_unique(osmium::item_type::node);
        sort_unique(osmium::item_type::way);
        sort_unique(osmium::item_type::relation);
    } else {
        throw argument_error("Need at least one id to look for...");
    }

    if (vm.count("verbose")) {
        m_vout.verbose(true);
    }

    setup_input_file(vm);
    setup_output_file(vm);

    m_vout << "Started osmium apply-changes\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input data file name: " << m_input_filename << "\n";
    m_vout << "  output filename: " << m_output_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  output format: " << m_output_format << "\n";
    m_vout << "  looking for these ids:\n";
    m_vout << "    nodes:";
    for (osmium::object_id_type id : ids(osmium::item_type::node)) {
        m_vout << " " << id;
    }
    m_vout << "\n";
    m_vout << "    ways:";
    for (osmium::object_id_type id : ids(osmium::item_type::way)) {
        m_vout << " " << id;
    }
    m_vout << "\n";
    m_vout << "    relations:";
    for (osmium::object_id_type id : ids(osmium::item_type::relation)) {
        m_vout << " " << id;
    }
    m_vout << "\n";

    return true;
}

bool CommandGetId::run() {
    osmium::osm_entity_bits::type types = osmium::osm_entity_bits::nothing;

    if (! ids(osmium::item_type::node).empty()) {
        types |= osmium::osm_entity_bits::node;
    }
    if (! ids(osmium::item_type::way).empty()) {
        types |= osmium::osm_entity_bits::way;
    }
    if (! ids(osmium::item_type::relation).empty()) {
        types |= osmium::osm_entity_bits::relation;
    }

    m_vout << "Reading input file...\n";
    osmium::io::Reader reader(m_input_file, types);

    osmium::io::Header header;
    header.set("generator", m_generator);

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::OSMObject> object_iterator;

    object_iterator it(reader);
    object_iterator end;
    osmium::memory::Buffer output_buffer(10240);

    size_t num_ids = ids(osmium::item_type::node).size() +
                     ids(osmium::item_type::way).size() +
                     ids(osmium::item_type::relation).size();

    for (; it != end; ++it) {
        auto& index = ids(it->type());
        auto result = std::equal_range(index.begin(), index.end(), it->id());
        if (result.first != result.second) {
            output_buffer.add_item(*it);
            output_buffer.commit();
            --num_ids;
        }
    }

    m_vout << "Writing out results...\n";
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite);
    writer(std::move(output_buffer));
    writer.close();

    if (num_ids == 0) {
        m_vout << "Found all objects.\n";
    } else {
        m_vout << "Did not find " << num_ids << " objects.\n";
    }
    m_vout << "Done.\n";

    return num_ids == 0;
}

namespace {

    const bool register_get_id_command = CommandFactory::add("getid", "Get objects with given ID from OSM file", []() {
        return new CommandGetId();
    });

}

