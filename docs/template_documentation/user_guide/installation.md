# Installation

## Requirements

| Requirement  | Minimum                     |
| ------------ | --------------------------- |
| C++ standard | C++20                       |
| CMake        | 3.25                        |
| Generator    | Any. The presets use Ninja. |

Compilers, split by whether the minimum is actually tested:

| Compiler   | Minimum | Tested in CI                                        |
| ---------- | ------- | --------------------------------------------------- |
| GCC        | 13      | Yes, pinned (`toolchain-floor-gcc-13`)              |
| Clang      | 16      | Yes, pinned (`toolchain-floor-clang-16`)            |
| AppleClang | 15      | No -- the macOS job uses the current runner image   |
| MSVC       | 19.38   | No -- the Windows job uses the current runner image |

The last two are a stated intent rather than a guarantee: the GitHub runner
images do not offer older Apple or MSVC toolchains, so there is nothing to pin
them against. If you need to hold a specific MSVC toolset, add
`-DCMAKE_GENERATOR_TOOLSET=version=14.38` to the Windows cells.

Nothing else is required. The library has no runtime dependencies, and
GoogleTest is fetched automatically when it is not already available.

Optional tools, each unlocking one workflow:

| Tool         | Install                                         | Unlocks                     |
| ------------ | ----------------------------------------------- | --------------------------- |
| `gcovr`      | `pipx install gcovr`                            | The coverage report         |
| `doxygen`    | `apt install doxygen`                           | The API reference           |
| `zensical`   | `uv pip install -r tools/docs/requirements.txt` | The documentation site      |
| `clang-tidy` | `apt install clang-tidy`                        | Static analysis locally     |
| `conan`      | `pipx install conan`                            | Packaging, and dependencies |
| `prek`       | `pipx install prek`                             | The git hooks               |

## Building from a checkout

```bash
git clone https://github.com/isaac-cf-wong/cpp-library-template.git
cd cpp-library-template
cmake --workflow --preset dev
```

To install into a prefix of your own:

```bash
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix /usr/local
```

!!! warning "Clone with the tags"

    The version comes from `git describe`. A shallow clone or a clone without
    tags produces version `0.0.0` and a warning at configure time. In CI, use
    `fetch-depth: 0`.

## Consuming the library

### With `find_package`

After installing:

```cmake
find_package(cpp_library_template 1.0 REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE cpp_library_template::cpp_library_template)
```

The imported target carries the include directories and the C++20 requirement,
so you do not need to set `CMAKE_CXX_STANDARD` yourself. Version matching is
`SameMajorVersion`: asking for 1.0 accepts any 1.x, never a 2.x.

### With FetchContent

No install step, and no package manager:

```cmake
include(FetchContent)
FetchContent_Declare(
    cpp_library_template
    GIT_REPOSITORY https://github.com/isaac-cf-wong/cpp-library-template.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(cpp_library_template)

target_link_libraries(my_app PRIVATE cpp_library_template::cpp_library_template)
```

Consumed this way the project turns its own tests, CLI, docs and install rules
off automatically, so it adds nothing to your build but the library.

### With Conan

```bash
conan install --requires=cpp_library_template/1.0.0 --build=missing
```

Or in a `conanfile.txt`:

```ini
[requires]
cpp_library_template/1.0.0

[generators]
CMakeDeps
CMakeToolchain
```

The Conan package exposes the same `find_package` name and the same namespaced
target, so your `CMakeLists.txt` does not change depending on where the library
came from.

## Verifying an installation

```bash
cpp_library_template --version
cpp_library_template hello world
```

Or from a program, `tests/package_test/consumer/main.cpp` is a complete working
example.

## Where next

- [Quick start](quick_start.md) -- the day-to-day commands
- [Build system](build_system.md) -- every option and preset
