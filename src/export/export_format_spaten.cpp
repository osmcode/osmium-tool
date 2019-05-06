#include "export_format_spaten.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/string_util.hpp>

#include <protozero/pbf_writer.hpp>

enum {
    initial_buffer_size = 15u * 1024u * 1024u
};

enum {
    flush_buffer_size = 15u * 900u * 1024u
};

const std::string version = std::string(4, '\0');
const std::string flags = std::string(2, '\0');
const std::string compression = std::string(1, '\0');
const std::string message_type = std::string(1, '\0');

ExportFormatSpaten::ExportFormatSpaten(const std::string& /*output_format*/,
                               const std::string& output_filename,
                               osmium::io::overwrite overwrite,
                               osmium::io::fsync fsync,
                               const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync) {
    write_file_header();
    m_buffer.reserve(initial_buffer_size);
}

void ExportFormatSpaten::write_file_header() {
    std::string fh;
    fh.reserve(8);
    fh.append("SPAT");
    fh.append(version);
    osmium::io::detail::reliable_write(m_fd, fh.data(), fh.size());
}

void ExportFormatSpaten::start_feature(Geom gt) {
    feature_msg.add_enum(1, gt);
    feature_msg.add_enum(2, 0);
}

void ExportFormatSpaten::node(const osmium::Node& node) {
    start_feature(gt_node);
    feature_msg.add_string(3, m_factory.create_point(node));
    write_tags(node, feature_msg);
    finish_feature();
}

void ExportFormatSpaten::way(const osmium::Way& way) {
    start_feature(gt_line);
    feature_msg.add_string(3, m_factory.create_linestring(way));
    write_tags(way, feature_msg);
    finish_feature();
}

void ExportFormatSpaten::area(const osmium::Area& area) {
    start_feature(gt_poly);
    feature_msg.add_string(3, m_factory.create_multipolygon(area));
    write_tags(area, feature_msg);
    finish_feature();
}

void ExportFormatSpaten::finish_feature() {
    block.add_message(2, ft_buf);
    ft_buf.clear();
    if (m_buffer.size() > flush_buffer_size) {
        flush_to_output();
    }
}

void ExportFormatSpaten::write_tags(const osmium::OSMObject& object, protozero::pbf_writer& proto_feat) {
    std::string tagbuf;
    for (const auto& tag : object.tags()) {
        if (options().tags_filter(tag)) {
            protozero::pbf_writer ptag{tagbuf};
            ptag.add_string(1, tag.key());
            ptag.add_string(2, tag.value());
            ptag.add_enum(3, 0);

            proto_feat.add_message(8, tagbuf);

            tagbuf.clear();
        }
    }
}

void ExportFormatSpaten::flush_to_output() {
    std::string blockmeta;
    uint buffer_size = m_buffer.size();
    blockmeta.reserve(8);

    blockmeta.append(reinterpret_cast<const char*>(&buffer_size), 4);
    blockmeta.append(flags);
    blockmeta.append(compression);
    blockmeta.append(message_type);

    osmium::io::detail::reliable_write(m_fd, blockmeta.data(), blockmeta.size());
    osmium::io::detail::reliable_write(m_fd, m_buffer.data(), buffer_size);
    m_buffer.clear();
}

void ExportFormatSpaten::close() {
    if (m_fd > 0) {
        flush_to_output();
        if (m_fsync == osmium::io::fsync::yes) {
            osmium::io::detail::reliable_fsync(m_fd);
        }
        ::close(m_fd);
        m_fd = -1;
    }
}
