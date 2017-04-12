#ifndef UTIL_HPP
#define UTIL_HPP

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

#include <string>
#include <utility>
#include <vector>

#include <osmium/osm/entity_bits.hpp>
#include <osmium/util/string_matcher.hpp>
#include <osmium/tags/matcher.hpp>

namespace osmium {

    class TagsFilter;

    namespace io {
        class File;
    }

}

std::string get_filename_suffix(const std::string& file_name);
const char* yes_no(bool choice) noexcept;
void warning(const char* text);
void warning(const std::string& text);
std::size_t file_size_sum(const std::vector<osmium::io::File>& files);
osmium::osm_entity_bits::type get_types(const std::string& s);
std::pair<osmium::osm_entity_bits::type, std::string> get_filter_expression(const std::string& s);
void strip_whitespace(std::string& string);
osmium::StringMatcher get_string_matcher(std::string string);
osmium::TagMatcher get_tag_matcher(const std::string& expression);
void initialize_tags_filter(osmium::TagsFilter& tags_filter, bool default_result, const std::vector<std::string>& strings);

#endif // UTIL_HPP
