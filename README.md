
# Osmium Command Line Tool

Command line tool for working with OpenStreetMap data based on the Osmium
library.

[![Build Status](https://secure.travis-ci.org/osmcode/osmium-tool.png)](http://travis-ci.org/osmcode/osmium-tool)
[![Build Status](https://ci.appveyor.com/api/projects/status/ttem4j2gxa64p3w8?svg=true)](https://ci.appveyor.com/project/Mapbox/osmium-tool)

## Prerequisites

You need a C++11 compliant compiler. GCC 4.8 and later as well as clang 3.4 and
later are known to work. You also need the following libraries:

    Osmium Library
        Need at least version 2.5.1
        http://osmcode.org/libosmium

    boost-program-options (for parsing command line options)
        http://www.boost.org/doc/libs/1_54_0/doc/html/program_options.html
        Debian/Ubuntu: libboost-program-options-dev

    boost-crc
        http://www.boost.org/doc/libs/1_57_0/libs/crc/
        Debian/Ubuntu: libboost-dev

    zlib (for PBF support)
        http://www.zlib.net/
        Debian/Ubuntu: zlib1g-dev
        openSUSE: zlib-devel

    Expat (for parsing XML files)
        http://expat.sourceforge.net/
        Debian/Ubuntu: libexpat1-dev
        openSUSE: libexpat-devel

    cmake (for building)
        http://www.cmake.org/
        Debian/Ubuntu: cmake

    Pandoc (optional, to build documentation)
        http://johnmacfarlane.net/pandoc/
        Debian/Ubuntu: pandoc


## Building

Osmium uses CMake for its builds. For Unix/Linux systems a simple Makefile
wrapper is provided to make the build even easier. Just type `make` to compile.
Results will be in the `build` directory.

Or you can go the long route explicitly calling CMake as follows:

    mkdir build
    cd build
    cmake ..
    make

To set the build type call cmake with `-DCMAKE_BUILD_TYPE=type`. Possible
values are empty, Debug, Release, RelWithDebInfo, MinSizeRel, and Dev. The
defaults is RelWithDebInfo.


## Documentation

See the [manual](doc/manual.md).

There are man pages in the 'man' directory. To build them you need 'pandoc'.
If the `pandoc` command was found during the CMake config step, the manpages
will be built, if not they will not be built.


## Tests

Call `ctest` in the build directory to run the tests after build.

More extensive tests of the libosmium I/O system can also be run. See
`test/io/Makefile.in` for instructions.


## License

Copyright (C) 2013-2016  Jochen Topf <jochen@topf.org>

This program is available under the GNU GENERAL PUBLIC LICENSE Version 3.
See the file LICENSE.txt for the complete text of the license.


## Authors

This program was written and is maintained by Jochen Topf <jochen@topf.org>.

