#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - derive-changes
#
#-----------------------------------------------------------------------------

function(check_derive_changes _name _options _input1 _input2 _output)
    check_output(derive-changes ${_name} "derive-changes --generator=test -f osc ${_options} derive-changes/${_input1} derive-changes/${_input2}" "derive-changes/${_output}")
endfunction()

check_derive_changes(normal       ""                    input1.osm input2.osm output.osc)
check_derive_changes(keep_details "--keep-details"      input1.osm input2.osm output-keep-details.osc)
check_derive_changes(incr_version "--increment-version" input1.osm input2.osm output-incr-version.osc)


#-----------------------------------------------------------------------------
