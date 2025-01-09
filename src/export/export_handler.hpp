#ifndef EXPORT_EXPORT_HANDLER_HPP
#define EXPORT_EXPORT_HANDLER_HPP

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

#include "export_format.hpp"
#include "ruleset.hpp"

#include <osmium/fwd.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/tags/tags_filter.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class ExportHandler : public osmium::handler::Handler {

    std::unique_ptr<ExportFormat> m_handler;
    const Ruleset& m_linear_ruleset;
    const Ruleset& m_area_ruleset;
    uint64_t m_error_count = 0;

    geometry_types m_geometry_types;

    bool m_show_errors;
    bool m_stop_on_error;

    bool is_linear(const osmium::TagList& tags) const noexcept;

    bool is_area(const osmium::TagList& tags) const noexcept;

    void show_error(const std::runtime_error& error);

public:

    ExportHandler(std::unique_ptr<ExportFormat>&& handler,
                  const Ruleset& linear_ruleset,
                  const Ruleset& area_ruleset,
                  geometry_types geometry_types,
                  bool show_errors,
                  bool stop_on_error);

    void node(const osmium::Node& node);

    void way(const osmium::Way& way);

    void area(const osmium::Area& area);

    void close() const {
        m_handler->close();
    }

    std::uint64_t count() const noexcept {
        return m_handler->count();
    }

    std::uint64_t error_count() const noexcept {
        return m_error_count;
    }

}; // class ExportHandler

#endif // EXPORT_EXPORT_HANDLER_HPP
