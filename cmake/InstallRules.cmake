# Install and export rules.
#
# The goal is that a consumer needs exactly this and nothing else:
#
#   find_package(cpp_library_template REQUIRED)
#   target_link_libraries(app PRIVATE cpp_library_template::cpp_library_template)
#
# tests/package_test/ builds against the installed tree in CI, so a mistake here
# fails the pipeline rather than a downstream user's build.

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CPP_LIBRARY_TEMPLATE_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/cpp_library_template"
    CACHE STRING
    "Install location of the CMake package files"
)

install(
    TARGETS cpp_library_template
    EXPORT cpp_library_templateTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT cpp_library_template_Runtime
    LIBRARY
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT cpp_library_template_Runtime
        NAMELINK_COMPONENT cpp_library_template_Development
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT cpp_library_template_Development
    FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT cpp_library_template_Development
    FILE_SET generated_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT cpp_library_template_Development
)

if(CPP_LIBRARY_TEMPLATE_BUILD_CLI)
    install(
        TARGETS cpp_library_template_cli
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT cpp_library_template_Runtime
    )
endif()

install(
    EXPORT cpp_library_templateTargets
    DESTINATION "${CPP_LIBRARY_TEMPLATE_INSTALL_CMAKEDIR}"
    NAMESPACE cpp_library_template::
    FILE cpp_library_templateTargets.cmake
    COMPONENT cpp_library_template_Development
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/cpp_library_templateConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/cpp_library_templateConfig.cmake"
    INSTALL_DESTINATION "${CPP_LIBRARY_TEMPLATE_INSTALL_CMAKEDIR}"
)

# SameMajorVersion matches the SOVERSION set on the library: a consumer asking
# for 1.2 accepts any 1.x, and never a 2.x.
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/cpp_library_templateConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/cpp_library_templateConfig.cmake"
        "${PROJECT_BINARY_DIR}/cpp_library_templateConfigVersion.cmake"
    DESTINATION "${CPP_LIBRARY_TEMPLATE_INSTALL_CMAKEDIR}"
    COMPONENT cpp_library_template_Development
)

install(
    FILES "${PROJECT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/cpp_library_template"
    COMPONENT cpp_library_template_Runtime
)

# There is deliberately no export(EXPORT ...) build-tree export here. It would
# have to carry cpp_library_template_warnings, a BUILD_INTERFACE-only target,
# and CMake rejects that. FetchContent and add_subdirectory consumers get the
# cpp_library_template::cpp_library_template ALIAS target instead, which is the
# same name find_package() gives them.
