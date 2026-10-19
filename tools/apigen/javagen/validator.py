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
#

"""Static Parity & Reflection Validator for Generated Java and JNI C++ Bindings.

This module provides static analysis and cross-language verification between generated (or
handwritten) Android Java binding classes and their corresponding C++ JNI implementation files.
It catches linkage mismatches, JNI signature discrepancies, and invalid reflection lookups at
code-generation time rather than at Android runtime (`UnsatisfiedLinkError` or `NoSuchFieldError`).

Architecture & Validation Passes:
---------------------------------
```
  ┌──────────────────────────┐        ┌──────────────────────────┐
  │   Java Source Files      │        │   C++ JNI Source Files   │
  │     (*.java)             │        │       (*.cpp)            │
  └────────────┬─────────────┘        └────────────┬─────────────┘
               │                                   │
               ▼                                   ▼
  ┌──────────────────────────┐        ┌──────────────────────────┐
  │ 1. Java Source Parsing   │        │ 2. C++ JNI Source Parsing│
  │ - Mask comments/strings  │        │ - Extract JNIEXPORT fns  │
  │ - Track enclosing classes│        │ - Track FindClass /      │
  │ - Extract native methods │        │   GetObjectClass vars    │
  │ - Extract member fields  │        │ - Extract GetFieldID &   │
  │                          │        │   GetMethodID calls      │
  └────────────┬─────────────┘        └────────────┬─────────────┘
               └─────────────────┬─────────────────┘
                                 │
                                 ▼
               ┌───────────────────────────────────┐
               │ 3. Static Validation Passes       │
               │ - Pass A: Symbol Parity           │
               │ - Pass B: Signature Compatibility │
               │ - Pass C: Reflection Integrity    │
               └─────────────────┬─────────────────┘
                                 │
                                 ▼
               ┌───────────────────────────────────┐
               │        ValidationResult           │
               │   (Errors, Warnings, Report)      │
               └───────────────────────────────────┘
```

1. **Java Source Parsing (`_parse_java_sources`)**:
   - Scans `.java` files, masks out comments and string literals (`_mask_code`) while preserving
     character offsets and line numbers, and tracks nested class scopes (`Outer$Inner`).
   - Extracts all `native` method declarations (`JavaNativeMethod`) and member fields (`JavaField`).

2. **C++ JNI Source Parsing (`_parse_cpp_sources`)**:
   - Scans `.cpp` files for `extern "C" JNIEXPORT ... JNICALL Java_...` function definitions
     (`CppJniFunction`), decoding JNI name mangling (`_00024` for inner classes, `__` for overloaded
     method signatures).
   - Tracks `env->FindClass(...)` and `env->GetObjectClass(...)` assignments to resolve the target
     Java class of subsequent `env->GetFieldID(...)` (`CppReflectionField`) and
     `env->GetMethodID(...)` (`CppReflectionMethod`) reflection calls.

3. **Validation Rules**:
   - **Symbol Parity (`_validate_symbol_parity`)**: Verifies a 1-to-1 bidirectional mapping between
     Java `native` declarations and C++ `JNIEXPORT` implementations.
   - **Signature Compatibility (`_validate_signatures`)**: Normalizes Java and C++ JNI parameter
     types into canonical JNI types (`jint`, `jlong`, `jfloatArray`, `jobject`, etc.) and verifies
     that every Java native method overload matches a C++ JNI implementation signature.
   - **Reflection Field Integrity (`_validate_reflection_fields`)**: Verifies that every field
     queried via `env->GetFieldID` exists on the target Java class and has a matching type
     descriptor (`I` -> `int`, `F` -> `float`, `J` -> `long`, etc.).
"""

import bisect
from dataclasses import dataclass, field
import os
from pathlib import Path
import re
from typing import Dict, List, Optional, Set, Tuple

# -----------------------------------------------------------------------------
# Pre-compiled Regular Expressions for Java and C++ JNI Parsing
# -----------------------------------------------------------------------------

# Matches Java package declarations: `package com.google.android.filament;`
_JAVA_PACKAGE_RE = re.compile(r"^\s*package\s+([\w\.]+);", re.MULTILINE)

# Matches Java native method declarations: `private static native long nCreateBuilder(...);`
_JAVA_NATIVE_METHOD_RE = re.compile(
    r"\bnative\s+([\w<>\[\],\s\?]+?)\s+(n\w+)\s*\((.*?)\)\s*;", re.DOTALL
)

# Strips inline Java parameter annotations (e.g., `@NonNull`, `@Nullable`, `@Size(min = 4)`)
_JAVA_ANNOTATION_RE = re.compile(r"@\w+(?:\([^)]*\))?\s*")

# Matches Java field declarations while excluding type/statement keywords
_JAVA_FIELD_RE = re.compile(
    r"^\s*(?:(?:public|protected|private|final|static|volatile|transient)\s+)*"
    r"(?!class\b|enum\b|interface\b|return\b|new\b|import\b|package\b)"
    r"([\w<>\[\]\.]+)\s+(\w+)\s*(?:=\s*[^;]+)?\s*;",
    re.MULTILINE,
)

# Tokenizes Java class/enum/interface declarations and curly braces for scope tracking
_JAVA_CLASS_TOKEN_RE = re.compile(r"\b(class|enum|interface)\s+(\w+)|([{}])")

# Matches C++ JNI exported function definitions:
# `extern "C" JNIEXPORT <ret> JNICALL Java_<pkg>_<cls>_<method>[__<overload>](<params>) {`
_CPP_JNI_FUNC_RE = re.compile(
    r'extern\s+"C"\s+JNIEXPORT\s+([\w\*\s]+?)\s+JNICALL\s+'
    r'(Java_([a-zA-Z0-9_]+?)_([A-Za-z0-9]+)_(n[A-Za-z0-9]+?)(?:__([A-Za-z0-9_]+))?)\s*\((.*?)\)\s*\{',
    re.DOTALL,
)

# Matches JNI FindClass calls: `clazz = env->FindClass("com/google/android/filament/Foo")`
_CPP_FIND_CLASS_RE = re.compile(r'(\w+)\s*=\s*env->FindClass\s*\(\s*"([^"]+)"\s*\)')

# Matches JNI GetObjectClass calls: `clazz = env->GetObjectClass(obj)`
_CPP_GET_OBJ_CLASS_RE = re.compile(r"(\w+)\s*=\s*env->GetObjectClass\s*\(\s*(\w+)\s*\)")

# Matches the nearest enclosing JNI function header for resolving GetObjectClass parameter types
_CPP_JNI_HEADER_RE = re.compile(
    r"Java_([a-zA-Z0-9_]+?)_([A-Za-z0-9]+)_(n[A-Za-z0-9]+?)(?:__([A-Za-z0-9_]+))?\s*\((.*?)\)\s*\{",
    re.DOTALL,
)

# Matches JNI GetFieldID / GetStaticFieldID calls: `env->GetFieldID(clazz, "mField", "I")`
_CPP_FIELD_ID_RE = re.compile(
    r'env->Get(?:Static)?FieldID\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
)

# Matches JNI GetMethodID / GetStaticMethodID calls: `env->GetMethodID(clazz, "accept", "(J)V")`
_CPP_METHOD_ID_RE = re.compile(
    r'env->Get(Static)?MethodID\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
)


# -----------------------------------------------------------------------------
# Line Number Resolution Helpers
# -----------------------------------------------------------------------------


def _compute_line_offsets(content: str) -> List[int]:
    """Compute a sorted list of 0-based character start offsets for each line in `content`.

    Args:
        content: Full source file text.

    Returns:
        List of character indices where each line begins, suitable for binary search via `bisect`.
    """
    offsets: List[int] = []
    cur = 0
    for line in content.splitlines(keepends=True):
        offsets.append(cur)
        cur += len(line)
    return offsets


def _get_line_num(line_offsets: List[int], pos: int) -> int:
    """Resolve a 0-based character offset `pos` to a 1-based line number in O(log N) time.

    Args:
        line_offsets: Precomputed line start offsets from `_compute_line_offsets`.
        pos: 0-based character index in the source file.

    Returns:
        1-based line number corresponding to `pos`.
    """
    return max(1, bisect.bisect_right(line_offsets, pos))


# -----------------------------------------------------------------------------
# Diagnostic & Parsed AST Data Structures
# -----------------------------------------------------------------------------


@dataclass
class ValidationDiagnostic:
    """Represents a single static validation error or warning with source location context.

    Attributes:
        file_path: Path to the Java or C++ file where the issue was detected.
        line: 1-based line number in `file_path` (or 0 if file-level).
        severity: Diagnostic severity level (`"ERROR"` or `"WARNING"`).
        category: Validation pass category (`"symbol_parity"`, `"signature"`, `"reflection"`, `"enum"`).
        message: Human-readable explanation of the mismatch.
    """

    file_path: Path
    line: int
    severity: str  # "ERROR" or "WARNING"
    category: str  # "symbol_parity", "signature", "reflection", "enum"
    message: str

    def __str__(self) -> str:
        loc = f"{self.file_path.name}:{self.line}" if self.line > 0 else self.file_path.name
        return f"[{self.severity}] [{self.category}] {loc}: {self.message}"


@dataclass
class ValidationResult:
    """Aggregates all diagnostics produced across static validation passes.

    Attributes:
        diagnostics: Chronological list of all recorded errors and warnings.
    """

    diagnostics: List[ValidationDiagnostic] = field(default_factory=list)

    @property
    def errors(self) -> List[ValidationDiagnostic]:
        """Return all diagnostics with `"ERROR"` severity."""
        return [d for d in self.diagnostics if d.severity == "ERROR"]

    @property
    def warnings(self) -> List[ValidationDiagnostic]:
        """Return all diagnostics with `"WARNING"` severity."""
        return [d for d in self.diagnostics if d.severity == "WARNING"]

    @property
    def is_clean(self) -> bool:
        """Return True if no validation errors were recorded (warnings are permitted)."""
        return len(self.errors) == 0

    def add_error(self, file_path: Path, line: int, category: str, message: str) -> None:
        """Record a new `"ERROR"` diagnostic."""
        self.diagnostics.append(
            ValidationDiagnostic(file_path, line, "ERROR", category, message)
        )

    def add_warning(self, file_path: Path, line: int, category: str, message: str) -> None:
        """Record a new `"WARNING"` diagnostic."""
        self.diagnostics.append(
            ValidationDiagnostic(file_path, line, "WARNING", category, message)
        )

    def format_report(self, verbose: bool = False) -> str:
        """Format a human-readable summary report of all errors (and warnings if `verbose`)."""
        lines = []
        if self.errors:
            lines.append(f"Found {len(self.errors)} validation error(s):")
            for err in self.errors:
                lines.append(f"  {err}")
        if verbose and self.warnings:
            lines.append(f"Found {len(self.warnings)} validation warning(s):")
            for warn in self.warnings:
                lines.append(f"  {warn}")
        if self.is_clean:
            lines.append("All static validation checks passed successfully.")
        return "\n".join(lines)


@dataclass
class JavaNativeMethod:
    """Represents a parsed Java `native` method declaration.

    Attributes:
        class_name: Fully-qualified Java class name (using `$` for nested classes).
        method_name: Name of the native method (typically prefixed with `n`).
        return_type: Declared Java return type string.
        param_types: Ordered list of declared Java parameter type strings (annotations stripped).
        file_path: Source `.java` file path.
        line: 1-based line number of the declaration.
    """

    class_name: str
    method_name: str
    return_type: str
    param_types: List[str]
    file_path: Path
    line: int


@dataclass
class JavaField:
    """Represents a parsed Java member or static field declaration.

    Attributes:
        class_name: Fully-qualified enclosing Java class name (using `$` for nested classes).
        field_name: Name of the field (e.g., `mNativeObject`, `mRenderable`).
        field_type: Declared Java type of the field (e.g., `int`, `float`, `long`).
        file_path: Source `.java` file path.
        line: 1-based line number of the field declaration.
    """

    class_name: str
    field_name: str
    field_type: str
    file_path: Path
    line: int


@dataclass
class CppJniFunction:
    """Represents a parsed C++ `extern "C" JNIEXPORT ... JNICALL Java_...` function definition.

    Attributes:
        package: Dotted Java package name decoded from the JNI symbol (e.g., `com.google.android.filament`).
        class_name: Mangled class name portion of the JNI symbol (e.g., `View` or `View_00024Builder`).
        method_name: Native method name portion of the JNI symbol (e.g., `nCreateBuilder`).
        overload_sig: Optional JNI mangled parameter suffix following `__` for overloaded native methods.
        return_type: C++ JNI return type string (e.g., `jlong`, `void`).
        param_types: C++ JNI parameter types excluding the leading `JNIEnv*` and `jclass`/`jobject`.
        file_path: Source `.cpp` file path.
        line: 1-based line number of the function definition.
        full_symbol: Complete C symbol name (e.g., `Java_com_google_android_filament_View_nPick`).
    """

    package: str
    class_name: str
    method_name: str
    overload_sig: Optional[str]
    return_type: str
    param_types: List[str]
    file_path: Path
    line: int
    full_symbol: str


@dataclass
class CppReflectionField:
    """Represents a C++ JNI `GetFieldID` or `GetStaticFieldID` reflection lookup.

    Attributes:
        target_class: Resolved fully-qualified Java class name being queried.
        field_name: Java field name string passed to `GetFieldID`.
        signature: JNI type descriptor string passed to `GetFieldID` (e.g., `"I"`, `"F"`, `"J"`).
        file_path: Source `.cpp` file path.
        line: 1-based line number of the lookup call.
    """

    target_class: str
    field_name: str
    signature: str
    file_path: Path
    line: int


@dataclass
class CppReflectionMethod:
    """Represents a C++ JNI `GetMethodID` or `GetStaticMethodID` reflection lookup.

    Attributes:
        target_class: Resolved fully-qualified Java class name being queried.
        method_name: Java method name string passed to `GetMethodID`.
        signature: JNI method descriptor string (e.g., `"(J)V"`).
        is_static: True if queried via `GetStaticMethodID`, False if `GetMethodID`.
        file_path: Source `.cpp` file path.
        line: 1-based line number of the lookup call.
    """

    target_class: str
    method_name: str
    signature: str
    is_static: bool
    file_path: Path
    line: int


# -----------------------------------------------------------------------------
# Cross-Language Type Normalization Tables & Helpers
# -----------------------------------------------------------------------------

JAVA_TO_JNI_TYPE = {
    "void": "void",
    "boolean": "jboolean",
    "byte": "jbyte",
    "char": "jchar",
    "short": "jshort",
    "int": "jint",
    "long": "jlong",
    "float": "jfloat",
    "double": "jdouble",
    "String": "jstring",
    "boolean[]": "jbooleanArray",
    "byte[]": "jbyteArray",
    "char[]": "jcharArray",
    "short[]": "jshortArray",
    "int[]": "jintArray",
    "long[]": "jlongArray",
    "float[]": "jfloatArray",
    "double[]": "jdoubleArray",
}

CPP_TO_JNI_TYPE = {
    "jboolean": "jboolean",
    "jbyte": "jbyte",
    "jchar": "jchar",
    "jshort": "jshort",
    "jint": "jint",
    "jlong": "jlong",
    "jfloat": "jfloat",
    "jdouble": "jdouble",
    "jstring": "jstring",
    "jobject": "jobject",
    "jclass": "jclass",
    "jbooleanArray": "jbooleanArray",
    "jbyteArray": "jbyteArray",
    "jcharArray": "jcharArray",
    "jshortArray": "jshortArray",
    "jintArray": "jintArray",
    "jlongArray": "jlongArray",
    "jfloatArray": "jfloatArray",
    "jdoubleArray": "jdoubleArray",
    "jobjectArray": "jobjectArray",
    "int": "jint",
    "float": "jfloat",
    "double": "jdouble",
    "bool": "jboolean",
    "char": "jbyte",
    "short": "jshort",
    "long": "jlong",
    "long long": "jlong",
    "int8_t": "jbyte",
    "uint8_t": "jbyte",
    "int16_t": "jshort",
    "uint16_t": "jshort",
    "int32_t": "jint",
    "uint32_t": "jint",
    "int64_t": "jlong",
    "uint64_t": "jlong",
    "size_t": "jlong",
}

SIG_TO_JAVA_TYPE = {
    "V": "void",
    "Z": "boolean",
    "B": "byte",
    "C": "char",
    "S": "short",
    "I": "int",
    "J": "long",
    "F": "float",
    "D": "double",
}


def normalize_java_type(t: str) -> str:
    """Normalize a Java type name into a canonical JNI type representation.

    Maps primitive types and primitive arrays to their exact JNI counterparts (`int` -> `jint`,
    `float[]` -> `jfloatArray`), reference arrays to `jobjectArray`, `String` to `jstring`, and
    all other object references (such as `Buffer`, `Runnable`, `Object`) to `jobject`.
    """
    t = t.strip()
    if t in JAVA_TO_JNI_TYPE:
        return JAVA_TO_JNI_TYPE[t]
    if t.endswith("[]"):
        elem = t[:-2]
        if elem in JAVA_TO_JNI_TYPE:
            return JAVA_TO_JNI_TYPE[elem] + "Array"
        return "jobjectArray"
    return "jobject"


def normalize_cpp_jni_type(t: str) -> str:
    """Normalize a C++ JNI parameter type into a canonical JNI type representation.

    Strips `const`, pointer (`*`), and reference (`&`) qualifiers and maps both standard JNI
    typedefs (`jint`, `jfloatArray`) and raw C/C++ scalar types (`int32_t`, `bool`, `float`)
    into canonical JNI types. Unrecognized types default to `jobject`.
    """
    t = t.strip().rstrip("*&")
    if t.startswith("const "):
        t = t[6:].strip()
    return CPP_TO_JNI_TYPE.get(t, "jobject")


# -----------------------------------------------------------------------------
# Main Binding Validator Engine
# -----------------------------------------------------------------------------


class BindingValidator:
    """Static parity and reflection validator for Java and JNI C++ bindings.

    Orchestrates source parsing across Java and C++ directories and executes symbol parity,
    signature compatibility, and reflection field verification passes.
    """

    def __init__(
        self,
        java_dir: Path,
        cpp_dir: Path,
        verbose: bool = False,
        target_classes: Optional[Set[str]] = None,
    ):
        """Initialize the validator with target Java and C++ source directories.

        Args:
            java_dir: Root directory containing Java source files (e.g., `android/filament-android/src/main/java`).
            cpp_dir: Root directory containing C++ JNI source files (e.g., `android/filament-android/src/main/cpp`).
            verbose: Whether to include warnings in formatted reports.
            target_classes: Optional set of simple class names (e.g., `{"View", "Engine"}`) to restrict
                validation scope; if `None`, all discovered classes are validated.
        """
        self.java_dir = java_dir
        self.cpp_dir = cpp_dir
        self.verbose = verbose
        self.target_classes = target_classes

        # Parsed Java data
        self.java_native_methods: Dict[str, List[JavaNativeMethod]] = {}
        self.java_fields: Dict[Tuple[str, str], JavaField] = {}
        self.java_class_field_names: Dict[str, Set[str]] = {}
        self.java_classes_by_simple_name: Dict[str, List[str]] = {}

        # Parsed C++ data
        self.cpp_jni_functions: Dict[Tuple[str, str], List[CppJniFunction]] = {}
        self.cpp_reflection_fields: List[CppReflectionField] = []
        self.cpp_reflection_methods: List[CppReflectionMethod] = []

    def run(self) -> ValidationResult:
        """Execute all parsing and validation passes and return the aggregated `ValidationResult`."""
        result = ValidationResult()
        self._parse_java_sources()
        self._parse_cpp_sources()

        self._validate_symbol_parity(result)
        self._validate_signatures(result)
        self._validate_reflection_fields(result)
        return result

    # -------------------------------------------------------------------------
    # Java Source Parsing
    # -------------------------------------------------------------------------

    def _parse_java_sources(self) -> None:
        """Recursively walk `self.java_dir` and parse all `.java` source files."""
        if not self.java_dir.exists():
            return

        for root, _, files in os.walk(self.java_dir):
            for file_name in files:
                if not file_name.endswith(".java"):
                    continue

                file_path = Path(root) / file_name
                try:
                    content = file_path.read_text(encoding="utf-8")
                except Exception:
                    continue

                self._parse_single_java_file(file_path, content)

    def _parse_single_java_file(self, file_path: Path, content: str) -> None:
        """Extract package, native method declarations, and member fields from a single Java file."""
        rel_path = file_path.relative_to(self.java_dir)
        package = ".".join(rel_path.parent.parts) if rel_path.parent.parts else ""

        pkg_match = _JAVA_PACKAGE_RE.search(content)
        if pkg_match:
            package = pkg_match.group(1)

        line_offsets = _compute_line_offsets(content)
        masked = self._mask_code(content)

        # Extract all Java `native` method declarations
        for m in _JAVA_NATIVE_METHOD_RE.finditer(content):
            start_pos = m.start()
            line_num = _get_line_num(line_offsets, start_pos)
            ret_type, method_name, params_str = m.groups()
            ret_type = ret_type.strip()

            enclosing_class = self._find_enclosing_java_class(masked, start_pos, file_path.stem)
            full_class_name = f"{package}.{enclosing_class}" if package else enclosing_class

            clean_params_str = _JAVA_ANNOTATION_RE.sub("", params_str)
            param_types = []
            if clean_params_str.strip():
                for p in clean_params_str.split(","):
                    parts = p.strip().split()
                    if parts:
                        param_types.append(parts[0])

            method = JavaNativeMethod(
                class_name=full_class_name,
                method_name=method_name,
                return_type=ret_type,
                param_types=param_types,
                file_path=file_path,
                line=line_num,
            )
            self.java_native_methods.setdefault(full_class_name, []).append(method)

        # Extract all Java member field declarations for reflection validation
        for m in _JAVA_FIELD_RE.finditer(content):
            start_pos = m.start()
            line_num = _get_line_num(line_offsets, start_pos)
            field_type, field_name = m.groups()
            enclosing_class = self._find_enclosing_java_class(masked, start_pos, file_path.stem)
            full_class_name = f"{package}.{enclosing_class}" if package else enclosing_class

            f_obj = JavaField(
                class_name=full_class_name,
                field_name=field_name,
                field_type=field_type,
                file_path=file_path,
                line=line_num,
            )
            self.java_fields[(full_class_name, field_name)] = f_obj
            self.java_class_field_names.setdefault(full_class_name, set()).add(field_name)
            self.java_classes_by_simple_name.setdefault(enclosing_class, []).append(
                full_class_name
            )

    def _mask_code(self, code: str) -> str:
        """Mask comments and string literals with spaces to allow reliable brace/scope parsing.

        Preserves all newline characters (`\\n`) and exact character positions so that offsets in
        the masked string map 1-to-1 to offsets in the original source string.
        """
        chars = list(code)
        n = len(chars)
        i = 0
        while i < n:
            # Mask block comments: /* ... */
            if i + 1 < n and chars[i] == "/" and chars[i + 1] == "*":
                chars[i] = " "
                chars[i + 1] = " "
                i += 2
                while i + 1 < n and not (chars[i] == "*" and chars[i + 1] == "/"):
                    if chars[i] != "\n":
                        chars[i] = " "
                    i += 1
                if i + 1 < n:
                    chars[i] = " "
                    chars[i + 1] = " "
                    i += 2
                continue
            # Mask single-line comments: // ...
            if i + 1 < n and chars[i] == "/" and chars[i + 1] == "/":
                chars[i] = " "
                chars[i + 1] = " "
                i += 2
                while i < n and chars[i] != "\n":
                    chars[i] = " "
                    i += 1
                continue
            # Mask string literals: "..."
            if chars[i] == '"':
                chars[i] = " "
                i += 1
                while i < n and chars[i] != '"':
                    if chars[i] == "\\" and i + 1 < n:
                        chars[i] = " "
                        i += 1
                    if chars[i] != "\n":
                        chars[i] = " "
                    i += 1
                if i < n and chars[i] == '"':
                    chars[i] = " "
                    i += 1
                continue
            i += 1
        return "".join(chars)

    def _find_enclosing_java_class(self, masked: str, pos: int, default_name: str) -> str:
        """Determine the `$`-delimited enclosing Java class hierarchy at character offset `pos`.

        Scans `masked[:pos]` for `class`/`enum`/`interface` declarations and tracks brace depth
        (`{` and `}`) to pop nested classes when their scope closes before `pos`.
        """
        sub = masked[:pos]
        class_stack: List[str] = []
        depth_stack: List[int] = []
        current_depth = 0

        for token in _JAVA_CLASS_TOKEN_RE.finditer(sub):
            kind = token.group(1)
            name = token.group(2)
            brace = token.group(3)
            if kind:
                class_stack.append(name)
                depth_stack.append(current_depth)
            elif brace == "{":
                current_depth += 1
            elif brace == "}":
                current_depth -= 1
                while depth_stack and current_depth <= depth_stack[-1]:
                    class_stack.pop()
                    depth_stack.pop()

        if not class_stack:
            return default_name
        return "$".join(class_stack)

    # -------------------------------------------------------------------------
    # C++ JNI Source Parsing
    # -------------------------------------------------------------------------

    def _parse_cpp_sources(self) -> None:
        """Recursively walk `self.cpp_dir` and parse all `.cpp` JNI implementation files."""
        if not self.cpp_dir.exists():
            return

        for root, _, files in os.walk(self.cpp_dir):
            for file_name in files:
                if not file_name.endswith(".cpp"):
                    continue

                file_path = Path(root) / file_name
                try:
                    content = file_path.read_text(encoding="utf-8")
                except Exception:
                    continue

                self._parse_single_cpp_file(file_path, content)

    def _parse_single_cpp_file(self, file_path: Path, content: str) -> None:
        """Extract `JNIEXPORT` function definitions and JNI reflection calls from a `.cpp` file."""
        line_offsets = _compute_line_offsets(content)

        # 1. Parse exported JNI function definitions
        for m in _CPP_JNI_FUNC_RE.finditer(content):
            start_pos = m.start()
            line_num = _get_line_num(line_offsets, start_pos)
            ret_type, full_symbol, pkg_raw, cls_name, method_name, overload_sig, params_str = (
                m.groups()
            )

            package = pkg_raw.replace("_", ".")
            raw_params = [p.strip() for p in params_str.split(",") if p.strip()]

            types = []
            for rp in raw_params:
                tokens = rp.split()
                if tokens:
                    t = tokens[0]
                    if t == "const" and len(tokens) > 1:
                        t = tokens[1]
                    types.append(t)

            # Skip leading (JNIEnv* env, jclass/jobject clazz) parameters
            param_types = types[2:] if len(types) >= 2 else []

            fn = CppJniFunction(
                package=package,
                class_name=cls_name,
                method_name=method_name,
                overload_sig=overload_sig,
                return_type=ret_type.strip(),
                param_types=param_types,
                file_path=file_path,
                line=line_num,
                full_symbol=full_symbol,
            )
            self.cpp_jni_functions.setdefault((package, cls_name), []).append(fn)

        # 2. Parse JNI Reflection Calls: FindClass, GetFieldID, GetMethodID
        var_to_class: Dict[str, str] = {}
        for m in _CPP_FIND_CLASS_RE.finditer(content):
            var_to_class[m.group(1)] = m.group(2).replace("/", ".")

        # Track GetObjectClass lookups inside JNI functions by correlating parameter index with Java
        for m in _CPP_GET_OBJ_CLASS_RE.finditer(content):
            clazz_var, obj_var = m.groups()
            fn_m = list(_CPP_JNI_HEADER_RE.finditer(content[: m.start()]))
            if fn_m:
                last_fn = fn_m[-1]
                pkg_raw, cls_name, method_name, _, p_str = last_fn.groups()
                p_tokens = [p.strip() for p in p_str.split(",") if p.strip()][2:]
                p_names = [p.split()[-1].lstrip("*&") for p in p_tokens if p.split()]
                if obj_var in p_names:
                    idx = p_names.index(obj_var)
                    full_jname = f"{pkg_raw.replace('_', '.')}.{cls_name}"
                    j_methods = self.java_native_methods.get(full_jname, [])
                    matching_jm = [jm for jm in j_methods if jm.method_name == method_name]
                    if matching_jm and idx < len(matching_jm[0].param_types):
                        raw_t = matching_jm[0].param_types[idx]
                        if "." in raw_t:
                            parts = raw_t.split(".")
                            var_to_class[clazz_var] = (
                                f"com.google.android.filament.{parts[0]}${parts[1]}"
                            )
                        else:
                            var_to_class[clazz_var] = f"com.google.android.filament.{raw_t}"

        # GetFieldID lookups
        for m in _CPP_FIELD_ID_RE.finditer(content):
            line_num = _get_line_num(line_offsets, m.start())
            clazz_var, field_name, sig = m.groups()
            target_class = var_to_class.get(
                clazz_var, f"com.google.android.filament.{file_path.stem}"
            )
            self.cpp_reflection_fields.append(
                CppReflectionField(
                    target_class=target_class,
                    field_name=field_name,
                    signature=sig,
                    file_path=file_path,
                    line=line_num,
                )
            )

        # GetMethodID lookups
        for m in _CPP_METHOD_ID_RE.finditer(content):
            line_num = _get_line_num(line_offsets, m.start())
            is_static_str, clazz_var, method_name, sig = m.groups()
            target_class = var_to_class.get(
                clazz_var, f"com.google.android.filament.{file_path.stem}"
            )
            self.cpp_reflection_methods.append(
                CppReflectionMethod(
                    target_class=target_class,
                    method_name=method_name,
                    signature=sig,
                    is_static=bool(is_static_str),
                    file_path=file_path,
                    line=line_num,
                )
            )

    # -------------------------------------------------------------------------
    # Validation Rules
    # -------------------------------------------------------------------------

    def _validate_symbol_parity(self, result: ValidationResult) -> None:
        """Assert that every Java native method has a matching C++ JNI export and vice-versa.

        Handles both top-level classes and nested classes (where JNI symbols may either use
        `_00024` inner-class mangling or reside directly on the enclosing class).
        """
        for full_cname, j_methods in self.java_native_methods.items():
            simple_cls = full_cname.split(".")[-1].split("$")[0]
            if self.target_classes and simple_cls not in self.target_classes:
                continue

            pkg = ".".join(full_cname.split(".")[:-1])
            cls_name = full_cname.split(".")[-1].replace("$", "_00024")

            cpp_matches = self.cpp_jni_functions.get((pkg, cls_name), [])
            if not cpp_matches and "$" in full_cname:
                base_cls = full_cname.split("$")[0].split(".")[-1]
                cpp_matches = self.cpp_jni_functions.get((pkg, base_cls), [])

            cpp_method_names = {fn.method_name for fn in cpp_matches}

            for jm in j_methods:
                if jm.method_name not in cpp_method_names:
                    result.add_error(
                        jm.file_path,
                        jm.line,
                        "symbol_parity",
                        f"Java native method '{full_cname}.{jm.method_name}' has no matching "
                        f"C++ JNI implementation (expected 'Java_{pkg.replace('.', '_')}_{cls_name}_{jm.method_name}')",
                    )

        for (pkg, cls_name), c_functions in self.cpp_jni_functions.items():
            simple_cls = cls_name.split("_00024")[0]
            if self.target_classes and simple_cls not in self.target_classes:
                continue

            full_cname = f"{pkg}.{cls_name.replace('_00024', '$')}"
            j_methods = self.java_native_methods.get(full_cname, [])
            if not j_methods and "$" in full_cname:
                base_cname = full_cname.split("$")[0]
                j_methods = self.java_native_methods.get(base_cname, [])

            j_method_names = {jm.method_name for jm in j_methods}

            for cfn in c_functions:
                if cfn.method_name not in j_method_names:
                    result.add_error(
                        cfn.file_path,
                        cfn.line,
                        "symbol_parity",
                        f"C++ JNI function '{cfn.full_symbol}' has no corresponding Java native "
                        f"declaration in class '{full_cname}'",
                    )

    def _validate_signatures(self, result: ValidationResult) -> None:
        """Verify parameter count and type compatibility between Java native methods and C++ JNI.

        Normalizes Java parameter types (`normalize_java_type`) and C++ JNI parameter types
        (`normalize_cpp_jni_type`) and checks that each Java overload matches at least one C++
        overload of the same method name. Allows `jobject` on the Java side to match specialized
        JNI reference types (`jobject`, `jclass`, `jobjectArray`, `jstring`) on the C++ side.
        """
        for full_cname, j_methods in self.java_native_methods.items():
            simple_cls = full_cname.split(".")[-1].split("$")[0]
            if self.target_classes and simple_cls not in self.target_classes:
                continue

            pkg = ".".join(full_cname.split(".")[:-1])
            cls_name = full_cname.split(".")[-1].replace("$", "_00024")

            cpp_matches = self.cpp_jni_functions.get((pkg, cls_name), [])
            if not cpp_matches and "$" in full_cname:
                base_cls = full_cname.split("$")[0].split(".")[-1]
                cpp_matches = self.cpp_jni_functions.get((pkg, base_cls), [])

            cpp_by_method: Dict[str, List[CppJniFunction]] = {}
            for fn in cpp_matches:
                cpp_by_method.setdefault(fn.method_name, []).append(fn)

            j_by_method: Dict[str, List[JavaNativeMethod]] = {}
            for jm in j_methods:
                j_by_method.setdefault(jm.method_name, []).append(jm)

            for mname, overloads in j_by_method.items():
                c_overloads = cpp_by_method.get(mname, [])
                if not c_overloads:
                    continue

                for jm in overloads:
                    norm_j_params = [normalize_java_type(t) for t in jm.param_types]
                    matching_c = None

                    for cfn in c_overloads:
                        norm_c_params = [normalize_cpp_jni_type(t) for t in cfn.param_types]
                        if len(norm_j_params) != len(norm_c_params):
                            continue

                        types_match = True
                        for jp, cp in zip(norm_j_params, norm_c_params):
                            if jp == "jobject" and cp in (
                                "jobject",
                                "jclass",
                                "jobjectArray",
                                "jstring",
                            ):
                                continue
                            if jp != cp:
                                types_match = False
                                break

                        if types_match:
                            matching_c = cfn
                            break

                    if not matching_c:
                        candidates_str = ", ".join(
                            f"({', '.join(cfn.param_types)})" for cfn in c_overloads
                        )
                        result.add_error(
                            jm.file_path,
                            jm.line,
                            "signature",
                            f"Signature mismatch for '{full_cname}.{mname}': Java signature is "
                            f"({', '.join(jm.param_types)}) [mapped: {norm_j_params}], but available "
                            f"C++ JNI overloads are: {candidates_str}",
                        )

    def _validate_reflection_fields(self, result: ValidationResult) -> None:
        """Verify that all `GetFieldID` calls query fields that exist in the target Java class with matching types."""
        for rf in self.cpp_reflection_fields:
            target_class = rf.target_class
            simple_cls = target_class.split(".")[-1].split("$")[0]
            if self.target_classes and simple_cls not in self.target_classes:
                continue

            declared_fields = self.java_class_field_names.get(target_class)
            if declared_fields is None:
                candidates = self.java_classes_by_simple_name.get(
                    target_class.split(".")[-1], []
                )
                if candidates:
                    target_class = candidates[0]
                    declared_fields = self.java_class_field_names.get(target_class)

            if declared_fields is None or rf.field_name not in declared_fields:
                # Fallback: search across all known classes for this field name
                matching_classes = [
                    cls
                    for (cls, fn) in self.java_fields
                    if fn == rf.field_name
                ]
                if len(matching_classes) == 1:
                    target_class = matching_classes[0]
                    declared_fields = self.java_class_field_names.get(target_class)

            if declared_fields is None:
                result.add_error(
                    rf.file_path,
                    rf.line,
                    "reflection",
                    f"GetFieldID queries non-existent or unparsed Java class '{rf.target_class}' "
                    f"for field '{rf.field_name}'",
                )
                continue

            if rf.field_name not in declared_fields:
                result.add_error(
                    rf.file_path,
                    rf.line,
                    "reflection",
                    f"Reflection field lookup failed: Java class '{target_class}' does not contain "
                    f"field '{rf.field_name}' (expected JNI signature '{rf.signature}')",
                )
                continue

            field_obj = self.java_fields.get((target_class, rf.field_name))
            if field_obj and rf.signature in SIG_TO_JAVA_TYPE:
                expected_java_type = SIG_TO_JAVA_TYPE[rf.signature]
                if field_obj.field_type != expected_java_type:
                    result.add_error(
                        rf.file_path,
                        rf.line,
                        "reflection",
                        f"Field type mismatch: '{target_class}.{rf.field_name}' is declared as "
                        f"'{field_obj.field_type}', but JNI GetFieldID requested signature '{rf.signature}' "
                        f"('{expected_java_type}')",
                    )
