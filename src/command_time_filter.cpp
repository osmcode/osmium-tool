/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013  Jochen Topf <jochen@topf.org>

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

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/diff_iterator.hpp>

#include "command_time_filter.hpp"

bool CommandTimeFilter::setup(const std::vector<std::string>& arguments) {
    po::variables_map vm;
    try {
        po::options_description cmdline("Allowed options");
        cmdline.add_options()
        ("verbose,v", "Set verbose mode")
        ("output,o", po::value<std::string>(), "Output file")
        ("output-format,f", po::value<std::string>(), "Format of output file")
        ("input-format,F", po::value<std::string>(), "Format of input file")
        ("generator", po::value<std::string>(), "Generator setting for file header")
        ("overwrite,O", "Allow existing output file to be overwritten")
        ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
        ("input-filename", po::value<std::string>(), "OSM input file")
        ("time-from", po::value<std::string>(), "Start of time range")
        ("time-to", po::value<std::string>(), "End of time range")
        ;

        po::options_description desc("Allowed options");
        desc.add(cmdline).add(hidden);

        po::positional_options_description positional;
        positional.add("input-filename", 1);
        positional.add("time-from", 1);
        positional.add("time-to", 1);

        po::store(po::command_line_parser(arguments).options(desc).positional(positional).run(), vm);
        po::notify(vm);

        if (vm.count("input-filename")) {
            m_input_filename = vm["input-filename"].as<std::string>();
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

        if (vm.count("overwrite")) {
            m_output_overwrite = true;
        }

        if (vm.count("verbose")) {
            m_vout.verbose(true);
        }

        if (vm.count("generator")) {
            m_generator = vm["generator"].as<std::string>();
        }

        m_from = osmium::Timestamp(time(0));
        m_to = m_from;

        if (vm.count("time-from")) {
            m_from = osmium::Timestamp(vm["time-from"].as<std::string>().c_str());
            m_to = m_from;
        }

        if (vm.count("time-to")) {
            m_to = osmium::Timestamp(vm["time-to"].as<std::string>().c_str());
            if (m_to < m_from) {
                std::cerr << "Second timestamp is before first one.\n";
                return false;
            }
        }

    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        return false;
    }

    if ((m_output_filename == "-" || m_output_filename == "") && m_output_format.empty()) {
        std::cerr << "When writing to STDOUT you need to use the --output-format,f option to declare the file format.\n";
        return false;
    }

    if ((m_input_filename == "-" || m_input_filename == "") && m_input_format.empty()) {
        std::cerr << "When reading from STDIN you need to use the --input-format,F option to declare the file format.\n";
        return false;
    }

    m_input_file = osmium::io::File(m_input_filename, m_input_format);
    m_output_file = osmium::io::File(m_output_filename, m_output_format);

    if (m_from == m_to) { // point in time
        if (m_output_file.has_multiple_object_versions()) {
            std::cerr << "Warning! You are writing to a file marked as having multiple object versions,\n";
            std::cerr << "but there will be only a single version of each object.\n";
        }
    } else { // time range
        if (!m_output_file.has_multiple_object_versions()) {
            std::cerr << "Warning! You are writing to a file marked as having a single object version,\n";
            std::cerr << "but there might be multiple versions of each object.\n";
        }
    }

    m_vout << "Start osmium-time-filter\n";
    m_vout << "Filtering from time " << m_from.to_iso() << " to " << m_to.to_iso() << "\n";

    return true;
}

bool CommandTimeFilter::run() {
    osmium::io::Reader reader(m_input_file);

    osmium::io::Header header = reader.header();
    header.set("generator", m_generator);

    osmium::io::Writer writer(m_output_file, header, m_output_overwrite);
    osmium::io::OutputIterator<osmium::io::Writer> out(writer);

    typedef osmium::io::InputIterator<osmium::io::Reader, osmium::Object> object_iterator;

    object_iterator object_it(reader);
    object_iterator object_end;

    typedef osmium::DiffIterator<object_iterator> diff_iterator;

    std::copy_if(
        diff_iterator(object_it, object_end),
        diff_iterator(object_end, object_end),
        out,
        [this](const osmium::DiffObject& d){
            return ((d.end_time() == 0 || d.end_time() > m_from) &&
                   d.start_time() <= m_to) &&
                   (m_from != m_to || d.curr().visible());
    });

    out.flush();
    writer.close();

    return true;
}

namespace {

    const bool register_time_filter_command = CommandFactory::add("time-filter", "Filter OSM data from a point in time or a time span out of a history file", []() {
        return new CommandTimeFilter();
    });

}

