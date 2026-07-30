# Customization

Ongoing changes, once the template is yours.

## Adding a dependency

See [Packaging](packaging.md#adding-a-dependency). The short version: declare it
in `conanfile.py`, find and link it in CMake, and if it appears in a public
header link it `PUBLIC` **and** add a `find_dependency` to
`cmake/cpp_library_templateConfig.cmake.in`.

## Changing the C++ standard

Three places, and all three must agree:

```cmake
# CMakeLists.txt
set(CMAKE_CXX_STANDARD 23)

# src/CMakeLists.txt -- what consumers are required to use
target_compile_features(cpp_library_template PUBLIC cxx_std_23)
```

```python
# conanfile.py
check_min_cppstd(self, 23)
```

Then update `.clang-format` (`Standard: c++23`), the badge in `README.md`, and
the compiler floor in the CI matrix -- a newer standard means newer minimum
compilers.

## Adjusting the warnings

`cmake/CompilerWarnings.cmake`. The lists are per-compiler because the flags are
not portable; adding a GCC flag to the shared list breaks the clang build and,
more subtly, breaks `clang-tidy`, which re-parses the GCC compile database.

To silence one warning for one file:

```cmake
set_source_files_properties(awkward.cpp PROPERTIES COMPILE_OPTIONS "-Wno-conversion")
```

That is better than removing the flag project-wide.

## Adjusting clang-tidy

`.clang-tidy`. Every exclusion has a comment explaining why; keep that up when
you add one. An unexplained exclusion is indistinguishable from a mistake later.

To silence one instance rather than a whole check:

```cpp
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
auto* bytes = reinterpret_cast<std::byte*>(data);
```

## Updating the hooks

```bash
prek auto-update      # or: pre-commit autoupdate
prek run --all-files
```

Renovate does this too, grouped and automerged.

## Adding a test file

Create it in `tests/` and add it to the `add_executable` call in
`tests/CMakeLists.txt`. Sources are listed rather than globbed on purpose: CMake
does not notice a new file appearing under a glob until something else triggers
a reconfigure, and a test that silently is not built is worse than no test.

## Changing the release cadence

The cron in `.github/workflows/scheduled_release.yml`:

```yaml
schedule:
    - cron: '0 0 * * 2' # Tuesday 00:00 UTC
```

For releases only when you ask, delete the `schedule` block and keep
`workflow_dispatch`.

## Changing the release notes

`cliff.toml`. The `commit_parsers` decide the sections and their order; the
`body` template decides the layout. See
[Changelog](../development/changelog.md).

## Adding a spelling exception

`.typos.toml`. Prefer a real word in `[default.extend-words]` over a broad
regex.

## Turning something off

| To remove     | Delete                                                                         |
| ------------- | ------------------------------------------------------------------------------ |
| The CLI       | `apps/`, its `add_subdirectory`, the `..._BUILD_CLI` option, its install rule  |
| Conan         | `conanfile.py`, `test_package/`, the two publish workflows, the `conan` CI job |
| The docs site | `docs/`, `zensical.toml`, `documentation.yml`                                  |
| Sanitizer CI  | The `sanitizers` job in `ci.yml`                                               |
| Coverage      | The `coverage` job in `ci.yml`, and the codecov badge                          |
| CodeRabbit    | `.coderabbit.yaml`                                                             |
| Renovate      | `renovate.json`                                                                |

Keep `cliff.toml`, `cmake/GitVersion.cmake` and `.pre-commit-config.yaml`:
something else depends on each of them.
