#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - sort
#
#-----------------------------------------------------------------------------

function(check_sort2 _name _in1 _in2 _output)
    check_output(sort ${_name} "sort --generator=test -f osm sort/${_in1} sort/${_in2}" "sort/${_output}")
    check_output(sort ${_name}_mp "sort --generator=test -f osm -s multipass sort/${_in1} sort/${_in2}" "sort/${_output}")
endfunction()

function(check_sort1 _name _input _output _format)
    check_output(sort ${_name} "sort --generator=test -f ${_format} sort/${_input}" "sort/${_output}")
    check_output(sort ${_name}_mp "sort --generator=test -f ${_format} -s multipass sort/${_input}" "sort/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_sort2(simple input-simple1.osm input-simple2.osm output-simple.osm)
check_sort2(bounds input-bounds1.osm input-bounds2.osm output-bounds.osm)
check_sort2(history input-history1.osm input-history2.osm output-history.osm)

check_sort1(neg input-neg.osm output-neg.osm osm)
check_sort1(change input-change.osc output-change.osc osc)

# Tests with limited metadata
check_sort2(simple-1-only-version input-simple1-only-version.osm input-simple2.osm output-simple-1-only-version.osm)
check_sort1(mixed-metadata input-simple-onefile.osm output-simple-onefile.osm osm)
check_sort1(history-partially-only-version input-history-partially-only-version.osm output-history-partially-only-version.osm osm)
check_sort1(history-only-version input-history-only-version.osm output-history-only-version.osm osm)

#-----------------------------------------------------------------------------
