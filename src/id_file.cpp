
#include "id_file.hpp"

#include "util.hpp"

#include <osmium/io/reader.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/way.hpp>

#include <istream>
#include <string>

void add_nodes(const osmium::Way& way, ids_type& ids) {
    for (const auto& nr : way.nodes()) {
        ids(osmium::item_type::node).set(nr.positive_ref());
    }
}

void read_id_osm_file(const std::string& file_name, ids_type& ids) {
    osmium::io::Reader reader{file_name, osmium::osm_entity_bits::object};
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            ids(object.type()).set(object.positive_id());
        }
    }
    reader.close();
}

void parse_and_add_id(const std::string& s, ids_type& ids, osmium::item_type default_item_type) {
    auto p = osmium::string_to_object_id(s.c_str(), osmium::osm_entity_bits::nwr, default_item_type);
    if (p.second < 0) {
        throw std::runtime_error{"This command does not work with negative IDs"};
    }
    ids(p.first).set(static_cast<osmium::unsigned_object_id_type>(p.second));
}

void read_id_file(std::istream& stream, ids_type& ids, osmium::item_type default_item_type) {
    for (std::string line; std::getline(stream, line);) {
        strip_whitespace(line);
        const auto pos = line.find_first_of(" #");
        if (pos != std::string::npos) {
            line.erase(pos);
        }
        if (!line.empty()) {
            parse_and_add_id(line, ids, default_item_type);
        }
    }
}

bool no_ids(const ids_type& ids) noexcept {
    return ids(osmium::item_type::node).empty() &&
           ids(osmium::item_type::way).empty() &&
           ids(osmium::item_type::relation).empty();
}

