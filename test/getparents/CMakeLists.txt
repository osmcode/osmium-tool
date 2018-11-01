#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - getparents
#
#-----------------------------------------------------------------------------

function(check_getparents _name _input _ids _output)
    check_output(getparents ${_name} "getparents --generator=test --output-header=xml_josm_upload=false -f osm getparents/${_input} ${_ids}" "getparents/${_output}")
endfunction()

function(check_getparents_r _name _input _ids _output)
    check_output(getparents ${_name}-s "getparents --generator=test --output-header=xml_josm_upload=false -f osm --add-self getparents/${_input} ${_ids}" "getparents/${_output}")
endfunction()

#-----------------------------------------------------------------------------

check_getparents(n10 input.osm n10 out-n10.osm)
check_getparents(n12 input.osm n12 out-n12.osm)
check_getparents(w20 input.osm w20 out-w20.osm)

check_getparents_r(n10 input.osm n10 out-n10-s.osm)
check_getparents_r(n12 input.osm n12 out-n12-s.osm)
check_getparents_r(w20 input.osm w20 out-w20-s.osm)


#-----------------------------------------------------------------------------
