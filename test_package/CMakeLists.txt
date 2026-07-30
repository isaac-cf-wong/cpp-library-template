cmake_minimum_required(VERSION 3.25)
project(cpp_library_template_test_package LANGUAGES CXX)

find_package(cpp_library_template CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE cpp_library_template::cpp_library_template)
