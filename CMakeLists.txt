#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
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

project(osmium)

set(OSMIUM_VERSION_MAJOR 1)
set(OSMIUM_VERSION_MINOR 7)
set(OSMIUM_VERSION_PATCH 1)

set(OSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})

set(AUTHOR "Jochen Topf <jochen@topf.org>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#-----------------------------------------------------------------------------
#
#  Find external dependencies
#
#-----------------------------------------------------------------------------

find_package(Boost 1.55.0 REQUIRED COMPONENTS program_options)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(Osmium 2.13.1 REQUIRED COMPONENTS io)
include_directories(SYSTEM ${OSMIUM_INCLUDE_DIRS})


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

    add_custom_target(cppcheck ${CPPCHECK} --std=c++11 ${CPPCHECK_OPTIONS} ${CMAKE_SOURCE_DIR}/src/*pp)
else()
    message(STATUS "Looking for cppcheck - not found")
    message(STATUS "  Build target 'cppcheck' will not be available")
endif()


#-----------------------------------------------------------------------------
#
#  Optional "iwyu" target to check headers
#  http://include-what-you-use.org/
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
#  Decide which C++ version to use (Minimum/default: C++11).
#
#-----------------------------------------------------------------------------
if(NOT MSVC)
    if(NOT USE_CPP_VERSION)
        set(USE_CPP_VERSION c++11)
    endif()
    message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
    # following only available from cmake 2.8.12:
    #   add_compile_options(-std=${USE_CPP_VERSION})
    # so using this instead:
    add_definitions(-std=${USE_CPP_VERSION})
endif()


#-----------------------------------------------------------------------------
#
#  Compiler and Linker flags
#
#-----------------------------------------------------------------------------
if(MSVC)
    set(USUAL_COMPILE_OPTIONS "/Ox")
else()
    set(USUAL_COMPILE_OPTIONS "-O3 -g")
endif()

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32)
endif()

set(CMAKE_CXX_FLAGS_DEV "${USUAL_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during developer builds."
    FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEV ""
    CACHE STRING "Flags used by the linker during developer builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_DEV
    CMAKE_EXE_LINKER_FLAGS_DEV
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${USUAL_COMPILE_OPTIONS}"
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
    derive-changes
    diff
    export
    extract
    fileinfo
    getid
    merge
    merge-changes
    renumber
    show
    sort
    tags-filter
    time-filter
    values-filter
)

set(OSMIUM_SOURCE_FILES
    cmd.cpp
    cmd_factory.cpp
    io.cpp
    util.cpp
    command_help.cpp
    export/export_format_json.cpp
    export/export_format_text.cpp
    export/export_handler.cpp
    extract/extract_bbox.cpp
    extract/extract.cpp
    extract/extract_polygon.cpp
    extract/geojson_file_parser.cpp
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

enable_testing()
add_subdirectory(test)

#-----------------------------------------------------------------------------
#
#  Optional "clang-tidy" target
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for clang-tidy")
find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-5.0 clang-tidy-4.0 clang-tidy-3.9 clang-tidy-3.8 clang-tidy-3.7)

if(CLANG_TIDY)
    message(STATUS "Looking for clang-tidy - found")

    # For a list of check options, see:
    # http://clang.llvm.org/extra/clang-tidy/checks/list.html

    list(APPEND CT_CHECKS "bugprone-*")

    list(APPEND CT_CHECKS "cert-*")

    list(APPEND CT_CHECKS "clang-analyzer-*")

    list(APPEND CT_CHECKS "google-*"
                         "-google-runtime-references")

    list(APPEND CT_CHECKS "hicpp-*")

    list(APPEND CT_CHECKS "llvm-*")

    list(APPEND CT_CHECKS "misc-*")

    list(APPEND CT_CHECKS "modernize-*"
                          "-modernize-make-unique") # not available in C++11

    list(APPEND CT_CHECKS "performance-*")

    list(APPEND CT_CHECKS "readability-*"
                         "-readability-implicit-bool-cast"
                         "-readability-redundant-member-init")

    string(REPLACE ";" "," ALL_CHECKS "${CT_CHECKS}")

    file(GLOB _unit_tests RELATIVE "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/test/*/*.cpp")

    add_custom_target(clang-tidy
        ${CLANG_TIDY}
        -p ${CMAKE_BINARY_DIR}
        -checks="${ALL_CHECKS}"
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
