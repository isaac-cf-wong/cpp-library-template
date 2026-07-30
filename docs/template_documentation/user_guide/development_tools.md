# Development tools

## One concern, one tool

| Concern                    | Tool            | Config                   |
| -------------------------- | --------------- | ------------------------ |
| C++ formatting             | `clang-format`  | `.clang-format`          |
| C++ lint, naming, security | `clang-tidy`    | `.clang-tidy`            |
| CMake formatting           | `gersemi`       | `.gersemirc`             |
| Markdown, YAML, JSON       | `prettier`      | `.prettierrc`            |
| Markdown rules             | `markdownlint`  | `.markdownlint.yaml`     |
| TOML formatting            | `taplo`         | -- (defaults)            |
| Python (the few files)     | `ruff`          | `ruff.toml`              |
| Shell                      | `shellcheck`    | -- (defaults)            |
| Spelling                   | `typos`         | `.typos.toml`            |
| Secrets                    | `gitleaks`      | -- (defaults)            |
| Workflow syntax            | `actionlint`    | -- (defaults)            |
| Runtime correctness        | ASan/UBSan/TSan | `cmake/Sanitizers.cmake` |

## The hooks

```bash
pipx install prek
prek install
prek run --all-files
```

`prek` is a drop-in reimplementation of `pre-commit` in Rust; `pre-commit`
itself works identically. On pull requests, `pre-commit.ci` runs the same set.

`clang-tidy` is deliberately **not** a hook. It needs `compile_commands.json`
and a configured build; a hook that takes thirty seconds is a hook people
bypass. It runs in CI instead.

There is one local hook, `scripts/check_headers.sh`, which compiles each public
header on its own. A header that only works because something else was included
first is fine here and broken for consumers. If no build tree exists yet the
hook says so and passes rather than failing your commit for an unrelated reason.

## clang-format

The line limit is 120, matching `markdownlint`. Prose in Markdown is wrapped at
80 by prettier; code is not prose.

Include ordering is by category: the header this file implements, then the
standard library, then third party, then ours. Add a new category in
`IncludeCategories` if you take on a dependency with a distinctive include
prefix.

```bash
clang-format -i src/*.cpp include/cpp_library_template/*.hpp
```

## clang-tidy

The check list is broad -- `bugprone`, `cert`, `clang-analyzer`, `concurrency`,
`cppcoreguidelines`, `google`, `hicpp`, `misc`, `modernize`, `performance`,
`portability`, `readability` -- with individually justified exclusions. Every
exclusion in `.clang-tidy` has a comment saying why; if you add one, do the
same. An unexplained exclusion is indistinguishable from a mistake six months
later.

`cert-*` and `bugprone-*` are what stands in for `bandit` in a Python project.

Run it:

```bash
cmake --preset dev
cmake --build --preset dev              # generated headers must exist first
clang-tidy -p build/dev src/*.cpp apps/*.cpp tests/*.cpp
```

!!! warning "Use a clang build tree"

    `clang-tidy` re-parses each command from `compile_commands.json` with clang.
    A GCC build tree contains GCC-only flags such as `-Wduplicated-cond`, which
    clang rejects outright, and you get a wall of
    `unknown warning option` errors instead of findings. Configure with
    `CC=clang CXX=clang++` for this. CI does exactly that.

Naming rules are enforced: `snake_case` functions and variables, `CamelCase`
types, `kCamelCase` constants, trailing underscore on private and protected
members.

## The sanitizers

```bash
cmake --preset asan-ubsan && ctest --preset asan-ubsan
cmake --preset tsan       && ctest --preset tsan
```

These find bugs static analysis cannot: use-after-free, buffer overflow,
undefined behaviour on overflow, data races. They only report what the tests
actually run, so their value is proportional to your coverage.

!!! note "If TSan aborts immediately"

    `FATAL: ThreadSanitizer: unexpected memory mapping` means the kernel
    randomises more of the address space than TSan's shadow mapping allows. It is
    an environment problem, not your code:

    ```bash
    sudo sysctl -w vm.mmap_rnd_bits=28
    ```

    CI does this too. Without root, `setarch -R ctest ...` works.

## typos

Three escape hatches, all comment-syntax agnostic so they work in C++, CMake and
Markdown alike:

```cpp
// typos:off
const char* kOcurrance = "an intentional misspelling in a wire format";
// typos:on

auto ocurrance = 1;  // typos:disable-line

// typos:ignore-next-line
auto anothr = 2;
```

Prefer adding a genuine word to `[default.extend-words]` in `.typos.toml` over a
broad ignore.

## Conventional commits

The hooks do not check commit messages, and CI does check pull request titles,
via `.github/workflows/semantic_pull_request.yml`. That is where it matters:
pull requests are squash merged, so the title becomes the commit on `main`, and
`git-cliff` reads those commits to decide the next version.

See [Changelog](../development/changelog.md).
