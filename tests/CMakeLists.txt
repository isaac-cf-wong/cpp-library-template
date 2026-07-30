# GoogleTest is resolved in this order:
#   1. A package already provided by the environment (Conan, vcpkg, the distro).
#   2. FetchContent, so `cmake -B build && cmake --build build && ctest` works
#      on a clean machine with no package manager installed.
#
# Keep the FetchContent tag and the conanfile.py requirement in step; Renovate
# updates both.

find_package(GTest 1.15 QUIET)

if(NOT GTest_FOUND)
    message(STATUS "cpp_library_template: GTest not found, fetching it")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0 # renovate: datasource=github-tags depName=google/googletest
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    # Do not let gtest hijack our build settings on Windows.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

add_executable(cpp_library_template_tests test_hello_world.cpp test_log.cpp test_version.cpp)

target_link_libraries(
    cpp_library_template_tests
    PRIVATE
        cpp_library_template::cpp_library_template
        $<BUILD_INTERFACE:cpp_library_template_warnings>
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)

# DISCOVERY_MODE PRE_TEST defers enumeration to test time, which is what makes
# this work when cross-compiling or running under a sanitizer.
gtest_discover_tests(cpp_library_template_tests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS unit)

# The `coverage` target from cmake/Coverage.cmake runs ctest, so it needs the test
# binary to exist. That target is created before this directory is processed,
# which is why the dependency is declared here rather than there.
if(TARGET coverage)
    add_dependencies(coverage cpp_library_template_tests)
endif()

# Command line tests. They exercise the argument parser through the real binary,
# which is the only way to test exit statuses.
add_subdirectory(cli_test)

# The API-surface test: configure, build and run a *separate* project that
# consumes the installed package through find_package(). This is the analogue of
# the Python template's smoke_test.py running against the built wheel.
add_subdirectory(package_test)
