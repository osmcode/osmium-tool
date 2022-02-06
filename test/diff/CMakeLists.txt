#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - diff
#
#-----------------------------------------------------------------------------

function(check_diff _name _options _input1 _input2 _output _return_code)
    check_output(diff ${_name} "diff ${_options} diff/${_input1} diff/${_input2}" "diff/${_output}" "${_return_code}")
endfunction()

check_diff(compact "" input1.osm input2.osm output-compact 1)

check_diff(compact-c "-c" input1.osm input2.osm output-compact-c 1)

check_diff(opl "-f opl" input1.osm input2.osm output.opl 1)

check_diff(opl-c "-f opl -c" input1.osm input2.osm output-c.opl 1)

check_diff(same "" input1.osm input1.osm output-same 0)

#-----------------------------------------------------------------------------
