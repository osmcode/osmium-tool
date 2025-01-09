#ifndef COMMAND_APPLY_CHANGES_HPP
#define COMMAND_APPLY_CHANGES_HPP

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

#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/object_pointer_collection.hpp>

#include <string>
#include <vector>

class CommandApplyChanges : public CommandWithSingleOSMInput, public with_osm_output {

    std::vector<std::string> m_change_filenames;

    std::string m_change_file_format;

    bool m_with_history = false;
    bool m_locations_on_ways = false;
    bool m_redact = false;

    void apply_changes_and_write(osmium::ObjectPointerCollection &objects,
                                 const std::vector<osmium::memory::Buffer> &changes,
                                 osmium::io::Reader &reader,
                                 osmium::io::Writer &writer);

public:

    explicit CommandApplyChanges(const CommandFactory& command_factory) :
        CommandWithSingleOSMInput(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "apply-changes";
    }

    const char* synopsis() const noexcept override final {
        return "osmium apply-changes [OPTIONS] OSM-FILE OSM-CHANGE-FILE...";
    }

}; // class CommandApplyChanges


#endif // COMMAND_APPLY_CHANGES_HPP
