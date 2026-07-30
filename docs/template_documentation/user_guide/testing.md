# Testing

GoogleTest for the tests, CTest to run them, gcovr for coverage.

## Running them

```bash
ctest --preset dev                       # unit tests only
ctest --test-dir build/dev               # everything
ctest --test-dir build/dev -R Logger     # by name
ctest --test-dir build/dev -L unit       # by label
ctest --test-dir build/dev -LE integration
ctest --test-dir build/dev -j8           # in parallel
ctest --test-dir build/dev --rerun-failed
```

Or run the binary directly, which is usually faster to iterate on:

```bash
./build/dev/bin/cpp_library_template_tests
./build/dev/bin/cpp_library_template_tests --gtest_filter='Logger*'
./build/dev/bin/cpp_library_template_tests --gtest_repeat=100 --gtest_shuffle
./build/dev/bin/cpp_library_template_tests --gtest_break_on_failure
```

## Labels

| Label         | What                                                                 |
| ------------- | -------------------------------------------------------------------- |
| `unit`        | Everything in the GoogleTest binary. Fast, no I/O beyond temp files. |
| `integration` | `package_consumption`, which shells out to CMake and a compiler.     |

This is the analogue of pytest markers, and `ctest -LE integration` is the
analogue of `-m 'not integration'`. The `dev` test preset applies it for you.

## Where GoogleTest comes from

`tests/CMakeLists.txt` tries `find_package(GTest)` first, and falls back to
FetchContent:

```cmake
find_package(GTest 1.15 QUIET)

if(NOT GTest_FOUND)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0 # renovate: datasource=github-tags depName=google/googletest
        ...
    )
endif()
```

So a clean machine with no package manager still builds and tests, while Conan
or vcpkg or a distro package is used when present. The `# renovate:` comment is
what lets Renovate keep this pin in step with the `gtest` requirement in
`conanfile.py`: two places encoding one version is a drift waiting to happen, so
they are updated by the same rule, in the same pull request.

## Writing a test

```cpp
#include "cpp_library_template/hello_world.hpp"

#include <gtest/gtest.h>

namespace {

TEST(HelloWorld, HelloGreetsTheGivenName) {
    EXPECT_EQ(cpp_library_template::hello("developer"), "Hello, developer!");
}

}  // namespace
```

Conventions in this project:

- One file per header under test: `test_<header>.cpp`.
- Everything in an anonymous namespace.
- `TEST(SuiteName, DescribesTheBehaviour)` -- the test name is a sentence about
  behaviour, not `Test1`.
- `EXPECT_` to continue after a failure, `ASSERT_` to stop. Use `ASSERT_` when
  everything after it would be meaningless.

Add the file to `tests/CMakeLists.txt`.

## Fixtures

The equivalent of a pytest fixture:

```cpp
class LoggerTest : public testing::Test {
protected:
    std::ostringstream sink_;
    cpp_library_template::Logger logger_{"test", sink_, cpp_library_template::LogLevel::info};

    [[nodiscard]] std::string output() const { return sink_.str(); }
};

TEST_F(LoggerTest, RecordsBelowTheActiveLevelAreDropped) {
    logger_.debug("dropped");
    logger_.info("kept");
    EXPECT_THAT(output(), testing::Not(testing::HasSubstr("dropped")));
}
```

Each test gets a fresh instance. Use `SetUp()` and `TearDown()` when
construction is not enough -- creating and removing a temporary directory, for
instance, as `LoggerFileSinkTest` does.

## Parameterised tests

The equivalent of `pytest.mark.parametrize`:

```cpp
class HelloWorldNames : public testing::TestWithParam<std::pair<std::string_view, std::string_view>> {};

TEST_P(HelloWorldNames, HelloProducesTheExpectedGreeting) {
    const auto& [name, expected] = GetParam();
    EXPECT_EQ(cpp_library_template::hello(name), expected);
}

INSTANTIATE_TEST_SUITE_P(
    Names,
    HelloWorldNames,
    testing::Values(
        std::pair<std::string_view, std::string_view>{"", "Hello, world!"},
        std::pair<std::string_view, std::string_view>{"Isaac", "Hello, Isaac!"}
    )
);
```

Each case becomes a separate CTest entry, so a failure names the input.

## Matchers

gmock's matchers are available in any test, without mocking anything:

```cpp
EXPECT_THAT(output(), testing::HasSubstr("| warning |"));
EXPECT_THAT(values, testing::ElementsAre(1, 2, 3));
EXPECT_THAT(ptr, testing::NotNull());
```

!!! warning "Avoid `MatchesRegex`"

    Its flavour depends on the platform. gmock's own simple regex supports `\d`
    but not `{n}`; POSIX supports `{n}` but not `\d`. A regex that passes on your
    machine can fail on another. Parse the value, or write a custom
    `::testing::AssertionResult` helper -- `tests/test_log.cpp` has one for
    checking a timestamp.

## Testing something that writes to stdout

Prefer taking a `std::ostream&`, which is why `say_hello` has that overload -- a
testable seam beats capturing a global. When you have no choice:

```cpp
testing::internal::CaptureStdout();
cpp_library_template::say_hello("developer");
EXPECT_EQ(testing::internal::GetCapturedStdout(), "Hello, developer!\n");
```

## The package consumption test

`tests/package_test/` is the most valuable test here and the one people leave
out. It installs the project into a throwaway prefix, then configures, builds
and runs a **separate** CMake project that finds it with `find_package`.

It catches what unit tests cannot see:

- A header that is not in the install list
- A missing `CPP_LIBRARY_TEMPLATE_EXPORT`
- A broken package config file
- A dependency missing a `find_dependency()` call
- An imported target that does not carry its C++ standard requirement

It skips itself in coverage and sanitizer builds, because an instrumented
library cannot be linked by an uninstrumented consumer. `test_package/` does the
same job for Conan consumers.

## Coverage

```bash
cmake --workflow --preset coverage
cmake --build --preset coverage --target coverage
open build/coverage/coverage/index.html
```

`coverage.xml` is Cobertura, which Codecov reads directly. There is no minimum
threshold configured -- that is your call to make, in `.codecov.yml` or by
adding `--fail-under-line` to the gcovr arguments in `cmake/Coverage.cmake`.

Branch coverage is worth looking at rather than line coverage. Lines are easy to
hit; branches are where the bugs are.

## In CI

Every push and pull request runs the matrix: Linux with GCC and Clang, macOS
with AppleClang, Windows with MSVC, static and shared, plus the oldest supported
compiler. Then coverage, the sanitizers, clang-tidy, CodeQL and `conan create`.

See [CI/CD](ci_cd.md).
