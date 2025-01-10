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

#include "option_clean.hpp"

#include "exception.hpp"

#include <osmium/osm/object.hpp>
#include <osmium/osm/types.hpp>

#include <string>
#include <vector>

void OptionClean::setup(const boost::program_options::variables_map& vm) {
    if (vm.count("clean")) {
        for (const auto& c : vm["clean"].as<std::vector<std::string>>()) {
            if (c == "version") {
                m_clean_attrs |= clean_options::clean_version;
            } else if (c == "changeset") {
                m_clean_attrs |= clean_options::clean_changeset;
            } else if (c == "timestamp") {
                m_clean_attrs |= clean_options::clean_timestamp;
            } else if (c == "uid") {
                m_clean_attrs |= clean_options::clean_uid;
            } else if (c == "user") {
                m_clean_attrs |= clean_options::clean_user;
            } else {
                throw argument_error{"Unknown attribute on --clean option: '" + c + "'"};
            }
        }
    }
}

void OptionClean::clean_buffer(osmium::memory::Buffer& buffer) const {
    for (auto& object : buffer.select<osmium::OSMObject>()) {
        if (m_clean_attrs & clean_options::clean_version) {
            object.set_version(static_cast<osmium::object_version_type>(0));
        }
        if (m_clean_attrs & clean_options::clean_changeset) {
            object.set_changeset(static_cast<osmium::changeset_id_type>(0));
        }
        if (m_clean_attrs & clean_options::clean_timestamp) {
            object.set_timestamp(osmium::Timestamp{});
        }
        if (m_clean_attrs & clean_options::clean_uid) {
            object.set_uid(static_cast<osmium::user_id_type>(0));
        }
        if (m_clean_attrs & clean_options::clean_user) {
            object.clear_user();
        }
    }
}

std::string OptionClean::to_string() const {
    if (!m_clean_attrs) {
        return "(none)";
    }

    std::string clean_names;
    if (m_clean_attrs & clean_options::clean_version) {
        clean_names += "version,";
    }
    if (m_clean_attrs & clean_options::clean_changeset) {
        clean_names += "changeset,";
    }
    if (m_clean_attrs & clean_options::clean_timestamp) {
        clean_names += "timestamp,";
    }
    if (m_clean_attrs & clean_options::clean_uid) {
        clean_names += "uid,";
    }
    if (m_clean_attrs & clean_options::clean_user) {
        clean_names += "user,";
    }

    clean_names.resize(clean_names.size() - 1);

    return clean_names;
}
