"""
My recommendation

If your goal is high-quality Markdown conversion for lecture slides (especially Stanford CS144 PDFs), I would redesign the algorithm as follows:

Sort blocks by (y, x).
Merge adjacent text blocks that belong to the same paragraph based on geometry.
Merge wrapped lines inside a paragraph using indentation and vertical spacing rather than preserving every PDF line break.
Convert headings based on font size.
Convert bullets based on bullet glyphs and indentation.
Discard obvious diagram labels (isolated short tokens like R7, A, B, S1, etc.) unless they are part of a table.
Output one Markdown paragraph per logical paragraph, not per PDF line.

This geometry-based approach produces significantly cleaner Markdown than line-by-line conversion and is the strategy used by many high-quality PDF-to-Markdown tools.

PDF
 │
 ▼
Extract all spans (font, position, size)
 │
 ▼
Sort by (y, x)
 │
 ▼
Merge spans → visual lines
 │
 ▼
Merge visual lines → paragraphs
 │
 ▼
Detect headings
 │
 ▼
Detect bullet lists
 │
 ▼
Remove repeated footer/page number
 │
 ▼
Remove isolated diagram labels
 │
 ▼
Generate Markdown


pdf_to_markdown_pymupdf.py

Convert a PDF to Markdown using PyMuPDF, recovering headings (from font size)
and bold text (from font flags).

Usage:
    python3 pdf_to_markdown_pymupdf.py document.pdf          # -> document.md
    python3 pdf_to_markdown_pymupdf.py document.pdf out.md

"""

import re
import sys
from collections import Counter
from pathlib import Path

import fitz  # pymupdf


BULLET_RE = re.compile(r'^[•·▪▸▶\-–—*]\s+')


def detect_body_size(doc: fitz.Document) -> float:
    """Most common font size (weighted by character count) = body text size."""
    sizes: Counter = Counter()
    for page in doc:
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                for span in line.get("spans", []):
                    size = round(span["size"])
                    sizes[size] += len(span["text"])
    return float(sizes.most_common(1)[0][0]) if sizes else 12.0


def heading_level(size: float, body_size: float) -> int:
    """
    Return 1/2/3 for heading levels, 0 for body text.
    Thresholds are ratios relative to the detected body font size.
    """
    if size >= body_size * 1.8:
        return 1
    elif size >= body_size * 1.4:
        return 2
    elif size >= body_size * 1.15:
        return 3
    return 0


def line_text(spans: list[dict]) -> str:
    """Assemble one line's spans into text, wrapping bold spans in **."""
    parts = []
    for span in spans:
        s = span["text"].strip()
        if not s:
            continue
        if span["flags"] & (1 << 4):   # bold flag (bit 4)
            parts.append(f"**{s}**")
        else:
            parts.append(s)
    return " ".join(parts).strip()


def block_to_md_lines(block: dict, body_size: float) -> list[str]:
    """
    Convert one block's lines to a list of Markdown lines.

    BUG 1 FIX: consecutive lines that classify at the SAME heading level
    are merged into a single heading line (joined by a space), instead of
    being emitted as separate heading lines.
    """
    raw_lines: list[tuple[int, str]] = []   # (heading_level, text) per line

    for line in block.get("lines", []):
        spans = line.get("spans", [])
        if not spans:
            continue
        text = line_text(spans)
        if not text:
            continue
        level = heading_level(spans[0]["size"], body_size)
        raw_lines.append((level, text))

    # Merge consecutive same-level heading lines (level > 0) into one line.
    # Body-level lines (level == 0) are NOT merged here — that happens later,
    # across whole blocks, in convert_page().
    merged: list[str] = []
    i = 0
    while i < len(raw_lines):
        level, text = raw_lines[i]
        if level > 0:
            # Look ahead: absorb any immediately-following lines at the SAME level
            j = i + 1
            texts = [text]
            while j < len(raw_lines) and raw_lines[j][0] == level:
                texts.append(raw_lines[j][1])
                j += 1
            prefix = "#" * level + " "
            merged.append(prefix + " ".join(texts))
            i = j
        else:
            merged.append(text)
            i += 1

    return merged


def is_body_line(line: str) -> bool:
    """A line is body prose if it's not a heading and not a bullet."""
    return not line.startswith("#") and not line.startswith("- ")


def convert_page(page: fitz.Page, body_size: float) -> str:
    """
    Convert one page to Markdown.

    BUG 2 FIX: join wrapped lines of ONE paragraph that got split across
    multiple blocks, WITHOUT merging genuinely separate paragraphs.

    The distinguishing signal is vertical spacing, not just "both blocks
    are body text." A line that wraps to a new block sits close to the
    previous block (small gap = normal line leading). A new paragraph has
    extra spacing before it (larger gap = paragraph break), even when the
    PDF's paragraph-spacing "blank line" isn't represented as its own block.

    GAP_RATIO_THRESHOLD is expressed relative to body_size because line
    spacing scales with font size. In practice, wrapped-line gaps run
    roughly 0.2-0.4x the body font size, while paragraph-break gaps run
    noticeably larger (~1x or more). 0.6x sits safely between the two.
    """
    GAP_RATIO_THRESHOLD = 0.6

    blocks = page.get_text("dict")["blocks"]
    output: list[str] = []
    pending_paragraph: list[str] = []
    prev_bottom: float | None = None

    def flush_paragraph():
        if pending_paragraph:
            output.append(" ".join(pending_paragraph))
            pending_paragraph.clear()

    for block in blocks:
        if block["type"] != 0:   # skip images
            continue

        md_lines = block_to_md_lines(block, body_size)
        if not md_lines:
            continue

        # Apply bullet conversion to any line that looks like a bullet
        final_lines = []
        for line in md_lines:
            if BULLET_RE.match(line):
                clean = BULLET_RE.sub("", line)
                final_lines.append(f"- {clean}")
            else:
                final_lines.append(line)

        block_is_body_only = all(is_body_line(l) for l in final_lines)
        top = block["bbox"][1]
        bottom = block["bbox"][3]

        gap = (top - prev_bottom) if prev_bottom is not None else None
        is_tight_gap = gap is not None and gap <= body_size * GAP_RATIO_THRESHOLD

        if block_is_body_only and is_tight_gap and pending_paragraph:
            # Close vertical spacing + both body text => same wrapped paragraph
            pending_paragraph.extend(final_lines)
        else:
            flush_paragraph()
            if block_is_body_only:
                pending_paragraph.extend(final_lines)
            else:
                output.extend(final_lines)

        prev_bottom = bottom

    flush_paragraph()
    return "\n\n".join(output)


def pdf_to_markdown(pdf_path: str) -> str:
    doc = fitz.open(pdf_path)
    body_size = detect_body_size(doc)

    pages_md = []
    for page_num, page in enumerate(doc, start=1):
        page_md = convert_page(page, body_size)
        if page_md.strip():
            pages_md.append(f"<!-- page {page_num} -->\n\n{page_md}")

    return "\n\n---\n\n".join(pages_md)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 pdf_to_markdown_pymupdf.py <input.pdf> [output.md]")
        sys.exit(1)

    pdf_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else str(Path(pdf_path).with_suffix(".md"))

    md = pdf_to_markdown(pdf_path)
    Path(out_path).write_text(md, encoding="utf-8")
    print(f"Written: {out_path}")