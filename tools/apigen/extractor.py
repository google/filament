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

import sys
import json
import argparse
import os
import re
import shutil
import platform
import glob
import itertools
import subprocess
import textwrap
import collections


# -----------------------------------------------------------------------------
# Clang Bindings Auto-Discovery
# -----------------------------------------------------------------------------

def setup_clang():
    try:
        import clang.cindex
        return clang.cindex
    except ImportError:
        # Try to find it in Homebrew LLVM
        paths = glob.glob("/opt/homebrew/Cellar/llvm/*/lib/python*/site-packages")
        paths.extend(glob.glob("/usr/local/Cellar/llvm/*/lib/python*/site-packages"))
        
        if not paths:
            print("Error: 'clang' module not found and no local LLVM candidates found.", file=sys.stderr)
            sys.exit(1)
            
        # Sort by version (heuristic: use the path string, higher nums usually better)
        # /opt/homebrew/Cellar/llvm/17.0.6/...
        paths.sort(reverse=True)
        
        sys.path.insert(0, paths[0])
        
        try:
            import clang.cindex
            # Also try to set library path if needed
            # usually cindex.py finds it relative to itself or in lib, but we can help it.
            # site-packages/clang -> ../../../lib
            # /opt/homebrew/Cellar/llvm/17.0.6/lib/python3.11/site-packages
            
            lib_dir = os.path.abspath(os.path.join(paths[0], "../../../"))
            if os.path.exists(os.path.join(lib_dir, "libclang.dylib")):
                clang.cindex.Config.set_library_path(lib_dir)
            
            return clang.cindex
        except ImportError:
            print(f"Error: Failed to import clang.cindex even after adding {paths[0]} to path.", file=sys.stderr)
            sys.exit(1)

clang_cindex = setup_clang()
from clang.cindex import CursorKind, TypeKind, AccessSpecifier

# -----------------------------------------------------------------------------
# Configuration & Constants
# -----------------------------------------------------------------------------

OP_VERBOSE = True

def log(msg):
    if OP_VERBOSE:
        print(f"[LOG] {msg}", file=sys.stderr)

# -----------------------------------------------------------------------------
# Schema Helpers
# -----------------------------------------------------------------------------

def split_safe(text):
    parts = []
    current = []
    balance = 0
    for char in text:
        if (char == ',' or char == '\n') and balance == 0:
            part = "".join(current).strip()
            if part:
                parts.append(part)
            current = []
        else:
            if char in '(<':
                balance += 1
            elif char in ')>':
                balance -= 1
            # Prevent negative balance just in case
            if balance < 0: balance = 0
            current.append(char)
    if current:
        part = "".join(current).strip()
        if part:
            parts.append(part)
    cleaned = []
    for p in parts:
        p = re.sub(r'[\.;,]+$', '', p).strip()
        p = re.sub(r'\s+', ' ', p).strip()
        if p:
            cleaned.append(p)
    return cleaned

def convert_latex_formulas(text):
    if not text:
        return text
        
    def replace_formula(match):
        formula = match.group(1).strip()
        
        symbols = {
            r"\pi": "π",
            r"\cdot": "·",
            r"\times": "×",
            r"\langle": "⟨",
            r"\rangle": "⟩",
            r"\le": "≤",
            r"\ge": "≥",
            r"\approx": "≈",
            r"\neq": "≠",
            r"\pm": "±",
            r"\infty": "∞",
            r"\alpha": "α",
            r"\beta": "β",
            r"\gamma": "γ",
            r"\theta": "θ",
            r"\lambda": "λ",
            r"\sigma": "σ",
            r"\omega": "ω",
        }
        for latex_sym, uni_sym in symbols.items():
            formula = re.sub(re.escape(latex_sym) + r"(?![a-zA-Z])", uni_sym, formula)

        def replace_frac(m):
            num = m.group(1).strip()
            den = m.group(2).strip()
            if re.search(r"[\+\-\*/·×\s]", den):
                den = f"({den})"
            if re.search(r"[\+\-\*/·×\s]", num):
                num = f"({num})"
            num = re.sub(r"(\w+)\^\{?(\d+)\}?", r"\1<sup>\2</sup>", num)
            den = re.sub(r"(\w+)\^\{?(\d+)\}?", r"\1<sup>\2</sup>", den)
            return f"{num} / {den}"

        formula = re.sub(r"\\frac\{([^{}]+)\}\{([^{}]+)\}", replace_frac, formula)
        formula = re.sub(r"\\hat\{([^{}]+)\}", r"\1^", formula)
        formula = re.sub(r"_\{([^{}]+)\}", r"<sub>\1</sub>", formula)
        formula = re.sub(r"_([a-zA-Z0-9])", r"<sub>\1</sub>", formula)
        formula = re.sub(r"\^\{([^{}]+)\}", r"<sup>\1</sup>", formula)
        formula = re.sub(r"\^([0-9])", r"<sup>\1</sup>", formula)
        formula = re.sub(r"\\([a-zA-Z]+)\{([^{}]+)\}", r"\2", formula)
        formula = re.sub(r"\\([a-zA-Z]+)", r"\1", formula)
        
        return formula.strip()

    text = re.sub(r"[@\\]f\$(.*?)[@\\]f\$", replace_formula, text, flags=re.DOTALL)
    text = re.sub(r"[@\\]f\[(.*?)[@\\]f\]", replace_formula, text, flags=re.DOTALL)
    text = re.sub(r"[@\\]f\{[^\}]*\}(.*?)[@\\]f\}", replace_formula, text, flags=re.DOTALL)
    return text

def is_doxygen_comment(comment):
    if not comment:
        return False
    s = comment.strip()
    return s.startswith("/**") or s.startswith("/*!") or s.startswith("///") or s.startswith("//!")

def parse_doxygen(comment):
    if not comment or not is_doxygen_comment(comment):
        return {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}

    lines = comment.splitlines()
    cleaned_lines = []
    
    # Regex to match optional whitespace followed by * or /// or //! and optional single space
    # We want to capture the content AFTER this marker.
    # Group 0: Full match
    # content is line[match.end():]
    marker_re = re.compile(r"^\s*(\*+|/{2,3}[!<]*|/\*+[\*!<]*)\s?")
    
    has_same_line_title = False
    for line in lines:
        s_line = line.strip()
        if s_line.startswith("/**") or s_line.startswith("/*!"):
            content = re.sub(r"^/\*+[\*!<]?\s*", "", s_line).strip()
            if content and not content.endswith("*/"):
                has_same_line_title = True
                cleaned_lines.append(content)
            elif content.endswith("*/"):
                content = content[:-2].strip()
                if content:
                    cleaned_lines.append(content)
            continue
        if s_line == "*/" or s_line == "**/":
            continue
        if s_line.endswith("*/"):
            line = line[:line.rfind("*/")]
        
        match = marker_re.match(line)
        if match:
            cleaned_lines.append(line[match.end():].rstrip())
        else:
            cleaned_lines.append(line.rstrip())

    # Dedent the whole block to normalize
    full_text = textwrap.dedent("\n".join(cleaned_lines)).strip()
    full_text = convert_latex_formulas(full_text)
    
    doc = {
        "brief": "",
        "details": "",
        "params": {},
        "returns": "",
        "meta": {}
    }

    # Pre-pass: Helper to extract @see tags and remove them from text to preserve flow
    # A @see / \see / @sa / \sa block may continue across lines if a line ends with a comma
    # or was empty (e.g. tag alone on line). Non-comma lines (and new tags/blank lines) terminate the block.
    lines = full_text.splitlines(keepends=True)
    new_lines = []
    see_tags = []

    def _line_continues(content):
        if not content:
            return True
        balance = 0
        for ch in content:
            if ch in "(<": balance += 1
            elif ch in ")>": balance -= 1
        if balance > 0:
            return True
        return content.rstrip().endswith(",")

    i = 0
    while i < len(lines):
        line = lines[i]
        m = re.match(r"^[ \t]*[@\\](?:see|sa)\b([^\n]*)", line)
        if m:
            content_parts = []
            first_line_content = m.group(1).strip()
            if first_line_content:
                content_parts.append(first_line_content)

            cur_content = first_line_content
            while _line_continues(cur_content) and i + 1 < len(lines):
                next_line = lines[i + 1]
                if re.match(r"^[ \t]*[@\\][a-zA-Z_]", next_line) or not next_line.strip():
                    break
                i += 1
                cur_content = next_line.strip()
                if cur_content:
                    content_parts.append(cur_content)

            full_see_content = " ".join(content_parts)
            for c in split_safe(full_see_content):
                see_tags.append(c)
        else:
            new_lines.append(line)
        i += 1

    full_text = "".join(new_lines)

    # Replacement for \p or @p param -> `param`
    # Match \p or @p followed by whitespace, then capture word chars (identifier)
    # Doxygen says it ends at separators. We assume simple identifiers for now.
    # We use \b to ensure we don't partial match if that were possible, but \p requires space.
    full_text = re.sub(r'[@\\]p\s+([a-zA-Z0-9_:]+)', r'`\1`', full_text)

    # Store extracted see tags
    if see_tags:
        doc["meta"]["see"] = see_tags

    # Pre-pass for Block Tags (warning, note, etc)
    # They should consume only one paragraph (up to double newline)
    # Pattern: @tag or \tag ... content ... (stop at \n\s*\n or end of string)
    block_tags = ["warning", "note", "deprecated", "since", "todo"]
    
    for tag in block_tags:
        # (?m) for ^ matching start of line
        # (?s) for . matching newlines (DOTALL equivalent)
        # Match: ^\s*[@\\]tag\s+(.*?) (Lookahead for \n\s*\n or $)
        # We need to use re.compile to specify flags if we use them inline, or passed as args
        # Python inline flags: (?ms) works.
        pattern = re.compile(r'(?ms)^\s*[@\\]' + tag + r'\s+(.*?)(?=\n\s*\n|\Z)')
        
        extracted = []
        def extract_block(match):
            content = match.group(1).strip()
            if content:
                extracted.append(content)
            return "" # Remove from text

        full_text = pattern.sub(extract_block, full_text)
        
        if extracted:
            if len(extracted) == 1:
                doc["meta"][tag] = extracted[0]
            else:
                 doc["meta"][tag] = extracted


    # Replace \n line breaks in text
    full_text = re.sub(r'\\n(?:\s|\n|$)', '\n', full_text)

    # Tokenize only by known section/block tags
    known_section_tags = r'(?:param|tparam|return|returns|brief|details|see|sa|thread_safe|throws|throw|exception|retval)'
    tokens = re.split(r'([@\\]' + known_section_tags + r')\b', full_text)
    
    # Handle initial text (Brief/Details)
    initial_text = tokens[0].strip()
    if initial_text:
        lines = initial_text.splitlines()
        first_list_idx = -1
        for idx, l in enumerate(lines):
            s = l.strip()
            if s.startswith("- ") or s.startswith("* ") or (s and s[0].isdigit() and s[1:3] == ". "):
                first_list_idx = idx
                break
        
        if first_list_idx > 0:
            doc["brief"] = lines[0].strip()
            doc["details"] = "\n".join(lines[1:]).strip()
        elif has_same_line_title and len(lines) > 1 and not lines[0].strip().endswith("."):
            doc["brief"] = lines[0].strip()
            doc["details"] = "\n".join(lines[1:]).strip()
        else:
            if "\n\n" in initial_text:
                parts = initial_text.split("\n\n", 1)
                p0 = parts[0].strip()
                rest = parts[1].strip()
            else:
                p0 = initial_text.strip()
                rest = ""

            m = re.search(r"^(.+?\.)(?:\s+([A-Z0-9`\"{\[<].*)|$)", p0, re.DOTALL)
            if m and m.group(2):
                doc["brief"] = m.group(1).replace("\n", " ").strip()
                remaining_p0 = m.group(2).strip()
                doc["details"] = (remaining_p0 + ("\n\n" + rest if rest else "")).strip()
            else:
                doc["brief"] = p0.replace("\n", " ").strip()
                doc["details"] = rest
    
    i = 1
    while i < len(tokens):
        tag_raw = tokens[i].strip() # e.g. @param
        tag = tag_raw[1:] # param
        content = tokens[i+1].strip() if i+1 < len(tokens) else ""
        
        if tag == "param":
            # Content should start with param name, optionally prefixed by [in], [out], [in,out]
            match = re.match(r'(?:\[(?:in|out|in,\s*out)\]\s+)?(\w+)\s*(.*)', content, re.DOTALL)
            if match:
                p_name, p_desc = match.groups()
                doc["params"][p_name] = p_desc.strip()
        elif tag in ["return", "returns"]:
            doc["returns"] = content
        elif tag == "brief":
            doc["brief"] = content.replace("\n", " ").strip()
        elif tag == "details":
            doc["details"] = content.strip()
        elif tag == "thread_safe":
            doc["meta"][tag] = content
        else:
            if tag not in ["see", "sa"] and tag not in block_tags:
                 doc["meta"][tag] = content
            
        i += 2
        
    if doc["brief"]:
        doc["brief"] = re.sub(r"^<+(?![a-zA-Z/!])\s*", "", doc["brief"]).strip()
    if doc["details"]:
        doc["details"] = re.sub(r"^<+(?![a-zA-Z/!])\s*", "", doc["details"]).strip()

    return doc

# -----------------------------------------------------------------------------
# Type System
# -----------------------------------------------------------------------------

SIZED_TYPES = {
    "int8_t", "uint8_t",
    "int16_t", "uint16_t",
    "int32_t", "uint32_t",
    "int64_t", "uint64_t",
    "size_t", "ssize_t",
    "intptr_t", "uintptr_t",
    "ptrdiff_t",
}

def get_fully_qualified_name(t):
    """
    recursively builds fully qualified name for a type.
    """
    if t.kind == TypeKind.POINTER:
        return get_fully_qualified_name(t.get_pointee()) + "*"
    if t.kind == TypeKind.LVALUEREFERENCE:
        return get_fully_qualified_name(t.get_pointee()) + "&"
    if t.kind == TypeKind.RVALUEREFERENCE:
        return get_fully_qualified_name(t.get_pointee()) + "&&"
    if t.kind == TypeKind.CONSTANTARRAY:
        return get_fully_qualified_name(t.get_array_element_type()) + f"[{t.get_array_size()}]"
    if t.kind == TypeKind.INCOMPLETEARRAY:
        return get_fully_qualified_name(t.get_array_element_type()) + "[]"
    
    # Desugar Typedefs/Aliases
    decl = t.get_declaration()
    if decl.kind in (CursorKind.TYPEDEF_DECL, CursorKind.TYPE_ALIAS_DECL):
        decl_name = decl.spelling
        if decl_name.startswith("std::"):
            decl_name = decl_name[5:]
        if decl_name in SIZED_TYPES:
            return decl_name
        underlying = decl.underlying_typedef_type
        if underlying and underlying.kind != TypeKind.INVALID:
            return get_fully_qualified_name(underlying)
        return get_fully_qualified_name(t.get_canonical())
    
    if decl.kind == CursorKind.NO_DECL_FOUND:
        return t.get_canonical().spelling.replace("struct ", "").replace("class ", "").replace("enum ", "").strip()
        
    # Traverse parents for Scoped Types (Classes, Enums)
    parts = []
    parent = decl
    while parent and parent.kind != CursorKind.TRANSLATION_UNIT:
        if parent.kind != CursorKind.NAMESPACE or parent.spelling:
            if parent.spelling:
                parts.append(parent.spelling)
        parent = parent.semantic_parent
        
    # Reverse to get proper order: Global -> Specific
    parts.reverse()
    
    if t.get_num_template_arguments() > 0:
        args = []
        for i in range(t.get_num_template_arguments()):
            arg_type = t.get_template_argument_type(i)
            q_arg = get_fully_qualified_name(arg_type)
            if q_arg and q_arg.strip():
                args.append(q_arg)
        if parts and args:
            parts[-1] += f"<{', '.join(args)}>"
            
    q_name = "::".join(parts)
    # Normalize libc++ inline namespace
    q_name = q_name.replace("std::__1::", "std::")
    # Normalize std::string
    if q_name.startswith("std::basic_string<char"):
        q_name = "std::string"
    return q_name

def get_cursor_qualified_name(cursor):
    parts = []
    parent = cursor
    while parent and parent.kind != CursorKind.TRANSLATION_UNIT:
        if parent.kind != CursorKind.NAMESPACE or parent.spelling:
             if parent.spelling:
                 parts.append(parent.spelling)
        parent = parent.semantic_parent
    parts.reverse()
    return "::".join(parts)

def get_type_info(t):
    # Canonicalize
    cpp_name = t.spelling
    qualified_name = get_fully_qualified_name(t)
    
    is_const = t.is_const_qualified()
    is_pointer = t.kind == TypeKind.POINTER
    is_reference = t.kind == TypeKind.LVALUEREFERENCE
    is_move_reference = t.kind == TypeKind.RVALUEREFERENCE
    
    # For pointers and references, we care if the pointee is const
    if is_pointer or is_reference:
        # Check if pointee is const
        if t.get_pointee().is_const_qualified():
            is_const = True
    
    # Drill down for category
    canonical = t.get_canonical()
    
    # Strip pointers and references to find underlying category
    while canonical.kind in (TypeKind.POINTER, TypeKind.LVALUEREFERENCE, TypeKind.RVALUEREFERENCE):
        canonical = canonical.get_pointee()
    
    kind = canonical.kind
    
    category = "unknown"
    
    if kind == TypeKind.VOID:
        category = "void"
    elif kind in (TypeKind.BOOL, TypeKind.CHAR_U, TypeKind.UCHAR, 
                  TypeKind.CHAR16, TypeKind.CHAR32, TypeKind.USHORT, 
                  TypeKind.UINT, TypeKind.ULONG, TypeKind.ULONGLONG, 
                  TypeKind.UINT128, TypeKind.CHAR_S, TypeKind.SCHAR, 
                  TypeKind.WCHAR, TypeKind.SHORT, TypeKind.INT, 
                  TypeKind.LONG, TypeKind.LONGLONG, TypeKind.INT128, 
                  TypeKind.FLOAT, TypeKind.DOUBLE, TypeKind.LONGDOUBLE):
        category = "primitive"
    elif is_pointer and t.get_pointee().kind == TypeKind.CHAR_S: 
        category = "string" 
    elif kind == TypeKind.RECORD:
        if ("std::" in cpp_name or "utils::" in cpp_name) and "string" in cpp_name.lower():
            category = "string"
        elif "CString" in cpp_name or "StaticString" in cpp_name or "ImmutableCString" in cpp_name:
            category = "string"
        elif "std::" in cpp_name and "chrono" in cpp_name:
            category = "chrono"
        elif "tribool" in cpp_name:
            category = "tribool"
        elif "optional" in cpp_name:
            category = "optional"
        elif "std::" in cpp_name and "vector" in cpp_name:
            category = "container"
        elif "std::" in cpp_name and "function" in cpp_name:
            category = "stdfunc"
        else:
            category = "object"
    elif kind == TypeKind.ENUM:
        category = "enum"
        
    # Fixup for const char* which clang might report as Pointer -> Char_S
    if is_pointer:
        pointee = t.get_pointee()
        if pointee.kind in (TypeKind.CHAR_S, TypeKind.UCHAR, TypeKind.SCHAR, TypeKind.CHAR_U):
             category = "string"

    nullability = "unspecified"
    if "_Nullable" in cpp_name:
        nullability = "nullable"
    elif "_Nonnull" in cpp_name:
        nullability = "nonnull"

    return {
        "cpp_name": cpp_name,
        "qualified_name": qualified_name,
        "category": category,
        "is_const": is_const,
        "is_pointer": is_pointer,
        "is_reference": is_reference,
        "is_move_reference": is_move_reference,
        "nullability": nullability
    }

# -----------------------------------------------------------------------------
# Visitor
# -----------------------------------------------------------------------------


def extract_attributes(cursor):
    """
    Extracts [[clang::annotate("...")]] attributes from a cursor.
    """
    attrs = []
    for child in cursor.get_children():
        if child.kind == CursorKind.ANNOTATE_ATTR:
            attrs.append(child.displayname)
    return attrs

def get_cursor_tokens(cursor):
    """Retrieve tokens for a cursor, falling back to physical file offset range if clang_tokenize fails on macro/attribute extents."""
    toks = list(cursor.get_tokens())
    if not toks and cursor.extent.start.file:
        s, e = cursor.extent.start, cursor.extent.end
        tu = cursor.translation_unit
        try:
            rng = clang_cindex.SourceRange.from_locations(
                clang_cindex.SourceLocation.from_offset(tu, s.file, s.offset),
                clang_cindex.SourceLocation.from_offset(tu, e.file, e.offset)
            )
            toks = list(tu.get_tokens(extent=rng))
        except Exception:
            pass
    return toks

def get_default_includes(filename):
    input_abs = os.path.abspath(filename)
    cur = os.path.dirname(input_abs)
    repo_root = None
    while cur and cur != os.path.dirname(cur):
        if os.path.exists(os.path.join(cur, "filament", "include")) and os.path.exists(os.path.join(cur, "libs", "utils")):
            repo_root = cur
            break
        cur = os.path.dirname(cur)
        
    if repo_root:
        return [
            repo_root,
            os.path.join(repo_root, "filament", "include"),
            os.path.join(repo_root, "filament", "backend", "include"),
            os.path.join(repo_root, "libs", "utils", "include"),
            os.path.join(repo_root, "libs", "math", "include"),
            os.path.join(repo_root, "libs", "filabridge", "include"),
            os.path.join(repo_root, "libs", "filaflat", "include"),
        ]
    return []

_DARWIN_SDK_PATH = None
_CLANG_RESOURCE_DIR = None

def get_clang_args(filename, include_paths=None, std="c++17"):
    system = platform.system()
    if system == "Darwin":
        target = "x86_64-apple-darwin"
    elif system == "Windows":
        target = "x86_64-pc-windows-msvc"
    else:
        target = "x86_64-unknown-linux-gnu"

    clang_args = [
        f"-std={std}", 
        "-x", "c++", 
        "-target", target
    ]

    global _DARWIN_SDK_PATH, _CLANG_RESOURCE_DIR
    if system == "Darwin":
        if _DARWIN_SDK_PATH is None:
            try:
                res = subprocess.run(["xcrun", "--show-sdk-path"], capture_output=True, text=True, check=True)
                _DARWIN_SDK_PATH = res.stdout.strip()
            except Exception:
                _DARWIN_SDK_PATH = ""
        if _DARWIN_SDK_PATH:
            clang_args.append(f"-isysroot{_DARWIN_SDK_PATH}")
            clang_args.append(f"-isystem{_DARWIN_SDK_PATH}/usr/include/c++/v1")

    if _CLANG_RESOURCE_DIR is None:
        try:
            res = subprocess.run(["clang", "-print-resource-dir"], capture_output=True, text=True, check=True)
            _CLANG_RESOURCE_DIR = res.stdout.strip()
        except Exception:
            _CLANG_RESOURCE_DIR = ""
    if _CLANG_RESOURCE_DIR:
        clang_args.append(f"-isystem{_CLANG_RESOURCE_DIR}/include")

    all_includes = list(include_paths or [])
    for inc in get_default_includes(filename):
        if inc not in all_includes and os.path.isdir(inc):
            all_includes.append(inc)

    for inc in all_includes:
        clang_args.append(f"-I{inc}")

    return clang_args, all_includes

class Extractor:
    def __init__(self, filename, include_paths=None, std="c++17"):
        self.filename = os.path.abspath(filename)
        all_includes = list(include_paths or [])
        for inc in get_default_includes(self.filename):
            if inc not in all_includes and os.path.isdir(inc):
                all_includes.append(inc)
        self.include_paths = [os.path.abspath(p) for p in all_includes]
        self.output = {
            "meta": {
                "generator_version": "1.0.0",
                "source_file": filename
            },
            "includes": {
                "user": [],
                "system": []
            },
            "aliases": [],
            "enums": [],
            "classes": [],
            "functions": [],
            "referenced_classes": []
        }
        self.std = std
        self.traits = {} # name -> list of types
        self.pending_trait_aliases = {} # alias_name -> target_trait_name
        self._parsed_headers = {self.filename}
        self._source_lines_cache = {}

    def get_source_line(self, file_path, line_num):
        if not file_path or not os.path.isfile(file_path):
            return None
        abs_p = os.path.abspath(file_path)
        if abs_p not in self._source_lines_cache:
            try:
                with open(abs_p, "r", encoding="utf-8", errors="replace") as f:
                    self._source_lines_cache[abs_p] = f.readlines()
            except Exception:
                self._source_lines_cache[abs_p] = []
        lines = self._source_lines_cache[abs_p]
        if 1 <= line_num <= len(lines):
            return lines[line_num - 1]
        return None

    def relativize_path(self, path):
        if not path:
            return ""
        abs_path = os.path.abspath(path)
        
        best_match_path = None
        best_match_len = -1
        
        # Try to find the longest include path that contains this file for most specific match
        for inc in self.include_paths:
            # We want to check if abs_path starts with inc
            # os.path.commonpath might be safest but startswith on abspath usually works if normalized
            if abs_path.startswith(inc):
                # Ensure it's a directory match (avoid /foo matching /foobar)
                # But os.path.abspath usually strips trailing slash.
                # Just using relpath is cleaner.
                try:
                    rel = os.path.relpath(abs_path, inc)
                    if not rel.startswith(".."):
                        if len(inc) > best_match_len:
                            best_match_len = len(inc)
                            best_match_path = rel
                except ValueError:
                    continue
        
        if best_match_path:
            return best_match_path
            
        # Fallback: if it's the input file, we might want to return it relative to CWD if possible?
        # User requested relative to KEY PARAMETERS (includes).
        # If no include matches, keeps it as is (absolute or whatever clang gave, but we have abs_path).
        # We can try to make it relative to CWD for cleanliness if it's NOT in an include path?
        # For now, return original path if no include match, or maybe CWD relative.
        # Let's return the abspath if no match? Or normalized path.
        return path # Return as-is if no match (likely what clang gave or abspath)

    def create_entity(self, name, qualified_name, cursor, attributes=None):
        if attributes is None:
            attributes = []
        
        # Merge with clang attributes
        attributes.extend(extract_attributes(cursor))
        
        # Extract docs
        doc = parse_doxygen(cursor.raw_comment) if cursor.raw_comment else {
            "brief": "", "details": "", "params": {}, "returns": "", "meta": {}
        }
        
        file_path = ""
        if cursor.location.file:
             file_path = self.relativize_path(cursor.location.file.name)

        return {
            "name": name,
            "qualified_name": qualified_name,
            "location": {
                "file": file_path,
                "line": cursor.location.line
            },
            "doc": doc,
            "attributes": attributes
        }

    def resolve_pending_trait_aliases(self):
        changed = True
        iterations = 0
        while changed and iterations < 10:
            changed = False
            iterations += 1
            for alias, target in list(self.pending_trait_aliases.items()):
                target_simple = target.split("::")[-1]
                if target in self.traits:
                    self.traits[alias] = list(self.traits[target])
                    del self.pending_trait_aliases[alias]
                    changed = True
                elif target_simple in self.traits:
                    self.traits[alias] = list(self.traits[target_simple])
                    del self.pending_trait_aliases[alias]
                    changed = True

    def pre_scan_traits(self, tu_cursor):
        def visit(c, parent_scope=""):
            if c.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_TEMPLATE):
                cls_name = c.spelling
                current_scope = f"{parent_scope}::{cls_name}" if parent_scope else cls_name
                try:
                    tokens = [t.spelling for t in c.get_tokens()]
                    self.scan_tokens_for_traits(tokens, scope_prefix=current_scope)
                except Exception:
                    pass
                for child in c.get_children():
                    if child.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_TEMPLATE):
                        visit(child, current_scope)
            elif c.kind == CursorKind.NAMESPACE:
                ns_name = c.spelling
                current_scope = f"{parent_scope}::{ns_name}" if parent_scope else ns_name
                for child in c.get_children():
                    visit(child, current_scope)

        for child in tu_cursor.get_children():
            loc = child.location
            if not loc.file:
                continue
            file_name = os.path.abspath(loc.file.name)
            if any(file_name.startswith(p) for p in self.include_paths):
                visit(child)

        self.resolve_pending_trait_aliases()

    def extract(self, cursor, resolve_references=True):
        if resolve_references:
            self.pre_scan_traits(cursor)
        # We iterate children
        abs_filename = os.path.abspath(self.filename)
        
        for child in cursor.get_children():
            loc = child.location
            if not loc.file:
                continue
            
            # Use absolute paths for comparison
            child_abs_path = os.path.abspath(loc.file.name)
            is_in_main_file = (child_abs_path == abs_filename)
            
            if child.kind == CursorKind.INCLUSION_DIRECTIVE:
                if is_in_main_file:
                    self.handle_inclusion(child)
                continue

            if not is_in_main_file:
                continue

            # Skip out-of-line method definitions (semantic parent is class/struct)
            if child.kind in (CursorKind.FUNCTION_DECL, CursorKind.FUNCTION_TEMPLATE):
                if child.semantic_parent and child.semantic_parent.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
                    continue

            if child.kind == CursorKind.STRUCT_DECL:
                if child.is_definition():
                    self.handle_class(child, "struct")
            elif child.kind == CursorKind.CLASS_DECL:
                if child.is_definition():
                    self.handle_class(child, "object")
            elif child.kind == CursorKind.CLASS_TEMPLATE:
                 if child.is_definition():
                    self.handle_class(child, "object") # Treat templates as objects for now
            elif child.kind == CursorKind.ENUM_DECL:
                self.handle_enum(child)
            elif child.kind in (CursorKind.TYPEDEF_DECL, CursorKind.TYPE_ALIAS_DECL):
                alias = self.create_entity(child.spelling, get_cursor_qualified_name(child), child)
                alias["type"] = get_type_info(child.underlying_typedef_type)
                decl = child.underlying_typedef_type.get_declaration()
                if decl and (not alias.get("doc") or not any(alias["doc"].values())):
                    decl_doc = None
                    if decl.raw_comment:
                        decl_doc = parse_doxygen(decl.raw_comment)
                    if (not decl_doc or not any(decl_doc.values())) and decl.get_definition():
                        defn = decl.get_definition()
                        if defn and defn.raw_comment:
                            decl_doc = parse_doxygen(defn.raw_comment)
                    if decl_doc and any(decl_doc.values()):
                        alias["doc"] = decl_doc
                self.output.setdefault("aliases", []).append(alias)
            elif child.kind == CursorKind.NAMESPACE:
                if child.spelling not in ("filament", "utils", "math", "std", "android", "") and self._namespace_has_functions(child):
                    self.handle_namespace(child)
                else:
                    self.extract(child, resolve_references=False) 

        if resolve_references:
            self.resolve_referenced_types(cursor)

    def handle_inclusion(self, cursor):
        name = cursor.displayname
        # Crude check for system vs user
        args = [t.spelling for t in cursor.get_tokens()]
        
        is_system = False
        if '<' in args and '>' in args:
            is_system = True
        
        # Fallback list
        if name.startswith("std") or name in ["vector", "string", "iostream", "cmath", "math.h", "stddef.h", "stdint.h"]:
             is_system = True
        
        # formatting: if system, ensure it has <>.
        if is_system and not name.startswith("<"):
            out_name = f"<{name}>"
        else:
             out_name = name

        if is_system:
            if out_name not in self.output["includes"]["system"]:
                 self.output["includes"]["system"].append(out_name)
        else:
            if out_name not in self.output["includes"]["user"]:
                self.output["includes"]["user"].append(out_name)

    def extract_trait(self, cursor):
        """
        Attempts to extract supported types from a trait struct like:
        template<typename T> struct is_supported_x {
            using type = std::enable_if_t<
                std::is_same_v<math::float4, T> || ...
            >;
        };
        """
        if cursor.kind != CursorKind.CLASS_TEMPLATE:
            return None
            
        tokens = [t.spelling for t in cursor.get_tokens()]
        # Simple token stream check
        # Look for 'using', 'type', '=', 'std', '::', 'enable_if_t'
        # Then look for 'std', '::', 'is_same_v', '<', TYPE, ',', 'T', '>'
        
        # We assume the last template param is the one we check against
        # But for is_supported_vector_type<T>, T is the first and only.
        
        start_scan = -1
        try:
            # find 'using type =' or just 'enable_if_t'
            for i, t in enumerate(tokens):
                if t == "enable_if_t":
                    start_scan = i
                    break
        except ValueError:
            pass
            
        if start_scan == -1:
            return None
            
        supported_types = []
        i = start_scan
        while i < len(tokens):
            # Look for is_same_v < TYPE , T >
            # or is_same < TYPE , T > :: value
            if tokens[i] == "is_same_v" or tokens[i] == "is_same":
                # Expect <
                if i+1 < len(tokens) and tokens[i+1] == "<":
                    # Capture type until comma
                    # This is tricky if TYPE has template args with comma.
                    # Assuming simple types for now or namespaced types.
                    # Scan until comma
                    type_start = i + 2
                    comma_pos = -1
                    bracket_balance = 0
                    for j in range(type_start, len(tokens)):
                        if tokens[j] == "<": bracket_balance += 1
                        elif tokens[j] == ">": bracket_balance -= 1
                        elif tokens[j] == "," and bracket_balance == 0:
                            comma_pos = j
                            break
                            
                    if comma_pos != -1:
                        # Extract type string
                        # e.g. math :: float4
                        t_parts = tokens[type_start:comma_pos]
                        t_str = "".join(t_parts)
                        # Fix spacing for ::
                        t_str = t_str.replace("::", "::") # dumb join
                        # Actually a better join is needed, or just keep it simple.
                        # Reconstruct properly
                        t_clean = ""
                        for part in t_parts:
                            if part == "::":
                                t_clean += part
                            elif t_clean and t_clean[-1] != ":" and part != ":":
                                t_clean += " " + part
                            else:
                                t_clean += part
                        supported_types.append(t_clean.strip())
            i += 1
            
        if supported_types:
            return supported_types
        return None

    def handle_class(self, cursor, category, is_referenced=False, parent_class=None, alias_name=None):
        name = alias_name if alias_name else cursor.spelling
        display_name = cursor.displayname
        
        # Extract base classes
        bases = []
        base_decls = []
        for child in cursor.get_children():
            if child.kind == CursorKind.CXX_BASE_SPECIFIER:
                bases.append(child.type.spelling)
                b_decl = child.type.get_declaration()
                if b_decl and b_decl.kind in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
                    base_decls.append(b_decl)
        
        cls = self.create_entity(name, get_cursor_qualified_name(cursor), cursor)
        cls["type"] = {
            "cpp_name": name,
            "category": category,
            "is_const": False,
            "is_pointer": False,
            "is_reference": False,
            "is_move_reference": False
        }
        cls["bases"] = bases
        cls["methods"] = []
        cls["fields"] = []
        cls["aliases"] = []
        cls["constants"] = []
        
        # Fallback: Scan tokens for traits FIRST, before processing methods
        tokens = [t.spelling for t in cursor.get_tokens()]
        self.scan_tokens_for_traits(tokens, scope_prefix=cls["name"])
        self.resolve_pending_trait_aliases()
        
        # Check if nested in another class/struct
        if parent_class:
            cls["parent_class"] = parent_class
            cls["is_nested"] = True
        elif cursor.semantic_parent and cursor.semantic_parent.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
            cls["parent_class"] = cursor.semantic_parent.spelling
            cls["is_nested"] = True

        # Check if a cursor represents a polymorphic type
        def is_polymorphic_cursor(c):
            return any(child.is_virtual_method() for child in c.get_children() if child.kind in (CursorKind.CXX_METHOD, CursorKind.DESTRUCTOR))

        # Extract fields and methods from non-polymorphic base structs (if any)
        for b_decl in base_decls:
            if not is_polymorphic_cursor(b_decl):
                for child in b_decl.get_children():
                    if child.kind == CursorKind.FIELD_DECL:
                        if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                            self.handle_field(child, cls)
                    elif child.kind == CursorKind.CXX_METHOD or child.kind == CursorKind.FUNCTION_TEMPLATE:
                        if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                            if child.spelling.startswith("operator"):
                                continue
                            if not any(m["name"] == child.spelling for m in cls["methods"]):
                                self.handle_method(child, cls)

        for child in cursor.get_children():
            # If default access specifier (private for class, public for struct) handling is needed,
            # we can check cursor.access_specifier
            if child.kind == CursorKind.CXX_METHOD or child.kind == CursorKind.FUNCTION_TEMPLATE:
                if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                    if child.spelling.startswith("operator"):
                        continue
                    self.handle_method(child, cls)
            elif child.kind == CursorKind.CONSTRUCTOR:
                if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                    func = self.parse_function(child)
                    func["is_constructor"] = True
                    # Skip copy and move constructors
                    if len(func["arguments"]) == 1:
                        arg_t = func["arguments"][0]["type"]
                        if (arg_t.get("is_reference") or arg_t.get("is_move_reference")) and (name in arg_t.get("cpp_name", "") or name in arg_t.get("qualified_name", "")):
                            continue
                    cls["methods"].append(func)
            elif child.kind == CursorKind.FIELD_DECL:
                if child.access_specifier in (clang_cindex.AccessSpecifier.PUBLIC, clang_cindex.AccessSpecifier.INVALID):
                    self.handle_field(child, cls)
            elif child.kind == CursorKind.UNION_DECL:
                if child.access_specifier in (clang_cindex.AccessSpecifier.PUBLIC, clang_cindex.AccessSpecifier.INVALID):
                    for u_child in child.get_children():
                        if u_child.kind == CursorKind.FIELD_DECL:
                            if u_child.access_specifier in (clang_cindex.AccessSpecifier.PUBLIC, clang_cindex.AccessSpecifier.INVALID):
                                self.handle_field(u_child, cls)
            elif child.kind == CursorKind.VAR_DECL:
                if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                    self.handle_constant(child, cls)
            elif child.kind == CursorKind.TYPEDEF_DECL or child.kind == CursorKind.TYPE_ALIAS_DECL:
                if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                    self.handle_alias(child, cls)
            elif child.kind == CursorKind.ENUM_DECL:
                if child.access_specifier == clang_cindex.AccessSpecifier.PUBLIC:
                    self.handle_enum(child, cls)
            elif child.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_TEMPLATE):
                # Check for nested traits
                trait_types = self.extract_trait(child)
                if trait_types:
                    self.traits[child.spelling] = trait_types
                elif child.is_definition():
                    # Handle nested class
                    cat = "object"
                    if child.kind == CursorKind.STRUCT_DECL:
                        cat = "struct"
                    self.handle_class(child, cat, is_referenced=is_referenced, parent_class=cls["name"])
                
        # Check if aggregate struct (pure POD/value struct with only public fields)
        is_poly = is_polymorphic_cursor(cursor) or any(is_polymorphic_cursor(b) for b in base_decls)

        all_fields = [child for child in cursor.get_children() if child.kind == CursorKind.FIELD_DECL]
        for b_decl in base_decls:
            all_fields.extend([c for c in b_decl.get_children() if c.kind == CursorKind.FIELD_DECL])
        has_non_public_field = any(f.access_specifier != clang_cindex.AccessSpecifier.PUBLIC for f in all_fields)
        
        is_aggregate = (not is_poly and len(cls["fields"]) > 0 and not has_non_public_field)
        cls["is_aggregate"] = is_aggregate
        if is_aggregate:
            cls["type"]["category"] = "struct"

        is_utility = (not is_poly and len(bases) == 0 and len(cls["fields"]) == 0 and len(cls["methods"]) > 0 and all(m.get("is_static", False) for m in cls["methods"]))
        if is_utility:
            cls["category"] = "utility"
            cls["is_utility"] = True

        is_builder = (name == "Builder" or any("BuilderBase" in b for b in bases))
        if is_builder:
            cls["is_builder"] = True
            cls["category"] = "builder"
            cls["archetype"] = "builder"

        # Check for bitfield archetype auto-deduction:
        # 1. Direct bitfield struct: all non-static fields are bitfields
        # 2. Wrapper class: single non-static data member whose type is a struct comprised solely of bitfields
        is_direct_bitfield = (not is_poly and not is_builder and not is_utility and len(all_fields) > 0 and all(f.is_bitfield() for f in all_fields))

        is_wrapper_bitfield = False
        wrapper_sub_fields = []
        if not is_poly and not is_builder and not is_utility and len(all_fields) == 1 and not all_fields[0].is_bitfield():
            canonical_type = all_fields[0].type.get_canonical()
            field_decl = canonical_type.get_declaration()
            if field_decl.kind in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
                sub_fields = [c for c in field_decl.get_children() if c.kind == CursorKind.FIELD_DECL]
                if len(sub_fields) > 0 and all(sf.is_bitfield() for sf in sub_fields):
                    is_wrapper_bitfield = True
                    wrapper_sub_fields = sub_fields

        has_bitfield_attr = any(
            attr in ("filament:apigen:bitfield", "filament:bitfield", "apigen:bitfield", "bitfield")
            for attr in cls.get("attributes", [])
        )

        if is_direct_bitfield or is_wrapper_bitfield or has_bitfield_attr:
            raw_size = cursor.type.get_size()
            if raw_size > 0:
                bitfield_size = raw_size
            elif is_direct_bitfield:
                total_bits = sum(f.get_bitfield_width() for f in all_fields if f.is_bitfield())
                bitfield_size = (total_bits + 7) // 8
            elif is_wrapper_bitfield:
                total_bits = sum(sf.get_bitfield_width() for sf in wrapper_sub_fields if sf.is_bitfield())
                bitfield_size = (total_bits + 7) // 8
            else:
                bitfield_size = 4

            if bitfield_size <= 8:
                cls["archetype"] = "bitfield"
                cls["category"] = "bitfield"
                cls["is_aggregate"] = False
                cls["bitfield_size"] = bitfield_size
                if bitfield_size <= 2:
                    cls["bitfield_primitive"] = "short"
                elif bitfield_size <= 4:
                    cls["bitfield_primitive"] = "int"
                else:
                    cls["bitfield_primitive"] = "long"
            else:
                # sizeof > 64 bits (> 8 bytes): class cannot be backed by a primitive register in Java.
                # Skip bitfield deduction and proceed as-if it was not a bitfield.
                pass
                
        # Check if struct originates from an options header (e.g. filament/Options.h)
        # or is a nested options / picking result POD struct
        file_path = cls.get("location", {}).get("file", "")
        effective_parent = cls.get("parent_class") or parent_class
        if file_path.endswith("Options.h"):
            cls["is_pojo_struct"] = True
            cls["archetype"] = "pojo_struct"
        elif effective_parent == "View" and name == "PickingQueryResult":
            cls["is_pojo_struct"] = True
            cls["archetype"] = "pojo_struct"
        elif effective_parent == "Engine" and name == "Config":
            cls["is_pojo_struct"] = True
            cls["archetype"] = "pojo_struct"
            cls["is_aggregate"] = False

        if is_referenced:
            self.output.setdefault("referenced_classes", []).append(cls)
        else:
            self.output["classes"].append(cls)

    def scan_tokens_for_traits(self, tokens, scope_prefix=""):
        # State machine to find "template ... using NAME = std::enable_if_t ..."
        # OR "using NAME = std::enable_if_t ..."
        # OR "using NAME = TargetTrait<T>;"
        
        i = 0
        while i < len(tokens):
            if tokens[i] == "using":
                # Check if next is name, then =
                if i + 2 < len(tokens) and tokens[i + 2] == "=":
                    name = tokens[i + 1]
                    # Find terminating semicolon
                    semi_pos = -1
                    for k in range(i + 3, min(i + 300, len(tokens))):
                        if tokens[k] == ";":
                            semi_pos = k
                            break
                    if semi_pos != -1:
                        subset = tokens[i + 3:semi_pos]
                        # Check if enable_if_t in subset
                        enable_idx = -1
                        for idx, tok in enumerate(subset):
                            if tok == "enable_if_t":
                                enable_idx = idx
                                break
                        if enable_idx != -1:
                            trait_types = self.parse_types_from_enable_if(subset[enable_idx:])
                            if trait_types:
                                self.traits[name] = trait_types
                                if scope_prefix:
                                    self.traits[f"{scope_prefix}::{name}"] = trait_types
                        else:
                            # Check if alias to another trait: Trait<T> or Scope::Trait<T>
                            bracket_idx = -1
                            for idx, tok in enumerate(subset):
                                if tok == "<":
                                    bracket_idx = idx
                                    break
                            if bracket_idx != -1:
                                target = "".join(subset[:bracket_idx]).strip()
                            else:
                                target = "".join(subset).strip()
                            target_simple = target.split("::")[-1]
                            if target in self.traits:
                                self.traits[name] = list(self.traits[target])
                                if scope_prefix:
                                    self.traits[f"{scope_prefix}::{name}"] = list(self.traits[target])
                            elif target_simple in self.traits:
                                self.traits[name] = list(self.traits[target_simple])
                                if scope_prefix:
                                    self.traits[f"{scope_prefix}::{name}"] = list(self.traits[target_simple])
                            elif target:
                                self.pending_trait_aliases[name] = target
                                if scope_prefix:
                                    self.pending_trait_aliases[f"{scope_prefix}::{name}"] = target
                        i = semi_pos
            i += 1

    def parse_types_from_enable_if(self, tokens):
        # Expected starts with enable_if_t
        # Look for is_same_v < TYPE, T >
        supported_types = []
        i = 0
        while i < len(tokens):
            if tokens[i] == "is_same_v" or tokens[i] == "is_same":
                # Expect < (maybe space)
                # Find next <
                next_bracket = -1
                for k in range(i+1, min(i+5, len(tokens))):
                    if tokens[k] == "<":
                        next_bracket = k
                        break
                
                if next_bracket != -1:
                    # Capture type until comma
                    i = next_bracket + 1
                    type_start = i
                    comma_pos = -1
                    bracket_balance = 0
                    while i < len(tokens):
                        if tokens[i] == "<": bracket_balance += 1
                        elif tokens[i] == ">": bracket_balance -= 1
                        elif tokens[i] == "," and bracket_balance == 0:
                            comma_pos = i
                            break
                        elif tokens[i] == ";" or tokens[i] == "}": # End of statement safety
                            break
                        i += 1
                    
                    if comma_pos != -1:
                        # Extract type string
                        t_parts = tokens[type_start:comma_pos]
                        # Clean up "math :: float3" -> "math::float3"
                        t_clean = ""
                        for part in t_parts:
                            if part == "::":
                                t_clean += part
                            elif t_clean and t_clean[-1] != ":" and part != ":":
                                t_clean += " " + part
                            else:
                                t_clean += part
                        supported_types.append(t_clean.strip())
            i += 1
            # Stop if we hit a semicolon (end of using decl)
            if i < len(tokens) and tokens[i] == ";":
                break
        return supported_types

    def handle_field(self, cursor, cls):
        field = self.create_entity(cursor.spelling, cursor.spelling, cursor)
        field["type"] = get_type_info(cursor.type)
        
        # Extract default value if present
        # Heuristic: scan tokens for '='
        default_val = None
        tokens = list(cursor.get_tokens())
        for i, tok in enumerate(tokens):
            if tok.spelling == '=':
                # Capture everything after '=' until semicolon
                val_tokens = []
                for t in tokens[i+1:]:
                    if t.spelling == ';':
                        break
                    val_tokens.append(t.spelling)
                default_val = " ".join(val_tokens)
                break
        
        if default_val is not None:
            clean_default_val = (
                default_val
                .replace(" :: ", "::")
                .replace(" ,", ",")
                .replace("{ ", "{")
                .replace(" }", "}")
            )
            field["default_value"] = clean_default_val
        else:
            # Check for brace initialization without '=' e.g. Entity renderable{}; float depth{};
            token_spellings = [t.spelling for t in tokens]
            if "{" in token_spellings and "}" in token_spellings:
                open_idx = token_spellings.index("{")
                close_idx = token_spellings.index("}")
                if close_idx >= open_idx:
                    inner = token_spellings[open_idx:close_idx+1]
                    field["default_value"] = "".join(inner)
            else:
                field["default_value"] = None

        # Extract comment directives (%codegen_java_float%, %codegen_java_flatten%, etc.)
        loc = cursor.location
        line_text = self.get_source_line(loc.file.name, loc.line) if loc.file else None
        
        tags = set(re.findall(r"%codegen_([a-zA-Z0-9_]+)%", line_text or ""))
        if cursor.raw_comment:
            tags.update(re.findall(r"%codegen_([a-zA-Z0-9_]+)%", cursor.raw_comment))
        
        for tag in tags:
            if tag == "java_float":
                if "apigen:java_type:float" not in field["attributes"]:
                    field["attributes"].append("apigen:java_type:float")
            elif tag == "java_flatten":
                if "apigen:flatten" not in field["attributes"]:
                    field["attributes"].append("apigen:flatten")
            else:
                attr_name = f"codegen:{tag}"
                if attr_name not in field["attributes"]:
                    field["attributes"].append(attr_name)

        is_bitfield = cursor.is_bitfield()
        field["is_bitfield"] = is_bitfield
        if is_bitfield:
            field["bitfield_width"] = cursor.get_bitfield_width()

        cls["fields"].append(field)

    def handle_constant(self, cursor, cls):
        const_obj = self.create_entity(cursor.spelling, get_cursor_qualified_name(cursor), cursor)
        const_obj["type"] = get_type_info(cursor.type)
        
        # Check if initialized from another referenced VAR_DECL
        ref_cursor = None
        for child in cursor.get_children():
            if child.referenced and child.referenced.kind == CursorKind.VAR_DECL and child.referenced != cursor:
                ref_cursor = child.referenced
                break
        
        target_cursor = ref_cursor if ref_cursor else cursor
        tokens = list(target_cursor.get_tokens())
        val_tokens = []
        eq_idx = -1
        for i, tok in enumerate(tokens):
            if tok.spelling == '=':
                eq_idx = i
                break
        if eq_idx != -1:
            for t in tokens[eq_idx+1:]:
                if t.spelling == ';':
                    break
                val_tokens.append(t.spelling)
        
        raw_val = " ".join(val_tokens)
        const_obj["raw_value"] = raw_val
        clean_val = raw_val.replace(" :: ", "::").replace(" ( ", "(").replace(" )", ")")
        
        # If this constant aliases a constant already defined in this class, reference it by name
        if ref_cursor and any(c.get("name") == ref_cursor.spelling for c in cls.get("constants", [])):
            clean_val = ref_cursor.spelling
            
        const_obj["value"] = clean_val
        cls.setdefault("constants", []).append(const_obj)

    def handle_alias(self, cursor, cls):
        alias = self.create_entity(cursor.spelling, get_cursor_qualified_name(cursor), cursor)
        skip_attrs = ("filament:apigen:skip", "filament:skip", "apigen:skip", "no_apigen", "noapigen", "skip_generation", "binding:skip")
        is_skipped = any(a in skip_attrs or a.startswith("no_apigen") or a.startswith("skip_generation") for a in alias.get("attributes", []))
        if is_skipped:
            return

        alias["type"] = get_type_info(cursor.underlying_typedef_type)
        
        decl = cursor.underlying_typedef_type.get_declaration()
        decl_doc = None
        if decl:
            if decl.raw_comment:
                decl_doc = parse_doxygen(decl.raw_comment)
            if (not decl_doc or not any(decl_doc.values())) and decl.get_definition():
                defn = decl.get_definition()
                if defn and defn.raw_comment:
                    decl_doc = parse_doxygen(defn.raw_comment)

        if decl and (not alias.get("doc") or not any(alias["doc"].values())):
            if decl_doc and any(decl_doc.values()):
                alias["doc"] = decl_doc
        cls["aliases"].append(alias)

        if decl and decl.kind not in (CursorKind.ENUM_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
            can_decl = cursor.underlying_typedef_type.get_canonical().get_declaration()
            if can_decl and can_decl.kind in (CursorKind.ENUM_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
                decl = can_decl
        
        if decl and decl.kind == CursorKind.ENUM_DECL:
            enum_obj = self.create_entity(cursor.spelling, get_cursor_qualified_name(cursor), cursor)
            if not enum_obj.get("doc") or not any(enum_obj["doc"].values()):
                if decl_doc and any(decl_doc.values()):
                    enum_obj["doc"] = decl_doc
            for attr in extract_attributes(decl):
                if attr not in enum_obj["attributes"]:
                    enum_obj["attributes"].append(attr)
            if any(a in ("filament:apigen:flags", "filament:flags", "apigen:flags", "flags", "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask") for a in enum_obj["attributes"]):
                enum_obj["is_flags"] = True
            enum_obj["underlying_type"] = get_type_info(decl.enum_type).get("cpp_name", "int")
            enum_obj["entries"] = []
            for child in decl.get_children():
                if child.kind == CursorKind.ENUM_CONSTANT_DECL:
                    entry = {
                        "name": child.spelling,
                        "value": child.enum_value,
                        "doc": parse_doxygen(child.raw_comment)
                    }
                    enum_obj["entries"].append(entry)
            skip_attrs = ("filament:apigen:skip", "filament:skip", "apigen:skip", "no_apigen", "noapigen", "skip_generation", "binding:skip")
            is_skipped_enum = any(a in skip_attrs or a.startswith("no_apigen") or a.startswith("skip_generation") for a in enum_obj.get("attributes", []))
            if enum_obj["entries"] and not is_skipped_enum:
                cls.setdefault("enums", []).append(enum_obj)
        elif decl and decl.kind in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL):
            defn = decl.get_definition() or (decl if decl.is_definition() else None)
            if defn:
                # Check if this struct has already been ingested into classes for this parent
                existing = any(
                    c.get("name") == cursor.spelling and c.get("parent_class") == cls["name"]
                    for c in self.output.get("classes", [])
                )
                if not existing:
                    self.handle_class(
                        defn,
                        "struct" if defn.kind == CursorKind.STRUCT_DECL else "object",
                        is_referenced=False,
                        parent_class=cls["name"],
                        alias_name=cursor.spelling
                    )

    def handle_method(self, cursor, cls):
        func = self.parse_function(cursor)
        cls["methods"].append(func)

    def _namespace_has_functions(self, cursor):
        abs_filename = os.path.abspath(self.filename)
        for child in cursor.get_children():
            loc = child.location
            if loc.file and os.path.abspath(loc.file.name) == abs_filename:
                if child.kind in (CursorKind.FUNCTION_DECL, CursorKind.FUNCTION_TEMPLATE):
                    return True
        return False

    def handle_namespace(self, cursor):
        abs_filename = os.path.abspath(self.filename)
        funcs = []
        enums = []
        constants = []
        
        for child in cursor.get_children():
            loc = child.location
            if not loc.file or os.path.abspath(loc.file.name) != abs_filename:
                continue
            if child.kind in (CursorKind.FUNCTION_DECL, CursorKind.FUNCTION_TEMPLATE):
                func = self.parse_function(child)
                func["is_static"] = True
                funcs.append(func)
            elif child.kind == CursorKind.ENUM_DECL:
                enums.append(self.handle_enum(child))
            elif child.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL, CursorKind.CLASS_TEMPLATE):
                if child.is_definition():
                    self.handle_class(child, "struct" if child.kind == CursorKind.STRUCT_DECL else "object")
            elif child.kind == CursorKind.NAMESPACE:
                self.extract(child)

        cls = self.create_entity(cursor.spelling, get_cursor_qualified_name(cursor), cursor)
        cls["category"] = "utility"
        cls["is_namespace"] = True
        cls["is_utility"] = True
        cls["methods"] = funcs
        cls["enums"] = enums
        cls["constants"] = constants
        cls["fields"] = []
        cls["bases"] = []
        cls["doc"] = parse_doxygen(cursor.raw_comment)
        self.output["classes"].append(cls)

    def handle_function(self, cursor):
        func = self.parse_function(cursor)
        self.output["functions"].append(func)

    def parse_function(self, cursor):
        name = cursor.spelling
        display_name = cursor.displayname
        
        template_params = []
        constraint_str = None
        
        if cursor.kind == CursorKind.FUNCTION_TEMPLATE:
            # First pass: Extract explicit constraints from tokens (heuristic)
            tokens = [t.spelling for t in cursor.get_tokens()]
            sig = " ".join(tokens)

            
            # Allow spaces in template args
            match = re.search(r'(std::enable_if_t\s*<.*?>|is_supported_parameter_t\s*<.*?>)', sig)
            if match:
                constraint_str = match.group(1)
            
            # Second pass: Visit template parameters
            for child in cursor.get_children():
                if child.kind == CursorKind.TEMPLATE_TYPE_PARAMETER:
                    param_name = child.spelling
                    if not param_name:
                        # Unnamed parameter, likely SFINAE
                        # Try to extract default value
                        # We can look at tokens of this child
                        child_tokens = list(child.get_tokens())
                        # "typename", "=", "...", "class", "..."
                        # find '='
                        eq_idx = -1
                        for i, t in enumerate(child_tokens):
                            if t.spelling == '=':
                                eq_idx = i
                                break
                        
                        if eq_idx != -1:
                            # Join with space to preserve readability
                            default_val = " ".join([t.spelling for t in child_tokens[eq_idx+1:]])
                            # It's a constraint!
                            if constraint_str:
                                constraint_str += " && " + default_val
                            else:
                                constraint_str = default_val
                    else:
                        template_params.append({
                            "name": param_name,
                            "type": "typename" 
                        })
                elif child.kind == CursorKind.TEMPLATE_NON_TYPE_PARAMETER:
                     template_params.append({
                        "name": child.spelling,
                        "type": child.type.spelling
                    })

        func = self.create_entity(name, display_name, cursor)
        func["return_type"] = get_type_info(cursor.result_type)

        # Check for return type nullability qualifiers (_Nullable/_Nonnull/UTILS_NULLABLE/UTILS_NONNULL)
        tokens_before_name = []
        for tok in get_cursor_tokens(cursor):
            if tok.spelling == name or tok.spelling == "(":
                break
            tokens_before_name.append(tok.spelling)

        if "UTILS_NULLABLE" in tokens_before_name or "_Nullable" in tokens_before_name:
            func["return_type"]["nullability"] = "nullable"
        elif "UTILS_NONNULL" in tokens_before_name or "_Nonnull" in tokens_before_name:
            func["return_type"]["nullability"] = "nonnull"

        func["arguments"] = []
        func["is_static"] = cursor.is_static_method()
        func["is_const"] = cursor.is_const_method()

        is_noexcept = False
        if hasattr(cursor, "exception_specification_kind"):
            esk = cursor.exception_specification_kind
            if esk in (clang_cindex.ExceptionSpecificationKind.BASIC_NOEXCEPT,
                       clang_cindex.ExceptionSpecificationKind.DYNAMIC_NONE):
                is_noexcept = True
            elif esk == clang_cindex.ExceptionSpecificationKind.COMPUTED_NOEXCEPT:
                tokens = [t.spelling for t in cursor.get_tokens()]
                if "noexcept" in tokens:
                    idx = tokens.index("noexcept")
                    if idx + 2 < len(tokens) and tokens[idx+1] == "(" and tokens[idx+2] == "false":
                        is_noexcept = False
                    else:
                        is_noexcept = True
        func["is_noexcept"] = is_noexcept
        func["template_parameters"] = template_params
        func["constraint"] = constraint_str
        func["specializations"] = []

        if constraint_str:
            # Try to resolve specializations
            param_map = {} # ParamName -> [Types]
            
            for trait_name, types in self.traits.items():
                pattern = r"(?:[\w:]+::)?" + re.escape(trait_name) + r"\s*<\s*(\w+)\s*>"
                matches = re.findall(pattern, constraint_str)
                for param_name in matches:
                    param_map[param_name] = types
            
            if param_map:
                # Generate cartesian product
                params = list(param_map.keys())
                values = list(param_map.values())
                
                combinations = list(itertools.product(*values))
                
                for combo in combinations:
                    spec = {}
                    for i, val in enumerate(combo):
                        spec[params[i]] = val
                    func["specializations"].append(spec)

        # Extract arguments using child traversal for robustness (SFINAE often breaks .get_arguments())
        # We manually look for PARM_DECL children
        
        args = []
        for child in cursor.get_children():
            if child.kind == CursorKind.PARM_DECL:
                arg_obj = {
                    "name": child.spelling,
                    "type": get_type_info(child.type),
                    "default_value": None
                }
                # Default value check
                tokens = list(child.get_tokens())
                token_spellings = [t.spelling for t in tokens]

                if "UTILS_NULLABLE" in token_spellings or "_Nullable" in token_spellings:
                     arg_obj["type"]["nullability"] = "nullable"
                elif "UTILS_NONNULL" in token_spellings or "_Nonnull" in token_spellings:
                     arg_obj["type"]["nullability"] = "nonnull"

                # Attributes on argument
                arg_obj["attributes"] = extract_attributes(child)

                for i, tok in enumerate(tokens):
                    if tok.spelling == '=':
                        val = " ".join([t.spelling for t in tokens[i+1:]])
                        # remove trailing comma or paren if captured? 
                        # tokens usually stop at the end of the decl.
                        arg_obj["default_value"] = val
                        break
                args.append(arg_obj)
        
        func["arguments"] = args

        # Mark arguments and return_type that reference template parameters
        tmpl_names = [tp["name"] for tp in func.get("template_parameters", [])]
        if tmpl_names:
            for arg in func.get("arguments", []):
                arg_t = arg["type"]
                cpp_n = arg_t.get("cpp_name", "")
                qual_n = arg_t.get("qualified_name", "")
                for tp_name in tmpl_names:
                    if re.search(r'\b' + re.escape(tp_name) + r'\b', cpp_n) or "type-parameter-" in qual_n:
                        arg_t["is_template_param"] = True
                        arg_t["template_param_name"] = tp_name
                        break

            ret_t = func.get("return_type", {})
            cpp_n = ret_t.get("cpp_name", "")
            qual_n = ret_t.get("qualified_name", "")
            for tp_name in tmpl_names:
                if re.search(r'\b' + re.escape(tp_name) + r'\b', cpp_n) or "type-parameter-" in qual_n:
                    ret_t["is_template_param"] = True
                    ret_t["template_param_name"] = tp_name
                    break

        return func

    def handle_enum(self, cursor, parent=None):
        enum_obj = self.create_entity(cursor.spelling, get_cursor_qualified_name(cursor), cursor)
        if any(a in ("filament:apigen:flags", "filament:flags", "apigen:flags", "flags", "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask") for a in enum_obj["attributes"]):
            enum_obj["is_flags"] = True
        enum_obj["underlying_type"] = get_type_info(cursor.enum_type).get("cpp_name", "int")
        enum_obj["entries"] = []
        
        for child in cursor.get_children():
            if child.kind == CursorKind.ENUM_CONSTANT_DECL:
                entry = {
                    "name": child.spelling,
                    "value": child.enum_value,
                    "doc": parse_doxygen(child.raw_comment)
                }
                enum_obj["entries"].append(entry)
                
        if parent is not None:
            parent.setdefault("enums", []).append(enum_obj)
        else:
            self.output["enums"].append(enum_obj)

    def find_header_for_type(self, type_name):
        if not hasattr(self, "_header_by_type_cache"):
            self._header_by_type_cache = {}
        if type_name in self._header_by_type_cache:
            return self._header_by_type_cache[type_name]

        clean = re.sub(r'<.*>', '', type_name).strip()
        clean = re.sub(r'\b(const|volatile|struct|class|_Nonnull|_Nullable|UTILS_NONNULL|UTILS_NULLABLE)\b', '', clean)
        clean = clean.replace('*', '').replace('&', '').strip()
        parts = [p.strip() for p in clean.split('::') if p.strip()]
        if not parts:
            self._header_by_type_cache[type_name] = None
            return None
        simple_name = parts[-1]
        if not re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', simple_name):
            self._header_by_type_cache[type_name] = None
            return None
        if simple_name in (
            'void', 'bool', 'char', 'int', 'short', 'long', 'float', 'double',
            'int8_t', 'uint8_t', 'int16_t', 'uint16_t', 'int32_t', 'uint32_t',
            'int64_t', 'uint64_t', 'size_t', 'ssize_t', 'intptr_t', 'uintptr_t',
            'ptrdiff_t', 'string', 'string_view', 'vector', 'function', 'optional',
            'shared_ptr', 'unique_ptr', 'weak_ptr', 'tuple', 'pair', 'int2', 'int3',
            'int4', 'uint2', 'uint3', 'uint4', 'float2', 'float3', 'float4',
            'double2', 'double3', 'double4', 'mat2', 'mat3', 'mat4', 'mat2f', 'mat3f',
            'mat4f', 'quat', 'quatf', 'TVec2', 'TVec3', 'TVec4', 'TMat22', 'TMat33', 'TMat44',
            'TQuaternion', 'Entity', 'EntityInstance', 'BufferDescriptor'
        ):
            self._header_by_type_cache[type_name] = None
            return None
        candidates = [
            f'{simple_name}.h',
            f'filament/{simple_name}.h',
            f'filament/backend/{simple_name}.h',
            f'backend/{simple_name}.h',
            f'utils/{simple_name}.h',
            f'math/{simple_name}.h',
        ]
        if len(parts) > 1:
            candidates.insert(0, '/'.join(parts) + '.h')
            if parts[0] == 'filament':
                candidates.insert(1, '/'.join(parts[1:]) + '.h')
        for inc in self.include_paths:
            for cand in candidates:
                cand_path = os.path.join(inc, cand)
                if os.path.isfile(cand_path):
                    res = os.path.abspath(cand_path)
                    self._header_by_type_cache[type_name] = res
                    return res

        pattern = re.compile(r'\b(struct|class)\s+(?:UTILS_PUBLIC\s+)?' + re.escape(simple_name) + r'\b')
        for inc in self.include_paths:
            # Skip searching repo root directory recursively to avoid scanning out/ and build/
            if os.path.isdir(os.path.join(inc, 'filament', 'include')) and os.path.isdir(os.path.join(inc, 'libs', 'utils')):
                continue
            for root, dirs, files in os.walk(inc):
                dirs[:] = [d for d in dirs if d not in ('.git', 'out', 'build', 'third_party')]
                for f in files:
                    if f.endswith('.h'):
                        full = os.path.join(root, f)
                        try:
                            with open(full, 'r', encoding='utf-8', errors='ignore') as fp:
                                content = fp.read()
                                if pattern.search(content):
                                    res = os.path.abspath(full)
                                    self._header_by_type_cache[type_name] = res
                                    return res
                        except Exception:
                            pass
        self._header_by_type_cache[type_name] = None
        return None

    def resolve_referenced_types(self, tu_cursor=None):
        known_names = set()
        for c in self.output["classes"]:
            known_names.add(c["name"])
            if "qualified_name" in c:
                known_names.add(c["qualified_name"])
            if "filament::" in c.get("qualified_name", ""):
                known_names.add(c["qualified_name"].replace("filament::", ""))
            if "backend::" in c.get("qualified_name", ""):
                known_names.add(c["qualified_name"].replace("backend::", ""))

        def extract_candidate_names(t_dict):
            if not t_dict or not isinstance(t_dict, dict):
                return []
            names = []
            for k in ("cpp_name", "qualified_name"):
                val = t_dict.get(k)
                if val:
                    cleaned = re.sub(r'<.*>', '', val)
                    cleaned = re.sub(r'\b(const|volatile|struct|class|_Nonnull|_Nullable|UTILS_NONNULL|UTILS_NULLABLE)\b', '', cleaned)
                    cleaned = cleaned.replace('*', '').replace('&', '').strip()
                    if cleaned:
                        names.append(cleaned)
            return names

        # Scan definitions present in current TU (from included headers)
        tu_defined = {}
        if tu_cursor:
            def scan_tu(c):
                if c.kind in (CursorKind.STRUCT_DECL, CursorKind.CLASS_DECL) and c.is_definition():
                    sname = c.spelling
                    qname = get_cursor_qualified_name(c)
                    if sname:
                        tu_defined[sname] = c
                    if qname:
                        tu_defined[qname] = c
                for ch in c.get_children():
                    scan_tu(ch)
            scan_tu(tu_cursor)

        queue = collections.deque()
        for cls in self.output["classes"]:
            for b in cls.get("bases", []):
                queue.append(b)
            for f in cls.get("fields", []):
                queue.extend(extract_candidate_names(f.get("type")))
            for m in cls.get("methods", []):
                queue.extend(extract_candidate_names(m.get("return_type")))
                for a in m.get("arguments", []):
                    queue.extend(extract_candidate_names(a.get("type")))
        for fn in self.output.get("functions", []):
            queue.extend(extract_candidate_names(fn.get("return_type")))
            for a in fn.get("arguments", []):
                queue.extend(extract_candidate_names(a.get("type")))

        visited = set()
        sub_index = None
        while queue:
            type_name = queue.popleft()
            if type_name in visited:
                continue
            visited.add(type_name)

            simple = type_name.split("::")[-1]
            if (type_name in known_names or 
                simple in known_names or 
                f"filament::{simple}" in known_names or
                f"backend::{simple}" in known_names):
                continue

            if simple in (
                "void", "bool", "char", "int", "short", "long", "float", "double",
                "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
                "int64_t", "uint64_t", "size_t", "ssize_t", "intptr_t", "uintptr_t",
                "ptrdiff_t", "string", "string_view", "vector", "function", "optional",
                "shared_ptr", "unique_ptr", "weak_ptr", "tuple", "pair", "int2", "int3",
                "int4", "uint2", "uint3", "uint4", "float2", "float3", "float4",
                "double2", "double3", "double4", "mat2", "mat3", "mat4", "mat2f", "mat3f",
                "mat4f", "quat", "quatf", "TVec2", "TVec3", "TVec4", "TMat22", "TMat33", "TMat44",
                "TQuaternion", "Entity", "EntityInstance", "BufferDescriptor"
            ):
                continue

            # Check if defined in current TU
            cur = tu_defined.get(type_name) or tu_defined.get(simple)
            if cur:
                cat = "struct" if cur.kind == CursorKind.STRUCT_DECL else "object"
                self.handle_class(cur, cat, is_referenced=True)
                added_cls = self.output["referenced_classes"][-1]
                known_names.add(added_cls["name"])
                if "qualified_name" in added_cls:
                    known_names.add(added_cls["qualified_name"])
                for f in added_cls.get("fields", []):
                    queue.extend(extract_candidate_names(f.get("type")))
                continue

            # Discover header
            h = self.find_header_for_type(type_name)
            if h and h not in self._parsed_headers:
                self._parsed_headers.add(h)
                clang_args, all_includes = get_clang_args(h, include_paths=self.include_paths, std=self.std)
                if sub_index is None:
                    sub_index = clang_cindex.Index.create()
                sub_tu = sub_index.parse(h, args=clang_args, options=clang_cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)
                if sub_tu:
                    sub_ex = Extractor(h, include_paths=all_includes, std=self.std)
                    sub_ex.extract(sub_tu.cursor, resolve_references=False)
                    for sub_c in sub_ex.output["classes"]:
                        if sub_c["name"] not in known_names and sub_c.get("qualified_name") not in known_names:
                            self.output.setdefault("referenced_classes", []).append(sub_c)
                            known_names.add(sub_c["name"])
                            if "qualified_name" in sub_c:
                                known_names.add(sub_c["qualified_name"])
                            for f in sub_c.get("fields", []):
                                queue.extend(extract_candidate_names(f.get("type")))

# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="C++ API Extractor")
    parser.add_argument("input", help="Input C++ header file")
    parser.add_argument("-I", "--include", action="append", help="Include search paths", default=[])
    parser.add_argument("-o", "--output", help="Output JSON file", default=None)
    parser.add_argument("--std", help="C++ Standard", default="c++17")
    parser.add_argument("--ignore-header", action="append", dest="ignore_headers", help="Headers to ignore (treat as empty)", default=[])
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"Error: File {args.input} not found.", file=sys.stderr)
        sys.exit(1)

    clang_args, all_includes = get_clang_args(args.input, include_paths=args.include, std=args.std)

    # Option 2: Virtual Header Remapping
    unsaved_files = []
    if args.ignore_headers:
        for header in args.ignore_headers:
            unsaved_files.append((header, ""))

    # Parse
    index = clang_cindex.Index.create()
    tu = index.parse(
        args.input, 
        args=clang_args, 
        unsaved_files=unsaved_files,
        options=clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
    )
    
    did_fail = False
    for diag in tu.diagnostics:
        if diag.severity >= clang_cindex.Diagnostic.Error:
            # Option 3: Diagnostic Filtering
            # Check if this error is about an ignored header
            is_ignored = False
            if args.ignore_headers:
                for header in args.ignore_headers:
                    # Error messages usually look like: "'header.h' file not found"
                    if header in diag.spelling and "file not found" in diag.spelling:
                        is_ignored = True
                        break
            
            if is_ignored:
                print(f"Warning: Ignoring missing header error: {diag.spelling}", file=sys.stderr)
            else:
                print(f"Clang Parse Error: {diag.spelling}", file=sys.stderr)
                did_fail = True
    
    if did_fail:
        print("Parsing failed: Errors during AST generation.", file=sys.stderr)
        sys.exit(1)

    extractor = Extractor(os.path.abspath(args.input), include_paths=all_includes, std=args.std)
    extractor.extract(tu.cursor)
    
    json_str = json.dumps(extractor.output, indent=2)
    
    if args.output:
        with open(args.output, "w") as f:
            f.write(json_str)
    else:
        print(json_str)

if __name__ == "__main__":
    main()
