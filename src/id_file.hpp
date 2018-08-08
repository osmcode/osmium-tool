#ifndef ID_FILES_HPP
#define ID_FILES_HPP

#include <osmium/index/id_set.hpp>
#include <osmium/index/nwr_array.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/way.hpp>

#include <string>

using ids_type = osmium::nwr_array<osmium::index::IdSetDense<osmium::unsigned_object_id_type>>;

void add_nodes(const osmium::Way& way, ids_type& ids);
void read_id_osm_file(const std::string& file_name, ids_type& ids);

void parse_and_add_id(const std::string& s, ids_type& ids, osmium::item_type default_item_type);
void read_id_file(std::istream& stream, ids_type& ids, osmium::item_type default_item_type);

bool no_ids(const ids_type& ids) noexcept;

#endif // ID_FILES_HPP
