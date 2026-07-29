# Quick start

Working _on_ the library, rather than with it.

## The whole loop in one command

```bash
cmake --workflow --preset dev
```

Configure, build, unit test. Use it when you have changed the build files or you
are not sure what state the tree is in.

## The three commands you will actually run

```bash
cmake --build --preset dev      # build
ctest --preset dev             # unit tests
prek run --all-files           # format and lint
```

`ctest --preset dev` skips the `package_consumption` test, which installs the
project and compiles a separate consumer against it. That takes a couple of
seconds, which is a long time in an inner loop. Run everything before pushing:

```bash
ctest --test-dir build/dev
```

## Running one test

```bash
ctest --test-dir build/dev -R Logger              # by name, a regex
ctest --test-dir build/dev --output-on-failure    # show output from failures
./build/dev/bin/cpp_library_template_tests --gtest_filter='Logger*'
```

Running the test binary directly is usually the fastest way to iterate, and it
gives you GoogleTest's own flags: `--gtest_repeat`, `--gtest_shuffle`,
`--gtest_break_on_failure`.

## The example CLI

```bash
./build/dev/bin/cpp_library_template hello world
./build/dev/bin/cpp_library_template --verbose debug goodbye Isaac
./build/dev/bin/cpp_library_template --version
```

## When something is wrong at runtime

```bash
cmake --preset asan-ubsan
cmake --build --preset asan-ubsan
ctest --preset asan-ubsan
```

The address and undefined-behaviour sanitizers catch a class of bug no static
tool can. They only see what the tests execute, which is a good reason to write
the test first.

## Coverage

```bash
cmake --workflow --preset coverage
cmake --build --preset coverage --target coverage
open build/coverage/coverage/index.html
```

## The documentation

```bash
cmake --preset dev -DCPP_LIBRARY_TEMPLATE_BUILD_DOCS=ON
cmake --build build/dev --target docs-serve
```

Then open <http://127.0.0.1:8000>.

## Where the code goes

| What                   | Where                                      |
| ---------------------- | ------------------------------------------ |
| A public header        | `include/cpp_library_template/`            |
| An implementation file | `src/`, listed in `src/CMakeLists.txt`     |
| A test                 | `tests/`, listed in `tests/CMakeLists.txt` |
| Anything CLI-related   | `apps/`                                    |

Adding a source or a test file means adding it to the corresponding
`CMakeLists.txt`. That is deliberate -- globbing sources means CMake does not
notice a new file until something else forces a reconfigure.

See [Project structure](project_structure.md) for the full tour.
