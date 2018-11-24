#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - cat
#
#-----------------------------------------------------------------------------

function(check_cat _name _in1 _in2 _output)
    check_output(cat ${_name} "cat --no-progress --generator=test -f osm cat/${_in1} cat/${_in2}" "cat/${_output}")
endfunction()

function(check_convert _name _input _output _format)
    check_output(cat ${_name} "cat --no-progress --generator=test cat/${_input} -f ${_format}" "cat/${_output}")
endfunction()


#-----------------------------------------------------------------------------

check_cat(cat12 input1.osm input2.osm output-cat12.osm)
check_cat(cat21 input2.osm input1.osm output-cat21.osm)

check_convert(osm input1.osm output1.osm.opl opl)
check_convert(gzip input1.osm.gz output1.osm.opl opl)
check_convert(bzip2 input1.osm.bz2 output1.osm.opl opl)
check_convert(pbf input1.osm.pbf output1.osm.opl opl)
check_convert(opl output1.osm.opl output1.osm.opl opl)


#-----------------------------------------------------------------------------
