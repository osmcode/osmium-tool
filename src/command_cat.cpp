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

#include <iostream>
#include <iterator>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>

#include "command_cat.hpp"

bool CommandCat::setup(const std::vector<std::string>& arguments) {
    po::variables_map vm;
    try {
        po::options_description cmdline("Allowed options");
        cmdline.add_options()
        ("verbose,v", "Set verbose mode")
        ("output,o", po::value<std::string>(), "Output file")
        ("output-format,f", po::value<std::string>(), "Format of output file")
        ("input-format,F", po::value<std::string>(), "Format of input files")
        ("generator", po::value<std::string>(), "Generator setting for file header")
        ("output-header", po::value<std::vector<std::string>>(), "Add output header")
        ("overwrite,O", "Allow existing output file to be overwritten")
        ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
        ("input-filenames", po::value<std::vector<std::string>>(), "Input files")
        ;

        po::options_description desc("Allowed options");
        desc.add(cmdline).add(hidden);

        po::positional_options_description positional;
        positional.add("input-filenames", -1);

        po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
        po::notify(vm);

        if (vm.count("verbose")) {
            m_vout.verbose(true);
        }

        if (vm.count("generator")) {
            m_generator = vm["generator"].as<std::string>();
        }

        if (vm.count("input-filenames")) {
            m_input_filenames = vm["input-filenames"].as<std::vector<std::string>>();
        } else {
            m_input_filenames.push_back("-"); // default is stdin
        }

        if (vm.count("output")) {
            m_output_filename = vm["output"].as<std::string>();
        }

        if (vm.count("input-format")) {
            m_input_format = vm["input-format"].as<std::string>();
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

    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        return false;
    }

    m_vout << "Started osmium cat\n";

    m_vout << "Command line options and default settings:\n";
    m_vout << "  generator: " << m_generator << "\n";
    m_vout << "  input-filenames: \n";
    for (const auto& fn : m_input_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  output-filename: " << m_output_filename << "\n";
    m_vout << "  input-format: " << m_input_format << "\n";
    m_vout << "  output-format: " << m_output_format << "\n";
    m_vout << "  output-header: \n";
    for (const auto& h : m_output_headers) {
        m_vout << "    " << h << "\n";
    }

    if ((m_output_filename == "-" || m_output_filename == "") && m_output_format.empty()) {
        std::cerr << "When writing to STDOUT you need to use the --output-format,f option to declare the file format.\n";
        return false;
    }

    if (m_input_format.empty()) {
        bool uses_stdin = false;
        for (auto& filename : m_input_filenames) {
            if (filename.empty() || filename == "-") {
                uses_stdin = true;
            }
        }
        if (uses_stdin) {
            std::cerr << "When reading from STDIN you need to use the --input-format,F option to declare the file format.\n";
            return false;
        }
    }

    m_output_file = osmium::io::File(m_output_filename, m_output_format);

    for (const std::string& input_filename : m_input_filenames) {
        osmium::io::File input_file(input_filename, m_input_format);
        m_input_files.push_back(input_file);
    }

    return true;
}

bool CommandCat::run() {

    try {
        if (m_input_files.size() == 1) { // single input file
            m_vout << "Copying input file '" << m_input_files[0].filename() << "'\n";
            osmium::io::Reader reader(m_input_files[0]);
            osmium::io::Header header = reader.header();
            header.set("generator", m_generator);
            for (const auto& h : m_output_headers) {
                header.set(h);
            }
            osmium::io::Writer writer(m_output_file, header, m_output_overwrite);
            while (osmium::memory::Buffer buffer = reader.read()) {
                writer(std::move(buffer));
            }
            writer.close();
        } else { // multiple input files
            osmium::io::Header header({{"generator", m_generator}});
            for (const auto& h : m_output_headers) {
                header.set(h);
            }
            osmium::io::Writer writer(m_output_file, header, m_output_overwrite);
            for (const auto& input_file : m_input_files) {
                m_vout << "Copying input file '" << input_file.filename() << "'\n";
                osmium::io::Reader reader(input_file);
                while (osmium::memory::Buffer buffer = reader.read()) {
                    writer(std::move(buffer));
                }
            }
            writer.close();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return false;
    }

    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_cat_command = CommandFactory::add("cat", "Concatenate OSM files and convert to different formats", []() {
        return new CommandCat();
    });

}

