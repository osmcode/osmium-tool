#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool
#
#-----------------------------------------------------------------------------

set(OSMIUM_SOURCE_FILES
    ${PROJECT_BINARY_DIR}/src/version.cpp
    cmd.cpp
    cmd_factory.cpp
    io.cpp
    main.cpp
    util.cpp
    command_add_locations_to_ways.cpp
    command_apply_changes.cpp
    command_cat.cpp
    command_changeset_filter.cpp
    command_check_refs.cpp
    command_derive_changes.cpp
    command_diff.cpp
    command_export.cpp
    command_extract.cpp
    command_fileinfo.cpp
    command_getid.cpp
    command_help.cpp
    command_merge_changes.cpp
    command_merge.cpp
    command_renumber.cpp
    commands.cpp
    command_show.cpp
    command_sort.cpp
    command_tags_filter.cpp
    command_time_filter.cpp
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

# This is the list of all source files that are in the repository. It should be
# the same as the configured list above, if not we'll find out about that below
# and create a fatal error.
file(GLOB _osmium_source_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp */*.cpp)

add_executable(osmium ${OSMIUM_SOURCE_FILES})

target_link_libraries(osmium ${Boost_LIBRARIES} ${OSMIUM_LIBRARIES})

set_pthread_on_target(osmium)

install(TARGETS osmium DESTINATION bin)

foreach(_file ${OSMIUM_SOURCE_FILES})
    list(REMOVE_ITEM _osmium_source_files ${_file})
endforeach()

if(NOT ${_osmium_source_files} STREQUAL "${PROJECT_BINARY_DIR}/src/version.cpp")
    message(FATAL_ERROR "Found source files that are not configured: ${_osmium_source_files}")
endif()

