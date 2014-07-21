
add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)

if(USE_CRYPTOPP)
    add_definitions(-DUSE_CRYPTOPP=${USE_CRYPTOPP})
endif()

add_compile_options(-O3)
add_compile_options(-std=c++11)
add_compile_options(-Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast)

if(APPLE)
    add_compile_options(-stdlib=libc++)
    set(LDFLAGS ${LDFLAGS} -stdlib=libc++)
endif(APPLE)

file(GLOB OSMIUM_SOURCE_FILES *.cpp)

add_executable(osmium ${OSMIUM_SOURCE_FILES})

target_link_libraries(osmium ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${EXPAT_LIBRARIES} ${PROTOBUF_LITE_LIBRARIES} ${Boost_LIBRARIES} ${CRYPTOPP_LIBRARIES} ${OSMPBF_LIBRARY})

install(TARGETS osmium DESTINATION bin)

