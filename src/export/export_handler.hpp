#ifndef EXPORT_EXPORT_HANDLER_HPP
#define EXPORT_EXPORT_HANDLER_HPP

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

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <osmium/fwd.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/tags/tags_filter.hpp>

#include "export_format.hpp"

class ExportHandler : public osmium::handler::Handler {

    std::unique_ptr<ExportFormat> m_handler;
    osmium::TagsFilter m_linear_filter;
    osmium::TagsFilter m_area_filter;
    uint64_t m_error_count = 0;
    bool m_show_errors;
    bool m_stop_on_error;

    bool is_linear(const osmium::Way& way) const noexcept;

    bool is_area(const osmium::Area& area) const noexcept;

    void show_error(const std::runtime_error& error);

public:

    ExportHandler(std::unique_ptr<ExportFormat>&& handler,
                  const std::vector<std::string>& linear_tags,
                  const std::vector<std::string>& area_tags,
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
