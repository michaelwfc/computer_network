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

"""

# pip install pymupdf
import fitz  # pymupdf
import re
import sys
from pathlib import Path

def block_to_markdown(block: dict, body_size: float) -> str:
    """Convert a single PDF text block to a Markdown element."""
    lines = []

    for line in block.get("lines", []):
        spans = line.get("spans", [])
        if not spans:
            continue

        # Use the first span's font size to determine heading level
        size  = spans[0]["size"]
        flags = spans[0]["flags"]   # bitmask: bit0=superscript, bit1=italic,
                                    #          bit2=serif, bit3=monospace, bit4=bold
        is_bold = bool(flags & (1 << 4))

        # Assemble the full line text, applying **bold** spans inline
        text = ""
        for span in spans:
            s = span["text"].strip()
            if not s:
                continue
            if bool(span["flags"] & (1 << 4)):   # bold span
                text += f"**{s}** "
            else:
                text += s + " "
        text = text.strip()

        if not text:
            continue

        # Map font size → Markdown heading level
        # Body size is the most common font size in the document
        if size >= body_size * 1.8:
            lines.append(f"# {text}")
        elif size >= body_size * 1.4:
            lines.append(f"## {text}")
        elif size >= body_size * 1.15:
            lines.append(f"### {text}")
        else:
            lines.append(text)

    return "\n".join(lines)


def looks_like_bullet(text: str) -> bool:
    """Detect common bullet patterns used in slide PDFs."""
    return bool(re.match(r'^[•·▪▸▶\-–—*]\s+', text))


def detect_body_size(doc: fitz.Document) -> float:
    """Find the most common font size in the document — that's the body size."""
    from collections import Counter
    sizes: Counter = Counter()
    for page in doc:
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                for span in line.get("spans", []):
                    size = round(span["size"])
                    sizes[size] += len(span["text"])
    return float(sizes.most_common(1)[0][0]) if sizes else 12.0


def pdf_to_markdown(pdf_path: str) -> str:
    doc   = fitz.open(pdf_path)
    body  = detect_body_size(doc)
    parts = []

    for page_num, page in enumerate(doc, start=1):
        parts.append(f"\n\n---\n<!-- page {page_num} -->\n")
        blocks = page.get_text("dict")["blocks"]

        for block in blocks:
            if block["type"] != 0:   # 0 = text, 1 = image
                continue

            md = block_to_markdown(block, body)
            if not md:
                continue

            # Convert bullet characters to Markdown list items
            lines = []
            for line in md.splitlines():
                if looks_like_bullet(line):
                    clean = re.sub(r'^[•·▪▸▶\-–—*]\s+', '', line)
                    lines.append(f"- {clean}")
                else:
                    lines.append(line)

            parts.append("\n".join(lines))

    return "\n\n".join(parts)


if __name__ == "__main__":
    pdf_path = sys.argv[1]
    output   = Path(pdf_path).with_suffix(".md")
    md       = pdf_to_markdown(pdf_path)
    output.write_text(md, encoding="utf-8")
    print(f"Written: {output}")