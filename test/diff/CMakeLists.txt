#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - diff
#
#-----------------------------------------------------------------------------

function(check_diff _name _options _input1 _input2 _output)
    check_output(diff ${_name} "diff ${_options} --generator=test diff/${_input1} diff/${_input2}" "diff/${_output}")
endfunction()

check_diff(compact "" input1.osm input2.osm output-compact)
set_tests_properties(diff-compact PROPERTIES WILL_FAIL true)

check_diff(compact-c "-c" input1.osm input2.osm output-compact-c)
set_tests_properties(diff-compact-c PROPERTIES WILL_FAIL true)

check_diff(opl "-f opl" input1.osm input2.osm output.opl)
set_tests_properties(diff-opl PROPERTIES WILL_FAIL true)

check_diff(opl-c "-f opl -c" input1.osm input2.osm output-c.opl)
set_tests_properties(diff-opl-c PROPERTIES WILL_FAIL true)


#-----------------------------------------------------------------------------
