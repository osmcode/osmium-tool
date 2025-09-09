#
# Test script to check that stderr does not contain a warning message
#

if(NOT cmd)
    message(FATAL_ERROR "Variable 'cmd' not defined")
endif()

separate_arguments(cmd)

execute_process(
    COMMAND ${cmd}
    RESULT_VARIABLE _return_code
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr
)

if(NOT _return_code EQUAL 0)
    message(FATAL_ERROR "Command failed with return code ${_return_code}")
endif()

string(FIND "${_stderr}" "WARNING:" _found_pos)
if(NOT _found_pos EQUAL -1)
    message(FATAL_ERROR "Unexpected warning message found in stderr output: '${_stderr}'")
endif()

message(STATUS "Test passed: No warning message found")