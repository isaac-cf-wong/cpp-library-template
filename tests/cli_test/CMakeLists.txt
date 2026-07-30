# Command line tests: run the built executable and check its exit status and its
# output together.
#
# The CLI had no tests at all until an argument-parsing defect went unnoticed
# through review and into main. The cases that matter most below are the two
# marked as regression guards: an option that takes a value has to advance the
# argument index by exactly two, and getting that wrong makes the value look like
# a command. That is invisible in a build and obvious in a test.

if(NOT TARGET cpp_library_template_cli)
    message(STATUS "cpp_library_template: CLI not built, skipping the command line tests")
    return()
endif()

# Each case is a separate call rather than an entry in a packed string list. The
# first attempt packed them with a `|` separator, which silently mangled every
# expectation that contained a `|` -- and the log format is
# `TIMESTAMP | LEVEL | name | message`.
#
# add_cli_test(<name> <arguments> <expected exit status> <output regex>)
function(add_cli_test name arguments expected_code output_regex)
    add_test(
        NAME "cli_${name}"
        COMMAND
            "${CMAKE_COMMAND}" "-DCLI=$<TARGET_FILE:cpp_library_template_cli>" "-DCLI_ARGS=${arguments}"
            "-DEXPECT_CODE=${expected_code}" "-DEXPECT_OUTPUT=${output_regex}" -P
            "${CMAKE_CURRENT_SOURCE_DIR}/run_cli_test.cmake"
    )
    set_tests_properties("cli_${name}" PROPERTIES LABELS cli)
endfunction()

add_cli_test(help "--help" 0 "usage: cpp_library_template")
add_cli_test(short_help "-h" 0 "Commands:")
add_cli_test(version "--version" 0 "^[0-9]+\\.[0-9]+\\.[0-9]+")

add_cli_test(hello_defaults_to_world "hello" 0 "Hello, world!")
add_cli_test(hello_named "hello Isaac" 0 "Hello, Isaac!")
add_cli_test(goodbye_named "goodbye Isaac" 0 "Goodbye, Isaac!")

add_cli_test(no_arguments_is_a_usage_error "" 2 "usage:")
add_cli_test(unknown_command "bogus" 2 "unknown command 'bogus'")
add_cli_test(unknown_option "-x" 2 "unknown option '-x'")
add_cli_test(verbose_requires_a_value "--verbose" 2 "requires a level")
add_cli_test(verbose_rejects_an_unknown_level "--verbose bogus hello" 1 "unknown log level: bogus")

# Regression guards for the argument-index arithmetic: the level must be
# consumed, and whatever follows it must still be seen.
add_cli_test(verbose_consumes_its_value "--verbose info hello" 0 "Hello, world!")
add_cli_test(verbose_then_command_and_argument "--verbose info goodbye Isaac" 0 "Goodbye, Isaac!")

add_cli_test(verbose_emits_a_log_record "--verbose info hello" 0 "\\| info \\| cpp_library_template \\|")
add_cli_test(verbose_off_is_silent "--verbose off hello" 0 "^Hello, world!")
add_cli_test(last_verbose_wins "-v error -v info hello" 0 "\\| info \\|")
