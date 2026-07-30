/// @file
/// @brief Consumes the Conan package exactly as a downstream user would.

#include <iostream>

#include <cpp_library_template/cpp_library_template.hpp>

int main() {
    std::cout << "cpp_library_template " << cpp_library_template::compiled_version_full() << '\n';

    if (cpp_library_template::compiled_version() != cpp_library_template::version) {
        std::cerr << "header/library version mismatch\n";
        return 1;
    }

    cpp_library_template::say_hello(std::cout, "conan");
    return 0;
}
