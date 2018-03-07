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

#include "command_merge.hpp"
#include "util.hpp"

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

#include <algorithm>
#include <memory>
#include <numeric>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace osmium {

    namespace io {
        class File;
    } // namespace io

} // namespace osmium

bool CommandMerge::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_multiple_inputs_options()};
    po::options_description opts_output{add_output_options()};

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

    setup_common(vm, desc);
    setup_progress(vm);
    setup_input_files(vm);
    setup_output_file(vm);

    return true;
}

void CommandMerge::show_arguments() {
    show_multiple_inputs_arguments(m_vout);
    show_output_arguments(m_vout);
}

namespace {

    class DataSource {

        using it_type = osmium::io::InputIterator<osmium::io::Reader, osmium::OSMObject>;

        std::unique_ptr<osmium::io::Reader> reader;
        it_type iterator;

    public:

        explicit DataSource(const osmium::io::File& file) :
            reader(new osmium::io::Reader{file}),
            iterator(*reader) {
        }

        bool empty() const noexcept {
            return iterator == it_type{};
        }

        bool next() noexcept {
            ++iterator;
            return iterator != it_type{};
        }

        const osmium::OSMObject* get() noexcept {
            return &*iterator;
        }

        std::size_t offset() const noexcept {
            return reader->offset();
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
    } else if (m_input_files.size() == 2) {
        // Use simpler code when there are exactly two files to merge
        m_vout << "Merging 2 input files to output file...\n";

        // The larger file should be first so the progress bar will work better
        if (osmium::util::file_size(m_input_files[0].filename()) <
            osmium::util::file_size(m_input_files[1].filename())) {
            using std::swap;
            swap(m_input_files[0], m_input_files[1]);
        }

        osmium::io::ReaderWithProgressBar reader1(display_progress(), m_input_files[0], osmium::osm_entity_bits::object);
        osmium::io::Reader reader2(m_input_files[1], osmium::osm_entity_bits::object);
        auto in1 = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader1);
        auto in2 = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader2);
        auto out = osmium::io::make_output_iterator(writer);

        std::set_union(in1.cbegin(), in1.cend(),
                       in2.cbegin(), in2.cend(),
                       out);
    } else {
        // Three or more files to merge
        m_vout << "Merging " << m_input_files.size() << " input files to output file...\n";
        osmium::ProgressBar progress_bar{file_size_sum(m_input_files), display_progress()};
        std::vector<DataSource> data_sources;
        data_sources.reserve(m_input_files.size());

        std::priority_queue<QueueElement> queue;

        int index = 0;
        for (const osmium::io::File& file : m_input_files) {
            data_sources.emplace_back(file);

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

            const int index = element.data_source_index();
            if (data_sources[index].next()) {
                queue.emplace(data_sources[index].get(), index);
            }

            if (n++ > 10000) {
                n = 0;
                progress_bar.update(std::accumulate(data_sources.cbegin(), data_sources.cend(), 0, [](std::size_t sum, const DataSource& source){
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

