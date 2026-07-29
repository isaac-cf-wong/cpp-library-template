# CI/CD

Nine workflows. Six of them exist to answer "does it still work", three to turn
commits into releases.

## ci.yml

Runs on every push and pull request to `main`, and is callable by other
workflows.

### build-and-test

| Cell                       | Platform       | Compiler   | Linkage |
| -------------------------- | -------------- | ---------- | ------- |
| `linux-gcc-shared`         | ubuntu-latest  | GCC        | shared  |
| `linux-gcc-static`         | ubuntu-latest  | GCC        | static  |
| `linux-clang-shared`       | ubuntu-latest  | Clang      | shared  |
| `macos-appleclang-shared`  | macos-latest   | AppleClang | shared  |
| `windows-msvc-shared`      | windows-latest | MSVC       | shared  |
| `windows-msvc-static`      | windows-latest | MSVC       | static  |
| `toolchain-floor-gcc-13`   | ubuntu-24.04   | GCC 13     | shared  |
| `toolchain-floor-clang-16` | ubuntu-24.04   | Clang 16   | shared  |

Static and shared are both there because they take different paths through the
export header: a missing `CPP_LIBRARY_TEMPLATE_EXPORT` links fine statically and
fails in the shared build.

Every cell runs the full `ctest`, including `package_consumption`, so the
install and export rules are verified on all three platforms.

### The support floor

The `toolchain-floor-*` cells are the analogue of the Python template's
`lowest-direct` resolution: they build with the **oldest compiler the project
claims to support**, so the claim in `README.md` is tested rather than asserted.
Both the runner image and the compiler version are pinned, because a floor cell
on `ubuntu-latest` stops testing the floor the moment the image moves.

Two of the four advertised minimums are **not** pinned, and the README says so:
the GitHub runner images ship no older AppleClang or MSVC, so the macOS and
Windows cells test whatever those images currently contain. If you need a
specific MSVC toolset, pass `-DCMAKE_GENERATOR_TOOLSET=version=14.38` in the
Windows cells.

There is no equivalent of SPEC 0 for C++ -- no rolling policy tells you when to
drop GCC 13. Raising the floor is a deliberate decision, and when you make it,
three places change in the same commit:

1. The `toolchain-floor-*` matrix cells in `ci.yml`
2. The requirements table in `README.md`
3. The requirements table in `installation.md`

Renovate is configured **not** to touch compiler or CMake versions for exactly
this reason: a bot should not quietly change your support promise.

### coverage

Configures the `coverage` preset, runs the tests, renders
`build/coverage/coverage.xml`, uploads it to Codecov.

### sanitizers

`asan-ubsan` and `tsan`, with Clang. It also lowers `vm.mmap_rnd_bits`, because
TSan aborts on kernels that randomise more of the address space than its shadow
mapping allows.

### static-analysis

`clang-tidy` over `src/`, `apps/` and `tests/`. It configures with
`CC=clang CXX=clang++`: a GCC compile database contains GCC-only warning flags
that clang rejects, and the job would report `unknown warning option` errors
instead of findings.

### conan

`conan create`, which builds the package and runs the test package. Publishing
depends on this working, so it is checked on every commit rather than discovered
at release time.

## codeql.yml

CodeQL for `c-cpp`, on push, pull request and weekly. C++ is a compiled
language, so CodeQL needs a real build; the workflow runs `cmake --preset ci`
explicitly rather than using `autobuild`, so the analysed configuration is the
one CI tests.

## documentation.yml

On every push to `main`: generate the API Markdown with Doxygen, build the site
with Zensical, deploy to GitHub Pages. `$CI` is set, so an undocumented public
symbol fails the build here.

## The release chain

```text
conventional commit
   │
   ├─► semantic_pull_request.yml   checks the PR title on every PR
   │
   ├─► draft_release.yml           refreshes "Next Release (Draft)" on every push
   │
   └─► scheduled_release.yml       weekly, Tuesday 00:00 UTC
          │
          ├─► check_new_commits    anything since the last tag?
          ├─► ci + codeql          the full checks
          ├─► create_tag           git-cliff --bumped-version, then push the tag
          ├─► release.yml          release notes, source archive, GitHub Release
          └─► publish.yml          conan create, conan upload (opt-in)
```

Two properties worth noticing:

- **No version number is written anywhere.** `git-cliff --bumped-version` reads
  the commit messages since the last tag and computes the next SemVer. `feat:`
  bumps the minor, `fix:` the patch, a `!` or `BREAKING CHANGE:` the major.
- **No `CHANGELOG.md` is committed.** The notes are generated into the GitHub
  Release body. There is no file to conflict on.

### draft_release.yml

Every push to `main` deletes and recreates a draft release tagged
`next-release`, containing the notes for everything unreleased. You can see at
any moment what the next release will say -- and notice a bad commit message
before it ships.

### release.yml

Renders `git-cliff --current`, builds a source archive, and publishes the
release. The archive carries a `version.txt`, because `git archive` output has
no `.git` and CMake could not otherwise determine the version. A tag containing
`alpha` or `beta` is marked as a prerelease automatically.

### publish.yml and publish_test.yml

`publish.yml` builds the Conan package for a tag, static and shared, and uploads
it. `publish_test.yml` does the same for every push to `main`, to a separate
test remote, with a version derived from `git describe` -- so a development
package can never be mistaken for a release.

**Both are off until you opt in.** Each job carries
`if: vars.ENABLE_PUBLISHING == 'true'`, and an unset repository variable is an
empty string, so the template default is off. A skipped job claims no runner, so
a fresh instantiation neither publishes nor spends CI minutes on every push to
`main`.

Turning it on is one variable plus the environment secrets -- see
[Customize and clean up](../onboarding/customize_repository.md). One variable
governs both workflows on purpose; two switches would be two things to remember,
and the interesting failure mode is publishing being half-configured.

Once enabled, each job first checks that its three secrets exist and fails with
a message naming the missing ones, rather than spending minutes building a
package it cannot upload. There is deliberately no `continue-on-error` on the
upload itself, because a publish step that swallows its own failure reports a
green release that never reached the remote.

In the scheduled release chain the GitHub Release is created either way -- only
the Conan upload is gated. With publishing off, the `publish` job shows as
skipped and the release still completes.

## Running a release by hand

```bash
gh workflow run scheduled_release.yml        # the whole chain now
gh workflow run release.yml -f tag_name=v1.2.3
gh workflow run publish.yml -f tag_name=v1.2.3
```

Or tag it yourself and let `release.yml` do the rest:

```bash
git tag -a v1.2.3 -m "Release v1.2.3"
git push origin v1.2.3
```

## Action pinning

Every third-party action is pinned to a **commit SHA** with a `# vN` comment. A
tag can be moved; a SHA cannot. Renovate updates them, grouped and automerged at
the weekend, with major bumps left to a human.

## Adding a matrix cell

```yaml
- name: linux-clang-static
  os: ubuntu-latest
  preset: ci
  cc: clang
  cxx: clang++
  shared: 'OFF'
```

If you make it a required check in branch protection, the name must match
exactly: `CI / linux-clang-static`.

## Troubleshooting

**The version is 0.0.0 in CI** -- the checkout is missing `fetch-depth: 0`.

**`create_tag` exits 1 with "Tag already exists"** -- the computed version is
already released. It means no releasable commits landed, and the guard is doing
its job.

**The Windows cells fail to find the library at test time** -- a shared build
needs the DLL beside the executable or on `PATH`. The install layout puts it in
`bin/`, which is what `run_package_test.cmake` relies on.

**`conan create` fails only in CI** -- almost always a missing `--build=missing`
for a dependency with no prebuilt binary for that compiler and ABI.
