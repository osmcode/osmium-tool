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

#include "cmd.hpp"
#include "exception.hpp"
#include "util.hpp"

#include <osmium/io/any_input.hpp> // IWYU pragma: keep
#include <osmium/io/any_output.hpp> // IWYU pragma: keep
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/writer_options.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <string>
#include <vector>

void with_single_osm_input::setup_input_file(const boost::program_options::variables_map& vm) {
    if (vm.count("input-filename")) {
        m_input_filename = vm["input-filename"].as<std::string>();
    }

    if (vm.count("input-format")) {
        m_input_format = vm["input-format"].as<std::string>();
    }

    if (m_input_format.empty()) {
        if (m_input_filename == "-") {
            throw argument_error{"When reading from STDIN you need to use the --input-format/-F option\n"
                                 "to specify the file format."};
        }

        if (m_input_filename.empty()) {
            throw argument_error{"Missing input file. Use '-' to read from STDIN and add the --input-format/-F\n"
                                 "option to specify the file format or specify the input file name."};
        }
    }

    m_input_file = osmium::io::File{m_input_filename, m_input_format};
}

po::options_description with_single_osm_input::add_single_input_options() {
    po::options_description options{"INPUT OPTIONS"};

    options.add_options()
    ("input-format,F", po::value<std::string>(), "Format of input file")
    ;

    return options;
}

void with_single_osm_input::show_single_input_arguments(osmium::VerboseOutput& vout) {
    vout << "  input options:\n";
    vout << "    file name: " << m_input_filename << "\n";
    vout << "    file format: " << m_input_format << "\n";
}

void with_multiple_osm_inputs::setup_input_files(const boost::program_options::variables_map& vm) {
    if (vm.count("input-filenames")) {
        m_input_filenames = vm["input-filenames"].as<std::vector<std::string>>();
    } else {
        m_input_filenames.emplace_back("-"); // default is stdin
    }

    bool uses_stdin = false;
    for (auto& filename : m_input_filenames) {
        if (filename == "-") {
            if (uses_stdin) {
                throw argument_error{"Can read at most one file from STDIN."};
            }
            uses_stdin = true;
        }
    }

    if (vm.count("input-format")) {
        m_input_format = vm["input-format"].as<std::string>();
    }

    if (uses_stdin && m_input_format.empty()) {
        throw argument_error{"When reading from STDIN you need to use the --input-format/-F option\n"
                             "to specify the file format. Or are you missing a file name argument?"};
    }

    for (const std::string& input_filename : m_input_filenames) {
        const osmium::io::File input_file{input_filename, m_input_format};
        m_input_files.push_back(input_file);
    }
}


po::options_description with_multiple_osm_inputs::add_multiple_inputs_options() {
    po::options_description options{"INPUT OPTIONS"};

    options.add_options()
    ("input-format,F", po::value<std::string>(), "Format of input files")
    ;

    return options;
}

void with_multiple_osm_inputs::show_multiple_inputs_arguments(osmium::VerboseOutput& vout) {
    vout << "  input options:\n";
    vout << "    file names: \n";
    for (const auto& fn : m_input_filenames) {
        vout << "      " << fn << "\n";
    }
    vout << "    file format: " << m_input_format << "\n";
}

void with_osm_output::init_output_file(const po::variables_map& vm) {
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
}

void with_osm_output::check_output_file() {
    if (m_output_format.empty()) {
        if (m_output_filename == "-") {
            throw argument_error{"When writing to STDOUT you need to use the --output-format/-f\n"
                                 "option to specify the file format."};
        }
        if (m_output_filename.empty()) {
            throw argument_error{"Missing output file. Set the output file with --output/-o and/or\n"
                                 "add the --output-format/-f option to specify the file format."};
        }
    }

    m_output_file = osmium::io::File{m_output_filename, m_output_format};
    m_output_file.check();
}

void with_osm_output::setup_output_file(const po::variables_map& vm) {
    init_output_file(vm);
    check_output_file();
}

po::options_description with_osm_output::add_output_options() {
    po::options_description options{"OUTPUT OPTIONS"};

    options.add_options()
    ("output-format,f", po::value<std::string>(), "Format of output file")
    ("fsync", "Call fsync after writing file")
    ("generator", po::value<std::string>(), "Generator setting for file header")
    ("output,o", po::value<std::string>(), "Output file")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ("output-header", po::value<std::vector<std::string>>(), "Add output header")
    ;

    return options;
}

void with_osm_output::show_output_arguments(osmium::VerboseOutput& vout) {
    vout << "  output options:\n";
    vout << "    file name: " << m_output_filename << "\n";
    vout << "    file format: " << m_output_format << "\n";
    vout << "    generator: " << m_generator << "\n";
    vout << "    overwrite: " << yes_no(m_output_overwrite == osmium::io::overwrite::allow);
    vout << "    fsync: " << yes_no(m_fsync == osmium::io::fsync::yes);
    if (!m_output_headers.empty()) {
        vout << "    output header:\n";
        for (const auto& h : m_output_headers) {
            vout << "      " << h << "\n";
        }
    }
}

void with_osm_output::setup_header(osmium::io::Header& header) const {
    header.set("generator", m_generator);
    for (const auto& h : m_output_headers) {
        header.set(h);
    }
}

void init_header(osmium::io::Header& header, const osmium::io::Header& input_header, const std::vector<std::string>& options) {
    for (const auto& h : options) {
        if (!h.empty() && h.back() == '!') {
            std::string hc{h};
            hc.resize(h.size() - 1);
            header.set(hc, input_header.get(hc));
        } else {
            header.set(h);
        }
    }
}

void with_osm_output::setup_header(osmium::io::Header& header, const osmium::io::Header& input_header) const {
    header.set("generator", m_generator);
    init_header(header, input_header, m_output_headers);
}

