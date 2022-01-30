#ifndef EXPORT_EXPORT_FORMAT_SPATEN_HPP
#define EXPORT_EXPORT_FORMAT_SPATEN_HPP

#include "export_format.hpp"

#include <osmium/fwd.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/io/writer_options.hpp>

#include <protozero/pbf_builder.hpp>

#include <string>

namespace spaten_pbf {

    enum Geom : uint32_t {
        gt_node = 1,
        gt_line = 2,
        gt_poly = 3
    };

    enum GeomSerial : uint32_t {
        wkb = 0
    };

    enum TagValueType : uint32_t {
        string = 0,
        uint64 = 1,
        float64 = 2
    };

    enum class Body : protozero::pbf_tag_type {
        optional_Meta_meta = 1,
        repeated_Feature_feature = 2
    };

    enum class Feature : protozero::pbf_tag_type {
        optional_Geom_geomtype = 1,
        optional_GeomSerial_geomserial = 2,
        optional_string_geom = 3,

        optional_double_left = 4,
        optional_double_right = 5,
        optional_double_top = 6,
        optional_double_bottom = 7,

        optional_Tag_tags = 8
    };

    enum class Tag : protozero::pbf_tag_type {
        optional_string_key = 1,
        optional_string_value = 2,
        optional_ValueType_type = 3
    };

} // namespace spaten_pbf

class ExportFormatSpaten : public ExportFormat {

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::wkb, osmium::geom::out_type::binary};
    std::string m_buffer;
    std::string m_feature_buffer;
    protozero::pbf_builder<spaten_pbf::Body> m_spaten_block_body{m_buffer};
    protozero::pbf_builder<spaten_pbf::Feature> m_spaten_feature{m_feature_buffer};
    int m_fd;
    osmium::io::fsync m_fsync;

    void reserve_block_header_space();
    void flush_to_output();
    void write_file_header() const;
    bool write_tags(const osmium::OSMObject& object, protozero::pbf_builder<spaten_pbf::Feature>& proto_feat);
    void add_attributes(const osmium::OSMObject& object, protozero::pbf_builder<spaten_pbf::Feature>& proto_feat);
    void start_feature(spaten_pbf::Geom gt, osmium::object_id_type id);
    void finish_feature(const osmium::OSMObject& object);

public:

    ExportFormatSpaten(const std::string& output_format,
                       const std::string& output_filename,
                       osmium::io::overwrite overwrite,
                       osmium::io::fsync fsync,
                       const options_type& options);

    ~ExportFormatSpaten() override {
        try {
            close();
        } catch (...) {
        }
    }

    void node(const osmium::Node& node) override;

    void way(const osmium::Way& way) override;

    void area(const osmium::Area& area) override;

    void close() override;

}; // class ExportFormatSpaten

#endif // EXPORT_EXPORT_FORMAT_SPATEN_HPP
