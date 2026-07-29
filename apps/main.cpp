/// @file
/// @brief Example command line front end for the library.
///
/// Argument parsing is hand-rolled on purpose: the template ships with zero
/// required runtime dependencies. If you want a real parser, add CLI11 to
/// `conanfile.py` and replace this file -- the surrounding CMake, install rules
/// and smoke test do not care.

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "cpp_library_template/cpp_library_template.hpp"

namespace {

constexpr int kExitOk = 0;
constexpr int kExitUsage = 2;

void print_usage(std::ostream& out) {
    out << "usage: cpp_library_template [OPTIONS] <command> [ARGS]\n"
        << '\n'
        << "Commands:\n"
        << "  hello [NAME]      Greet NAME (default: world)\n"
        << "  goodbye [NAME]    Say goodbye to NAME (default: world)\n"
        << '\n'
        << "Options:\n"
        << "  -v, --verbose LEVEL   Log level: trace, debug, info, warning,\n"
        << "                        error, critical, off (default: warning)\n"
        << "  -h, --help            Show this message and exit\n"
        << "  -V, --version         Show the version and exit\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const std::vector<std::string_view> args{argv + 1, argv + argc};

        auto level = cpp_library_template::LogLevel::warning;
        std::vector<std::string_view> positional;

        for (std::size_t i = 0; i < args.size(); ++i) {
            const std::string_view arg = args[i];

            if (arg == "-h" || arg == "--help") {
                print_usage(std::cout);
                return kExitOk;
            }
            if (arg == "-V" || arg == "--version") {
                std::cout << cpp_library_template::compiled_version_full() << '\n';
                return kExitOk;
            }
            if (arg == "-v" || arg == "--verbose") {
                if (i + 1 >= args.size()) {
                    std::cerr << "error: " << arg << " requires a level\n";
                    return kExitUsage;
                }
                level = cpp_library_template::level_from_string(args[++i]);
                continue;
            }
            if (arg.starts_with('-')) {
                std::cerr << "error: unknown option '" << arg << "'\n";
                print_usage(std::cerr);
                return kExitUsage;
            }
            positional.push_back(arg);
        }

        if (positional.empty()) {
            print_usage(std::cerr);
            return kExitUsage;
        }

        cpp_library_template::Logger logger{"cpp_library_template", level};
        logger.log_version_information();

        const std::string_view command = positional.front();
        const std::string_view name = positional.size() > 1 ? positional[1] : std::string_view{};

        if (command == "hello") {
            logger.debug("greeting");
            cpp_library_template::say_hello(std::cout, name);
            return kExitOk;
        }
        if (command == "goodbye") {
            logger.debug("saying goodbye");
            cpp_library_template::say_goodbye(std::cout, name);
            return kExitOk;
        }

        std::cerr << "error: unknown command '" << command << "'\n";
        print_usage(std::cerr);
        return kExitUsage;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
