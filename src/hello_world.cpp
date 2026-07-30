#include "cpp_library_template/hello_world.hpp"

#include <iostream>
#include <ostream>
#include <string>

namespace cpp_library_template {
namespace {

/// @brief Fall back to "world" for an empty name.
[[nodiscard]] std::string_view or_world(std::string_view name) noexcept {
    return name.empty() ? std::string_view{"world"} : name;
}

}  // namespace

std::string hello(std::string_view name) {
    return "Hello, " + std::string{or_world(name)} + "!";
}

std::string goodbye(std::string_view name) {
    return "Goodbye, " + std::string{or_world(name)} + "!";
}

void say_hello(std::ostream& out, std::string_view name) {
    out << hello(name) << '\n';
}

void say_hello(std::string_view name) {
    say_hello(std::cout, name);
}

void say_goodbye(std::ostream& out, std::string_view name) {
    out << goodbye(name) << '\n';
}

void say_goodbye(std::string_view name) {
    say_goodbye(std::cout, name);
}

}  // namespace cpp_library_template
