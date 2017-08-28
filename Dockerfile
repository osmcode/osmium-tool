FROM alpine:3.6

RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories

WORKDIR /usr/src/apps

RUN apk add --no-cache \
        boost-dev \
        bzip2-dev \
        cmake \
        expat-dev \
        g++ \
        gdal-dev \
        geos-dev \
        git \
        make \
        proj4-dev \
        sparsehash \
        zlib-dev && \
    git clone https://github.com/osmcode/libosmium.git && \
        cd libosmium && \
        mkdir build && \
        cd build && \
        cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_EXAMPLES=OFF .. && \
        make

COPY . /usr/src/apps/osmium

WORKDIR /usr/src/apps/osmium
RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=MinSizeRel .. && \
    make && \
    make install

RUN apk del git cmake make

ENTRYPOINT ["osmium"]

CMD ["--help"]