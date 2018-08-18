#ifndef EXPORT_RULESET_HPP
#define EXPORT_RULESET_HPP

#include "../util.hpp"

#include <osmium/tags/tags_filter.hpp>

#include <string>
#include <vector>

enum class tags_filter_rule_type {
    none  = 0,
    any   = 1,
    list  = 2,
    other = 3
};

class Ruleset {

    tags_filter_rule_type m_type = tags_filter_rule_type::any;
    std::vector<std::string> m_tags;
    osmium::TagsFilter m_filter{false};

public:

    void set_rule_type(tags_filter_rule_type type) noexcept {
        m_type = type;
    }

    tags_filter_rule_type rule_type() const noexcept {
        return m_type;
    }

    const std::vector<std::string>& tags() const noexcept {
        return m_tags;
    }

    template <typename T>
    void add_rule(T&& rule) {
        m_tags.emplace_back(std::forward<T>(rule));
    }

    const osmium::TagsFilter& filter() const noexcept {
        return m_filter;
    }

    void init_filter() {
        switch (m_type) {
            case tags_filter_rule_type::none:
                break;
            case tags_filter_rule_type::any:
                m_filter.set_default_result(true);
                break;
            case tags_filter_rule_type::list:
                initialize_tags_filter(m_filter, false, m_tags);
                break;
            case tags_filter_rule_type::other:
                break;
        }
    }

}; // class Ruleset

#endif // EXPORT_RULESET_HPP
