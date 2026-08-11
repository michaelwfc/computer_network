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

# PyMuPDF span flags bitmask: bit 3 (value 8) = monospaced font
MONOSPACE_BIT = 1 << 3


def is_monospace_line(spans: list[dict]) -> bool:
    """A line counts as code if EVERY non-empty span uses a monospace font."""
    real_spans = [s for s in spans if s["text"].strip()]
    if not real_spans:
        return False
    return all(s["flags"] & MONOSPACE_BIT for s in real_spans)


def raw_code_text(spans: list[dict]) -> str:
    """
    Assemble a code line spans into text WITHOUT bold markup and WITHOUT
    stripping leading whitespace — indentation is semantically meaningful
    in code and must be preserved exactly as extracted.
    """
    text = "".join(s["text"] for s in spans)
    return text.rstrip()


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


def block_to_elements(block: dict, body_size: float) -> list[tuple[str, object]]:
    """
    Convert one block's lines into a list of typed elements:
        ('code',    [raw_line, raw_line, ...])   -- one or more contiguous code lines
        ('heading', "# text")                    -- a merged heading line
        ('body',    "text")                      -- one body-text line

    BUG 1 FIX: consecutive lines at the SAME heading level are merged.
    BUG 3 FIX: consecutive monospace lines are grouped into one 'code'
    element as a LIST of raw lines — never joined into a single string,
    so downstream code can preserve every line break exactly.
    """
    # First pass: classify every line individually
    # tag is 'code', or an int heading level (0 = body)
    classified: list[tuple[str, object]] = []
    for line in block.get("lines", []):
        spans = line.get("spans", [])
        if not spans:
            continue

        if is_monospace_line(spans):
            text = raw_code_text(spans)
            if text.strip():   # keep even blank-looking lines out; real blanks are rare mid-block
                classified.append(("code", text))
            continue

        text = line_text(spans)
        if not text:
            continue
        level = heading_level(spans[0]["size"], body_size)
        classified.append(("level", (level, text)))

    # Second pass: merge consecutive same-kind entries
    elements: list[tuple[str, object]] = []
    i = 0
    while i < len(classified):
        kind, payload = classified[i]

        if kind == "code":
            # Absorb all immediately-following code lines into one element
            code_lines = [payload]
            j = i + 1
            while j < len(classified) and classified[j][0] == "code":
                code_lines.append(classified[j][1])
                j += 1
            elements.append(("code", code_lines))
            i = j

        else:  # kind == "level"
            level, text = payload
            if level > 0:
                texts = [text]
                j = i + 1
                while j < len(classified) and classified[j][0] == "level" \
                        and classified[j][1][0] == level:
                    texts.append(classified[j][1][1])
                    j += 1
                elements.append(("heading", "#" * level + " " + " ".join(texts)))
                i = j
            else:
                elements.append(("body", text))
                i += 1

    return elements


def convert_page(page: fitz.Page, body_size: float) -> str:
    """
    Convert one page to Markdown.

    BUG 2 FIX: join wrapped lines of ONE paragraph that got split across
    multiple blocks, WITHOUT merging genuinely separate paragraphs — using
    vertical gap as the signal (see module docstring).

    BUG 3 FIX: code elements are accumulated separately from body text and
    are NEVER subject to paragraph-joining. Consecutive code-only blocks
    are combined into a single fenced code block, with a blank line
    inserted between them to preserve visual separation between statements
    (e.g. separate CREATE TABLE statements), while every original line
    break within the code is kept intact.

    GAP_RATIO_THRESHOLD is expressed relative to body_size because line
    spacing scales with font size. In practice, wrapped-line gaps run
    roughly 0.2-0.4x the body font size, while paragraph-break gaps run
    noticeably larger (~1x or more). 0.6x sits safely between the two.
    """
    GAP_RATIO_THRESHOLD = 0.6

    blocks = page.get_text("dict")["blocks"]
    output: list[str] = []
    pending_paragraph: list[str] = []
    pending_code: list[str] = []
    prev_bottom: float | None = None

    def flush_paragraph():
        if pending_paragraph:
            output.append(" ".join(pending_paragraph))
            pending_paragraph.clear()

    def flush_code():
        if pending_code:
            output.append("```\n" + "\n".join(pending_code) + "\n```")
            pending_code.clear()

    for block in blocks:
        if block["type"] != 0:   # skip images
            continue

        elements = block_to_elements(block, body_size)
        if not elements:
            continue

        top = block["bbox"][1]
        bottom = block["bbox"][3]
        gap = (top - prev_bottom) if prev_bottom is not None else None
        is_tight_gap = gap is not None and gap <= body_size * GAP_RATIO_THRESHOLD

        block_is_code_only = all(kind == "code" for kind, _ in elements)
        block_is_body_only = all(kind == "body" for kind, _ in elements)

        if block_is_code_only:
            # Never merge code into the prose paragraph accumulator.
            flush_paragraph()
            if pending_code and not is_tight_gap:
                # A visual gap between two code blocks = separate statements;
                # preserve that as a blank line INSIDE the same fence.
                pending_code.append("")
            for _, code_lines in elements:
                pending_code.extend(code_lines)

        elif block_is_body_only:
            flush_code()
            for kind, text in elements:
                if BULLET_RE.match(text):
                    clean = BULLET_RE.sub("", text)
                    flush_paragraph()
                    output.append(f"- {clean}")
                elif is_tight_gap and pending_paragraph:
                    pending_paragraph.append(text)
                else:
                    flush_paragraph()
                    pending_paragraph.append(text)

        else:
            # Mixed block (e.g. contains a heading) — flush everything first
            flush_paragraph()
            flush_code()
            for kind, value in elements:
                if kind == "code":
                    output.append("```\n" + "\n".join(value) + "\n```")
                elif kind == "heading":
                    output.append(value)
                elif kind == "body":
                    if BULLET_RE.match(value):
                        clean = BULLET_RE.sub("", value)
                        output.append(f"- {clean}")
                    else:
                        output.append(value)

        prev_bottom = bottom

    flush_paragraph()
    flush_code()
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


def convert_pdf_to_markdown(
    input_pdf: str,
    out_path :str | None = None
):
    """
    Convert PDF file to Markdown file.

    This is the public API used by cli.py.
    """

    md = pdf_to_markdown(input_pdf)

    if out_path is None:
        out_path = str(Path(input_pdf).with_suffix(".md"))
    Path(out_path).write_text(
        md,
        encoding="utf-8"
    )
    

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 pdf_to_markdown_pymupdf.py <input.pdf> [output.md]")
        sys.exit(1)

    pdf_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else str(Path(pdf_path).with_suffix(".md"))

    md = pdf_to_markdown(pdf_path)
    Path(out_path).write_text(md, encoding="utf-8")
    print(f"Written: {out_path}")