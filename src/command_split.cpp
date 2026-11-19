#include "command_split.hpp"
#include "exception.hpp"
#include "util.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm.hpp>
#include <osmium/util/progress_bar.hpp>
#include <osmium/util/verbose_output.hpp>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

CommandSplit::CommandSplit(const CommandFactory& command_factory) :
    CommandWithSingleOSMInput(command_factory) {
}

bool CommandSplit::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
            ("prefix,p", po::value<std::string>(&m_output_prefix)->default_value("split"), "Output file prefix")
            ("output-format,f", po::value<std::string>(&m_output_format)->default_value("osm.pbf"), "Format of output files (e.g. osm.pbf, osm.bz2)")
            ("overwrite,O", po::bool_switch(&m_overwrite), "Allow existing output files to be overwritten")
            ("fsync", po::bool_switch(&m_use_fsync), "Call fsync after writing output files")
            ("split-details,d", po::bool_switch(&m_split_details), "Split into 6 files separating tags from geometry/references")
            ("skip-empty-tags,e", po::bool_switch(&m_skip_empty_tags), "Do not write objects without tags to *_tags files (only with --split-details)")
            ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};

    po::options_description hidden;
    hidden.add_options()
          ("input-filename", po::value<std::string>(&m_input_filename), "Input file")
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

    if (!setup_common(vm, desc)) {
        return false;
    }
    setup_progress(vm);
    setup_input_file(vm);

    return true;
}

void CommandSplit::show_arguments() {
    show_single_input_arguments(m_vout);
    m_vout << "  other options:\n";
    m_vout << "    output prefix: " << m_output_prefix << '\n';
    m_vout << "    output format: " << m_output_format << '\n';
    m_vout << "    overwrite: " << (m_overwrite ? "yes" : "no") << '\n';
    m_vout << "    fsync: " << (m_use_fsync ? "yes" : "no") << '\n';
    m_vout << "    split details (6 files): " << (m_split_details ? "yes" : "no") << '\n';
    m_vout << "    skip empty tags: " << (m_skip_empty_tags ? "yes" : "no") << '\n';
}

namespace {

    class BufferedWriter {
        static constexpr size_t initial_buffer_size = 10 * 1024 * 1024;
        static constexpr size_t flush_threshold = 8 * 1024 * 1024;

        osmium::io::Writer m_writer;
        osmium::memory::Buffer m_buffer;

    public:
        BufferedWriter(const osmium::io::File& file, const osmium::io::Header& header,
                       osmium::io::overwrite overwrite, osmium::io::fsync fsync) :
            m_writer(file, header, overwrite, fsync),
            m_buffer(initial_buffer_size, osmium::memory::Buffer::auto_grow::yes) {
        }

        osmium::memory::Buffer& buffer() noexcept {
            return m_buffer;
        }

        void commit_and_maybe_flush() {
            m_buffer.commit();
            if (m_buffer.committed() >= flush_threshold) {
                flush();
            }
        }

        void flush() {
            if (m_buffer.committed() > 0) {
                m_writer(std::move(m_buffer));
                m_buffer = osmium::memory::Buffer{initial_buffer_size, osmium::memory::Buffer::auto_grow::yes};
            }
        }

        void close() {
            flush();
            m_writer.close();
        }
    };

    template <typename TBuilder>
    void copy_attributes(TBuilder& builder, const osmium::OSMObject& obj) {
        builder.set_id(obj.id())
               .set_version(obj.version())
               .set_changeset(obj.changeset())
               .set_timestamp(obj.timestamp())
               .set_uid(obj.uid())
               .set_user(obj.user());
    }

    void copy_tags(osmium::memory::Buffer& buffer, osmium::builder::Builder& parent, const osmium::TagList& tags) {
        if (tags.empty()) {
            return;
        }
        osmium::builder::TagListBuilder tl_builder{buffer, &parent};
        for (const auto& tag : tags) {
            tl_builder.add_tag(tag.key(), tag.value());
        }
    }

} // anonymous namespace

bool CommandSplit::run() {
    m_vout << "Opening input file...\n";
    osmium::io::Reader reader{m_input_file};
    osmium::io::Header header = reader.header();
    header.set("generator", "osmium");

    const auto overwrite = m_overwrite ? osmium::io::overwrite::allow : osmium::io::overwrite::no;
    const auto fsync = m_use_fsync ? osmium::io::fsync::yes : osmium::io::fsync::no;

    const auto comma_pos = m_output_format.find(',');
    const std::string file_ext = m_output_format.substr(0, comma_pos);

    const auto make_file = [&](const std::string& suffix) {
        osmium::io::File file{m_output_prefix + "_" + suffix + "." + file_ext, m_output_format};
        file.check();
        return file;
    };

    osmium::ProgressBar progress{reader.file_size(), display_progress()};

    if (!m_split_details) {
        osmium::io::Writer writer_nodes{make_file("nodes"), header, overwrite, fsync};
        osmium::io::Writer writer_ways{make_file("ways"), header, overwrite, fsync};
        osmium::io::Writer writer_rels{make_file("relations"), header, overwrite, fsync};

        while (osmium::memory::Buffer buffer = reader.read()) {
            progress.update(reader.offset());
            for (const auto& entity : buffer) {
                switch (entity.type()) {
                    case osmium::item_type::node:
                        writer_nodes(static_cast<const osmium::Node&>(entity));
                        break;
                    case osmium::item_type::way:
                        writer_ways(static_cast<const osmium::Way&>(entity));
                        break;
                    case osmium::item_type::relation:
                        writer_rels(static_cast<const osmium::Relation&>(entity));
                        break;
                    default:
                        break;
                }
            }
        }
        writer_nodes.close();
        writer_ways.close();
        writer_rels.close();
    } else {
        BufferedWriter w_node_tags{make_file("nodes_tags"), header, overwrite, fsync};
        BufferedWriter w_node_geom{make_file("nodes_geom"), header, overwrite, fsync};
        BufferedWriter w_way_tags{make_file("ways_tags"), header, overwrite, fsync};
        BufferedWriter w_way_geom{make_file("ways_geom"), header, overwrite, fsync};
        BufferedWriter w_rel_tags{make_file("relations_tags"), header, overwrite, fsync};
        BufferedWriter w_rel_geom{make_file("relations_geom"), header, overwrite, fsync};

        while (osmium::memory::Buffer buffer = reader.read()) {
            progress.update(reader.offset());
            for (const auto& entity : buffer) {
                if (entity.type() == osmium::item_type::node) {
                    const auto& node = static_cast<const osmium::Node&>(entity);

                    if (!m_skip_empty_tags || !node.tags().empty()) {
                        {
                            osmium::builder::NodeBuilder builder{w_node_tags.buffer()};
                            copy_attributes(builder, node);
                            copy_tags(w_node_tags.buffer(), builder, node.tags());
                        }
                        w_node_tags.commit_and_maybe_flush();
                    }

                    {
                        osmium::builder::NodeBuilder builder{w_node_geom.buffer()};
                        copy_attributes(builder, node);
                        builder.set_location(node.location());
                    }
                    w_node_geom.commit_and_maybe_flush();

                } else if (entity.type() == osmium::item_type::way) {
                    const auto& way = static_cast<const osmium::Way&>(entity);

                    if (!m_skip_empty_tags || !way.tags().empty()) {
                        {
                            osmium::builder::WayBuilder builder{w_way_tags.buffer()};
                            copy_attributes(builder, way);
                            copy_tags(w_way_tags.buffer(), builder, way.tags());
                        }
                        w_way_tags.commit_and_maybe_flush();
                    }

                    {
                        osmium::builder::WayBuilder builder{w_way_geom.buffer()};
                        copy_attributes(builder, way);
                        if (!way.nodes().empty()) {
                            osmium::builder::WayNodeListBuilder wnl_builder{w_way_geom.buffer(), &builder};
                            for (const auto& nr : way.nodes()) {
                                wnl_builder.add_node_ref(nr);
                            }
                        }
                    }
                    w_way_geom.commit_and_maybe_flush();

                } else if (entity.type() == osmium::item_type::relation) {
                    const auto& rel = static_cast<const osmium::Relation&>(entity);

                    if (!m_skip_empty_tags || !rel.tags().empty()) {
                        {
                            osmium::builder::RelationBuilder builder{w_rel_tags.buffer()};
                            copy_attributes(builder, rel);
                            copy_tags(w_rel_tags.buffer(), builder, rel.tags());
                        }
                        w_rel_tags.commit_and_maybe_flush();
                    }

                    {
                        osmium::builder::RelationBuilder builder{w_rel_geom.buffer()};
                        copy_attributes(builder, rel);
                        if (!rel.members().empty()) {
                            osmium::builder::RelationMemberListBuilder rml_builder{w_rel_geom.buffer(), &builder};
                            for (const auto& member : rel.members()) {
                                rml_builder.add_member(member.type(), member.ref(), member.role());
                            }
                        }
                    }
                    w_rel_geom.commit_and_maybe_flush();
                }
            }
        }

        w_node_tags.close();
        w_node_geom.close();
        w_way_tags.close();
        w_way_geom.close();
        w_rel_tags.close();
        w_rel_geom.close();
    }

    progress.done();
    reader.close();
    m_vout << "Done.\n";

    return true;
}
