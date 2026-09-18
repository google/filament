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

"""Backward-compatible facade and CLI entry point for Filament JavaGen.

Architectural Purpose:
    This module serves as the public facade and backward-compatible command-line
    interface (CLI) driver for the Filament Java and JNI C++ binding generator.
    Historically, the entire binding pipeline resided in a monolithic 8,000+ line
    script at this exact path. To preserve seamless integration with existing
    build orchestration scripts (such as `tools/apigen/generate_android.py`) and
    comprehensive unit test suites (such as `tools/apigen/tests/test_javagen.py`),
    this file remains at `tools/apigen/javagen.py` and transparently re-exports
    all primary classes, configuration dictionaries, utilities, and execution
    routines from the modular `javagen` subpackage.

Execution & Lifecycle:
    1. Path Resolution: When executed directly from the shell or invoked as a
       subprocess, this module ensures its parent directory (`tools/apigen/`) is
       prepended to `sys.path` so that relative package imports of `javagen.*`
       succeed regardless of the caller's current working directory.
    2. Symbol Re-export: Re-exports core pipeline constructs including `ClassContext`,
       `JavaEmitter`, `JniEmitter`, `TypeResolver`, `process_file`, and configuration
       tables (`TYPE_MAP`, `MATH_TYPES`, etc.).
    3. CLI Invocation: When invoked as `__main__`, delegates argument parsing and
       code generation dispatch directly to `javagen.orchestrator.main()`.

Re-exported API Groups:
    - Pipeline Drivers: `process_file`, `main`
    - Core Models & Engines: `ClassContext`, `JavaEmitter`, `JniEmitter`, `TypeResolver`
    - Documentation Helpers: `generate_javadoc`, `markdown_to_javadoc`, `format_see_tag`
    - AST & Identifier Utilities: `get_effective_method_name`, `sanitize_identifier`
    - Configuration & Registries: `TYPE_MAP`, `MATH_TYPES`, `KNOWN_CLASSES`,
      `VALUE_TYPES`, `TYPE_PATTERNS`, `register_known_classes`
    - Exceptions: `JavaGenError`, `TypeResolutionError`, `TemplateExpansionError`,
      `EmissionError`, `IRParseError`
"""

import sys
from pathlib import Path

# Step 1: Ensure the directory containing the javagen subpackage is in sys.path.
# When this script is invoked directly (e.g. `python3 tools/apigen/javagen.py`),
# Python automatically adds the script's immediate directory to sys.path[0].
# However, if imported programmatically from an arbitrary working directory or
# test harness without setting PYTHONPATH, we explicitly verify and inject
# the parent directory to guarantee unambiguous package discovery.
_PACKAGE_DIR = Path(__file__).resolve().parent
if str(_PACKAGE_DIR) not in sys.path:
    sys.path.insert(0, str(_PACKAGE_DIR))

# Step 2: Import all core models, emitters, utilities, and configuration tables
# from the modular javagen package for public re-export.
from javagen import (
    ClassContext,
    EmissionError,
    IRParseError,
    JAVA_OBJECT_FINAL_NOARG_METHODS,
    JavaEmitter,
    JavaGenError,
    JniEmitter,
    KNOWN_CLASSES,
    MATH_TYPES,
    TYPE_MAP,
    TYPE_PATTERNS,
    TemplateExpansionError,
    TypeResolutionError,
    TypeResolver,
    VALUE_TYPES,
    format_see_tag,
    generate_javadoc,
    get_effective_method_name,
    main,
    markdown_to_javadoc,
    process_file,
    register_known_classes,
    sanitize_identifier,
)

# Step 3: Define __all__ for strict public API export semantics.
# External modules executing `from tools.apigen.javagen import *` receive exactly
# these public symbols, mirroring the historical monolithic script's namespace.
__all__ = [
    "ClassContext",
    "JavaEmitter",
    "JniEmitter",
    "TypeResolver",
    "process_file",
    "main",
    "generate_javadoc",
    "markdown_to_javadoc",
    "format_see_tag",
    "get_effective_method_name",
    "sanitize_identifier",
    "register_known_classes",
    "TYPE_MAP",
    "MATH_TYPES",
    "KNOWN_CLASSES",
    "VALUE_TYPES",
    "TYPE_PATTERNS",
    "JAVA_OBJECT_FINAL_NOARG_METHODS",
    "JavaGenError",
    "TypeResolutionError",
    "TemplateExpansionError",
    "EmissionError",
    "IRParseError",
]

# Step 4: CLI Entrypoint Guard.
# When executed directly as a standalone script (e.g., via CLI or subprocess in
# generate_android.py), delegate execution directly to the orchestrator driver.
if __name__ == "__main__":
    main()
