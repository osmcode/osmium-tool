
# OSMIUM Command Line Tool

Command line tool for working with OpenStreetMap data based on the Osmium library.


## Prerequisites

You need a C++11 compliant compiler. GCC 4.7.3 and clang 3.2 are known to work.

    Osmium Library
        http://osmcode.org/osmium

    boost-program-options (for parsing command line options)
        http://www.boost.org/doc/libs/1_54_0/doc/html/program_options.html
        Debian/Ubuntu: libboost-program-options-dev

    Google protocol buffers (for PBF support)
        http://code.google.com/p/protobuf/ (at least version 2.3.0 needed)
        Debian/Ubuntu: libprotobuf-dev protobuf-compiler
        openSUSE: protobuf-devel
        Also see http://wiki.openstreetmap.org/wiki/PBF_Format

    OSMPBF (for PBF support)
        https://github.com/scrosby/OSM-binary
        Debian/Ubuntu: libosmpbf-dev

    zlib (for PBF support)
        http://www.zlib.net/
        Debian/Ubuntu: zlib1g-dev
        openSUSE: zlib-devel

    Expat (for parsing XML files)
        http://expat.sourceforge.net/
        Debian/Ubuntu: libexpat1-dev
        openSUSE: libexpat-devel

    libcrypto++ (for checksumming)
        http://www.cryptopp.com/
        Debian/Ubuntu: libcrypto++-dev


## Building

Type 'make'.


## Documentation

There are man pages in the 'doc' directory. To build them you need 'pandoc'.
Run 'make doc' to build.


## License

This program is available under the GNU GENERAL PUBLIC LICENSE Version 3. See
the file LICENSE.txt for the complete text of the license.


## Authors

This program was written and is maintained by Jochen Topf <jochen@topf.org>.

