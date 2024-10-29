#
#  First, if variable 'tmpdir' ist set, this directory will be removed with
#  all its content and recreated.
#
#  Then runs a test command given in the variable 'cmd' in directory 'dir'.
#  Checks that the return code is the same as variable 'return_code'.
#  Checks that there is nothing on stderr.
#  If the variable 'cmd2' is set, the command will be run and checked in the
#  same manner.
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

if(tmpdir)
    file(REMOVE_RECURSE ${tmpdir})
    file(MAKE_DIRECTORY ${tmpdir})
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

if(NOT ("$ENV{osmium_cmake_stderr}" STREQUAL "ignore"))
    if(NOT (stderr STREQUAL ""))
        message(SEND_ERROR "Command tested wrote to stderr: ${stderr}")
    endif()
endif()

if(NOT result EQUAL ${return_code})
    message(FATAL_ERROR "Error when calling '${cmd}': ${result} (should be ${return_code})")
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

set(compare "${CMAKE_COMMAND};-E;compare_files;${reference};${output}")
message("Executing: ${compare}")
execute_process(
    COMMAND ${compare}
    RESULT_VARIABLE result
)

if(result AND NOT result EQUAL 0)
    message(SEND_ERROR "Test output does not match '${reference}'. Output is in '${output}'. Result: ${result}.")
endif()

