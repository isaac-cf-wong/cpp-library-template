# Packaging

Three ways to consume this library, one source of truth for each fact.

| Route          | What the consumer needs   | Where it is defined            |
| -------------- | ------------------------- | ------------------------------ |
| `find_package` | An installed prefix       | `cmake/InstallRules.cmake`     |
| FetchContent   | Nothing but CMake and git | The top level `CMakeLists.txt` |
| Conan          | `conan install`           | `conanfile.py`                 |

There is deliberately **no `vcpkg.json`**. A second manifest would encode the
same dependency list a second time, and two lists drift. If you prefer vcpkg,
replace `conanfile.py` rather than adding to it.

## The Conan recipe

`conanfile.py` is the analogue of `pyproject.toml`: metadata, dependencies,
options, and how to build and package.

```bash
conan create . --build=missing -s compiler.cppstd=20
conan create . --build=missing -o '&:shared=True'
conan upload cpp_library_template/1.0.0 -r <remote> --confirm
```

`conan create` also builds and runs `test_package/`, so a package that cannot be
consumed never gets as far as a remote.

### Options

| Option     | Default | Effect                             |
| ---------- | ------- | ---------------------------------- |
| `shared`   | False   | Shared instead of static           |
| `fPIC`     | True    | Position independent code          |
| `with_cli` | False   | Also package the command line tool |

### The version, again

`set_version()` resolves it in this order: an explicit `--version`, then a
`version.txt` recorded at export time, then `git describe`. `export()` writes
that `version.txt`, because the Conan cache has no `.git` and the package
reference and the compiled-in version must agree.

`generate()` then passes it to CMake as `CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE`.
So `conan create` at tag `v1.2.3` produces a package named
`cpp_library_template/1.2.3` whose binary reports `1.2.3`. Those two agreeing is
not an accident, and it is worth keeping that way if you change this code.

## Adding a dependency

Three edits, and the third is the one people miss.

**1. Declare it in `conanfile.py`:**

```python
def requirements(self) -> None:
    """Declare runtime dependencies."""
    self.requires("fmt/11.0.2")
```

**2. Find and link it in CMake:**

```cmake
find_package(fmt REQUIRED)
target_link_libraries(cpp_library_template PRIVATE fmt::fmt)
```

**3. If it appears in a public header, propagate it.** Link it `PUBLIC` instead
of `PRIVATE`, _and_ add a `find_dependency` to
`cmake/cpp_library_templateConfig.cmake.in`:

```cmake
find_dependency(fmt REQUIRED)
```

Without step 3, `find_package(cpp_library_template)` succeeds and the consumer
then fails to compile with a missing include -- a genuinely confusing failure.
`PRIVATE` versus `PUBLIC` is the same question asked twice; keep the two answers
consistent.

If you also want the dependency available without Conan, add a FetchContent
fallback next to the GoogleTest one in `tests/CMakeLists.txt`, with a
`# renovate:` comment so the two pins move together.

## Installing

```bash
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix /tmp/prefix
```

Components let a packager split the install:

```bash
cmake --install build/release --component cpp_library_template_Runtime      # .so, the CLI
cmake --install build/release --component cpp_library_template_Development  # headers, CMake config
```

## Source archives

Each GitHub Release carries a `.tar.gz` and a `.sha256`. The archive contains a
`version.txt`, which `cmake/GitVersion.cmake` reads, so an archive build
produces the right version despite having no git history.

```bash
tar xf cpp_library_template-1.2.3.tar.gz
cd cpp_library_template-1.2.3
cmake --preset release && cmake --build --preset release
```

## Publishing

Publishing is **off** in the template, with both workflows kept intact. Create
the `conan` and `conan-test` environments with their secrets, then set the
`ENABLE_PUBLISHING` repository variable to `true`. Until that variable is set
the publish jobs are skipped before they start.

See [CI/CD](ci_cd.md) for the details.

ConanCenter is a separate route: it means opening a pull request against
`conan-io/conan-center-index` with a recipe, and it is not automated here.

## ABI and versioning

The shared library carries `SOVERSION` = the major version, and the package
version file uses `SameMajorVersion` compatibility. A consumer that asked for
1.2 accepts any 1.x and never a 2.x.

That promise is only as good as your discipline about what a major bump means.
Anything that changes the layout of an exported type, the signature of an
exported function, or the set of exported symbols is a major bump -- even when
the source still compiles. `BREAKING CHANGE:` in the commit message is what
makes it happen.
