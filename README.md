
# OSMIUM Command Line Tool

Command line tool for working with OpenStreetMap data based on the Osmium
library.


## Prerequisites

You need a C++11 compliant compiler. GCC 4.8 and clang 3.2 are known to work.
You also need the following libraries:

    Osmium Library
        http://osmcode.org/libosmium

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
        (You need at least version 1.3.2,
         install from GIT if the package is too old.)

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

    cmake (for building)
        http://www.cmake.org/
        Debian/Ubuntu: cmake


## Building

Osmium uses CMake for its builds. On Unix/Linux systems a simple Makefile
wrapper is provided. Just type `make` to compile. Results will be in the
`build` directory.

Or you can go the long route explicitly calling CMake as follows:

```
mkdir build
cd build
cmake ..
make
```

To set the build type call cmake with `-DCMAKE_BUILD_TYPE=type`. Possible
values are empty, Debug, Release, RelWithDebInfo and MinSizeRel.


## Documentation

There are man pages in the 'doc' directory. To build them you need 'pandoc'.
If the `pandoc` command was found during the CMake config step, the manpages
will be built, if not they will not be built.


## Building Debian Package

A `debian` directory is provided for building (unofficial) Debian packages.
Call `make deb` or `debuild -I -us -uc` to build the package. Note that there
currently is no libosmium package with the new libosmium version needed for the
Osmium tool, so the build dependencies are not complete.


## License

This program is available under the GNU GENERAL PUBLIC LICENSE Version 3. See
the file LICENSE.txt for the complete text of the license.


## Authors

This program was written and is maintained by Jochen Topf <jochen@topf.org>.

