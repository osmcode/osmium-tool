# Setup common compiler options adviced for building Osmium headers

if(NOT MSVC)
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

message(STATUS "Some compiler and linker options were changed by OsmiumOptions")
