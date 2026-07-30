# Customize and clean up

## Renaming: let the script do it

```bash
./setup_repo.sh
```

A C++ project has more names to keep in step than a Python one, and all of them
are derived from the repository name:

| Form                   | Where it appears                                                          |
| ---------------------- | ------------------------------------------------------------------------- |
| `cpp-library-template` | Every URL: the README badges, `cliff.toml`, `CITATION.cff`                |
| `cpp_library_template` | The namespace, the CMake target, `include/<name>/`, the library file name |
| `CPP_LIBRARY_TEMPLATE` | The CMake options, and the export macro in every header                   |
| `C++ Library Template` | `site_name` in `zensical.toml`, and the README heading                    |

The script rewrites all four, renames `include/cpp_library_template/` and
`cmake/cpp_library_templateConfig.cmake.in`, and then greps for anything it
missed and shows you.

It deliberately leaves alone the third-party GitHub Actions owned by
`isaac-cf-wong`, such as `isaac-cf-wong/detect-git-changes-action`. Those are
different repositories and must keep pointing at their own owner.

## What only you can fill in

The script prints this list; here it is with the reasoning.

| File                 | What to change                                                                                                                                                          |
| -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `LICENSE`            | The copyright holder and year. Or replace it entirely -- if you do, also update the `license` field in `conanfile.py` and `CITATION.cff`, and the badge in `README.md`. |
| `CITATION.cff`       | `authors`, `affiliation`, `orcid`. Delete the file if you do not want your library to be citable.                                                                       |
| `conanfile.py`       | `author`, `description`, `topics`.                                                                                                                                      |
| `CMakeLists.txt`     | The `DESCRIPTION` in the `project()` call.                                                                                                                              |
| `zensical.toml`      | `site_description`, `site_author`, `copyright`. `site_url` is rewritten by `setup_repo.sh`; check it matches your Pages URL.                                            |
| `README.md`          | The description, and the DOI badge if you mint one on Zenodo.                                                                                                           |
| `CODE_OF_CONDUCT.md` | The `[INSERT CONTACT METHOD]` placeholder under **Enforcement**.                                                                                                        |

## GitHub settings to enable

None of this can be done from a file in the repository.

### GitHub Pages

**Settings -> Pages -> Source: GitHub Actions.** Without this the documentation
workflow fails at the deploy step.

`site_url` in `zensical.toml` is already set, and `setup_repo.sh` rewrites it to
your owner and repository. Check it matches the URL GitHub shows on that
settings page -- if the two disagree, the site builds but its canonical links
and `sitemap.xml` point somewhere else.

### Codecov

Add a `CODECOV_TOKEN` repository secret. Public repositories can often skip
this, but the token makes uploads reliable.

### Branch protection

**Settings -> Branches -> Add rule** for `main`. Require these checks, named as
the jobs report them:

- `CI / linux-gcc-shared`, and whichever other matrix cells you consider
  mandatory
- `CI / coverage`
- `CI / clang-tidy`
- `CI / conan create`
- `Lint PR / main`

### The Conan remote, if you want to publish

Publishing ships **off**, and both workflow files are complete. The switch is a
single repository variable:

1. **Settings -> Secrets and variables -> Actions -> Environments.** Create
   `conan` with `CONAN_REMOTE_URL`, `CONAN_LOGIN_USERNAME` and `CONAN_PASSWORD`,
   and `conan-test` with `CONAN_TEST_REMOTE_URL` plus the same two credentials.
2. **Settings -> Secrets and variables -> Actions -> Variables.** Add
   `ENABLE_PUBLISHING` with the value `true`.

That is the whole change: no file to edit, nothing to un-comment. While the
variable is unset both publish jobs are skipped before a runner starts, so a
template you have not configured cannot publish by accident and does not spend
CI minutes trying.

Set the variable but forget a secret and the job fails immediately, naming the
missing ones. That is deliberate: a half-configured publish is a mistake, not a
reason to skip quietly.

Publishing to ConanCenter is a different thing: it means opening a pull request
against `conan-io/conan-center-index`, and it is not automated here.

## What you can delete

The template is meant to be cut down. Each of these is independent.

| If you do not want            | Delete                                                                                                                                                  |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The example command line tool | `apps/`, the `add_subdirectory(apps)` block in `CMakeLists.txt`, the `CPP_LIBRARY_TEMPLATE_BUILD_CLI` option, and the `cli` install rule                |
| The logger example            | `include/cpp_library_template/log.hpp`, `src/log.cpp`, `tests/test_log.cpp`, and their entries in `src/CMakeLists.txt` and `tests/CMakeLists.txt`       |
| Conan                         | `conanfile.py`, `test_package/`, `publish.yml`, `publish_test.yml`, and the `conan` job in `ci.yml`. The FetchContent fallback keeps the build working. |
| The documentation site        | `docs/`, `zensical.toml`, `documentation.yml`, and the `CPP_LIBRARY_TEMPLATE_BUILD_DOCS` option                                                         |
| Sanitizer CI                  | The `sanitizers` job in `ci.yml`. Keep `cmake/Sanitizers.cmake`: it costs nothing when unused.                                                          |
| CodeRabbit                    | `.coderabbit.yaml`                                                                                                                                      |
| Renovate                      | `renovate.json`                                                                                                                                         |

What you should **not** delete, because something else depends on it:

- `cliff.toml` -- the release automation reads it.
- `cmake/GitVersion.cmake` -- without it there is no version.
- `.pre-commit-config.yaml` -- `prek run --all-files` is what keeps the
  formatting reviewable.

## Finally

```bash
git rm setup_repo.sh
git rm -r docs/template_documentation
# and remove the "Template documentation" entry from nav in zensical.toml
```
