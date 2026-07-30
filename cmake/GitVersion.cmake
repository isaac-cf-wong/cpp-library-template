# Derive the project version from git tags.
#
# This is the C++ analogue of hatch-vcs: the annotated git tag pushed by
# .github/workflows/scheduled_release.yml is the only place a version number is
# written down. Nothing in the repository needs editing at release time.
#
# cpp_library_template_version_from_git(<numeric_var> <full_var>)
#
#   <numeric_var> receives a strict MAJOR.MINOR.PATCH string suitable for
#                 project(VERSION ...). CMake rejects anything else.
#   <full_var>    receives the descriptive version, which may carry a
#                 pre-release suffix and distance/commit information, e.g.
#                 "0.3.1-4-gdeadbee" or "1.0.0-alpha.1".
#
# Resolution order:
#   1. CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE (cache variable or -D). Conan passes
#      this, because there is no .git inside the Conan cache.
#   2. A version.txt next to this CMakeLists.txt. The source archives attached to
#      a GitHub Release carry one, since `git archive` output has no .git either.
#   3. git describe --tags --dirty.
#   4. The fallback 0.0.0, with a warning.
#
# Every path leads back to the same git tag; there is no second place a version
# is decided.

function(cpp_library_template_version_from_git numeric_var full_var)
    set(_fallback "0.0.0")

    if(DEFINED CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE AND NOT CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE STREQUAL "")
        set(_described "${CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE}")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" _described)
        string(STRIP "${_described}" _described)
    else()
        find_package(Git QUIET)
        set(_described "")
        if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" describe --tags --dirty --match "v[0-9]*"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE _described
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
    endif()

    if(_described STREQUAL "")
        message(
            WARNING
            "cpp_library_template: no git tag found, falling back to version ${_fallback}. "
            "Pass -DCPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE=<version>, or place a version.txt at the "
            "project root, for reproducible archive builds."
        )
        set(_described "${_fallback}")
    endif()

    # Strip the leading "v" used by the tag pattern.
    string(REGEX REPLACE "^v" "" _full "${_described}")

    # Extract the leading MAJOR.MINOR.PATCH for project(VERSION ...).
    if(_full MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        set(_numeric "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    else()
        message(WARNING "cpp_library_template: cannot parse '${_described}' as a version, using ${_fallback}.")
        set(_numeric "${_fallback}")
    endif()

    set(${numeric_var} "${_numeric}" PARENT_SCOPE)
    set(${full_var} "${_full}" PARENT_SCOPE)
endfunction()
