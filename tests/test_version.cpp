/// @file
/// @brief Guards on the version wiring.
///
/// These are the C++ analogue of the Python template's `test_import.py`: cheap
/// checks that the build plumbing produced something sane, so a broken
/// `GitVersion.cmake` fails a test instead of shipping.

#include "cpp_library_template/version.hpp"

#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "cpp_library_template/cpp_library_template.hpp"

namespace {

TEST(Version, NumericVersionMatchesItsComponents) {
    const std::string expected = std::to_string(cpp_library_template::version_major) + "." +
                                 std::to_string(cpp_library_template::version_minor) + "." +
                                 std::to_string(cpp_library_template::version_patch);

    EXPECT_EQ(cpp_library_template::version, expected);
}

TEST(Version, FullVersionStartsWithTheNumericVersion) {
    EXPECT_TRUE(cpp_library_template::version_full.starts_with(cpp_library_template::version))
        << "full='" << cpp_library_template::version_full << "' numeric='" << cpp_library_template::version << "'";
}

// If these differ, the headers and the linked library came from different
// builds. In-tree that is impossible; against an installed package it is the
// failure mode this catches.
TEST(Version, LibraryAndHeadersAgree) {
    EXPECT_EQ(cpp_library_template::compiled_version(), cpp_library_template::version);
    EXPECT_EQ(cpp_library_template::compiled_version_full(), cpp_library_template::version_full);
}

TEST(Version, FullVersionCarriesGitInformationWhenATagExists) {
    // 0.0.0 is the GitVersion.cmake fallback. A freshly instantiated template
    // has no tag yet, so this is a skip rather than a failure -- but once the
    // first release tag exists, a regression in the version plumbing shows up
    // here instead of shipping.
    if (cpp_library_template::version == std::string_view{"0.0.0"}) {
        GTEST_SKIP() << "no git tag found at configure time; this is expected before the first release. "
                        "In CI make sure the checkout uses fetch-depth: 0.";
    }

    EXPECT_FALSE(cpp_library_template::version_full.empty());
}

// The umbrella header must be self-sufficient: this file includes it and uses
// one symbol from each public header, so a missing include in it fails to
// compile.
TEST(UmbrellaHeader, ExposesTheWholePublicApi) {
    EXPECT_EQ(cpp_library_template::hello("x"), "Hello, x!");
    EXPECT_EQ(cpp_library_template::to_string(cpp_library_template::LogLevel::info), "info");
    EXPECT_FALSE(cpp_library_template::version.empty());
}

}  // namespace
