# Support

## 💬 Questions and discussion

For "how do I", "is this supposed to", and design discussion, use
[Discussions](https://github.com/isaac-cf-wong/cpp-library-template/discussions).

## 🐛 Bugs and feature requests

Open an
[issue](https://github.com/isaac-cf-wong/cpp-library-template/issues/new/choose).

A C++ bug report is only actionable with the build details, so please include:

- The library version (`cpp_library_template --version`, or the tag or commit)
- Your compiler and version (`g++ --version`, `clang++ --version`, or the MSVC
  toolset version)
- The standard library: libstdc++, libc++ or the MSVC STL
- Your operating system and architecture
- The CMake version, and the preset or configure command you used
- Whether it is a static or a shared build
- A minimal reproducer, and the exact error output

If the failure is a crash or memory corruption, please try
`cmake --preset asan-ubsan` first and attach the sanitizer output. It usually
turns a day of guessing into a five-minute fix.

## 🙋 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md), and please read
[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).

## 🔐 Security

Do not report security problems in public. Follow [SECURITY.md](SECURITY.md).
