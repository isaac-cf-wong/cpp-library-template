# The example command line tool. It is a *consumer* of the library, wired the
# same way a downstream project would be, so breaking the public API breaks this
# target too.

add_executable(cpp_library_template_cli main.cpp)

set_target_properties(cpp_library_template_cli PROPERTIES OUTPUT_NAME cpp_library_template)

target_link_libraries(
    cpp_library_template_cli
    PRIVATE cpp_library_template::cpp_library_template $<BUILD_INTERFACE:cpp_library_template_warnings>
)
