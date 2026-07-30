# C++ Library Template

[![CI](https://github.com/isaac-cf-wong/cpp-library-template/actions/workflows/ci.yml/badge.svg)](https://github.com/isaac-cf-wong/cpp-library-template/actions/workflows/ci.yml)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/isaac-cf-wong/cpp-library-template/main.svg)](https://results.pre-commit.ci/latest/github/isaac-cf-wong/cpp-library-template/main)
[![Documentation](https://github.com/isaac-cf-wong/cpp-library-template/actions/workflows/documentation.yml/badge.svg)](https://isaac-cf-wong.github.io/cpp-library-template)
[![codecov](https://codecov.io/gh/isaac-cf-wong/cpp-library-template/graph/badge.svg?token=XEF20VWRAJ)](https://codecov.io/gh/isaac-cf-wong/cpp-library-template)
[![CodeQL](https://github.com/isaac-cf-wong/cpp-library-template/actions/workflows/codeql.yml/badge.svg)](https://github.com/isaac-cf-wong/cpp-library-template/actions/workflows/codeql.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.25%2B-064F8C.svg?logo=cmake&logoColor=white)](https://cmake.org)
[![Conan](https://img.shields.io/badge/Conan-2-6699cb.svg)](https://conan.io)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](LICENSE)
[![clang-format](https://img.shields.io/badge/code%20style-clang--format-black.svg)](.clang-format)

A GitHub template repository for a C++ library: a CMake build that installs and
exports properly, a GoogleTest suite, sanitizer and coverage builds, a Conan 2
recipe, and a release pipeline that derives every version number from git tags.

## Getting started

Click **Use this template** on GitHub, clone your new repository, and run
`./setup_repo.sh` to rename the library after your project.

Then build and test:

```bash
cmake --workflow --preset dev
```

Read `docs/template_documentation/` for the full tour: how the build is put
together, what each CI workflow does, and how releases and publishing work. When
you no longer need it, delete that directory and remove the **Template
documentation** entry from `nav` in `zensical.toml`.

Your library lives in `include/` and `src/`.

## What you get

| Concern             | Tool                                                      |
| ------------------- | --------------------------------------------------------- |
| Build               | CMake 3.25+ with presets, install/export, `find_package`  |
| Version             | Derived from the git tag, never written down in a file    |
| Tests               | GoogleTest + CTest, plus a package consumption test       |
| Coverage            | gcovr, Cobertura XML uploaded to Codecov                  |
| Runtime correctness | ASan, UBSan and TSan builds in CI                         |
| Static analysis     | clang-tidy, CodeQL                                        |
| Format              | clang-format, gersemi, prettier, taplo                    |
| Packaging           | Conan 2 recipe with a test package                        |
| Docs                | Doxygen to Markdown to Zensical, deployed to GitHub Pages |
| Releases            | Conventional commits, git-cliff, automated weekly tagging |

## Requirements

- A C++20 compiler.
    - **Pinned in CI:** GCC 13 and Clang 16. These are tested on every push,
      against a pinned runner image.
    - **Expected to work, not pinned:** AppleClang 15 and MSVC 19.38 (Visual
      Studio 2022 17.8). The GitHub runners do not offer older toolchains, so
      the macOS and Windows jobs test whatever the current images ship. Treat
      these two numbers as intent rather than a guarantee.
- CMake 3.25 or newer, and a generator. Ninja is what the presets use.
- Optional: Conan 2, `gcovr`, `doxygen`, `clang-tidy`, `prek`.
