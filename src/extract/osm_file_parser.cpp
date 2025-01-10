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

#include "osm_file_parser.hpp"

#include "../exception.hpp"

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_collector.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/file.hpp>
#include <osmium/memory/buffer.hpp>

#include <cstddef>
#include <string>
#include <utility>

using index_type = osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

OSMFileParser::OSMFileParser(osmium::memory::Buffer& buffer, std::string file_name) :
    m_buffer(buffer),
    m_file_name(std::move(file_name)) {
}

std::size_t OSMFileParser::operator()() {
    const osmium::io::File input_file{m_file_name};

    const osmium::area::Assembler::config_type assembler_config;
    osmium::area::MultipolygonCollector<osmium::area::Assembler> collector{assembler_config};

    {
        osmium::io::Reader reader{input_file, osmium::osm_entity_bits::relation};
        collector.read_relations(reader);
        reader.close();
    }

    bool has_ring = false;
    try {
        index_type index;
        location_handler_type location_handler{index};
        osmium::builder::AreaBuilder builder{m_buffer};

        osmium::io::Reader reader{input_file};
        osmium::apply(reader, location_handler, collector.handler([&](const osmium::memory::Buffer& buffer) {
            for (const auto& area : buffer.select<osmium::Area>()) {
                for (const auto& item : area) {
                    if (item.type() == osmium::item_type::outer_ring ||
                        item.type() == osmium::item_type::inner_ring) {
                        builder.add_item(item);
                        has_ring = true;
                    }
                }
            }
        }));
        reader.close();
    } catch (const osmium::invalid_location&) {
        throw config_error{"Invalid location in boundary (multi)polygon in '" + m_file_name + "'."};
    } catch (const osmium::not_found&) {
        throw config_error{"Missing node in boundary (multi)polygon in '" + m_file_name + "'."};
    }

    if (has_ring) {
        return m_buffer.commit();
    }

    m_buffer.rollback();
    throw osmium::io_error{"No areas found in the OSM file."};
}

