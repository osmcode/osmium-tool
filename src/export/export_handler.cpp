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

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <osmium/geom/factory.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/tags/taglist.hpp>

#include "../exception.hpp"
#include "../util.hpp"
#include "export_handler.hpp"

static bool check_filter(const osmium::TagList& tags, const char* area_tag_value, const osmium::TagsFilter& filter) noexcept {
    const char* area_tag = tags.get_value_by_key("area");

    if (area_tag) {
        // has "area" tag and check that it does NOT have the area_tag_value
        return std::strcmp(area_tag, area_tag_value);
    }

    return osmium::tags::match_any_of(tags, filter);
}

bool ExportHandler::is_linear(const osmium::Way& way) const noexcept {
    return check_filter(way.tags(), "yes", m_linear_filter);
}

bool ExportHandler::is_area(const osmium::Area& area) const noexcept {
    return check_filter(area.tags(), "no", m_area_filter);
}

ExportHandler::ExportHandler(std::unique_ptr<ExportFormat>&& handler,
                             const std::vector<std::string>& linear_tags,
                             const std::vector<std::string>& area_tags,
                             bool show_errors) :
    m_handler(std::move(handler)),
    m_linear_filter(true),
    m_area_filter(true),
    m_show_errors(show_errors) {
    if (!linear_tags.empty()) {
        initialize_tags_filter(m_linear_filter, false, linear_tags);
    }
    if (!area_tags.empty()) {
        initialize_tags_filter(m_area_filter, false, area_tags);
    }
}

void ExportHandler::show_error(const std::runtime_error& e) const {
    if (m_show_errors) {
        std::cout << "Geometry error: " << e.what() << "\n";
    }
}

void ExportHandler::node(const osmium::Node& node) const {
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

void ExportHandler::way(const osmium::Way& way) const {
    if (way.nodes().size() <= 1) {
        return;
    }

    if (!way.is_closed() || is_linear(way)) {
        try {
            m_handler->way(way);
        } catch (const osmium::geometry_error& e) {
            show_error(e);
        } catch (const osmium::invalid_location& e) {
            show_error(e);
        }
    }
}

void ExportHandler::area(const osmium::Area& area) const {
    if (!area.from_way() || is_area(area)) {
        try {
            m_handler->area(area);
        } catch (const osmium::geometry_error& e) {
            show_error(e);
        } catch (const osmium::invalid_location& e) {
            show_error(e);
        }
    }
}

