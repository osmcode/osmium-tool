#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests
#
#-----------------------------------------------------------------------------

function(do_test _name _command _regex)
    separate_arguments(_command)
    add_test(NAME ${_name} COMMAND ${_command})
    set_tests_properties(${_name} PROPERTIES
                         ENVIRONMENT "MANPATH=${PROJECT_BINARY_DIR}/man"
                         PASS_REGULAR_EXPRESSION ${_regex}
    )
endfunction()

#-----------------------------------------------------------------------------

function(check_output _name _command _output)
    set(_cmd "${PROJECT_BINARY_DIR}/src/osmium ${_command}")
    add_test(
        NAME ${_name}
        COMMAND ${CMAKE_COMMAND}
        -Dcmd=${_cmd}
        -Ddir=${PROJECT_SOURCE_DIR}/test
        -Dreference=${_output}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()

function(check_output2 _name _command1 _command2 _output)
    set(_cmd1 "${PROJECT_BINARY_DIR}/src/osmium ${_command1}")
    set(_cmd2 "${PROJECT_BINARY_DIR}/src/osmium ${_command2}")
    add_test(
        NAME ${_name}
        COMMAND ${CMAKE_COMMAND}
        -Dcmd=${_cmd1}
        -Dcmd2=${_cmd2}
        -Ddir=${PROJECT_SOURCE_DIR}/test
        -Dreference=${_output}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()

#-----------------------------------------------------------------------------

add_subdirectory(cat)
add_subdirectory(check-refs)
add_subdirectory(fileinfo)
add_subdirectory(help)
add_subdirectory(renumber)
add_subdirectory(time-filter)


#-----------------------------------------------------------------------------
