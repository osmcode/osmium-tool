#ifndef COMMAND_RENUMBER_HPP
#define COMMAND_RENUMBER_HPP

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

#include "cmd.hpp" // IWYU pragma: export

#include <osmium/handler/check_order.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Holds the mapping from old IDs to new IDs of one object type.
 */
class id_map {

    // Internally this uses two different means of storing the mapping:
    //
    // Most of the old IDs are stored in a sorted vector. The index into the
    // vector is the new ID. All IDs from the nodes, ways, and relations
    // themselves will end up here.
    std::vector<osmium::object_id_type> m_ids;

    // For IDs that can't be written into the sorted vector because this would
    // destroy the sorting, a hash map is used. These are the IDs not read
    // in order, ie the node IDs referenced from the ways and the member IDs
    // referenced from the relations.
    std::unordered_map<osmium::object_id_type, osmium::object_id_type> m_extra_ids;

    // Because we still have to allocate unique new IDs for the mappings
    // ending up in m_extra_ids, we add dummy IDs of the same value as the
    // last one to the end of the m_ids vector. This gives us new IDs without
    // destroying the ordering of m_ids. But to find a new ID from an old ID
    // in m_ids we have to take the first of potentially several identical
    // IDs we find (using std::lower_bound), its position is then the new ID.

    osmium::object_id_type m_start_id = 1;

public:

    id_map() = default;

    osmium::object_id_type start_id() const noexcept {
        return m_start_id;
    }

    void set_start_id(osmium::object_id_type start_id) noexcept {
        m_start_id = start_id;
    }

    osmium::object_id_type add_offset_to_id(osmium::object_id_type id) const noexcept;

    // Map from old ID to new ID. If the old ID has been seen before, it will
    // be returned, otherwise a new ID will be allocated and stored.
    osmium::object_id_type operator()(osmium::object_id_type id);

    // Write the mappings into a file in binary form. This will first copy
    // the mappings from m_extra_ids into the m_ids vector. After this
    // operation this object becomes unusable!
    void write(int fd);

    void print(osmium::object_id_type new_id);

    // Read the mappings from a binary file into m_ids and m_extra_ids.
    void read(int fd, std::size_t file_size);

    // The number of mappings currently existing. Also the last allocated
    // new ID.
    std::size_t size() const noexcept {
        return m_ids.size();
    }

}; // class id_map

class CommandRenumber : public CommandWithSingleOSMInput, public with_osm_output {

    std::string m_index_directory;

    osmium::handler::CheckOrder m_check_order;

    // id mappings for nodes, ways, and relations
    osmium::nwr_array<id_map> m_id_map;

    void renumber(osmium::memory::Buffer& buffer);

    std::string filename(const char* name) const;

    void set_start_ids(const std::string& str);

    void read_start_ids_file();

    void read_index(osmium::item_type type);

    void write_index(osmium::item_type type);

    void show_index(const std::string& type);

public:

    explicit CommandRenumber(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "renumber";
    }

    const char* synopsis() const noexcept override final {
        return "osmium renumber [OPTIONS] OSM-FILE";
    }

}; // class CommandRenumber


#endif // COMMAND_RENUMBER_HPP
