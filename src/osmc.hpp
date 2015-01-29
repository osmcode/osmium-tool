#ifndef OSMC_HPP
#define OSMC_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013, 2014  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <osmium/util/verbose_output.hpp>

/**
 * Virtual base class for commands that can be called from the command line.
 */
class Command {

protected:

    std::string m_generator {"osmium/" OSMIUM_VERSION};
    bool m_debug {false};
    osmium::util::VerboseOutput m_vout {false};

public:

    virtual ~Command() {
    }

    // This function parses the command line arguments for a
    // command.
    // It returns true if the parsing was successful.
    // It returns false if the arguments could not be parsed.
    //
    // This function should not attempt to open any files or
    // do any other actual work. That will happen in the run()
    // function.
    virtual bool setup(const std::vector<std::string>&) {
        return true;
    }

    // Run the actual command.
    // It returns true if everything ran successfully.
    // It returns false if there was an error.
    virtual bool run() = 0;

}; // class Command


/**
 * All commands than can be called from the command line are registered
 * with this factory. When the program is running it uses this factory
 * to create the command object from the class depending on the name of
 * the command.
 */
class CommandFactory {

    typedef std::function<Command*()> create_command_type;

    struct command_info {
        std::string description; // description of command for help
        create_command_type create; // function that creates C++ object
    };

    std::map<const std::string, command_info> m_commands;

    // The constructor is private because CommandFactory is a singleton
    CommandFactory() :
        m_commands() {
    }

public:

    // CommandFactory is a singleton, this returns the only instance
    static CommandFactory& instance() {
        static CommandFactory factory;
        return factory;
    }

    // Return a vector with names and descriptions of all commands
    static std::vector<std::pair<std::string, std::string>> help() {
        std::vector<std::pair<std::string, std::string>> commands;
        for (const auto& cmd : instance().m_commands) {
            commands.push_back(std::make_pair(cmd.first, cmd.second.description));
        }
        return commands;
    }

    static bool add(const std::string& name, const std::string& description, create_command_type create_function) {
        return instance().register_command(name, description, create_function);
    }

    bool register_command(const std::string& name, const std::string& description, create_command_type create_function) {
        command_info info {description, create_function};
        return m_commands.insert(std::make_pair(name, info)).second;
    }

    // This will create a C++ command object from the given name and
    // return it wrapped in a unique_ptr.
    std::unique_ptr<Command> create_command(const std::string& name) {
        const auto it = m_commands.find(name);

        if (it != m_commands.end()) {
            return std::unique_ptr<Command>((it->second.create)());
        }

        return std::unique_ptr<Command>();
    }

}; // class CommandFactory

#endif // OSMC_HPP
