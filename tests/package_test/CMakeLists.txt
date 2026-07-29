# Registers one CTest test that installs this project into a staging prefix and
# then configures, builds and runs a standalone consumer project against it via
# find_package(). It catches the class of mistake unit tests cannot see: a header
# that is not installed, a missing export, a broken package config file.
#
# The test is labelled "integration" because it shells out to CMake and a
# compiler. Skip it during a fast inner loop with `ctest -LE integration`.

if(NOT CPP_LIBRARY_TEMPLATE_INSTALL)
    message(STATUS "cpp_library_template: install rules disabled, skipping the package consumption test")
    return()
endif()

# A coverage-instrumented library carries references to the gcov runtime
# (__gcov_init and friends). The consumer project is built without --coverage, so
# linking it would fail. The test adds no coverage of our own sources anyway: it
# builds in a separate tree.
if(CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE)
    message(STATUS "cpp_library_template: coverage build, skipping the package consumption test")
    return()
endif()

# The sanitizer runtime has the same problem as gcov: the consumer would need the
# matching -fsanitize flags to link against an instrumented library.
if(CPP_LIBRARY_TEMPLATE_SANITIZERS)
    message(STATUS "cpp_library_template: sanitizer build, skipping the package consumption test")
    return()
endif()

add_test(
    NAME package_consumption
    COMMAND
        "${CMAKE_COMMAND}" "-DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
        "-DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/consumer" "-DSTAGING_DIR=${PROJECT_BINARY_DIR}/package_test"
        "-DCONFIG=$<CONFIG>" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "-DGENERATOR=${CMAKE_GENERATOR}"
        "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DEXPECTED_VERSION=${PROJECT_VERSION}" -P
        "${CMAKE_CURRENT_SOURCE_DIR}/run_package_test.cmake"
)

set_tests_properties(package_consumption PROPERTIES LABELS integration TIMEOUT 600)
