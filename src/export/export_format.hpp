#ifndef EXPORT_EXPORT_FORMAT_HPP
#define EXPORT_EXPORT_FORMAT_HPP

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

#include "options.hpp"

#include <osmium/fwd.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/util/verbose_output.hpp>

#include <cstdint>

class ExportFormat {

    const options_type& m_options;

protected:

    std::uint64_t m_count;

    explicit ExportFormat(const options_type& options) :
        m_options(options),
        m_count(0) {
    }

public:

    const options_type& options() const noexcept {
        return m_options;
    }

    std::uint64_t count() const noexcept {
        return m_count;
    }

    virtual ~ExportFormat() = default;

    virtual void node(const osmium::Node&) = 0;

    virtual void way(const osmium::Way&) = 0;

    virtual void area(const osmium::Area&) = 0;

    virtual void close() = 0;

    virtual void debug_output(osmium::VerboseOutput& /*out*/, const std::string& /*filename*/) {
    }

    template <typename TFunc>
    bool add_tags(const osmium::OSMObject& object, TFunc&& func) {
        bool has_tags = false;

        for (const auto& tag : object.tags()) {
            if (options().tags_filter(tag)) {

                // If the tag key looks like any of the attribute keys, drop
                // the tag on the floor. This should be okay for most cases
                // when the attribute name chosen is sufficiently special.
                if (!options().type.empty() && tag.key() == options().type) {
                    continue;
                }
                if (!options().id.empty() && tag.key() == options().id) {
                    continue;
                }
                if (!options().version.empty() && tag.key() == options().version) {
                    continue;
                }
                if (!options().changeset.empty() && tag.key() == options().changeset) {
                    continue;
                }
                if (!options().uid.empty() && tag.key() == options().uid) {
                    continue;
                }
                if (!options().user.empty() && tag.key() == options().user) {
                    continue;
                }
                if (!options().timestamp.empty() && tag.key() == options().timestamp) {
                    continue;
                }
                if (!options().way_nodes.empty() && tag.key() == options().way_nodes) {
                    continue;
                }

                has_tags = true;

                std::forward<TFunc>(func)(tag);
            }
        }

        return has_tags;
    }

}; // class ExportFormat

#endif // EXPORT_EXPORT_FORMAT_HPP
