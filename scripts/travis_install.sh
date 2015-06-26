#!/bin/sh
#
#  travis_install.sh
#

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    brew install protobuf osm-pbf || true
fi

cd ..
git clone --quiet --depth 1 https://github.com/osmcode/libosmium.git
git clone --quiet --depth 1 https://github.com/scrosby/OSM-binary.git
cd OSM-binary/src
make

