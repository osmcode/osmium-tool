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

#include "command_merge.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <string>
#include <utility>
#include <vector>

bool CommandMerge::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("with-history,H", "Do not warn about input files with multiple object versions")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_multiple_inputs_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filenames", po::value<std::vector<std::string>>(), "Input files")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    if (vm.count("with-history")) {
        m_with_history = true;
    }

    return true;
}

void CommandMerge::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);
}

namespace {

    class DataSource {

        using it_type = osmium::io::InputIterator<osmium::io::Reader, osmium::OSMObject>;

        std::unique_ptr<osmium::io::Reader> m_reader;
        std::string m_name;
        it_type m_iterator;

        osmium::item_type m_last_type = osmium::item_type::node;
        osmium::object_id_type m_last_id = 0;
        osmium::object_version_type m_last_version = 0;

        bool m_warning;

    public:

        explicit DataSource(const osmium::io::File& file, bool with_history) :
            m_reader(std::make_unique<osmium::io::Reader>(file)),
            m_name(file.filename()),
            m_iterator(*m_reader),
            m_warning(!with_history) {
            if (m_iterator != it_type{}) {
                m_last_type = m_iterator->type();
                m_last_id = m_iterator->id();
                m_last_version = m_iterator->version();
            }
        }

        bool empty() const noexcept {
            return m_iterator == it_type{};
        }

        bool next() {
            ++m_iterator;

            if (m_iterator == it_type{}) { // reached end of file
                return false;
            }

            if (m_iterator->type() < m_last_type) {
                throw std::runtime_error{"Objects in input file '" + m_name + "' out of order (must be nodes, then ways, then relations)."};
            }
            if (m_iterator->type() > m_last_type) {
                m_last_type = m_iterator->type();
                m_last_id = m_iterator->id();
                m_last_version = m_iterator->version();
                return true;
            }

            if (m_iterator->id() < m_last_id) {
                throw std::runtime_error{"Objects in input file '" + m_name + "' out of order (smaller ids must come first)."};
            }
            if (m_iterator->id() > m_last_id) {
                m_last_id = m_iterator->id();
                m_last_version = m_iterator->version();
                return true;
            }

            if (m_iterator->version() < m_last_version) {
                throw std::runtime_error{"Objects in input file '" + m_name + "' out of order (smaller version must come first)."};
            }
            if (m_iterator->version() == m_last_version) {
                throw std::runtime_error{"Two objects in input file '" + m_name + "' with same version."};
            }

            if (m_warning) {
                std::cerr << "Warning: Multiple objects with same id in input file '" + m_name + "'!\n";
                std::cerr << "If you are reading history files, this is to be expected. Use --with-history to disable warning.\n";
                m_warning = false;
            }

            m_last_version = m_iterator->version();

            return true;
        }

        const osmium::OSMObject* get() noexcept {
            return &*m_iterator;
        }

        std::size_t offset() const noexcept {
            return m_reader->offset();
        }

    }; // DataSource

    class QueueElement {

        const osmium::OSMObject* m_object;
        int m_data_source_index;

    public:

        QueueElement(const osmium::OSMObject* object, int data_source_index) noexcept :
            m_object(object),
            m_data_source_index(data_source_index) {
        }

        const osmium::OSMObject& object() const noexcept {
            return *m_object;
        }

        int data_source_index() const noexcept {
            return m_data_source_index;
        }

    }; // QueueElement

    bool operator<(const QueueElement& lhs, const QueueElement& rhs) noexcept {
        return lhs.object() > rhs.object();
    }

    bool operator==(const QueueElement& lhs, const QueueElement& rhs) noexcept {
        return lhs.object() == rhs.object();
    }

    bool operator!=(const QueueElement& lhs, const QueueElement& rhs) noexcept {
        return !(lhs == rhs);
    }

} // anonymous namespace

bool CommandMerge::run() {
    m_vout << "Opening output file...\n";
    osmium::io::Header header;
    setup_header(header);

    osmium::io::Writer writer{m_output_file, header, m_output_overwrite, m_fsync};

    if (m_input_files.size() == 1) {
        m_vout << "Single input file. Copying to output file...\n";
        osmium::io::ReaderWithProgressBar reader{display_progress(), m_input_files[0]};
        while (osmium::memory::Buffer buffer = reader.read()) {
            writer(std::move(buffer));
        }
    } else {
        m_vout << "Merging " << m_input_files.size() << " input files to output file...\n";
        osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
        std::vector<DataSource> data_sources;
        data_sources.reserve(m_input_files.size());

        std::priority_queue<QueueElement> queue;

        int index = 0;
        for (const osmium::io::File& file : m_input_files) {
            data_sources.emplace_back(file, m_with_history);

            if (!data_sources.back().empty()) {
                queue.emplace(data_sources.back().get(), index);
            }

            ++index;
        }

        int n = 0;
        while (!queue.empty()) {
            const auto element = queue.top();
            queue.pop();
            if (queue.empty() || element != queue.top()) {
                writer(element.object());
            }

            const int dsindex = element.data_source_index();
            if (data_sources[dsindex].next()) {
                queue.emplace(data_sources[dsindex].get(), dsindex);
            }

            if (n++ > 10000) {
                n = 0;
                progress_bar.update(std::accumulate(data_sources.cbegin(), data_sources.cend(), static_cast<std::size_t>(0), [](std::size_t sum, const DataSource& source){
                    return sum + source.offset();
                }));
            }
        }
    }

    m_vout << "Closing output file...\n";
    writer.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

