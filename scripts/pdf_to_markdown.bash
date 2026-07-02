#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# pdf_to_markdown.sh
# Convert every PDF in a directory to Markdown, then merge into one file.
#
# Usage:
#   ./pdf_to_markdown.sh                  # prompts for a directory interactively
#   ./pdf_to_markdown.sh /path/to/dir     # use the given directory directly
#   ./pdf_to_markdown.sh -o out.md /path  # custom output filename
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
    for cmd in pdftotext find sort; do
        command -v "$cmd" &>/dev/null || missing+=("$cmd")
    done
    if [[ ${#missing[@]} -gt 0 ]]; then
        die "Missing required tools: ${missing[*]}\n  Install with: sudo apt install poppler-utils"
    fi
}

# ── argument parsing ──────────────────────────────────────────────────────────
OUTPUT_FILE=""
TARGET_DIR=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -o|--output)
            [[ -n "${2:-}" ]] || die "Flag $1 requires an argument."
            OUTPUT_FILE="$2"; shift 2 ;;
        -h|--help)
            sed -n '3,9p' "$0" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        -*)
            die "Unknown option: $1" ;;
        *)
            [[ -z "$TARGET_DIR" ]] || die "Too many positional arguments."
            TARGET_DIR="$1"; shift ;;
    esac
done

# ── choose a directory ────────────────────────────────────────────────────────
if [[ -z "$TARGET_DIR" ]]; then
    echo -e "\n${BOLD}PDF → Markdown converter${RESET}"
    echo "──────────────────────────────────────────"
    read -rp "$(echo -e "${CYAN}?${RESET}  Enter the directory path: ")" TARGET_DIR
fi

# expand ~ and resolve the path
TARGET_DIR="${TARGET_DIR/#\~/$HOME}"
TARGET_DIR="$(realpath "$TARGET_DIR" 2>/dev/null)" \
    || die "Cannot resolve path: $TARGET_DIR"

[[ -d "$TARGET_DIR" ]] || die "Directory not found: $TARGET_DIR"

# ── list all PDFs (sorted alphabetically) ────────────────────────────────────
echo ""
info "Scanning: $TARGET_DIR"

# collect PDFs into an array, sorted
mapfile -t PDF_FILES < <(find "$TARGET_DIR" -maxdepth 1 -iname "*.pdf" | sort)

if [[ ${#PDF_FILES[@]} -eq 0 ]]; then
    die "No PDF files found in: $TARGET_DIR"
fi

info "Found ${#PDF_FILES[@]} PDF file(s):"
for pdf in "${PDF_FILES[@]}"; do
    echo "     $(basename "$pdf")"
done

# ── set up output paths ───────────────────────────────────────────────────────
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT          # clean up temp files on exit

if [[ -z "$OUTPUT_FILE" ]]; then
    OUTPUT_FILE="$TARGET_DIR/merged.md"
fi

# resolve output to absolute path
OUTPUT_FILE="$(realpath -m "$OUTPUT_FILE")"

echo ""
info "Output file: $OUTPUT_FILE"
echo ""

# ── convert each PDF to Markdown ─────────────────────────────────────────────
CONVERTED=0
FAILED=0

for pdf in "${PDF_FILES[@]}"; do
    basename_noext="$(basename "$pdf" .pdf)"
    # sanitise filename for use as temp path
    safe_name="${basename_noext//[^a-zA-Z0-9._-]/_}"
    txt_file="$WORK_DIR/${safe_name}.txt"

    echo -e -n "  Converting: ${BOLD}$(basename "$pdf")${RESET} … "

    # pdftotext flags:
    #   -layout   → preserve the spatial layout of the text
    #   -nopgbrk  → omit form-feed characters between pages (cleaner Markdown)
    #   -enc UTF-8 → force UTF-8 output
    if pdftotext -layout -nopgbrk -enc UTF-8 "$pdf" "$txt_file" 2>/dev/null; then
        echo -e "${GREEN}✔${RESET}"
        CONVERTED=$(( CONVERTED + 1 ))
    else
        echo -e "${RED}✖  (skipped)${RESET}"
        warn "pdftotext failed for: $(basename "$pdf")"
        FAILED=$(( FAILED + 1 ))
        continue
    fi
done

[[ $CONVERTED -gt 0 ]] || die "All conversions failed. Nothing to merge."

# ── merge all converted files into one Markdown document ─────────────────────
echo ""
info "Merging $CONVERTED file(s) into: $(basename "$OUTPUT_FILE")"

# overwrite (or create) the output file
> "$OUTPUT_FILE"

first=true
for pdf in "${PDF_FILES[@]}"; do
    basename_noext="$(basename "$pdf" .pdf)"
    safe_name="${basename_noext//[^a-zA-Z0-9._-]/_}"
    txt_file="$WORK_DIR/${safe_name}.txt"

    [[ -f "$txt_file" ]] || continue     # skip files that failed conversion

    # add a separator between documents (not before the very first one)
    if [[ "$first" == true ]]; then
        first=false
    else
        # horizontal rule + blank lines act as a clear visual break in Markdown
        printf '\n\n---\n\n' >> "$OUTPUT_FILE"
    fi

    # write a Markdown H1 heading using the original PDF filename
    printf '# %s\n\n' "$basename_noext" >> "$OUTPUT_FILE"

    # append the converted text; wrap in a code block to preserve -layout spacing
    printf '```\n' >> "$OUTPUT_FILE"
    cat "$txt_file"  >> "$OUTPUT_FILE"
    printf '\n```\n' >> "$OUTPUT_FILE"
done

# ── summary ───────────────────────────────────────────────────────────────────
echo ""
echo "──────────────────────────────────────────"
success "Done!"
echo "   Converted : $CONVERTED PDF(s)"
[[ $FAILED -gt 0 ]] && warn "   Failed    : $FAILED PDF(s)"
echo "   Output    : $OUTPUT_FILE"
echo "   Size      : $(du -sh "$OUTPUT_FILE" | cut -f1)"
echo ""