from .pymupdf_for_pdf import convert_pdf_to_markdown

def main():
    import argparse

    parser = argparse.ArgumentParser(description="Convert PDF to Markdown")
    parser.add_argument("input_pdf", help="Path to the input PDF file")
    # Make the output_md argument optional with a default value of None
    parser.add_argument("output_md", help="Path to the output Markdown file", nargs='?', default=None)
    args = parser.parse_args()

    convert_pdf_to_markdown(args.input_pdf, args.output_md)