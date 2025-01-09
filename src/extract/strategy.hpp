#ifndef EXTRACT_STRATEGY_HPP
#define EXTRACT_STRATEGY_HPP

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

#include "extract.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>

#include <cassert>
#include <memory>

template <typename T>
class ExtractData : public T {

    Extract* m_extract_ptr;

public:

    explicit ExtractData(Extract& extract) :
        T(),
        m_extract_ptr(&extract) {
    }

    bool contains(const osmium::Location& location) const noexcept {
        return m_extract_ptr->contains(location);
    }

    void write(const osmium::memory::Item& item) {
        m_extract_ptr->write(item);
    }

    void close() {
        m_extract_ptr->close_file();
    }

}; // class ExtractData


class ExtractStrategy {

public:

    ExtractStrategy() = default;

    virtual ~ExtractStrategy() = default;

    virtual const char* name() const noexcept = 0;

    virtual void show_arguments(osmium::VerboseOutput& /*vout*/) {
    }

    virtual void run(osmium::VerboseOutput& vout, bool display_progress, const osmium::io::File& input_file) = 0;

}; // class ExtractStrategy


template <typename TStrategy, typename TChild>
class Pass {

    TStrategy* m_strategy;

    void run_impl(osmium::ProgressBar& progress_bar, osmium::io::Reader& reader) {
        while (osmium::memory::Buffer buffer = reader.read()) {
            progress_bar.update(reader.offset());
            for (const auto& object : buffer) {
                switch (object.type()) {
                    case osmium::item_type::node:
                        self().node(static_cast<const osmium::Node&>(object));
                        for (auto& e : extracts()) {
                            self().enode(&e, static_cast<const osmium::Node&>(object));
                        }
                        break;
                    case osmium::item_type::way:
                        self().way(static_cast<const osmium::Way&>(object));
                        for (auto& e : extracts()) {
                            self().eway(&e, static_cast<const osmium::Way&>(object));
                        }
                        break;
                    case osmium::item_type::relation:
                        self().relation(static_cast<const osmium::Relation&>(object));
                        for (auto& e : extracts()) {
                            self().erelation(&e, static_cast<const osmium::Relation&>(object));
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

protected:

    using extract_data = typename TStrategy::extract_data;

    TStrategy& strategy() {
        return *m_strategy;
    }

    std::vector<extract_data>& extracts() {
        return m_strategy->m_extracts;
    }

    TChild& self() {
        return *static_cast<TChild*>(this);
    }

    void node(const osmium::Node&) {
    }

    void way(const osmium::Way&) {
    }

    void relation(const osmium::Relation&) {
    }

    void enode(extract_data*, const osmium::Node&) {
    }

    void eway(extract_data*, const osmium::Way&) {
    }

    void erelation(extract_data*, const osmium::Relation&) {
    }

public:

    explicit Pass(TStrategy* strategy) :
        m_strategy(strategy) {
        assert(strategy);
    }

    template <typename... Args>
    void run(osmium::ProgressBar& progress_bar, Args... args) {
        osmium::io::Reader reader{std::forward<Args>(args)...};
        run_impl(progress_bar, reader);
        reader.close();
    }

}; // class Pass


#endif // EXTRACT_STRATEGY_HPP
