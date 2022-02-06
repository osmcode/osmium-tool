#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  Osmium Tool Tests
#
#-----------------------------------------------------------------------------

include_directories(include)
include_directories(../src)
include_directories(../src/extract)
include_directories(../include)

set(ALL_UNIT_TESTS
    cat/test_setup.cpp
    diff/test_setup.cpp
    extract/test_unit.cpp
    time-filter/test_setup.cpp
    util/test_unit.cpp
)

foreach(_source_file ${OSMIUM_SOURCE_FILES})
    list(APPEND ALL_COMMANDS "../src/${_source_file}")
endforeach()

add_executable(unit_tests unit_tests.cpp ${ALL_COMMANDS} ${ALL_UNIT_TESTS} ${PROJECT_BINARY_DIR}/src/version.cpp)
target_link_libraries(unit_tests ${Boost_LIBRARIES} ${OSMIUM_LIBRARIES})
set_pthread_on_target(unit_tests)
add_test(NAME unit_tests COMMAND unit_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")


#-----------------------------------------------------------------------------

configure_file(io/Makefile.in ${CMAKE_CURRENT_BINARY_DIR}/io/Makefile @ONLY)

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

function(check_output _dir _name _command _reference)
    set(_cmd "$<TARGET_FILE:osmium> ${_command}")
    if(ARGC GREATER 4)
        set(_return_code "${ARGV4}")
    else()
        set(_return_code 0)
    endif()
    add_test(
        NAME "${_dir}-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D reference:FILEPATH=${PROJECT_SOURCE_DIR}/test/${_reference}
        -D output:FILEPATH=${PROJECT_BINARY_DIR}/test/${_dir}/cmd-output-${_name}
        -D return_code=${_return_code}
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()

function(check_output2 _dir _name _tmpdir _command1 _command2 _reference)
    set(_cmd1 "$<TARGET_FILE:osmium> ${_command1}")
    set(_cmd2 "$<TARGET_FILE:osmium> ${_command2}")
    add_test(
        NAME "${_dir}-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd1}
        -D cmd2:FILEPATH=${_cmd2}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D tmpdir:PATH=${_tmpdir}
        -D reference:FILEPATH=${PROJECT_SOURCE_DIR}/test/${_reference}
        -D output:FILEPATH=${PROJECT_BINARY_DIR}/test/${_dir}/cmd-output-${_name}
        -D return_code=0
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()


#-----------------------------------------------------------------------------
#
#  Configure tests for all commands
#
#-----------------------------------------------------------------------------

foreach(_cmd IN LISTS OSMIUM_COMMANDS)
    do_test(help_cmd_${_cmd} "osmium ${_cmd} -h" "Usage: osmium ${_cmd}.*OPTIONS:")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_cmd}/CMakeLists.txt")
        message(STATUS "Adding tests in ${_cmd}")
        add_subdirectory(${_cmd})
    else()
        message(STATUS "No tests for ${_cmd} command found")
    endif()
endforeach()

foreach(_extra_tests formats help misc)
    message(STATUS "Adding tests in ${_extra_tests}")
    add_subdirectory(${_extra_tests})
endforeach()


#-----------------------------------------------------------------------------
