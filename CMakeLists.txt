#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)

project(osmium VERSION 1.18.0 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


#-----------------------------------------------------------------------------
#
#  Project version
#
#-----------------------------------------------------------------------------

# It is important that this setting remains before the "project" call.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;Dev"
    CACHE STRING
    "List of available configuration types"
    FORCE)

set(AUTHOR "Jochen Topf <jochen@topf.org>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_LZ4 "Build with lz4 support for PBF files" ON)

option(BUILD_TESTING "Build the tests" ON)

# Some tests compare with binary files which might be different on your
# setup. Setting this to OFF disables those test.
# See https://github.com/osmcode/osmium-tool/issues/274
option(RUN_TESTS_WITH_BINARY_COMPARE "Run tests that do binary comparisons" ON)


#-----------------------------------------------------------------------------
#
#  Find external dependencies
#
#-----------------------------------------------------------------------------

find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(Osmium 2.20.0 REQUIRED COMPONENTS io)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})

find_package(nlohmann_json)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp)
include_directories(SYSTEM ${NLOHMANN_INCLUDE_DIR})

if(WITH_LZ4)
    find_package(LZ4)

    if(LZ4_FOUND)
        message(STATUS "lz4 library found, compiling with it")
        add_definitions(-DOSMIUM_WITH_LZ4)
        include_directories(SYSTEM ${LZ4_INCLUDE_DIRS})
        list(APPEND OSMIUM_LIBRARIES ${LZ4_LIBRARIES})
    else()
        message(WARNING "lz4 library not found, compiling without it")
    endif()
else()
    message(STATUS "Building without lz4 support: Set WITH_LZ4=ON to change this")
endif()


#-----------------------------------------------------------------------------
#
#  Optional "cppcheck" target that checks C++ code
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for cppcheck")
find_program(CPPCHECK cppcheck)

if(CPPCHECK)
    message(STATUS "Looking for cppcheck - found")
    set(CPPCHECK_OPTIONS --enable=warning,style,performance,portability,information,missingInclude)

    # cpp doesn't find system includes for some reason, suppress that report
    set(CPPCHECK_OPTIONS ${CPPCHECK_OPTIONS} --suppress=missingIncludeSystem)

    add_custom_target(cppcheck ${CPPCHECK} --std=c++14 ${CPPCHECK_OPTIONS} ${CMAKE_SOURCE_DIR}/src/*pp)
else()
    message(STATUS "Looking for cppcheck - not found")
    message(STATUS "  Build target 'cppcheck' will not be available")
endif()


#-----------------------------------------------------------------------------
#
#  Optional "iwyu" target to check headers
#  https://include-what-you-use.org/
#
#-----------------------------------------------------------------------------
find_program(IWYU_TOOL NAMES iwyu_tool iwyu_tool.py)

if(IWYU_TOOL)
    message(STATUS "Looking for iwyu_tool.py - found")
    add_custom_target(iwyu ${IWYU_TOOL} -p ${CMAKE_BINARY_DIR} -- --mapping_file=${CMAKE_SOURCE_DIR}/iwyu.imp)
else()
    message(STATUS "Looking for iwyu_tool.py - not found")
    message(STATUS "  Make target 'iwyu' will not be available")
endif()


#-----------------------------------------------------------------------------
#
#  Decide which C++ version to use (Minimum/default: C++14).
#
#-----------------------------------------------------------------------------
if(NOT MSVC)
    if(NOT USE_CPP_VERSION)
        set(USE_CPP_VERSION c++14)
    endif()
    message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
    add_compile_options(-std=${USE_CPP_VERSION})
endif()


#-----------------------------------------------------------------------------
#
#  Compiler and Linker flags
#
#-----------------------------------------------------------------------------
if(MSVC)
    set(DEV_COMPILE_OPTIONS "/Ox")
    set(RWD_COMPILE_OPTIONS "/Ox /DNDEBUG")
else()
    set(DEV_COMPILE_OPTIONS "-O3 -g")
    set(RWD_COMPILE_OPTIONS "-O3 -g -DNDEBUG")
endif()

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32)
endif()

set(CMAKE_CXX_FLAGS_DEV "${DEV_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during developer builds."
    FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEV ""
    CACHE STRING "Flags used by the linker during developer builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_DEV
    CMAKE_EXE_LINKER_FLAGS_DEV
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${RWD_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during RELWITHDEBINFO builds."
    FORCE)


#-----------------------------------------------------------------------------
#
#  Build Type
#
#-----------------------------------------------------------------------------

add_definitions(${OSMIUM_WARNING_OPTIONS})

# In 'Dev' mode: compile with very strict warnings and turn them into errors.
if(CMAKE_BUILD_TYPE STREQUAL "Dev")
    if(NOT MSVC)
        add_definitions(-Werror)
    endif()
endif()

# Force RelWithDebInfo build type if none was given
if(CMAKE_BUILD_TYPE)
    set(build_type ${CMAKE_BUILD_TYPE})
else()
    set(build_type "RelWithDebInfo")
endif()

set(CMAKE_BUILD_TYPE ${build_type}
    CACHE STRING
    "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
    FORCE)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")


#-----------------------------------------------------------------------------
#
#  Version
#
#-----------------------------------------------------------------------------

find_package(Git)

if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty=-changed
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE VERSION_FROM_GIT
                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(VERSION_FROM_GIT)
        set(VERSION_FROM_GIT " (${VERSION_FROM_GIT})")
    endif()
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/src/version.cpp.in
    ${PROJECT_BINARY_DIR}/src/version.cpp
)

#-----------------------------------------------------------------------------

configure_file(
    ${PROJECT_SOURCE_DIR}/osmium-wrapper.in
    ${PROJECT_BINARY_DIR}/osmium
)

include_directories(SYSTEM include)
include_directories(${PROJECT_BINARY_DIR}/src)

#-----------------------------------------------------------------------------

SET(OSMIUM_COMMANDS
    add-locations-to-ways
    apply-changes
    cat
    changeset-filter
    check-refs
    create-locations-index
    derive-changes
    diff
    export
    extract
    fileinfo
    getid
    getparents
    merge
    merge-changes
    query-locations-index
    removeid
    renumber
    show
    sort
    tags-count
    tags-filter
    time-filter
)

set(OSMIUM_SOURCE_FILES
    cmd.cpp
    cmd_factory.cpp
    id_file.cpp
    io.cpp
    util.cpp
    command_help.cpp
    option_clean.cpp
    export/export_format_json.cpp
    export/export_format_pg.cpp
    export/export_format_text.cpp
    export/export_handler.cpp
    extract/extract_bbox.cpp
    extract/extract.cpp
    extract/extract_polygon.cpp
    extract/geojson_file_parser.cpp
    extract/geometry_util.cpp
    extract/osm_file_parser.cpp
    extract/poly_file_parser.cpp
    extract/strategy_complete_ways.cpp
    extract/strategy_complete_ways_with_history.cpp
    extract/strategy_simple.cpp
    extract/strategy_smart.cpp
)

foreach(_command ${OSMIUM_COMMANDS})
    string(REPLACE "-" "_" _ucmd ${_command})
    list(APPEND OSMIUM_SOURCE_FILES "command_${_ucmd}.cpp")
endforeach()

add_subdirectory(man)
add_subdirectory(src)


#-----------------------------------------------------------------------------
#
#  Tests
#
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
#
#  Optional "clang-tidy" target
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for clang-tidy")
find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-20 clang-tidy-19 clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy-15 clang-tidy-14)

if(CLANG_TIDY)
    message(STATUS "Looking for clang-tidy - found ${CLANG_TIDY}")

    file(GLOB _unit_tests RELATIVE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/test/*/*.cpp")

    add_custom_target(clang-tidy
        ${CLANG_TIDY}
        -p ${CMAKE_BINARY_DIR}
        ${OSMIUM_SOURCE_FILES} ${_unit_tests}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
    )
else()
    message(STATUS "Looking for clang-tidy - not found")
    message(STATUS "  Build target 'clang-tidy' will not be available.")
endif()

#-----------------------------------------------------------------------------
#
#  Test install
#
#  Does a local install and checks that the files we expect to be installed
#  are installed and no extra files are installed. If this fails make sure
#  the list of files in cmake/test_install.cmake is correct. Only enables
#  if the WITH_EXTRA_TESTS option is set, because this test fails on some
#  platforms even if everything is correct.
#
#-----------------------------------------------------------------------------
option(WITH_EXTRA_TESTS "Run extra tests (usually only for developers)")
if(WITH_EXTRA_TESTS)
    add_test(NAME testinstall
             COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/test_install.cmake)
endif()

#-----------------------------------------------------------------------------
