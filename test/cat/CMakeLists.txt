#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - cat
#
#-----------------------------------------------------------------------------

function(check_cat _name _in1 _in2 _output)
    check_output(cat-${_name} "cat --generator=test -f osm cat/${_in1} cat/${_in2}" "cat/${_output}")
endfunction()

#-----------------------------------------------------------------------------

check_cat(cat12 input1.osm input2.osm output-cat12.osm)
check_cat(cat21 input2.osm input1.osm output-cat21.osm)


#-----------------------------------------------------------------------------
