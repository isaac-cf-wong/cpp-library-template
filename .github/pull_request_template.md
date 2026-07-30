---
name: 🚀 Pull Request
about: Submit your changes for review
---

<!--
The pull request TITLE must be a conventional commit, e.g.
`fix(hello): handle an empty name`. Pull requests are squash merged, so the
title becomes the commit message on main, and git-cliff derives the version bump
and the release notes from it. The Lint PR check enforces this.
-->

## 📝 Summary

Briefly describe the changes introduced by this PR. Mention any related issues
using keywords (e.g., `Closes #123`).

## ✨ Type of Change

- [ ] 🐛 Bug fix (non-breaking change which fixes an issue)
- [ ] ✨ New feature (non-breaking change which adds functionality)
- [ ] 💥 Breaking change (fix or feature that would cause existing functionality
      to not work as expected)
- [ ] 📝 Documentation update
- [ ] 🎨 Refactoring (no functional changes, no API changes)

## 🧪 How Has This Been Tested?

Please describe the tests that you ran to verify your changes.

- [ ] **Unit Tests:** `ctest --preset dev`
- [ ] **Package Test:** `ctest --test-dir build/dev`
- [ ] **Coverage:** `cmake --workflow --preset coverage`
- [ ] **Sanitizers:** `cmake --preset asan-ubsan && ctest --preset asan-ubsan`
- [ ] **Manual Test:** (Describe steps)

## 🏗️ Checklist

- [ ] My code follows the style guidelines of this project
      (`prek run --all-files` passes).
- [ ] I have performed a self-review of my own code.
- [ ] Every new public declaration carries a Doxygen comment, with `@param`,
      `@return` and `@throws` where they apply.
- [ ] Anything exported is annotated with `CPP_LIBRARY_TEMPLATE_EXPORT`, and new
      public headers compile on their own.
- [ ] I have made corresponding changes to the documentation.
- [ ] My changes generate no new warnings.
- [ ] I have added tests that prove my fix is effective or that my feature
      works.

## 📷 Screenshots (if applicable)

If this is a UI change or produces a specific output/graph, add screenshots
here.
