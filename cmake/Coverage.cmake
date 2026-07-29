# Coverage instrumentation, plus a "coverage" target that renders the report.
#
# The report format mirrors the Python template: a Cobertura XML file named
# coverage.xml at the build root, which codecov/codecov-action uploads directly.
# gcovr is the driver for both GCC (gcov) and Clang (llvm-cov gcov).

if(NOT CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE)
    return()
endif()

if(MSVC)
    message(FATAL_ERROR "CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE is not supported with MSVC.")
endif()

message(STATUS "cpp_library_template: enabling coverage instrumentation")

# -O0 keeps line attribution honest; without it inlining smears the report.
add_compile_options(--coverage -O0 -g -fprofile-update=atomic)
add_link_options(--coverage)

find_program(GCOVR_EXECUTABLE gcovr)
if(NOT GCOVR_EXECUTABLE)
    message(
        WARNING
        "gcovr not found: the 'coverage' target will not be available. Install it with 'pipx install gcovr'."
    )
    return()
endif()

# gcov reports the paths the compiler recorded, which are fully resolved. If the
# source tree is reached through a symlink, PROJECT_SOURCE_DIR does not match
# them and every file is silently filtered out -- a 0% report rather than an
# error. Resolve the path once and use it for both --root and --filter.
file(REAL_PATH "${PROJECT_SOURCE_DIR}" _coverage_source_dir)

set(_gcovr_args
    --root
    "${_coverage_source_dir}"
    --filter
    "${_coverage_source_dir}/src/"
    --filter
    "${_coverage_source_dir}/include/"
    --exclude-unreachable-branches
    --exclude-throw-branches
    --decisions
    --print-summary
)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    list(APPEND _gcovr_args --gcov-executable "llvm-cov gcov")
endif()

add_custom_target(
    coverage
    COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure --test-dir "${PROJECT_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/coverage"
    COMMAND
        "${GCOVR_EXECUTABLE}" ${_gcovr_args} --xml-pretty --output "${PROJECT_BINARY_DIR}/coverage.xml"
        "${PROJECT_BINARY_DIR}"
    COMMAND
        "${GCOVR_EXECUTABLE}" ${_gcovr_args} --html-details "${PROJECT_BINARY_DIR}/coverage/index.html"
        "${PROJECT_BINARY_DIR}"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Running the test suite and writing coverage.xml + coverage/index.html"
    VERBATIM
)
