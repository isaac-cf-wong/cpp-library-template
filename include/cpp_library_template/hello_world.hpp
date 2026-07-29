/// @file
/// @brief Greeting helpers used as the worked example of this template.
///
/// The functions here are deliberately trivial. They exist to demonstrate the
/// conventions the template enforces: a documented public header, symbols
/// exported through the generated export macro, `std::string_view` for
/// non-owning string parameters, and an injectable output stream so the
/// behaviour is testable without capturing `stdout`.

#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "cpp_library_template/export.hpp"

namespace cpp_library_template {

/// @brief Build a greeting for @p name.
///
/// @param name Name of the person to greet. An empty name yields a greeting
///             addressed to `world`.
/// @return The greeting, without a trailing newline.
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string hello(std::string_view name);

/// @brief Build a farewell for @p name.
///
/// @param name Name of the person to say goodbye to. An empty name yields a
///             farewell addressed to `world`.
/// @return The farewell, without a trailing newline.
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string goodbye(std::string_view name);

/// @brief Write `hello(name)` followed by a newline to @p out.
///
/// @param out  Stream to write to.
/// @param name Name of the person to greet.
CPP_LIBRARY_TEMPLATE_EXPORT void say_hello(std::ostream& out, std::string_view name);

/// @brief Write `hello(name)` followed by a newline to `std::cout`.
///
/// @param name Name of the person to greet.
CPP_LIBRARY_TEMPLATE_EXPORT void say_hello(std::string_view name);

/// @brief Write `goodbye(name)` followed by a newline to @p out.
///
/// @param out  Stream to write to.
/// @param name Name of the person to say goodbye to.
CPP_LIBRARY_TEMPLATE_EXPORT void say_goodbye(std::ostream& out, std::string_view name);

/// @brief Write `goodbye(name)` followed by a newline to `std::cout`.
///
/// @param name Name of the person to say goodbye to.
CPP_LIBRARY_TEMPLATE_EXPORT void say_goodbye(std::string_view name);

}  // namespace cpp_library_template
