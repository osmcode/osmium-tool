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

function(check_warning _name _input _format _expected_stderr)
    set(_cmd "$<TARGET_FILE:osmium> cat --no-progress --generator=test cat/${_input} -f ${_format} -o /tmp/test-warning-output.osm --overwrite")
    add_test(
        NAME "cat-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/test/cat/run_test_check_stderr.cmake
    )
endfunction()


#-----------------------------------------------------------------------------

check_cat(cat12 input1.osm input2.osm output-cat12.osm)
check_cat(cat21 input2.osm input1.osm output-cat21.osm)

check_convert(osm input1.osm output1.osm.opl opl)
check_convert(gzip input1.osm.gz output1.osm.opl opl)
check_convert(bzip2 input1.osm.bz2 output1.osm.opl opl)
check_convert(pbf input1.osm.pbf output1.osm.opl opl)
check_convert(opl output1.osm.opl output1.osm.opl opl)

check_warning(warning-xml input-with-locations.osm xml "Warning! Input file contains locations on ways that will be lost in output.")
check_warning(warning-pbf input-with-locations.osm pbf "Warning! Input file contains locations on ways that will be lost in output.")
check_warning(warning-opl input-with-locations.osm opl "Warning! Input file contains locations on ways that will be lost in output.")


#-----------------------------------------------------------------------------
