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

"""JavaGen package: Modular Java and JNI C++ binding generator for Filament.

Architectural Overview:
    JavaGen is the primary code generator of the Filament API binding generation
    toolchain. It consumes the JSON Intermediate Representation (IR) generated
    by `extractor.py` (which parses C++ headers using libclang) and generates two
    synchronized production artifacts per C++ class:
    1. An idiomatic Java class wrapper (`.java`) targeting Android and Desktop JVM.
    2. A high-performance, zero-cost JNI C++ translation unit (`.cpp`) bridging
       the Java native declarations directly to the Filament engine.

Pipeline Lifecycle & Data Flow:
    1. Ingestion (`orchestrator.py`):
       Reads the JSON IR file, discovers sibling IR files in the workspace, and
       populates the global known class registry (`config.KNOWN_CLASSES`).
    2. Contextual Modeling (`context.py`):
       Instantiates a `ClassContext` for each class, unrolling SFINAE trait
       specializations into concrete method variants, parsing AST attributes,
       and cataloging enums, nested builders, and type aliases.
    3. Type Resolution & Layout Computation (`type_resolver.py`):
       Queries `TypeResolver` to map C++ types to Java types, JNI types, and
       signature descriptors, resolving primitive array strides, buffer modes,
       and aggregate struct memory layouts.
    4. Code Emission (`java_emitter.py`, `jni_emitter.py`):
       - `JavaEmitter` synthesizes Java class declarations, builder inner classes,
         exploded zero-allocation struct fields, vector overloads, and native decls.
       - `JniEmitter` synthesizes zero-cost C++ translation units, `wrapJni` exception
         handling blocks, struct stack reconstructions, and NIO buffer transfers.
    5. Documentation Synthesis (`doc.py`):
       Parses CommonMark AST documentation blocks and renders standards-compliant
       HTML Javadoc blocks with parameter tags, return descriptions, and cross-links.

Module Breakdown:
    - config: Static type mapping tables, math types, value types, keywords, and class registry.
    - context: `ClassContext` data model, SFINAE template expansion, and signature tracking.
    - doc: CommonMark-to-Javadoc translation, HTML table rendering, and tag formatters.
    - errors: Custom typed exception hierarchy (`JavaGenError`, `TypeResolutionError`, etc.).
    - java_emitter: Dedicated `JavaEmitter` for `.java` wrapper generation.
    - jni_emitter: Dedicated `JniEmitter` for zero-cost JNI `.cpp` bridge generation.
    - orchestrator: Multi-file IR pipeline execution (`process_file`) and CLI driver (`main`).
    - type_resolver: `TypeResolver` engine for type mapping, struct leaves, and buffer classes.
    - utils: Pure helper routines for AST attributes, identifier sanitization, and naming.
"""

# Step 1: Configuration tables and registries
from .config import (
    JAVA_OBJECT_FINAL_NOARG_METHODS,
    KNOWN_CLASSES,
    MATH_TYPES,
    TYPE_MAP,
    TYPE_PATTERNS,
    VALUE_TYPES,
    register_known_classes,
)

# Step 2: Class contextual data model
from .context import ClassContext

# Step 3: Documentation and Markdown translation utilities
from .doc import format_see_tag, generate_javadoc, markdown_to_javadoc

# Step 4: Custom exception hierarchy
from .errors import (
    EmissionError,
    IRParseError,
    JavaGenError,
    TemplateExpansionError,
    TypeResolutionError,
)

# Step 5: Java wrapper emitter
from .java_emitter import JavaEmitter

# Step 6: JNI C++ bridge emitter
from .jni_emitter import JniEmitter

# Step 7: Orchestration and CLI driver
from .orchestrator import main, process_file

# Step 8: Type resolution and memory layout engine
from .type_resolver import TypeResolver

# Step 9: AST attribute helpers and identifier sanitization
from .utils import get_effective_method_name, sanitize_identifier
from .validator import BindingValidator, ValidationResult, ValidationDiagnostic

# Step 10: Public API export declaration
__all__ = [
    "ClassContext",
    "JavaEmitter",
    "JniEmitter",
    "TypeResolver",
    "BindingValidator",
    "ValidationResult",
    "ValidationDiagnostic",
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
