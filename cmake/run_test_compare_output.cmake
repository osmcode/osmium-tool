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

if(NOT output)
    message(FATAL_ERROR "Variable 'output' not defined")
endif()

message("Executing: ${cmd}")
separate_arguments(cmd)

execute_process(
    COMMAND ${cmd}
    WORKING_DIRECTORY ${dir}
    RESULT_VARIABLE result
    OUTPUT_FILE ${output}
    ERROR_VARIABLE stderr
)

if(NOT (stderr STREQUAL ""))
    message(SEND_ERROR "Command tested wrote to stderr: ${stderr}")
endif()

if(result)
    message(FATAL_ERROR "Error when calling '${cmd}': ${result}")
endif()

if(cmd2)
    message("Executing: ${cmd2}")
    separate_arguments(cmd2)

    execute_process(
        COMMAND ${cmd2}
        WORKING_DIRECTORY ${dir}
        RESULT_VARIABLE result
        OUTPUT_FILE ${output}
        ERROR_VARIABLE stderr
    )

    if(NOT (stderr STREQUAL ""))
        message(SEND_ERROR "Command tested wrote to stderr: ${stderr}")
    endif()

    if(result)
        message(FATAL_ERROR "Error when calling '${cmd}': ${result}")
    endif()
endif()

set(compare "${CMAKE_COMMAND} -E compare_files ${reference} ${output}")
message("Executing: ${compare}")
separate_arguments(compare)
execute_process(
    COMMAND ${compare}
    RESULT_VARIABLE result
)

if(result)
    message(SEND_ERROR "Test output does not match '${reference}'. Output is in '${output}'.")
endif()

