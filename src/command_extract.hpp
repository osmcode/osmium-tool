#ifndef COMMAND_EXTRACT_HPP
#define COMMAND_EXTRACT_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2018  Jochen Topf <jochen@topf.org>

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
#include "extract/extract.hpp"
#include "extract/strategy.hpp"

#include <osmium/memory/buffer.hpp>
#include <osmium/util/options.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class CommandExtract : public Command, public with_single_osm_input, public with_osm_output {

    static const std::size_t initial_buffer_size = 10 * 1024;

    std::string m_config_file_name;
    std::string m_config_directory;
    std::string m_output_directory;
    osmium::util::Options m_options;
    std::string m_strategy_name;
    std::unique_ptr<ExtractStrategy> m_strategy;
    std::vector<std::unique_ptr<Extract>> m_extracts;
    osmium::memory::Buffer m_buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
    bool m_with_history = false;
    bool m_set_bounds = false;

    void parse_config_file();
    void show_extracts();

    void set_directory(const std::string& directory);

    std::unique_ptr<ExtractStrategy> make_strategy(const std::string& name);

public:

    explicit CommandExtract(const CommandFactory& command_factory) :
        Command(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    void show_arguments() override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "extract";
    }

    const char* synopsis() const noexcept override final {
        return "osmium extract --config CONFIG-FILE [OPTIONS] OSM-FILE\n"
               "       osmium extract --bbox LEFT,BOTTOM,RIGHT,TOP [OPTIONS] OSM-FILE\n"
               "       osmium extract --polygon POLYGON-FILE [OPTIONS] OSM-FILE";
    }

}; // class CommandExtract


#endif // COMMAND_EXTRACT_HPP
