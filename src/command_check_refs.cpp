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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include <boost/program_options.hpp>

#include <osmium/index/bool_vector.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include "command_check_refs.hpp"

bool CommandCheckRefs::setup(const std::vector<std::string>& arguments) {
    po::variables_map vm;

    po::options_description cmdline("Allowed options");
    cmdline.add_options()
    ("show-ids,i", "Show IDs of missing objects")
    ("input-format,F", po::value<std::string>(), "Format of input files")
    ("check-relations,r", "Also check relations")
    ;

    add_common_options(cmdline);

    po::options_description hidden("Hidden options");
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc("Allowed options");
    desc.add(cmdline).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm);

    if (vm.count("show-ids")) {
        m_show_ids = true;
    }

    if (vm.count("check-relations")) {
        m_check_relations = true;
    }

    m_vout << "Started osmium check-refs\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  input filename: " << m_input_filename << "\n";
    m_vout << "  input format: " << m_input_format << "\n";
    m_vout << "  show ids: " << (m_show_ids ? "yes\n" : "no\n");
    m_vout << "  check relations: " << (m_check_relations ? "yes\n" : "no\n");

    setup_input_file(vm);

    return true;
}


class RefCheckHandler : public osmium::handler::Handler {

    osmium::index::BoolVector<osmium::unsigned_object_id_type> m_nodes;
    osmium::index::BoolVector<osmium::unsigned_object_id_type> m_ways;

    std::vector<uint32_t> m_relation_ids;
    std::set<uint32_t> m_member_relation_ids;
    std::vector<uint32_t> m_missing_relation_ids;

    uint64_t m_node_count = 0;
    uint64_t m_way_count = 0;
    uint64_t m_relation_count = 0;

    uint64_t m_missing_nodes_in_ways = 0;
    uint64_t m_missing_nodes_in_relations = 0;
    uint64_t m_missing_ways_in_relations = 0;

    osmium::util::VerboseOutput& m_vout;
    bool m_show_ids;
    bool m_check_relations;
    bool m_relations_done = false;

public:

    RefCheckHandler(osmium::util::VerboseOutput& vout, bool show_ids, bool check_relations) :
        m_vout(vout),
        m_show_ids(show_ids),
        m_check_relations(check_relations) {
    }

    uint64_t node_count() const {
        return m_node_count;
    }

    uint64_t way_count() const {
        return m_way_count;
    }

    uint64_t relation_count() const {
        return m_relation_count;
    }

    uint64_t missing_nodes_in_ways() const {
        return m_missing_nodes_in_ways;
    }

    uint64_t missing_nodes_in_relations() const {
        return m_missing_nodes_in_relations;
    }

    uint64_t missing_ways_in_relations() const {
        return m_missing_ways_in_relations;
    }

    uint64_t missing_relations_in_relations() {
        if (!m_relations_done) {
            std::sort(m_relation_ids.begin(), m_relation_ids.end());

            std::set_difference(m_member_relation_ids.cbegin(), m_member_relation_ids.cend(),
                                m_relation_ids.cbegin(), m_relation_ids.cend(),
                                std::back_inserter(m_missing_relation_ids));

            m_relations_done = true;
        }

        return m_missing_relation_ids.size();
    }

    bool any_errors() {
        return missing_nodes_in_ways()          > 0 ||
               missing_nodes_in_relations()     > 0 ||
               missing_ways_in_relations()      > 0 ||
               missing_relations_in_relations() > 0;
    }

    void node(const osmium::Node& node) {
        if (m_node_count == 0) {
            m_vout << "Reading nodes...\n";
        }
        ++m_node_count;

        m_nodes.set(node.positive_id());
    }

    void way(const osmium::Way& way) {
        if (m_way_count == 0) {
            m_vout << "Reading ways...\n";
        }
        ++m_way_count;

        if (m_check_relations) {
            m_ways.set(way.positive_id());
        }

        for (const auto& node_ref : way.nodes()) {
            if (!m_nodes.get(node_ref.positive_ref())) {
                ++m_missing_nodes_in_ways;
                if (m_show_ids) {
                    std::cout << "n" << node_ref.ref() << " in w" << way.id() << "\n";
                }
            }
        }
    }

    void relation(const osmium::Relation& relation) {
        if (m_relation_count == 0) {
            m_vout << "Reading relations...\n";
        }
        ++m_relation_count;

        if (m_check_relations) {
            m_relation_ids.push_back(uint32_t(relation.id()));
            for (const auto& member : relation.members()) {
                switch (member.type()) {
                    case osmium::item_type::node:
                        if (!m_nodes.get(member.positive_ref())) {
                            ++m_missing_nodes_in_relations;
                            m_nodes.set(member.positive_ref());
                            if (m_show_ids) {
                                std::cout << "n" << member.ref() << " in r" << relation.id() << "\n";
                            }
                        }
                        break;
                    case osmium::item_type::way:
                        if (!m_ways.get(member.positive_ref())) {
                            ++m_missing_ways_in_relations;
                            m_ways.set(member.positive_ref());
                            if (m_show_ids) {
                                std::cout << "w" << member.ref() << " in r" << relation.id() << "\n";
                            }
                        }
                        break;
                    case osmium::item_type::relation:
                        m_member_relation_ids.insert(uint32_t(relation.id()));
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void show_missing_relation_ids() {
        for (auto id : m_missing_relation_ids) {
            std::cout << "r" << id << " in r\n";
        }
    }

}; // class RefCheckHandler

bool CommandCheckRefs::run() {
    osmium::io::Reader reader(m_input_file);

    RefCheckHandler handler(m_vout, m_show_ids, m_check_relations);
    osmium::apply(reader, handler);

    std::cerr << "There are " << handler.node_count() << " nodes, " << handler.way_count() << " ways, and " << handler.relation_count() << " relations in this file.\n";

    if (m_check_relations) {
        std::cerr << "Nodes     in ways      missing: " << handler.missing_nodes_in_ways()          << "\n";
        std::cerr << "Nodes     in relations missing: " << handler.missing_nodes_in_relations()     << "\n";
        std::cerr << "Ways      in relations missing: " << handler.missing_ways_in_relations()      << "\n";
        std::cerr << "Relations in relations missing: " << handler.missing_relations_in_relations() << "\n";
    } else {
        std::cerr << "Nodes in ways missing: " << handler.missing_nodes_in_ways() << "\n";
    }

    if (m_show_ids) {
        handler.show_missing_relation_ids();
    }

    m_vout << "Done.\n";

    return !handler.any_errors();
}

namespace {

    const bool register_check_refs_command = CommandFactory::add("check-refs", "Check referential integrity of an OSM file", []() {
        return new CommandCheckRefs();
    });

}

