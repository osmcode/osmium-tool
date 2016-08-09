#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - apply-changes
#
#-----------------------------------------------------------------------------

function(check_apply_changes _name _options _input _change _type _output)
    check_output(apply-changes ${_name} "apply-changes ${_options} --generator=test -f ${_type} apply-changes/${_input} apply-changes/${_change}" "apply-changes/${_output}")
endfunction()

check_apply_changes(data              ""                            input-data.osm input-change.osc "osm" output-data.osm)

add_test(NAME check-apply-changes-mixed1 COMMAND osmium apply-changes ${CMAKE_SOURCE_DIR}/test/apply-changes/input-data.osm ${CMAKE_SOURCE_DIR}/test/apply-changes/input-changes.osc -f osh)
set_tests_properties(check-apply-changes-mixed1 PROPERTIES WILL_FAIL true)

add_test(NAME check-apply-changes-mixed2 COMMAND osmium apply-changes ${CMAKE_SOURCE_DIR}/test/apply-changes/input-data.osh ${CMAKE_SOURCE_DIR}/test/apply-changes/input-changes.osc -f osm)
set_tests_properties(check-apply-changes-mixed2 PROPERTIES WILL_FAIL true)

check_apply_changes(history-osh-osh    ""               input-history.osh input-change.osc "osh" output-history.osh)
check_apply_changes(history-osh-osh-wh "--with-history" input-history.osh input-change.osc "osh" output-history.osh)
check_apply_changes(history-osm-osh-wh "--with-history" input-history.osm input-change.osc "osh" output-history.osh)
check_apply_changes(history-osh-osm-wh "--with-history" input-history.osh input-change.osc "osm" output-history.osh)


#-----------------------------------------------------------------------------
