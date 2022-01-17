#include <osmium/io/file.hpp>
#include <osmium/io/reader.hpp>
#include "../option_clean.hpp"

class ExtractFile : public osmium::io::File {
    using Base = osmium::io::File;
    using Base::Base;

public:
    const OptionClean m_clean;

    ExtractFile(const osmium::io::File& file, OptionClean m_clean) : Base(file), m_clean(m_clean) {}
};

class ExtractReader : public osmium::io::Reader {
    using Base = osmium::io::Reader;
    using Base::Base;

    OptionClean m_clean;

public:
    template <typename... TArgs>
    explicit ExtractReader(const ExtractFile& file, TArgs&&... args) : Base(file, std::forward<TArgs>(args)...), m_clean(file.m_clean) {}

    inline osmium::memory::Buffer read() {
        auto buffer = osmium::io::Reader::read();
        m_clean.apply_to(buffer);
        return buffer;
    }
};
