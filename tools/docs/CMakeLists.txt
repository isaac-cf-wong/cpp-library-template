# Documentation targets. Enabled with -DCPP_LIBRARY_TEMPLATE_BUILD_DOCS=ON.
#
#   cmake --build build/dev --target docs        # doxygen XML -> Markdown
#   cmake --build build/dev --target docs-serve  # live-reloading local site
#
# Doxygen parses the headers; tools/docs/gen_api_pages.py renders its XML as Markdown
# into docs/api/generated/; Zensical builds the whole site. The generated
# Markdown is a build artefact and is git ignored.

find_package(Doxygen REQUIRED)
find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)

set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs/doxygen")
# Doxygen's INPUT is a space separated list; each entry is quoted so a path with
# a space in it survives. The Doxyfile template must therefore *not* add quotes
# of its own around the substitution.
set(DOXYGEN_INPUT_DIRS "\"${PROJECT_SOURCE_DIR}/include\" \"${PROJECT_BINARY_DIR}/generated\"")
set(DOXYGEN_XML_DIR "${DOXYGEN_OUTPUT_DIR}/xml")
set(GENERATED_API_DIR "${PROJECT_SOURCE_DIR}/docs/api/generated")

# An undocumented public symbol is a defect, so CI fails on a Doxygen warning.
# Locally it is only a warning, to keep the inner loop usable.
if(DEFINED ENV{CI})
    set(DOXYGEN_WARN_AS_ERROR "FAIL_ON_WARNINGS")
else()
    set(DOXYGEN_WARN_AS_ERROR "NO")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)

# The generated headers must exist before Doxygen parses them.
add_custom_command(
    OUTPUT "${DOXYGEN_XML_DIR}/index.xml"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXYGEN_OUTPUT_DIR}"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
    DEPENDS "${PROJECT_BINARY_DIR}/Doxyfile" cpp_library_template
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Parsing the public headers with Doxygen"
    VERBATIM
)

add_custom_command(
    OUTPUT "${GENERATED_API_DIR}/index.md"
    COMMAND
        "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/docs/gen_api_pages.py" --xml "${DOXYGEN_XML_DIR}" --output
        "${GENERATED_API_DIR}"
    DEPENDS "${DOXYGEN_XML_DIR}/index.xml" "${PROJECT_SOURCE_DIR}/tools/docs/gen_api_pages.py"
    COMMENT "Rendering the Doxygen XML as Markdown"
    VERBATIM
)

add_custom_target(docs DEPENDS "${GENERATED_API_DIR}/index.md")

find_program(ZENSICAL_EXECUTABLE zensical)
if(ZENSICAL_EXECUTABLE)
    add_custom_target(
        docs-build
        COMMAND "${ZENSICAL_EXECUTABLE}" build
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        DEPENDS docs
        COMMENT "Building the documentation site into site/"
        VERBATIM
    )

    add_custom_target(
        docs-serve
        COMMAND "${ZENSICAL_EXECUTABLE}" serve
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        DEPENDS docs
        COMMENT "Serving the documentation site on http://127.0.0.1:8000"
        VERBATIM
    )
else()
    message(
        STATUS
        "cpp_library_template: zensical not found, only the 'docs' target is available. "
        "Install it with 'uv pip install -r tools/docs/requirements.txt'."
    )
endif()
