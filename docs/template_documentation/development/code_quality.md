# Code quality

## Who owns what

| Concern                        | Tool                       | Where it runs              |
| ------------------------------ | -------------------------- | -------------------------- |
| Formatting, C++                | `clang-format`             | Hook                       |
| Formatting, CMake              | `gersemi`                  | Hook                       |
| Formatting, Markdown/YAML/JSON | `prettier`                 | Hook                       |
| Formatting, TOML               | `taplo`                    | Hook                       |
| Formatting, Python             | `ruff format`              | Hook                       |
| Lint, naming, modernisation    | `clang-tidy`               | CI                         |
| Security patterns              | `clang-tidy` cert/bugprone | CI                         |
| Security, deep                 | CodeQL                     | CI, and weekly             |
| Memory and threading errors    | ASan, UBSan, TSan          | CI                         |
| Compiler warnings              | ~40 flags, fatal in CI     | Every build                |
| Header self-sufficiency        | `scripts/check_headers.sh` | Hook                       |
| Public API documentation       | Doxygen, warnings fatal    | The documentation workflow |
| API stability                  | `package_consumption` test | Every test run             |
| Coverage                       | `gcovr`, Codecov           | CI                         |
| Spelling                       | `typos`                    | Hook                       |
| Secrets                        | `gitleaks`                 | Hook                       |
| Workflow syntax                | `actionlint`               | Hook                       |

Nothing is checked twice by two tools, and each row has exactly one owner. That
is deliberate: two tools with overlapping opinions produce conflicting
autofixes.

## What "quality" means here that is specific to C++

A Python library that imports is mostly usable. A C++ library has several extra
ways to be broken while looking fine:

| Failure                                      | Caught by                                                |
| -------------------------------------------- | -------------------------------------------------------- |
| A header missing from the install list       | `package_consumption` test                               |
| A symbol without an export annotation        | The shared-library CI cells                              |
| A header that needs another one first        | `scripts/check_headers.sh`                               |
| Undefined behaviour that happens to work     | UBSan                                                    |
| A use-after-free on an untested path         | ASan, plus coverage telling you which paths are untested |
| A dependency missing from the package config | `package_consumption` test                               |
| An ABI change without a major version bump   | Nothing automatic. Your discipline.                      |

The last row is honest rather than reassuring. There is no ABI checker here.
`abi-compliance-checker` or `abidiff` would fill the gap, and neither is set up.
If ABI stability matters to your users, add one.

## Coverage

No minimum is enforced. Set one if you want it, either in `.codecov.yml` or by
adding `--fail-under-line` to the gcovr arguments in `cmake/Coverage.cmake`.

Prefer branch coverage as the number you look at. Lines are easy to hit by
accident; branches are where the untested behaviour is. The report includes
decision coverage too, though gcovr describes that analysis as experimental --
treat it as a hint, not a measurement.

## No static type checker, and why that is not the same gap

The Python template has no mypy configuration, which is a real gap there. Here
the compiler is the type checker, it runs on every build, and its warnings are
fatal in CI. `clang-tidy`'s `cppcoreguidelines` and `bugprone` families cover
much of what a separate analyser would.

What is genuinely missing:

- **ABI checking**, as above.
- **Include-what-you-use.** `misc-include-cleaner` is disabled because it
  disagrees with the deliberate use of `<iosfwd>` in the public headers. The
  hook that compiles each header standalone catches the more damaging direction
  -- a header that is missing an include -- but not the unused ones.
- **Fuzzing.** For a library that parses anything, add libFuzzer targets. The
  sanitizer presets are already most of the setup.
