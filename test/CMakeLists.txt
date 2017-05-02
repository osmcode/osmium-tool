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

file(GLOB ALL_UNIT_TESTS */test_setup.cpp */test_unit.cpp)
file(GLOB ALL_COMMANDS ../src/command_*.cpp ../src/io.cpp ../src/cmd.cpp ../src/cmd_factory.cpp ../src/util.cpp ../src/*/*.cpp)
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
    add_test(
        NAME "${_dir}-${_name}"
        COMMAND ${CMAKE_COMMAND}
        -D cmd:FILEPATH=${_cmd}
        -D dir:PATH=${PROJECT_SOURCE_DIR}/test
        -D reference:FILEPATH=${PROJECT_SOURCE_DIR}/test/${_reference}
        -D output:FILEPATH=${PROJECT_BINARY_DIR}/test/${_dir}/cmd-output-${_name}
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
        -P ${CMAKE_SOURCE_DIR}/cmake/run_test_compare_output.cmake
    )
endfunction()


#-----------------------------------------------------------------------------
#
#  Check "osmium SUBCOMMAND -h" for most subcommands.
#
#-----------------------------------------------------------------------------
function(check_cmd_help _command)
    do_test(help_cmd_${_command} "osmium ${_command} -h" "Usage: osmium ${_command}.*OPTIONS:")
endfunction()

check_cmd_help(add-locations-to-ways)
check_cmd_help(apply-changes)
check_cmd_help(cat)
check_cmd_help(changeset-filter)
check_cmd_help(check-refs)
check_cmd_help(derive-changes)
check_cmd_help(diff)
check_cmd_help(export)
check_cmd_help(extract)
check_cmd_help(fileinfo)
check_cmd_help(getid)
check_cmd_help(merge)
check_cmd_help(merge-changes)
check_cmd_help(renumber)
check_cmd_help(show)
check_cmd_help(sort)
check_cmd_help(tags-filter)
check_cmd_help(time-filter)


#-----------------------------------------------------------------------------
#
#  Configure tests in all subdirectories
#
#-----------------------------------------------------------------------------

file(GLOB _testcmakes "test" "*/CMakeLists.txt")

foreach(_tc IN LISTS _testcmakes)
    string(REPLACE "/CMakeLists.txt" "" _dir ${_tc})
    message(STATUS "Adding tests in ${_dir}")
    add_subdirectory(${_dir})
endforeach()


#-----------------------------------------------------------------------------
