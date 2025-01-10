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

#include "command_check_refs.hpp"

#include "util.hpp"

#include <osmium/handler.hpp>
#include <osmium/handler/check_order.hpp>
#include <osmium/index/id_set.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

bool CommandCheckRefs::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("show-ids,i", "Show IDs of missing objects")
    ("check-relations,r", "Also check relations")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input);

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

    if (vm.count("show-ids")) {
        m_show_ids = true;
    }

    if (vm.count("check-relations")) {
        m_check_relations = true;
    }

    return true;
}

void CommandCheckRefs::show_arguments() {
    show_single_input_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    show ids: " << yes_no(m_show_ids);
    m_vout << "    check relations: " << yes_no(m_check_relations);
}

class RefCheckHandler : public osmium::handler::Handler {

    osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>> m_idset_pos;
    osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>> m_idset_neg;

    std::vector<std::pair<osmium::object_id_type, osmium::object_id_type>> m_relation_refs;

    osmium::handler::CheckOrder m_check_order;

    uint64_t m_node_count = 0;
    uint64_t m_way_count = 0;
    uint64_t m_relation_count = 0;

    uint64_t m_missing_nodes_in_ways = 0;
    uint64_t m_missing_nodes_in_relations = 0;
    uint64_t m_missing_ways_in_relations = 0;

    osmium::VerboseOutput* m_vout;
    osmium::ProgressBar* m_progress_bar;
    bool m_show_ids;
    bool m_check_relations;

    void set(osmium::item_type type, osmium::object_id_type id) {
        (id > 0 ? m_idset_pos(type) : m_idset_neg(type)).set(std::abs(id));
    }

    bool get(osmium::item_type type, osmium::object_id_type id) const noexcept {
        return (id > 0 ? m_idset_pos(type) : m_idset_neg(type)).get(std::abs(id));
    }

public:

    RefCheckHandler(osmium::VerboseOutput* vout, osmium::ProgressBar* progress_bar, bool show_ids, bool check_relations) :
        m_vout(vout),
        m_progress_bar(progress_bar),
        m_show_ids(show_ids),
        m_check_relations(check_relations) {
        assert(vout);
        assert(progress_bar);
    }

    uint64_t node_count() const noexcept {
        return m_node_count;
    }

    uint64_t way_count() const noexcept {
        return m_way_count;
    }

    uint64_t relation_count() const noexcept {
        return m_relation_count;
    }

    uint64_t missing_nodes_in_ways() const noexcept {
        return m_missing_nodes_in_ways;
    }

    uint64_t missing_nodes_in_relations() const noexcept {
        return m_missing_nodes_in_relations;
    }

    uint64_t missing_ways_in_relations() const noexcept {
        return m_missing_ways_in_relations;
    }

    uint64_t missing_relations_in_relations() const noexcept {
        return m_relation_refs.size();
    }

    void find_missing_relations() {
        std::sort(m_relation_refs.begin(), m_relation_refs.end());

        m_relation_refs.erase(
            std::remove_if(m_relation_refs.begin(), m_relation_refs.end(), [this](std::pair<osmium::object_id_type, osmium::object_id_type> refs){
                return get(osmium::item_type::relation, refs.first);
            }),
            m_relation_refs.end()
        );
    }

    bool no_errors() const noexcept {
        return missing_nodes_in_ways()          == 0 &&
               missing_nodes_in_relations()     == 0 &&
               missing_ways_in_relations()      == 0 &&
               missing_relations_in_relations() == 0;
    }

    void node(const osmium::Node& node) {
        m_check_order.node(node);

        if (m_node_count == 0) {
            m_progress_bar->remove();
            *m_vout << "Reading nodes...\n";
        }
        ++m_node_count;

        set(osmium::item_type::node, node.id());
    }

    void way(const osmium::Way& way) {
        m_check_order.way(way);

        if (m_way_count == 0) {
            m_progress_bar->remove();
            *m_vout << "Reading ways...\n";
        }
        ++m_way_count;

        if (m_check_relations) {
            set(osmium::item_type::way, way.id());
        }

        for (const auto& node_ref : way.nodes()) {
            if (!get(osmium::item_type::node, node_ref.ref())) {
                ++m_missing_nodes_in_ways;
                if (m_show_ids) {
                    std::cout << "n" << node_ref.ref() << " in w" << way.id() << "\n";
                }
            }
        }
    }

    void relation(const osmium::Relation& relation) {
        m_check_order.relation(relation);

        if (m_relation_count == 0) {
            m_progress_bar->remove();
            *m_vout << "Reading relations...\n";
        }
        ++m_relation_count;

        if (m_check_relations) {
            set(osmium::item_type::relation, relation.id());
            for (const auto& member : relation.members()) {
                switch (member.type()) {
                    case osmium::item_type::node:
                        if (!get(osmium::item_type::node, member.ref())) {
                            ++m_missing_nodes_in_relations;
                            set(osmium::item_type::node, member.ref());
                            if (m_show_ids) {
                                std::cout << "n" << member.ref() << " in r" << relation.id() << "\n";
                            }
                        }
                        break;
                    case osmium::item_type::way:
                        if (!get(osmium::item_type::way, member.ref())) {
                            ++m_missing_ways_in_relations;
                            set(osmium::item_type::way, member.ref());
                            if (m_show_ids) {
                                std::cout << "w" << member.ref() << " in r" << relation.id() << "\n";
                            }
                        }
                        break;
                    case osmium::item_type::relation:
                        if (member.ref() > relation.id() || !get(osmium::item_type::relation, member.ref())) {
                            m_relation_refs.emplace_back(member.ref(), relation.id());
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void show_missing_relation_ids() {
        for (const auto& refs : m_relation_refs) {
            std::cout << "r" << refs.first << " in r" << refs.second << "\n";
        }
    }

    std::size_t used_memory() const noexcept {
        return m_idset_pos(osmium::item_type::node).used_memory() +
               m_idset_pos(osmium::item_type::way).used_memory() +
               m_idset_pos(osmium::item_type::relation).used_memory() +
               m_idset_neg(osmium::item_type::node).used_memory() +
               m_idset_neg(osmium::item_type::way).used_memory() +
               m_idset_neg(osmium::item_type::relation).used_memory() +
               (m_relation_refs.capacity() * sizeof(decltype(m_relation_refs)::value_type));
    }

}; // class RefCheckHandler

bool CommandCheckRefs::run() {
    osmium::io::Reader reader{m_input_file};
    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    RefCheckHandler handler{&m_vout, &progress_bar, m_show_ids, m_check_relations};

    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        osmium::apply(buffer, handler);
    }
    progress_bar.done();

    reader.close();

    if (m_check_relations) {
        handler.find_missing_relations();

        if (m_show_ids) {
            handler.show_missing_relation_ids();
        }
    }

    std::cerr << "There are " << handler.node_count() << " nodes, "
                              << handler.way_count() << " ways, and "
                              << handler.relation_count() << " relations in this file.\n";

    if (m_check_relations) {
        std::cerr << "Nodes     in ways      missing: " << handler.missing_nodes_in_ways()          << "\n";
        std::cerr << "Nodes     in relations missing: " << handler.missing_nodes_in_relations()     << "\n";
        std::cerr << "Ways      in relations missing: " << handler.missing_ways_in_relations()      << "\n";
        std::cerr << "Relations in relations missing: " << handler.missing_relations_in_relations() << "\n";
    } else {
        std::cerr << "Nodes in ways missing: " << handler.missing_nodes_in_ways() << "\n";
    }

    m_vout << "Memory used for indexes: " << show_mbytes(handler.used_memory()) << " MBytes\n";

    show_memory_used();
    m_vout << "Done.\n";

    return handler.no_errors();
}

