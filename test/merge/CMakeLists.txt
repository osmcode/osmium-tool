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

# If one input file does not have timestamp attributes …
check_merge2(i2f-2-only-version input1.osm input2-only-version.osm output2-2-only-version.osm)
check_merge2(i2r-2-only-version input2-only-version.osm input1.osm output2-2-only-version.osm)
check_merge2(i2f-1-only-version input1-only-version.osm input2.osm output2-1-only-version.osm)
check_merge2(i2r-1-only-version input2.osm input1-only-version.osm output2-1-only-version.osm)

# If two input files do not have timestamp attributes …
check_merge3(i3f-1-only-version input1-only-version.osm input2.osm input3.osm output3-1-only-version.osm)
check_merge3(i3r-1-only-version input3.osm input2.osm input1-only-version.osm output3-1-only-version.osm)
check_merge3(i3m-1-only-version input2.osm input3.osm input1-only-version.osm output3-1-only-version.osm)

check_merge3(i3f-2-only-version input1.osm input2-only-version.osm input3.osm output3-2-only-version.osm)
check_merge3(i3r-2-only-version input3.osm input2-only-version.osm input1.osm output3-2-only-version.osm)
check_merge3(i3m-2-only-version input3.osm input1.osm input2-only-version.osm output3-2-only-version.osm)

check_merge3(i3f-12-only-version input1-only-version.osm input2-only-version.osm input3.osm output3-12-only-version.osm)
check_merge3(i3r-12-only-version input3.osm input2-only-version.osm input1-only-version.osm output3-12-only-version.osm)
check_merge3(i3m-12-only-version input3.osm input1-only-version.osm input2-only-version.osm output3-12-only-version.osm)

check_merge3(i3f-13-only-version input1-only-version.osm input2.osm input3-only-version.osm output3-13-only-version.osm)
check_merge3(i3r-13-only-version input3-only-version.osm input2.osm input1-only-version.osm output3-13-only-version.osm)
check_merge3(i3m-13-only-version input2.osm input1-only-version.osm input3-only-version.osm output3-13-only-version.osm)

check_merge3(i3f-23-only-version input1.osm input2-only-version.osm input3-only-version.osm output3-23-only-version.osm)
check_merge3(i3r-23-only-version input3-only-version.osm input2-only-version.osm input1.osm output3-23-only-version.osm)
check_merge3(i3m-23-only-version input2-only-version.osm input3-only-version.osm input1.osm output3-23-only-version.osm)

#-----------------------------------------------------------------------------
