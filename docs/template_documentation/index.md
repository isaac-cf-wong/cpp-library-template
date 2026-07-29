# Template documentation

This section documents the **template itself**: how the build is put together,
what each CI workflow does, and how releases and publishing work. It is written
for whoever instantiates the template, not for the users of your library.

!!! important

    Delete this section once you have made the template your own. It takes two
    steps:

    1. `git rm -r docs/template_documentation`
    2. Remove the `Template documentation` entry from `nav` in `zensical.toml`

## Where to start

If you have just clicked **Use this template**, read
[Using this GitHub template](onboarding/index.md) first.

## Contents

### Using this GitHub template

| Page                                                         | What it covers                                              |
| ------------------------------------------------------------ | ----------------------------------------------------------- |
| [Overview](onboarding/index.md)                              | What the onboarding section is for                          |
| [Template quick start](onboarding/quick_start.md)            | From clone to a green build, in six steps                   |
| [Customize and clean up](onboarding/customize_repository.md) | Renaming, the settings only you can fill in, what to delete |

### User guide

| Page                                                 | What it covers                                            |
| ---------------------------------------------------- | --------------------------------------------------------- |
| [Installation](user_guide/installation.md)           | Toolchain requirements, and consuming the library         |
| [Quick start](user_guide/quick_start.md)             | The three commands you will run all day                   |
| [Project structure](user_guide/project_structure.md) | Every directory, and why it is laid out this way          |
| [Build system](user_guide/build_system.md)           | CMake options, presets, the version, install and export   |
| [Development tools](user_guide/development_tools.md) | clang-format, clang-tidy, the hooks, the sanitizers       |
| [Testing](user_guide/testing.md)                     | GoogleTest, CTest, coverage, the package consumption test |
| [Documentation](user_guide/documentation.md)         | Doxygen to Markdown to Zensical, and GitHub Pages         |
| [CI/CD](user_guide/ci_cd.md)                         | Every workflow, and the support floor policy              |
| [Packaging](user_guide/packaging.md)                 | The Conan recipe, and publishing                          |
| [Customization](user_guide/customization.md)         | Adding dependencies, changing the rules                   |

### Development notes

| Page                                              | What it covers                                    |
| ------------------------------------------------- | ------------------------------------------------- |
| [Code quality](development/code_quality.md)       | Which tool owns which concern                     |
| [Changelog](development/changelog.md)             | Conventional commits, and how git-cliff uses them |
| [Troubleshooting](development/troubleshooting.md) | The failures you are most likely to hit           |

## The one idea worth taking away

Every version number in this project comes from a **git tag**, and every git tag
comes from **conventional commit messages**. There is no version in
`CMakeLists.txt`, no `CHANGELOG.md` to edit, no release checklist. If you write
`fix:` in a commit message, a patch release happens on its own.

That is why the commit message convention is enforced rather than suggested: it
is load-bearing.
