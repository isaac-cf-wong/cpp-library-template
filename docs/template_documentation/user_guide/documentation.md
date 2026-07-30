# Documentation

## The pipeline

```text
include/**/*.hpp
   │  Doxygen comments
   ▼
doxygen  ──►  build/<preset>/docs/doxygen/xml/*.xml
   │
   ▼
tools/docs/gen_api_pages.py  ──►  docs/api/generated/*.md
   │
   ▼
zensical build  ──►  site/
   │
   ▼
GitHub Pages
```

Doxygen is used only as a C++ **parser**. Its own HTML output is turned off; the
XML is converted to Markdown so the API reference lives in the same site as the
prose, with the same theme, navigation and search index. This is the equivalent
of `mkdocstrings` for a Python project.

## Building it

```bash
cmake --preset dev -DCPP_LIBRARY_TEMPLATE_BUILD_DOCS=ON
cmake --build build/dev --target docs         # API Markdown only
cmake --build build/dev --target docs-serve   # live reloading site
cmake --build build/dev --target docs-build   # site into site/
```

`docs-serve` and `docs-build` appear only when `zensical` is on `PATH`, so
install it into a virtual environment first and activate that:

```bash
uv venv
uv pip install -r tools/docs/requirements.txt
source .venv/bin/activate          # .venv\Scripts\activate on Windows
```

`uv pip install` on its own needs an environment to install _into_: with no
virtual environment active it either fails or, worse, quietly picks up a `.venv`
from a parent directory. `--system` is not the answer either -- see the comment
in `.github/workflows/documentation.yml`, where using it against a uv-managed
interpreter is what broke the docs build once already.

Re-run `cmake --preset dev -DCPP_LIBRARY_TEMPLATE_BUILD_DOCS=ON` after
activating, so the configure step can find `zensical`.

`docs/api/generated/` is a build artefact and is git ignored. Never edit those
files -- edit the comments in the headers.

## Writing the comments

Doxygen, `///` style:

```cpp
/// @brief Build a greeting for @p name.
///
/// A longer explanation goes here, and it may span paragraphs. Markdown works:
/// `code spans`, *emphasis*, and lists.
///
/// @param name Name of the person to greet. An empty name yields a greeting
///             addressed to `world`.
/// @return The greeting, without a trailing newline.
/// @throws std::invalid_argument if @p name contains a control character.
[[nodiscard]] CPP_LIBRARY_TEMPLATE_EXPORT std::string hello(std::string_view name);
```

For a class:

```cpp
/// @brief A logger that writes timestamped records to a stream.
///
/// Records are formatted as `TIMESTAMP | LEVEL | name | message`. The logger is
/// not thread safe; guard it externally if you share one across threads.
class CPP_LIBRARY_TEMPLATE_EXPORT Logger {
public:
    /// @brief Minimum level currently emitted.
    /// @return The active level.
    [[nodiscard]] LogLevel level() const noexcept;
};
```

Enumerators are documented inline:

```cpp
enum class LogLevel : std::uint8_t {
    trace,  ///< Fine-grained tracing, off by default.
    info,   ///< Normal operational messages.
};
```

Each file starts with a `@file` block saying what it is for.

## Documentation is enforced

`EXTRACT_ALL = NO`, so an undocumented public symbol simply does not appear in
the site. `WARN_IF_UNDOCUMENTED`, `WARN_IF_INCOMPLETE_DOC` and
`WARN_NO_PARAMDOC` are on, and `tools/docs/CMakeLists.txt` sets
`WARN_AS_ERROR = FAIL_ON_WARNINGS` when `$CI` is set.

The result: a public function without an `@param` for each parameter fails the
documentation workflow, and locally it is a warning you can work through.
Warnings are written to `build/<preset>/docs/doxygen/doxygen_warnings.log`.

## The generator

`tools/docs/gen_api_pages.py` walks the Doxygen XML and writes one page per
namespace, class and struct, plus an index. It renders the brief and detailed
descriptions, signatures, parameter tables, return values, exceptions and
enumerator tables.

It is intentionally small and in-repo rather than a third-party bridge, so when
the output is not what you want you can go and change it. If you need something
it does not do -- inheritance diagrams, grouped `@defgroup` pages, template
parameter tables -- add it there.

## The prose pages

`docs/` holds the hand-written pages. Three of them are one-line snippet
includes, so the content lives in one place:

```markdown
--8<-- "README.md"
```

Front matter sets the title, description and icon:

```markdown
---
title: 'My page'
description: 'One sentence, used for the meta description.'
icon: material/api
---
```

## Navigation

`nav` in `zensical.toml`, as nested TOML tables:

```toml
nav = [
    { "Get Started" = "index.md" },
    { "API reference" = "api/generated/index.md" },
    { "User guide" = [
        { Installation = "installation.md" },
    ] },
]
```

A page that is not in `nav` is still built but is not linked. Adding a page
means adding it here.

## GitHub Pages

One-time setup:

1. **Settings -> Pages -> Source: GitHub Actions.**
2. Set `site_url` in `zensical.toml` to `https://<owner>.github.io/<repo>/`.
   Without it the sitemap and the canonical links are wrong.
3. Push to `main`. `documentation.yml` builds and deploys.

For a custom domain, set it under **Settings -> Pages -> Custom domain**, commit
a `docs/CNAME` file containing the domain, and update `site_url`.

## The theme

Palette, fonts, features and icons are all in `[project.theme]` in
`zensical.toml`, with most options present but commented so you can see what is
available. To go further, add CSS:

```toml
[project]
extra_css = ["style/custom.css"]
```

## Troubleshooting

**`no documented namespaces or classes found in the Doxygen XML`** -- Doxygen
ran but found nothing to document. Usually `EXTRACT_ALL = NO` plus missing
comments, or `INPUT` pointing at the wrong directory.

**The API pages are stale** -- the `docs` target depends on the Doxygen XML,
which depends on the library target. If you changed only a comment, the library
does not rebuild but Doxygen still reruns. If something is genuinely stuck:
`rm -rf docs/api/generated build/dev/docs`.

**Pages does not update** -- check the workflow run. The most common causes are
**Source** not set to GitHub Actions, and the `github-pages` environment having
a protection rule that blocks the deploy.

**A snippet include renders literally** -- `pymdownx.snippets` must be enabled
in `zensical.toml`, and the path is relative to the project root.
