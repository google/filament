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

"""Pipeline orchestration and CLI driver for JavaGen.

Architectural Purpose:
    This module coordinates the end-to-end execution of the Java and JNI binding
    generation pipeline for a single C++ header's JSON Intermediate Representation (IR).
    It manages the ingestion of AST data, discovers and registers sibling IR types in
    the workspace, associates nested builders and aggregate structs with their enclosing
    parent classes, instantiates `ClassContext` data models, and coordinates writing
    the generated `.java` and `.cpp` translation units to disk.

Key Responsibilities:
    1. IR Ingestion & Validation (`process_file`):
       Deserializes the JSON file produced by `extractor.py` and raises typed
       `IRParseError` exceptions if the JSON is malformed or inaccessible.
    2. Sibling Type Discovery & Cross-Header Indexing:
       Discovers and registers sibling `.json` files in the same directory into
       `config.KNOWN_CLASSES` (cached by `(abspath, mtime)` in `_SIBLING_IR_CACHE`)
       so that composite aggregate structs (e.g. `Box`, `Viewport`) referenced in
       foreign methods can have their memory layouts and leaf fields resolved during
       parameter unrolling.
    3. Structural Filtering & Builder Association:
       - Detects classes annotated with `UTILS_NOAPIGEN` (`apigen:skip`, `no_apigen`)
         and excludes them from code generation.
       - Maps nested builder classes (e.g. `LightManager::Builder`) and nested structs
         (e.g. `LightManager::ShadowOptions`) to their respective parent classes,
         suppressing separate standalone file emission and nesting them as inner classes.
    4. Top-Level Merging & Utility Class Overrides:
       Merges header-level enums and type aliases into the primary class model.
       Supports `--class-name` overrides (e.g. mapping `Color.h` to Java `Colors`).
    5. Code Generation & Disk Output:
       Invokes `ClassContext.generate_java()` and `ClassContext.generate_jni()`
       and writes the formatted strings to the designated `--java-dir` and `--jni-dir`.
"""

import argparse
import collections
import glob
import json
import os
from typing import Any, Dict, List, Optional, Tuple

from .config import register_known_classes
from .context import ClassContext
from .errors import IRParseError
from .utils import has_skip_attr

# Cache of parsed sibling IR files: abspath -> (mtime, classes, referenced_classes)
# Avoids re-parsing sibling JSON files across multiple target invocations in the same process.
_SIBLING_IR_CACHE: Dict[str, Tuple[float, List[Dict[str, Any]], List[Dict[str, Any]]]] = {}


def _is_builder_ir(clazz: Dict[str, Any]) -> bool:
    """Return True if a class IR dictionary represents a Builder class."""
    return bool(
        clazz.get("is_builder")
        or clazz.get("archetype") == "builder"
        or clazz.get("category") == "builder"
        or (clazz.get("name") == "Builder" and clazz.get("parent_class"))
    )


def _register_sibling_irs(ir_file: str) -> None:
    """Discover and register classes from sibling JSON IR files in the same directory.

    When generating bindings for a class like Frustum, it may reference Box or Viewport.
    Sibling JSON files contain the field definitions necessary to unroll those structs.
    """
    target_abs = os.path.abspath(ir_file)
    ir_dir = os.path.dirname(target_abs)
    if not os.path.isdir(ir_dir):
        return

    for sibling in glob.glob(os.path.join(ir_dir, "*.json")):
        sibling_abs = os.path.abspath(sibling)
        if sibling_abs == target_abs:
            continue
        try:
            mtime = os.path.getmtime(sibling_abs)
            cached = _SIBLING_IR_CACHE.get(sibling_abs)
            if cached is not None and cached[0] == mtime:
                classes, ref_classes = cached[1], cached[2]
            else:
                with open(sibling_abs, "r") as sf:
                    sdata = json.load(sf)
                if not isinstance(sdata, dict):
                    continue
                classes = sdata.get("classes", [])
                ref_classes = sdata.get("referenced_classes", [])
                _SIBLING_IR_CACHE[sibling_abs] = (mtime, classes, ref_classes)

            if classes:
                register_known_classes(classes)
            if ref_classes:
                register_known_classes(ref_classes)
        except Exception:
            # Ignore unreadable or partially written sibling files during concurrent builds
            pass


def process_file(
    ir_file: str,
    java_dir: str,
    jni_dir: str,
    diagnostic_mode: bool = False,
    class_name: Optional[str] = None,
) -> None:
    """Process a JSON IR file and generate matching Java and JNI C++ binding files.

    Architectural Flow:
        1. Read and deserialize the primary JSON IR file.
        2. Index classes from the primary IR and referenced classes into `KNOWN_CLASSES`.
        3. Scan sibling `.json` files in the same directory and index their classes,
           enabling cross-header aggregate struct leaf traversal.
        4. Detect and catalog nested `Builder` classes by parent class name.
        5. Detect and catalog nested aggregate structs by parent class name.
        6. Iterate non-nested classes, merge top-level enums/aliases, evaluate name
           overrides, and instantiate a `ClassContext`.
        7. Generate Java code via `ctx.generate_java()` and write to `<java_dir>/<Name>.java`.
        8. Generate JNI C++ code via `ctx.generate_jni()` and write to `<jni_dir>/<Name>.cpp`.

    Args:
        ir_file: Filesystem path to the input JSON IR file.
        java_dir: Destination directory where `.java` wrapper files will be emitted.
        jni_dir: Destination directory where `.cpp` JNI bridge files will be emitted.
        diagnostic_mode: If True, enables verbose diagnostic warnings for unmapped types.
        class_name: Optional override name for the target class (e.g. 'Colors' for Color.h).

    Raises:
        IRParseError: If the input file cannot be opened or parsed as valid JSON.
        IOError: If destination output directories are unwritable.

    Side Effects:
        - Populates the module-global `config.KNOWN_CLASSES` dictionary.
        - Creates or overwrites `.java` and `.cpp` files in `java_dir` and `jni_dir`.
    """
    # Step 1: Read and parse the JSON IR file from disk
    try:
        with open(ir_file, "r") as f:
            data: Dict[str, Any] = json.load(f)
    except Exception as exc:
        raise IRParseError(ir_file, str(exc)) from exc

    # Step 2: Register current file classes and referenced classes into known registry
    register_known_classes(data.get("classes", []))
    register_known_classes(data.get("referenced_classes", []))

    # Step 3: Discover and register known classes from sibling JSON files in same directory
    _register_sibling_irs(ir_file)

    # Step 4: Resolve source header path for #include directive in JNI file
    source_header = "tests/java/00_basic.h"  # Fallback for unit test IRs without location
    if data.get("classes"):
        target_cls = (
            next((c for c in data["classes"] if c.get("name") == class_name), None)
            if class_name
            else None
        ) or data["classes"][0]
        source_header = target_cls.get("location", {}).get(
            "file", f"filament/{target_cls['name']}.h"
        )

    # Step 5 & 6: Catalog nested builders and nested aggregate structs by parent class name
    builders_by_parent: Dict[str, Dict[str, Any]] = {}
    nested_structs_by_parent: Dict[str, List[Dict[str, Any]]] = collections.defaultdict(list)

    for clazz in data.get("classes", []):
        # Skip builder or nested struct if annotated with UTILS_NOAPIGEN
        if has_skip_attr(clazz):
            continue
        parent = clazz.get("parent_class")
        if not parent:
            continue
        if _is_builder_ir(clazz) or clazz.get("name") == "Builder":
            builders_by_parent[parent] = clazz
        elif clazz.get("is_aggregate") or (
            len(clazz.get("fields", [])) > 0
            and (
                clazz.get("category") == "struct"
                or clazz.get("type", {}).get("category") == "struct"
            )
        ):
            nested_structs_by_parent[parent].append(clazz)

    # Step 7: Iterate over non-nested classes and emit bindings
    valid_classes = [c for c in data.get("classes", []) if not has_skip_attr(c)]
    has_exact_class_match = any(c.get("name") == class_name for c in data.get("classes", []))

    for clazz in valid_classes:
        # Skip standalone file generation for nested classes (they emit inside parent classes)
        if (clazz.get("is_nested") and clazz.get("parent_class")) or _is_builder_ir(clazz):
            continue

        # Step 7a: Merge top-level header enums into the class definition
        clazz_enums = list(clazz.get("enums", []))
        existing_enum_names = {e["name"] for e in clazz_enums}
        for e in data.get("enums", []):
            if e["name"] not in existing_enum_names:
                clazz_enums.append(e)
        clazz["enums"] = clazz_enums

        # Step 7b: Merge top-level header type aliases into the class definition
        clazz_aliases = list(clazz.get("aliases", []))
        existing_alias_names = {a["name"] for a in clazz_aliases}
        for a in data.get("aliases", []):
            if a["name"] not in existing_alias_names:
                clazz_aliases.append(a)
        clazz["aliases"] = clazz_aliases

        # Step 7c: Evaluate target class name override (e.g. for utility classes)
        override = None
        if class_name:
            if clazz.get("name") == class_name:
                override = class_name
            elif not has_exact_class_match and valid_classes and clazz == valid_classes[0]:
                # If class_name does not match any class by name, apply to primary class in header
                override = class_name
            else:
                continue

        # Step 7d: Instantiate the ClassContext data model
        ctx = ClassContext(
            clazz,
            source_header,
            name_override=override,
            builder_ir=builders_by_parent.get(clazz.get("name")),
            nested_structs_map=nested_structs_by_parent,
        )
        ctx.diagnostic_mode = diagnostic_mode

        # Step 8: Emit Java wrapper class file (.java)
        java_path = os.path.join(java_dir, f"{ctx.name}.java")
        with open(java_path, "w") as jf:
            jf.write(ctx.generate_java())

        # Step 9: Emit JNI C++ bridge file (.cpp)
        jni_path = os.path.join(jni_dir, f"{ctx.name}.cpp")
        with open(jni_path, "w") as cf:
            cf.write(ctx.generate_jni())


def main(argv: Optional[List[str]] = None) -> None:
    """CLI entry point and command-line argument parser for JavaGen.

    Parses command-line arguments and dispatches processing to `process_file`.

    Command-Line Arguments:
        ir: Path to input JSON IR file.
        --java-dir: Directory to place generated `.java` files (default current dir).
        --jni-dir: Directory to place generated `.cpp` files (default current dir).
        --class-name: Optional override for generated class name.
        -d, --diagnostic: Enable diagnostic logging for unmapped types.

    Args:
        argv: Optional command-line argument list (defaults to `sys.argv[1:]`).
    """
    # Step 1: Configure command-line arguments
    parser = argparse.ArgumentParser(
        description="APIGen Java Generator: Emits Java wrappers and JNI C++ bridges from JSON IR."
    )
    parser.add_argument("ir", help="Path to input JSON IR file produced by extractor.py")
    parser.add_argument("--java-dir", default=".", help="Output directory for generated Java files")
    parser.add_argument("--jni-dir", default=".", help="Output directory for generated JNI C++ files")
    parser.add_argument("--class-name", default=None, help="Override Java and JNI target class name")
    parser.add_argument(
        "-d",
        "--diagnostic",
        action="store_true",
        help="Enable diagnostic warnings for unknown or unmapped types",
    )

    # Step 2: Parse arguments
    args = parser.parse_args(argv)

    # Step 3: Dispatch pipeline execution
    process_file(
        args.ir,
        args.java_dir,
        args.jni_dir,
        args.diagnostic,
        class_name=args.class_name,
    )


# Step 4: Standalone script execution hook
if __name__ == "__main__":
    main()
