#ifndef COMMAND_ADD_REFS_HPP
#define COMMAND_ADD_REFS_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2016  Jochen Topf <jochen@topf.org>

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

#include <map>
#include <set>
#include <string>
#include <vector>

#include <osmium/fwd.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/types.hpp>

#include "cmd.hpp"

class CommandAddRefs : public Command, public with_multiple_osm_inputs, public with_osm_output {

    std::string m_source_filename;
    std::string m_source_format;
    osmium::io::File m_source_file;
    bool m_work_with_history = false;

    std::set<osmium::object_id_type> m_node_ids;
    std::set<osmium::object_id_type> m_way_ids;
    std::set<osmium::object_id_type> m_relation_ids;

public:

    CommandAddRefs() = default;

    void parse_and_add_id(const std::string& s);
    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    void add_nodes(const osmium::Way& way);
    osmium::osm_entity_bits::type add_members(const osmium::Relation& relation);

    void read_input_files();
    void mark_rel_ids(const std::multimap<osmium::object_id_type, osmium::object_id_type>& rel_in_rel, osmium::object_id_type id);
    bool find_relations_in_relations();
    void find_nodes_and_ways_in_relations();
    void find_nodes_in_ways();

}; // class CommandAddRefs


#endif // COMMAND_ADD_REFS_HPP
