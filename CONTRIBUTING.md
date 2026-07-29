# Contributing

Thanks for considering a contribution to **cpp-library-template**.

## Getting set up

You need a C++20 compiler, CMake 3.25 or newer, and a generator. The presets use
Ninja.

```bash
git clone https://github.com/isaac-cf-wong/cpp-library-template.git
cd cpp-library-template
cmake --workflow --preset dev
```

That configures, builds and runs the unit tests. GoogleTest is fetched
automatically if it is not already installed, so no package manager is required.

Install the git hooks, which run the formatters and the linters:

```bash
pipx install prek     # or: pipx install pre-commit
prek install
prek run --all-files
```

Optional tools, each unlocking one part of the workflow:

| Tool         | Needed for                                |
| ------------ | ----------------------------------------- |
| `gcovr`      | `cmake --workflow --preset coverage`      |
| `doxygen`    | the `docs` target                         |
| `clang-tidy` | running the static analysis locally       |
| `conan`      | building the package, adding dependencies |

## The development loop

```bash
cmake --build --preset dev        # build
ctest --preset dev                # unit tests, skipping the slow package test
ctest --test-dir build/dev        # everything, including the package test
```

Before opening a pull request:

```bash
prek run --all-files                                    # format and lint
cmake --workflow --preset coverage                      # coverage report
cmake --preset asan-ubsan && ctest --preset asan-ubsan  # sanitizers
```

## Coding conventions

- **Formatting is not a matter of taste.** `clang-format` decides, and the hook
  applies it. Same for CMake (`gersemi`), Markdown and YAML (`prettier`), and
  TOML (`taplo`).
- **Every public declaration carries a Doxygen comment**, with `@param`,
  `@return` and `@throws` where they apply. This is enforced: the documentation
  build turns an undocumented public symbol into an error.
- **Public headers must be self-contained.** A hook compiles each one on its
  own.
- **Names**: `snake_case` for functions and variables, `CamelCase` for types,
  `kCamelCase` for constants, a trailing underscore on private members.
  `clang-tidy` enforces this.
- **Anything exported must be annotated** with `CPP_LIBRARY_TEMPLATE_EXPORT`.
  The build hides symbols by default, so an unannotated symbol simply will not
  be there for consumers of the shared library.
- **Tests come with the change.** A bug fix should include a test that fails
  before it and passes after.

## Commit messages

This project uses [Conventional Commits](https://www.conventionalcommits.org/).
This is not decoration: `git-cliff` reads these messages to decide the next
version number and to write the release notes. A `feat:` bumps the minor
version, a `fix:` the patch version, and a `!` or a `BREAKING CHANGE:` footer
bumps the major version.

Allowed types: `build`, `chore`, `ci`, `docs`, `feat`, `fix`, `perf`,
`refactor`, `style`, `test`.

```text
feat(log): add a rotating file sink
fix(hello): handle an empty name without allocating
docs: explain the export macro
refactor!: take std::span instead of a pointer and a length
```

Bad:

```text
updates
fixed stuff
WIP
```

**Your pull request title must also be a conventional commit.** Pull requests
are squash merged, so the title becomes the commit message on `main`.
`.github/workflows/semantic_pull_request.yml` checks it.

## Pull requests

1. Branch off `main`.
2. Make the change, with tests.
3. Run `prek run --all-files` and the test suite.
4. Open the pull request with a conventional commit title, describing what
   changed and why.

CI will build on Linux, macOS and Windows, with GCC, Clang, AppleClang and MSVC,
static and shared, and run the sanitizers, clang-tidy, CodeQL, coverage and the
Conan recipe. Expect to iterate.

## Reporting bugs and asking questions

See [SUPPORT.md](SUPPORT.md). For anything security related, do **not** open a
public issue -- follow [SECURITY.md](SECURITY.md).

## Licence

By contributing you agree that your contribution is licensed under the 3-Clause
BSD License, the same terms as the rest of the project. See [LICENSE](LICENSE).
Please also read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
