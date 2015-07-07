#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - getid
#
#-----------------------------------------------------------------------------

function(check_getid _name _input _output)
    check_output(getid ${_name} "getid --generator=test -f osm getid/${_input} n11,n12 w21" "getid/${_output}")
endfunction()

check_getid(n input.osm output.osm)

#-----------------------------------------------------------------------------
