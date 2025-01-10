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

#include "export_handler.hpp"

#include "../exception.hpp"
#include "../util.hpp"

#include <osmium/geom/factory.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/tags/taglist.hpp>

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

bool check_conditions(const osmium::TagList& tags, const Ruleset& r1, const Ruleset& r2, bool is_no) noexcept {
    const char* area_tag = tags.get_value_by_key("area");
    if (area_tag) {
        if (std::strcmp(area_tag, "no") == 0) {
            return is_no;
        }
        if (std::strcmp(area_tag, "yes") == 0) {
            return !is_no;
        }
    }

    if (r1.rule_type() == tags_filter_rule_type::other) {
        return osmium::tags::match_none_of(tags, r2.filter());
    }

    return osmium::tags::match_any_of(tags, r1.filter());
}

} // anonymous namespace

bool ExportHandler::is_linear(const osmium::TagList& tags) const noexcept {
    return check_conditions(tags, m_linear_ruleset, m_area_ruleset, true);
}

bool ExportHandler::is_area(const osmium::TagList& tags) const noexcept {
    return check_conditions(tags, m_area_ruleset, m_linear_ruleset, false);
}

ExportHandler::ExportHandler(std::unique_ptr<ExportFormat>&& handler,
                             const Ruleset& linear_ruleset,
                             const Ruleset& area_ruleset,
                             geometry_types geometry_types,
                             bool show_errors,
                             bool stop_on_error) :
    m_handler(std::move(handler)),
    m_linear_ruleset(linear_ruleset),
    m_area_ruleset(area_ruleset),
    m_geometry_types(geometry_types),
    m_show_errors(show_errors),
    m_stop_on_error(stop_on_error) {
}

void ExportHandler::show_error(const std::runtime_error& error) {
    if (m_stop_on_error) {
        throw;
    }
    ++m_error_count;
    if (m_show_errors) {
        std::cerr << "Geometry error: " << error.what() << '\n';
    }
}

void ExportHandler::node(const osmium::Node& node) {
    if (!m_geometry_types.point) {
        return;
    }

    if (node.tags().empty() && !m_handler->options().keep_untagged) {
        return;
    }

    try {
        m_handler->node(node);
    } catch (const osmium::geometry_error& e) {
        show_error(e);
    } catch (const osmium::invalid_location& e) {
        show_error(e);
    }
}

void ExportHandler::way(const osmium::Way& way) {
    if (!m_geometry_types.linestring) {
        return;
    }

    try {
        if (way.nodes().size() <= 1) {
            throw osmium::geometry_error{"Way with less than two nodes (id=" + std::to_string(way.id()) + ")"};
        }
        if (!way.nodes().front().location() || !way.nodes().back().location()) {
            throw osmium::invalid_location{"invalid location"};
        }
        if ((way.tags().empty() && m_handler->options().keep_untagged)
            || !way.ends_have_same_location()
            || is_linear(way.tags())) {
                m_handler->way(way);
        }
    } catch (const osmium::geometry_error& e) {
        show_error(e);
    } catch (const osmium::invalid_location& e) {
        show_error(e);
    }
}

void ExportHandler::area(const osmium::Area& area) {
    if (!m_geometry_types.polygon) {
        return;
    }

    if (area.from_way() && !is_area(area.tags())) {
        return;
    }

    try {
        const auto rings = area.num_rings();
        if (rings.first == 0 && rings.second == 0) {
            throw osmium::geometry_error{"Could not build area geometry"};
        }

        m_handler->area(area);
    } catch (const osmium::geometry_error& e) {
        show_error(e);
    } catch (const osmium::invalid_location& e) {
        show_error(e);
    }
}

