
#include "../util.hpp"
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

static const std::string unique_id_field{"@fid"};

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

void ExportFormatSpaten::start_feature(spaten_pbf::Geom gt, osmium::object_id_type id) {
    m_spaten_feature.add_enum(spaten_pbf::Feature::optional_Geom_geomtype, gt);
    m_spaten_feature.add_enum(spaten_pbf::Feature::optional_GeomSerial_geomserial, spaten_pbf::GeomSerial::wkb);

    if (options().unique_id == unique_id_type::counter) {
        std::string tagbuf;
        protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};

        ptag.add_string(spaten_pbf::Tag::optional_string_key, unique_id_field);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(m_count));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        m_spaten_feature.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
    } else if (options().unique_id == unique_id_type::type_id) {
        std::string tagbuf;
        protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};

        char prefix = '\0';
        if (gt == spaten_pbf::Geom::gt_node) {
            prefix = 'n';
        } else if (gt == spaten_pbf::Geom::gt_line) {
            prefix = 'w';
        } else if (gt == spaten_pbf::Geom::gt_poly) {
            prefix = 'a';
        }

        ptag.add_string(spaten_pbf::Tag::optional_string_key, unique_id_field);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, prefix + std::to_string(id));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::string);
        m_spaten_feature.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
    }
}

void ExportFormatSpaten::node(const osmium::Node& node) {
    start_feature(spaten_pbf::Geom::gt_node, node.id());
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_point(node));
    finish_feature(node);
}

void ExportFormatSpaten::way(const osmium::Way& way) {
    start_feature(spaten_pbf::Geom::gt_line, way.id());
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_linestring(way));
    finish_feature(way);
}

void ExportFormatSpaten::area(const osmium::Area& area) {
    start_feature(spaten_pbf::Geom::gt_poly, area.id());
    m_spaten_feature.add_string(spaten_pbf::Feature::optional_string_geom, m_factory.create_multipolygon(area));
    finish_feature(area);
}

void ExportFormatSpaten::finish_feature(const osmium::OSMObject& object) {
    if (write_tags(object, m_spaten_feature) || options().keep_untagged) {
        m_spaten_block_body.add_message(spaten_pbf::Body::repeated_Feature_feature, m_feature_buffer);
        if (m_buffer.size() > flush_buffer_size) {
            flush_to_output();
        }
        ++m_count;
    }
    m_feature_buffer.clear();
}

void ExportFormatSpaten::add_attributes(const osmium::OSMObject& object, protozero::pbf_builder<spaten_pbf::Feature>& proto_feat) {
    std::string tagbuf;
    protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};

    if (!options().type.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().type);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, object_type_as_string(object));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::string);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().id.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().id);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(object.type() == osmium::item_type::area ? osmium::area_id_to_object_id(object.id()) : object.id()));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().version.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().version);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(object.version()));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().changeset.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().changeset);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(object.changeset()));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().uid.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().uid);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(object.uid()));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().user.empty()) {
        protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().user);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, object.user());
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().timestamp.empty()) {
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().timestamp);
        ptag.add_string(spaten_pbf::Tag::optional_string_value, uint64_buf(object.timestamp().seconds_since_epoch()));
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::uint64);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }

    if (!options().way_nodes.empty() && object.type() == osmium::item_type::way) {
        std::string ways;
        ptag.add_string(spaten_pbf::Tag::optional_string_key, options().way_nodes);
        for (const auto& nr : static_cast<const osmium::Way&>(object).nodes()) {
            ways += std::to_string(nr.ref()) + " ";
        }
        ptag.add_string(spaten_pbf::Tag::optional_string_value, ways.substr(0, ways.size()-1));
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    }
}

std::string ExportFormatSpaten::uint64_buf(uint64_t v)  {
    std::string buf;
    buf += static_cast<char>((v       ) & 0xffu);
    buf += static_cast<char>((v >>  8u) & 0xffu);
    buf += static_cast<char>((v >> 16u) & 0xffu);
    buf += static_cast<char>((v >> 24u) & 0xffu);
    buf += static_cast<char>((v >> 32u) & 0xffu);
    buf += static_cast<char>((v >> 40u) & 0xffu);
    buf += static_cast<char>((v >> 48u) & 0xffu);
    buf += static_cast<char>((v >> 56u) & 0xffu);
    return buf;
}

bool ExportFormatSpaten::write_tags(const osmium::OSMObject& object, protozero::pbf_builder<spaten_pbf::Feature>& proto_feat) {
    add_attributes(object, proto_feat);

    std::string tagbuf;
    const bool has_tags = add_tags(object, [&](const osmium::Tag& tag) {
        if (tag.key() == unique_id_field && options().unique_id != unique_id_type::none) {
            return;
        }

        protozero::pbf_builder<spaten_pbf::Tag> ptag{tagbuf};
        ptag.add_string(spaten_pbf::Tag::optional_string_key, tag.key());
        ptag.add_string(spaten_pbf::Tag::optional_string_value, tag.value());
        ptag.add_enum(spaten_pbf::Tag::optional_ValueType_type, spaten_pbf::TagValueType::string);
        proto_feat.add_message(spaten_pbf::Feature::optional_Tag_tags, tagbuf);
        tagbuf.clear();
    });

    return has_tags;
}

void ExportFormatSpaten::flush_to_output() {
    const uint32_t buffer_size = m_buffer.size() - block_header_size;

    std::string blockmeta;
    blockmeta += static_cast<char>((buffer_size       ) & 0xffu);
    blockmeta += static_cast<char>((buffer_size >>  8u) & 0xffu);
    blockmeta += static_cast<char>((buffer_size >> 16u) & 0xffu);
    blockmeta += static_cast<char>((buffer_size >> 24u) & 0xffu);
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

