#
# Runs a test command and checks that stderr is empty (no warning message).
# Used for testing that no false positive warnings appear.
#

if(NOT cmd)
    message(FATAL_ERROR "Variable 'cmd' not defined")
endif()

if(NOT dir)
    message(FATAL_ERROR "Variable 'dir' not defined")
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

string(FIND "${_stderr}" "Warning! Input file contains locations on ways" _found_pos)
if(NOT _found_pos EQUAL -1)
    message(FATAL_ERROR "Unexpected warning message found in stderr output: '${_stderr}'")
endif()

message(STATUS "Test passed: No warning message found")