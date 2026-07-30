# Changelog and versioning

There is no `CHANGELOG.md` in this repository, and no version number in any
file. Both are derived from the commit messages by
[git-cliff](https://git-cliff.org/).

## Conventional Commits

```text
<type>[optional scope][!]: <description>

[optional body]

[optional footer(s)]
```

<!-- typos:off -->

| Type       | Meaning                                 | Version effect |
| ---------- | --------------------------------------- | -------------- |
| `feat`     | A new feature                           | minor          |
| `fix`      | A bug fix                               | patch          |
| `perf`     | A performance improvement               | patch          |
| `refactor` | A change with no behavioural difference | patch          |
| `docs`     | Documentation only                      | patch          |
| `style`    | Formatting only                         | patch          |
| `test`     | Tests only                              | patch          |
| `build`    | The build system, or dependencies       | patch          |
| `ci`       | CI configuration                        | patch          |
| `chore`    | Anything else                           | patch          |

<!-- typos:on -->

A `!` after the type, or a `BREAKING CHANGE:` footer, bumps the **major**
version:

```text
refactor!: take std::span instead of a pointer and a length
```

```text
feat(log): make the sink owned rather than borrowed

BREAKING CHANGE: Logger now takes ownership of its stream. Callers passing a
stack-allocated stream must switch to the shared_ptr overload.
```

For a C++ library, remember that "breaking" includes ABI: changing the layout of
an exported type or the signature of an exported function breaks consumers who
do not recompile, even when the source still compiles. That is a major bump.

Good:

```text
feat(log): add a rotating file sink
fix(hello): handle an empty name without allocating
build(conan): require fmt 11
perf(log): avoid a string copy per record
```

Bad:

```text
updates
fixed stuff
WIP
```

## Where it is enforced

Not in a hook. In CI, on the **pull request title**, by
`.github/workflows/semantic_pull_request.yml`. Pull requests are squash merged,
so the title is the commit message that ends up on `main` -- that is the one
that has to be right.

## How the version is computed

```bash
git-cliff --bumped-version --config cliff.toml
```

It reads every commit since the last tag, finds the largest bump any of them
implies, and prints the next version. `scheduled_release.yml` runs exactly this
and tags the result. Nobody types a version number.

## Previewing the notes

```bash
git-cliff --unreleased              # what the next release will say
git-cliff --current                 # the notes for the current tag
git-cliff --bumped-version          # the version it would pick
git-cliff --tag v1.2.3 --unreleased # as if tagged v1.2.3
```

You can also just look: `draft_release.yml` keeps a **Next Release (Draft)**
release on GitHub up to date on every push to `main`. It is a good habit to
glance at it -- a badly worded commit message is much easier to fix before the
release than after.

## cliff.toml

```toml
commit_parsers = [
    { message = "^Merge pull request", skip = true },
    { message = "^feat", group = "<!-- 0 -->🚀 Features" },
    { message = "^fix", group = "<!-- 1 -->🐛 Bug Fixes" },
    ...
]
```

Order matters: the first matching parser wins. The `<!-- N -->` comments set the
order the sections appear in and are stripped from the rendered output.

The `postprocessors` entry is the one thing tied to your repository:

```toml
postprocessors = [
    { pattern = '<REPO>', replace = "https://github.com/isaac-cf-wong/cpp-library-template" },
]
```

Every link in the template is written as `<REPO>/...` and substituted at render
time, so there is a single place the repository URL appears. `setup_repo.sh`
rewrites it.

### Adding a section

```toml
{ message = "^deprecate", group = "<!-- 12 -->⚠️ Deprecations" },
```

Put it before the catch-all `{ message = ".*", ... }`, and add the type to the
allowed list in `CONTRIBUTING.md` so contributors know it exists.

### Tag selection

```toml
tag_pattern = "v[0-9].*"
skip_tags = "beta|alpha"
ignore_tags = "rc"
```

This is also why `git describe` uses `--match 'v[0-9]*'` in
`cmake/GitVersion.cmake`: the two must agree about what counts as a release tag,
or the version CMake reports and the version git-cliff bumps from will disagree.

## Troubleshooting

**A commit does not appear in the notes** -- it did not match a parser that has
a group, or it matched a `skip = true` rule. `filter_unconventional = false`
means unconventional commits land in **💼 Other** rather than vanishing, so
check there first.

**`--bumped-version` picks the wrong bump** -- look at the actual commit
messages with `git log --oneline`. Squash merges mean the pull request titles
are what it sees, not the individual commits on the branch.

**The links in the notes are wrong** -- the `postprocessors` entry still points
at the template. Run `setup_repo.sh`, or edit it.
