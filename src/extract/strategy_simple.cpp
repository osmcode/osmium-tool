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

#include "strategy_simple.hpp"

#include "../util.hpp"

#include <osmium/handler/check_order.hpp>

#include <cstdlib>
#include <memory>
#include <vector>

namespace strategy_simple {

    Strategy::Strategy(const std::vector<std::unique_ptr<Extract>>& extracts, const osmium::Options& options) {
        m_extracts.reserve(extracts.size());
        for (const auto& extract : extracts) {
            m_extracts.emplace_back(*extract);
        }

        for (const auto& option : options) {
            warning(std::string{"Ignoring unknown option '"} + option.first + "' for 'simple' strategy.\n");
        }
    }

    const char* Strategy::name() const noexcept {
        return "simple";
    }

    class Pass1 : public Pass<Strategy, Pass1> {

        osmium::handler::CheckOrder m_check_order;

    public:

        explicit Pass1(Strategy* strategy) :
            Pass(strategy) {
        }

        void node(const osmium::Node& node) {
            m_check_order.node(node);
        }

        void enode(extract_data* e, const osmium::Node& node) {
            if (e->contains(node.location())) {
                e->write(node);
                e->node_ids.set(node.positive_id());
            }
        }

        void way(const osmium::Way& way) {
            m_check_order.way(way);
        }

        void eway(extract_data* e, const osmium::Way& way) {
            for (const auto& nr : way.nodes()) {
                if (e->node_ids.get(nr.positive_ref())) {
                    e->write(way);
                    e->way_ids.set(way.positive_id());
                }
                return;
            }
        }

        void relation(const osmium::Relation& relation) {
            m_check_order.relation(relation);
        }

        void erelation(extract_data* e, const osmium::Relation& relation) {
            for (const auto& member : relation.members()) {
                switch (member.type()) {
                    case osmium::item_type::node:
                        if (e->node_ids.get(member.positive_ref())) {
                            e->write(relation);
                        }
                        return;
                    case osmium::item_type::way:
                        if (e->way_ids.get(member.positive_ref())) {
                            e->write(relation);
                        }
                        return;
                    default:
                        break;
                }
            }
        }

    }; // class Pass1

    void Strategy::run(osmium::VerboseOutput& vout, bool display_progress, const osmium::io::File& input_file) {
        vout << "Running 'simple' strategy in one pass...\n";
        const std::size_t file_size = input_file.filename().empty() ? 0 : osmium::file_size(input_file.filename());
        osmium::ProgressBar progress_bar{file_size, display_progress};

        Pass1 pass1{this};
        pass1.run(progress_bar, input_file);

        progress_bar.done();
    }

} // namespace strategy_simple

