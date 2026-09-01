#!/usr/bin/env python3

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

"""
bindcheck -- detect drift between Filament's C++ public API and its JS/Java bindings.

Filament's JavaScript and Java bindings are hand-written (see tools/beamsplitter/README.md
for the small part that *is* generated). Nothing keeps them in sync with the C++ headers, so
methods and enum values silently go missing. This tool compares the four surfaces and reports
what has drifted.

    surfaces
      cpp   filament/include/filament/*.h          the source of truth
      js    web/filament-js/jsbindings.cpp         embind registrations
            web/filament-js/jsenums*.cpp
            web/filament-js/extensions*.js         hand-written prototype wrappers
      dts   web/filament-js/filament.d.ts          the *declared* JS API
      java  android/.../com/google/android/filament/*.java

Usage:
    python3 tools/bindcheck/bindcheck.py                 # report everything
    python3 tools/bindcheck/bindcheck.py --check enum-order-java
    python3 tools/bindcheck/bindcheck.py --class Texture --verbose
    python3 tools/bindcheck/bindcheck.py --json          # machine-readable, for CI

Exit code is 1 if any finding survived the exclusions, so it can gate a presubmit.
Exclusions live in exclusions.json next to this file; every entry requires a reason.

Requires: pip install libclang
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile

# ---------------------------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------------------------

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

# Include paths sufficient to parse every public header standalone -- no build required.
INCLUDE_DIRS = [
    "filament/include",
    "libs/utils/include",
    "libs/math/include",
    "libs/filabridge/include",
    "filament/backend/include",
    "third_party/robin-map",
]

# Declarations are only trusted when they come from these trees, so that transitively included
# system and third-party headers do not pollute the C++ surface.
PUBLIC_HEADER_DIRS = ["filament/include/filament", "filament/backend/include/backend"]

CPP_HEADER_GLOB = "filament/include/filament"
JS_BINDINGS = "web/filament-js/jsbindings.cpp"
JS_ENUMS = ["web/filament-js/jsenums.cpp", "web/filament-js/jsenums_generated.cpp"]
JS_EXTENSIONS = ["web/filament-js/extensions.js", "web/filament-js/extensions_generated.js"]
DTS = "web/filament-js/filament.d.ts"
JAVA_DIR = "android/filament-android/src/main/java/com/google/android/filament"

EXCLUSIONS_FILE = os.path.join(os.path.dirname(__file__), "exclusions.json")


def path(rel):
    return os.path.join(ROOT, rel)


# ---------------------------------------------------------------------------------------------
# Canonical keys
#
# Every surface names nested types differently: C++ writes Texture::Builder, embind registers
# the string "Texture$Builder", the .d.ts declares `class Texture$Builder`, and Java nests a
# `static class Builder` inside Texture.java. embind's registered name is the one all four can
# agree on, so `Outer$Inner` is the canonical key throughout this tool.
# ---------------------------------------------------------------------------------------------

def key_of(*parts):
    return "$".join(p for p in parts if p)


# ---------------------------------------------------------------------------------------------
# Surfaces
#
# A Surface is what one language exposes:
#   methods: {class_key: set(method_name)}
#   enums:   {enum_key: [enumerator_name, ...]}   -- ordered; order is load-bearing for Java
# ---------------------------------------------------------------------------------------------

class Surface(object):
    def __init__(self, name):
        self.name = name
        self.methods = {}
        self.enums = {}         # {enum_key: [name, ...]} in declaration order
        self.enum_values = {}   # {enum_key: {name: int}} -- C++ only; aliases share a value
        self.nonpublic = {}     # {class_key: set(method)} -- Java only; declared without
                                # `public`, so absent from the API without being absent

    def add_method(self, cls, method):
        self.methods.setdefault(cls, set()).add(method)

    def add_enum(self, enum, values, numbering=None):
        self.enums[enum] = list(values)
        if numbering is not None:
            self.enum_values[enum] = dict(numbering)

    def classes(self):
        return set(self.methods)


# --- C++ -------------------------------------------------------------------------------------

def clang_resource_dir():
    try:
        return subprocess.check_output(
            ["clang", "-print-resource-dir"], stderr=subprocess.DEVNULL).decode().strip()
    except (OSError, subprocess.CalledProcessError):
        return None


def _parse(umbrella_path, extra_args):
    import clang.cindex as ci
    args = ["-std=c++17", "-x", "c++"] + extra_args
    args += ["-I" + path(d) for d in INCLUDE_DIRS]
    tu = ci.Index.create().parse(umbrella_path, args=args)
    errors = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    # Errors in the platform SDK or toolchain are version skew between the installed libclang and
    # the system's libc++ headers; clang recovers from them and Filament's declarations still come
    # through. Errors inside Filament's own headers are not survivable -- they mean the surface we
    # are about to trust is incomplete, which would read as a clean bill of health.
    filament_root = path("filament")
    ours = [d for d in errors if d.location.file
            and os.path.abspath(d.location.file.name).startswith(filament_root)]
    return tu, errors, ours


def parse_public_headers(umbrella_path):
    """
    Parse the umbrella TU, trying a few flag sets and keeping the cleanest.

    pip's libclang ships no builtin headers, and on Apple silicon the toolchain's arm_acle.h and
    the SDK's libc++ reference builtins an older libclang does not know. Which flags work depends
    on the machine, so rather than pin one combination, try the plausible ones and keep whichever
    upsets clang least.
    """
    candidates = [("host default", [])]
    resource_dir = clang_resource_dir()
    if resource_dir:
        include = os.path.join(resource_dir, "include")
        candidates.append(("x86_64 + builtin headers",
                           ["--target=x86_64-apple-macos12", "-idirafter", include]))
        candidates.append(("host default + builtin headers", ["-idirafter", include]))
    candidates.append(("x86_64", ["--target=x86_64-apple-macos12"]))

    best = None
    for label, extra in candidates:
        tu, errors, ours = _parse(umbrella_path, extra)
        if best is None or (len(ours), len(errors)) < (len(best[3]), len(best[2])):
            best = (label, tu, errors, ours)
        if not errors:
            break
    return best


def add_cpp_enum(surface, key, cursor, ci):
    """
    Record an enum's constants together with their actual values. Declaration order is not the
    same thing as value: RenderTarget::AttachmentPoint ends with `COLOR = COLOR0`, an alias whose
    value is 0 even though it is declared last, and other enums use explicit or computed values.
    """
    constants = [c for c in cursor.get_children()
                 if c.kind == ci.CursorKind.ENUM_CONSTANT_DECL]
    surface.add_enum(key,
                     [c.spelling for c in constants],
                     dict((c.spelling, c.enum_value) for c in constants))


def extract_cpp():
    """Parse every public header in one translation unit and collect public methods and enums."""
    try:
        import clang.cindex as ci
    except ImportError:
        sys.exit("bindcheck: missing dependency. Run:  pip3 install libclang")

    headers = sorted(f for f in os.listdir(path(CPP_HEADER_GLOB)) if f.endswith(".h"))

    # One umbrella TU instead of 39 parses: same result, a fraction of the time, and it
    # deduplicates the types that several headers pull in.
    umbrella = "\n".join('#include <filament/%s>' % h for h in headers) + "\n"
    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as fh:
        fh.write(umbrella)
        umbrella_path = fh.name

    try:
        label, tu, errors, ours = parse_public_headers(umbrella_path)
    finally:
        os.unlink(umbrella_path)

    if ours:
        sys.stderr.write("bindcheck: %d error(s) inside Filament headers (%s); refusing to report "
                         "partial results\n" % (len(ours), label))
        for d in ours[:5]:
            sys.stderr.write("  %s\n" % d)
        sys.exit(2)
    if errors:
        sys.stderr.write("bindcheck: note: %d diagnostic(s) from the platform SDK/toolchain "
                         "(libclang version skew); Filament's own headers parsed cleanly\n"
                         % len(errors))

    surface = Surface("cpp")
    surface = Surface("cpp")
    public_dirs = [path(d) for d in PUBLIC_HEADER_DIRS]

    def is_public_header(cursor):
        f = cursor.location.file
        return f is not None and any(os.path.abspath(f.name).startswith(d) for d in public_dirs)

    def walk(cursor, prefix):
        for child in cursor.get_children():
            kind = child.kind

            if kind == ci.CursorKind.NAMESPACE:
                walk(child, prefix)

            elif kind in (ci.CursorKind.CLASS_DECL, ci.CursorKind.STRUCT_DECL):
                if not child.is_definition() or not child.spelling:
                    continue
                walk(child, key_of(prefix, child.spelling))

            elif kind == ci.CursorKind.ENUM_DECL and child.spelling:
                if is_public_header(child):
                    add_cpp_enum(surface, key_of(prefix, child.spelling), child, ci)

            elif kind == ci.CursorKind.TYPE_ALIAS_DECL:
                # `using Sampler = backend::SamplerType;` -- Texture::Sampler is how every
                # binding refers to it, so record the alias under the enclosing class.
                decl = child.underlying_typedef_type.get_declaration()
                if decl.kind == ci.CursorKind.ENUM_DECL and prefix:
                    add_cpp_enum(surface, key_of(prefix, child.spelling), decl, ci)

            elif kind == ci.CursorKind.CXX_METHOD and prefix:
                if (child.access_specifier == ci.AccessSpecifier.PUBLIC
                        and is_public_header(child)
                        and not child.spelling.startswith("operator")):
                    surface.add_method(prefix, child.spelling)

    walk(tu.cursor, "")
    return surface


# --- JavaScript ------------------------------------------------------------------------------

# embind chains registrations onto one `class_<T>("Name")`, so a block runs until the next one.
# `[^(]*` rather than `[^>]*`: a subclass registration is `class_<Derived, base<Base>>("...")`,
# and stopping at the first `>` leaves the trailing one unmatched -- the block then goes
# unrecognised and its members are credited to whichever class was registered before it.
RE_CLASS_BLOCK = re.compile(r'class_<[^(]*>\s*\(\s*"([^"]+)"\s*\)')
# BUILDER_FUNCTION is jsbindings.cpp's own macro for the `return *this` builder methods; it
# expands to .function(...), so it registers a member just like the others.
RE_MEMBER = re.compile(
    r'\.(?:function|class_function|property|BUILDER_FUNCTION)\(\s*"([^"]+)"')
RE_ENUM_BLOCK = re.compile(r'enum_<[^>]*>\s*\(\s*"([^"]+)"\s*\)')
RE_ENUM_VALUE = re.compile(r'\.value\(\s*"([^"]+)"')


def _blocks(text, header_re):
    """Yield (name, body) for each registration block, body ending at the next block."""
    matches = list(header_re.finditer(text))
    for i, m in enumerate(matches):
        end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
        yield m.group(1), text[m.end():end]


def extract_js():
    surface = Surface("js")

    bindings = read(JS_BINDINGS)
    for name, body in _blocks(bindings, RE_CLASS_BLOCK):
        # A class registered with no members of its own is still bound: ToneMapper exists only
        # as the embind base its subclasses derive from, and reading the class as absent turns
        # a base class into a phantom unbound-class finding.
        surface.methods.setdefault(name, set())
        for member in RE_MEMBER.findall(body):
            # `_setImage` in embind is the private half of the public `setImage` wrapper
            # defined in extensions.js. One convention, not a special case.
            surface.add_method(name, member.lstrip("_"))

    for rel in JS_ENUMS:
        for name, body in _blocks(read(rel), RE_ENUM_BLOCK):
            surface.add_enum(name, RE_ENUM_VALUE.findall(body))

    # extensions.js adds and overrides members after embind is done, both on the prototype
    # (instance methods) and directly on the class object (statics such as Engine.create).
    re_member = re.compile(r'Filament\.([A-Za-z0-9_$]+)\.(?:prototype\.)?(\w+)\s*=')
    for rel in JS_EXTENSIONS:
        for cls, member in re_member.findall(read(rel)):
            if member != "prototype":
                surface.add_method(cls, member.lstrip("_"))

    return surface


# --- TypeScript declarations -------------------------------------------------------------------

def collapse_parens(text):
    """
    Replace every parameter list with `()`, innermost first.

    A signature that wraps puts its remaining parameters at the start of a line, where they are
    indistinguishable from a field declaration -- `near: number,` continuing setProjection() reads
    exactly like `width: number;` on DecodedImage. Removing parameter lists removes the ambiguity.
    """
    # A sentinel rather than "()": leaving the parentheses in place would block the next pass
    # from matching the enclosing list, so `f(cb: (n) => void)` would only collapse one level.
    while True:
        collapsed = re.sub(r'\([^()]*\)', "\0", text)
        if collapsed == text:
            return text.replace("\0", "()")
        text = collapsed


def extract_dts():
    surface = Surface("dts")
    text = read(DTS)

    for m in re.finditer(r'export\s+(?:declare\s+)?class\s+([A-Za-z0-9_$]+)[^{]*\{', text):
        body = collapse_parens(text[m.end():m.end() + brace_span(text, m.end())])
        # Anchored to the start of a line: a declaration always begins one, whereas an
        # unanchored scan reads the `Vector` of `getChildren(): Vector<Entity>` as a member.
        # `:` catches fields -- embind .property() bindings such as KtxInfo.glType are declared
        # as fields here, and a class of nothing but fields would otherwise look undeclared.
        for member in re.findall(r'^\s*(?:public\s+|readonly\s+|static\s+)*(\w+)\s*[(<:]',
                                 body, re.M):
            surface.add_method(m.group(1), member)

    for m in re.finditer(r'export\s+(?:const\s+)?enum\s+([A-Za-z0-9_$]+)\s*\{', text):
        body = text[m.end():m.end() + brace_span(text, m.end())]
        surface.add_enum(m.group(1), re.findall(r'^\s*(\w+)\s*(?:=|,|$)', body, re.M))

    return surface


def brace_span(text, start):
    """Length of the balanced-brace body beginning just after an opening brace at `start`."""
    depth, i = 1, start
    while depth and i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    return i - start - 1


# --- Java ---------------------------------------------------------------------------------------

# `(?:@\w+\s+)*` skips annotations sitting between the modifiers and the return type, as in
# TransformManager's `public @Entity @NonNull int[] getChildren(...)`. Without it the method is
# invisible and reads as unbound. `public @interface Foo {` is not matched: it has no argument list.
RE_JAVA_METHOD = re.compile(
    r'public\s+(?:static\s+)?(?:final\s+)?(?:synchronized\s+)?(?:@\w+\s+)*'
    r'[\w.<>\[\]$]+\s+(\w+)\s*\(')
# A declaration with no access modifier at all: package-private, so an app outside
# com.google.android.filament cannot call it. Requiring `) {` keeps statements out -- a call or
# an assignment ends in `;`, and `if (x) {` has one token before the paren where a declaration
# has two. private/protected are left out on purpose: those read as decisions, an absent
# modifier usually reads as an oversight.
RE_JAVA_PACKAGE_PRIVATE = re.compile(
    r'^[ \t]+(?!public\b|private\b|protected\b|return\b|new\b|else\b|case\b|default\b)'
    r'(?:@\w+(?:\([^()]*\))?\s+)*'
    r'(?:(?:static|final|synchronized)\s+)*'
    r'[\w.<>\[\]$]+\s+(\w+)\s*\([^;{]*\)\s*\{', re.M)

RE_JAVA_NESTED = re.compile(r'public\s+(?:static\s+)?(?:final\s+)?class\s+(\w+)')
RE_JAVA_ENUM = re.compile(r'public\s+enum\s+(\w+)\s*\{')


def split_top_level(text, sep=","):
    """Split on `sep`, ignoring separators nested inside parentheses."""
    out, depth, start = [], 0, 0
    for i, ch in enumerate(text):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth = max(0, depth - 1)
        elif ch == sep and depth == 0:
            out.append(text[start:i])
            start = i + 1
    out.append(text[start:])
    return out


def java_enum_constants(blob):
    """
    Return [(name, value)] for a Java enum body.

    Java packs several constants onto one line (`R8, R8_SNORM, R8UI, ...`), interleaves
    annotations (`@Deprecated` before a constant), does not always use SCREAMING_CASE
    (View.ShadowType.PCFd), and may give constants an explicit value through a constructor
    (`LINE_STRIP(3)`). Missing any of those drops or misnumbers a constant, and a misnumbered
    constant looks exactly like a real ordinal bug -- so parse the whole body, tolerantly.

    `value` is the explicit constructor argument when there is one, otherwise the ordinal.
    """
    blob = blob.split(";")[0]                            # methods may follow the constant list
    blob = re.sub(r'@\w+\s*(\([^()]*\))?', " ", blob)     # annotations, with or without arguments
    out = []
    for chunk in split_top_level(blob):
        m = re.match(r'\s*([A-Za-z_$][A-Za-z0-9_$]*)\s*(?:\(\s*(-?(?:0[xX][0-9a-fA-F]+|\d+)))?',
                     chunk)
        if not m:
            continue
        explicit = m.group(2)
        value = int(explicit, 0) if explicit is not None else len(out)
        out.append((m.group(1), value))
    return out


def scan_java_scope(surface, text, prefix):
    """Collect methods and enums declared directly in `text`, recursing into nested types."""
    carved = []

    for m in RE_JAVA_NESTED.finditer(text):
        body = text.index("{", m.end()) + 1
        span = brace_span(text, body)
        carved.append((m.start(), body + span + 1))
        scan_java_scope(surface, text[body:body + span], key_of(prefix, m.group(1)))

    for m in RE_JAVA_ENUM.finditer(text):
        body = m.end()
        span = brace_span(text, body)
        if any(start <= m.start() < end for start, end in carved):
            continue                                # belongs to a nested type, already handled
        carved.append((m.start(), body + span + 1))
        constants = java_enum_constants(text[body:body + span])
        surface.add_enum(key_of(prefix, m.group(1)),
                         [name for name, _ in constants], dict(constants))

    own = blank_out(text, carved)
    for member in RE_JAVA_METHOD.findall(own):
        surface.add_method(prefix, member)
    for member in RE_JAVA_PACKAGE_PRIVATE.findall(own):
        surface.nonpublic.setdefault(prefix, set()).add(member)


def extract_java():
    surface = Surface("java")
    java_dir = path(JAVA_DIR)
    for filename in sorted(os.listdir(java_dir)):
        if not filename.endswith(".java"):
            continue
        outer = filename[:-len(".java")]
        text = strip_comments(open(os.path.join(java_dir, filename)).read())
        # Descend into the file's own top-level type first, so that its members are attributed to
        # `Outer` and only genuinely nested types become `Outer$Inner`.
        top = re.search(r'public\s+(?:final\s+|abstract\s+)*(?:class|interface|enum)\s+'
                        + re.escape(outer) + r'\b[^{]*\{', text)
        if top:
            body = text[top.end():top.end() + brace_span(text, top.end())]
        else:
            body = text
        scan_java_scope(surface, body, outer)
    return surface


def blank_out(text, spans):
    """Replace the given spans with spaces, preserving offsets."""
    chars = list(text)
    for start, end in spans:
        for i in range(max(0, start), min(len(chars), end)):
            chars[i] = " "
    return "".join(chars)


def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    return re.sub(r'//[^\n]*', "", text)


def read(rel):
    return open(path(rel)).read()


# ---------------------------------------------------------------------------------------------
# Exclusions
#
# Deliberate divergence is the norm here -- the JS API is not meant to mirror C++ one-to-one.
# Exclusions are therefore first-class, and every one carries a reason that is printed by
# --explain, so the file stays a record of decisions rather than a pile of silenced noise.
# ---------------------------------------------------------------------------------------------

class Exclusions(object):
    def __init__(self, data):
        self.reasons = {}       # (check, key, member) -> reason
        self.class_reasons = {} # (check, key) -> reason
        self.check_reasons = {} # check -> reason  (whole check disabled)
        for entry in data.get("ignore", []):
            check = entry.get("check", "*")
            reason = entry["reason"]  # required, on purpose
            members = entry.get("members")
            keys = entry.get("key")
            if keys is None:
                self.check_reasons[check] = reason
                continue
            # `key` takes a list as well as a string: a divergence is usually a convention
            # rather than a one-off, and Java's nested tone mappers are eight keys, one decision.
            for key in ([keys] if isinstance(keys, str) else keys):
                if members is None:
                    self.class_reasons[(check, key)] = reason
                else:
                    for member in members:
                        self.reasons[(check, key, member)] = reason

    def reason_for(self, check, key, member):
        # `"key": "*"` covers a member name wherever it appears: `name` is inherited by every
        # Builder, `async` by every async-creatable resource, so the alternative is the same
        # entry repeated once per class.
        for c in (check, "*"):
            for k in (key, "*"):
                for table, probe in ((self.check_reasons, (c,)),
                                     (self.class_reasons, (c, k)),
                                     (self.reasons, (c, k, member))):
                    if probe in table:
                        return table[probe]
        return None

    @staticmethod
    def load():
        if not os.path.exists(EXCLUSIONS_FILE):
            return Exclusions({})
        with open(EXCLUSIONS_FILE) as fh:
            return Exclusions(json.load(fh))


# ---------------------------------------------------------------------------------------------
# Checks
#
# Each check is a function (surfaces) -> [Finding]. To add one, write the function and append
# it to CHECKS; nothing else needs to change.
# ---------------------------------------------------------------------------------------------

class Finding(object):
    def __init__(self, check, key, member, message, severity="warning"):
        self.check = check
        self.key = key
        self.member = member
        self.message = message
        self.severity = severity
        self.excused = None

    def as_dict(self):
        return {"check": self.check, "class": self.key, "member": self.member,
                "message": self.message, "severity": self.severity}


def check_unbound(surfaces, target, name):
    """C++ public methods with no counterpart in the target binding."""
    cpp, other = surfaces["cpp"], surfaces[target]
    out = []
    for key, methods in sorted(cpp.methods.items()):
        if key not in other.methods:
            continue  # the whole class is unbound; check_unbound_class reports that instead
        # A member Java declares without `public` is reported by nonpublic-java instead: it is
        # bound, just not reachable, which is a different bug with a different fix.
        missing = methods - other.methods[key] - other.nonpublic.get(key, set())
        for method in sorted(missing):
            out.append(Finding(name, key, method,
                               "%s::%s is public in C++ but not bound in %s" % (key, method, target)))
    return out


def check_unbound_class(surfaces, target):
    """Classes bound in one language but not the other -- usually an oversight, sometimes policy."""
    other = "java" if target == "js" else "js"
    out = []
    for key in sorted(surfaces["cpp"].classes() - surfaces[target].classes()):
        if key not in surfaces[other].classes():
            continue  # bound nowhere: out of scope, not drift
        out.append(Finding("unbound-class-" + target, key, None,
                           "%s is bound in %s but entirely absent from %s" % (key, other, target)))
    return out


def check_undeclared_ts(surfaces):
    """Reachable from JavaScript but missing from filament.d.ts -- invisible to TypeScript users."""
    js, dts = surfaces["js"], surfaces["dts"]
    out = []
    for key, methods in sorted(js.methods.items()):
        if key not in dts.methods:
            out.append(Finding("undeclared-ts", key, None,
                               "%s is bound in JS but has no class declaration in filament.d.ts" % key))
            continue
        for method in sorted(methods - dts.methods[key] - {"delete"}):
            out.append(Finding("undeclared-ts", key, method,
                               "%s.%s is bound in JS but not declared in filament.d.ts" % (key, method)))
    return out


def check_phantom_ts(surfaces):
    """Declared in filament.d.ts but bound nowhere -- a lie to TypeScript users; fails at runtime."""
    js, dts = surfaces["js"], surfaces["dts"]
    out = []
    for key, methods in sorted(dts.methods.items()):
        if key not in js.methods:
            continue  # hand-written types with no embind class (Camutils, gltfio helpers)
        for method in sorted(methods - js.methods[key] - {"delete", "constructor"}):
            out.append(Finding("phantom-ts", key, method,
                               "%s.%s is declared in filament.d.ts but bound nowhere" % (key, method),
                               severity="error"))
    return out


def check_nonpublic_java(surfaces):
    """
    Public in C++, declared in Java without an access modifier.

    The method is bound and works; it is simply package-private, so nothing outside
    com.google.android.filament can call it. That reads as an omitted `public` rather than a
    decision, especially where the matching setter is public and only the getter is not.
    """
    cpp, java = surfaces["cpp"], surfaces["java"]
    out = []
    for key, methods in sorted(cpp.methods.items()):
        hidden = java.nonpublic.get(key, set()) - java.methods.get(key, set())
        for method in sorted(methods & hidden):
            paired = "set" + method[2:] if method.startswith("is") else None
            note = ""
            if paired and paired in java.methods.get(key, set()):
                note = "; %s.%s is public, so only the getter is unreachable" % (key, paired)
            out.append(Finding("nonpublic-java", key, method,
                               "%s.%s is public in C++ but package-private in Java, so no caller "
                               "outside com.google.android.filament can reach it%s"
                               % (key, method, note)))
    return out


def check_enum_values(surfaces, target, name):
    """C++ enumerators missing from a binding."""
    cpp, other = surfaces["cpp"], surfaces[target]
    out = []
    for key, values in sorted(cpp.enums.items()):
        if key not in other.enums:
            continue
        for value in [v for v in values if v not in other.enums[key]]:
            out.append(Finding(name, key, value,
                               "%s.%s exists in C++ but is missing from %s" % (key, value, target)))
    return out


def check_enum_order_java(surfaces):
    """
    Java passes enum.ordinal() across JNI and the C++ side casts the int straight back to the
    enum -- see Texture.java's nBuilderSampler(..., target.ordinal()) and the matching
    (Texture::Sampler) cast in cpp/Texture.cpp. A Java enum that inserts, drops or reorders a
    constant therefore still compiles and still runs, but selects a different C++ value.

    The comparison is against each C++ constant's *value*, never its position: aliases such as
    AttachmentPoint's `COLOR = COLOR0` are declared last but are worth 0, and several enums set
    explicit values. A constant C++ does not declare at all is a rename, reported separately.
    """
    cpp, java = surfaces["cpp"], surfaces["java"]
    out = []
    for key, expected in sorted(cpp.enum_values.items()):
        if key not in java.enums:
            continue
        numbering = java.enum_values.get(key, {})
        for name in java.enums[key]:
            ordinal = numbering.get(name)
            if name not in expected:
                out.append(Finding("enum-name-java", key, name,
                                   "%s: Java declares %s at ordinal %d, which C++ does not have "
                                   "(a rename, or a value C++ dropped)" % (key, name, ordinal)))
                continue
            if ordinal is not None and expected[name] != ordinal:
                out.append(Finding("enum-order-java", key, name,
                                   "%s: %s is ordinal %d in Java but worth %d in C++; ordinal() "
                                   "is cast straight to the C++ enum across JNI, so this selects "
                                   "the wrong value at runtime"
                                   % (key, name, ordinal, expected[name]),
                                   severity="error"))
                break  # one report per enum: the first divergence explains the rest
    return out


CHECKS = [
    ("unbound-js", lambda s: check_unbound(s, "js", "unbound-js")),
    ("unbound-java", lambda s: check_unbound(s, "java", "unbound-java")),
    ("unbound-class-js", lambda s: check_unbound_class(s, "js")),
    ("unbound-class-java", lambda s: check_unbound_class(s, "java")),
    ("undeclared-ts", check_undeclared_ts),
    ("phantom-ts", check_phantom_ts),
    ("nonpublic-java", check_nonpublic_java),
    ("enum-values-js", lambda s: check_enum_values(s, "js", "enum-values-js")),
    ("enum-values-java", lambda s: check_enum_values(s, "java", "enum-values-java")),
    ("enum-order-java", check_enum_order_java),
    ("enum-name-java", lambda s: []),  # emitted by check_enum_order_java; listed so --check finds it
]



# ---------------------------------------------------------------------------------------------
# Self-test
#
# Every false alarm this tool produced while it was being written came from the text extractors,
# not the checks -- a dropped enum constant shifts every ordinal after it and reads exactly like
# a real JNI bug. These assertions pin the cases that actually bit.
# ---------------------------------------------------------------------------------------------

def selftest():
    # several constants per line, as in Texture.InternalFormat
    assert java_enum_constants("R8, R8_SNORM, R8UI") == [("R8", 0), ("R8_SNORM", 1), ("R8UI", 2)]

    # an annotation between constants, as in View.ShadowType, and a name that is not all-caps
    assert java_enum_constants("PCF,\n@Deprecated\nDPCF,\nPCFd") == [
        ("PCF", 0), ("DPCF", 1), ("PCFd", 2)]

    # explicit values via a constructor, as in RenderableManager.PrimitiveType
    assert java_enum_constants("POINTS(0), LINES(1), LINE_STRIP(3)") == [
        ("POINTS", 0), ("LINES", 1), ("LINE_STRIP", 3)]

    # methods after the constant list must not be read as constants
    assert java_enum_constants("A, B;\nint getValue() { return v; }") == [("A", 0), ("B", 1)]

    # annotations between `public` and the return type must not hide a method
    surface = Surface("test")
    scan_java_scope(surface, "public @Entity @NonNull int[] getChildren(int i) {}\n"
                             "public @interface Marker {}\n", "T")
    assert surface.methods["T"] == {"getChildren"}, surface.methods

    # package-private declarations are found, and statements inside a method body are not:
    # every one of these lines sits in real Java and would be a phantom finding
    surface = Surface("test")
    scan_java_scope(surface, """
        public void publicOne() {}
        boolean isShadowingEnabled() {
            int n = helper(1);
            if (n > 0) {
                for (int i = 0; i < n; i++) { }
            } else if (n < 0) {
            }
            try { run(); } catch (Exception e) { }
            return n > 0;
        }
        static double computeEffectiveFov(double fov, double d) {}
        private void privateOne() {}
        protected void protectedOne() {}
    """, "T")
    assert surface.methods["T"] == {"publicOne"}, surface.methods
    assert surface.nonpublic["T"] == {"isShadowingEnabled", "computeEffectiveFov"}, \
        surface.nonpublic

    # commas inside constructor arguments do not separate constants
    assert split_top_level("A(1, 2), B") == ["A(1, 2)", " B"]

    # balanced-brace scanning, used to carve nested types out of a Java file
    assert brace_span("a{b{c}d}e", 2) == 5

    # a wrapped signature's parameters must not be read as filament.d.ts fields: `near` below
    # starts a line and looks exactly like DecodedImage's `public width: number;`
    assert collapse_parens("setProjection(fov: number,\n near: number): void;") \
        == "setProjection(): void;"
    assert collapse_parens("f(cb: (n: number) => void): void;") == "f(): void;"

    # a nested type's members belong to Outer$Inner, never to Outer -- getting this wrong
    # doubled every key the first time round
    surface = Surface("test")
    scan_java_scope(surface, """
        public int outerMethod() {}
        public static class Builder {
            public int build() {}
            public enum Mode { A, B }
        }
    """, "Outer")
    assert surface.methods["Outer"] == {"outerMethod"}, surface.methods
    assert surface.methods["Outer$Builder"] == {"build"}, surface.methods
    assert surface.enums["Outer$Builder$Mode"] == ["A", "B"], surface.enums

    print("bindcheck: self-test passed")
    return 0


# ---------------------------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[1],
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--check", action="append", metavar="NAME",
                        help="only run this check (repeatable); default is all of: "
                             + ", ".join(name for name, _ in CHECKS))
    parser.add_argument("--class", dest="klass", action="append", metavar="KEY",
                        help="only report this class, e.g. Texture or Texture$Builder")
    parser.add_argument("--json", action="store_true", help="machine-readable output")
    parser.add_argument("--explain", action="store_true",
                        help="also list excluded findings and why they are excluded")
    parser.add_argument("--list-checks", action="store_true")
    parser.add_argument("--self-test", action="store_true",
                        help="check the text extractors against the cases that have caused "
                             "false alarms")
    opts = parser.parse_args()

    if opts.self_test:
        return selftest()

    if opts.list_checks:
        for name, _ in CHECKS:
            print(name)
        return 0

    surfaces = {"cpp": extract_cpp(), "js": extract_js(),
                "dts": extract_dts(), "java": extract_java()}
    exclusions = Exclusions.load()

    findings = []
    for name, fn in CHECKS:
        if opts.check and name not in opts.check:
            continue
        findings.extend(fn(surfaces))

    if opts.klass:
        findings = [f for f in findings if f.key in opts.klass]

    for f in findings:
        f.excused = exclusions.reason_for(f.check, f.key, f.member)

    active = [f for f in findings if f.excused is None]
    excused = [f for f in findings if f.excused is not None]

    if opts.json:
        print(json.dumps({"findings": [f.as_dict() for f in active],
                          "excluded": len(excused)}, indent=2))
        return 1 if active else 0

    by_check = {}
    for f in active:
        by_check.setdefault(f.check, []).append(f)

    for name, _ in CHECKS:
        group = by_check.get(name)
        if not group:
            continue
        print("\n%s  (%d)" % (name, len(group)))
        print("-" * (len(name) + 8))
        for f in group:
            mark = "!" if f.severity == "error" else " "
            print("  %s %s" % (mark, f.message))

    if opts.explain and excused:
        print("\nexcluded  (%d)" % len(excused))
        print("-" * 18)
        for f in excused:
            print("    %s %s -- %s" % (f.key, f.member or "", f.excused))

    print("\n%d finding(s), %d excluded by %s"
          % (len(active), len(excused), os.path.relpath(EXCLUSIONS_FILE, ROOT)))
    return 1 if active else 0


if __name__ == "__main__":
    sys.exit(main())
