#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - merge
#
#-----------------------------------------------------------------------------

function(check_merge1 _name _input1 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1}" "merge/${_output}")
endfunction()

function(check_merge2 _name _input1 _input2 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1} merge/${_input2}" "merge/${_output}")
endfunction()

function(check_merge3 _name _input1 _input2 _input3 _output)
    check_output(merge ${_name} "merge --generator=test -f osm merge/${_input1} merge/${_input2} merge/${_input3}" "merge/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_merge1(i1 input1.osm output1.osm)
check_merge2(i2f input1.osm input2.osm output2.osm)
check_merge2(i2r input2.osm input1.osm output2.osm)
check_merge3(i3f input1.osm input2.osm input3.osm output3.osm)

# If both input files do not have timestamp attributes
check_merge2(i2f-only-version input1-only-version.osm input2-only-version.osm output2-12-only-version.osm)
check_merge2(i2r-only-version input2-only-version.osm input1-only-version.osm output2-12-only-version.osm)

#-----------------------------------------------------------------------------
