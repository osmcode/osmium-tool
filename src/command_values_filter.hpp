#ifndef COMMAND_VALUES_FILTER_HPP
#define COMMAND_VALUES_FILTER_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2017  Jochen Topf <jochen@topf.org>

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

#include "cmd.hpp" // IWYU pragma: export

#include <osmium/fwd.hpp>
#include <osmium/index/id_set.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/tags/tags_filter.hpp>

#include <string>
#include <vector>

namespace osmium {

    namespace index {
        class RelationsMapIndex;
    } // namespace index

} // namespace osmium

class CommandValuesFilter : public Command, public with_single_osm_input, public with_osm_output {

    bool m_add_referenced_objects = true;
    bool m_invert_match = false;

    osmium::nwr_array<osmium::TagsFilter> m_filters;

    osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>> m_ids;

    osmium::osm_entity_bits::type get_needed_types() const;

    void find_referenced_objects();

    void add_nodes(const osmium::Way& way);
    void add_members(const osmium::Relation& relation);

    void mark_rel_ids(const osmium::index::RelationsMapIndex& rel_in_rel, osmium::object_id_type parent_id);
    bool find_relations_in_relations();
    void find_nodes_and_ways_in_relations();
    void find_nodes_in_ways();

    void add_filter(osmium::osm_entity_bits::type entities, const osmium::TagMatcher& matcher);
    void parse_and_add_expression(const std::string& expression);
    void read_expressions_file(const std::string& file_name);

public:

    explicit CommandValuesFilter(const CommandFactory& command_factory) :
        Command(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "tags-filter";
    }

    const char* synopsis() const noexcept override final {
        return "osmium values-filter [OPTIONS] OSM-FILE FILTER-EXPRESSION...\n"
               "       osmium values-filter [OPTIONS] --expressions=FILE OSM-FILE";
    }

}; // class CommandValuesFilter


#endif // COMMAND_VALUES_FILTER_HPP
