add_library(cpp_library_template hello_world.cpp log.cpp version.cpp)
add_library(cpp_library_template::cpp_library_template ALIAS cpp_library_template)

# Generate the export macro header. With CMAKE_CXX_VISIBILITY_PRESET=hidden this
# is what makes a symbol part of the shared library ABI.
include(GenerateExportHeader)
generate_export_header(
    cpp_library_template
    BASE_NAME CPP_LIBRARY_TEMPLATE
    EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/generated/cpp_library_template/export.hpp"
)

# version.hpp carries the git-derived version as compile-time constants.
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/version.hpp.in"
    "${PROJECT_BINARY_DIR}/generated/cpp_library_template/version.hpp"
    @ONLY
)

target_sources(
    cpp_library_template
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
            FILES
                "${PROJECT_SOURCE_DIR}/include/cpp_library_template/cpp_library_template.hpp"
                "${PROJECT_SOURCE_DIR}/include/cpp_library_template/hello_world.hpp"
                "${PROJECT_SOURCE_DIR}/include/cpp_library_template/log.hpp"
    PUBLIC
        FILE_SET generated_headers
            TYPE HEADERS
            BASE_DIRS "${PROJECT_BINARY_DIR}/generated"
            FILES
                "${PROJECT_BINARY_DIR}/generated/cpp_library_template/export.hpp"
                "${PROJECT_BINARY_DIR}/generated/cpp_library_template/version.hpp"
)

target_compile_features(cpp_library_template PUBLIC cxx_std_20)
target_link_libraries(cpp_library_template PRIVATE $<BUILD_INTERFACE:cpp_library_template_warnings>)

set_target_properties(
    cpp_library_template
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME cpp_library_template
        EXPORT_NAME cpp_library_template
)

# A static build must not advertise dllimport/visibility attributes.
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(cpp_library_template PUBLIC CPP_LIBRARY_TEMPLATE_STATIC_DEFINE)
endif()
