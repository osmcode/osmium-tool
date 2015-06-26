#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


#-----------------------------------------------------------------------------
#
#  Project version
#
#-----------------------------------------------------------------------------

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;Dev"
    CACHE STRING
    "List of available configuration types"
    FORCE)

project(osmium)

set(OSMIUM_VERSION_MAJOR 1)
set(OSMIUM_VERSION_MINOR 0)
set(OSMIUM_VERSION_PATCH 1)

set(OSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})

add_definitions(-DOSMIUM_VERSION="${OSMIUM_VERSION}")

set(AUTHOR "Jochen Topf <jochen@topf.org>")


#-----------------------------------------------------------------------------
#
#  Find external dependencies
#
#-----------------------------------------------------------------------------

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Osmium REQUIRED COMPONENTS io)
include_directories(${OSMIUM_INCLUDE_DIRS})


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
    message(STATUS "  Make target cppcheck not available")
endif(CPPCHECK)


#-----------------------------------------------------------------------------
#
#  Optional "man" target to generate man pages
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for pandoc")
find_program(PANDOC pandoc)

function(add_man_page _section _name)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/man/man${_section})
    set(_output_file ${CMAKE_BINARY_DIR}/man/man${_section}/${_name}.${_section})
    set(_source_file ${CMAKE_SOURCE_DIR}/man/${_name}.md)
    set(_install_dir "share/man/man{$_section}")
    string(TOUPPER ${_name} _name_upcase)
    add_custom_command(OUTPUT ${_output_file}
        COMMAND ${PANDOC}
            ${PANDOC_MAN_OPTIONS}
            --variable "title=${_name_upcase}"
            --variable "section=${_section}"
            -o ${_output_file}
            ${_source_file}
        DEPENDS ${_source_file}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building manpage ${_name}.${_section}"
        VERBATIM)
    set(ALL_MAN_PAGES "${ALL_MAN_PAGES};${_output_file}" PARENT_SCOPE)
endfunction()


if(PANDOC)
    message(STATUS "Looking for pandoc - found")
    message(STATUS "  Manual pages will be built")
    execute_process(COMMAND date "+%Y-%m-%d" OUTPUT_VARIABLE PUBDATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PANDOC_MAN_OPTIONS
        -s
        -t man
        --template ${CMAKE_CURRENT_SOURCE_DIR}/man/manpage.template
        --variable "description=osmium/${OSMIUM_VERSION}"
        --variable "date=${PUBDATE}"
        --variable "author=${AUTHOR}"
    )
    set(PANDOC_HTML_OPTIONS -s -t html)

    add_man_page(1 osmium)
    add_man_page(1 osmium-apply-changes)
    add_man_page(1 osmium-cat)
    add_man_page(1 osmium-check-refs)
    add_man_page(1 osmium-fileinfo)
    add_man_page(1 osmium-merge-changes)
    add_man_page(1 osmium-time-filter)
    add_man_page(5 osmium-file-formats)

    install(DIRECTORY ${CMAKE_BINARY_DIR}/man DESTINATION share)

    add_custom_target(man ALL DEPENDS ${ALL_MAN_PAGES})
else()
    message(STATUS "Looking for pandoc - not found")
    message(STATUS "  Manual pages will not be built")
endif(PANDOC)


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
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32
                    -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0600)
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

# In 'Dev' mode: compile with very strict warnings and turn them into errors.
if(CMAKE_BUILD_TYPE STREQUAL "Dev")
    if(NOT MSVC)
        add_definitions(-Werror)
    endif()
    add_definitions(${OSMIUM_WARNING_OPTIONS})
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


#-----------------------------------------------------------------------------

configure_file(
    ${PROJECT_SOURCE_DIR}/osmium-wrapper.in
    ${PROJECT_BINARY_DIR}/osmium
)


#-----------------------------------------------------------------------------
#
#  Tests
#
#-----------------------------------------------------------------------------

enable_testing()

function(do_test _name _command _regex)
    separate_arguments(_command)
    add_test(NAME ${_name} COMMAND ${_command})
    set_tests_properties(${_name} PROPERTIES
                         ENVIRONMENT "MANPATH=${PROJECT_BINARY_DIR}/man"
                         PASS_REGULAR_EXPRESSION ${_regex}
    )
endfunction()

do_test(version  "osmium --version" "^osmium version ${OSMIUM_VERSION}\n")
do_test(help     "osmium help"      "^Usage: .*Commands are:")
do_test(help2    "osmium --help"    "^Usage: .*Commands are:")

if(PANDOC)
    do_test(help_cat "osmium help cat" "^OSMIUM-CAT\\(1\\)")
endif()

#-----------------------------------------------------------------------------

function(check_output _name _command _output)
    set(_cmd "${PROJECT_BINARY_DIR}/src/osmium ${_command}")
    add_test(
        NAME ${_name}
        COMMAND ${CMAKE_COMMAND}
        -Dcmd=${_cmd}
        -Ddir=${PROJECT_SOURCE_DIR}/test
        -Dreference=${_output}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()

function(check_time_filter _name _type _from _to _output)
    check_output(time-filter-${_name} "time-filter --generator=test -f ${_type} time-filter/input.osh ${_from} ${_to}" "time-filter/output-${_output}.${_type}")
endfunction()

check_time_filter(last osm ""                   "" last)
check_time_filter(ts1  osm 2015-01-01T01:00:00Z "" ts1)
check_time_filter(ts1a osm 2015-01-01T01:01:00Z "" ts1)
check_time_filter(ts2  osm 2015-01-01T02:00:00Z "" ts2)
check_time_filter(ts2a osm 2015-01-01T02:01:00Z "" ts2)
check_time_filter(ts3  osm 2015-01-01T03:00:00Z "" ts3)
check_time_filter(ts3a osm 2015-01-01T03:01:00Z "" ts3)

check_time_filter(range-0-5   osh 2015-01-01T00:00:00Z 2015-01-01T05:00:00Z range-0-5)
check_time_filter(range-1-2   osh 2015-01-01T01:00:00Z 2015-01-01T02:00:00Z range-1-2)
check_time_filter(range-1a-2a osh 2015-01-01T01:01:00Z 2015-01-01T02:01:00Z range-1a-2a)
check_time_filter(range-2-3   osh 2015-01-01T02:00:00Z 2015-01-01T03:00:00Z range-2-3)
check_time_filter(range-2-3a  osh 2015-01-01T02:00:00Z 2015-01-01T03:01:00Z range-2-3a)
check_time_filter(range-2-4   osh 2015-01-01T02:00:00Z 2015-01-01T04:00:00Z range-2-4)


function(check_cat _name _in1 _in2 _output)
    check_output(cat-${_name} "cat --generator=test -f osm cat/${_in1} cat/${_in2}" "cat/${_output}")
endfunction()

check_cat(cat12 input1.osm input2.osm output-cat12.osm)
check_cat(cat21 input2.osm input1.osm output-cat21.osm)


function(check_fileinfo _name _options _input _output)
    check_output(fileinfo-${_name} "fileinfo ${_options} fileinfo/${_input}" "fileinfo/${_output}")
endfunction()

check_fileinfo(fi1 "--extended" fi1.osm fi1-result.txt)


#-----------------------------------------------------------------------------

include_directories(include)
include_directories(${PROJECT_BINARY_DIR}/src)

add_subdirectory(src)


#-----------------------------------------------------------------------------
