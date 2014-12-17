# Setup common compiler options adviced for building Osmium headers

if(NOT MSVC)
    #always compile with optimization, even in debug mode
    set(CMAKE_CXX_FLAGS_DEBUG -O3)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
endif()

message(STATUS "Some compiler and linker options were changed by OsmiumOptions")
