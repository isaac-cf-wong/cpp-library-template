# Driver for the package_consumption test. Run in `cmake -P` script mode, so
# only variables passed with -D are available.
#
# Steps: install this project into a throwaway prefix, configure and build the
# consumer project against it, run the consumer, check its output.

foreach(_required PROJECT_BINARY_DIR CONSUMER_SOURCE_DIR STAGING_DIR EXPECTED_VERSION)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "run_package_test.cmake: -D${_required} is required")
    endif()
endforeach()

set(_prefix "${STAGING_DIR}/prefix")
set(_build "${STAGING_DIR}/build")

# Start from a clean slate so a stale install cannot mask a missing header.
file(REMOVE_RECURSE "${_prefix}" "${_build}")

function(run_step description)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE _result OUTPUT_VARIABLE _output ERROR_VARIABLE _output)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "package_consumption: ${description} failed (exit ${_result})\n${_output}")
    endif()
    set(_last_output "${_output}" PARENT_SCOPE)
endfunction()

set(_install_args
    "${CMAKE_COMMAND}"
    --install
    "${PROJECT_BINARY_DIR}"
    --prefix
    "${_prefix}"
)
if(CONFIG)
    list(APPEND _install_args --config "${CONFIG}")
endif()
run_step("installing the project" ${_install_args})

set(_configure_args
    "${CMAKE_COMMAND}"
    -S
    "${CONSUMER_SOURCE_DIR}"
    -B
    "${_build}"
    "-DCMAKE_PREFIX_PATH=${_prefix}"
    "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
)
if(GENERATOR)
    list(APPEND _configure_args -G "${GENERATOR}")
endif()
if(CXX_COMPILER)
    list(APPEND _configure_args "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}")
endif()
if(CONFIG)
    list(APPEND _configure_args "-DCMAKE_BUILD_TYPE=${CONFIG}")
endif()
run_step("configuring the consumer project" ${_configure_args})

set(_build_args "${CMAKE_COMMAND}" --build "${_build}")
if(CONFIG)
    list(APPEND _build_args --config "${CONFIG}")
endif()
run_step("building the consumer project" ${_build_args})

# The consumer prints the version it linked against, so locate its binary and
# check what comes back. On Windows a shared build also needs the DLL on PATH,
# which the install layout already provides via <prefix>/bin.
find_program(
    _consumer
    NAMES consumer consumer.exe
    PATHS "${_build}" "${_build}/${CONFIG}" "${_build}/bin" "${_build}/bin/${CONFIG}"
    NO_DEFAULT_PATH
    REQUIRED
)

if(WIN32)
    set(ENV{PATH} "${_prefix}/bin;$ENV{PATH}")
endif()

run_step("running the consumer" "${_consumer}")

if(NOT _last_output MATCHES "Hello, template!")
    message(FATAL_ERROR "package_consumption: unexpected consumer output:\n${_last_output}")
endif()

if(NOT _last_output MATCHES "${EXPECTED_VERSION}")
    message(
        FATAL_ERROR
        "package_consumption: consumer reported a different version than the project "
        "(expected ${EXPECTED_VERSION}):\n${_last_output}"
    )
endif()

message(STATUS "package_consumption: ok\n${_last_output}")
