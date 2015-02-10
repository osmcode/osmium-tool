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

project(osmium)

set(OSMIUM_VERSION_MAJOR 0)
set(OSMIUM_VERSION_MINOR 0)
set(OSMIUM_VERSION_PATCH 1)

set(OSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})

add_definitions(-DOSMIUM_VERSION="${OSMIUM_VERSION}")


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
option(BUILD_WITH_CRYPTOPP "Build using libcryptopp" ON)

if(BUILD_WITH_CRYPTOPP)
    find_package(Cryptopp)
    include_directories(${CRYPTOPP_INCLUDE_DIRS})
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
    set(_source_file ${CMAKE_SOURCE_DIR}/doc/${_name}.md)
    set(_install_dir "share/man/man{$_section}")
    add_custom_command(OUTPUT ${_output_file}
        COMMAND ${PANDOC} ${PANDOC_MAN_OPTIONS} -o ${_output_file} ${_source_file}
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
    set(PANDOC_MAN_OPTIONS -s -t man --template ${CMAKE_CURRENT_SOURCE_DIR}/doc/manpage.template --variable "description=osmium/${OSMIUM_VERSION}" --variable "date=${PUBDATE}")
    set(PANDOC_HTML_OPTIONS -s -t html)

    add_man_page(1 osmium)
    add_man_page(1 osmium-apply-changes)
    add_man_page(1 osmium-cat)
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
#  Optional "format" target that formats C++ code
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for astyle")
find_program(ASTYLE astyle)

if(ASTYLE)
    add_custom_target(format astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none --recursive src/*pp)
    message(STATUS "Looking for astyle - found")
    message(STATUS "  Adding 'indent' target")
else()
    message(STATUS "Looking for astyle - not found")
    message(STATUS "  No 'indent' target available")
endif(ASTYLE)


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
#  Build Type
#
#-----------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug Release RelWithDebInfo MinSizeRel Dev")

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


if(BUILD_WITH_CRYPTOPP AND CRYPTOPP_FOUND)
    do_test(version_sha "osmium --version" "\nSHA support: enabled\n")
else()
    do_test(version_sha "osmium --version" "\nSHA support: disabled\n")
endif()


#-----------------------------------------------------------------------------

include_directories(${PROJECT_BINARY_DIR}/src)

add_subdirectory(src)


#-----------------------------------------------------------------------------
if(BUILD_WITH_CRYPTOPP AND NOT CRYPTOPP_FOUND)
    message(WARNING "=========================================================================
  Libcryptopp/crypto++ not found! - Some functionality will be missing.
  Install the library/headers to fix this. You might have to set the
  CRYPTOPP_INCLUDE_DIR and CRYPTOPP_LIBRARY CMake variables.
=========================================================================")
endif()


#-----------------------------------------------------------------------------
