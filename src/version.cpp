/// @file
/// @brief Runtime accessors for the version baked in at configure time.
///
/// `version.hpp` exposes the version as `constexpr` values, which are inlined
/// into every consumer. These functions read the version compiled into the
/// library binary instead, so a program can detect a header/library mismatch
/// after a shared library was swapped underneath it.

#include "cpp_library_template/version.hpp"

namespace cpp_library_template {

std::string_view compiled_version() noexcept {
    return version;
}

std::string_view compiled_version_full() noexcept {
    return version_full;
}

}  // namespace cpp_library_template
