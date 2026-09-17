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

from dataclasses import dataclass, field
import os
from pathlib import Path
import re
from typing import Dict, List, Optional, Set, Tuple


@dataclass
class ValidationDiagnostic:
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
    diagnostics: List[ValidationDiagnostic] = field(default_factory=list)

    @property
    def errors(self) -> List[ValidationDiagnostic]:
        return [d for d in self.diagnostics if d.severity == "ERROR"]

    @property
    def warnings(self) -> List[ValidationDiagnostic]:
        return [d for d in self.diagnostics if d.severity == "WARNING"]

    @property
    def is_clean(self) -> bool:
        return len(self.errors) == 0

    def add_error(self, file_path: Path, line: int, category: str, message: str) -> None:
        self.diagnostics.append(
            ValidationDiagnostic(file_path, line, "ERROR", category, message)
        )

    def add_warning(self, file_path: Path, line: int, category: str, message: str) -> None:
        self.diagnostics.append(
            ValidationDiagnostic(file_path, line, "WARNING", category, message)
        )

    def format_report(self, verbose: bool = False) -> str:
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
    class_name: str
    method_name: str
    return_type: str
    param_types: List[str]
    file_path: Path
    line: int


@dataclass
class JavaField:
    class_name: str
    field_name: str
    field_type: str
    file_path: Path
    line: int


@dataclass
class CppJniFunction:
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
    target_class: str
    field_name: str
    signature: str
    file_path: Path
    line: int


@dataclass
class CppReflectionMethod:
    target_class: str
    method_name: str
    signature: str
    is_static: bool
    file_path: Path
    line: int


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
    """Normalize a Java type name into a canonical JNI type representation."""
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
    """Normalize a C++ JNI parameter type into a canonical JNI type representation."""
    t = t.strip().rstrip("*&")
    if t.startswith("const "):
        t = t[6:].strip()
    return CPP_TO_JNI_TYPE.get(t, "jobject")


class BindingValidator:
    """Static parity and reflection validator for Java and JNI C++ bindings."""

    def __init__(
        self,
        java_dir: Path,
        cpp_dir: Path,
        verbose: bool = False,
        target_classes: Optional[Set[str]] = None,
    ):
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
        """Executes all validation checks and returns the aggregated result."""
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
        rel_path = file_path.relative_to(self.java_dir)
        package = ".".join(rel_path.parent.parts) if len(rel_path.parent.parts) > 0 else ""

        pkg_match = re.search(r"^\s*package\s+([\w\.]+);", content, re.MULTILINE)
        if pkg_match:
            package = pkg_match.group(1)

        line_offsets = []
        cur_offset = 0
        for line in content.splitlines(keepends=True):
            line_offsets.append(cur_offset)
            cur_offset += len(line)

        def get_line_num(pos: int) -> int:
            import bisect
            idx = bisect.bisect_right(line_offsets, pos)
            return max(1, idx)

        masked = self._mask_code(content)

        native_pattern = re.compile(
            r"\bnative\s+([\w<>\[\],\s\?]+?)\s+(n\w+)\s*\((.*?)\)\s*;",
            re.DOTALL
        )
        for m in native_pattern.finditer(content):
            start_pos = m.start()
            line_num = get_line_num(start_pos)
            ret_type, method_name, params_str = m.groups()
            ret_type = ret_type.strip()

            enclosing_class = self._find_enclosing_java_class(masked, start_pos, file_path.stem)
            full_class_name = f"{package}.{enclosing_class}" if package else enclosing_class

            clean_params_str = re.sub(r"@\w+(?:\([^)]*\))?\s*", "", params_str)
            param_types = []
            if clean_params_str.strip():
                for p in clean_params_str.split(","):
                    p = p.strip()
                    parts = p.split()
                    if parts:
                        param_types.append(parts[0])

            method = JavaNativeMethod(
                class_name=full_class_name,
                method_name=method_name,
                return_type=ret_type,
                param_types=param_types,
                file_path=file_path,
                line=line_num
            )
            self.java_native_methods.setdefault(full_class_name, []).append(method)

        field_pattern = re.compile(
            r"^\s*(?:(?:public|protected|private|final|static|volatile|transient)\s+)*"
            r"(?!class\b|enum\b|interface\b|return\b|new\b|import\b|package\b)"
            r"([\w<>\[\]\.]+)\s+(\w+)\s*(?:=\s*[^;]+)?\s*;",
            re.MULTILINE
        )
        for m in field_pattern.finditer(content):
            start_pos = m.start()
            line_num = get_line_num(start_pos)
            field_type, field_name = m.groups()
            enclosing_class = self._find_enclosing_java_class(masked, start_pos, file_path.stem)
            full_class_name = f"{package}.{enclosing_class}" if package else enclosing_class

            f_obj = JavaField(
                class_name=full_class_name,
                field_name=field_name,
                field_type=field_type,
                file_path=file_path,
                line=line_num
            )
            self.java_fields[(full_class_name, field_name)] = f_obj
            self.java_class_field_names.setdefault(full_class_name, set()).add(field_name)

            simple_name = enclosing_class
            self.java_classes_by_simple_name.setdefault(simple_name, []).append(full_class_name)

    def _mask_code(self, code: str) -> str:
        """Mask comments and string literals with spaces to allow reliable syntax parsing."""
        chars = list(code)
        n = len(chars)
        i = 0
        while i < n:
            if i + 1 < n and chars[i] == '/' and chars[i + 1] == '*':
                chars[i] = ' '
                chars[i + 1] = ' '
                i += 2
                while i + 1 < n and not (chars[i] == '*' and chars[i + 1] == '/'):
                    if chars[i] != '\n':
                        chars[i] = ' '
                    i += 1
                if i + 1 < n:
                    chars[i] = ' '
                    chars[i + 1] = ' '
                    i += 2
                continue
            if i + 1 < n and chars[i] == '/' and chars[i + 1] == '/':
                chars[i] = ' '
                chars[i + 1] = ' '
                i += 2
                while i < n and chars[i] != '\n':
                    chars[i] = ' '
                    i += 1
                continue
            if chars[i] == '"':
                chars[i] = ' '
                i += 1
                while i < n and chars[i] != '"':
                    if chars[i] == '\\' and i + 1 < n:
                        chars[i] = ' '
                        i += 1
                    if chars[i] != '\n':
                        chars[i] = ' '
                    i += 1
                if i < n and chars[i] == '"':
                    chars[i] = ' '
                    i += 1
                continue
            i += 1
        return "".join(chars)

    def _find_enclosing_java_class(self, masked: str, pos: int, default_name: str) -> str:
        """Walks backwards through masked Java code to determine enclosing class hierarchy."""
        sub = masked[:pos]
        tokens = re.finditer(r"\b(class|enum|interface)\s+(\w+)|([{}])", sub)
        class_stack: List[str] = []
        depth_stack: List[int] = []
        current_depth = 0

        for token in tokens:
            kind = token.group(1)
            name = token.group(2)
            brace = token.group(3)
            if kind:
                class_stack.append(name)
                depth_stack.append(current_depth)
            elif brace == '{':
                current_depth += 1
            elif brace == '}':
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
        line_offsets = []
        cur_offset = 0
        for line in content.splitlines(keepends=True):
            line_offsets.append(cur_offset)
            cur_offset += len(line)

        def get_line_num(pos: int) -> int:
            import bisect
            idx = bisect.bisect_right(line_offsets, pos)
            return max(1, idx)

        # 1. Parse exported JNI function definitions
        jni_func_pattern = re.compile(
            r'extern\s+"C"\s+JNIEXPORT\s+([\w\*\s]+?)\s+JNICALL\s+'
            r'(Java_([a-zA-Z0-9_]+?)_([A-Za-z0-9]+)_(n[A-Za-z0-9]+?)(?:__([A-Za-z0-9_]+))?)\s*\((.*?)\)\s*\{',
            re.DOTALL
        )
        for m in jni_func_pattern.finditer(content):
            start_pos = m.start()
            line_num = get_line_num(start_pos)
            ret_type, full_symbol, pkg_raw, cls_name, method_name, overload_sig, params_str = m.groups()

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
                full_symbol=full_symbol
            )
            self.cpp_jni_functions.setdefault((package, cls_name), []).append(fn)

        # 2. Parse JNI Reflection Calls: FindClass, GetFieldID, GetMethodID
        var_to_class: Dict[str, str] = {}
        for m in re.finditer(r'(\w+)\s*=\s*env->FindClass\s*\(\s*"([^"]+)"\s*\)', content):
            var_to_class[m.group(1)] = m.group(2).replace("/", ".")

        # Track GetObjectClass lookups inside JNI functions
        for m in re.finditer(r'(\w+)\s*=\s*env->GetObjectClass\s*\(\s*(\w+)\s*\)', content):
            clazz_var, obj_var = m.groups()
            fn_m = list(re.finditer(
                r'Java_([a-zA-Z0-9_]+?)_([A-Za-z0-9]+)_(n[A-Za-z0-9]+?)(?:__([A-Za-z0-9_]+))?\s*\((.*?)\)\s*\{',
                content[:m.start()],
                re.DOTALL
            ))
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
                            var_to_class[clazz_var] = f"com.google.android.filament.{parts[0]}${parts[1]}"
                        else:
                            var_to_class[clazz_var] = f"com.google.android.filament.{raw_t}"

        # GetFieldID lookups
        field_id_pattern = re.compile(
            r'env->Get(?:Static)?FieldID\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
        )
        for m in field_id_pattern.finditer(content):
            line_num = get_line_num(m.start())
            clazz_var, field_name, sig = m.groups()
            target_class = var_to_class.get(clazz_var, f"com.google.android.filament.{file_path.stem}")
            self.cpp_reflection_fields.append(
                CppReflectionField(
                    target_class=target_class,
                    field_name=field_name,
                    signature=sig,
                    file_path=file_path,
                    line=line_num
                )
            )

        # GetMethodID lookups
        method_id_pattern = re.compile(
            r'env->Get(Static)?MethodID\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\)'
        )
        for m in method_id_pattern.finditer(content):
            line_num = get_line_num(m.start())
            is_static_str, clazz_var, method_name, sig = m.groups()
            target_class = var_to_class.get(clazz_var, f"com.google.android.filament.{file_path.stem}")
            self.cpp_reflection_methods.append(
                CppReflectionMethod(
                    target_class=target_class,
                    method_name=method_name,
                    signature=sig,
                    is_static=bool(is_static_str),
                    file_path=file_path,
                    line=line_num
                )
            )

    # -------------------------------------------------------------------------
    # Validation Rules
    # -------------------------------------------------------------------------

    def _validate_symbol_parity(self, result: ValidationResult) -> None:
        """Asserts that every Java native method has a matching C++ JNI export and vice-versa."""
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
                        f"C++ JNI implementation (expected 'Java_{pkg.replace('.', '_')}_{cls_name}_{jm.method_name}')"
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
                        f"declaration in class '{full_cname}'"
                    )

    def _validate_signatures(self, result: ValidationResult) -> None:
        """Verifies parameter count and type compatibility between Java native methods and C++ JNI."""
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
                            if jp == "jobject" and cp in ("jobject", "jclass", "jobjectArray", "jstring"):
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
                            f"C++ JNI overloads are: {candidates_str}"
                        )

    def _validate_reflection_fields(self, result: ValidationResult) -> None:
        """Verifies that all GetFieldID calls query fields that exist in the target Java class with matching types."""
        for rf in self.cpp_reflection_fields:
            target_class = rf.target_class
            simple_cls = target_class.split(".")[-1].split("$")[0]
            if self.target_classes and simple_cls not in self.target_classes:
                continue

            declared_fields = self.java_class_field_names.get(target_class)
            if declared_fields is None:
                candidates = self.java_classes_by_simple_name.get(target_class.split(".")[-1], [])
                if candidates:
                    target_class = candidates[0]
                    declared_fields = self.java_class_field_names.get(target_class)

            if declared_fields is None or rf.field_name not in declared_fields:
                # Fallback: search across all known classes for this field name
                matching_classes = [
                    cls for (cls, fn), fobj in self.java_fields.items()
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
                    f"for field '{rf.field_name}'"
                )
                continue

            if rf.field_name not in declared_fields:
                result.add_error(
                    rf.file_path,
                    rf.line,
                    "reflection",
                    f"Reflection field lookup failed: Java class '{target_class}' does not contain "
                    f"field '{rf.field_name}' (expected JNI signature '{rf.signature}')"
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
                        f"('{expected_java_type}')"
                    )
