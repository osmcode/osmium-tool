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

#include "extract_polygon.hpp"

#include "../exception.hpp"

#include <osmium/geom/wkt.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/segment.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

void add_ring(std::vector<osmium::Segment>* segments, const osmium::NodeRefList& ring) {
    const auto* it = ring.begin();
    const auto* const end = ring.end();

    if (it == end) {
        throw config_error{"Ring without any points."};
    }

    const auto* prev_it = it++;
    while (it != end) {
        segments->emplace_back(prev_it->location(), it->location());
        prev_it = it++;
    }
}

} // anonymous namespace

const osmium::Area& ExtractPolygon::area() const noexcept {
    return m_buffer.get<osmium::Area>(m_offset);
}

ExtractPolygon::ExtractPolygon(const osmium::io::File& output_file, const std::string& description, const osmium::memory::Buffer& buffer, std::size_t offset) :
    Extract(output_file, description, buffer.get<osmium::Area>(offset).envelope()),
    m_buffer(buffer),
    m_offset(offset) {

    // get segments from all rings
    std::vector<osmium::Segment> segments;
    for (const auto& outer_ring : area().outer_rings()) {
        add_ring(&segments, outer_ring);

        for (const auto& inner_ring : area().inner_rings(outer_ring)) {
            add_ring(&segments, inner_ring);
        }
    }

    // split y range into equal-sized bands
    constexpr const int32_t segments_per_band = 10;
    constexpr const int32_t max_bands = 10000;
    int32_t num_bands = static_cast<int32_t>(segments.size()) / segments_per_band;
    if (num_bands < 1) {
        num_bands = 1;
    } else if (num_bands > max_bands) {
        num_bands = max_bands;
    }

    m_bands.resize(num_bands + 1);

    m_dy = (y_max() - y_min() + num_bands - 1) / num_bands;

    // put segments into the bands they overlap
    for (const auto& segment : segments) {
        const std::pair<int32_t, int32_t> mm = std::minmax(segment.first().y(), segment.second().y());
        const uint32_t band_min = (mm.first - y_min()) / m_dy;
        const uint32_t band_max = (mm.second - y_min()) / m_dy;
        assert(band_min < m_bands.size() && band_max < m_bands.size());

        for (auto band = band_min; band <= band_max; ++band) {
            m_bands[band].push_back(segment);
        }
    }
}

/*

  Simple point-in-polygon algorithm adapted from

  https://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

    int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
    {
        int i, j, c = 0;
        for (i = 0, j = nvert-1; i < nvert; j = i++) {
            if ( ((verty[i]>testy) != (verty[j]>testy)) &&
                (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
        }
        return c;
    }

  In our implementation we split the y-range into equal-sized subranges and
  only have to test all segments in the subrange that contains the y coordinate
  of the node.

*/

bool ExtractPolygon::contains(const osmium::Location& location) const noexcept {
    if (!location.valid() || !envelope().contains(location)) {
        return false;
    }

    const std::size_t band = (location.y() - y_min()) / m_dy;
    assert(band < m_bands.size());

    bool inside = false;

    for (const auto& segment : m_bands[band]) {
        if (segment.first() == location || segment.second() == location) {
            return true;
        }
        if ((segment.second().y() > location.y()) != (segment.first().y() > location.y())) {
            const auto ax = static_cast<int64_t>(segment.first().x()) - static_cast<int64_t>(segment.second().x());
            const auto ay = static_cast<int64_t>(segment.first().y()) - static_cast<int64_t>(segment.second().y());
            const auto tx = static_cast<int64_t>(location.x())        - static_cast<int64_t>(segment.second().x());
            const auto ty = static_cast<int64_t>(location.y())        - static_cast<int64_t>(segment.second().y());

            const bool comp = tx * ay < ax * ty;

            if ((ay > 0) == comp) {
                inside = !inside;
            }
        }

    }

    return inside;
}

const char* ExtractPolygon::geometry_type() const noexcept {
    return "polygon";
}

std::string ExtractPolygon::geometry_as_text() const {
    osmium::geom::WKTFactory<> factory;
    return factory.create_multipolygon(area());
}

