#
#  Runs a test command given in the variable 'cmd' in directory 'dir'.
#  Checks that the return code is 0.
#  Checks that there is nothing on stderr.
#  Compares output on stdout with reference file in variable 'reference'.
#

if(NOT cmd)
    message(FATAL_ERROR "Variable 'cmd' not defined")
endif()

if(NOT dir)
    message(FATAL_ERROR "Variable 'dir' not defined")
endif()

if(NOT reference)
    message(FATAL_ERROR "Variable 'reference' not defined")
endif()

separate_arguments(cmd)

execute_process(
    COMMAND ${cmd}
    WORKING_DIRECTORY ${dir}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

if(result)
    message(SEND_ERROR "Error when calling '${cmd}': ${result}")
endif()

if(NOT (stderr STREQUAL ""))
    message(SEND_ERROR "Command tested wrote to stderr: ${_stderr}")
endif()

file(READ ${dir}/${reference} ref)

if(NOT (ref STREQUAL stdout))
    file(WRITE "test.stdout" ${stdout})
    message(SEND_ERROR "Test output does not match '${reference}'. Output is in 'test.stdout'.")
endif()

