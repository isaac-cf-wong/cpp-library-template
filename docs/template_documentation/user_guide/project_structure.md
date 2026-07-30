# Project structure

```text
cpp-library-template/
├── CMakeLists.txt              The top level build: options, and what to include
├── CMakePresets.json           Named configurations: dev, coverage, asan-ubsan, ci, ...
├── conanfile.py                The Conan recipe: metadata, dependencies, packaging
├── cliff.toml                  git-cliff: how commits become release notes
├── setup_repo.sh               One-shot rename after instantiating the template
│
├── include/
│   └── cpp_library_template/   The public API. This directory *is* the interface.
│       ├── cpp_library_template.hpp   Umbrella header
│       ├── hello_world.hpp
│       └── log.hpp
│
├── src/                        The implementation, plus the source list
│   ├── CMakeLists.txt
│   ├── hello_world.cpp
│   ├── log.cpp
│   └── version.cpp
│
├── apps/                       The example CLI: a consumer of the library
│   ├── CMakeLists.txt
│   └── main.cpp
│
├── tests/
│   ├── CMakeLists.txt          GoogleTest wiring, and the FetchContent fallback
│   ├── test_hello_world.cpp
│   ├── test_log.cpp
│   ├── test_version.cpp
│   └── package_test/           Installs the project and builds a consumer against it
│       ├── CMakeLists.txt
│       ├── run_package_test.cmake
│       └── consumer/
│
├── test_package/               The Conan equivalent of package_test
│
├── cmake/                      The build's own modules
│   ├── GitVersion.cmake        Derives the version from the git tag
│   ├── CompilerWarnings.cmake  The warning set, as one interface target
│   ├── Sanitizers.cmake
│   ├── Coverage.cmake          Instrumentation, and the `coverage` target
│   ├── InstallRules.cmake      install(), export(), the package config
│   ├── version.hpp.in          Template for the generated version header
│   └── cpp_library_templateConfig.cmake.in   Template for the find_package config
│
├── docs/                       Site content only -- everything here is published
│   ├── index.md                A snippet include of README.md
│   ├── api/generated/          Written by the docs target. Git ignored.
│   └── template_documentation/ This section. Delete it when you are done.
│
├── tools/docs/                 The documentation build, kept out of docs/ so it
│   ├── CMakeLists.txt          is not published as part of the site
│   ├── Doxyfile.in             Doxygen, configured to emit XML only
│   ├── gen_api_pages.py        Doxygen XML -> Markdown
│   └── requirements.txt        The Python tooling for the site
│
├── scripts/
│   └── check_headers.sh        Checks each public header compiles standalone
│
└── .github/
    ├── workflows/              CI, release and publishing
    ├── ISSUE_TEMPLATE/         Bug and feature templates, plus the chooser that
    │                           sends questions to Discussions instead
    └── pull_request_template.md
```

## Why `include/` is separate from `src/`

The include directory is the contract. A consumer gets exactly the headers under
`include/`, at exactly the paths they appear there, because that is what the
install rules copy. Anything in `src/` is invisible to them.

The consequence worth internalising: `#include "cpp_library_template/log.hpp"`
works identically inside this project and in a downstream one. There is no
"works locally, breaks after install" gap, which is the failure the
`package_consumption` test exists to catch.

## The generated headers

Two public headers are not in `include/`, because they do not exist until CMake
runs:

| Header                             | Generated from             | Contains                  |
| ---------------------------------- | -------------------------- | ------------------------- |
| `cpp_library_template/export.hpp`  | `generate_export_header()` | The `..._EXPORT` macro    |
| `cpp_library_template/version.hpp` | `cmake/version.hpp.in`     | The version, as constants |

They land in `${PROJECT_BINARY_DIR}/generated/cpp_library_template/` and are
installed alongside the hand-written ones, so a consumer cannot tell the
difference.

## Adding a source file

1. Create `src/thing.cpp` and, if it is public,
   `include/cpp_library_template/thing.hpp`.
2. Add `thing.cpp` to the `add_library` call in `src/CMakeLists.txt`.
3. Add the header to the `FILE_SET HEADERS` list in the same file.
4. Add it to the umbrella header,
   `include/cpp_library_template/cpp_library_template.hpp`.
5. Add `tests/test_thing.cpp` and list it in `tests/CMakeLists.txt`.

Step 3 is the one people forget. The `FILE_SET` is what gets installed; a header
missing from it compiles fine here and is absent for consumers. The
`package_consumption` test will fail, which is the point.

## The `FILE_SET HEADERS` idiom

```cmake
target_sources(
    cpp_library_template
    PUBLIC FILE_SET HEADERS
           BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
           FILES "${PROJECT_SOURCE_DIR}/include/cpp_library_template/log.hpp"
)
```

`BASE_DIRS` becomes the include directory, for both the build and the install,
and `FILES` becomes the install list. One declaration replaces a
`target_include_directories` with `BUILD_INTERFACE`/`INSTALL_INTERFACE` plus a
separate `install(DIRECTORY)`, and it cannot drift out of step with itself.

## Conventions in the code

- One `@file` comment per file, saying what it is for.
- Every public declaration documented, with `@param`, `@return` and `@throws`.
- Everything in `namespace cpp_library_template`. Implementation details go in
  an anonymous namespace in the `.cpp`.
- Everything public annotated with `CPP_LIBRARY_TEMPLATE_EXPORT`.
- `#pragma once`, not include guards.
- Includes in four groups: the header this file implements, the standard
  library, third party, then our own. `clang-format` sorts within each group.
