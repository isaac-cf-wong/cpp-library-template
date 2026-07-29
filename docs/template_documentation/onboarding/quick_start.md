# Template quick start

Six steps from clicking **Use this template** to a green build.

## 1. Create your repository and clone it

Click **Use this template** on GitHub, create your repository, then:

```bash
git clone git@github.com:<your-owner>/<your-repo>.git
cd <your-repo>
```

## 2. Check the toolchain

```bash
cmake --version    # 3.25 or newer
c++ --version      # a C++20 compiler
ninja --version    # the generator the presets use
```

If Ninja is missing, `apt install ninja-build`, `brew install ninja`, or
`pipx install ninja`.

## 3. Build and test

```bash
cmake --workflow --preset dev
```

That configures, builds and runs the unit tests in one command. GoogleTest is
fetched automatically if it is not already installed, so nothing else is needed.

Expect a warning that no git tag was found and the version fell back to `0.0.0`.
That is correct: there is no release yet. One test skips itself for the same
reason.

## 4. Rename the library after your project

```bash
./setup_repo.sh
```

It reads the owner and repository name from your `origin` remote and rewrites
every derived name: the namespace, the CMake target, the export macro, the
include directory and every URL. It then prints the handful of things only you
can fill in.

Rebuild to confirm nothing broke:

```bash
cmake --workflow --preset dev
```

## 5. Install the git hooks

```bash
pipx install prek
prek install
prek run --all-files
```

`prek` is a fast reimplementation of `pre-commit`; either works. The hooks
format the C++, CMake, Markdown, YAML and TOML, and check for typos, secrets and
workflow mistakes.

## 6. Make the first commit

```bash
git add -A
git commit -m "chore: instantiate the template"
git push
```

Use a [conventional commit](../development/changelog.md) message. It is not
cosmetic: the release automation derives version numbers from these messages.

## Next

- [Customize and clean up](customize_repository.md) -- the settings only you can
  fill in, and what to delete.
- [Quick start](../user_guide/quick_start.md) -- the day-to-day commands.
