/// @file
/// @brief Consumes the installed package exactly as a downstream user would.

#include <iostream>

#include <cpp_library_template/cpp_library_template.hpp>

int main() {
    std::cout << "linked version: " << cpp_library_template::compiled_version_full() << '\n';
    std::cout << "header version: " << cpp_library_template::version_full << '\n';

    if (cpp_library_template::compiled_version() != cpp_library_template::version) {
        std::cerr << "header/library version mismatch\n";
        return 1;
    }

    cpp_library_template::say_hello(std::cout, "template");

    cpp_library_template::Logger logger{"consumer", std::cerr, cpp_library_template::LogLevel::debug};
    logger.info("the installed package works");

    return 0;
}
