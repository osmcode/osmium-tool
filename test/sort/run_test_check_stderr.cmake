#
# Runs a test command and checks that stderr contains expected warning message.
# Used for testing warning functionality.
#

if(NOT cmd)
    message(FATAL_ERROR "Variable 'cmd' not defined")
endif()

if(NOT dir)
    message(FATAL_ERROR "Variable 'dir' not defined")
endif()

if(NOT expected_stderr)
    message(FATAL_ERROR "Variable 'expected_stderr' not defined")
endif()

message("Executing: ${cmd}")
separate_arguments(cmd)

execute_process(
    COMMAND ${cmd}
    WORKING_DIRECTORY ${dir}
    RESULT_VARIABLE _return_code
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr
)

if(NOT _return_code EQUAL 0)
    message(FATAL_ERROR "Command failed with return code ${_return_code}")
endif()

string(FIND "${_stderr}" "${expected_stderr}" _found_pos)
if(_found_pos EQUAL -1)
    message(FATAL_ERROR "Expected stderr message '${expected_stderr}' not found in stderr output: '${_stderr}'")
endif()

message(STATUS "Test passed: Found expected stderr message")