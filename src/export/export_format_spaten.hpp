#ifndef EXPORT_EXPORT_FORMAT_SPATEN_HPP
#define EXPORT_EXPORT_FORMAT_SPATEN_HPP

#include "export_format.hpp"

#include <osmium/fwd.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/io/writer_options.hpp>

#include <protozero/pbf_writer.hpp>

#include <string>

enum Geom {
    gt_node = 1,
    gt_line = 2,
    gt_poly = 3
};

class ExportFormatSpaten : public ExportFormat {

    osmium::geom::WKBFactory<> m_factory{osmium::geom::wkb_type::wkb, osmium::geom::out_type::binary};
    std::string m_buffer;
    int m_fd;
    osmium::io::fsync m_fsync;
    protozero::pbf_writer block{m_buffer};
    std::string ft_buf;
    protozero::pbf_writer feature_msg{ft_buf};

    void flush_to_output();
    void write_file_header();
    void write_tags(const osmium::OSMObject& object, protozero::pbf_writer& proto_feat);
    void start_feature(Geom gt);
    void finish_feature();

public:

    ExportFormatSpaten(const std::string& output_format,
                   const std::string& output_filename,
                   osmium::io::overwrite overwrite,
                   osmium::io::fsync fsync,
                   const options_type& options);

    ~ExportFormatSpaten() override {
        try {
            close();
        } catch(...) {
        }
    }

    void node(const osmium::Node& node) override;

    void way(const osmium::Way& way) override;

    void area(const osmium::Area& area) override;

    void close() override;

}; // class ExportFormatPg   

#endif // EXPORT_EXPORT_FORMAT_SPATEN_HPP
