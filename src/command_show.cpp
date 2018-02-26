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

#include "command_show.hpp"
#include "exception.hpp"
#include "util.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#ifndef _MSC_VER
# include <unistd.h>
#endif

#ifndef _MSC_VER
void CommandShow::setup_pager_from_env() noexcept {
    m_pager = "less";
    const char* pager = ::getenv("OSMIUM_PAGER");
    if (pager) {
        m_pager = pager;
    } else {
        pager = ::getenv("PAGER");
        if (pager) {
            m_pager = pager;
        }
    }

    if (m_pager == "cat") {
        m_pager = "";
    }
}
#endif

bool CommandShow::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("format-debug,d", "Use debug format")
    ("format-opl,o", "Use OPL format")
    ("format-xml,x", "Use XML format")
#ifndef _MSC_VER
    ("no-pager", "Do not run pager program")
#endif
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation, changeset)")
    ("output-format,f", po::value<std::string>(), "Format of output file")
    ;

    po::options_description opts_common{add_common_options(false)};
    po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "Input file")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_object_type_nwrc(vm);
    setup_input_file(vm);

#ifndef _MSC_VER
    if (vm.count("no-pager")) {
        m_pager = "";
    } else {
        setup_pager_from_env();
    }
#endif

    if (vm.count("output-format") &&
        vm.count("format-debug") &&
        vm.count("format-opl") &&
        vm.count("format-xml")) {
        throw argument_error{"You can only use at most one of the following options: --output-format/-f, --format-debug/-d, --format-opl/-o, and --format-xml/-x."};
    }

    if (vm.count("output-format")) {
        m_output_format = vm["output-format"].as<std::string>();
    } else if (vm.count("format-debug")) {
        m_output_format = "debug,color=true";
    } else if (vm.count("format-opl")) {
        m_output_format = "opl";
    } else if (vm.count("format-xml")) {
        m_output_format = "xml";
    } else {
        const char* output_format_from_env = ::getenv("OSMIUM_SHOW_FORMAT");
        if (output_format_from_env) {
            m_output_format = output_format_from_env;
        }
    }

    m_color_output = m_output_format.find("color=true") != std::string::npos;

    return true;
}

void CommandShow::show_arguments() {
    show_single_input_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    file format: " << m_output_format << "\n";
    m_vout << "    use color: " << yes_no(m_color_output);
    m_vout << "    use pager: " << (m_pager.empty() ? "(no pager)" : m_pager) << "\n";
    show_object_types(m_vout);
}

#ifndef _MSC_VER
static int execute_pager(const std::string& pager, bool with_color) {
    int pipefd[2];
    if (::pipe(pipefd) < 0) {
        throw std::system_error{errno, std::system_category(), "Could not run pager: pipe() call failed"};
    }

    const pid_t pid = fork();
    if (pid < 0) {
        throw std::system_error{errno, std::system_category(), "Could not run pager: fork() call failed"};
    }

    if (pid == 0) {
        // child
        ::close(pipefd[1]); // close write end of the pipe
        ::close(0); // close stdin
        if (::dup2(pipefd[0], 0) < 0) { // put end of pipe as stdin
            std::exit(1);
        }

        if (with_color && pager.substr(pager.size() - 4, 4) == "less") {
            ::execlp(pager.c_str(), pager.c_str(), "-R", nullptr);
        } else {
            // execute pager without arguments
            ::execlp(pager.c_str(), pager.c_str(), nullptr);
        }

        // Exec will either succeed and never return here, or it fails and
        // we'll exit.
        std::exit(1);
    }

    // parent
    ::close(pipefd[0]); // close read end of the pipe

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) { // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        throw std::system_error{errno, std::system_category(), "Could not run pager: signal() call failed"};
    }

    return pipefd[1];
}
#endif

bool CommandShow::run() {
    osmium::io::Reader reader{m_input_file, osm_entity_bits()};
    osmium::io::Header header{reader.header()};

    if (m_pager.empty()) {
        osmium::io::File file{"-", m_output_format};
        osmium::io::Writer writer{file, header};
        while (osmium::memory::Buffer buffer = reader.read()) {
            writer(std::move(buffer));
        }
        writer.close();
    } else {
#ifndef _MSC_VER
        const int fd = execute_pager(m_pager, m_color_output);

        ::close(1); // close stdout
        if (::dup2(fd, 1) < 0) { // put end of pipe as stdout
            throw std::system_error{errno, std::system_category(), "Could not run pager: dup2() call failed"};
        }

        osmium::io::File file{"-", m_output_format};
        osmium::io::Writer writer{file, header};
        try {
            while (osmium::memory::Buffer buffer = reader.read()) {
                writer(std::move(buffer));
            }
        } catch (const std::system_error& e) {
            if (e.code().value() != EPIPE) {
                throw;
            }
        }

        close(fd);
        writer.close();

        int status = 0;
        const int pid = ::wait(&status);
        if (pid < 0) {
            throw std::system_error{errno, std::system_category(), "Could not run pager: wait() call failed"};
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) { // NOLINT(hicpp-signed-bitwise)
            throw argument_error{std::string{"Could not run pager '"} + m_pager + "'"};
        }
#endif
    }

    reader.close();

    return true;
}

