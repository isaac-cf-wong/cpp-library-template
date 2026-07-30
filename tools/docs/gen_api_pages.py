#!/usr/bin/env python3
r"""Turn Doxygen XML into Markdown pages for the Zensical site.

This is the C++ counterpart of ``mkdocstrings``: Doxygen parses the headers and
emits XML, and this script renders that XML as Markdown so the API reference sits
in the same site, with the same theme and search index, as the prose.

Usage::

    python tools/docs/gen_api_pages.py \
        --xml build/dev/docs/doxygen/xml \
        --output docs/api/generated

One page is written per namespace and per class or struct, plus an ``index.md``
listing them. The output directory is overwritten on every run and is git
ignored -- it is a build artefact, not a source file.

Only Doxygen's documented public API appears, because the Doxyfile sets
``EXTRACT_ALL = NO``. An undocumented public symbol is therefore invisible in the
site *and* a Doxygen warning, which CI treats as an error.
"""

from __future__ import annotations

import argparse
import shutil
import sys
from dataclasses import dataclass, field
from pathlib import Path
from xml.etree import ElementTree

# Doxygen compound kinds that get a page of their own.
PAGE_KINDS = ("namespace", "class", "struct")

# Kinds rendered as a section within a page, in the order the sections appear.
MEMBER_SECTIONS = (
    ("enum", "Enumerations"),
    ("typedef", "Type aliases"),
    ("variable", "Constants"),
    ("function", "Functions"),
)


def text_of(element: ElementTree.Element | None) -> str:
    """Render a Doxygen description element as inline Markdown.

    Args:
        element: A ``briefdescription``, ``detaileddescription`` or ``para``
            element, or ``None``.

    Returns:
        Markdown text, with paragraphs separated by blank lines.
    """
    if element is None:
        return ""

    parts: list[str] = []
    _render(element, parts)
    rendered = "".join(parts)

    # Collapse the runs of blank lines that nested paragraphs produce.
    lines = [line.rstrip() for line in rendered.splitlines()]
    out: list[str] = []
    for line in lines:
        if not line and out and not out[-1]:
            continue
        out.append(line)
    return "\n".join(out).strip()


def _render(element: ElementTree.Element, parts: list[str]) -> None:  # noqa: PLR0911, PLR0912
    """Append the Markdown rendering of ``element`` to ``parts``.

    Args:
        element: Element to render.
        parts: Accumulator the rendered fragments are appended to.
    """
    tag = element.tag

    # Structured blocks are rendered by the caller, not inline.
    if tag in ("parameterlist", "simplesect", "xrefsect"):
        return

    if tag == "para":
        if element.text:
            parts.append(element.text)
        for child in element:
            _render(child, parts)
            if child.tail:
                parts.append(child.tail)
        parts.append("\n\n")
        return

    if tag == "computeroutput":
        parts.append(f"`{''.join(element.itertext())}`")
        return

    if tag in ("emphasis", "italic"):
        parts.append(f"*{''.join(element.itertext())}*")
        return

    if tag == "bold":
        parts.append(f"**{''.join(element.itertext())}**")
        return

    if tag == "ref":
        # Cross-references become code spans: linking would need a refid to
        # page-anchor map that is not worth the coupling.
        parts.append(f"`{''.join(element.itertext())}`")
        return

    if tag == "programlisting":
        code = "\n".join("".join(line.itertext()) for line in element.findall("codeline"))
        parts.append(f"\n```cpp\n{code}\n```\n\n")
        return

    if tag == "itemizedlist":
        parts.append("\n")
        for item in element.findall("listitem"):
            body = text_of(item).replace("\n", " ").strip()
            parts.append(f"- {body}\n")
        parts.append("\n")
        return

    if tag == "orderedlist":
        parts.append("\n")
        for index, item in enumerate(element.findall("listitem"), start=1):
            body = text_of(item).replace("\n", " ").strip()
            parts.append(f"{index}. {body}\n")
        parts.append("\n")
        return

    if tag == "linebreak":
        parts.append("\n")
        return

    # Anything else: recurse and keep the text.
    if element.text:
        parts.append(element.text)
    for child in element:
        _render(child, parts)
        if child.tail:
            parts.append(child.tail)


def parameter_docs(detailed: ElementTree.Element | None, kind: str) -> list[tuple[str, str]]:
    """Extract a Doxygen ``parameterlist`` as name/description pairs.

    Args:
        detailed: The ``detaileddescription`` element, or ``None``.
        kind: ``"param"`` for ``@param``, ``"exception"`` for ``@throws``.

    Returns:
        A list of ``(name, description)`` pairs, in declaration order.
    """
    if detailed is None:
        return []

    result: list[tuple[str, str]] = []
    for plist in detailed.iter("parameterlist"):
        if plist.get("kind") != kind:
            continue
        for item in plist.findall("parameteritem"):
            names = ["".join(name.itertext()).strip() for name in item.findall("parameternamelist/parametername")]
            description = text_of(item.find("parameterdescription")).replace("\n", " ").strip()
            result.append((", ".join(n for n in names if n), description))
    return result


def simple_section(detailed: ElementTree.Element | None, kind: str) -> str:
    """Extract a Doxygen ``simplesect`` such as ``@return``.

    Args:
        detailed: The ``detaileddescription`` element, or ``None``.
        kind: The ``simplesect`` kind, e.g. ``"return"``.

    Returns:
        The rendered text, or an empty string if the section is absent.
    """
    if detailed is None:
        return ""

    for section in detailed.iter("simplesect"):
        if section.get("kind") == kind:
            return text_of(section.find("para")).replace("\n", " ").strip()
    return ""


@dataclass
class Member:
    """A documented member of a namespace, class or struct."""

    kind: str
    name: str
    signature: str
    brief: str
    detail: str
    params: list[tuple[str, str]] = field(default_factory=list)
    throws: list[tuple[str, str]] = field(default_factory=list)
    returns: str = ""
    values: list[tuple[str, str]] = field(default_factory=list)


@dataclass
class Compound:
    """A namespace, class or struct that gets a page."""

    kind: str
    name: str
    brief: str
    detail: str
    members: list[Member] = field(default_factory=list)

    @property
    def slug(self) -> str:
        """Filename stem for this compound.

        Returns:
            The fully qualified name with ``::`` replaced by ``__``.
        """
        return self.name.replace("::", "__")


def parse_member(node: ElementTree.Element) -> Member:
    """Build a :class:`Member` from a Doxygen ``memberdef`` element.

    Args:
        node: The ``memberdef`` element.

    Returns:
        The parsed member.
    """
    kind = node.get("kind", "")
    name = "".join(node.findtext("name", default="").split())
    detailed = node.find("detaileddescription")

    definition = node.findtext("definition", default="").strip()
    argsstring = node.findtext("argsstring", default="").strip()

    if kind == "function":
        signature = f"{definition}{argsstring}"
    elif kind == "enum":
        signature = f"enum class {name}"
    elif kind == "variable":
        initializer = node.findtext("initializer", default="").strip()
        signature = f"{definition} {initializer}".strip()
    else:
        signature = definition or name

    member = Member(
        kind=kind,
        name=name,
        signature=" ".join(signature.split()),
        brief=text_of(node.find("briefdescription")),
        detail=text_of(detailed),
        params=parameter_docs(detailed, "param"),
        throws=parameter_docs(detailed, "exception"),
        returns=simple_section(detailed, "return"),
    )

    if kind == "enum":
        for value in node.findall("enumvalue"):
            value_name = value.findtext("name", default="").strip()
            value_brief = text_of(value.find("briefdescription")).replace("\n", " ").strip()
            member.values.append((value_name, value_brief))

    return member


def parse_compound(path: Path) -> Compound | None:
    """Parse one Doxygen compound XML file.

    Args:
        path: Path of the compound XML file.

    Returns:
        The parsed compound, or ``None`` if it is not a kind that gets a page.
    """
    root = ElementTree.parse(path).getroot()  # noqa: S314
    node = root.find("compounddef")
    if node is None:
        return None

    kind = node.get("kind", "")
    if kind not in PAGE_KINDS:
        return None

    name = node.findtext("compoundname", default="").strip()

    # Doxygen emits a compound for every anonymous namespace it sees in a
    # header; those are implementation detail.
    if "anonymous_namespace" in name or "@" in name:
        return None

    compound = Compound(
        kind=kind,
        name=name,
        brief=text_of(node.find("briefdescription")),
        detail=text_of(node.find("detaileddescription")),
    )

    for section in node.findall("sectiondef"):
        for member in section.findall("memberdef"):
            if member.get("prot") != "public":
                continue
            compound.members.append(parse_member(member))

    return compound


def render_member(member: Member) -> str:
    """Render one member as a Markdown section.

    Args:
        member: The member to render.

    Returns:
        The Markdown section, ending in a newline.
    """
    lines = [f"### `{member.name}`", ""]

    if member.signature:
        lines += ["```cpp", member.signature, "```", ""]

    if member.brief:
        lines += [member.brief, ""]

    # The detailed description repeats the brief when @brief is the only tag.
    if member.detail and member.detail != member.brief:
        lines += [member.detail, ""]

    if member.values:
        lines += ["| Value | Description |", "| --- | --- |"]
        lines += [f"| `{name}` | {description} |" for name, description in member.values]
        lines += [""]

    if member.params:
        lines += ["| Parameter | Description |", "| --- | --- |"]
        lines += [f"| `{name}` | {description} |" for name, description in member.params]
        lines += [""]

    if member.returns:
        lines += [f"**Returns:** {member.returns}", ""]

    for name, description in member.throws:
        lines += [f"**Throws** `{name}`: {description}", ""]

    return "\n".join(lines)


def render_compound(compound: Compound) -> str:
    """Render a whole compound as a Markdown page.

    Args:
        compound: The compound to render.

    Returns:
        The complete page, including front matter.
    """
    label = {"namespace": "namespace", "class": "class", "struct": "struct"}[compound.kind]
    summary = (compound.brief or f"The {compound.name} {label}.").replace("\n", " ").strip()

    lines = [
        "---",
        f'title: "{compound.name}"',
        f'description: "{summary}"',
        "icon: material/api",
        "---",
        "",
        f"# `{compound.name}`",
        "",
        f"*{label}*",
        "",
    ]

    if compound.brief:
        lines += [compound.brief, ""]
    if compound.detail and compound.detail != compound.brief:
        lines += [compound.detail, ""]

    if not compound.members:
        lines += ["This page has no documented public members.", ""]

    for kind, heading in MEMBER_SECTIONS:
        members = [member for member in compound.members if member.kind == kind]
        if not members:
            continue
        lines += [f"## {heading}", ""]
        lines += [render_member(member) for member in members]

    return "\n".join(lines).rstrip() + "\n"


def render_index(compounds: list[Compound]) -> str:
    """Render the API reference landing page.

    Args:
        compounds: All compounds that got a page.

    Returns:
        The complete ``index.md`` content.
    """
    lines = [
        "---",
        'title: "API reference"',
        'description: "Generated reference for the public C++ API."',
        "icon: material/api",
        "---",
        "",
        "# API reference",
        "",
        "This page is generated from the header comments by `docs/gen_api_pages.py`.",
        "Edit the documentation comments in `include/`, not the files here.",
        "",
    ]

    for kind, heading in (("namespace", "Namespaces"), ("class", "Classes"), ("struct", "Structs")):
        selected = sorted((c for c in compounds if c.kind == kind), key=lambda c: c.name)
        if not selected:
            continue
        lines += [f"## {heading}", ""]
        for compound in selected:
            summary = compound.brief.replace("\n", " ").strip()
            suffix = f" -- {summary}" if summary else ""
            lines.append(f"- [`{compound.name}`]({compound.slug}.md){suffix}")
        lines.append("")

    return "\n".join(lines).rstrip() + "\n"


def main(argv: list[str] | None = None) -> int:
    """Entry point.

    Args:
        argv: Command line arguments, defaulting to ``sys.argv[1:]``.

    Returns:
        A process exit status.
    """
    parser = argparse.ArgumentParser(description="Turn Doxygen XML into Markdown pages for the Zensical site.")
    parser.add_argument("--xml", required=True, type=Path, help="Doxygen XML output directory")
    parser.add_argument("--output", required=True, type=Path, help="Directory to write Markdown pages to")
    args = parser.parse_args(argv)

    index = args.xml / "index.xml"
    if not index.is_file():
        print(f"error: {index} not found; run doxygen first", file=sys.stderr)
        return 1

    compounds: list[Compound] = []
    for compound_xml in sorted(args.xml.glob("*.xml")):
        if compound_xml.name in ("index.xml", "Doxyfile.xml"):
            continue
        compound = parse_compound(compound_xml)
        if compound is not None:
            compounds.append(compound)

    if not compounds:
        print("error: no documented namespaces or classes found in the Doxygen XML", file=sys.stderr)
        return 1

    # Rewrite from scratch so a renamed symbol does not leave a stale page.
    if args.output.exists():
        shutil.rmtree(args.output)
    args.output.mkdir(parents=True)

    for compound in compounds:
        (args.output / f"{compound.slug}.md").write_text(render_compound(compound), encoding="utf-8")

    (args.output / "index.md").write_text(render_index(compounds), encoding="utf-8")

    print(f"wrote {len(compounds) + 1} pages to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
