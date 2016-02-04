#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - check-refs
#
#-----------------------------------------------------------------------------

add_test(NAME check-ref-w-okay COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/check-refs/okay.osm)
add_test(NAME check-ref-r-okay COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/okay.osm)

add_test(NAME check-ref-fail-n-in-w COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/check-refs/fail-n-in-w.osm)
set_tests_properties(check-ref-fail-n-in-w PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-w-way-okay COMMAND osmium check-refs    ${CMAKE_SOURCE_DIR}/test/check-refs/way-okay.osm)
add_test(NAME check-ref-r-way-okay COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/way-okay.osm)
set_tests_properties(check-ref-r-way-okay PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-okay-r-in-r COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/okay-r-in-r.osm)

add_test(NAME check-ref-fail-n-in-r COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/fail-n-in-r.osm)
set_tests_properties(check-ref-fail-n-in-r PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-w-in-r COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/fail-w-in-r.osm)
set_tests_properties(check-ref-fail-w-in-r PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-r-in-r-1 COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/fail-r-in-r-1.osm)
set_tests_properties(check-ref-fail-r-in-r-1 PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-r-in-r-2 COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/check-refs/fail-r-in-r-2.osm)
set_tests_properties(check-ref-fail-r-in-r-2 PROPERTIES WILL_FAIL true)

#-----------------------------------------------------------------------------

# input data not ordered properly

add_test(NAME check-ref-fail-order-n COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/order/fail-order-n.osm)
set_tests_properties(check-ref-fail-order-n PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-order-w COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/order/fail-order-w.osm)
set_tests_properties(check-ref-fail-order-w PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-order-r COMMAND osmium check-refs -r ${CMAKE_SOURCE_DIR}/test/order/fail-order-r.osm)
set_tests_properties(check-ref-fail-order-r PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-order-wn COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/order/fail-order-wn.osm)
set_tests_properties(check-ref-fail-order-wn PROPERTIES WILL_FAIL true)

add_test(NAME check-ref-fail-order-rw COMMAND osmium check-refs ${CMAKE_SOURCE_DIR}/test/order/fail-order-rw.osm)
set_tests_properties(check-ref-fail-order-rw PROPERTIES WILL_FAIL true)


#-----------------------------------------------------------------------------
