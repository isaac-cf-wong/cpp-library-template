cmake_minimum_required(VERSION 3.25)

# ---------------------------------------------------------------------------
# Version
#
# The version is derived from git tags -- it is never written down in this file.
# This mirrors the way hatch-vcs works for Python packages: the git tag is the
# single source of truth, and CI creates tags from conventional commits.
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GitVersion)
cpp_library_template_version_from_git(CPP_LIBRARY_TEMPLATE_VERSION CPP_LIBRARY_TEMPLATE_VERSION_FULL)

project(
    cpp_library_template
    VERSION ${CPP_LIBRARY_TEMPLATE_VERSION}
    DESCRIPTION "A GitHub template repository for a C++ library"
    HOMEPAGE_URL "https://github.com/isaac-cf-wong/cpp-library-template"
    LANGUAGES CXX
)

# Detect whether we are the top-level project. Everything user-facing
# (tests, docs, CLI, install rules) defaults to OFF when consumed via
# add_subdirectory()/FetchContent so we never surprise a downstream build.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL ON)
else()
    set(CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL OFF)
endif()

option(CPP_LIBRARY_TEMPLATE_BUILD_TESTS "Build the test suite" ${CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL})
option(CPP_LIBRARY_TEMPLATE_BUILD_CLI "Build the example command line tool" ${CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL})
option(CPP_LIBRARY_TEMPLATE_BUILD_DOCS "Add the Doxygen documentation targets" OFF)
option(CPP_LIBRARY_TEMPLATE_INSTALL "Generate install and export rules" ${CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL})
option(CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE "Instrument the build for coverage reporting" OFF)
option(CPP_LIBRARY_TEMPLATE_WARNINGS_AS_ERRORS "Turn compiler warnings into errors" OFF)
set(CPP_LIBRARY_TEMPLATE_SANITIZERS
    ""
    CACHE STRING
    "Semicolon separated sanitizers to enable, e.g. 'address;undefined'"
)

# ---------------------------------------------------------------------------
# Global build settings (only when we are the top level project)
# ---------------------------------------------------------------------------
if(CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Always emit compile_commands.json for clang-tidy, clangd and the IDEs.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Hide symbols by default so the exported ABI is exactly what is annotated
    # with CPP_LIBRARY_TEMPLATE_EXPORT (see the generated export header).
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    endif()

    # Keep the layout predictable across generators.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif()

include(CompilerWarnings)
include(Sanitizers)
include(Coverage)

# ---------------------------------------------------------------------------
# The library
# ---------------------------------------------------------------------------
add_subdirectory(src)

if(CPP_LIBRARY_TEMPLATE_BUILD_CLI)
    add_subdirectory(apps)
endif()

if(CPP_LIBRARY_TEMPLATE_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

if(CPP_LIBRARY_TEMPLATE_BUILD_DOCS)
    add_subdirectory(tools/docs)
endif()

if(CPP_LIBRARY_TEMPLATE_INSTALL)
    include(InstallRules)
endif()

if(CPP_LIBRARY_TEMPLATE_IS_TOP_LEVEL)
    message(STATUS "cpp_library_template ${CPP_LIBRARY_TEMPLATE_VERSION_FULL}")
    message(STATUS "  build type ......... ${CMAKE_BUILD_TYPE}")
    message(STATUS "  shared library ..... ${BUILD_SHARED_LIBS}")
    message(STATUS "  tests .............. ${CPP_LIBRARY_TEMPLATE_BUILD_TESTS}")
    message(STATUS "  cli ................ ${CPP_LIBRARY_TEMPLATE_BUILD_CLI}")
    message(STATUS "  coverage ........... ${CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE}")
    message(STATUS "  sanitizers ......... ${CPP_LIBRARY_TEMPLATE_SANITIZERS}")
endif()
