/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

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

#include <algorithm>
#include <utility>

#include <osmium/geom/wkt.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/segment.hpp>

#include "extract.hpp"

static void add_ring(std::vector<osmium::Segment>& segments, const osmium::NodeRefList& ring) {
    auto it = ring.begin();
    const auto end = ring.end();

    assert(it != end);
    auto last_it = it++;
    while (it != end) {
        segments.emplace_back(last_it->location(), it->location());
        last_it = it++;
    }
}

const osmium::Area& ExtractPolygon::area() const noexcept {
    return m_buffer.get<osmium::Area>(m_offset);
}

ExtractPolygon::ExtractPolygon(const std::string& output, const std::string& output_format, const std::string& description, const osmium::memory::Buffer& buffer, std::size_t offset) :
    Extract(output, output_format, description, buffer.get<osmium::Area>(offset).envelope()),
    m_buffer(buffer),
    m_offset(offset) {

    // prepare area
    for (const auto& outer_ring : area().outer_rings()) {
        add_ring(m_segments, outer_ring);

        for (const auto& inner_ring : area().inner_rings(outer_ring)) {
            add_ring(m_segments, inner_ring);
        }
    }
}

/**
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

*/

bool ExtractPolygon::contains(const osmium::Location& location) const noexcept {
    if (!location.valid() || ! envelope().contains(location)) {
        return false;
    }

    bool inside = false;

    for (const auto& segment : m_segments) {
        if ((segment.second().y() > location.y()) != (segment.first().y() > location.y())) {
            const int64_t ax = int64_t(segment.first().x()) - int64_t(segment.second().x());
            const int64_t ay = int64_t(segment.first().y()) - int64_t(segment.second().y());
            const int64_t tx = int64_t(location.x())        - int64_t(segment.second().x());
            const int64_t ty = int64_t(location.y())        - int64_t(segment.second().y());

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

