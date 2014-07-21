
project(osmium)

set(OSMIUM_VERSION 0.0.1)

cmake_minimum_required(VERSION 2.8.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#----------------------------------------------------------------------
find_package(Threads REQUIRED)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIRS})

find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${BOOST_INCLUDE_DIRS})

find_package(Cryptopp REQUIRED)
include_directories(${CRYPTOPP_INCLUDE_DIRS})

find_package(OSMPBF REQUIRED)
include_directories(${OSMPBF_INCLUDE_DIRS})

find_package(Osmium REQUIRED)
include_directories(${OSMIUM_INCLUDE_DIRS})

#----------------------------------------------------------------------
add_subdirectory(src)

install(FILES README.md DESTINATION share/doc/osmium)

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
endif()


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
else()
    message(STATUS "Looking for pandoc - not found")
    message(STATUS "  Manual pages will not be built")
endif()

#----------------------------------------------------------------------
