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

"""Class representation context and method expansion model for JavaGen.

This module provides :class:`ClassContext`, the central intermediary representation (IR)
coordinator in the Filament Java and JNI binding generation pipeline. It acts as the
primary data structure representing a parsed C++ class or struct AST extracted by libclang
or the JSON IR generator.

Architectural Position and Data Flow:
-------------------------------------
1. **Input**: Deserialized JSON AST node representing a C++ class/struct produced by the
   AST extraction phase, along with source header location and optional builder / nested
   struct metadata.
2. **Context Construction & Normalization**:
   - Attribute filtering: Discards methods, fields, and constants tagged with skip markers
     (e.g., `filament:apigen:skip`, `no_apigen`, `binding:skip`).
   - Archetype classification: Categorizes the class into one of six core binding archetypes:
     `builder`, `handle`, `inline_buffer`, `bitfield`, `aggregate`, or `utility`.
   - Typedef and enum indexing: Normalizes C++ namespaces, cross-references nested enums,
     and indexes typedef aliases.
   - Tagged array enum synthesis: Identifies generic buffer setter templates and dynamically
     synthesizes nested element enums (`FloatElement`, `IntElement`, `BoolElement`).
   - Field caching analysis: Pairs symmetrical setters and getters for Filament handles and
     structs to generate Java-side object caching fields (`mConfiguration`), eliminating
     redundant JNI bridge transitions and object allocations.
   - Child context recursion: Recursively instantiates :class:`ClassContext` instances for
     nested inner classes, such as nested Builders and nested structs.
   - Native signature pre-scan: Pre-evaluates native method overload signatures to detect
     name collisions and determine when JNI overload signature mangling (e.g. `nSetBuffer__J...`)
     is strictly required.
3. **Method Expansion**:
   - Expands compressed SFINAE trait specializations into concrete, distinct method variants.
   - Synthesizes overloaded pairs for packed NIO direct buffers and primitive Java arrays.
   - Appends unmangled type suffixes to generic methods returning concrete scalar types.
4. **Code Generation Delegation**:
   - Delegates idiomatic Java wrapper emission to :class:`~javagen.java_emitter.JavaEmitter`.
   - Delegates high-performance C++ JNI bridge emission to :class:`~javagen.jni_emitter.JniEmitter`.
   - Coordinates type translation and struct flattening with :class:`~javagen.type_resolver.TypeResolver`.
"""

import collections
import copy
import re
from typing import Any, Dict, List, Optional, Set, Tuple, Union

from .config import (
    KNOWN_CLASSES,
    TAGGED_SCALAR_FAMILIES,
    VALUE_TYPES,
    register_known_classes,
)
from .type_resolver import TypeResolver
from .utils import (
    classify_scalar_family,
    get_element_entry_name,
    get_size_param_attr,
    get_tagged_array_info,
    is_reserved_or_padding_field,
    sanitize_identifier,
)


class ClassContext:
    """Represents a C++ class and manages its Java and JNI code generation state.

    This class serves as the core semantic wrapper around the raw JSON IR of a C++
    class or struct. It normalizes member definitions, resolves C++ types into their
    target Java and JNI counterparts, expands template specializations, manages
    nested builder and struct hierarchies, and delegates final code emission.

    Attributes:
        cpp_name: Fully qualified or unqualified C++ name of the class (e.g. `Engine`).
        name: Name of the class used in Java/JNI emission, accounting for overrides.
        doc: Structured Doxygen documentation dictionary extracted from C++ comments.
        parent_context: Enclosing :class:`ClassContext` if this is a nested class/struct,
            or `None` if this is a top-level class.
        builder_ir: Raw IR dictionary of the nested Builder class, if one exists.
        nested_structs_map: Global dictionary mapping enclosing class names to lists of
            nested struct IR nodes.
        methods: Filtered list of non-skipped member method IR dictionaries.
        fields: Filtered list of non-skipped member field IR dictionaries.
        constants: Filtered list of non-skipped member constant IR dictionaries.
        enums: Filtered list of non-skipped member enum IR dictionaries, including any
            dynamically synthesized tagged array element enums.
        ir: The raw JSON IR dictionary describing this class.
        source_header: Relative or absolute path to the C++ header file declaring this class.
        diagnostic_mode: Boolean flag enabling verbose debug traces during code emission.
        current_method: Name of the method currently being emitted (used for error context).
        emitted_signatures: Set of (method_name, param_types_tuple) tracking emitted Java
            methods to enforce signature deduplication.
        emitted_native_signatures: Set of native method signature strings to prevent duplicate
            private native declarations.
        emitted_jni_funcs: Set of C++ JNI export function names emitted into the native file.
        is_used_by_native: Boolean indicating if the class carries the `used_by_native`
            attribute, requiring specialized JNI bridge access.
        type_resolver: Bound instance of :class:`TypeResolver` used for all type queries.
        raw_aliases: List of raw typedef/using alias IR dictionaries from the C++ class.
        aliases: Dictionary mapping alias names and qualified names to their underlying C++ types.
        package: Target Java package name (defaults to `com.google.android.filament`).
        jni_package: Escaped package name suitable for JNI function names (e.g. `com_google_android_filament`).
        bases: List of base class names, filtered of internal API tags and backend classes.
        base_class: Primary base class name if single inheritance applies, else `None`.
        enum_map: Dictionary mapping unqualified, qualified, and stripped enum names to their
            corresponding enum IR dictionaries.
        archetype: Architectural classification string: `'builder'`, `'handle'`,
            `'inline_buffer'`, `'bitfield'`, `'aggregate'`, or `'utility'`.
        value_type_info: Configuration dictionary from :data:`VALUE_TYPES` if this class
            is an inline buffer or bitfield value class, else `None`.
        is_aggregate: Boolean indicating whether this class is an aggregate C-style struct.
        is_value_class: Boolean indicating whether this class is an inline buffer value type.
        is_bitfield_class: Boolean indicating whether this class wraps a bitfield/bitmask.
        is_utility_class: Boolean indicating whether this class contains only static utility functions.
        is_builder_class: Boolean indicating whether this class implements the Builder pattern.
        cached_fields: Dictionary of property names mapped to metadata for Java instance field caching.
        builder: Child :class:`ClassContext` for the nested Builder class, if present.
        nested_struct_contexts: List of child :class:`ClassContext` instances for nested structs.
        native_counts: Dictionary mapping JNI native function base names to the total count of
            overloaded signatures, used to decide whether explicit overload mangling is needed.
    """

    def __init__(
        self,
        ir_class: Dict[str, Any],
        source_header: str,
        name_override: Optional[str] = None,
        builder_ir: Optional[Dict[str, Any]] = None,
        parent_context: Optional["ClassContext"] = None,
        nested_structs_map: Optional[Dict[str, List[Dict[str, Any]]]] = None,
    ) -> None:
        """Initialize a ClassContext instance from raw JSON IR.

        Performs comprehensive normalization of the class definition, filters out
        members annotated with skip directives, establishes type resolution bindings,
        indexes enums and aliases, classifies the architectural archetype, detects
        cacheable field pairs, recursively initializes nested class contexts, and
        pre-computes native overload counts for JNI signature mangling.

        Args:
            ir_class: Dictionary representing the class AST in JSON IR.
            source_header: Path to the C++ header file declaring this class.
            name_override: Optional name override for Java/JNI output (used for Builders).
            builder_ir: Optional raw IR dictionary of the nested Builder class.
            parent_context: Enclosing :class:`ClassContext` if this is an inner/nested class.
            nested_structs_map: Mapping of parent class names to nested struct IR dictionaries.
        """
        # Step 1: Initialize Identity, Metadata, and Skip Attribute Filtering
        # -------------------------------------------------------------------
        # Store fundamental class naming and doc metadata. Extract both the original C++ name
        # and the Java emission name (which may be overridden, e.g. for nested "Builder" classes).
        self.cpp_name: str = ir_class["name"]
        self.name: str = name_override if name_override else ir_class["name"]
        self.doc: Dict[str, Any] = ir_class.get("doc", {})
        self.parent_context: Optional[ClassContext] = parent_context
        self.builder_ir: Optional[Dict[str, Any]] = builder_ir
        self.nested_structs_map: Dict[str, List[Dict[str, Any]]] = nested_structs_map or {}

        # Filter out methods, fields, constants, and enums tagged with any apigen skip attributes.
        # This prevents internal engine details, iterator accessors, or non-public APIs from
        # polluting the generated Java surface.
        skip_attrs = (
            "filament:apigen:skip", "filament:skip", "apigen:skip",
            "no_apigen", "noapigen", "skip_generation", "binding:skip"
        )
        self.methods: List[Dict[str, Any]] = [
            m for m in ir_class.get("methods", [])
            if not any(
                attr in skip_attrs or attr.startswith("no_apigen") or attr.startswith("skip_generation")
                for attr in m.get("attributes", [])
            )
        ]
        self.fields: List[Dict[str, Any]] = [
            f for f in ir_class.get("fields", [])
            if not any(
                attr in skip_attrs or attr.startswith("no_apigen") or attr.startswith("skip_generation")
                for attr in f.get("attributes", [])
            ) and not is_reserved_or_padding_field(f.get("name", ""))
        ]
        self.constants: List[Dict[str, Any]] = [
            c for c in ir_class.get("constants", [])
            if not any(
                attr in skip_attrs or attr.startswith("no_apigen") or attr.startswith("skip_generation")
                for attr in c.get("attributes", [])
            )
        ]
        self.enums: List[Dict[str, Any]] = [
            e for e in ir_class.get("enums", [])
            if not any(
                attr in skip_attrs or attr.startswith("no_apigen") or attr.startswith("skip_generation")
                for attr in e.get("attributes", [])
            )
        ]
        self.ir: Dict[str, Any] = ir_class
        self.source_header: str = source_header
        self.diagnostic_mode: bool = False
        self.current_method: Optional[str] = None
        self.emitted_signatures: Set[Tuple[str, Tuple[str, ...]]] = set()
        self.emitted_native_signatures: Set[str] = set()
        self.emitted_jni_funcs: Set[str] = set()
        self.is_used_by_native: bool = any(
            attr in (
                "filament:apigen:used_by_native", "filament:used_by_native",
                "apigen:used_by_native", "used_by_native"
            )
            for attr in ir_class.get("attributes", [])
        )

        # Step 2: Initialize Bound TypeResolver and Alias Tables
        # ------------------------------------------------------
        # The TypeResolver encapsulates all type translation logic (C++ -> Java/JNI types,
        # Marshalling expressions, Struct unpacking, Invocables, etc.).
        self.type_resolver: TypeResolver = TypeResolver(self)

        # Parse and resolve class-level typedef and using aliases (e.g. `using Type = Backend::ElementType`).
        self.raw_aliases: List[Dict[str, Any]] = ir_class.get("aliases", [])
        self.aliases: Dict[str, str] = {}
        for alias in self.raw_aliases:
            alias_name = alias["name"]
            alias_target = alias["type"].get("qualified_name") or alias["type"]["cpp_name"]
            self.aliases[alias_name] = alias_target
            if "qualified_name" in alias:
                self.aliases[alias["qualified_name"]] = alias_target

        # Step 3: Determine Package Names and Base Class Inheritance
        # ----------------------------------------------------------
        # If this is a nested class, inherit the Java package configuration from its parent.
        if parent_context:
            self.package: str = parent_context.package
            self.jni_package: str = parent_context.jni_package
        else:
            self.package = "com.google.android.filament"
            self.jni_package = self.package.replace(".", "_")

        # Strip internal Filament C++ base classes (like FilamentAPI, BuilderBase) that
        # do not map to Java class inheritance hierarchies.
        self.bases: List[str] = [
            b.split("::")[-1] for b in ir_class.get("bases", [])
            if b not in (
                "FilamentAPI", "BuilderBase", "BuilderDetails",
                "utils::EnableBitmaskOperators"
            )
            and not b.startswith("backend::")
            and b.split("::")[-1] != self.name
        ]
        self.base_class: Optional[str] = self.bases[0] if self.bases else None

        # Step 4: Index Inner and Enclosing Enums
        # ---------------------------------------
        # Index all nested enums under multiple naming conventions (unqualified name, fully qualified
        # name, stripped namespace name, and class-prefixed name) so they can be looked up reliably
        # regardless of how C++ method signatures reference them.
        self.enum_map: Dict[str, Dict[str, Any]] = {}
        for enum_ir in self.enums:
            self.enum_map[enum_ir["name"]] = enum_ir
            self.enum_map[enum_ir["qualified_name"]] = enum_ir
            clean_qname = self.type_resolver.strip_namespaces(enum_ir["qualified_name"])
            self.enum_map[clean_qname] = enum_ir
            self.enum_map[f"{self.name}::{enum_ir['name']}"] = enum_ir
            self.enum_map[f"{self.cpp_name}::{enum_ir['name']}"] = enum_ir

        # Cross-reference typedef aliases targeting enums into the enum map.
        for alias in ir_class.get("aliases", []):
            alias_name = alias["name"]
            alias_type = alias.get("type", {})
            underlying_qname = alias_type.get("qualified_name")
            underlying_cpp = alias_type.get("cpp_name")
            if alias_name in self.enum_map:
                enum_ir = self.enum_map[alias_name]
                if underlying_qname:
                    self.enum_map[underlying_qname] = enum_ir
                    self.enum_map[self.type_resolver.strip_namespaces(underlying_qname)] = enum_ir
                if underlying_cpp:
                    self.enum_map[underlying_cpp] = enum_ir
                    self.enum_map[self.type_resolver.strip_namespaces(underlying_cpp)] = enum_ir

        # Step 5: Synthesize Tagged Scalar Array Enums
        # --------------------------------------------
        # For methods with tagged array arguments (e.g. `setParameter(..., T*, size_t)` where T
        # represents varying scalar/vector sizes), collect all concrete specialization types
        # and group them by scalar family (`float`, `int`, `bool`).
        # Then, dynamically synthesize nested enum definitions (`FloatElement`, `IntElement`, `BoolElement`)
        # so Java callers can specify the vector width / component type safely at runtime.
        needed_family_entries: Dict[str, List[str]] = collections.defaultdict(list)
        for method in self.methods:
            tagged_info = get_tagged_array_info(method)
            if not tagged_info:
                continue
            specs = method.get("specializations", [])
            tp_name = tagged_info["tagged_arg"].get("type", {}).get("template_param_name", "T")
            for spec in specs:
                concrete_type = spec.get(tp_name)
                if not concrete_type:
                    continue
                family = classify_scalar_family(concrete_type)
                if not family:
                    continue
                entry = get_element_entry_name(concrete_type)
                if entry not in needed_family_entries[family]:
                    needed_family_entries[family].append(entry)

        # Standard deterministic order for synthesized enum entries
        entry_order = {
            "FLOAT": 0, "FLOAT2": 1, "FLOAT3": 2, "FLOAT4": 3, "MAT3": 4, "MAT4": 5,
            "INT": 0, "INT2": 1, "INT3": 2, "INT4": 3,
            "BOOL": 0, "BOOL2": 1, "BOOL3": 2, "BOOL4": 3,
        }
        for family in ("float", "int", "bool"):
            if family in needed_family_entries:
                entries = list(needed_family_entries[family])
                entries.sort(key=lambda x: entry_order.get(x, 99))
                family_cfg = TAGGED_SCALAR_FAMILIES[family]
                enum_name = family_cfg["enum_name"]
                if enum_name not in self.enum_map and not any(e["name"] == enum_name for e in self.enums):
                    enum_ir = {
                        "name": enum_name,
                        "qualified_name": f"{self.cpp_name}::{enum_name}",
                        "attributes": [],
                        "underlying_type": "int",
                        "entries": [
                            {
                                "name": entry,
                                "value": idx,
                                "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}
                            }
                            for idx, entry in enumerate(entries)
                        ],
                        "doc": {
                            "brief": f"Element types for {family_cfg['family'].lower()} tagged array buffers.",
                            "details": "",
                            "params": {},
                            "returns": "",
                            "meta": {}
                        }
                    }
                    self.enums.append(enum_ir)
                    self.enum_map[enum_name] = enum_ir
                    self.enum_map[enum_ir["qualified_name"]] = enum_ir
                    self.enum_map[f"{self.name}::{enum_name}"] = enum_ir
                    self.enum_map[f"{self.cpp_name}::{enum_name}"] = enum_ir

        # Register this class in the global KNOWN_CLASSES registry so other classes can reference it.
        register_known_classes([ir_class])
        vt = VALUE_TYPES.get(self.name) or VALUE_TYPES.get(self.cpp_name)
        is_builder = (
            ir_class.get("archetype") == "builder"
            or ir_class.get("category") == "builder"
            or ir_class.get("is_builder")
            or (self.name == "Builder" and self.parent_context is not None)
        )

        # Step 6: Classify Class Archetype
        # --------------------------------
        # Determine the architectural archetype of the class to govern its Java class
        # scaffolding, constructors, instance field storage, and JNI conversion:
        # - 'builder': Nested or standalone Builder pattern implementing fluid setters.
        # - 'handle': Opaque native pointer wrapper (e.g. Engine, Material, Scene).
        # - 'inline_buffer': Direct NIO buffer-backed value class (e.g. Mat4, Box, Aabb).
        # - 'bitfield': 64-bit primitive long-backed bitmask (e.g. Sampler).
        # - 'aggregate': Plain-Old-Data (POD) struct with public member fields.
        # - 'utility': Static-only function namespace (no constructors, private instance).
        if is_builder:
            self.archetype = "builder"
            self.value_type_info = None
        elif vt:
            self.archetype = vt.get("archetype", "handle")
            self.value_type_info = vt
        elif (
            ir_class.get("archetype") == "bitfield"
            or ir_class.get("category") == "bitfield"
            or any(
                a in ("filament:apigen:bitfield", "filament:bitfield", "apigen:bitfield", "bitfield")
                for a in ir_class.get("attributes", [])
            )
        ):
            self.archetype = "bitfield"
            primitive = ir_class.get("bitfield_primitive", "int")
            jni_prim = f"j{primitive}"
            self.bitfield_primitive = primitive
            self.bitfield_jni_type = jni_prim
            self.value_type_info = {
                "java": self.name,
                "jni": jni_prim,
                "jni_type": jni_prim,
                "archetype": "bitfield",
                "storage_type": primitive,
                "field_name": "mSampler" if "Sampler" in self.name else f"m{self.name}",
                "cpp_type": self.cpp_name,
                "header": ir_class.get("location", {}).get("file", ""),
                "to_cpp": f"filament::JniUtils::from_{primitive}({{value}})",
                "from_cpp": f"filament::JniUtils::to_{primitive}({{value}})",
            }
        elif (
            ir_class.get("is_aggregate", False)
            or (len(self.fields) > 0 and ir_class.get("type", {}).get("category") == "struct")
        ) and not self.base_class:
            self.archetype = "aggregate"
            self.value_type_info = None
        elif (
            ir_class.get("is_utility", False)
            or ir_class.get("is_namespace", False)
            or ir_class.get("category") == "utility"
            or (len(self.methods) > 0 and all(m.get("is_static", False) for m in self.methods) and len(self.fields) == 0)
        ):
            self.archetype = "utility"
            self.value_type_info = None
        else:
            self.archetype = "handle"
            self.value_type_info = None

        # Convenience boolean flags representing archetype checks
        self.is_aggregate: bool = (self.archetype == "aggregate")
        self.is_value_class: bool = (self.archetype == "inline_buffer")
        self.is_bitfield_class: bool = (self.archetype == "bitfield")
        self.is_utility_class: bool = (self.archetype == "utility")
        self.is_builder_class: bool = (self.archetype == "builder")

        # Step 7: Recursively Initialize Nested Builder and Nested Struct Contexts
        # ------------------------------------------------------------------------
        # If this class defines a nested Builder class, instantiate a child ClassContext for it
        # and inherit the parent's enums and aliases.
        if self.builder_ir:
            self.builder: Optional[ClassContext] = ClassContext(
                self.builder_ir,
                source_header,
                name_override="Builder",
                parent_context=self
            )
            self.builder.enum_map.update(self.enum_map)
            self.builder.aliases.update(self.aliases)
        else:
            self.builder = None

        # Instantiate child ClassContext instances for all registered nested structs.
        self.nested_struct_contexts: List[ClassContext] = []
        for child_ir in self.nested_structs_map.get(self.name, []):
            child_ctx = ClassContext(
                child_ir,
                source_header,
                parent_context=self,
                nested_structs_map=self.nested_structs_map
            )
            child_ctx.enum_map.update(self.enum_map)
            child_ctx.aliases.update(self.aliases)
            self.nested_struct_contexts.append(child_ctx)

        # Step 8: Detect Cached Getter/Setter Field Pairs and Retained References
        # -----------------------------------------------------------------------
        # Identify matching setter/getter pairs that benefit from Java-side instance caching
        # to avoid unnecessary native JNI transitions and duplicate object allocations.
        self.cached_fields: Dict[str, Any] = self._detect_cached_fields()
        self.retained_references: Dict[str, Dict[str, Any]] = self._detect_retained_references()

        # Step 9: Pre-Scan Native Signature Occurrences to Compute Overload Counts
        # -------------------------------------------------------------------------
        # The JNI specification requires overloaded native methods in the same class to include
        # a signature suffix (e.g. `Java_..._method__JFI`) to disambiguate them at linking time.
        # We perform a dry-run expansion of all methods to count how many distinct signatures
        # share each native method base name (`nMethod`). If count > 1, mangling is activated.
        native_sigs: Dict[str, Set[Any]] = collections.defaultdict(set)
        for method in self.methods:
            if method["name"] in [c["getter"] for c in self.cached_fields.values()]:
                continue
            if self.is_value_class and method["name"] == self.value_type_info.get("buffer_getter"):  # type: ignore
                continue
            if method["name"] in self.retained_references:
                continue
            generated_methods = self._expand_method(method)
            for m in generated_methods:
                if self.archetype == "builder":
                    if m["ir"].get("is_constructor"):
                        native_sigs["nCreateBuilder"].add(())
                    else:
                        sig = self._generate_native_decl(m, dry_run=True, return_sig=True)
                        if sig:
                            native_sigs[sig[0]].add(sig[1])
                else:
                    sig = self._generate_native_decl(m, dry_run=True, return_sig=True)
                    if sig:
                        native_sigs[sig[0]].add(sig[1])

        if self.archetype == "builder" and not any(m.get("is_constructor") for m in self.methods):
            native_sigs["nCreateBuilder"].add(())

        self.native_counts: Dict[str, int] = {n_name: len(sigs) for n_name, sigs in native_sigs.items()}
        if self.builder:
            self.native_counts.update(self.builder.native_counts)
            self.builder.native_counts = self.native_counts

    @property
    def has_retained_buffers(self) -> bool:
        """Whether this builder retains native buffer memory across invocations.

        If a builder method accepts a direct NIO buffer that is retained by the native
        builder across configuration calls (e.g. vertex/index buffers during mesh construction),
        the Java wrapper must maintain a hard reference to prevent premature garbage collection.

        Returns:
            True if any builder method accepts a retained buffer argument, False otherwise.
        """
        if self.archetype == "builder":
            return any(
                any(self.type_resolver.is_builder_buffer_arg(a) for a in m.get("arguments", []))
                for m in self.methods
            )
        if self.builder:
            return self.builder.has_retained_buffers
        return False

    # -------------------------------------------------------------------------
    # Delegated Type Resolution APIs
    # -------------------------------------------------------------------------

    def resolve_type_info(self, type_input: Any, silent: bool = False) -> Tuple[Dict[str, Any], Tuple[Any, ...]]:
        """Resolve a raw IR type dictionary or type string into rich binding metadata.

        Args:
            type_input: Type AST dictionary or C++ type string.
            silent: If True, suppress warning logs when an unsupported type is encountered.

        Returns:
            Tuple of (type_info_dict, dimension_tuple).
        """
        return self.type_resolver.resolve_type_info(type_input, silent=silent)

    def is_aggregate_struct(self, type_name_or_info: Any) -> Optional[Dict[str, Any]]:
        """Check whether a type name or type info represents a flattened aggregate struct.

        Args:
            type_name_or_info: C++ type name string or resolved type metadata dictionary.

        Returns:
            The struct class IR dictionary if aggregate, else None.
        """
        return self.type_resolver.is_aggregate_struct(type_name_or_info)

    def is_pojo_struct(self, type_name_or_info: Any = None) -> Optional[Dict[str, Any]]:
        """Check whether a type name or type info represents a public POJO struct."""
        return self.type_resolver.is_pojo_struct(type_name_or_info)

    def translate_default_value(self, *args: Any, **kwargs: Any) -> Optional[str]:
        """Translate a C++ default value expression to an idiomatic Java literal."""
        return self.type_resolver.translate_default_value(*args, **kwargs)

    def get_pojo_leaves(self, *args: Any, **kwargs: Any) -> List[Dict[str, Any]]:
        """Extract unrolled leaf descriptors for a POJO struct."""
        return self.type_resolver.get_pojo_leaves(*args, **kwargs)

    def get_pojo_field_annotation(self, *args: Any, **kwargs: Any) -> List[str]:
        """Compute annotations for a POJO struct field."""
        return self.type_resolver.get_pojo_field_annotation(*args, **kwargs)

    def get_flattened_leaves(self, type_info: Any, prefix: str = "") -> List[Dict[str, Any]]:
        """Recursively flatten nested aggregate struct fields into primitive scalar leaves.

        Args:
            type_info: Resolved type metadata dictionary of the struct.
            prefix: Dot-separated field path prefix for nested member access.

        Returns:
            List of flattened field leaf descriptors.
        """
        return self.type_resolver.get_flattened_leaves(type_info, prefix=prefix)

    def get_struct_leaf_accessors(self, struct_cls: Any, prefix: str = "", cpp_prefix: str = "") -> Any:
        """Generate getter/setter accessor expressions for flattened struct fields.

        Args:
            struct_cls: Class IR dictionary or type metadata representing the struct.
            prefix: Java field prefix path.
            cpp_prefix: C++ member access prefix path.

        Returns:
            List of leaf accessor mappings.
        """
        return self.type_resolver.get_struct_leaf_accessors(struct_cls, prefix=prefix, cpp_prefix=cpp_prefix)

    def generate_cpp_struct_construction(self, struct_cls: Any, var_name: str, leaf_prefix: str = "", type_override: Any = None) -> List[str]:
        """Generate C++ statements reconstructing an aggregate struct from JNI primitive parameters.

        Args:
            struct_cls: Struct class IR dictionary.
            var_name: Target C++ local variable name to declare and populate.
            leaf_prefix: Variable name prefix for the unpacked JNI primitive parameters.
            type_override: Optional C++ type name override.

        Returns:
            List of C++ source lines performing the struct initialization.
        """
        return self.type_resolver.generate_cpp_struct_construction(struct_cls, var_name, leaf_prefix=leaf_prefix, type_override=type_override)

    def generate_cpp_struct_cleanup(self, struct_cls: Any, leaf_prefix: str = "") -> List[str]:
        """Generate C++ cleanup statements for temporary allocations created during struct unpacking.

        Args:
            struct_cls: Struct class IR dictionary.
            leaf_prefix: Parameter prefix used during reconstruction.

        Returns:
            List of C++ cleanup statements (e.g. releasing string buffers).
        """
        return self.type_resolver.generate_cpp_struct_cleanup(struct_cls, leaf_prefix=leaf_prefix)

    def get_array_input_info(self, t: Any) -> Optional[Dict[str, Any]]:
        """Check if a type represents a fixed-size vector or array suitable for exploded parameter lists.

        Args:
            t: Type AST dictionary or type name string.

        Returns:
            Dictionary with scalar type and component count if matched, else None.
        """
        return self.type_resolver.get_array_input_info(t)

    def get_java_type(self, type_input: Any) -> str:
        """Convert a C++ type descriptor into its target Java type name.

        Args:
            type_input: Type AST dictionary or C++ type name string.

        Returns:
            Java type string (e.g. `int`, `float[]`, `Engine`, `Buffer`).
        """
        return self.type_resolver.get_java_type(type_input)

    def get_jni_type(self, type_input: Any) -> str:
        """Convert a C++ type descriptor into its target JNI C signature type name.

        Args:
            type_input: Type AST dictionary or C++ type name string.

        Returns:
            JNI type string (e.g. `jint`, `jfloatArray`, `jobject`, `jlong`).
        """
        return self.type_resolver.get_jni_type(type_input)

    def get_annotation(self, type_input: Any) -> Optional[str]:
        """Retrieve any AndroidX nullability or value range annotation for a given type.

        Args:
            type_input: Type AST dictionary or C++ type name string.

        Returns:
            Annotation string (e.g. `@NonNull`, `@Nullable`, `@IntRange(from = 0)`) or None.
        """
        return self.type_resolver.get_annotation(type_input)

    def get_to_cpp_expr(self, type_input: Any, val_name: str) -> str:
        """Produce a C++ expression converting a JNI parameter into its native C++ type.

        Args:
            type_input: Type AST dictionary or type name.
            val_name: Variable name of the JNI parameter.

        Returns:
            C++ expression string (e.g. `reinterpret_cast<Engine*>(val_name)`).
        """
        return self.type_resolver.get_to_cpp_expr(type_input, val_name)

    def get_from_cpp_expr(self, type_input: Any, val_expr: str, jni_ret: str) -> str:
        """Produce a C++ expression converting a native C++ return value into a JNI return type.

        Args:
            type_input: Type AST dictionary or type name.
            val_expr: C++ expression producing the native value.
            jni_ret: Target JNI return type.

        Returns:
            C++ conversion expression string.
        """
        return self.type_resolver.get_from_cpp_expr(type_input, val_expr, jni_ret)

    def _strip_namespaces(self, t: str) -> str:
        """Strip C++ namespaces from a type string (e.g. `filament::Engine` -> `Engine`).

        Args:
            t: Qualified C++ type string.

        Returns:
            Unqualified type name.
        """
        return self.type_resolver.strip_namespaces(t)

    def get_invocable_info(self, t: Any, method_name: str = "") -> Optional[Dict[str, Any]]:
        """Check if a type is a functional callback/invocable and retrieve its dispatch metadata.

        Args:
            t: Type AST dictionary or type name.
            method_name: Name of the enclosing method (for logging context).

        Returns:
            Callback metadata dictionary if invocable, else None.
        """
        return self.type_resolver.get_invocable_info(t, method_name=method_name)

    def _is_unsupported_type(self, type_dict: Any) -> bool:
        """Check if a type cannot be marshalled across the Java/JNI boundary.

        Args:
            type_dict: Type AST dictionary.

        Returns:
            True if type cannot be bound and must cause method suppression.
        """
        return self.type_resolver.is_unsupported_type(type_dict)

    def is_buffer_descriptor_method(self, method: Any) -> bool:
        """Check if a method accepts a Filament BufferDescriptor requiring asynchronous release.

        Args:
            method: Method AST dictionary.

        Returns:
            True if method accepts a BufferDescriptor.
        """
        return self.type_resolver.is_buffer_descriptor_method(method)

    def is_pixel_buffer_descriptor_method(self, method: Any) -> bool:
        """Check if a method accepts a PixelBufferDescriptor.

        Args:
            method: Method AST dictionary.

        Returns:
            True if method accepts a PixelBufferDescriptor.
        """
        return self.type_resolver.is_pixel_buffer_descriptor_method(method)

    def is_async_callback_method(self, method: Any) -> bool:
        """Check if a method accepts an asynchronous completion callback.

        Args:
            method: Method AST dictionary.

        Returns:
            True if method accepts an async callback.
        """
        return self.type_resolver.is_async_callback_method(method)

    def get_struct_float_stride(self, struct_cls: Any) -> Optional[int]:
        """Compute the stride in floats for a struct containing only float fields.

        Args:
            struct_cls: Struct class AST dictionary.

        Returns:
            Number of float components if homogeneous float struct, else None.
        """
        return self.type_resolver.get_struct_float_stride(struct_cls)

    def is_attribute_bitset(self, type_dict: Any) -> bool:
        """Check if a type represents an AttributeBitSet (std::bitset<...>) binding.

        Args:
            type_dict: Type AST dictionary.

        Returns:
            True if type is an AttributeBitSet.
        """
        return self.type_resolver.is_attribute_bitset(type_dict)

    def is_builder_buffer_arg(self, arg: Any) -> bool:
        """Check if a method argument is a retained direct NIO buffer in a builder.

        Args:
            arg: Argument AST dictionary.

        Returns:
            True if argument is a retained builder buffer.
        """
        return self.type_resolver.is_builder_buffer_arg(arg)

    def is_packed_buffer_method(self, method: Any) -> bool:
        """Check if a method qualifies for dual direct NIO Buffer and primitive array overloads.

        Args:
            method: Method AST dictionary.

        Returns:
            True if dual buffer overloads should be emitted.
        """
        return self.type_resolver.is_packed_buffer_method(method)

    def get_packed_buffer_info(self, method: Any) -> Optional[Dict[str, Any]]:
        """Extract metadata for packed buffer method overload generation.

        Args:
            method: Method AST dictionary.

        Returns:
            Packed buffer metadata dictionary if matched, else None.
        """
        return self.type_resolver.get_packed_buffer_info(method)

    def get_builder_packed_buffer_info(self, method: Any) -> Optional[Dict[str, Any]]:
        """Extract packed buffer metadata specifically tailored for Builder methods.

        Args:
            method: Builder method AST dictionary.

        Returns:
            Builder packed buffer metadata dictionary if matched, else None.
        """
        return self.type_resolver.get_builder_packed_buffer_info(method)

    def is_bone_buffer_method(self, method: Any) -> bool:
        """Check if a method accepts bone transformation matrix buffers.

        Args:
            method: Method AST dictionary.

        Returns:
            True if method accepts bone buffers.
        """
        return self.type_resolver.is_bone_buffer_method(method)

    def get_bone_buffer_info(self, method: Any) -> Optional[Dict[str, Any]]:
        """Extract metadata for bone matrix buffer marshalling.

        Args:
            method: Method AST dictionary.

        Returns:
            Bone buffer metadata dictionary if matched, else None.
        """
        return self.type_resolver.get_bone_buffer_info(method)

    def _get_clean_type_name(self, type_dict: Any) -> str:
        """Sanitize a C++ type name by stripping template arguments, qualifiers, and namespaces.

        Args:
            type_dict: Type AST dictionary.

        Returns:
            Sanitized type name string.
        """
        return self.type_resolver.get_clean_type_name(type_dict)

    # -------------------------------------------------------------------------
    # Signature, Default Value & Caching Helpers
    # -------------------------------------------------------------------------

    def _get_method_sig(self, name: str, params_list: List[str]) -> Tuple[str, Tuple[str, ...]]:
        """Extract a deduplication signature from method name and formatted parameters.

        Strips any leading AndroidX annotations (e.g. `@NonNull`, `@Nullable`, `@IntRange(...)`)
        from parameter declarations and isolates their base types so that method overloads with
        identical type signatures can be detected and deduplicated.

        Args:
            name: Java method name.
            params_list: List of parameter declarations (e.g. `["@NonNull Engine engine", "int flags"]`).

        Returns:
            Tuple of (method_name, tuple_of_parameter_types).
        """
        # Step 1: Strip leading annotations and extract base type tokens
        types = []
        for p in params_list:
            cleaned = re.sub(r'@[A-Za-z0-9_]+(?:\([^)]*\))?\s*', '', p).strip()
            parts = cleaned.split()
            if len(parts) >= 2:
                types.append(parts[0])
            elif parts:
                types.append(parts[0])
        return (name, tuple(types))

    def translate_default_value(self, val_str: Optional[str], arg_type: Any) -> Optional[str]:
        """Convert a C++ default argument literal to its Java equivalent.

        Handles boolean literals, null pointer representations, empty curly brace
        initializers `{}` for primitives, class constants, floating-point infinities,
        scoped enum constants, and C++ unsigned literal suffixes (`100u`, `0x10u`).

        Examples:
            - `"true"` -> `"true"`
            - `"nullptr"` -> `"null"`
            - `"{}"` with `float` -> `"0.0f"`
            - `"{}"` with `byte` -> `"(byte) 0"`
            - `"- INFINITY"` with `float` -> `"Float.NEGATIVE_INFINITY"`
            - `"Backend::BUFFER_OBJECT"` -> `"Backend.BUFFER_OBJECT"`
            - `"128u"` -> `"128"`

        Args:
            val_str: Raw C++ default expression string from clang IR.
            arg_type: Type AST dictionary or resolved type metadata of the argument.

        Returns:
            Java-compatible literal string, or None if no default exists.
        """
        if isinstance(val_str, dict):
            return self.type_resolver.translate_default_value(val_str, arg_type)
        # Step 1: Return None immediately if no default value string is present
        if not val_str:
            return None
        val = str(val_str).strip()

        # Step 2: Translate standard boolean literals directly
        if val in ["true", "false"]:
            return val

        # Step 3: Map C++ null pointer representations to Java null
        if val in ["nullptr", "NULL"]:
            return "null"

        type_info, _ = self.resolve_type_info(arg_type, silent=True)
        j_type = type_info.get("java", "")
        is_float = (j_type == "float")

        # Step 4: Handle C++11 uniform zero-initialization syntax `{}`
        if val in ["{}", "{ }"]:
            if j_type in ["int", "short", "byte", "char", "long"]:
                return "(byte) 0" if j_type == "byte" else "0"
            if j_type == "float":
                return "0.0f"
            if j_type == "double":
                return "0.0"
            if j_type == "boolean":
                return "false"
            return "null"

        # Step 5: Check if value matches any static constant declared on this class
        for const in self.constants:
            c_name = const["name"]
            if val == c_name or val.endswith(f"::{c_name}"):
                return c_name

        # Step 6: Map zero literals for byte parameters with explicit casts
        if j_type == "byte" and val in ["0", "0x0"]:
            return "(byte) 0"

        # Step 7: Map IEEE 754 floating-point infinity representations
        if "INFINITY" in val:
            is_neg = val.startswith("-") or "- INFINITY" in val
            if is_float:
                return "Float.NEGATIVE_INFINITY" if is_neg else "Float.POSITIVE_INFINITY"
            return "Double.NEGATIVE_INFINITY" if is_neg else "Double.POSITIVE_INFINITY"

        # Step 8: Map C++ scoped enum constants to Java enum constant references
        if type_info.get("is_enum"):
            parts = [p.strip() for p in val.split("::")]
            entry_name = parts[-1]
            return f"{type_info['java']}.{entry_name}"

        # Step 9: Strip C++ unsigned integer suffixes (e.g. 10u, 0xFFu, 1ULL)
        val = re.sub(r"-\s+", "-", val)
        val = re.sub(r'(?i)\b(0x[0-9a-f]+|\d+)u(?:ll|l)?\b', r'\1', val)
        val = re.sub(r'(?i)(?<=[0-9a-fA-F])u(?:ll|l)?$', '', val)
        return val

    def format_constant_value(
        self,
        val: Optional[str],
        j_type: str,
        constants: Optional[List[Dict[str, Any]]] = None
    ) -> str:
        """Format a class static constant value for Java assignment.

        Cleans up unsigned literal suffixes, maps C++ `std::numeric_limits` expressions,
        ensures single-precision float literals terminate with `'f'`, and handles negative
        literal formatting.

        Args:
            val: Raw C++ constant expression string.
            j_type: Target Java primitive or object type string.
            constants: Optional list of other constant dictionaries for cross-reference.

        Returns:
            Sanitized Java constant literal string.
        """
        # Step 1: Default to "0" if empty value is provided
        if not val:
            return "0"
        val = val.strip()

        # Step 2: Cross-reference other known constants on this class
        if constants:
            for c in constants:
                if val == c["name"]:
                    return val

        # Step 3: Handle negative 1 parenthesized expressions and numeric limits
        if re.search(r"\((?:-\s*1|-1)\)", val):
            return "-1"
        if "std::numeric_limits" in val and "max" in val:
            return "-1"

        # Step 4: Strip C++ unsigned integer suffixes
        val = re.sub(r'(?i)\b(0x[0-9a-f]+|\d+)u(?:ll|l)?\b', r'\1', val)
        val = re.sub(r'(?i)(?<=[0-9a-fA-F])u(?:ll|l)?$', '', val)

        # Step 5: Append 'f' suffix to float literals if missing
        if j_type == "float":
            if not val.endswith("f") and not val.endswith("F"):
                val += "f"
        return val

    def get_jni_mangled_type(self, jni_type: str) -> str:
        """Map a JNI primitive or object type to its signature mangling string.

        Conforms to the Oracle JNI specification for native method signature mangling,
        used when overloaded native methods require distinct exported symbol names.

        Type Mappings:
            - `jboolean`       -> `"Z"`
            - `jbyte`          -> `"B"`
            - `jchar`          -> `"C"`
            - `jshort`         -> `"S"`
            - `jint`           -> `"I"`
            - `jlong`          -> `"J"`
            - `jfloat`         -> `"F"`
            - `jdouble`        -> `"D"`
            - `j<type>Array`   -> `"_3<mangled_elem>"` (e.g. `_3F` for `jfloatArray`)
            - `jstring`        -> `"Ljava_lang_String_2"`
            - `Buffer`         -> `"Ljava_nio_Buffer_2"`
            - `Runnable`       -> `"Ljava_lang_Runnable_2"`
            - Other objects    -> `"Ljava_lang_Object_2"`

        Args:
            jni_type: JNI type name string.

        Returns:
            Mangled JNI signature fragment string.
        """
        mapping = {
            "jboolean": "Z",
            "jbyte": "B",
            "jchar": "C",
            "jshort": "S",
            "jint": "I",
            "jlong": "J",
            "jfloat": "F",
            "jdouble": "D",
            "jbooleanArray": "_3Z",
            "jbyteArray": "_3B",
            "jcharArray": "_3C",
            "jshortArray": "_3S",
            "jintArray": "_3I",
            "jlongArray": "_3J",
            "jfloatArray": "_3F",
            "jdoubleArray": "_3D",
            "jstring": "Ljava_lang_String_2",
            "jobject": "Ljava_lang_Object_2",
            "Buffer": "Ljava_nio_Buffer_2",
            "Runnable": "Ljava_lang_Runnable_2",
        }
        return mapping.get(jni_type, "Ljava_lang_Object_2")

    def _detect_cached_fields(self) -> Dict[str, Dict[str, Any]]:
        """Identify matching setter/getter pairs that benefit from Java-side instance caching.

        Scanning Strategy:
        ------------------
        1. **Setter Scan**: Identify methods matching `set<Property>(<Type> value)` or `configure(<Type>)`
           where `<Type>` is a Filament handle class or an aggregate struct, and return type is `void`.
        2. **Getter Scan**: Identify methods matching `get<Property>()` returning the identical `<Type>`.
        3. **Pair Correlation**: When a setter and getter share a property name and type, record
           a cache entry. In Java, this generates a private instance field `m<Property>`, updates the
           field during setter invocations, and serves the getter directly from the cached field.
        4. **Struct Asymmetry**: For aggregate structs, even if no explicit getter exists in C++,
           Java generates a synthesized getter returning the cached struct instance.

        Returns:
            Dictionary mapping property name strings to their caching metadata dictionaries.
        """
        # Step 1: Scan all methods for eligible setters and getters
        setters: Dict[str, Tuple[str, str, str, bool]] = {}
        getters: Dict[str, Tuple[str, str, bool]] = {}
        for m in self.methods:
            m_name = m["name"]
            args = m.get("arguments", [])
            ret_type = m["return_type"]

            is_set = m_name.startswith("set") and len(m_name) > 3 and m_name[3].isupper()
            is_configure = (m_name == "configure")
            if (is_set or is_configure) and len(args) == 1:
                arg_t = args[0]["type"]
                info, _ = self.resolve_type_info(arg_t, silent=True)
                ret_info, _ = self.resolve_type_info(ret_type, silent=True)
                if (info.get("is_filament_type") or info.get("is_struct")) and ret_info.get("java") == "void":
                    prop = "Configuration" if is_configure else m_name[3:]
                    setters[prop] = (m_name, info["java"], args[0]["name"], info.get("is_struct", False))

            elif m_name.startswith("get") and len(m_name) > 3 and m_name[3].isupper() and len(args) == 0:
                ret_info, _ = self.resolve_type_info(ret_type, silent=True)
                if ret_info.get("is_filament_type") or ret_info.get("is_struct"):
                    prop = m_name[3:]
                    is_ref = ret_type.get("is_reference", False) and not ret_type.get("is_pointer", False)
                    getters[prop] = {
                        "name": m_name,
                        "type": ret_info["java"],
                        "is_struct": ret_info.get("is_struct", False),
                        "is_pojo": bool(ret_info.get("is_pojo_struct")),
                        "is_filament_type": bool(ret_info.get("is_filament_type")),
                        "is_reference": is_ref,
                        "method": m,
                    }

        # Step 2: Correlate matching pairs into cached field descriptors
        cached: Dict[str, Dict[str, Any]] = {}
        for prop, (s_name, s_type, arg_name, s_is_struct) in setters.items():
            if prop in getters:
                g = getters[prop]
                if s_type == g["type"]:
                    cached[prop] = {
                        "prop": prop,
                        "type": s_type,
                        "field_name": f"m{prop}",
                        "setter": s_name,
                        "getter": g["name"],
                        "arg_name": arg_name,
                        "is_struct": s_is_struct,
                    }
            elif s_is_struct:
                cached[prop] = {
                    "prop": prop,
                    "type": s_type,
                    "field_name": f"m{prop}",
                    "setter": s_name,
                    "getter": f"get{prop}",
                    "arg_name": arg_name,
                    "is_struct": True,
                }

        # Step 3: Generic Standalone Zero-Argument POJO Struct Getters
        for prop, g in getters.items():
            if prop not in cached and g.get("is_pojo"):
                cached[prop] = {
                    "prop": prop,
                    "type": g["type"],
                    "field_name": f"m{prop}",
                    "setter": None,
                    "getter": g["name"],
                    "arg_name": prop[0].lower() + prop[1:],
                    "is_struct": True,
                }

        # Step 4: Generic Standalone Zero-Argument Handle Reference Getters (Component Managers)
        for prop, g in getters.items():
            if prop not in cached and g.get("is_filament_type") and g.get("is_reference"):
                cached[prop] = {
                    "prop": prop,
                    "type": g["type"],
                    "field_name": f"m{prop}",
                    "setter": None,
                    "getter": g["name"],
                    "arg_name": prop[0].lower() + prop[1:],
                    "is_handle_reference": True,
                    "is_struct": False,
                    "method": g.get("method"),
                }
        return cached

    def _detect_retained_references(self) -> Dict[str, Dict[str, Any]]:
        """Identify methods annotated with UTILS_APIGEN_RETAINED.

        Methods annotated with [[clang::annotate("filament:apigen:retained")]] represent
        parent or peer object references passed during construction and retained in Java fields.
        These references are retained across the lifetime of the handle and served directly
        from Java instance fields rather than bridging through JNI.

        Returns:
            Dictionary mapping method name (getter) to its retained reference descriptor.
        """
        retained: Dict[str, Dict[str, Any]] = collections.OrderedDict()
        for m in self.methods:
            attrs = m.get("attributes", [])
            is_retained = any(
                a in ("filament:apigen:retained", "apigen:retained")
                for a in attrs
            )
            if not is_retained:
                continue

            name = m["name"]
            prop = name[3:] if name.startswith("get") and len(name) > 3 else name
            field_name = f"m{prop}"
            param_name = prop[0].lower() + prop[1:]

            ret_t = m.get("return_type", {})
            cpp_name = ret_t.get("cpp_name", "") if isinstance(ret_t, dict) else str(ret_t)
            clean_t = re.sub(r'\bconst\b', '', cpp_name).replace("*", "").strip()

            if clean_t == "void":
                java_type = "Object"
            else:
                info, _ = self.resolve_type_info(ret_t, silent=True)
                java_type = info.get("java", "Object")

            is_nullable = (ret_t.get("nullability") == "nullable") if isinstance(ret_t, dict) else False
            nullability_ann = "@Nullable" if is_nullable else "@NonNull"

            retained[name] = {
                "getter": name,
                "prop": prop,
                "field_name": field_name,
                "param_name": param_name,
                "java_type": java_type,
                "is_nullable": is_nullable,
                "nullability_annotation": nullability_ann,
                "return_type": ret_t,
                "method_ir": m,
            }
        return retained

    def _get_template_type_suffix(self, type_name: str) -> str:
        """Produce a capitalized suffix for unmangling template methods returning concrete types.

        When a generic C++ template method returns `T` and is unrolled for multiple concrete types,
        Java cannot distinguish them by return type alone. This method generates a descriptive name
        suffix to produce distinct Java method names.

        Examples:
            - `"float"` -> `"Float"` (e.g. `getConstantFloat`)
            - `"int"`   -> `"Int"`   (e.g. `getConstantInt`)
            - `"bool"`  -> `"Boolean"` (e.g. `getConstantBoolean`)

        Args:
            type_name: Qualified or unqualified C++ type name string.

        Returns:
            Capitalized suffix string for method name unmangling.
        """
        clean = type_name.split("::")[-1].strip()
        if clean in ("bool", "jboolean", "boolean"):
            return "Boolean"
        elif clean in ("float", "jfloat"):
            return "Float"
        elif clean in ("int", "int32_t", "uint32_t", "jint"):
            return "Int"
        elif clean in ("long", "int64_t", "uint64_t", "jlong"):
            return "Long"
        elif clean in ("double", "jdouble"):
            return "Double"
        elif clean in ("short", "int16_t", "uint16_t", "jshort"):
            return "Short"
        elif clean in ("byte", "int8_t", "uint8_t", "jbyte"):
            return "Byte"
        elif clean in ("char", "char_s", "char_u", "jchar"):
            return "Char"
        else:
            return clean[0].upper() + clean[1:]

    def _expand_method(self, method_ir: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Expand compressed SFINAE trait specializations into concrete method variant entries.

        Expansion Strategy & Walkthrough:
        ---------------------------------
        1. **Operator & Constructor Filtering**:
           - C++ operator overloads (`operator=`, `operator[]`, etc.) are discarded.
           - Constructors are filtered out unless the class is an inline buffer or bitfield
             value class, and copy/move constructors are explicitly suppressed.
        2. **Iterator & Entity Pointer Filtering**:
           - Methods returning raw C++ iterators (`begin`, `end`, `_iterator`, `_range`)
             or raw `Entity*` pointer arrays are filtered out to avoid unsafe pointer exposure.
        3. **Tagged Array Grouping**:
           - If a method accepts a tagged generic array (`T* values, size_t count`), group
             concrete template specializations by scalar family (`float`, `int`, `bool`).
             Each family generates a single method accepting an element enum (`FloatElement`)
             and a direct NIO buffer.
        4. **Standard Template Unrolling**:
           - For standard template methods with explicit specialization lists in the IR, deep-copy
             the method IR for each specialization, replace template parameter placeholders with
             concrete types, update pointer/reference qualifiers, and synthesize unique method names
             if the return type was templated.
        5. **Packed Buffer Dual Emission**:
           - For packed buffer setters (e.g. `setBuffer(...)`), expand into two variants:
             one taking a direct NIO `Buffer`, and one taking a primitive Java array (`float[]`, etc.).

        Args:
            method_ir: Raw method AST dictionary from JSON IR.

        Returns:
            List of expanded method context dictionaries ready for code emission.
        """
        name = method_ir["name"]

        # Step 1: Filter out C++ operators and unbindable constructors
        if name.startswith("operator"):
            return []
        if method_ir.get("is_constructor"):
            if self.archetype in ("bitfield", "inline_buffer") or self.base_class:
                args = method_ir.get("arguments", [])
                if len(args) == 1:
                    arg_type_name = args[0]["type"].get("cpp_name", "")
                    if arg_type_name in (
                        self.name, f"const {self.name} &", f"{self.name} &", f"{self.name} &&",
                        f"{self.cpp_name} const &", f"{self.cpp_name} &", f"{self.cpp_name} &&"
                    ):
                        return []
                if any(self._is_unsupported_type(a["type"]) for a in args):
                    return []
                return [{"ir": method_ir, "specialization_type": None}]
            return []

        # Step 2: Suppress redundant buffer getter pointers for inline value classes
        if self.is_value_class and self.value_type_info.get("buffer_getter") == name and method_ir["return_type"].get("is_pointer"):  # type: ignore
            return []

        # Step 3: Suppress C++ iterators, ranges, and raw Entity pointer accessors
        ret_t = method_ir["return_type"].get("qualified_name") or method_ir["return_type"].get("cpp_name", "")
        if any(ret_t.endswith(s) for s in ("_iterator", "_range", "_sentinel", "iterator", "const_iterator")):
            return []
        if name in (
            "begin", "end", "cbegin", "cend", "rbegin", "rend",
            "getChildrenBegin", "getChildrenEnd", "getChildrenRange"
        ):
            return []

        if name == "getEntities" and method_ir["return_type"].get("is_pointer"):
            return []
        if (
            ret_t.startswith("utils::Entity const*")
            or ret_t.startswith("utils::Entity*")
            or ret_t.startswith("Entity const*")
            or ret_t.startswith("Entity*")
        ):
            return []

        # Step 4: Check for explicit apigen skip attributes
        skip_attrs = (
            "filament:apigen:skip", "filament:skip", "apigen:skip",
            "no_apigen", "noapigen", "skip_generation", "binding:skip"
        )
        if any(
            attr in skip_attrs or attr.startswith("no_apigen") or attr.startswith("skip_generation")
            for attr in method_ir.get("attributes", [])
        ):
            return []

        # Step 5: Check for tagged generic array templates and group by scalar family
        tagged_info = get_tagged_array_info(method_ir)
        if tagged_info:
            specializations = method_ir.get("specializations", [])
            tp_name = tagged_info["tagged_arg"].get("type", {}).get("template_param_name", "T")

            family_groups = collections.defaultdict(list)
            for spec in specializations:
                concrete_type = spec.get(tp_name)
                if not concrete_type:
                    continue
                family = classify_scalar_family(concrete_type)
                if family:
                    family_groups[family].append(spec)

            variants = []
            for family in ("float", "int", "bool"):
                if family in family_groups:
                    variants.append({
                        "ir": method_ir,
                        "is_tagged_array": True,
                        "family": family,
                        "tagged_info": tagged_info,
                        "specializations": family_groups[family],
                        "specialization_type": None,
                    })
            return variants

        # Step 6: Unroll standard template specializations into concrete method variants
        specializations = method_ir.get("specializations")
        if specializations:
            variants = []
            seen_sigs = set()
            for spec in specializations:
                spec_ir = copy.deepcopy(method_ir)
                spec_ir["orig_name"] = method_ir.get("orig_name", method_ir["name"])

                # Handle templated return type substitutions
                ret_type = spec_ir.get("return_type", {})
                is_ret_templated = False
                tp_ret = ret_type.get("template_param_name")
                if ret_type.get("is_template_param") or (tp_ret and tp_ret in spec):
                    is_ret_templated = True
                    concrete_ret = spec.get(tp_ret or "T")
                    if concrete_ret:
                        ret_type["cpp_name"] = concrete_ret
                        ret_type["qualified_name"] = concrete_ret
                        ret_type["is_template_param"] = False

                # Handle templated argument substitutions and qualifier reconstruction
                for arg in spec_ir.get("arguments", []):
                    arg_type = arg.get("type", {})
                    tp_arg = arg_type.get("template_param_name")
                    if arg_type.get("is_template_param") or (tp_arg and tp_arg in spec):
                        concrete_arg = spec.get(tp_arg or "T")
                        if concrete_arg:
                            is_ptr = arg_type.get("is_pointer", False)
                            is_const = arg_type.get("is_const", False)
                            is_ref = arg_type.get("is_reference", False)
                            if is_const and not is_ptr:
                                new_cpp_name = f"const {concrete_arg} &" if is_ref else f"const {concrete_arg}"
                            elif is_ptr:
                                new_cpp_name = f"const {concrete_arg} *" if is_const else f"{concrete_arg} *"
                            elif is_ref:
                                new_cpp_name = f"{concrete_arg} &"
                            else:
                                new_cpp_name = concrete_arg
                            arg_type["cpp_name"] = new_cpp_name
                            arg_type["qualified_name"] = concrete_arg if not is_ptr else f"{concrete_arg} *"
                            arg_type["is_template_param"] = False

                # Format specialization type string and synthesize unmangled return name if needed
                spec_type_str = ", ".join(spec.values())
                template_unmangled = False
                if is_ret_templated:
                    type_suffix = self._get_template_type_suffix(spec_type_str)
                    spec_ir["name"] = f"{method_ir['name']}{type_suffix}"
                    template_unmangled = True

                # Discard variants containing unsupported argument or return types
                if self._is_unsupported_type(spec_ir["return_type"]):
                    continue
                if any(self._is_unsupported_type(a["type"]) for a in spec_ir.get("arguments", [])):
                    continue

                # Deduplicate identical unrolled Java method signatures
                java_ret = self.get_java_type(spec_ir["return_type"])
                unrolled_args = []
                for a in spec_ir.get("arguments", []):
                    arr_in = self.get_array_input_info(a["type"])
                    req_type_str = (
                        a["type"].get("qualified_name") or a["type"]["cpp_name"]
                        if isinstance(a["type"], dict) else a["type"]
                    )
                    is_matrix = bool(re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str)
                    is_pointer = a["type"].get("is_pointer", False) if isinstance(a["type"], dict) else False
                    size_param = bool(get_size_param_attr(a))
                    if arr_in and not is_matrix and not is_pointer and not size_param and not arr_in.get("is_slice"):
                        unrolled_args.extend([arr_in["scalar"]] * arr_in["size"])
                    else:
                        unrolled_args.append(self.get_java_type(a["type"]))
                java_args = tuple(unrolled_args)
                sig_key = (spec_ir["name"], java_args, java_ret)
                if sig_key in seen_sigs:
                    continue
                seen_sigs.add(sig_key)

                variants.append({
                    "ir": spec_ir,
                    "specialization_type": spec_type_str,
                    "template_unmangled": template_unmangled
                })
            return variants

        # Step 7: Filter non-templated methods for unsupported types or special patterns
        if self._is_unsupported_type(method_ir["return_type"]):
            return []
        if self.is_async_callback_method(method_ir):
            return [{"ir": method_ir, "specialization_type": None, "is_async_callback": True}]
        if any(self._is_unsupported_type(a["type"]) for a in method_ir.get("arguments", [])):
            return []

        # Step 8: Expand packed buffer methods into dual NIO Buffer and primitive array variants
        if self.is_packed_buffer_method(method_ir):
            return [
                {"ir": method_ir, "specialization_type": None, "buffer_mode": "buffer"},
                {"ir": method_ir, "specialization_type": None, "buffer_mode": "array"},
            ]

        # Step 9: Standard single method variant
        return [{"ir": method_ir, "specialization_type": None}]

    def _has_annotations(self) -> bool:
        """Check if any member field or method signature requires AndroidX annotations.

        Inspects all fields, method return types, and parameter types for nullability,
        integer range, or size annotations. Used by the Java emitter to determine if
        `androidx.annotation.*` imports must be included in the header.

        Returns:
            True if any annotation is required, False otherwise.
        """
        for f in self.fields:
            if self.get_annotation(f["type"]):
                return True
        for m in self.methods:
            if self.get_annotation(m["return_type"]):
                return True
            for arg in m.get("arguments", []):
                if self.get_annotation(arg["type"]):
                    return True
        return False

    # -------------------------------------------------------------------------
    # Code Generation Entry Points (Delegated to Dedicated Emitters)
    # -------------------------------------------------------------------------

    def generate_java(self) -> str:
        """Generate idiomatic Java class wrapper code for this class context.

        Delegates code emission to :class:`~javagen.java_emitter.JavaEmitter`.

        Returns:
            Complete Java source file content as a string.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self).generate()

    def generate_jni(self) -> str:
        """Generate zero-cost JNI C++ bridge code for this class context.

        Delegates bridge emission to :class:`~javagen.jni_emitter.JniEmitter`.

        Returns:
            Complete JNI C++ source file content as a string.
        """
        from .jni_emitter import JniEmitter
        return JniEmitter(self).generate()

    def generate_builder_class_java(self) -> str:
        """Emit Java code for the nested Builder inner class.

        Delegates inner Builder emission to :class:`~javagen.java_emitter.JavaEmitter`.

        Returns:
            Java source code block defining the inner static Builder class.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self).generate_builder_class_java()

    def generate_builder_native_decls(self) -> List[str]:
        """Emit JNI native method declarations for the nested Builder class.

        Delegates declaration emission to :class:`~javagen.java_emitter.JavaEmitter`.

        Returns:
            List of private static native method declaration strings in Java.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self).generate_builder_native_decls()

    def generate_builder_jni_methods(self) -> List[str]:
        """Emit JNI C++ bridge implementations for the nested Builder class.

        Delegates JNI function emission to :class:`~javagen.jni_emitter.JniEmitter`.

        Returns:
            List of C++ JNI function implementation blocks as strings.
        """
        from .jni_emitter import JniEmitter
        return JniEmitter(self).generate_builder_jni_methods()

    def _generate_native_decl(
        self,
        m_ctx: Dict[str, Any],
        dry_run: bool = False,
        return_sig: bool = False
    ) -> Any:
        """Forward native method declaration emission to JavaEmitter.

        Args:
            m_ctx: Expanded method context dictionary.
            dry_run: If True, do not record emitted signatures in class context state.
            return_sig: If True, return a tuple of `(native_name, jni_param_types_tuple)`
                instead of the formatted Java declaration string.

        Returns:
            Formatted Java declaration string, or signature tuple if return_sig is True.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self)._generate_native_decl(m_ctx, dry_run=dry_run, return_sig=return_sig)

    def _generate_vector_array_overloads(self, *args: Any, **kwargs: Any) -> List[str]:
        """Forward vector array overload generation to JavaEmitter.

        Args:
            *args: Positional arguments forwarded to :meth:`JavaEmitter._generate_vector_array_overloads`.
            **kwargs: Keyword arguments forwarded to :meth:`JavaEmitter._generate_vector_array_overloads`.

        Returns:
            List of Java method overload code blocks.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self)._generate_vector_array_overloads(*args, **kwargs)

    def _generate_convenience_overloads(self, *args: Any, **kwargs: Any) -> List[str]:
        """Forward convenience overload generation to JavaEmitter.

        Args:
            *args: Positional arguments forwarded to :meth:`JavaEmitter._generate_convenience_overloads`.
            **kwargs: Keyword arguments forwarded to :meth:`JavaEmitter._generate_convenience_overloads`.

        Returns:
            List of Java method convenience overload code blocks.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self)._generate_convenience_overloads(*args, **kwargs)

    def _generate_aggregate_java(self, *args: Any, **kwargs: Any) -> str:
        """Forward aggregate struct Java code emission to JavaEmitter.

        Args:
            *args: Positional arguments forwarded to :meth:`JavaEmitter._generate_aggregate_java`.
            **kwargs: Keyword arguments forwarded to :meth:`JavaEmitter._generate_aggregate_java`.

        Returns:
            Complete Java class definition string for an aggregate struct.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self)._generate_aggregate_java(*args, **kwargs)

    def _generate_pojo_struct(self, *args: Any, **kwargs: Any) -> str:
        """Forward POJO struct Java code emission to JavaEmitter.

        Args:
            *args: Positional arguments forwarded to :meth:`JavaEmitter._generate_pojo_struct`.
            **kwargs: Keyword arguments forwarded to :meth:`JavaEmitter._generate_pojo_struct`.

        Returns:
            Complete Java class definition string for a POJO struct.
        """
        from .java_emitter import JavaEmitter
        return JavaEmitter(self)._generate_pojo_struct(*args, **kwargs)

