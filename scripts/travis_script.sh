#!/bin/sh
#
#  travis_script.sh
#

set -e

mkdir build
cd build

# GCC ignores the pragmas in the code that disable the "return-type" warning
# selectively, so use this workaround.
if [ "${CXX}" = "g++" ]; then
    WORKAROUND="-DCMAKE_CXX_FLAGS=-Wno-return-type"
else
    WORKAROUND=""
fi

if [ "${CXX}" = "g++" ]; then
    CXX=g++-4.8
    CC=gcc-4.8
fi

echo "travis_fold:start:cmake\nRunning cmake..."
cmake -LA \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    ${WORKAROUND} \
    ..
echo "travis_fold:end:cmake"

echo "travis_fold:start:make\nRunning make..."
make VERBOSE=1
echo "travis_fold:end:make"

echo "travis_fold:start:ctest\nRunning ctest..."
ctest --output-on-failure
echo "travis_fold:end:ctest"

