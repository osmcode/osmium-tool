#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - apply-changes
#
#-----------------------------------------------------------------------------

function(check_apply_changes _name _options _input _change _type _output)
    check_output(apply-changes-${_name} "apply-changes ${_options} --generator=test -f ${_type} apply-changes/${_input} apply-changes/${_change}" "apply-changes/${_output}")
endfunction()

check_apply_changes(withhist "" input-data.osm input-change.osc "osh" output.osh)
check_apply_changes(simplified "--simplify --remove-deleted" input-data.osm input-change.osc "osm" simplified.osm)


#-----------------------------------------------------------------------------
