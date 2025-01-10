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

#include "command_tags_count.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/io/reader.hpp>
#include <osmium/osm.hpp>
#include <osmium/util/progress_bar.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

void CommandTagsCount::add_matcher(const std::string& expression) {
    bool has_value_matcher = false;
    auto matcher = get_tag_matcher(expression, &has_value_matcher);
    if (has_value_matcher) {
        m_tags_filter.add_rule(true, std::move(matcher));
    } else {
        m_keys_filter.add_rule(true, std::move(matcher));
    }
}

void CommandTagsCount::read_expressions_file(const std::string& file_name) {
    m_vout << "Reading expressions file...\n";

    std::ifstream file{file_name};
    if (!file.is_open()) {
        throw argument_error{"Could not open file '" + file_name + "'"};
    }

    for (std::string line; std::getline(file, line);) {
        const auto pos = line.find_first_of('#');
        if (pos != std::string::npos) {
            line.erase(pos);
        }
        if (!line.empty()) {
            if (line.back() == '\r') {
                line.resize(line.size() - 1);
            }
            add_matcher(line);
        }
    }
}

namespace {

sort_func_type get_sort_function(const std::string& sort_order) {
    static const std::pair<std::string, sort_func_type> sort_options[] = {
        { "count-desc", [](const element_type& a, const element_type& b){
                if (a.count == b.count) {
                    return *a.name < *b.name;
                }
                return a.count > b.count;
            }
        },
        { "count-asc", [](const element_type& a, const element_type& b){
                if (a.count == b.count) {
                    return *a.name < *b.name;
                }
                return a.count < b.count;
            }
        },
        { "name-desc", [](const element_type& a, const element_type& b){
                return *a.name > *b.name;
            }
        },
        { "name-asc", [](const element_type& a, const element_type& b){
                return *a.name < *b.name;
            }
        }
    };

    for (const auto& nf : sort_options) {
        if (nf.first == sort_order) {
            return nf.second;
        }
    }

    throw argument_error{"Unknown sort order '" + sort_order + "'"};
}

} // anonymous namespace

bool CommandTagsCount::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("expressions,e", po::value<std::string>(), "Read tag expressions from file")
    ("min-count,m", po::value<counter_type>(), "Min count shown (default: 0)")
    ("max-count,M", po::value<counter_type>(), "Max count shown (default: none)")
    ("output,o", po::value<std::string>(), "Output file (default: stdout)")
    ("overwrite,O", "Allow existing output file to be overwritten")
    ("sort,s", po::value<std::string>(), "Sort order of results ('count-asc', 'count-desc' (default), 'name-asc', or 'name-desc')")
    ("object-type,t", po::value<std::vector<std::string>>(), "Read only objects of given type (node, way, relation)")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("expression-list", po::value<std::vector<std::string>>(), "Count expressions")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("expression-list", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_object_type_nwr(vm);
    setup_input_file(vm);

    if (vm.count("output")) {
        m_output_filename = vm["output"].as<std::string>();
    }

    if (vm.count("overwrite")) {
        m_output_overwrite = osmium::io::overwrite::allow;
    }

    if (vm.count("expression-list")) {
        for (const auto& e : vm["expression-list"].as<std::vector<std::string>>()) {
            add_matcher(e);
        }
    } else {
        m_keys_filter = osmium::TagsFilter{true};
    }

    if (vm.count("expressions")) {
        read_expressions_file(vm["expressions"].as<std::string>());
    }

    if (vm.count("min-count")) {
        m_min_count = vm["min-count"].as<counter_type>();
    }

    if (vm.count("max-count")) {
        m_max_count = vm["max-count"].as<counter_type>();
    }

    if (vm.count("sort")) {
        m_sort_order = vm["sort"].as<std::string>();
        if (m_sort_order == "name") {
            m_sort_order = "name-asc";
        } else if (m_sort_order == "count") {
            m_sort_order = "count-desc";
        }
    }

    m_sort_func = get_sort_function(m_sort_order);

    return true;
}

void CommandTagsCount::show_arguments() {
    show_single_input_arguments(m_vout);

    m_vout << "  output options:\n";
    m_vout << "    file name: " << m_output_filename << '\n';
    m_vout << "    overwrite: " << yes_no(m_output_overwrite == osmium::io::overwrite::allow);
    m_vout << "  other options:\n";
    m_vout << "    sort order: " << m_sort_order << '\n';
    m_vout << "    min count: " << m_min_count << '\n';

    if (m_max_count == std::numeric_limits<counter_type>::max()) {
        m_vout << "    max count: (none)\n";
    } else {
        m_vout << "    max count: " << m_max_count << '\n';
    }
}

std::vector<element_type> CommandTagsCount::sort_results() const {
    std::vector<element_type> results;

    results.reserve(m_counts.size());
    for (const auto& c : m_counts) {
        if (c.second >= m_min_count && c.second <= m_max_count) {
            results.emplace_back(c.first, c.second);
        }
    }

    std::sort(results.begin(), results.end(), m_sort_func);

    return results;
}

namespace {

void append_escaped(std::string* out, const char* str) {
    *out += '"';
    for (; *str != '\0'; ++str) {
        if (*str == '"') {
            *out += '"';
        }
        *out += *str;
    }
    *out += '"';
}

void write_results(const std::vector<element_type>& results, int fd) {
    std::string out;
    const std::size_t buffer_size = 1024UL * 1024UL;
    out.reserve(buffer_size);

    for (const auto& c : results) {
        out += std::to_string(c.count);
        out += '\t';
        append_escaped(&out, c.name->key());
        const char* value = c.name->value();
        if (value) {
            out += '\t';
            append_escaped(&out, value);
        }
        out += '\n';
        if (out.size() > (buffer_size - 1000)) {
            osmium::io::detail::reliable_write(fd, out.data(), out.size());
            out.clear();
        }
    }

    osmium::io::detail::reliable_write(fd, out.data(), out.size());

    close(fd);
}

} // anonymous namespace

bool CommandTagsCount::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file, osm_entity_bits(), osmium::io::read_meta::no};

    m_vout << "Opening output file...\n";
    int fd = 1;
    if (!m_output_filename.empty()) {
        fd = osmium::io::detail::open_for_writing(m_output_filename,
                                                  m_output_overwrite);
    }

    m_vout << "Count matching keys/tags...\n";
    osmium::ProgressBar progress_bar{reader.file_size(), display_progress()};
    while (osmium::memory::Buffer buffer = reader.read()) {
        progress_bar.update(reader.offset());
        for (const auto& object : buffer.select<osmium::OSMObject>()) {
            for (const auto& tag : object.tags()) {
                if (m_keys_filter(tag)) {
                    ++m_counts[tag.key()];
                }
                if (m_tags_filter(tag)) {
                    ++m_counts[tag];
                }
            }
        }
    }
    progress_bar.done();

    m_vout << "Closing input file...\n";
    reader.close();

    show_memory_used();

    m_vout << "Sorting results...\n";
    const auto results = sort_results();

    show_memory_used();

    m_vout << "Writing results...\n";
    write_results(results, fd);

    m_vout << "Done.\n";

    return true;
}

