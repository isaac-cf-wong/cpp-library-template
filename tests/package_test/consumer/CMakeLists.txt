# A standalone project, intentionally not part of the main build. It is what a
# downstream user writes, and nothing more.

cmake_minimum_required(VERSION 3.25)
project(cpp_library_template_consumer LANGUAGES CXX)

find_package(cpp_library_template REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE cpp_library_template::cpp_library_template)

# Deliberately do not set CMAKE_CXX_STANDARD: the imported target must carry the
# cxx_std_20 requirement itself.
