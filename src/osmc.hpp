#ifndef OSMC_HPP
#define OSMC_HPP

/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2015  Jochen Topf <jochen@topf.org>

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

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <osmium/io/file.hpp>
#include <osmium/io/overwrite.hpp>
#include <osmium/util/verbose_output.hpp>

/**
 *  Thrown when there is a problem with the command line arguments.
 */
struct argument_error : std::runtime_error {

    argument_error(const char* message) :
        std::runtime_error(message) {
    }

    argument_error(const std::string& message) :
        std::runtime_error(message) {
    }

};

/**
 * Virtual base class for commands that can be called from the command line.
 */
class Command {

protected:

    bool m_debug {false};
    osmium::util::VerboseOutput m_vout {false};

public:

    Command() = default;

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

    void add_common_options(po::options_description& options) {
        options.add_options()
        ("verbose,v", "Set verbose mode")
        ;
    }

    void setup_common(const boost::program_options::variables_map& vm) {
        if (vm.count("verbose")) {
            m_vout.verbose(true);
        }
    }

    void print_arguments() {
        if (m_vout.verbose()) {
            show_arguments();
        }
    }

}; // class Command

class with_single_osm_input {

protected:

    std::string m_input_filename = "-"; // default: stdin
    std::string m_input_format;
    osmium::io::File m_input_file;

public:

    void setup_input_file(const boost::program_options::variables_map& vm) {
        if (vm.count("input-filename")) {
            m_input_filename = vm["input-filename"].as<std::string>();
        }

        if (vm.count("input-format")) {
            m_input_format = vm["input-format"].as<std::string>();
        }

        if ((m_input_filename == "-" || m_input_filename == "") && m_input_format.empty()) {
            throw argument_error("When reading from STDIN you need to use the --input-format,F option to declare the file format.");
        }

        m_input_file = osmium::io::File(m_input_filename, m_input_format);
    }

    void add_single_input_options(po::options_description& options) {
        options.add_options()
        ("input-format,F", po::value<std::string>(), "Format of input file")
        ;
    }

}; // class with_single_osm_input

class with_multiple_osm_inputs {

protected:

    std::vector<std::string> m_input_filenames;
    std::string m_input_format;
    std::vector<osmium::io::File> m_input_files;

public:

    void setup_input_files(const boost::program_options::variables_map& vm) {
        if (vm.count("input-filenames")) {
            m_input_filenames = vm["input-filenames"].as<std::vector<std::string>>();
        } else {
            m_input_filenames.push_back("-"); // default is stdin
        }

        if (vm.count("input-format")) {
            m_input_format = vm["input-format"].as<std::string>();
        }

        if (m_input_format.empty()) {
            bool uses_stdin = false;
            for (auto& filename : m_input_filenames) {
                if (filename.empty() || filename == "-") {
                    uses_stdin = true;
                }
            }
            if (uses_stdin) {
                throw argument_error("When reading from STDIN you need to use the --input-format,F option to declare the file format.");
            }
        }

        for (const std::string& input_filename : m_input_filenames) {
            osmium::io::File input_file(input_filename, m_input_format);
            m_input_files.push_back(input_file);
        }
    }

    void add_multiple_inputs_options(po::options_description& options) {
        options.add_options()
        ("input-format,F", po::value<std::string>(), "Format of input files")
        ;
    }

}; // class with_multiple_osm_inputs

class with_osm_output {

protected:

    std::string m_generator;
    std::vector<std::string> m_output_headers;
    std::string m_output_filename = "-"; // default: stdout
    std::string m_output_format;
    osmium::io::File m_output_file;
    osmium::io::overwrite m_output_overwrite = osmium::io::overwrite::no;

public:

    with_osm_output() :
        m_generator("osmium/" OSMIUM_VERSION) {
    }

    void setup_output_file(const po::variables_map& vm) {
        if (vm.count("generator")) {
            m_generator = vm["generator"].as<std::string>();
        }

        if (vm.count("output")) {
            m_output_filename = vm["output"].as<std::string>();
        }

        if (vm.count("output-format")) {
            m_output_format = vm["output-format"].as<std::string>();
        }

        if (vm.count("output-header")) {
            m_output_headers = vm["output-header"].as<std::vector<std::string>>();
        }

        if (vm.count("overwrite")) {
            m_output_overwrite = osmium::io::overwrite::allow;
        }

        if ((m_output_filename == "-" || m_output_filename == "") && m_output_format.empty()) {
            throw argument_error("When writing to STDOUT you need to use the --output-format,f option to declare the file format.");
        }

        m_output_file = osmium::io::File(m_output_filename, m_output_format);
    }

    void add_output_options(po::options_description& options) {
        options.add_options()
        ("generator", po::value<std::string>(), "Generator setting for file header")
        ("output,o", po::value<std::string>(), "Output file")
        ("output-format,f", po::value<std::string>(), "Format of output file")
        ("output-header", po::value<std::vector<std::string>>(), "Add output header")
        ("overwrite,O", "Allow existing output file to be overwritten")
        ;
    }

}; // class with_osm_output

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

    static std::string get_description(const std::string& name) {
        auto it = instance().m_commands.find(name);
        if (it == instance().m_commands.end()) {
            return "";
        }
        return it->second.description;
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
