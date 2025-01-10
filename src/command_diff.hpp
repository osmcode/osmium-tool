#ifndef COMMAND_DIFF_HPP
#define COMMAND_DIFF_HPP

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

#include <osmium/osm/crc.hpp>
#include <osmium/osm/crc_zlib.hpp>

#include <string>
#include <vector>

class CommandDiff : public CommandWithMultipleOSMInputs, public with_osm_output {

    std::string m_output_action;
    bool m_ignore_attrs_changeset = false;
    bool m_ignore_attrs_uid = false;
    bool m_ignore_attrs_user = false;
    bool m_show_summary = false;
    bool m_suppress_common = false;

    void update_object_crc(osmium::CRC<osmium::CRC_zlib>* crc, const osmium::OSMObject &object) const;

public:

    explicit CommandDiff(const CommandFactory& command_factory) :
        CommandWithMultipleOSMInputs(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "diff";
    }

    const char* synopsis() const noexcept override final {
        return "osmium diff [OPTIONS] OSM-FILE1 OSM-FILE2";
    }

}; // class CommandDiff


#endif // COMMAND_DIFF_HPP
