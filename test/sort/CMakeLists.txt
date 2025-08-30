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

function(check_sort_warning _name _input _format _expected_stderr)
    set(_cmd "$<TARGET_FILE:osmium> sort --no-progress --generator=test sort/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/sort/warning-${_name}.osm --overwrite")
    add_test(
        NAME "sort-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/test/sort/run_test_check_stderr.cmake
    )
    set(_cmd_mp "$<TARGET_FILE:osmium> sort --no-progress --generator=test -s multipass sort/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/sort/warning-${_name}-mp.osm --overwrite")
    add_test(
        NAME "sort-${_name}-mp"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd_mp}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/test/sort/run_test_check_stderr.cmake
    )
endfunction()

function(check_sort_no_warning _name _input _format)
    set(_cmd "$<TARGET_FILE:osmium> sort --no-progress --generator=test sort/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/sort/no-warning-${_name}.osm --overwrite")
    add_test(
        NAME "sort-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -P ${CMAKE_SOURCE_DIR}/test/sort/run_test_check_no_stderr.cmake
    )
    set(_cmd_mp "$<TARGET_FILE:osmium> sort --no-progress --generator=test -s multipass sort/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/sort/no-warning-${_name}-mp.osm --overwrite")
    add_test(
        NAME "sort-${_name}-mp"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd_mp}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -P ${CMAKE_SOURCE_DIR}/test/sort/run_test_check_no_stderr.cmake
    )
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

check_sort_warning(warning-pbf-to-xml input-with-locations.osm.pbf xml "Warning! Input file contains locations on ways that will be lost in output.")
check_sort_warning(warning-pbf-to-opl input-with-locations.osm.pbf opl "Warning! Input file contains locations on ways that will be lost in output.")

check_sort_no_warning(no-warning-pbf-to-xml input-without-locations.osm.pbf xml)
check_sort_no_warning(no-warning-pbf-to-opl input-without-locations.osm.pbf opl)

#-----------------------------------------------------------------------------
