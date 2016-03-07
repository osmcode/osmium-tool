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

function(check_getid_file _name _file _input _output)
    check_output(getid ${_name} "getid --generator=test -i getid/${_file} -f osm getid/${_input}" "getid/${_output}")
endfunction()

check_getid(n input.osm output.osm)
check_getid_file(file1 idfile input.osm output.osm)

#-----------------------------------------------------------------------------
