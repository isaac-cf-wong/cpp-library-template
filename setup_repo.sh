#!/usr/bin/env bash
# Rename this template after your own project.
#
# Run it once, immediately after cloning your new repository from the template:
#
#   ./setup_repo.sh
#
# The new owner and repository name are read from `git remote get-url origin`, so
# there is nothing to type and nothing to get wrong.
#
# A C++ project has more names to change than a Python one. Every one of these is
# derived from the repository name, so they stay consistent:
#
#   cpp-library-template        the repository, and every URL that points at it
#   cpp_library_template        the namespace, the CMake target, the include
#                               directory, the library file name
#   CPP_LIBRARY_TEMPLATE        the CMake options, and the export macro
#   C++ Library Template        the documentation site title
#
# What it deliberately does NOT change, because only you know the right value:
#   * the author name and the copyright line, in LICENSE, CITATION.cff,
#     conanfile.py and zensical.toml
#   * the ORCID and affiliation in CITATION.cff
#   * the [INSERT CONTACT METHOD] placeholder in CODE_OF_CONDUCT.md
#   * the project description, in CMakeLists.txt, conanfile.py, zensical.toml,
#     README.md and CITATION.cff
#
# It reports each of those at the end so none is forgotten.

set -euo pipefail

OLD_OWNER_NAME="isaac-cf-wong"
OLD_REPO_NAME="cpp-library-template"
OLD_SNAKE_NAME="cpp_library_template"
OLD_UPPER_NAME="CPP_LIBRARY_TEMPLATE"
OLD_TITLE_NAME="C++ Library Template"

# ---------------------------------------------------------------------------
# Work out the new names
# ---------------------------------------------------------------------------

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "error: not inside a git repository." >&2
    exit 1
fi

cd "$(git rev-parse --show-toplevel)"

remote_url="$(git config --get remote.origin.url || true)"
if [[ -z "$remote_url" ]]; then
    echo "error: no 'origin' remote is set. Add one first:" >&2
    echo "  git remote add origin git@github.com:<owner>/<repo>.git" >&2
    exit 1
fi

if [[ ! "$remote_url" =~ [:/]([^/]+)/([^/]+)$ ]]; then
    echo "error: cannot parse owner and repository from '$remote_url'." >&2
    exit 1
fi

NEW_OWNER_NAME="${BASH_REMATCH[1]}"
NEW_REPO_NAME="${BASH_REMATCH[2]%.git}"

# cpp_library_template style: dashes become underscores.
NEW_SNAKE_NAME="${NEW_REPO_NAME//-/_}"
# CPP_LIBRARY_TEMPLATE style.
NEW_UPPER_NAME="$(printf '%s' "$NEW_SNAKE_NAME" | tr '[:lower:]' '[:upper:]')"
# "My Library" style, for the documentation site title.
NEW_TITLE_NAME="$(
    printf '%s' "$NEW_SNAKE_NAME" | awk -F_ '{
        for (i = 1; i <= NF; i++) {
            $i = toupper(substr($i, 1, 1)) substr($i, 2)
        }
        print
    }' OFS=' '
)"

# A CMake target and a C++ namespace cannot start with a digit.
if [[ "$NEW_SNAKE_NAME" =~ ^[0-9] ]]; then
    echo "error: '$NEW_REPO_NAME' starts with a digit, which is not a valid C++ identifier." >&2
    echo "       Rename the repository, or edit the names by hand." >&2
    exit 1
fi
if [[ ! "$NEW_SNAKE_NAME" =~ ^[A-Za-z_][A-Za-z0-9_]*$ ]]; then
    echo "error: '$NEW_REPO_NAME' does not map to a valid C++ identifier ('$NEW_SNAKE_NAME')." >&2
    exit 1
fi

# ---------------------------------------------------------------------------
# Preflight
# ---------------------------------------------------------------------------

if [[ "$NEW_REPO_NAME" == "$OLD_REPO_NAME" ]]; then
    echo "error: the remote still points at the template itself ($OLD_OWNER_NAME/$OLD_REPO_NAME)." >&2
    echo "       Point 'origin' at your own repository first." >&2
    exit 1
fi

for required in cliff.toml CMakeLists.txt zensical.toml "include/$OLD_SNAKE_NAME"; do
    if [[ ! -e "$required" ]]; then
        echo "error: '$required' is missing. Run this from a fresh clone of the template." >&2
        exit 1
    fi
done

if [[ -e "include/$NEW_SNAKE_NAME" ]]; then
    echo "error: 'include/$NEW_SNAKE_NAME' already exists; it looks like this script already ran." >&2
    exit 1
fi

if [[ -n "$(git status --porcelain)" ]]; then
    echo "warning: the working tree is not clean. Review 'git diff' carefully afterwards." >&2
fi

cat <<SUMMARY
Renaming the template:

  repository   $OLD_OWNER_NAME/$OLD_REPO_NAME  ->  $NEW_OWNER_NAME/$NEW_REPO_NAME
  identifier   $OLD_SNAKE_NAME  ->  $NEW_SNAKE_NAME
  macros       $OLD_UPPER_NAME  ->  $NEW_UPPER_NAME
  site title   $OLD_TITLE_NAME  ->  $NEW_TITLE_NAME

SUMMARY

# ---------------------------------------------------------------------------
# Rewrite the file contents
# ---------------------------------------------------------------------------

# GNU sed takes -i, BSD sed (macOS) needs -i ''.
if sed --version >/dev/null 2>&1; then
    SED_INPLACE=(-i)
else
    SED_INPLACE=(-i '')
fi

# Only tracked files, so a build directory or a virtual environment is never
# touched. This script is excluded: rewriting it while bash is still reading it
# is a good way to produce nonsense.
#
# `mapfile` would be the obvious way to collect these, and it is deliberately not
# used: it arrived in Bash 4.0, and macOS still ships Bash 3.2 as /bin/bash. This
# script already handles BSD sed, so it has to run there too.
# `read -d ''` predates Bash 4, so the NUL-separated listing is still safe to use.
files=()
while IFS= read -r -d '' path; do
    case "$path" in
    setup_repo.sh | LICENSE) continue ;;
    esac
    # Skip anything that is not text.
    if [[ -f "$path" ]] && grep -Iq . "$path" 2>/dev/null; then
        files+=("$path")
    fi
done < <(git ls-files -z)

if [[ ${#files[@]} -eq 0 ]]; then
    echo "error: no tracked text files found." >&2
    exit 1
fi

# Order matters. The owner/repository pair goes first so that the bare
# repository-name rule cannot break the URLs, and so that third-party action
# references such as `isaac-cf-wong/detect-git-changes-action` -- a different
# repository that must keep pointing at its own owner -- are left alone.
sed "${SED_INPLACE[@]}" \
    -e "s|$OLD_OWNER_NAME/$OLD_REPO_NAME|$NEW_OWNER_NAME/$NEW_REPO_NAME|g" \
    -e "s|$OLD_TITLE_NAME|$NEW_TITLE_NAME|g" \
    -e "s|$OLD_REPO_NAME|$NEW_REPO_NAME|g" \
    -e "s|$OLD_SNAKE_NAME|$NEW_SNAKE_NAME|g" \
    -e "s|$OLD_UPPER_NAME|$NEW_UPPER_NAME|g" \
    "${files[@]}"

# The Pages URL contains the owner on its own, in the documentation badge.
sed "${SED_INPLACE[@]}" \
    -e "s|$OLD_OWNER_NAME\.github\.io|$NEW_OWNER_NAME.github.io|g" \
    "${files[@]}"

# ---------------------------------------------------------------------------
# Rename the paths
# ---------------------------------------------------------------------------

git mv "include/$OLD_SNAKE_NAME" "include/$NEW_SNAKE_NAME"

# Then every remaining file whose *name* carries the old identifier: the umbrella
# header and the package config template today, and whatever you add later. Doing
# this by pattern rather than by a hardcoded list is what keeps the script correct
# as the project grows.
while IFS= read -r path; do
    directory="$(dirname "$path")"
    filename="$(basename "$path")"
    git mv "$path" "$directory/${filename//$OLD_SNAKE_NAME/$NEW_SNAKE_NAME}"
done < <(git ls-files | grep "/[^/]*${OLD_SNAKE_NAME}[^/]*$" || true)

# ---------------------------------------------------------------------------
# Report what is left to do by hand
# ---------------------------------------------------------------------------

echo
echo "Done. Verify the build:"
echo
echo "  cmake --workflow --preset dev"
echo
echo "Then edit these by hand -- only you know the right values:"
echo
printf '  %s\n' \
    "LICENSE ................ the copyright holder" \
    "CITATION.cff ........... author, affiliation, ORCID" \
    "conanfile.py ........... author, description, topics" \
    "CMakeLists.txt ......... the project DESCRIPTION" \
    "zensical.toml .......... site_description, site_author, copyright" \
    "README.md .............. the description, and the badge for your DOI" \
    "CODE_OF_CONDUCT.md ..... the [INSERT CONTACT METHOD] placeholder"

leftovers="$(git grep -In -e "$OLD_REPO_NAME" -e "$OLD_SNAKE_NAME" -e "$OLD_UPPER_NAME" -- . ':!setup_repo.sh' || true)"
if [[ -n "$leftovers" ]]; then
    echo
    echo "Remaining references to the template name (review each one):"
    printf '%s\n' "$leftovers"
fi

echo
echo "Finally, delete this script and the template documentation:"
echo
echo "  git rm setup_repo.sh"
echo "  git rm -r docs/template_documentation"
echo "  # and remove the 'Template documentation' entry from nav in zensical.toml"
