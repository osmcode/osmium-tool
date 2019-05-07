#include "export_format_spaten.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/string_util.hpp>

#include <protozero/pbf_builder.hpp>

#include <cassert>

enum {
    initial_buffer_size = 15u * 1024u * 1024u
};

enum {
    flush_buffer_size = 15u * 900u * 1024u
};

enum {
    block_header_size = 8u
}; 

static const std::string version(4, '\0');
static const std::string flags(2, '\0');
static const std::string compression(1, '\0');
static const std::string message_type(1, '\0');

ExportFormatSpaten::ExportFormatSpaten(const std::string& /*output_format*/,
                               const std::string& output_filename,
                               osmium::io::overwrite overwrite,
                               osmium::io::fsync fsync,
                               const options_type& options) :
    ExportFormat(options),
    m_fd(osmium::io::detail::open_for_writing(output_filename, overwrite)),
    m_fsync(fsync) {
    write_file_header();
    reserve_block_header_space();
    m_buffer.reserve(initial_buffer_size);
}

void ExportFormatSpaten::write_file_header() {
    std::string fh{"SPAT"};
    fh.append(version);
    osmium::io::detail::reliable_write(m_fd, fh.data(), fh.size());
}

void ExportFormatSpaten::reserve_block_header_space() {
    m_buffer += std::string(block_header_size, '\0');
}

void ExportFormatSpaten::start_feature(Geom gt) {
    m_spaten_feature.add_enum(Feature::optional_Geom_geomtype, gt);
    m_spaten_feature.add_enum(Feature::optional_GeomSerial_geomserial, GeomSerial::wkb);
}

void ExportFormatSpaten::node(const osmium::Node& node) {
    start_feature(gt_node);
    m_spaten_feature.add_string(Feature::optional_string_geom, m_factory.create_point(node));
    write_tags(node, m_spaten_feature);
    finish_feature();
}

void ExportFormatSpaten::way(const osmium::Way& way) {
    start_feature(gt_line);
    m_spaten_feature.add_string(Feature::optional_string_geom, m_factory.create_linestring(way));
    write_tags(way, m_spaten_feature);
    finish_feature();
}

void ExportFormatSpaten::area(const osmium::Area& area) {
    start_feature(gt_poly);
    m_spaten_feature.add_string(Feature::optional_string_geom, m_factory.create_multipolygon(area));
    write_tags(area, m_spaten_feature);
    finish_feature();
}

void ExportFormatSpaten::finish_feature() {
    m_spaten_block_body.add_message(Body::repeated_Feature_feature, m_feature_buffer);
    m_feature_buffer.clear();
    if (m_buffer.size() > flush_buffer_size) {
        flush_to_output();
    }
}

void ExportFormatSpaten::write_tags(const osmium::OSMObject& object, protozero::pbf_builder<Feature>& proto_feat) {
    std::string tagbuf;
    for (const auto& tag : object.tags()) {
        if (options().tags_filter(tag)) {
            protozero::pbf_builder<Tag> ptag{tagbuf};
            ptag.add_string(Tag::optional_string_key, tag.key());
            ptag.add_string(Tag::optional_string_value, tag.value());
            ptag.add_enum(Tag::optional_ValueType_type, 0);

            proto_feat.add_message(Feature::optional_Tag_tags, tagbuf);

            tagbuf.clear();
        }
    }
}

void ExportFormatSpaten::flush_to_output() {
    uint32_t buffer_size = m_buffer.size() - block_header_size;

    std::string blockmeta;
    blockmeta.append(reinterpret_cast<const char*>(&buffer_size), sizeof(buffer_size));
    blockmeta.append(flags);
    blockmeta.append(compression);
    blockmeta.append(message_type);
    assert(blockmeta.size() == block_header_size);
    m_buffer.replace(0, blockmeta.size(), blockmeta);

    osmium::io::detail::reliable_write(m_fd, m_buffer.data(), m_buffer.size());
    m_buffer.clear();
    reserve_block_header_space();
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
