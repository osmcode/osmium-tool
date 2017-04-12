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
set(OSMIUM_VERSION_MINOR 6)
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

find_package(Osmium 2.12.1 REQUIRED COMPONENTS io)
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
#  Optional "man" target to generate man pages
#
#-----------------------------------------------------------------------------
message(STATUS "Looking for pandoc")
find_program(PANDOC pandoc)

function(add_man_page _section _name)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/man/man${_section})
    set(_output_file ${CMAKE_BINARY_DIR}/man/man${_section}/${_name}.${_section})
    set(_source_file ${CMAKE_SOURCE_DIR}/man/${_name}.md)
    set(_dest_file ${CMAKE_BINARY_DIR}/man-source/${_name}.md)
    file(READ ${CMAKE_SOURCE_DIR}/man/common-options.md MAN_COMMON_OPTIONS)
    file(READ ${CMAKE_SOURCE_DIR}/man/progress-options.md MAN_PROGRESS_OPTIONS)
    file(READ ${CMAKE_SOURCE_DIR}/man/input-options.md MAN_INPUT_OPTIONS)
    file(READ ${CMAKE_SOURCE_DIR}/man/output-options.md MAN_OUTPUT_OPTIONS)
    file(READ ${CMAKE_SOURCE_DIR}/export-example-config/default-config.json EXPORT_DEFAULT_CONFIG)
    configure_file(${_source_file} ${_dest_file} @ONLY)
    string(TOUPPER ${_name} _name_upcase)
    add_custom_command(OUTPUT ${_output_file}
        COMMAND ${PANDOC}
            ${PANDOC_MAN_OPTIONS}
            --variable "title=${_name_upcase}"
            --variable "section=${_section}"
            -o ${_output_file}
            ${_dest_file}
        DEPENDS ${_source_file} man/manpage.template man/common-options.md man/input-options.md man/output-options.md
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building manpage ${_name}.${_section}"
        VERBATIM)
    set(ALL_MAN_PAGES "${ALL_MAN_PAGES};${_output_file}" PARENT_SCOPE)
endfunction()


if(PANDOC)
    message(STATUS "Looking for pandoc - found")
    message(STATUS "  Manual pages will be built")
    set(PANDOC_MAN_OPTIONS
        -s
        -t man
        --template ${CMAKE_CURRENT_SOURCE_DIR}/man/manpage.template
        --variable "description=osmium/${OSMIUM_VERSION}"
        --variable "version=${OSMIUM_VERSION}"
        --variable "author=${AUTHOR}"
    )
    set(PANDOC_HTML_OPTIONS -s -t html)

    add_man_page(1 osmium)
    add_man_page(1 osmium-add-locations-to-ways)
    add_man_page(1 osmium-apply-changes)
    add_man_page(1 osmium-cat)
    add_man_page(1 osmium-changeset-filter)
    add_man_page(1 osmium-check-refs)
    add_man_page(1 osmium-derive-changes)
    add_man_page(1 osmium-diff)
    add_man_page(1 osmium-export)
    add_man_page(1 osmium-extract)
    add_man_page(1 osmium-fileinfo)
    add_man_page(1 osmium-getid)
    add_man_page(1 osmium-merge)
    add_man_page(1 osmium-merge-changes)
    add_man_page(1 osmium-renumber)
    add_man_page(1 osmium-show)
    add_man_page(1 osmium-sort)
    add_man_page(1 osmium-tags-filter)
    add_man_page(1 osmium-time-filter)
    add_man_page(5 osmium-file-formats)

    install(DIRECTORY ${CMAKE_BINARY_DIR}/man DESTINATION share)

    add_custom_target(man ALL DEPENDS ${ALL_MAN_PAGES})
else()
    message(STATUS "Looking for pandoc - not found")
    message(STATUS "  Manual pages will not be built")
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
#
#  Tests
#
#-----------------------------------------------------------------------------

enable_testing()
add_subdirectory(test)

#-----------------------------------------------------------------------------

add_subdirectory(src)


#-----------------------------------------------------------------------------
