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

#include "cmd.hpp"
#include "exception.hpp"

void with_single_osm_input::setup_input_file(const boost::program_options::variables_map& vm) {
    if (vm.count("input-filename")) {
        m_input_filename = vm["input-filename"].as<std::string>();
    }

    if (vm.count("input-format")) {
        m_input_format = vm["input-format"].as<std::string>();
    }

    if ((m_input_filename == "-" || m_input_filename == "") && m_input_format.empty()) {
        throw argument_error("When reading from STDIN you need to use the --input-format/-F option to declare the file format.");
    }

    m_input_file = osmium::io::File(m_input_filename, m_input_format);
}

void with_single_osm_input::add_single_input_options(po::options_description& options) {
    options.add_options()
    ("input-format,F", po::value<std::string>(), "Format of input file")
    ;
}

void with_multiple_osm_inputs::setup_input_files(const boost::program_options::variables_map& vm) {
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
            throw argument_error("When reading from STDIN you need to use the --input-format/-F option to declare the file format.");
        }
    }

    for (const std::string& input_filename : m_input_filenames) {
        osmium::io::File input_file(input_filename, m_input_format);
        m_input_files.push_back(input_file);
    }
}

void with_multiple_osm_inputs::add_multiple_inputs_options(po::options_description& options) {
    options.add_options()
    ("input-format,F", po::value<std::string>(), "Format of input files")
    ;
}

void with_osm_output::setup_output_file(const po::variables_map& vm) {
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

    if (vm.count("fsync")) {
        m_fsync = osmium::io::fsync::yes;
    }

    if ((m_output_filename == "-" || m_output_filename == "") && m_output_format.empty()) {
        throw argument_error("When writing to STDOUT you need to use the --output-format/-f option to declare the file format.");
    }

    m_output_file = osmium::io::File(m_output_filename, m_output_format);
    m_output_file.check();
}

void with_osm_output::add_output_options(po::options_description& options) {
    options.add_options()
    ("generator", po::value<std::string>(), "Generator setting for file header")
    ("output,o", po::value<std::string>(), "Output file")
    ("output-format,f", po::value<std::string>(), "Format of output file")
    ("output-header", po::value<std::vector<std::string>>(), "Add output header")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ("fsync", "Call fsync after writing file")
    ;
}

