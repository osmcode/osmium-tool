#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - sort
#
#-----------------------------------------------------------------------------

function(check_sort2 _name _in1 _in2 _output)
    check_output(sort ${_name} "sort --generator=test -f osm sort/${_in1} sort/${_in2}" "sort/${_output}")
endfunction()

function(check_sort1 _name _input _output)
    check_output(sort ${_name} "sort --generator=test -f osc sort/${_input}" "sort/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_sort2(simple input-simple1.osm input-simple2.osm output-simple.osm)
check_sort2(bounds input-bounds1.osm input-bounds2.osm output-bounds.osm)
check_sort2(history input-history1.osm input-history2.osm output-history.osm)
check_sort1(change input-change.osc output-change.osc)


#-----------------------------------------------------------------------------
