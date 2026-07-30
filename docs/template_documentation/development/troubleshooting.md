# Troubleshooting

## Configure

### `no git tag found, falling back to version 0.0.0`

Expected in a fresh template: there is no release yet. It becomes a real problem
when it happens in CI, where it means the checkout has no tags:

```yaml
- uses: actions/checkout@...
  with:
      fetch-depth: 0
```

For a build with no git history at all -- a source archive, a vendored copy:

```bash
cmake -B build -DCPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE=1.2.3
# or put the version in a version.txt at the project root
```

### `CMake 3.25 or higher is required`

`CMakePresets.json` uses schema version 6. Upgrade CMake, or
`pipx install cmake`.

### `Could not find a package configuration file provided by "GTest"`

Not an error. The message right after it is `GTest not found, fetching it`, and
FetchContent takes over. If the fetch itself fails you have no network, or git
cannot reach GitHub.

### `install(EXPORT ...) includes target ... which requires target ... that is not in any export set`

You linked an interface target without wrapping it:

```cmake
# wrong
target_link_libraries(mylib PRIVATE cpp_library_template_warnings)
# right
target_link_libraries(mylib PRIVATE $<BUILD_INTERFACE:cpp_library_template_warnings>)
```

Even a `PRIVATE` link is recorded in `INTERFACE_LINK_LIBRARIES` for a static
library, so CMake insists the target be exportable. `$<BUILD_INTERFACE:...>`
keeps it out of the export.

## Build

### Warnings became errors and now I cannot build

The `dev` and `ci` presets set `CPP_LIBRARY_TEMPLATE_WARNINGS_AS_ERRORS=ON` on
purpose. To get moving:

```bash
cmake --preset release          # warnings are not fatal here
```

Then fix it. If it is genuinely a false positive, silence it for one file:

```cmake
set_source_files_properties(awkward.cpp PROPERTIES COMPILE_OPTIONS "-Wno-conversion")
```

### `ignoring return value of ... declared with attribute 'nodiscard'`

Bind the value instead of discarding it. In a test:

```cpp
EXPECT_THROW(
    {
        [[maybe_unused]] const auto value = might_throw();
    },
    std::invalid_argument
);
```

### Undefined reference to something that exists

In a shared build, this means a missing export annotation:

```cpp
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string hello(std::string_view name);
```

Symbols are hidden by default. It links in a static build and fails in the
shared one, which is why CI covers both.

### It compiles on Linux and not on macOS or Windows

The usual causes, in order of likelihood:

- A C++20 library feature your other compilers do not have yet. `std::format`
  and `std::chrono::zoned_time` are the common traps -- AppleClang lagged on
  both. `src/log.cpp` uses `std::put_time` and a platform-conditional `gmtime_r`
  / `gmtime_s` for exactly this reason.
- A POSIX function on Windows. Guard it with `#ifdef _WIN32`.
- Case-sensitive versus case-insensitive include paths.

## Test

### `package_consumption` fails but the unit tests pass

That is the test doing its job. Read its output: it prints the configure or
build log from the consumer project. The usual causes:

- A header not listed in a `FILE_SET HEADERS` in `src/CMakeLists.txt`
- A missing export annotation
- A dependency in a public header without a `find_dependency` in
  `cmake/cpp_library_templateConfig.cmake.in`

### `package_consumption` fails with `undefined reference to __gcov_init`

You are in a coverage build. An instrumented library cannot be linked by an
uninstrumented consumer. The test is meant to skip itself there -- if it does
not, `CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE` was not set at configure time, only
later. Reconfigure with the `coverage` preset.

### `FATAL: ThreadSanitizer: unexpected memory mapping`

An environment problem, not your code. The kernel randomises more of the address
space than TSan's shadow mapping allows:

```bash
sudo sysctl -w vm.mmap_rnd_bits=28
setarch -R ctest --preset tsan     # without root
```

### The coverage report says 0%

`All coverage data is filtered out` means gcovr found the data and then
discarded it, because its `--filter` paths do not match the paths gcov recorded.
This happens when the source tree is reached through a symlink.
`cmake/Coverage.cmake` resolves the path with `file(REAL_PATH ...)` to avoid it;
if you have added filters of your own, use the same resolved variable.

### A test passes alone and fails in the suite

Shared state. `ctest` runs tests in separate processes by default, so this
points at something on disk -- a fixed temporary filename, most often. Use a
unique directory per test, as `LoggerFileSinkTest` does.

## clang-tidy

### `unknown warning option '-Wduplicated-cond'`

Your compile database came from GCC and clang-tidy parses with clang. Configure
a separate tree:

```bash
CC=clang CXX=clang++ cmake -S . -B build/tidy -G Ninja
cmake --build build/tidy
clang-tidy -p build/tidy src/*.cpp
```

### Thousands of warnings about GoogleTest

`HeaderFilterRegex` is too loose. A pattern like `include/.*\.h` also matches
the vendored `build/*/_deps/.../include/gmock/*.h`. The one in `.clang-tidy`
requires `include/cpp_library_template` specifically.

Also: `run-clang-tidy` treats its positional arguments as regexes over the
compile database, and with **no** arguments it analyses everything. If you pass
`$(git ls-files ...)` and the files are untracked, you silently pass nothing and
get the whole database, GoogleTest included.

### `file not found: cpp_library_template/export.hpp`

Build before analysing. `export.hpp` and `version.hpp` are generated.

## Documentation

### `no documented namespaces or classes found in the Doxygen XML`

`EXTRACT_ALL = NO`, so undocumented symbols do not appear at all. Either the
comments are missing, or `INPUT` points at the wrong directory.

### `No system Python installation found for Python 3.x.y`

The documentation workflow's tooling step. `astral-sh/setup-uv` with a
`python-version` installs a **uv-managed** interpreter and points `UV_PYTHON` at
it; `uv pip install --system` then looks for that exact version installed on the
runner and does not find it. The two options contradict each other.

Install into a virtual environment instead, which is what the workflow now does:

```yaml
- run: |
      uv venv
      uv pip install -r tools/docs/requirements.txt
      echo "$PWD/.venv/bin" >> "$GITHUB_PATH"
```

### The docs build fails in CI and passes locally

By design. `tools/docs/CMakeLists.txt` sets `WARN_AS_ERROR = FAIL_ON_WARNINGS`
when `$CI` is set. Reproduce it:

```bash
CI=1 cmake --preset dev -DCPP_LIBRARY_TEMPLATE_BUILD_DOCS=ON
cmake --build build/dev --target docs
cat build/dev/docs/doxygen/doxygen_warnings.log
```

Usually a new public function without an `@param` for each parameter.

### Pages does not update

**Settings -> Pages -> Source** must be **GitHub Actions**. Then check whether
the `github-pages` environment has a protection rule waiting on a review.

## Hooks

### `check_headers: no configured build tree found, skipping`

Working as intended. The hook needs the generated headers. Run
`cmake --preset dev` once.

### clang-format keeps reformatting my code

That is the point. Configure your editor to format on save with the project's
`.clang-format`, and stop fighting it.

### A hook modified files and the commit failed

Standard `pre-commit` behaviour: the fixes are applied but unstaged.
`git add -u` and commit again.

## Conan

### `conan create` fails with a missing binary

```bash
conan create . --build=missing
```

Without it Conan refuses to build a dependency from source.

### The Conan package version and the compiled version disagree

`export()` writes a `version.txt` and `generate()` passes it to CMake as
`CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE`. If you have edited either, they can
drift apart. `test_package/main.cpp` compares the header and library versions,
which is the check that catches it.

## Release

### `create_tag` exits 1 with "Tag already exists"

The computed version is already released, meaning nothing releasable landed. The
guard is doing its job.

### The release notes are missing a commit

See [Changelog](changelog.md#troubleshooting). Unconventional commits go to **💼
Other** rather than disappearing, so look there first.

### The links in the release notes point at the template

The `postprocessors` entry in `cliff.toml` was not rewritten. Run
`setup_repo.sh`, or edit it directly.

## Getting help

- [Discussions](https://github.com/isaac-cf-wong/cpp-library-template/discussions)
- [Issues](https://github.com/isaac-cf-wong/cpp-library-template/issues)
- [SUPPORT.md](https://github.com/isaac-cf-wong/cpp-library-template/blob/main/SUPPORT.md)
