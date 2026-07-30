# Driver for the command line tests. Run in `cmake -P` script mode, so only
# variables passed with -D are available.
#
# CTest can assert an exit code (WILL_FAIL) or match output
# (PASS_REGULAR_EXPRESSION), but not both: setting a pass expression makes it
# ignore the exit status. The CLI's contract is both -- 0 on success, 2 for a
# usage error, 1 for a bad value, and specific text on stderr -- so it gets a
# driver that checks the pair.
#
#   -DCLI=<path>            executable under test
#   -DCLI_ARGS=<string>     arguments, space separated (may be empty)
#   -DEXPECT_CODE=<int>     required exit status
#   -DEXPECT_OUTPUT=<regex> required to appear in stdout+stderr (may be empty)

foreach(_required CLI EXPECT_CODE)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "run_cli_test.cmake: -D${_required} is required")
    endif()
endforeach()

separate_arguments(_args UNIX_COMMAND "${CLI_ARGS}")

execute_process(COMMAND "${CLI}" ${_args} RESULT_VARIABLE _code OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr)

set(_output "${_stdout}${_stderr}")

if(NOT _code EQUAL EXPECT_CODE)
    message(FATAL_ERROR "exit status was ${_code}, expected ${EXPECT_CODE}.\nArgs: ${CLI_ARGS}\nOutput:\n${_output}")
endif()

if(DEFINED EXPECT_OUTPUT AND NOT EXPECT_OUTPUT STREQUAL "")
    if(NOT _output MATCHES "${EXPECT_OUTPUT}")
        message(FATAL_ERROR "output did not match '${EXPECT_OUTPUT}'.\nArgs: ${CLI_ARGS}\nOutput:\n${_output}")
    endif()
endif()
