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

#include "command_apply_changes.hpp"

#include "exception.hpp"
#include "util.hpp"

#include <osmium/index/id_set.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/io/reader_with_progress_bar.hpp>
#include <osmium/io/writer.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/object_comparisons.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <boost/function_output_iterator.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using location_index_type = osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location>;

bool CommandApplyChanges::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("change-file-format", po::value<std::string>(), "Format of the change file(s)")
    ("redact",            "Redact (patch) OSM files")
    ("with-history,H",    "Apply changes to history file")
    ("locations-on-ways", "Expect and update locations on ways")
    ;

    const po::options_description opts_common{add_common_options()};
    const po::options_description opts_input{add_single_input_options()};
    const po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("change-filenames", po::value<std::vector<std::string>>(), "OSM change input files")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("change-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("change-filenames")) {
        m_change_filenames = vm["change-filenames"].as<std::vector<std::string>>();
    } else {
        throw argument_error{"Need data file and at least one change file on the command line."};
    }

    if (vm.count("change-file-format")) {
        m_change_file_format = vm["change-file-format"].as<std::string>();
    }

    if (vm.count("locations-on-ways")) {
        m_locations_on_ways = true;
    }

    if (vm.count("with-history")) {
        if (m_locations_on_ways) {
            throw argument_error{"Can not use --with-history/-H and --locations-on-ways together."};
        }
        m_with_history = true;
        m_output_file.set_has_multiple_object_versions(true);
    } else {
        if (m_input_file.has_multiple_object_versions() && m_output_file.has_multiple_object_versions()) {
            if (m_locations_on_ways) {
                throw argument_error{"Can not use --locations-on-ways on history files."};
            }
            m_with_history = true;
        } else if (m_input_file.has_multiple_object_versions() != m_output_file.has_multiple_object_versions()) {
            throw argument_error{"Input and output file must both be OSM data files or both OSM history files (force with --with-history)."};
        }
    }

    if (vm.count("redact")) {
        if (m_locations_on_ways) {
            throw argument_error{"Can not use --redact and --locations-on-ways together."};
        }
        m_with_history = true;
        m_redact = true;
        m_output_file.set_has_multiple_object_versions(true);
    }

    return true;
}

void CommandApplyChanges::show_arguments() {
    m_vout << "  input data file name: " << m_input_filename << "\n";
    m_vout << "  input change file names: \n";
    for (const auto& fn : m_change_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  data file format: " << m_input_format << "\n";
    m_vout << "  change file format: " << m_change_file_format << "\n";
    show_output_arguments(m_vout);
    m_vout << "  reading and writing history file: " << yes_no(m_with_history);
    m_vout << "  locations on ways: " << yes_no(m_locations_on_ways);
}

namespace {

/**
 *  Copy the first OSM object with a given Id to the output. Keep
 *  track of the Id of each object to do this.
 *
 *  We are using this functor class instead of a simple lambda, because the
 *  lambda doesn't build on MSVC.
 */
class copy_first_with_id {

    osmium::io::Writer* writer;
    osmium::object_id_type id = 0;

public:

    explicit copy_first_with_id(osmium::io::Writer* w) :
        writer(w) {
    }

    void operator()(const osmium::OSMObject& obj) {
        if (obj.id() != id) {
            if (obj.visible()) {
                (*writer)(obj);
            }
            id = obj.id();
        }
    }

}; // class copy_first_with_id

void update_nodes_if_way(osmium::OSMObject* object, const location_index_type& location_index) {
    if (object->type() != osmium::item_type::way) {
        return;
    }

    for (auto& node_ref : static_cast<osmium::Way*>(object)->nodes()) {
        auto location = location_index.get_noexcept(node_ref.positive_ref());
        if (location) {
            node_ref.set_location(location);
        }
    }
}

} // anonymous namespace

void CommandApplyChanges::apply_changes_and_write(osmium::ObjectPointerCollection &objects,
                                                  const std::vector<osmium::memory::Buffer> &changes,
                                                  osmium::io::Reader &reader,
                                                  osmium::io::Writer &writer) {
    objects.unique(osmium::object_equal_type_id{});
    m_vout << "There are " << objects.size() << " unique objects in the change files\n";

    osmium::index::IdSetSmall<osmium::unsigned_object_id_type> node_ids;
    m_vout << "Creating node index...\n";
    for (const auto& buffer : changes) {
        for (const auto& way : buffer.select<osmium::Way>()) {
            for (const auto& nr : way.nodes()) {
                node_ids.set(nr.positive_ref());
            }
        }
    }
    node_ids.sort_unique();
    m_vout << "Node index has " << node_ids.size() << " entries\n";

    m_vout << "Creating location index...\n";
    location_index_type location_index;
    for (const auto& buffer : changes) {
        for (const auto& node : buffer.select<osmium::Node>()) {
            location_index.set(node.positive_id(), node.location());
        }
    }
    m_vout << "Location index has " << location_index.size() << " entries\n";

    m_vout << "Applying changes and writing them to output...\n";
    auto it = objects.begin();
    auto last_type = osmium::item_type::undefined;
    while (osmium::memory::Buffer buffer = reader.read()) {
        for (auto& object : buffer.select<osmium::OSMObject>()) {
            if (object.type() < last_type) {
                throw std::runtime_error{"Input data out of order. Need nodes, ways, relations in ID order."};
            }
            if (object.type() == osmium::item_type::node) {
                const auto& node = static_cast<osmium::Node&>(object);
                if (node_ids.get_binary_search(node.positive_id())) {
                    const auto location = location_index.get_noexcept(node.positive_id());
                    if (!location) {
                        location_index.set(node.positive_id(), node.location());
                    }
                }
            } else if (object.type() == osmium::item_type::way) {
                if (last_type == osmium::item_type::node) {
                    location_index.sort();
                    node_ids.clear();
                }
            }

            last_type = object.type();

            auto last_it = it;
            while (it != objects.end() && osmium::object_order_type_id_reverse_version{}(*it, object)) {
                if (it->visible()) {
                    update_nodes_if_way(&*it, location_index);
                    writer(*it);
                }
                last_it = it;
                ++it;
            }

            if (last_it == objects.end() || last_it->type() != object.type() || last_it->id() != object.id()) {
                update_nodes_if_way(&object, location_index);
                writer(object);
            }
        }
    }
    while (it != objects.end()) {
        if (it->visible()) {
            update_nodes_if_way(&*it, location_index);
            writer(*it);
        }
        ++it;
    }
}

bool CommandApplyChanges::run() {
    if (m_locations_on_ways) {
        m_output_file.set("locations_on_ways");
    }

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer{m_output_file, m_output_overwrite, m_fsync};

    std::vector<osmium::memory::Buffer> changes;
    osmium::ObjectPointerCollection objects;

    m_vout << "Reading change file contents...\n";

    for (const std::string& change_file_name : m_change_filenames) {
        if (change_file_name == "-" && m_change_file_format.empty()) {
            throw argument_error{"When reading the change file from STDIN you have to use\n"
                                 "the --change-file-format option to specify the file format."};
        }
        const osmium::io::File file{change_file_name, m_change_file_format};
        osmium::io::Reader reader{file, osmium::osm_entity_bits::object};
        while (osmium::memory::Buffer buffer = reader.read()) {
            osmium::apply(buffer, objects);
            changes.push_back(std::move(buffer));
        }
        reader.close();
    }

    m_vout << "Opening input file...\n";
    osmium::io::ReaderWithProgressBar reader{display_progress(), m_input_file, osmium::osm_entity_bits::object};

    osmium::io::Header header;
    setup_header(header);
    if (m_with_history) {
        header.set_has_multiple_object_versions(true);
    }
    writer.set_header(header);

    if (m_with_history) {
        // For history files this is a straightforward sort of the change
        // files followed by a merge with the input file.
        m_vout << "Sorting change data...\n";
        objects.sort(osmium::object_order_type_id_version());

        m_vout << "Applying changes and writing them to output...\n";
        const auto input = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader);
        auto out = osmium::io::make_output_iterator(writer);

        if (m_redact) {
            std::set_union(objects.begin(),
                           objects.end(),
                           input.begin(),
                           input.end(),
                           out,
                           osmium::object_order_type_id_version_without_timestamp());
        } else {
            std::set_union(objects.begin(),
                           objects.end(),
                           input.begin(),
                           input.end(),
                           out);
        }
    } else {
        // For normal data files we sort with the largest version of each
        // object first and then only copy this last version of any object
        // to the output.
        m_vout << "Sorting change data...\n";

        // This is needed for a special case: When change files have been
        // created from extracts it is possible that they contain objects
        // with the same type, id, version, and timestamp. In that case we
        // still want to get the last object available. So we have to make
        // sure it appears first in the objects vector before doing the
        // stable sort.
        std::reverse(objects.ptr_begin(), objects.ptr_end());
        objects.sort(osmium::object_order_type_id_reverse_version{});

        if (m_locations_on_ways) {
            apply_changes_and_write(objects, changes, reader, writer);
        } else {
            m_vout << "Applying changes and writing them to output...\n";
            const auto input = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader);
            auto output_it = boost::make_function_output_iterator(copy_first_with_id(&writer));

            std::set_union(objects.begin(),
                           objects.end(),
                           input.begin(),
                           input.end(),
                           output_it,
                           osmium::object_order_type_id_reverse_version());
        }
    }

    writer.close();
    reader.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

