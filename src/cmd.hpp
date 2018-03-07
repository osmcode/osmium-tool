#ifndef CMD_HPP
#define CMD_HPP

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

#include <osmium/io/file.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/util/file.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

const char* get_osmium_version() noexcept;
const char* get_osmium_long_version() noexcept;
const char* get_libosmium_version() noexcept;

namespace osmium {

    namespace io {
        class Header;
    } // namespace io

} // namespace osmium

class CommandFactory;

namespace po = boost::program_options;

/**
 * Virtual base class for commands that can be called from the command line.
 */
class Command {

    osmium::osm_entity_bits::type m_osm_entity_bits = osmium::osm_entity_bits::all;

    enum class display_progress_type {
        never  = 0,
        on_tty = 1,
        always = 2
    } m_display_progress = display_progress_type::on_tty;

protected:

    const CommandFactory& m_command_factory;
    osmium::util::VerboseOutput m_vout{false};

public:

    explicit Command(const CommandFactory& command_factory) :
        m_command_factory(command_factory) {
    }

    virtual ~Command() {
    }

    // This function parses the command line arguments for a
    // command.
    // It returns true if the parsing was successful and the run()
    // function should be called. It returns false if the work is
    // done and run() should not be called.
    // It throws if there was a problem with the arguments.
    //
    // This function should not attempt to open any files or
    // do any other actual work. That will happen in the run()
    // function.
    virtual bool setup(const std::vector<std::string>&) {
        return true;
    }

    // Show command line arguments. This is only called when the
    // verbose option is true;
    virtual void show_arguments() {
    }

    // Run the actual command.
    // It returns true if everything ran successfully.
    // It returns false if there was an error.
    virtual bool run() = 0;

    // The name of the command.
    virtual const char* name() const noexcept = 0;

    // The command line usage synopsis of the command.
    virtual const char* synopsis() const noexcept = 0;

    po::options_description add_common_options(bool with_progress = true);
    void setup_common(const boost::program_options::variables_map& vm, const po::options_description& desc);
    void setup_progress(const boost::program_options::variables_map& vm);
    void setup_object_type_nwrc(const boost::program_options::variables_map& vm);
    void setup_object_type_nwr(const boost::program_options::variables_map& vm);
    void show_object_types(osmium::util::VerboseOutput& vout);
    void print_arguments(const std::string& command);
    void show_memory_used();

    osmium::osm_entity_bits::type osm_entity_bits() const {
        return m_osm_entity_bits;
    }

    bool display_progress() const {
        switch (m_display_progress) {
            case display_progress_type::on_tty:
                return osmium::util::isatty(1) && osmium::util::isatty(2); // if STDOUT and STDERR are a TTY
            case display_progress_type::always:
                return true;
            default:
                break;
        }
        return false;
    }

}; // class Command

class with_single_osm_input {

protected:

    std::string m_input_filename;
    std::string m_input_format;
    osmium::io::File m_input_file;

public:

    void setup_input_file(const boost::program_options::variables_map& vm);

    po::options_description add_single_input_options();

    void show_single_input_arguments(osmium::util::VerboseOutput& vout);

    const osmium::io::File& input_file() const {
        return m_input_file;
    }

}; // class with_single_osm_input

class with_multiple_osm_inputs {

protected:

    std::vector<std::string> m_input_filenames;
    std::string m_input_format;
    std::vector<osmium::io::File> m_input_files;

public:

    void setup_input_files(const boost::program_options::variables_map& vm);

    po::options_description add_multiple_inputs_options();

    void show_multiple_inputs_arguments(osmium::util::VerboseOutput& vout);

    const std::vector<osmium::io::File>& input_files() const {
        return m_input_files;
    }

}; // class with_multiple_osm_inputs

class with_osm_output {

protected:

    std::string m_generator;
    std::vector<std::string> m_output_headers;
    std::string m_output_filename;
    std::string m_output_format;
    osmium::io::File m_output_file;
    osmium::io::overwrite m_output_overwrite = osmium::io::overwrite::no;
    osmium::io::fsync m_fsync = osmium::io::fsync::no;

public:

    with_osm_output() :
        m_generator("osmium/") {
        m_generator.append(get_osmium_version());
    }

    void init_output_file(const po::variables_map& vm);
    void check_output_file();
    void setup_output_file(const po::variables_map& vm);

    po::options_description add_output_options();

    void show_output_arguments(osmium::util::VerboseOutput& vout);

    const osmium::io::File& output_file() const {
        return m_output_file;
    }

    void setup_header(osmium::io::Header& header) const;

    osmium::io::overwrite output_overwrite() const {
        return m_output_overwrite;
    }

}; // class with_osm_output


/**
 * All commands than can be called from the command line are registered
 * with this factory. When the program is running it uses this factory
 * to create the command object from the class depending on the name of
 * the command.
 */
class CommandFactory {

    using create_command_type = std::function<Command*()>;

    struct command_info {
        std::string description; // description of command for help
        create_command_type create; // function that creates C++ object
    };

    std::map<const std::string, command_info> m_commands;

public:

    bool register_command(const std::string& name, const std::string& description, create_command_type&& create_function);

    // Return a vector with names and descriptions of all commands
    std::vector<std::pair<std::string, std::string>> help() const;

    int max_command_name_length() const;

    std::string get_description(const std::string& name) const;

    // This will create a C++ command object from the given name and
    // return it wrapped in a unique_ptr.
    std::unique_ptr<Command> create_command(const std::string& name) const;

}; // class CommandFactory

void register_commands(CommandFactory& cmd_factory);

#endif // CMD_HPP
