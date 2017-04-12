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

#include <iostream>
#include <string>
#include <vector>

#include <osmium/io/file.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/string.hpp>
#include <osmium/tags/tags_filter.hpp>

#include "exception.hpp"
#include "util.hpp"

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

std::size_t file_size_sum(const std::vector<osmium::io::File>& files) {
    std::size_t sum = 0;

    for (const auto& file : files) {
        sum += osmium::util::file_size(file.filename());
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
            default:
                throw argument_error{std::string{"Unknown object type '"} + c + "' (allowed are 'n', 'w', and 'r')."};
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

osmium::TagMatcher get_tag_matcher(const std::string& expression) {
    const auto op_pos = expression.find('=');
    if (op_pos == std::string::npos) {
        return osmium::TagMatcher{get_string_matcher(expression)};
    }

    auto key = expression.substr(0, op_pos);
    const auto value = expression.substr(op_pos + 1);

    bool invert = false;
    if (!key.empty() && key.back() == '!') {
        key.pop_back();
        invert = true;
    }

    return osmium::TagMatcher{get_string_matcher(key), get_string_matcher(value), invert};
}

void initialize_tags_filter(osmium::TagsFilter& tags_filter, bool default_result, const std::vector<std::string>& strings) {
    tags_filter.set_default_result(default_result);
    for (const auto& str : strings) {
        assert(!str.empty());
        tags_filter.add_rule(!default_result, get_tag_matcher(str));
    }
}

