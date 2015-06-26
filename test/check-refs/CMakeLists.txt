#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - check-refs
#
#-----------------------------------------------------------------------------

add_test(NAME check-ref-nwr-okay COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/nwr-okay.osm)
add_test(NAME check-ref-nw-okay COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/check-refs/nw-okay.osm)


#-----------------------------------------------------------------------------
