#include "export_format_spaten.hpp"

#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/string_util.hpp>

#include <protozero/pbf_builder.hpp>

#include <cassert>


enum {
    // spaten block size, should be benchmarked
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
    m_buffer.resize(m_buffer.size() + block_header_size);
}

void ExportFormatSpaten::start_feature(spaten_pbf::Geom gt) {
    m_spaten_feature.add_enum(spaten_pbf::Feature::optional_Geom_geomtype, static_cast<uint32_t>(gt));
    m_spaten_feature.add_enum(spaten_pbf::Feature::optional_GeomSerial_geomserial, static_cast<uint32_t>(spaten_pbf::GeomSerial::wkb));
}

void ExportFormatSpaten::node(const osmium::Node& node) {
    start_feature(spaten_pbf::Geom::gt_node);
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_point(node));
    finish_feature(node);
}

void ExportFormatSpaten::way(const osmium::Way& way) {
    start_feature(spaten_pbf::Geom::gt_line);
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_linestring(way));
    finish_feature(way);
}

void ExportFormatSpaten::area(const osmium::Area& area) {
    start_feature(spaten_pbf::Geom::gt_poly);
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_multipolygon(area));
    finish_feature(area);
}

void ExportFormatSpaten::finish_feature(const osmium::OSMObject& object) {
    if (write_tags(object, m_spaten_feature) || options().keep_untagged) {
        m_spaten_block_body.add_message(spaten_pbf::Body::repeated_Feature_feature, m_feature_buffer);
        if (m_buffer.size() > flush_buffer_size) {
            flush_to_output();
        }
    }
    m_feature_buffer.clear();
}

bool ExportFormatSpaten::write_tags(const osmium::OSMObject& object, protozero::pbf_builder<spaten_pbf::Feature>& proto_feat) {
    std::string tagbuf;
    bool has_tags = false;

    for (const auto& tag : object.tags()) {
        if (options().tags_filter(tag)) {
            has_tags = true;
            protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};
            ptag.add_string(spaten_pbf::Tag::optional_string_key, tag.key());
            ptag.add_string(spaten_pbf::Tag::optional_string_value, tag.value());
            ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, 0);

            proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);

            tagbuf.clear();
        }
    }
    return has_tags;
}

void ExportFormatSpaten::flush_to_output() {
    const uint32_t buffer_size = m_buffer.size() - block_header_size;

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
