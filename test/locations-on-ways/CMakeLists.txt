#
# Test LocationsOnWays warning functionality across multiple commands
#

function(check_warning_cat _name _input _format _expected_stderr)
    set(_cmd "$<TARGET_FILE:osmium> cat --no-progress --generator=test ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/warning-cat-${_name}.osm --overwrite")
    add_test(
        NAME "locations-on-ways-cat-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_stderr.cmake
    )
endfunction()

function(check_no_warning_cat _name _input _format)
    set(_cmd "$<TARGET_FILE:osmium> cat --no-progress --generator=test ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/no-warning-cat-${_name}.osm --overwrite")
    add_test(
        NAME "locations-on-ways-cat-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_no_stderr.cmake
    )
endfunction()

function(check_warning_sort _name _input _format _expected_stderr)
    set(_cmd "$<TARGET_FILE:osmium> sort --no-progress --generator=test ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/warning-sort-${_name}.osm --overwrite")
    add_test(
        NAME "locations-on-ways-sort-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_stderr.cmake
    )
    set(_cmd_mp "$<TARGET_FILE:osmium> sort --no-progress --generator=test -s multipass ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/warning-sort-${_name}-mp.osm --overwrite")
    add_test(
        NAME "locations-on-ways-sort-${_name}-mp"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd_mp}
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_stderr.cmake
    )
endfunction()

function(check_no_warning_sort _name _input _format)
    set(_cmd "$<TARGET_FILE:osmium> sort --no-progress --generator=test ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/no-warning-sort-${_name}.osm --overwrite")
    add_test(
        NAME "locations-on-ways-sort-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_no_stderr.cmake
    )
    set(_cmd_mp "$<TARGET_FILE:osmium> sort --no-progress --generator=test -s multipass ${CMAKE_SOURCE_DIR}/test/locations-on-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/locations-on-ways/no-warning-sort-${_name}-mp.osm --overwrite")
    add_test(
        NAME "locations-on-ways-sort-${_name}-mp"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd_mp}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_check_no_stderr.cmake
    )
endfunction()

#-----------------------------------------------------------------------------

# Test cases with files that HAVE locations on ways - should warn when output format doesn't preserve them
check_warning_cat(warning-pbf-to-xml input-with-locations.osm.pbf xml "WARNING: Input file contains locations on ways that will be lost in output.")
check_warning_sort(warning-pbf-to-xml input-with-locations.osm.pbf xml "WARNING: Input file contains locations on ways that will be lost in output.")

# Test cases with files that DON'T have locations on ways - should NOT warn
check_no_warning_cat(no-warning-pbf-to-xml input-without-locations.osm.pbf xml)
check_no_warning_sort(no-warning-pbf-to-xml input-without-locations.osm.pbf xml)

#-----------------------------------------------------------------------------