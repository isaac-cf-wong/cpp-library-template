# Build system

## Options

All of them are prefixed, so they cannot collide with a parent project's
options.

| Option                                    | Default          | What it does                                           |
| ----------------------------------------- | ---------------- | ------------------------------------------------------ |
| `CPP_LIBRARY_TEMPLATE_BUILD_TESTS`        | ON if top level  | Builds the test suite                                  |
| `CPP_LIBRARY_TEMPLATE_BUILD_CLI`          | ON if top level  | Builds the example command line tool                   |
| `CPP_LIBRARY_TEMPLATE_BUILD_DOCS`         | OFF              | Adds the `docs`, `docs-build` and `docs-serve` targets |
| `CPP_LIBRARY_TEMPLATE_INSTALL`            | ON if top level  | Generates the install and export rules                 |
| `CPP_LIBRARY_TEMPLATE_ENABLE_COVERAGE`    | OFF              | Instruments the build, and adds the `coverage` target  |
| `CPP_LIBRARY_TEMPLATE_WARNINGS_AS_ERRORS` | OFF              | Turns compiler warnings into errors                    |
| `CPP_LIBRARY_TEMPLATE_SANITIZERS`         | empty            | A `;`-separated list, e.g. `address;undefined`         |
| `BUILD_SHARED_LIBS`                       | Platform default | Shared or static. Standard CMake.                      |

"ON if top level" is what makes the project safe to consume through
`add_subdirectory` or FetchContent: a downstream build gets the library and
nothing else -- no test targets in their `ctest` output, no install rules
fighting with theirs.

## Presets

```bash
cmake --list-presets
```

| Preset       | For                                                    |
| ------------ | ------------------------------------------------------ |
| `dev`        | Day-to-day work. Debug, warnings as errors.            |
| `release`    | An optimised build with debug info.                    |
| `coverage`   | Debug plus instrumentation, and the `coverage` target. |
| `asan-ubsan` | The address and undefined-behaviour sanitizers.        |
| `tsan`       | The thread sanitizer.                                  |
| `static`     | `BUILD_SHARED_LIBS=OFF`.                               |
| `shared`     | `BUILD_SHARED_LIBS=ON`.                                |
| `ci`         | What CI configures on Linux and macOS.                 |
| `ci-msvc`    | What CI configures on Windows.                         |

!!! note "Why `ci-msvc` does not name a Visual Studio version"

    Every other preset uses Ninja explicitly. `ci-msvc` names no generator at
    all, so CMake picks the newest Visual Studio actually installed.

    This is not an oversight. The preset originally pinned
    `Visual Studio 17 2022`, and the first CI run failed with
    `could not find any instance of Visual Studio` -- because `windows-latest`
    had moved to an image carrying VS 2026 and nothing else. A pinned generator
    in a template is a time bomb set by whoever wrote it and detonated by
    whoever inherits it.

    If you *need* a specific toolset -- for ABI reasons, say -- pin it
    deliberately with `-DCMAKE_GENERATOR_TOOLSET=version=14.38` and pin the
    runner image alongside it, so the two cannot drift apart.

Each preset builds into `build/<preset>/`, so they coexist. There are also
workflow presets, which do configure-build-test in one step:

```bash
cmake --workflow --preset dev
cmake --workflow --preset coverage
```

For your own local settings, create `CMakeUserPresets.json`. It is git ignored
and can `inherit` from any preset here.

## The version

There is no version number anywhere in the source. `cmake/GitVersion.cmake` runs
`git describe --tags --dirty --match 'v[0-9]*'` **before** the `project()` call
and feeds the result in.

Resolution order:

1. `-DCPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE=<version>`. Conan passes this,
   because the Conan cache has no `.git`.
2. A `version.txt` at the project root. The source archives attached to a GitHub
   Release carry one, for the same reason.
3. `git describe`.
4. `0.0.0`, with a warning.

Every path leads back to the same tag. Two forms come out of it:

| Variable                            | Example            | Used for                          |
| ----------------------------------- | ------------------ | --------------------------------- |
| `CPP_LIBRARY_TEMPLATE_VERSION`      | `0.3.1`            | `project(VERSION)`, the SOVERSION |
| `CPP_LIBRARY_TEMPLATE_VERSION_FULL` | `0.3.1-4-gdeadbee` | The version reported at runtime   |

CMake rejects anything that is not strictly `MAJOR.MINOR.PATCH`, which is why
there are two.

In C++ the version is available two ways:

```cpp
#include <cpp_library_template/version.hpp>

cpp_library_template::version;                 // constexpr, inlined into you
cpp_library_template::compiled_version();      // read from the library binary
```

They differ only when the headers and the linked library come from different
builds -- which is exactly the situation you want to detect.

## Symbol visibility

`CMAKE_CXX_VISIBILITY_PRESET hidden` means nothing is exported unless it is
annotated:

```cpp
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string hello(std::string_view name);
```

The macro comes from the generated `cpp_library_template/export.hpp`. It expands
to the right thing for each case: `__attribute__((visibility("default")))`,
`__declspec(dllexport)` while building, `__declspec(dllimport)` when consuming,
and nothing at all in a static build.

This is more work than exporting everything, and it buys three things: a smaller
shared library, faster loading, and -- most usefully -- an ABI you decided on
rather than one that happened. Forgetting the annotation produces a link error
in the shared build, which the CI matrix covers.

## Install and export

```bash
cmake --install build/release --prefix /tmp/prefix
```

```text
/tmp/prefix/
├── bin/cpp_library_template
├── include/cpp_library_template/*.hpp
├── lib/libcpp_library_template.so.0.3.1
└── lib/cmake/cpp_library_template/
    ├── cpp_library_templateConfig.cmake
    ├── cpp_library_templateConfigVersion.cmake
    └── cpp_library_templateTargets.cmake
```

`cpp_library_templateConfig.cmake` is generated from
`cmake/cpp_library_templateConfig.cmake.in`. If you add a dependency that
appears in a public header, add a matching `find_dependency()` call there --
otherwise `find_package(cpp_library_template)` succeeds and the consumer then
fails to compile, which is a confusing way to find out.

Install components are declared, so packagers can split runtime from
development:

```bash
cmake --install build/release --component cpp_library_template_Runtime
cmake --install build/release --component cpp_library_template_Development
```

## Warnings

`cmake/CompilerWarnings.cmake` defines one interface target,
`cpp_library_template_warnings`, and every target we own links it **privately**,
wrapped in `$<BUILD_INTERFACE:...>`. The wrapper matters: without it CMake
refuses to generate the export set, because a target in an export set may not
reference a target that is not exported.

The warnings are private by design. Your compiler settings are not a consumer's
problem.

To make warnings fatal locally, as CI does:

```bash
cmake --preset dev   # already has it
cmake -B build -DCPP_LIBRARY_TEMPLATE_WARNINGS_AS_ERRORS=ON
```

## Sanitizers

```bash
cmake -B build -DCPP_LIBRARY_TEMPLATE_SANITIZERS="address;undefined"
```

Sanitizers must instrument every translation unit, so the flags are applied
directory-wide rather than through a link-time target. The module rejects
combinations that cannot work -- `thread` with `address`, or anything but
`address` on MSVC -- rather than letting the linker produce something confusing.

Note that a sanitizer-instrumented library cannot be linked by an uninstrumented
consumer, so the `package_consumption` test skips itself in those builds. The
same is true of coverage.

## Coverage

```bash
cmake --workflow --preset coverage
cmake --build --preset coverage --target coverage
```

Produces `build/coverage/coverage.xml` (Cobertura, which Codecov reads directly)
and `build/coverage/coverage/index.html`.

`-O0` is forced: with optimisation on, inlining smears line attribution and the
report becomes fiction. The report is filtered to `src/` and `include/`, using a
path resolved with `file(REAL_PATH ...)` -- gcov records fully resolved paths,
so a source tree reached through a symlink would otherwise filter everything out
and report a cheerful 0%.
