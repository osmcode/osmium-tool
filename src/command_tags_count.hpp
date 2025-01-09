#ifndef COMMAND_TAGS_COUNT_HPP
#define COMMAND_TAGS_COUNT_HPP

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

#include <osmium/fwd.hpp>
#include <osmium/osm/tag.hpp>
#include <osmium/tags/tags_filter.hpp>

#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * This class stores a key or key-value combination in a single std::string.
 * Key-value combinations are stored as key + \0 + value internally.
 */
class key_or_tag {

    std::string m_value;

public:

    key_or_tag(const osmium::Tag& tag) :
        m_value(tag.key()) {
        m_value += '\0';
        m_value += tag.value();
    }

    key_or_tag(const char* key) :
        m_value(key) {
    }

    /// Return the key.
    const char* key() const noexcept {
        return m_value.c_str();
    }

    /// Return value or nullptr if there is no value stored.
    const char* value() const noexcept {
        const auto pos = std::strlen(m_value.c_str());
        if (pos == m_value.size()) {
            return nullptr;
        }
        return m_value.c_str() + pos + 1;
    }

    const std::string& get() const noexcept {
        return m_value;
    }

    friend bool operator==(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() == b.get();
    }

    friend bool operator!=(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() != b.get();
    }

    friend bool operator<(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() < b.get();
    }

    friend bool operator<=(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() <= b.get();
    }

    friend bool operator>(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() > b.get();
    }

    friend bool operator>=(const key_or_tag& a, const key_or_tag& b) noexcept {
        return a.get() >= b.get();
    }

}; // class key_or_tag

using counter_type = uint32_t;

struct element_type {
    const key_or_tag* name;
    counter_type count;

    element_type(const key_or_tag& n, counter_type c) :
        name(&n),
        count(c) {
    }
};

namespace std {

    template <>
    struct hash<key_or_tag> {
        std::size_t operator()(const key_or_tag& s) const noexcept {
            return std::hash<std::string>{}(s.get());
        }
    };

} // namespace std

using sort_func_type =
    std::function<bool(const element_type&, const element_type&)>;

class CommandTagsCount : public CommandWithSingleOSMInput, public with_osm_output {

    osmium::TagsFilter m_keys_filter;
    osmium::TagsFilter m_tags_filter;

    std::string m_sort_order{"count-desc"};
    sort_func_type m_sort_func;

    std::unordered_map<key_or_tag, counter_type> m_counts;

    counter_type m_min_count = 0;
    counter_type m_max_count = std::numeric_limits<counter_type>::max();

    void add_matcher(const std::string& expression);
    void read_expressions_file(const std::string& file_name);
    std::vector<element_type> sort_results() const;

public:

    explicit CommandTagsCount(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "tags-count";
    }

    const char* synopsis() const noexcept override final {
        return "osmium tags-count [OPTIONS] OSM-FILE [TAG-EXPRESSION...]\n"
               "       osmium tags-count [OPTIONS] --expressions=FILE OSM-FILE";
    }

}; // class CommandTagsCount

#endif // COMMAND_TAGS_COUNT_HPP
