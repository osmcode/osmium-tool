#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests - add-locations-to-ways
#
#-----------------------------------------------------------------------------

function(check_add_locations_to_ways _name _options _input _output)
    check_output(add-locations-to-ways ${_name} "add-locations-to-ways ${_options} --generator=test --output-header=xml_josm_upload=false --output-format=xml add-locations-to-ways/${_input}" "add-locations-to-ways/${_output}")
endfunction()

function(check_add_locations_warning _name _input _format _expected_stderr)
    set(_cmd "$<TARGET_FILE:osmium> add-locations-to-ways --no-progress --generator=test add-locations-to-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/add-locations-to-ways/warning-${_name}.osm --overwrite")
    add_test(
        NAME "add-locations-to-ways-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D expected_stderr:STRING=${_expected_stderr}
        -P ${CMAKE_SOURCE_DIR}/test/add-locations-to-ways/run_test_check_stderr.cmake
    )
endfunction()

function(check_add_locations_no_warning _name _input _format)
    set(_cmd "$<TARGET_FILE:osmium> add-locations-to-ways --no-progress --generator=test add-locations-to-ways/${_input} -f ${_format} -o ${CMAKE_BINARY_DIR}/test/add-locations-to-ways/no-warning-${_name}.osm --overwrite")
    add_test(
        NAME "add-locations-to-ways-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -P ${CMAKE_SOURCE_DIR}/test/add-locations-to-ways/run_test_check_no_stderr.cmake
    )
endfunction()

check_add_locations_to_ways(taggednodes "" input.osm output.osm)
check_add_locations_to_ways(allnodes "-n" input.osm output-n.osm)
check_add_locations_to_ways(membernodes "--keep-member-nodes" input-rel.osm output-rel.osm)

check_add_locations_warning(warning-pbf-to-xml input-with-locations.osm.pbf xml "Warning! Input file contains locations on ways that will be lost in output.")
check_add_locations_warning(warning-pbf-to-opl input-with-locations.osm.pbf opl "Warning! Input file contains locations on ways that will be lost in output.")

check_add_locations_no_warning(no-warning-pbf-to-xml input-without-locations.osm.pbf xml)
check_add_locations_no_warning(no-warning-pbf-to-opl input-without-locations.osm.pbf opl)


#-----------------------------------------------------------------------------
