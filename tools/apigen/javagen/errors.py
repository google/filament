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

"""Custom exception hierarchy for the JavaGen binding generation pipeline.

Architectural Purpose:
    This module defines the strongly-typed exception hierarchy used across all
    phases of the JavaGen pipeline: IR deserialization, class context modeling,
    type resolution, template unrolling, and code emission.

Design Principles:
    1. Distinction of Failure Domains:
       By establishing distinct exception subclasses for each pipeline stage,
       callers and orchestrators can distinguish between malformed IR inputs,
       missing or unmapped C++ types, invalid SFINAE template specializations,
       and disk/emission failures.
    2. Rich Diagnostic Context:
       Every exception preserves domain-specific identifiers (e.g., file paths,
       unresolved type names, method signatures, or target language names) as
       first-class attributes rather than burying them in formatted strings.
    3. Hierarchy Rooting:
       All custom exceptions inherit from `JavaGenError`, allowing top-level CLI
       handlers and unit tests to catch all generator-specific errors uniformly.
"""


class JavaGenError(Exception):
    """Base class for all exceptions raised by the JavaGen pipeline.

    Architectural Role:
        Acts as the root ancestor for all domain-specific errors in JavaGen.
        Orchestration layers catch `JavaGenError` to report clean diagnostic
        messages without leaking Python tracebacks to CLI users unless
        diagnostic or verbose modes are explicitly requested.

    State & Attributes:
        message (str): Human-readable diagnostic description of the failure.
    """

    def __init__(self, message: str) -> None:
        """Initialize the base JavaGen error.

        Args:
            message: Descriptive explanation of the error condition.
        """
        # Step 1: Forward the formatted explanation to standard Exception
        super().__init__(message)
        self.message = message


class TypeResolutionError(JavaGenError):
    """Raised when a C++ type cannot be resolved to valid Java and JNI types.

    Architectural Role:
        Raised by `TypeResolver.resolve_type_info` when encountering a C++
        type that has no corresponding entry in `TYPE_MAP`, does not match
        known patterns in `TYPE_PATTERNS`, cannot be mapped to an aggregate
        struct via `KNOWN_CLASSES`, and is not an opaque Filament engine handle.

    Triggering Scenarios:
        - A new Filament C++ header introduces an unmapped third-party type.
        - An aggregate struct parameter lacks sibling JSON IR definition.
        - A complex template type (e.g. `std::vector<std::pair<...>>`) cannot
          be safely marshaled across the JNI boundary without manual shims.

    State & Attributes:
        type_name (str): The unresolved C++ type string or signature.
        details (str): Additional diagnostic context or hint for remediation.
    """

    def __init__(self, type_name: str, details: str = "") -> None:
        """Initialize the type resolution error.

        Args:
            type_name: The unresolved C++ type string or representation.
            details: Optional additional diagnostic context or hint.
        """
        # Step 1: Construct a descriptive message identifying the unresolved type
        msg = f"Failed to resolve C++ type '{type_name}'"
        if details:
            msg += f": {details}"

        # Step 2: Initialize base error and record structured attributes
        super().__init__(msg)
        self.type_name = type_name
        self.details = details


class TemplateExpansionError(JavaGenError):
    """Raised when template specialization or trait unrolling fails.

    Architectural Role:
        Raised by `ClassContext._expand_method` or `JavaEmitter` when an IR
        method declares template `specializations` or `UTILS_APIGEN_TAGGED_ARRAY`
        attributes, but the concrete type substitutions cannot be evaluated,
        contain invalid parameter keys, or produce duplicate conflicting Java
        method overloads that cannot be disambiguated.

    Triggering Scenarios:
        - A template parameter placeholder (e.g. `T`) is referenced in an argument
          or return type but is absent from the method's specialization dictionary.
        - Unrolling tagged scalar arrays fails due to an unrecognized scalar family.

    State & Attributes:
        method_name (str): Name of the template method being expanded.
        details (str): Detailed explanation of why substitution failed.
    """

    def __init__(self, method_name: str, details: str) -> None:
        """Initialize the template expansion error.

        Args:
            method_name: Name of the template method being expanded.
            details: Description of why the expansion or substitution failed.
        """
        # Step 1: Format message with method identifier and failure explanation
        msg = f"Template expansion failed for method '{method_name}': {details}"

        # Step 2: Initialize base error and record method context
        super().__init__(msg)
        self.method_name = method_name
        self.details = details


class EmissionError(JavaGenError):
    """Raised when Java wrapper or JNI C++ code emission fails.

    Architectural Role:
        Raised during string generation or disk writes in `JavaEmitter` or
        `JniEmitter` when encountering contradictory AST state, such as an
        aggregate struct missing required leaf accessors, invalid JNI signature
        mangling inputs, or write failures to the destination directories.

    Triggering Scenarios:
        - An aggregate struct declares a recursive self-referential member field.
        - Method signature mangling encounters an illegal character sequence.
        - Target output directories (`--java-dir`, `--jni-dir`) are unwritable.

    State & Attributes:
        entity_name (str): Name of the class, struct, or method being emitted.
        target_lang (str): Target language identifier ('Java' or 'JNI C++').
        details (str): Context describing the structural emission failure.
    """

    def __init__(self, entity_name: str, target_lang: str, details: str) -> None:
        """Initialize the code emission error.

        Args:
            entity_name: Name of the class or method whose emission failed.
            target_lang: Target language ('Java' or 'JNI C++').
            details: Context describing the specific emission failure.
        """
        # Step 1: Format language-specific failure description
        msg = f"{target_lang} code emission failed for '{entity_name}': {details}"

        # Step 2: Initialize base error and record emission target state
        super().__init__(msg)
        self.entity_name = entity_name
        self.target_lang = target_lang
        self.details = details


class IRParseError(JavaGenError):
    """Raised when the input JSON Intermediate Representation is malformed.

    Architectural Role:
        Raised by `orchestrator.process_file` when loading or validating an
        input JSON file generated by `extractor.py`. Signals that the file is
        missing on disk, contains invalid JSON syntax, or is missing required
        top-level sections (`classes`, `meta`, or `referenced_classes`).

    Triggering Scenarios:
        - Extractor output was truncated or corrupted due to build interruption.
        - File path passed via `--ir` does not exist or has invalid permissions.

    State & Attributes:
        file_path (str): Filesystem path to the invalid JSON IR file.
        details (str): Specific explanation of the structural failure.
    """

    def __init__(self, file_path: str, details: str) -> None:
        """Initialize the IR parse error.

        Args:
            file_path: Path to the JSON IR file that failed to parse.
            details: Specific explanation of the syntax or schema failure.
        """
        # Step 1: Format message identifying the offending file path
        msg = f"Malformed JSON IR in '{file_path}': {details}"

        # Step 2: Initialize base error and store the file path
        super().__init__(msg)
        self.file_path = file_path
        self.details = details
