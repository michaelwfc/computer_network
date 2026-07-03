#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# pdftotext_for_slide.sh
# Convert PDF slides to clean Markdown using pdftotext.
#
# Usage:
#   ./pdftotext_for_slide.sh                       # prompt for a path interactively
#   ./pdftotext_for_slide.sh /path/to/dir          # directory → merged.md inside it
#   ./pdftotext_for_slide.sh /path/to/file.pdf     # single PDF → file.md beside it
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

# ── colour helpers ────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}ℹ${RESET}  $*"; }
success() { echo -e "${GREEN}✔${RESET}  $*"; }
warn()    { echo -e "${YELLOW}⚠${RESET}  $*"; }
error()   { echo -e "${RED}✖${RESET}  $*" >&2; }
die()     { error "$*"; exit 1; }

# ── dependency check ──────────────────────────────────────────────────────────
check_deps() {
    local missing=()
    for cmd in pdftotext find sort sed; do
        command -v "$cmd" &>/dev/null || missing+=("$cmd")
    done
    if [[ ${#missing[@]} -gt 0 ]]; then
        die "Missing required tools: ${missing[*]}\n  Install with: sudo apt install poppler-utils"
    fi
}

# ── argument parsing ──────────────────────────────────────────────────────────
INPUT_PATH=""     # raw positional argument — may be a file or a directory

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            sed -n '3,9p' "$0" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        -*)
            die "Unknown option: $1" ;;
        *)
            [[ -z "$INPUT_PATH" ]] || die "Too many positional arguments."
            INPUT_PATH="$1"; shift ;;
    esac
done

# ── prompt interactively if no argument was given ─────────────────────────────
if [[ -z "$INPUT_PATH" ]]; then
    echo -e "\n${BOLD}PDF → Markdown converter${RESET}"
    echo "──────────────────────────────────────────"
    read -rp "$(echo -e "${CYAN}?${RESET}  Enter a PDF file or directory path: ")" INPUT_PATH
fi

# expand ~ and resolve to an absolute path
INPUT_PATH="${INPUT_PATH/#\~/$HOME}"
INPUT_PATH="$(realpath "$INPUT_PATH" 2>/dev/null)" \
    || die "Cannot resolve path: $INPUT_PATH"

# ── derive TARGET_DIR, OUTPUT_FILE, and MERGE_FILE from the input type ────────
#
#   file.pdf  →  TARGET_DIR = its parent dir
#                PDF_FILES  = (just that one file)
#                OUTPUT_FILE = same dir, same stem, .md extension
#                MERGE_FILE  = ""  (no merging needed — only one file)
#
#   directory →  TARGET_DIR = the directory itself
#                PDF_FILES  = all *.pdf inside (sorted)
#                OUTPUT_FILE = ""  (individual files are not used in dir mode)
#                MERGE_FILE  = TARGET_DIR/merged.md

TARGET_DIR=""
OUTPUT_FILE=""
MERGE_FILE=""

if [[ -f "$INPUT_PATH" ]]; then
    # ── single-file mode ──────────────────────────────────────────────────────
    [[ "$INPUT_PATH" =~ \.[Pp][Dd][Ff]$ ]] \
        || die "File is not a PDF: $INPUT_PATH"
    TARGET_DIR="$(dirname "$INPUT_PATH")"
    # strip extension (case-insensitive .pdf / .PDF) and append .md
    OUTPUT_FILE="${INPUT_PATH%.[Pp][Dd][Ff]}.md"

elif [[ -d "$INPUT_PATH" ]]; then
    # ── directory mode ────────────────────────────────────────────────────────
    TARGET_DIR="$INPUT_PATH"
    MERGE_FILE="$TARGET_DIR/merged.md"

else
    die "Path is neither a PDF file nor a directory: $INPUT_PATH"
fi

# ── build the list of PDFs to process ────────────────────────────────────────
echo ""

if [[ -n "$OUTPUT_FILE" ]]; then
    # single-file mode: the array contains exactly the one PDF passed in
    PDF_FILES=("$INPUT_PATH")
    info "Mode   : single  — $(basename "$INPUT_PATH") → $(basename "$OUTPUT_FILE")"
else
    # directory mode: discover all PDFs, sorted alphabetically
    info "Scanning: $TARGET_DIR"
    mapfile -t PDF_FILES < <(find "$TARGET_DIR" -maxdepth 1 -iname "*.pdf" | sort)
    [[ ${#PDF_FILES[@]} -gt 0 ]] || die "No PDF files found in: $TARGET_DIR"
    info "Mode   : directory — ${#PDF_FILES[@]} PDF(s) → merged.md"
    # truncate/create the merge file now so the append loop starts clean
    > "$MERGE_FILE"
fi

info "Found ${#PDF_FILES[@]} PDF file(s):"
for pdf in "${PDF_FILES[@]}"; do
    echo "     $(basename "$pdf")"
done
echo ""

# ── convert each PDF to clean Markdown ───────────────────────────────────────
# convert_pdf <input.pdf> <output.md>
#
# pdftotext flags used:
#   (no -layout)   → default reading-order mode: flows text left-to-right
#                    without inserting spaces to simulate column positions;
#                    produces clean prose suitable for Markdown
#   -nopgbrk       → suppress form-feed (\f) characters between pages
#   -enc UTF-8     → force UTF-8 output
#
# Post-processing (sed):
#   pass 1: strip trailing whitespace from every line
#   pass 2: collapse runs of 2+ consecutive blank lines into a single blank line
#           (slide PDFs leave large whitespace gaps between elements)
convert_pdf() {
    local pdf="$1"
    local out_md="$2"
    local basename_noext
    basename_noext="$(basename "$pdf" .pdf)"

    local raw_text
    raw_text="$(pdftotext -nopgbrk -enc UTF-8 "$pdf" - 2>/dev/null)" || return 1
    [[ -n "$raw_text" ]] || return 1

    {
        # H1 heading from the original PDF filename (no extension)
        printf '# %s\n\n' "$basename_noext"

        printf '%s\n' "$raw_text" \
            | sed 's/[[:space:]]*$//' \
            | sed -e '/^[[:space:]]*$/{
                N
                /^\n[[:space:]]*$/d
              }'
    } > "$out_md"
}

# ── main loop ─────────────────────────────────────────────────────────────────
CONVERTED=0
FAILED=0
first_merge=true

for pdf in "${PDF_FILES[@]}"; do

    # Determine where this PDF's Markdown goes:
    #   single-file mode  → the explicit OUTPUT_FILE derived from the input path
    #   directory mode    → a temp .md beside the PDF, later appended to MERGE_FILE
    if [[ -n "$OUTPUT_FILE" ]]; then
        dest_md="$OUTPUT_FILE"
    else
        # strip extension case-insensitively and place beside the source PDF
        dest_md="${pdf%.[Pp][Dd][Ff]}.md"
    fi

    echo -e -n "  Converting: ${BOLD}$(basename "$pdf")${RESET} … "

    if convert_pdf "$pdf" "$dest_md"; then
        echo -e "${GREEN}✔${RESET}  → $(basename "$dest_md")"
        CONVERTED=$(( CONVERTED + 1 ))
    else
        echo -e "${RED}✖  (skipped)${RESET}"
        warn "pdftotext failed or produced empty output: $(basename "$pdf")"
        FAILED=$(( FAILED + 1 ))
        continue
    fi

    # directory mode: append each converted .md into the merge file
    if [[ -n "$MERGE_FILE" ]]; then
        if [[ "$first_merge" == true ]]; then
            first_merge=false
        else
            printf '\n\n---\n\n' >> "$MERGE_FILE"
        fi
        cat "$dest_md" >> "$MERGE_FILE"
    fi
done

[[ $CONVERTED -gt 0 ]] || die "All conversions failed. Nothing was written."

# ── summary ───────────────────────────────────────────────────────────────────
echo ""
echo "──────────────────────────────────────────"
success "Done!"
echo "   Converted : $CONVERTED PDF(s)"
[[ $FAILED -gt 0 ]] && warn "   Failed    : $FAILED PDF(s)"

if [[ -n "$MERGE_FILE" ]]; then
    echo "   Output    : $MERGE_FILE"
    echo "   Size      : $(du -sh "$MERGE_FILE" | cut -f1)"
else
    echo "   Output    : $OUTPUT_FILE"
    echo "   Size      : $(du -sh "$OUTPUT_FILE" | cut -f1)"
fi
echo ""