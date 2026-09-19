#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""CommonMark-to-Javadoc compilation and documentation synthesis engine.

Architectural Purpose:
    Filament C++ source headers utilize Doxygen-style docstrings, which are
    pre-parsed and normalized by `extractor.py` into structured CommonMark
    documentation dictionaries.
    This module compiles CommonMark text blocks into standards-compliant HTML
    Javadoc comment blocks (`/** ... */`) suitable for the Android SDK and Java
    IDEs (IntelliJ, Android Studio, VSCode).

Compilation Capabilities:
    1. Inline Typography & Links (`_format_inline`):
       Translates Markdown bold, italic, code spans, and hyperlinks into standard
       HTML `<b>`, `<i>`, `{@code ...}`, and `<a href="...">` elements.
    2. Block-Level Structure (`markdown_to_javadoc`):
       - Paragraphs: Wrapped in `<p>...</p>` blocks with inter-paragraph spacing.
       - Code Blocks: Fenced code (` ``` ` or `~~~`) compiled to `<pre>{@code ... }</pre>`.
       - Markdown Tables: GFM pipe tables parsed and formatted into styled HTML `<table>` blocks.
       - Lists: Unordered (`<ul><li>...</li></ul>`) and ordered (`<ol><li>...</li></ol>`) lists.
       - Headings: ATX (`# H1` -> `<h1>`) and Setext (`===` -> `<h1>`, `---` -> `<h2>`).
    3. Tag Normalization & Synthesis (`generate_javadoc`, `format_see_tag`):
       - `@param`: Re-ordered to match Java method signatures, stripping default values.
       - `@return`: Formatted with concise summary text.
       - `@see`: Normalizes C++ scope operators (`Class::method`) to Java Javadoc syntax (`Class#method`).
       - Advisory paragraphs: `@deprecated`, `warning`, and `note` metadata blocks.
"""

import re
from typing import Any, Dict, List, Optional, Tuple


def _format_inline(text: str) -> str:
    """Transform CommonMark inline markup to HTML/Javadoc equivalents.

    Architectural Purpose:
        Standard JavaDoc toolchains require HTML tags for styling rather than
        Markdown syntax. This function translates inline typography while
        preserving Javadoc `@code` conventions for monospace symbols.

    Input-to-Output Transformations:
        - Bold: `**important**` -> `<b>important</b>`
        - Italic: `*optional*` -> `<i>optional</i>`
        - Code: `` `Engine.create()` `` -> `{@code Engine.create()}`
        - Hyperlinks: `[Guide](https://filament.dev)` -> `<a href="https://filament.dev">Guide</a>`

    Args:
        text: Raw inline Markdown string.

    Returns:
        Formatted HTML/Javadoc string.
    """
    # Step 1: Format bold text (**text** -> <b>text</b>)
    text = re.sub(r'\*\*(.+?)\*\*', r'<b>\1</b>', text)

    # Step 2: Format italic text (*text* -> <i>text</i>)
    text = re.sub(r'\*(.+?)\*', r'<i>\1</i>', text)

    # Step 3: Format inline code snippets using Javadoc {@code ...} tag
    # Using {@code ...} prevents Javadoc from interpreting HTML tags or generics inside code.
    text = re.sub(r'`(.+?)`', r'{@code \1}', text)

    # Step 4: Convert Markdown hyperlinks to standard HTML anchor tags ([text](url) -> <a href="url">text</a>)
    text = re.sub(r'\[(.+?)\]\((.+?)\)', r'<a href="\2">\1</a>', text)

    return text


def _process_table(lines: List[str]) -> List[str]:
    """Compile a Markdown table block into an HTML <table> representation.

    Architectural Purpose:
        Javadoc does not render GitHub-flavored Markdown (GFM) pipe tables natively.
        This helper parses table headers, delimiter alignments (`:---`, `:---:`, `---:`),
        and data rows, emitting a formatted HTML `<table>` with inline CSS text-align styles.

    Walkthrough Example:
        Input lines:
            `| Format | Bits | Alignment |`
            `| :---   | :---: | ---:     |`
            `| RGBA   | 32   | 4         |`
        Output lines:
            `<table summary="">`
            `  <tr>`
            `    <th style="text-align: left">Format</th>`
            `    <th style="text-align: center">Bits</th>`
            `    <th style="text-align: right">Alignment</th>`
            `  </tr>`
            `  <tr>`
            `    <td style="text-align: left">RGBA</td>`
            `    <td style="text-align: center">32</td>`
            `    <td style="text-align: right">4</td>`
            `  </tr>`
            `</table>`

    Args:
        lines: Raw text lines comprising the Markdown table (header, divider, rows).

    Returns:
        List of generated HTML table lines.
    """
    if len(lines) < 2:
        return lines

    # Step 1: Parse header row and delimiter divider row by splitting on pipe characters
    header_row = lines[0].strip('|').split('|')
    divider_row = lines[1].strip('|').split('|')

    # Step 2: Detect column alignments (:--- = left, :---: = center, ---: = right)
    alignments: List[str] = []
    for cell in divider_row:
        cell_text = cell.strip()
        if cell_text.startswith(':') and cell_text.endswith(':'):
            alignments.append('center')
        elif cell_text.endswith(':'):
            alignments.append('right')
        else:
            alignments.append('left')

    out = ['<table summary="">']

    # Step 3: Emit table header <th> cells with corresponding text alignment styles
    out.append("  <tr>")
    for i, cell in enumerate(header_row):
        style = ""
        if i < len(alignments):
            style = f' style="text-align: {alignments[i]}"'
        out.append(f"    <th{style}>{_format_inline(cell.strip())}</th>")
    out.append("  </tr>")

    # Step 4: Emit table body <td> rows with alignment styles and inline markup formatting
    for line in lines[2:]:
        row = line.strip().strip('|').split('|')
        out.append("  <tr>")
        for i, cell in enumerate(row):
            style = ""
            if i < len(alignments):
                style = f' style="text-align: {alignments[i]}"'
            out.append(f"    <td{style}>{_format_inline(cell.strip())}</td>")
        out.append("  </tr>")

    out.append("</table>")
    return out


def markdown_to_javadoc(text: str, strip_outer_paragraph: bool = False) -> str:
    """Translate multi-line Markdown documentation into HTML-compliant Javadoc body text.

    Architectural Purpose:
        Acts as the primary text translation engine for all free-form documentation
        blocks (`brief`, `details`, `note`, `warning`, parameter descriptions).
        Implements a lightweight line-oriented Markdown state machine tracking:
        - `in_code_block`: Fenced `<pre>{@code ... }</pre>` regions.
        - `in_table`: GFM table blocks converted to `<table>`.
        - `in_list`: Ordered (`<ol>`) vs. unordered (`<ul>`) list environments.
        - Paragraph buffering: Automatic wrapping of continuous prose lines in `<p>...</p>`.

    Args:
        text: Raw Markdown documentation string.
        strip_outer_paragraph: If True and the output consists of a single `<p>...</p>`
            enclosure, strips the `<p>` tags. Useful for compact `@param` and `@return` tags.

    Returns:
        HTML Javadoc string with normalized indentation and tags.
    """
    if not text:
        return ""

    lines = text.split('\n')
    out_lines: List[str] = []

    # State Machine State Flags
    in_code_block = False
    in_table = False
    in_list = False
    list_type: Optional[str] = None

    # Line Buffers
    table_lines: List[str] = []
    paragraph_buffer: List[str] = []
    list_item_buffer: List[str] = []

    def flush_paragraph() -> None:
        """Flush accumulated paragraph lines into out_lines wrapped in <p>...</p>."""
        if paragraph_buffer:
            content = "\n".join(paragraph_buffer).strip()
            if content:
                formatted = _format_inline(content)
                if out_lines and out_lines[-1] != "":
                    out_lines.append("")
                # Block-level HTML elements should not be wrapped in <p>...</p>
                block_tags = (
                    "<ul", "<ol", "<li", "</ul", "</ol", "</li",
                    "<table", "</table", "<tr", "</tr", "<td", "</td", "<th", "</th",
                    "<pre", "</pre", "<blockquote", "</blockquote",
                    "<h1", "<h2", "<h3", "<h4", "<h5", "<h6",
                )
                if any(formatted.startswith(tag) for tag in block_tags):
                    out_lines.append(formatted)
                elif formatted.startswith("<p>") or formatted.startswith("<p "):
                    if formatted.endswith("</p>"):
                        out_lines.append(formatted)
                    else:
                        out_lines.append(f"{formatted}</p>")
                elif formatted.endswith("</p>"):
                    out_lines.append(f"<p>{formatted}")
                else:
                    out_lines.append(f"<p>{formatted}</p>")
            paragraph_buffer.clear()

    def flush_list_item() -> None:
        """Flush accumulated list item text into out_lines wrapped in <li>...</li>."""
        if list_item_buffer:
            content = "\n".join(list_item_buffer)
            out_lines.append(f"  <li>{_format_inline(content)}</li>")
            list_item_buffer.clear()

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Step 1: Code block fencing (``` or ~~~)
        if stripped.startswith('```') or stripped.startswith('~~~'):
            flush_list_item()
            if in_list and list_type:
                out_lines.append(f'</{list_type}>')
                in_list = False

            flush_paragraph()

            if out_lines and out_lines[-1] != "":
                out_lines.append("")

            # Toggle code block state
            if in_code_block:
                out_lines.append('}</pre>')
                in_code_block = False
            else:
                out_lines.append('<pre>{@code')
                in_code_block = True
            i += 1
            continue

        # If inside code block, preserve lines literally without inline formatting
        if in_code_block:
            out_lines.append(line)
            i += 1
            continue

        # Step 2: Table block detection (header row followed by delimiter divider row)
        if not in_table and i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            if '---' in next_line and '|' in next_line:
                flush_list_item()
                if in_list and list_type:
                    out_lines.append(f'</{list_type}>')
                    in_list = False
                flush_paragraph()

                if out_lines and out_lines[-1] != "":
                    out_lines.append("")

                in_table = True
                table_lines = [line, lines[i + 1]]
                i += 2
                continue

        # If inside table block, accumulate rows until a blank line ends the table
        if in_table:
            if not stripped:
                out_lines.extend(_process_table(table_lines))
                in_table = False
                table_lines = []
                i += 1
                continue
            table_lines.append(line)
            i += 1
            continue

        # Step 3: Setext header detection (underline with === for h1 or --- for h2)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            is_h1 = next_line.startswith('===') and all(c == '=' for c in next_line)
            is_h2 = next_line.startswith('---') and all(c == '-' for c in next_line)

            if is_h1 or is_h2:
                flush_list_item()
                if in_list and list_type:
                    out_lines.append(f'</{list_type}>')
                    in_list = False
                flush_paragraph()

                if out_lines and out_lines[-1] != "":
                    out_lines.append("")

                tag = 'h1' if is_h1 else 'h2'
                out_lines.append(f"<{tag}>{_format_inline(stripped)}</{tag}>")
                i += 2
                continue

        # Step 4: ATX header detection (# H1, ## H2, ### H3, etc.)
        if stripped.startswith('#'):
            match = re.match(r'^(#+)\s+(.+)$', stripped)
            if match:
                flush_list_item()
                if in_list and list_type:
                    out_lines.append(f'</{list_type}>')
                    in_list = False
                flush_paragraph()

                if out_lines and out_lines[-1] != "":
                    out_lines.append("")

                level = len(match.group(1))
                content = match.group(2)
                out_lines.append(f"<h{level}>{_format_inline(content)}</h{level}>")
                i += 1
                continue

        # Step 5: List item detection (- item, * item, 1. item)
        is_ul = stripped.startswith('- ') or stripped.startswith('* ')
        is_ol = bool(re.match(r'^\d+\.', stripped))

        if is_ul or is_ol:
            flush_paragraph()
            flush_list_item()

            target_list_type = 'ul' if is_ul else 'ol'
            if not in_list:
                in_list = True
                list_type = target_list_type
                if out_lines and out_lines[-1] != "":
                    out_lines.append("")
                out_lines.append(f'<{list_type}>')
            elif list_type != target_list_type:
                out_lines.append(f'</{list_type}>')
                list_type = target_list_type
                out_lines.append(f'<{list_type}>')

            if is_ul:
                content = stripped[2:].strip()
            else:
                content = stripped.split('.', 1)[1].strip()
            list_item_buffer.append(content)
            i += 1
            continue

        if in_list:
            if not stripped:
                i += 1
                continue
            list_item_buffer.append(stripped)
            i += 1
            continue

        # Step 6: Blank line (Inter-paragraph separator)
        if not stripped:
            flush_paragraph()
            i += 1
            continue

        # Step 7: Regular prose line accumulation
        paragraph_buffer.append(line)
        i += 1

    # Step 8: Final cleanup flushes for remaining buffers
    flush_list_item()
    if in_code_block:
        out_lines.append('}</pre>')
    if in_table:
        out_lines.extend(_process_table(table_lines))
    if in_list and list_type:
        out_lines.append(f'</{list_type}>')
    flush_paragraph()

    result = '\n'.join(out_lines)

    # Step 9: Post-process: strip outer paragraph if requested for compact single-line embedding
    if strip_outer_paragraph and result.startswith("<p>") and result.endswith("</p>"):
        inner = result[3:-4]
        # Only strip if no additional paragraph tags are nested inside
        if "<p>" not in inner:
            return inner

    return result


def format_arg_type(raw_arg: str) -> str:
    """Parse and map a single C++ parameter type to its Java equivalent for @see links.

    Architectural Purpose:
        In Doxygen `@see` tags referencing overloaded C++ methods, parameter types
        are written in C++ syntax (e.g. `const math::float3&`, `size_t`, `const char*`).
        To generate working Javadoc `@see TargetClass#method(float[], int, String)`
        links, this function translates C++ type signatures to Java types.

    Walkthrough Examples:
        - `"const math::float3&"` -> `"float[]"`
        - `"size_t count"` -> `"int"`
        - `"const char* name"` -> `"String"`
        - `"uint32_t"` -> `"int"`
        - `"bool"` -> `"boolean"`

    Args:
        raw_arg: Raw C++ parameter string (e.g. 'const math::float3& v').

    Returns:
        Mapped Java type string (e.g. 'float[]').
    """
    arg = raw_arg.strip()
    if not arg:
        return ""

    # Step 1: Strip default argument initializers (e.g. 'float val = 0.0f' -> 'float val')
    arg = arg.split('=')[0].strip()

    # Step 2: Separate parameter name from type if both are present in the signature
    tokens = arg.split()
    if len(tokens) > 1:
        last = tokens[-1]
        c_keywords = (
            "const", "volatile", "unsigned", "signed", "long", "int",
            "short", "char", "float", "double", "bool", "void"
        )
        # If the last token is an identifier and not a C type qualifier, it is the param name
        if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', last) and last not in c_keywords:
            tokens = tokens[:-1]
            arg = " ".join(tokens)

    is_pointer_or_array = ("*" in arg or "[]" in arg)

    # Step 3: Strip C++ qualifiers (const, volatile, struct, class, enum, nullability)
    clean = re.sub(r'\b(const|volatile|struct|class|enum|_Nonnull|_Nullable)\b', '', arg)
    clean = clean.replace('&', '').replace('*', '').replace('[', '').replace(']', '').strip()

    # Step 4: Strip C++ namespace prefixes (e.g. 'math::float3' -> 'float3')
    if "::" in clean:
        clean = clean.split("::")[-1]

    # Step 5: Translate to canonical Java types
    type_map = {
        "uint8_t": "int",
        "int8_t": "int",
        "uint16_t": "int",
        "int16_t": "int",
        "uint32_t": "int",
        "int32_t": "int",
        "size_t": "int",
        "uint64_t": "long",
        "int64_t": "long",
        "bool": "boolean",
        "Entity": "int",
        "Instance": "int",
        "float2": "float[]",
        "float3": "float[]",
        "float4": "float[]",
        "mat3": "float[]",
        "mat4": "float[]",
        "double2": "double[]",
        "double3": "double[]",
        "double4": "double[]",
        "int2": "int[]",
        "int3": "int[]",
        "int4": "int[]",
        "uint2": "int[]",
        "uint3": "int[]",
        "uint4": "int[]",
        "char": "String" if is_pointer_or_array else "byte",
        "string": "String",
        "string_view": "String",
        "CString": "String",
        "PixelBufferDescriptor": "Buffer",
        "BufferDescriptor": "Buffer",
    }

    if clean in type_map:
        mapped = type_map[clean]
        if is_pointer_or_array and not mapped.endswith("[]") and mapped != "String":
            return f"{mapped}[]"
        return mapped

    # Handle primitive pointer types mapping to arrays
    if is_pointer_or_array and clean in ("float", "double", "int", "long", "short", "byte", "boolean"):
        return f"{clean}[]"

    return clean


def format_see_tag(s: str) -> str:
    """Format a Doxygen \\see tag reference into an idiomatic Javadoc @see link target.

    Architectural Purpose:
        In C++ Doxygen, cross-references use C++ scope syntax (e.g. `\\see View::setViewport`
        or `\\see Camera::setProjection(float, float, float, float)`).
        Javadoc requires the `#` separator for member references (e.g. `@see View#setViewport`
        or `@see Camera#setProjection(float, float, float, float)`).
        This function normalizes scope operators and translates parameter type signatures.

    Walkthrough Examples:
        - `"Camera::setCustomProjection"` -> `"Camera#setCustomProjection"`
        - `"setViewport"` -> `"#setViewport"` (lowercase method reference on current class)
        - `"Engine"` -> `"Engine"` (class reference preserved)
        - `"LightManager::Builder::color(const math::float3&)"` ->
          `"LightManager.Builder#color(float[])"`

    Args:
        s: Raw \\see target string from Doxygen comment.

    Returns:
        Formatted Javadoc @see link target string.
    """
    if not s:
        return ""
    s = s.strip()

    # Step 1: If already formatted as a quoted string, HTML link, or {@link ...}, preserve as-is
    if s.startswith('"') or s.startswith('<') or s.startswith('{@'):
        return s

    # Step 2: Strip trailing punctuation (periods, commas, semicolons)
    s = re.sub(r'[\.;,]+$', '', s).strip()

    # Step 3: Parse parenthesized argument list if present
    paren_match = re.match(r'^(.*?)\((.*)\)$', s)
    if paren_match:
        target = paren_match.group(1).strip()
        args_str = paren_match.group(2).strip()
    else:
        target = s
        args_str = None

    # Step 4: Standardize C++ scope operators (:: or :) to dot notation
    target = re.sub(r':+', '.', target)

    # Step 5: Convert member reference separator to '#'
    if '#' in target:
        formatted_target = target
    else:
        parts = target.split('.')
        if len(parts) == 1:
            part = parts[0]
            # If identifier starts with lowercase, it is a method on the current class (#method)
            if part and part[0].islower():
                formatted_target = f"#{part}"
            else:
                formatted_target = part
        else:
            cls_part = ".".join(parts[:-1])
            member_part = parts[-1]
            # If member starts with lowercase or has argument list, format as Class#member
            if member_part and (member_part[0].islower() or args_str is not None):
                formatted_target = f"{cls_part}#{member_part}"
            else:
                formatted_target = target

    # Step 6: If no arguments present, return formatted target
    if args_str is None or args_str == "":
        return formatted_target

    # Step 7: Format argument types inside parentheses
    args_list = [format_arg_type(a) for a in args_str.split(',') if a.strip()]
    args_list = [a for a in args_list if a]
    if not args_list:
        return formatted_target

    return f"{formatted_target}({', '.join(args_list)})"


def adapt_param_doc_for_java(desc: str, java_type: Optional[str] = None) -> str:
    """Adapt C++ Doxygen parameter documentation for Java types.

    Translates C++ Slice terminology into idiomatic Java terminology
    based on the concrete Java parameter type ('Buffer' vs array).
    """
    if not desc:
        return desc

    is_buffer = java_type is not None and (java_type == "Buffer" or java_type.endswith("Buffer"))

    if is_buffer:
        desc = re.sub(r'^[Ss]lice\s+to\s+the\s+output\s+array\s+receiving', 'Buffer receiving', desc)
        desc = re.sub(r'\b([Aa])\s+slice\s+of\b', r'\1 buffer of', desc)
        desc = re.sub(r'\b([Ss])lice\s+of\b', lambda m: ('B' if m.group(1).isupper() else 'b') + 'uffer of', desc)
        desc = re.sub(r'\b([Ss])lice\s+containing\b', lambda m: ('B' if m.group(1).isupper() else 'b') + 'uffer containing', desc)
        desc = re.sub(r'\b([Ss])lice\s+to\b', lambda m: ('B' if m.group(1).isupper() else 'b') + 'uffer to', desc)
        desc = re.sub(r'\bSlice\b', 'Buffer', desc)
        desc = re.sub(r'\bslice\b', 'buffer', desc)
    else:
        desc = re.sub(r'^[Ss]lice\s+to\s+the\s+(output\s+array|contiguous\s+array)', lambda m: m.group(1).capitalize(), desc)
        desc = re.sub(r'\b[Ss]lice\s+to\s+the\s+(output\s+array|contiguous\s+array)', r'\1', desc)
        desc = re.sub(r'\b([Aa])\s+slice\s+of\b', lambda m: 'An array of' if m.group(1).isupper() else 'an array of', desc)
        desc = re.sub(r'\b([Ss])lice\s+of\b', lambda m: ('A' if m.group(1).isupper() else 'a') + 'rray of', desc)
        desc = re.sub(r'\b([Ss])lice\s+containing\b', lambda m: ('A' if m.group(1).isupper() else 'a') + 'rray containing', desc)
        desc = re.sub(r'\b([Ss])lice\s+to\b', lambda m: ('A' if m.group(1).isupper() else 'a') + 'rray to', desc)
        desc = re.sub(r'\bSlice\b', 'Array', desc)
        desc = re.sub(r'\bslice\b', 'array', desc)

    return desc


def generate_javadoc(
    doc: Optional[Dict[str, Any]],
    indent_spaces: int = 0,
    argument_names: Optional[List[str]] = None,
    param_types: Optional[Dict[str, str]] = None
) -> str:
    """Synthesize a complete Javadoc comment block from an IR documentation dictionary.

    Architectural Purpose:
        This is the primary public entrypoint for documentation synthesis. It converts
        an extracted class or method documentation dictionary into a properly indented
        Javadoc block (`/** ... */`) adhering strictly to Java style guidelines.

    Assembly Sequence:
        1. Brief Description: Extracted from `doc["brief"]`.
        2. Detailed Description: Extracted from `doc["details"]`.
        3. Warning & Note Paragraphs: Extracted from `doc["meta"]["warning"]` and `["note"]`.
        4. Parameter Tags (`@param`): Filtered and sequenced according to `argument_names`.
        5. Return Tag (`@return`): Extracted from `doc["returns"]`.
        6. Deprecation Notice (`@deprecated`): Extracted from `doc["meta"]["deprecated"]`.
        7. Cross References (`@see`): Formatted via `format_see_tag`.

    Args:
        doc: Documentation dictionary containing 'brief', 'details', 'params', etc., or None.
        indent_spaces: Number of leading indentation spaces to prepend (e.g. 0 for top class, 4 for methods).
        argument_names: Optional list of parameter names to sequence and filter `@param` tags.
        param_types: Optional mapping of parameter name to its Java type for terminology adaptation.

    Returns:
        Complete multi-line Javadoc block string (including /** and */), or empty string if no content.
    """
    if not doc:
        return ""

    indent = " " * indent_spaces

    # Step 1: Extract doc sections from dictionary
    brief = doc.get("brief")
    details = doc.get("details")
    doc_params = doc.get("params", {})
    returns = doc.get("returns")
    see = doc.get("meta", {}).get("see")
    warning = doc.get("meta", {}).get("warning")
    note = doc.get("meta", {}).get("note")
    deprecated = doc.get("meta", {}).get("deprecated")

    # Step 2: Filter and order parameters according to exposed method arguments
    valid_params: List[Tuple[str, str]] = []
    if argument_names is not None:
        for name in argument_names:
            if name in doc_params:
                desc = doc_params[name]
                j_type = param_types.get(name) if param_types else None
                desc = adapt_param_doc_for_java(desc, j_type)
                valid_params.append((name, desc))
    elif doc_params:
        for name, desc in doc_params.items():
            j_type = param_types.get(name) if param_types else None
            desc = adapt_param_doc_for_java(desc, j_type)
            valid_params.append((name, desc))

    # Guard: If no documentation content exists, return empty string
    has_content = bool(
        brief or details or valid_params or returns or see or warning or note or deprecated
    )
    if not has_content:
        return ""

    body_lines: List[str] = []

    # Step 3: Append brief description (single paragraph, stripped outer <p> for clean lead)
    if brief:
        brief_html = markdown_to_javadoc(brief, strip_outer_paragraph=True)
        body_lines.extend(brief_html.split('\n'))

    # Step 4: Append detailed description
    if details:
        if body_lines:
            body_lines.append("")
        details_html = markdown_to_javadoc(details)
        body_lines.extend(details_html.split('\n'))

    # Step 5: Append warnings and notes as distinct advisory paragraphs
    if warning:
        warning_list = warning if isinstance(warning, list) else [warning]
        for w in warning_list:
            if body_lines:
                body_lines.append("")
            w_html = markdown_to_javadoc(w)
            body_lines.extend(w_html.split('\n'))

    if note:
        note_list = note if isinstance(note, list) else [note]
        for n in note_list:
            if body_lines:
                body_lines.append("")
            n_html = markdown_to_javadoc(n)
            body_lines.extend(n_html.split('\n'))

    # Step 6: Append @param tags in signature order
    if valid_params:
        if body_lines:
            body_lines.append("")
        for name, desc in valid_params:
            if not desc:
                body_lines.append(f"@param {name}")
                continue

            desc_html = markdown_to_javadoc(desc, strip_outer_paragraph=True)
            lines = desc_html.split('\n')
            body_lines.append(f"@param {name} {lines[0]}")
            for line in lines[1:]:
                body_lines.append(line)

    # Step 7: Append @return tag
    if returns:
        if body_lines:
            body_lines.append("")
        returns_html = markdown_to_javadoc(returns, strip_outer_paragraph=True)
        lines = returns_html.split('\n')
        body_lines.append(f"@return {lines[0]}")
        for line in lines[1:]:
            body_lines.append(line)

    # Step 8: Append @deprecated notice
    if deprecated:
        if body_lines:
            body_lines.append("")
        dep_list = deprecated if isinstance(deprecated, list) else [deprecated]
        for d in dep_list:
            d_html = markdown_to_javadoc(d, strip_outer_paragraph=True)
            body_lines.append(f"@deprecated {d_html}")

    # Step 9: Append @see reference tags
    if see:
        if body_lines:
            body_lines.append("")

        see_list: List[str] = []
        if isinstance(see, list):
            see_list = see
        elif isinstance(see, str):
            see_list = [x.strip() for x in see.split(',') if x.strip()]

        for s in see_list:
            formatted_see = format_see_tag(s)
            if formatted_see:
                body_lines.append(f"@see {formatted_see}")

    if not body_lines:
        return ""

    # Step 10: Format single-line or multi-line Javadoc block with indentation
    if len(body_lines) == 1:
        return f"{indent}/** {body_lines[0]} */"

    out = [f"{indent}/**"]
    for line in body_lines:
        if line:
            out.append(f"{indent} * {line}")
        else:
            out.append(f"{indent} *")
    out.append(f"{indent} */")

    return "\n".join(out)
