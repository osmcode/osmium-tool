# Setup common compiler options adviced for building Osmium headers

add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)

if(MSVC)
    add_definitions(-wd4996 -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-std=c++11)
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
        -Wredundant-decls
        -Wdisabled-optimization
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wsign-promo
        -Wold-style-cast
    )

    #always compile with optimization, even in debug mode
    set(CMAKE_CXX_FLAGS_DEBUG -O3)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
endif()

if(APPLE)
    add_compile_options(-stdlib=libc++)
    set(LDFLAGS ${LDFLAGS} -stdlib=libc++)
endif(APPLE)

message(STATUS "Some compiler and linker options were changed by OsmiumOptions")
