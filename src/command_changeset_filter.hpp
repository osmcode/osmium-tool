#ifndef COMMAND_CHANGESET_FILTER_HPP
#define COMMAND_CHANGESET_FILTER_HPP

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

#include "cmd.hpp" // IWYU pragma: export

#include <osmium/osm/box.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>

#include <string>
#include <vector>

class CommandChangesetFilter : public CommandWithSingleOSMInput, public with_osm_output {

    std::string m_user;
    osmium::Box m_box;
    osmium::Timestamp m_after = osmium::start_of_time();
    osmium::Timestamp m_before = osmium::end_of_time();
    osmium::user_id_type m_uid = 0;

    bool m_with_discussion = false;
    bool m_without_discussion = false;
    bool m_with_changes = false;
    bool m_without_changes = false;
    bool m_open = false;
    bool m_closed = false;

public:

    explicit CommandChangesetFilter(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "changeset-filter";
    }

    const char* synopsis() const noexcept override final {
        return "osmium changeset-filter [OPTIONS] OSM-CHANGESET-FILE";
    }

}; // class CommandChangesetFilter


#endif // COMMAND_CHANGESET_FILTER_HPP
