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

#include "util.hpp"

#include "exception.hpp"

#include <osmium/io/file.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/tags/tags_filter.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/string.hpp>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/**
 * Get the suffix of the given file name. The suffix is everything after
 * the *first* dot (.). So multiple suffixes will all be returned.
 *
 * france.poly    -> poly
 * planet.osm.bz2 -> osm.bz2
 * some/path/planet.osm.bz2 -> osm.bz2
 */
std::string get_filename_suffix(const std::string& file_name) {
    auto slash = file_name.find_last_of('/');
    if (slash == std::string::npos) {
        slash = 0;
    }
    const auto dot = file_name.find_first_of('.', slash);
    if (dot == std::string::npos) {
        return "";
    }
    return file_name.substr(dot + 1);
}

const char* yes_no(bool choice) noexcept {
    return choice ? "yes\n" : "no\n";
}

void warning(const char* text) {
    std::cerr << "WARNING: " << text;
}

void warning(const std::string& text) {
    std::cerr << "WARNING: " << text;
}

std::size_t file_size(const osmium::io::File& file) {
    if (file.filename().empty()) {
        return 0;
    }
    return osmium::file_size(file.filename());
}

std::size_t file_size_sum(const std::vector<osmium::io::File>& files) {
    std::size_t sum = 0;

    for (const auto& file : files) {
        sum += file_size(file);
    }

    return sum;
}

osmium::osm_entity_bits::type get_types(const std::string& str) {
    osmium::osm_entity_bits::type entities{osmium::osm_entity_bits::nothing};

    for (const auto c : str) {
        switch (c) {
            case 'n':
                entities |= osmium::osm_entity_bits::node;
                break;
            case 'w':
                entities |= osmium::osm_entity_bits::way;
                break;
            case 'r':
                entities |= osmium::osm_entity_bits::relation;
                break;
            case 'a':
                entities |= osmium::osm_entity_bits::area;
                break;
            default:
                throw argument_error{std::string{"Unknown object type '"} + c + "' (allowed are 'n', 'w', 'r', and 'a')."};
        }
    }

    return entities;
}

std::pair<osmium::osm_entity_bits::type, std::string> get_filter_expression(const std::string& str) {
    auto pos = str.find('/');

    osmium::osm_entity_bits::type entities{osmium::osm_entity_bits::nwr};
    if (pos == std::string::npos) {
        pos = 0;
    } else if (pos == 0) {
        pos = 1;
    } else {
        entities = get_types(str.substr(0, pos));
        ++pos;
    }

    return std::make_pair(entities, &str[pos]);
}

void strip_whitespace(std::string& string) {
    while (!string.empty() && string.back() == ' ') {
        string.pop_back();
    }

    const auto pos = string.find_first_not_of(' ');
    if (pos != std::string::npos) {
        string.erase(0, pos);
    }
}

osmium::StringMatcher get_string_matcher(std::string string) {
    strip_whitespace(string);

    if (string.size() == 1 && string.front() == '*') {
        return osmium::StringMatcher::always_true{};
    }

    if (string.empty() || (string.back() != '*' && string.front() != '*')) {
        if (string.find(',') == std::string::npos) {
            return osmium::StringMatcher::equal{string};
        }
        auto sstrings = osmium::split_string(string, ',');
        for (auto& s : sstrings) {
            strip_whitespace(s);
        }
        return osmium::StringMatcher::list{sstrings};
    }

    auto s = string;

    if (s.back() == '*' && s.front() != '*') {
        s.pop_back();
        return osmium::StringMatcher::prefix{s};
    }

    if (s.front() == '*') {
        s.erase(0, 1);
    }

    if (!s.empty() && s.back() == '*') {
        s.pop_back();
    }

    return osmium::StringMatcher::substring{s};
}

osmium::TagMatcher get_tag_matcher(const std::string& expression, bool* has_value_matcher) {
    const auto op_pos = expression.find('=');
    if (op_pos == std::string::npos) {
        if (has_value_matcher) {
            *has_value_matcher = false;
        }
        return osmium::TagMatcher{get_string_matcher(expression)};
    }

    auto key = expression.substr(0, op_pos);
    const auto value = expression.substr(op_pos + 1);

    bool invert = false;
    if (!key.empty() && key.back() == '!') {
        key.pop_back();
        invert = true;
    }

    if (has_value_matcher) {
        *has_value_matcher = true;
    }
    return osmium::TagMatcher{get_string_matcher(key), get_string_matcher(value), invert};
}

void initialize_tags_filter(osmium::TagsFilter& tags_filter, const bool default_result, const std::vector<std::string>& strings) {
    tags_filter.set_default_result(default_result);
    for (const auto& str : strings) {
        assert(!str.empty());
        tags_filter.add_rule(!default_result, get_tag_matcher(str));
    }
}

osmium::Box parse_bbox(const std::string& str, const std::string& option_name) {
    const auto coordinates = osmium::split_string(str, ',');

    if (coordinates.size() != 4) {
        throw argument_error{"Need exactly four coordinates in " + option_name + " option."};
    }

    osmium::Location location1;
    location1.set_lon(coordinates[0].c_str());
    location1.set_lat(coordinates[1].c_str());

    osmium::Location location2;
    location2.set_lon(coordinates[2].c_str());
    location2.set_lat(coordinates[3].c_str());

    osmium::Box box;
    box.extend(location1);
    box.extend(location2);

    if (!box.valid()) {
        throw argument_error{"Invalid bounding box in " + option_name + " option. Format is LONG1,LAT1,LONG2,LAT2."};
    }

    return box;
}

osmium::item_type parse_item_type(const std::string& t) {
    if (t == "n" || t == "node") {
        return osmium::item_type::node;
    }

    if (t == "w" || t == "way") {
        return osmium::item_type::way;
    }

    if (t == "r" || t == "relation") {
        return osmium::item_type::relation;
    }

    throw argument_error{std::string{"Unknown default type '"} + t + "' (Allowed are 'node', 'way', and 'relation')."};
}

const char* object_type_as_string(const osmium::OSMObject& object) noexcept {
    if (object.type() == osmium::item_type::area) {
        if (static_cast<const osmium::Area&>(object).from_way()) {
            return "way";
        }
        return "relation";
    }
    return osmium::item_type_to_name(object.type());
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }

    return std::strcmp(str.c_str() + (str.size() - suffix.size()),
                       suffix.c_str()) == 0;
}

std::size_t show_mbytes(std::size_t value) noexcept {
    return value / (1024UL * 1024UL);
}

double show_gbytes(std::size_t value) noexcept {
    return static_cast<double>(show_mbytes(value)) / 1000; // NOLINT(bugprone-integer-division)
}

