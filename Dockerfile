FROM debian:stretch

RUN apt-get update && apt-get install -y \
    g++ \
    libprotozero-dev \
    libutfcpp-dev \
    rapidjson-dev \
    libboost-program-options-dev \
    libboost-dev \
    libbz2-dev \
    zlib1g-dev \
    libexpat1-dev \
    cmake \
    pandoc \
    curl && \
    rm -rf /var/lib/apt/lists/*

# cmake segfaults when run in /
# so we have to run it in a subdirectory
# https://gitlab.kitware.com/cmake/cmake/issues/16603
WORKDIR /osmium

COPY . .

# we need a newer libosmium than the one included in the distro
RUN mkdir -p libosmium && \
    curl -sSL https://github.com/osmcode/libosmium/archive/v2.12.2.tar.gz | \
    tar -v -xz -C libosmium --strip-components=1

RUN mkdir build && \
    cd build && \
    cmake -DOSMIUM_INCLUDE_DIR=../libosmium/include .. -DCMAKE_BUILD_TYPE=Release && \
    make

ENTRYPOINT ["build/osmium"]
CMD [ "help" ]
