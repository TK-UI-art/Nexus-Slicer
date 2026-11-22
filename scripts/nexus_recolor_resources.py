#!/usr/bin/env python3
"""Recolour OrcaSlicer's teal accent in resource files to the Nexus green palette.

The C++ side reads its accent colours from src/slic3r/GUI/NexusTheme.hpp. Icons (SVG) and the
embedded web pages (CSS/HTML) carry the colour as literals, so this script rewrites them. It is
idempotent: run it again after merging upstream OrcaSlicer, which brings new teal icons.

    python3 scripts/nexus_recolor_resources.py            # rewrite files
    python3 scripts/nexus_recolor_resources.py --check    # list files that still need it, exit 1 if any

Keep the colours in sync with NexusTheme.hpp.
"""
import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent

ACCENT = "#69E84A"          # NEXUS_ACCENT
ACCENT_TEXT = "#278A1A"     # NEXUS_ACCENT_TEXT (text on light backgrounds)

# teal family -> green family (case-insensitive hex match)
HEX_MAP = {
    "009688": ACCENT,       # Orca accent
    "26A69A": "#55D636",    # hover            NEXUS_ACCENT_HOVER
    "008172": "#55D636",    # dark hover
    "00675B": "#4BC72E",    # dark accent      NEXUS_ACCENT_PRESSED
    "00897B": "#4BC72E",    # pressed
    "4DB6AC": "#A5F293",    # light            NEXUS_ACCENT_LIGHT
    "52C7B8": "#A5F293",
    "BFE1DE": "#D9F9D2",    # 25 % tint        NEXUS_ACCENT_TINT25
    "E5F0EE": "#F0FDED",    # 10 % tint        NEXUS_ACCENT_TINT10
}
HEX_RE = re.compile(r"#(" + "|".join(HEX_MAP) + r")\b", re.I)
RGB_RE = re.compile(r"rgb(a?)\(\s*0\s*,\s*150\s*,\s*136\s*(,[^)]*)?\)", re.I)
# CSS text colour (not background-color / border-color): use the readable text green on light pages
CSS_TEXT_RE = re.compile(r"(?<![-\w])(color\s*:\s*)#009688\b", re.I)

SCAN = [("resources/images", (".svg",)),
        ("resources/web", (".svg", ".css", ".html"))]


ON_ACCENT = "#101010"       # NEXUS_ON_ACCENT: white text on the bright green is unreadable (1.6:1)
CSS_BLOCK_RE = re.compile(r"\{[^{}]*\}")
ACCENT_BG_RE = re.compile(r"background(-color)?\s*:\s*" + ACCENT, re.I)
WHITE_TEXT_RE = re.compile(r"(?<![-\w])(color\s*:\s*)(#fff\b|#ffffff\b|#fefefe\b|white\b)", re.I)


def dark_text_on_accent(block: re.Match) -> str:
    b = block.group(0)
    return WHITE_TEXT_RE.sub(lambda m: m.group(1) + ON_ACCENT, b) if ACCENT_BG_RE.search(b) else b


def recolor(path: pathlib.Path, text: str) -> str:
    dark_page = "dark" in path.name.lower()
    if path.suffix in (".css", ".html") and not dark_page:
        text = CSS_TEXT_RE.sub(lambda m: m.group(1) + ACCENT_TEXT, text)
    text = HEX_RE.sub(lambda m: HEX_MAP[m.group(1).upper()], text)
    text = RGB_RE.sub(lambda m: "rgb%s(105, 232, 74%s)" % (m.group(1), m.group(2) or ""), text)
    if path.suffix in (".css", ".html"):
        text = CSS_BLOCK_RE.sub(dark_text_on_accent, text)
    return text


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    pending = []
    for rel, exts in SCAN:
        for path in sorted((ROOT / rel).rglob("*")):
            if path.suffix.lower() not in exts or not path.is_file():
                continue
            raw = path.read_bytes()
            try:
                text = raw.decode("utf-8")
            except UnicodeDecodeError:
                continue
            new = recolor(path, text)
            if new != text:
                pending.append(path.relative_to(ROOT))
                if not args.check:
                    path.write_bytes(new.encode("utf-8"))
    verb = "need recolouring" if args.check else "recoloured"
    print(f"{len(pending)} files {verb}")
    return 1 if (args.check and pending) else 0


if __name__ == "__main__":
    sys.exit(main())
