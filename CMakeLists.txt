#----------------------------------------------------------------------
#
#  Osmium Tool CMakeLists.txt
#
#----------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.5)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


#----------------------------------------------------------------------
#
#  Project version
#
#----------------------------------------------------------------------

project(osmium)

set(OSMIUM_VERSION_MAJOR 0)
set(OSMIUM_VERSION_MINOR 0)
set(OSMIUM_VERSION_PATCH 1)

set(OSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})


#----------------------------------------------------------------------
#
#  Find external dependencies
#
#----------------------------------------------------------------------

find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
if(NOT Boost_FOUND)
    message(WARNING "Boost program_options not found!\n")
    set(MISSING_LIB 1)
endif()

find_package(Cryptopp REQUIRED)
include_directories(${CRYPTOPP_INCLUDE_DIRS})
if(NOT CRYPTOPP_FOUND)
    message(WARNING "Crypto++/Cryptopp library not found!\n")
    set(MISSING_LIB 1)
endif()

include(OsmiumOptions)

find_package(Osmium REQUIRED COMPONENTS io)
include_directories(${OSMIUM_INCLUDE_DIRS})
if(NOT OSMIUM_FOUND)
    message(WARNING "Libosmium not found!\n")
    set(MISSING_LIB 1)
endif()


#----------------------------------------------------------------------
#
#  Check that all required libraries are available
#
#----------------------------------------------------------------------
if(MISSING_LIB)
    message(FATAL_ERROR "Required library or libraries missing. Aborting...")
endif(MISSING_LIB)


#----------------------------------------------------------------------
#
#  Optional "cppcheck" target that checks C++ code
#
#----------------------------------------------------------------------
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


#----------------------------------------------------------------------
#
#  Optional "man" target to generate man pages
#
#----------------------------------------------------------------------
message(STATUS "Looking for pandoc")
find_program(PANDOC pandoc)

if(PANDOC)
    message(STATUS "Looking for pandoc - found")
    message(STATUS "  Manual pages will be built")
    execute_process(COMMAND date "+%Y-%m-%d" OUTPUT_VARIABLE PUBDATE OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PANDOC_MAN_OPTIONS -s -t man --template ${CMAKE_CURRENT_SOURCE_DIR}/doc/manpage.template --variable "description=osmium/${OSMIUM_VERSION}" --variable "date=${PUBDATE}")
    set(PANDOC_HTML_OPTIONS -s -t html)

    set(MAN_PAGES_1 osmium.1 osmium-apply-changes.1 osmium-cat.1 osmium-fileinfo.1 osmium-merge-changes.1 osmium-time-filter.1)
    set(MAN_PAGES_5 osmium-file-formats.5)
    set(MAN_NAMES ${MAN_PAGES_1} ${MAN_PAGES_5})
    set(MAN_FILES)
    foreach(m IN LISTS MAN_NAMES)
        set(mf ${CMAKE_BINARY_DIR}/${m})
        string(REGEX REPLACE ".[0-9]\$" "" mws "${m}")
        set(ms ${CMAKE_SOURCE_DIR}/doc/${mws}.md)
        add_custom_command(OUTPUT ${mf}
            COMMAND ${PANDOC} ${PANDOC_MAN_OPTIONS} -o ${mf} ${ms}
            DEPENDS ${ms}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Building manpage ${mf}"
            VERBATIM)
        list(APPEND MAN_FILES ${mf})
    endforeach()

    add_custom_target(man ALL DEPENDS ${MAN_FILES})

    foreach(m IN LISTS MAN_PAGES_1)
        install(FILES ${CMAKE_BINARY_DIR}/${m} DESTINATION share/man/man1)
    endforeach()
    foreach(m IN LISTS MAN_PAGES_5)
        install(FILES ${CMAKE_BINARY_DIR}/${m} DESTINATION share/man/man5)
    endforeach()
else()
    message(STATUS "Looking for pandoc - not found")
    message(STATUS "  Manual pages will not be built")
endif(PANDOC)


#----------------------------------------------------------------------
#
#  Optional "format" target that formats C++ code
#
#----------------------------------------------------------------------
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


#----------------------------------------------------------------------
#
#  Create src/config.hpp
#
#----------------------------------------------------------------------
configure_file(
    ${PROJECT_SOURCE_DIR}/src/config.hpp.in
    ${PROJECT_BINARY_DIR}/src/config.hpp
)

include_directories(${PROJECT_BINARY_DIR}/src)


#----------------------------------------------------------------------
#
#  Decide which C++ version to use (Minimum/default: C++11).
#
#----------------------------------------------------------------------

if(NOT USE_CPP_VERSION)
    set(USE_CPP_VERSION c++11)
endif()
message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
# following only available from cmake 2.8.12:
#   add_compile_options(-std=${USE_CPP_VERSION})
# so using this instead:
add_definitions(-std=${USE_CPP_VERSION})

#----------------------------------------------------------------------

if(NOT MSVC)
    add_definitions(${OSMIUM_WARNING_OPTIONS})
endif()


#----------------------------------------------------------------------

add_subdirectory(src)


#----------------------------------------------------------------------
