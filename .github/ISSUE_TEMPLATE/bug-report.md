---
name: 🐛 Bug Report
about: Create a report to help us improve
title: '[BUG]: '
labels: bug
assignees: ''
---

## 📝 Description

A clear and concise description of what the bug is.

## 🚀 Reproduction Steps

Steps to reproduce the behavior:

1. '...'
2. '...'
3. '...'

## 💻 Minimal Reproducible Example

Please provide a short snippet of C++ code that demonstrates the issue, together
with the CMake commands you used to build it. Use code blocks for readability:

```cpp
// Your code here
```

```bash
cmake --preset dev && cmake --build --preset dev
```

## 📋 Expected Behavior

A clear and concise description of what you expected to happen.

## 💥 Actual Behavior / Diagnostics

If applicable, add the compiler diagnostics, linker errors or runtime output
here **after redacting secrets** (API keys, tokens, credentials, private
URLs/paths, personal data). If this is a potential security vulnerability, do
**not** post it publicly; use the Security reporting channel instead.

```text
error: ...

```

If the failure is a crash or memory corruption, please run
`cmake --preset asan-ubsan && ctest --preset asan-ubsan` and paste the sanitizer
report as well.

## 🛠 Environment Information

- **Compiler and Version:** (e.g., GCC 14.2, Clang 19.1, AppleClang 16, MSVC
  19.44)
- **Standard Library:** (libstdc++, libc++ or the MSVC STL)
- **Library Version:** (e.g., 1.2.3, or the tag or commit)
- **CMake Version and Preset:** (e.g., 4.0.2, `--preset dev`)
- **Linkage:** (static or shared)
- **Operating System and Architecture:** (e.g., Windows 11 x86_64, Ubuntu 24.04
  x86_64, macOS 15 arm64)
- **Relevant Dependencies:** (e.g., the GoogleTest or Conan version, if
  applicable)

## 📎 Additional Context

Add any other context about the problem here (e.g., a regression range, logs, a
workaround you found).
