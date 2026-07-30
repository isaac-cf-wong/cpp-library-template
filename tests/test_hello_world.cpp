/// @file
/// @brief Tests for the greeting helpers.

#include "cpp_library_template/hello_world.hpp"

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

namespace {

TEST(HelloWorld, HelloGreetsTheGivenName) {
    EXPECT_EQ(cpp_library_template::hello("developer"), "Hello, developer!");
}

TEST(HelloWorld, HelloFallsBackToWorld) {
    EXPECT_EQ(cpp_library_template::hello(""), "Hello, world!");
}

TEST(HelloWorld, GoodbyeGreetsTheGivenName) {
    EXPECT_EQ(cpp_library_template::goodbye("developer"), "Goodbye, developer!");
}

TEST(HelloWorld, GoodbyeFallsBackToWorld) {
    EXPECT_EQ(cpp_library_template::goodbye(""), "Goodbye, world!");
}

TEST(HelloWorld, SayHelloWritesALineToTheStream) {
    std::ostringstream out;
    cpp_library_template::say_hello(out, "developer");
    EXPECT_EQ(out.str(), "Hello, developer!\n");
}

TEST(HelloWorld, SayGoodbyeWritesALineToTheStream) {
    std::ostringstream out;
    cpp_library_template::say_goodbye(out, "developer");
    EXPECT_EQ(out.str(), "Goodbye, developer!\n");
}

// The stdout-writing overloads are the ones a user calls interactively, so
// check they route through the same formatting rather than trusting they do.
TEST(HelloWorld, StdoutOverloadsMatchTheStreamOverloads) {
    testing::internal::CaptureStdout();
    cpp_library_template::say_hello("developer");
    cpp_library_template::say_goodbye("developer");
    const std::string captured = testing::internal::GetCapturedStdout();

    EXPECT_EQ(captured, "Hello, developer!\nGoodbye, developer!\n");
}

// A parameterised test, the analogue of pytest.mark.parametrize.
class HelloWorldNames : public testing::TestWithParam<std::pair<std::string_view, std::string_view>> {};

TEST_P(HelloWorldNames, HelloProducesTheExpectedGreeting) {
    const auto& [name, expected] = GetParam();
    EXPECT_EQ(cpp_library_template::hello(name), expected);
}

INSTANTIATE_TEST_SUITE_P(Names,
                         HelloWorldNames,
                         testing::Values(std::pair<std::string_view, std::string_view>{"", "Hello, world!"},
                                         std::pair<std::string_view, std::string_view>{"world", "Hello, world!"},
                                         std::pair<std::string_view, std::string_view>{"Isaac", "Hello, Isaac!"},
                                         std::pair<std::string_view, std::string_view>{"a b", "Hello, a b!"}));

}  // namespace
