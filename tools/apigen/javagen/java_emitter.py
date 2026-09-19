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

"""Java source code emission engine for Filament API bindings.

This module defines :class:`JavaEmitter`, which synthesizes clean, idiomatic, high-performance
Java wrapper classes (`.java`) from intermediate representation (IR) metadata extracted from
C++ headers.

Architectural Position and Code Generation Model:
-------------------------------------------------
1. **Zero-Overhead Memory and Lifecycle Model**:
   - Handle Classes: Wrap native C++ object pointers in a 64-bit integer (`long mNativeObject`).
     Native pointers are validated before JNI invocation and zeroed out upon destruction to
     prevent use-after-free and dangling pointer crashes.
   - Value / Inline Buffer Classes: Direct memory structures (such as Frustum planes or
     math matrices) store primitive backing arrays directly on the Java heap (`float[] mPlanes`),
     bypassing native heap allocation and JNI crossing overhead.
   - Bitfield Classes: Encode bitwise option flags into primitive integer storage (`long mSampler`),
     providing fluent builder-style mutation while guaranteeing zero heap allocation.
   - Aggregate POD Structs: Synthesized with exploded primitive storage fields, dual exploded and
     array-based constructors, and zero-allocation out-parameter getters (`get(out)`).
   - Utility Classes: Emitted with private constructors and purely static native methods.

2. **Enum Optimization and EnumCache**:
   - Standard Java `.values()` calls allocate a defensive array copy on every invocation.
   - To eliminate GC pressure in performance-critical graphics loops, :class:`JavaEmitter`
     synthesizes a private static inner class `EnumCache` caching `s<Enum>Values = <Enum>.values()`
     for all enums returned by native C++ calls.
   - Custom enums with non-sequential or explicit integer values emit custom `toFilamentNative()`
     and `from(int value)` switch lookup tables.

3. **Inner Builder Class Architecture**:
   - Entities requiring multi-parameter configuration (e.g. `VertexBuffer`, `Texture`, `Camera`)
     feature an inner static `Builder` class.
   - Native Resource Finalization: Each `Builder` owns a `BuilderFinalizer` ensuring native builder
     structures allocated via `nCreateBuilder` are freed via `nDestroyBuilder` if garbage-collected
     prior to invoking `build()`.
   - Memory Retention Invariant: For buffers that must remain pinned until `build()` finishes
     uploading data to GPU memory, the builder maintains an internal `mRetainedBuffers` list
     preventing early GC reclamation.

4. **Public API Overload Synthesis**:
   - Default Parameters: Synthesizes telescopic Java convenience overloads cascading trailing
     default arguments down to the fully-qualified native delegate.
   - Math Vectors: Unrolls fixed C++ math types (e.g. `math::float3`) into primitive components
     (`float x, float y, float z`) alongside convenient array overloads (`float[]`) annotated
     with AndroidX `@Size(min = 3)`.
   - Streaming Buffer Descriptors: Generates multi-tier direct buffer overloads supporting raw
     `java.nio.Buffer`, destination offsets, element counts, and asynchronous completion handlers.

5. **Type Safety & Android Support Annotations**:
   - Automatically scans classes, methods, parameters, and return types to discover required
     imports and attach `@NonNull`, `@Nullable`, `@IntRange`, `@Size`, `@Entity`, and `@LinearColor`.
"""

import copy
import re
import sys
from typing import Any, Dict, List, Optional, Set, Tuple, Union

from .config import (
    GENERATED_FILE_WARNING,
    JAVA_OBJECT_FINAL_NOARG_METHODS,
    KNOWN_CLASSES,
    LICENSE_HEADER,
    TAGGED_SCALAR_FAMILIES,
    VALUE_TYPES,
)
from .doc import generate_javadoc, markdown_to_javadoc, adapt_param_doc_for_java
from .utils import (
    classify_scalar_family,
    format_enum_entry_name,
    get_effective_method_name,
    get_element_entry_name,
    get_scoped_cpp_name,
    get_size_param_attr,
    get_tagged_array_info,
    indent_lines,
    is_custom_enum,
    is_reserved_or_padding_field,
    is_tagged_array_method,
    sanitize_identifier,
    to_camel_case,
)


class JavaEmitter:
    """Emits idiomatic Java class wrapper files (.java) for a given ClassContext.

    Coordinates class declaration generation, package and imports scanning, inner enum
    and EnumCache generation, nested Builder class generation, public method and convenience
    overload synthesis, and private static native JNI declarations.

    Attributes:
        ctx: The parent :class:`~javagen.context.ClassContext` representing the active class.
    """

    def __init__(self, context: Any) -> None:
        """Initialize JavaEmitter bound to an active ClassContext.

        Args:
            context: The parent ClassContext representing the active class.
        """
        self.ctx = context

    def __getattr__(self, name: str) -> Any:
        """Forward unrecognized attribute lookups to the bound ClassContext."""
        return getattr(self.ctx, name)

    # -------------------------------------------------------------------------
    # Context Forwarding Properties
    # -------------------------------------------------------------------------

    @property
    def name(self) -> str:
        """Simple Java name of the active class."""
        return self.ctx.name

    @property
    def cpp_name(self) -> str:
        """Fully-qualified or unqualified C++ identifier of the active class."""
        return self.ctx.cpp_name

    @property
    def methods(self) -> List[Dict[str, Any]]:
        """Filtered list of non-skipped member method IR dictionaries."""
        return self.ctx.methods

    @property
    def fields(self) -> List[Dict[str, Any]]:
        """Filtered list of non-skipped member field IR dictionaries."""
        return self.ctx.fields

    @property
    def constants(self) -> List[Dict[str, Any]]:
        """Filtered list of non-skipped member constant IR dictionaries."""
        return self.ctx.constants

    @property
    def enums(self) -> List[Dict[str, Any]]:
        """Filtered list of non-skipped member enum IR dictionaries."""
        return self.ctx.enums

    @property
    def archetype(self) -> str:
        """Architectural archetype classification string."""
        return self.ctx.archetype

    @property
    def is_aggregate(self) -> bool:
        """Whether this class is an aggregate POD struct."""
        return self.ctx.is_aggregate

    @property
    def is_value_class(self) -> bool:
        """Whether this class is an inline buffer value class."""
        return self.ctx.is_value_class

    @property
    def is_bitfield_class(self) -> bool:
        """Whether this class is a bitfield class."""
        return self.ctx.is_bitfield_class

    @property
    def is_utility_class(self) -> bool:
        """Whether this class is a utility class."""
        return self.ctx.is_utility_class

    @property
    def is_builder_class(self) -> bool:
        """Whether this context represents an inner builder class."""
        return self.ctx.is_builder_class

    @property
    def value_type_info(self) -> Any:
        """Metadata dictionary for value/bitfield archetypes."""
        return self.ctx.value_type_info

    @property
    def parent_context(self) -> Any:
        """Reference to parent ClassContext if this is an inner builder or struct."""
        return self.ctx.parent_context

    @property
    def builder(self) -> Any:
        """Child ClassContext representing the inner Builder class, if present."""
        return self.ctx.builder

    @property
    def package(self) -> str:
        """Target Java package name for this class."""
        return self.ctx.package

    @property
    def jni_package(self) -> str:
        """Escaped package name suitable for JNI C function naming."""
        return self.ctx.jni_package

    @property
    def base_class(self) -> Optional[str]:
        """Name of Java base class to extend, or None."""
        return self.ctx.base_class

    @property
    def cached_fields(self) -> Dict[str, Any]:
        """Dictionary of cached field descriptors lazily initialized by getters."""
        return self.ctx.cached_fields

    @property
    def emitted_signatures(self) -> Set[Any]:
        """Set of Java method signatures emitted to avoid duplicate method collisions."""
        return self.ctx.emitted_signatures

    @property
    def emitted_native_signatures(self) -> Set[Any]:
        """Set of native method signatures emitted to avoid duplicate native collisions."""
        return self.ctx.emitted_native_signatures

    @property
    def native_counts(self) -> Dict[str, int]:
        """Histogram tracking frequency of native method base names for JNI mangling."""
        return self.ctx.native_counts

    @property
    def is_used_by_native(self) -> bool:
        """Whether the class requires @UsedByNative annotation for ProGuard retention."""
        return self.ctx.is_used_by_native

    @property
    def nested_struct_contexts(self) -> List[Any]:
        """List of child ClassContext instances for nested structs."""
        return self.ctx.nested_struct_contexts

    @property
    def source_header(self) -> str:
        """Path to C++ source header declaring this class."""
        return self.ctx.source_header

    @property
    def doc(self) -> Dict[str, Any]:
        """Doxygen documentation dictionary."""
        return self.ctx.doc

    @property
    def ir(self) -> Dict[str, Any]:
        """Raw JSON IR dictionary."""
        return self.ctx.ir

    @property
    def enum_map(self) -> Dict[str, Any]:
        """Dictionary of registered enums in this class context."""
        return self.ctx.enum_map

    @property
    def aliases(self) -> Dict[str, Any]:
        """Class-level type alias mapping."""
        return self.ctx.aliases

    # -------------------------------------------------------------------------
    # Context Forwarding Methods
    # -------------------------------------------------------------------------

    def resolve_type_info(self, *a: Any, **kw: Any) -> Any:
        """Forward type resolution to parent context."""
        return self.ctx.resolve_type_info(*a, **kw)

    def is_aggregate_struct(self, *a: Any, **kw: Any) -> Any:
        """Forward aggregate struct check to parent context."""
        return self.ctx.is_aggregate_struct(*a, **kw)

    def get_flattened_leaves(self, *a: Any, **kw: Any) -> Any:
        """Forward aggregate struct leaf flattening to parent context."""
        return self.ctx.get_flattened_leaves(*a, **kw)

    def get_struct_leaf_accessors(self, *a: Any, **kw: Any) -> Any:
        """Forward struct leaf accessor retrieval to parent context."""
        return self.ctx.get_struct_leaf_accessors(*a, **kw)

    def get_array_input_info(self, *a: Any, **kw: Any) -> Any:
        """Forward array input info check to parent context."""
        return self.ctx.get_array_input_info(*a, **kw)

    def get_java_type(self, *a: Any, **kw: Any) -> str:
        """Forward Java type lookup to parent context."""
        return self.ctx.get_java_type(*a, **kw)

    def get_jni_type(self, *a: Any, **kw: Any) -> str:
        """Forward JNI type lookup to parent context."""
        return self.ctx.get_jni_type(*a, **kw)

    def get_annotation(self, *a: Any, **kw: Any) -> Optional[str]:
        """Forward annotation lookup to parent context."""
        return self.ctx.get_annotation(*a, **kw)

    def get_to_cpp_expr(self, *a: Any, **kw: Any) -> str:
        """Forward to-C++ expression synthesis to parent context."""
        return self.ctx.get_to_cpp_expr(*a, **kw)

    def get_from_cpp_expr(self, *a: Any, **kw: Any) -> str:
        """Forward from-C++ expression synthesis to parent context."""
        return self.ctx.get_from_cpp_expr(*a, **kw)

    def _strip_namespaces(self, *a: Any, **kw: Any) -> str:
        """Forward namespace stripping to parent context."""
        return self.ctx._strip_namespaces(*a, **kw)

    def get_invocable_info(self, *a: Any, **kw: Any) -> Any:
        """Forward invocable metadata resolution to parent context."""
        return self.ctx.get_invocable_info(*a, **kw)

    def _is_unsupported_type(self, *a: Any, **kw: Any) -> bool:
        """Forward unsupported type check to parent context."""
        return self.ctx._is_unsupported_type(*a, **kw)

    def is_buffer_descriptor_method(self, *a: Any, **kw: Any) -> bool:
        """Forward BufferDescriptor method check to parent context."""
        return self.ctx.is_buffer_descriptor_method(*a, **kw)

    def is_pixel_buffer_descriptor_method(self, *a: Any, **kw: Any) -> bool:
        """Forward PixelBufferDescriptor method check to parent context."""
        return self.ctx.is_pixel_buffer_descriptor_method(*a, **kw)

    def is_async_callback_method(self, *a: Any, **kw: Any) -> bool:
        """Forward async callback method check to parent context."""
        return self.ctx.is_async_callback_method(*a, **kw)

    def get_struct_float_stride(self, *a: Any, **kw: Any) -> Optional[int]:
        """Forward struct float stride computation to parent context."""
        return self.ctx.get_struct_float_stride(*a, **kw)

    def is_attribute_bitset(self, *a: Any, **kw: Any) -> bool:
        """Forward AttributeBitset check to parent context."""
        return self.ctx.is_attribute_bitset(*a, **kw)

    def is_pojo_struct(self, *a: Any, **kw: Any) -> Any:
        """Forward POJO struct check to parent context."""
        return self.ctx.is_pojo_struct(*a, **kw)

    def get_pojo_leaves(self, *a: Any, **kw: Any) -> Any:
        """Forward POJO leaf retrieval to parent context."""
        return self.ctx.get_pojo_leaves(*a, **kw)

    def get_pojo_field_annotation(self, *a: Any, **kw: Any) -> Any:
        """Forward POJO field annotation retrieval to parent context."""
        return self.ctx.get_pojo_field_annotation(*a, **kw)

    def translate_default_value(self, *a: Any, **kw: Any) -> Any:
        """Forward default value translation to parent context."""
        return self.ctx.translate_default_value(*a, **kw)

    def retains_parent_handle(self, target_class_name: str) -> bool:
        """Check if target_class_name has a retained reference of type self.name.

        Inspects KNOWN_CLASSES for methods annotated with [[clang::annotate("filament:apigen:retained")]]
        (such as Renderer.getEngine() returning Engine*, or MaterialInstance.getMaterial()
        returning Material*). If the return type matches self.name, the target class
        retains this parent instance.
        """
        cls = KNOWN_CLASSES.get(target_class_name)
        if not cls:
            return False
        for m in cls.get("methods", []):
            attrs = m.get("attributes", [])
            if any(a in ("filament:apigen:retained", "apigen:retained") for a in attrs):
                ret = m.get("return_type", {})
                cpp_name = ret.get("cpp_name", "") if isinstance(ret, dict) else str(ret)
                clean = (
                    cpp_name
                    .replace("filament::", "")
                    .replace("utils::", "")
                    .replace("const", "")
                    .replace("*", "")
                    .replace("&", "")
                    .strip()
                )
                if clean == self.name:
                    return True
        return False


    def is_builder_buffer_arg(self, *a: Any, **kw: Any) -> bool:
        """Forward builder buffer argument check to parent context."""
        return self.ctx.is_builder_buffer_arg(*a, **kw)

    def is_packed_buffer_method(self, *a: Any, **kw: Any) -> bool:
        """Forward packed buffer method check to parent context."""
        return self.ctx.is_packed_buffer_method(*a, **kw)

    def get_packed_buffer_info(self, *a: Any, **kw: Any) -> Any:
        """Forward packed buffer info retrieval to parent context."""
        return self.ctx.get_packed_buffer_info(*a, **kw)

    def get_builder_packed_buffer_info(self, *a: Any, **kw: Any) -> Any:
        """Forward builder packed buffer info retrieval to parent context."""
        return self.ctx.get_builder_packed_buffer_info(*a, **kw)

    def is_bone_buffer_method(self, *a: Any, **kw: Any) -> bool:
        """Forward bone buffer method check to parent context."""
        return self.ctx.is_bone_buffer_method(*a, **kw)

    def get_bone_buffer_info(self, *a: Any, **kw: Any) -> Any:
        """Forward bone buffer info retrieval to parent context."""
        return self.ctx.get_bone_buffer_info(*a, **kw)

    def _adapt_packed_doc(self, doc_dict: Dict[str, Any], buf_name: str, cnt_repr: str, has_offset_arg: bool = True) -> Dict[str, Any]:
        """Adapt method brief/details documentation for packed buffer overloads.

        Replaces C++ Slice range syntax '[offset, offset + buf.size())' and 'buf.size()'
        with concrete Java range expressions corresponding to the overload signature.
        """
        res = copy.deepcopy(doc_dict)
        for key in ("brief", "details"):
            val = res.get(key)
            if not val:
                continue
            if has_offset_arg:
                val = re.sub(rf'\[offset,\s*offset\s*\+\s*{re.escape(buf_name)}\.size\(\)\)', f'[offset, offset + {cnt_repr})', val)
            else:
                val = re.sub(rf'\[offset,\s*offset\s*\+\s*{re.escape(buf_name)}\.size\(\)\)', f'[0, {cnt_repr})', val)
            val = re.sub(rf'\b{re.escape(buf_name)}\.size\(\)', cnt_repr, val)
            res[key] = val
        return res

    def _get_clean_type_name(self, *a: Any, **kw: Any) -> str:
        """Forward clean type name computation to parent context."""
        return self.ctx._get_clean_type_name(*a, **kw)

    def _get_method_sig(self, *a: Any, **kw: Any) -> Tuple[str, Tuple[str, ...]]:
        """Forward method signature tuple generation to parent context."""
        return self.ctx._get_method_sig(*a, **kw)

    def translate_default_value(self, *a: Any, **kw: Any) -> Optional[str]:
        """Forward default value translation to parent context."""
        return self.ctx.translate_default_value(*a, **kw)

    def format_constant_value(self, *a: Any, **kw: Any) -> str:
        """Forward constant value formatting to parent context."""
        return self.ctx.format_constant_value(*a, **kw)

    def get_jni_mangled_type(self, *a: Any, **kw: Any) -> str:
        """Forward JNI mangled type lookup to parent context."""
        return self.ctx.get_jni_mangled_type(*a, **kw)

    def _has_annotations(self) -> bool:
        """Forward annotation presence check to parent context."""
        return self.ctx._has_annotations()

    def _expand_method(self, *a: Any, **kw: Any) -> List[Dict[str, Any]]:
        """Forward method expansion and unrolling to parent context."""
        return self.ctx._expand_method(*a, **kw)

    def generate(self) -> str:
        """Generate complete, idiomatic Java source code (.java) for this class context.

        Executes the 16-step Java wrapper emission pipeline:
        1. **Archetype Routing**: Dispatches aggregate POD structs to `_generate_aggregate_java`.
        2. **Headers & Package**: Emits Android Apache 2.0 license, APIGen generated warning,
           and package declaration.
        3. **Import Discovery**: Dynamically scans all method signatures, return types, arguments,
           and struct hierarchies to discover required NIO buffers, exceptions, and AndroidX
           annotations (@NonNull, @Nullable, @IntRange, @Size, ProGuard @UsedByNative, and
           custom @interface LinearColor).
        4. **Class Declaration**: Translates Doxygen documentation to Javadoc and declares class
           with inheritance hierarchy.
        5. **EnumCache Pre-caching**: Analyzes return types to synthesize static `EnumCache`
           pre-caching `.values()` arrays to avoid defensive array copy allocations on native returns.
        6. **Polymorphic & Bitset Bridges**: Synthesizes custom polymorphic subclasses (e.g. ToneMapper
           subtypes) and VertexAttribute bitset set conversion utilities.
        7. **Handle & Storage Fields**: Emits native pointer handles (`long mNativeObject`),
           owning engine references (`mEngine`), cached fields, or direct backing arrays (`mPlanes`).
        8. **Inner Enums & Bitmask Flags**: Emits inner enums with native ordinal conversion tables,
           or static final bitmask constants.
        9. **Functional Interfaces**: Declares `@FunctionalInterface` interfaces for Invocable callbacks.
        10. **Constants & Fields**: Formats and declares public static final constants and public fields.
        11. **Constructors**: Synthesizes package-private or public native handle constructors and
            utility class private constructors.
        12. **Inner Builder Class**: Delegates to `builder.generate_builder_class_java()`.
        13. **Type Aliases**: Emits backward-compatible deprecated inner classes (e.g. PixelBufferDescriptor).
        14. **Public Methods**: Iterates over methods and expands overloads via `_generate_java_method`.
        15. **Native Pointer Lifecycle**: Emits `getNativeObject()` validation and `clearNativeObject()`.
        16. **Native Declarations**: Emits collected private static native JNI method declarations.

        Returns:
            Complete Java source file content as a string.
        """
        # Step 1: Check Aggregate POD Struct Archetype
        if self.is_aggregate:
            return self._generate_aggregate_java()

        # Step 2: File License Header & Package Declaration
        out = []
        out.append(LICENSE_HEADER)
        out.append("")
        out.append(GENERATED_FILE_WARNING)
        out.append("")
        out.append(f"package {self.package};")
        out.append("")
        if self.archetype == "handle" or self.builder:
            out.append("import java.lang.IllegalStateException;")
            out.append("")
        
        # Step 3: Scan Imports & Nullability Annotations
        methods_to_scan = []
        for m in self.methods:
            for exp in self._expand_method(m):
                methods_to_scan.append(exp["ir"])
        if self.builder:
            for m in self.builder.methods:
                for exp in self.builder._expand_method(m):
                    methods_to_scan.append(exp["ir"])

        # Check for direct java.nio buffers and descriptors
        has_buffer_descriptor = any(self.is_buffer_descriptor_method(m) for m in self.methods)
        has_pixel_buffer_descriptor = any(self.is_pixel_buffer_descriptor_method(m) for m in self.methods)
        has_packed_buffer = any(self.is_packed_buffer_method(m) for m in self.methods)
        builder_has_retained = self.builder is not None and self.builder.has_retained_buffers
        builder_has_packed = self.builder is not None and any(self.builder.is_packed_buffer_method(m) for m in self.builder.methods)
        has_pbd_alias = any(
            alias.get("name") == "PixelBufferDescriptor"
            or alias.get("type", {}).get("cpp_name", "").endswith("PixelBufferDescriptor")
            or alias.get("type", {}).get("qualified_name", "").endswith("PixelBufferDescriptor")
            for alias in self.raw_aliases
        )
        has_buffer = (
            has_pbd_alias
            or builder_has_retained
            or builder_has_packed
            or any(any(self.resolve_type_info(arg["type"], silent=True)[0].get("java") == "Buffer" for arg in m.get("arguments", [])) for m in methods_to_scan)
        )
        has_byte_buffer = has_pbd_alias or any(any(self.resolve_type_info(arg["type"], silent=True)[0].get("java") == "ByteBuffer" for arg in m.get("arguments", [])) for m in methods_to_scan)
        if has_buffer or has_buffer_descriptor or has_pixel_buffer_descriptor or has_packed_buffer:
            out.append("import java.nio.Buffer;")
            if has_byte_buffer:
                out.append("import java.nio.ByteBuffer;")
            if has_buffer_descriptor or has_pixel_buffer_descriptor or has_packed_buffer or builder_has_packed:
                out.append("import java.nio.BufferOverflowException;")
            if has_pixel_buffer_descriptor:
                out.append("import java.nio.ReadOnlyBufferException;")
            out.append("")
            has_non_null = True
            has_nullability = True
            has_int_range = True
        
        # Check for AttributeBitset Set conversion
        has_attribute_bitset = any(self.is_attribute_bitset(m.get("return_type")) for m in methods_to_scan)
        if has_attribute_bitset or self.name == "View":
            if has_attribute_bitset:
                out.append("import java.util.Collections;")
            out.append("import java.util.EnumSet;")
            if has_attribute_bitset:
                out.append("import java.util.Set;")
            out.append("")
            has_non_null = True

        has_retained_non_null = any(not r["is_nullable"] for r in self.retained_references.values())
        has_retained_nullable = any(r["is_nullable"] for r in self.retained_references.values())
        has_tagged_array = any(is_tagged_array_method(m) for m in self.methods)
        has_int_range = has_packed_buffer or has_tagged_array or builder_has_packed
        has_non_null = self.is_value_class or (self.builder is not None) or has_buffer or has_packed_buffer or has_tagged_array or has_retained_non_null or any(self.resolve_type_info(m["return_type"], silent=True)[0].get("is_container") for m in methods_to_scan)
        has_nullability = self.is_value_class or has_retained_nullable
        has_size = self.is_value_class or has_packed_buffer or has_tagged_array or builder_has_packed
        
        def check_type(t):
            ann = self.get_annotation(t)
            if ann and "@IntRange" in ann: return True
            return False
            
        def check_math(t):
            info, _ = self.resolve_type_info(t, silent=True)
            return "assert" in info or info.get("is_value_object")

        for f in self.fields:
            if check_type(f["type"]): has_int_range = True

        def scan_struct_ctx(s_ctx):
            nonlocal has_size, has_int_range, has_non_null
            for f in s_ctx.fields:
                if s_ctx.get_array_input_info(f["type"]):
                    has_size = True
                    has_non_null = True
                if check_type(f["type"]):
                    has_int_range = True
            for child in s_ctx.nested_struct_contexts:
                scan_struct_ctx(child)

        for n_ctx in self.nested_struct_contexts:
            scan_struct_ctx(n_ctx)
        for m in methods_to_scan:
            if check_type(m["return_type"]): has_int_range = True
            r_info, _ = self.resolve_type_info(m["return_type"], silent=True)
            if r_info.get("is_string") or r_info.get("java") == "String":
                if m["return_type"].get("nullability") == "nullable" if isinstance(m["return_type"], dict) else False:
                    has_nullability = True
                else:
                    has_non_null = True
            if check_math(m["return_type"]):
                has_non_null = True
                has_nullability = True
                if not self.resolve_type_info(m["return_type"], silent=True)[0].get("is_value_object"):
                    has_size = True
            if r_info.get("is_struct") and not self.is_aggregate:
                has_non_null = True
                has_nullability = True
            for arg in m.get("arguments", []):
                req_t = arg["type"]
                if check_type(req_t): has_int_range = True
                
                arr_in = self.get_array_input_info(req_t)
                if arr_in:
                    if arr_in.get("annotation") and "@IntRange" in arr_in["annotation"]:
                        has_int_range = True
                    req_type_str = req_t.get("qualified_name") or req_t["cpp_name"] if isinstance(req_t, dict) else req_t
                    is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                    is_pointer = req_t.get("is_pointer", False)
                    size_param = bool(get_size_param_attr(arg))
                    if is_matrix or is_pointer or size_param or arr_in.get("is_slice"):
                        if not arr_in.get("is_custom_array"):
                            has_size = True
                        if req_t.get("nullability") == "nonnull":
                            has_non_null = True
                        elif req_t.get("nullability") == "nullable":
                            has_nullability = True
                    else:
                        has_size = True
                        has_non_null = True
                else:
                    t_info, _ = self.resolve_type_info(req_t, silent=True)
                    if t_info.get("is_string") or t_info.get("java") == "String":
                        if req_t.get("nullability") == "nullable" if isinstance(req_t, dict) else False:
                            has_nullability = True
                        else:
                            has_non_null = True
                    elif t_info.get("is_filament_type") or t_info.get("is_struct"):
                        if req_t.get("nullability") == "nonnull" or t_info.get("is_struct") or req_t.get("is_reference", False):
                            has_non_null = True
                        elif req_t.get("nullability") == "nullable":
                            has_nullability = True
        
        for m in methods_to_scan:
            if self.is_async_callback_method(m):
                has_nullability = True
            if self.is_buffer_descriptor_method(m) or self.is_pixel_buffer_descriptor_method(m):
                has_non_null = True
                has_nullability = True
                has_int_range = True
            for arg in m.get("arguments", []):
                if not self.is_async_callback_method(m) and self.get_invocable_info(arg["type"], m["name"]):
                    has_non_null = True
                t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                if t_info.get("is_enum"):
                    if arg["type"].get("nullability") == "nullable":
                        has_nullability = True
                    else:
                        has_non_null = True

        has_restrict_to = bool(self.archetype == "handle" and not self.base_class and self.name != "Engine")
        if has_restrict_to:
            has_non_null = True

        if self.name == "Engine":
            has_non_null = True
            has_nullability = True
            has_size = False

        if has_int_range or has_non_null or has_nullability or has_size or has_restrict_to or self.cached_fields:
            if has_int_range:
                out.append("import androidx.annotation.IntRange;")
            if has_non_null:
                out.append("import androidx.annotation.NonNull;")
            if has_nullability or self.cached_fields:
                out.append("import androidx.annotation.Nullable;")
            if has_restrict_to:
                out.append("import androidx.annotation.RestrictTo;")
            if has_size:
                out.append("import androidx.annotation.Size;")
            has_used_by_native_method = any(any("used_by_native" in str(a).lower() for a in m.get("attributes", [])) for m in self.methods)
            if self.is_used_by_native or self.name in ("View", "Engine") or has_used_by_native_method:
                out.append("")
                out.append("import com.google.android.filament.proguard.UsedByNative;")
            out.append("")

        def check_linear_color(t):
            if not t:
                return False
            info, _ = self.resolve_type_info(t, silent=True)
            if info.get("is_linear_color"):
                return True
            ann = self.get_annotation(t)
            return bool(ann and "@LinearColor" in ann)

        has_linear_color = (
            "LinearColor" in self.aliases
            or any(check_linear_color(f["type"]) for f in self.fields)
            or any(check_linear_color(m["return_type"]) for m in methods_to_scan)
            or any(check_linear_color(a["type"]) for m in methods_to_scan for a in m.get("arguments", []))
        )
        if has_linear_color:
            out.append("import java.lang.annotation.Retention;")
            out.append("import java.lang.annotation.Target;")
            out.append("")
            out.append("import static java.lang.annotation.ElementType.FIELD;")
            out.append("import static java.lang.annotation.ElementType.LOCAL_VARIABLE;")
            out.append("import static java.lang.annotation.ElementType.METHOD;")
            out.append("import static java.lang.annotation.ElementType.PARAMETER;")
            out.append("import static java.lang.annotation.RetentionPolicy.SOURCE;")
            out.append("")
        
        # Step 4: Class Javadoc & Class Declaration
        if self.doc:
            javadoc = generate_javadoc(self.doc, indent_spaces=0)
            if javadoc:
                out.append(javadoc)
            
        if self.is_used_by_native:
            out.append("@UsedByNative")
        if self.base_class:
            out.append(f"public class {self.name} extends {self.base_class} {{")
        else:
            out.append(f"public class {self.name} {{")
        if has_linear_color:
            out.append("    @Retention(SOURCE)")
            out.append("    @Target({PARAMETER, METHOD, LOCAL_VARIABLE, FIELD})")
            out.append("    public @interface LinearColor {")
            out.append("    }")
            out.append("")

        def is_custom_enum(enum_ir):
            if not enum_ir:
                return False
            entries = enum_ir.get("entries", [])
            for idx, entry in enumerate(entries):
                val = entry.get("value")
                if val is not None and val != idx:
                    return True
            return False

        # Step 5: EnumCache Pre-caching for Zero-Allocation Returns
        returned_enums = set()
        for method in self.methods:
            for exp_m in self._expand_method(method):
                if exp_m["ir"].get("is_constructor"):
                    continue
                r_type = exp_m["ir"]["return_type"]
                r_info, _ = self.resolve_type_info(r_type, silent=True)
                if r_info.get("is_enum"):
                    enum_name = r_info["java"]
                    if "." in enum_name:
                        owner_class, simple_enum = enum_name.rsplit(".", 1)
                        if owner_class == self.name:
                            enum_name = simple_enum
                        else:
                            returned_enums.add((simple_enum, enum_name))
                            continue
                    enum_ir = next((e for e in self.enums if e["name"] == enum_name), None)
                    if not enum_ir or not is_custom_enum(enum_ir):
                        returned_enums.add((enum_name, enum_name))

        if returned_enums:
            out.append("    static final class EnumCache {")
            for simple_enum, full_name in sorted(returned_enums):
                out.append(f"        static final {full_name}[] s{simple_enum}Values = {full_name}.values();")
            out.append("    }")
            out.append("")

        # Step 6: Synthetic Bitset & Polymorphic Class Bridges
        if has_attribute_bitset:
            out.append("    private static final VertexBuffer.VertexAttribute[] sVertexAttributeValues =")
            out.append("            VertexBuffer.VertexAttribute.values();")
            out.append("")
            out.append("    private static Set<VertexBuffer.VertexAttribute> getAttributes(int bitSet) {")
            out.append("        Set<VertexBuffer.VertexAttribute> set = EnumSet.noneOf(VertexBuffer.VertexAttribute.class);")
            out.append("        VertexBuffer.VertexAttribute[] values = sVertexAttributeValues;")
            out.append("        for (int i = 0, n = values.length; i < n; i++) {")
            out.append("            if ((bitSet & (1 << i)) != 0) {")
            out.append("                set.add(values[i]);")
            out.append("            }")
            out.append("        }")
            out.append("        return Collections.unmodifiableSet(set);")
            out.append("    }")
            out.append("")

        if self.name == "ToneMapper":
            out.append("    public static class Linear extends LinearToneMapper {}")
            out.append("    public static class ACES extends ACESToneMapper {}")
            out.append("    public static class ACESLegacy extends ACESLegacyToneMapper {}")
            out.append("    public static class Filmic extends FilmicToneMapper {}")
            out.append("    public static class PBRNeutral extends PBRNeutralToneMapper {}")
            out.append("    public static class GT7 extends GT7ToneMapper {}")
            out.append("    public static class Agx extends AgxToneMapper {")
            out.append("        public static class AgxLook {")
            out.append("            public static final AgxToneMapper.AgxLook NONE = AgxToneMapper.AgxLook.NONE;")
            out.append("            public static final AgxToneMapper.AgxLook PUNCHY = AgxToneMapper.AgxLook.PUNCHY;")
            out.append("            public static final AgxToneMapper.AgxLook GOLDEN = AgxToneMapper.AgxLook.GOLDEN;")
            out.append("        }")
            out.append("        public Agx() {")
            out.append("            super();")
            out.append("        }")
            out.append("        public Agx(AgxToneMapper.AgxLook look) {")
            out.append("            super(look);")
            out.append("        }")
            out.append("    }")
            out.append("    public static class Generic extends GenericToneMapper {")
            out.append("        public Generic() {")
            out.append("            super();")
            out.append("        }")
            out.append("        public Generic(float contrast, float midGrayIn, float midGrayOut, float hdrMax) {")
            out.append("            super(contrast, midGrayIn, midGrayOut, hdrMax);")
            out.append("        }")
            out.append("    }")
            out.append("")

        # Step 7: Lifecycle & Native Pointer Handle Fields
        if self.archetype == "handle" and not self.base_class:
            for ref in self.retained_references.values():
                ann = "@Nullable " if (ref.get("is_nullable") or self.name == "MaterialInstance") else ""
                out.append(f"    private final {ann}{ref['java_type']} {ref['field_name']};")
            out.append("    private long mNativeObject;")
            if self.name == "Material":
                out.append("    private final MaterialInstance mDefaultInstance;")
            for cached in self.cached_fields.values():
                out.append(f"    private @Nullable {cached['type']} {cached['field_name']};")
            out.append("")
        elif self.archetype == "inline_buffer":
            elem_type = self.value_type_info.get("element_type", "float")
            size = self.value_type_info.get("size", 24)
            field_name = self.value_type_info.get("field_name", "mPlanes")
            out.append(f"    /* package */ final {elem_type}[] {field_name} = new {elem_type}[{size}];")
            out.append("")
        elif self.archetype == "bitfield":
            storage_type = self.value_type_info.get("storage_type", "long")
            field_name = self.value_type_info.get("field_name", "mSampler")
            out.append(f"    /* package */ {storage_type} {field_name};")
            out.append("")
        
        # Step 8: Nested Enums and Bitmask Flags
        for enum_ir in self.enums:
            edoc = generate_javadoc(enum_ir.get("doc", {}), indent_spaces=4)
            if edoc:
                out.append(edoc)
            is_flags = enum_ir.get("is_flags", False) or any(a in ("filament:apigen:flags", "filament:flags", "apigen:flags", "flags", "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask") for a in enum_ir.get("attributes", []))
            if is_flags:
                out.append(f"    public static class {enum_ir['name']} {{")
                out.append(f"        private {enum_ir['name']}() {{}}")
                out.append("")
                for entry in enum_ir.get("entries", []):
                    edoc = generate_javadoc(entry.get("doc", {}), indent_spaces=8)
                    if edoc:
                        out.append(edoc)
                    val = entry.get("value", 0)
                    name = format_enum_entry_name(enum_ir['name'], entry['name'])
                    hex_val = f"0x{val:X}" if val > 0 else "0"
                    out.append(f"        public static final int {name} = {hex_val};")
                out.append("    }")
                out.append("")
                continue
            out.append(f"    public enum {enum_ir['name']} {{")
            custom_enum = is_custom_enum(enum_ir)
            entry_strs = []
            for entry in enum_ir.get("entries", []):
                edoc = generate_javadoc(entry.get("doc", {}), indent_spaces=8)
                name = format_enum_entry_name(enum_ir['name'], entry['name'])
                if custom_enum:
                    val = entry.get("value", 0)
                    entry_line = f"        {name}({val})"
                else:
                    entry_line = f"        {name}"
                if edoc:
                    entry_strs.append(f"{edoc}\n{entry_line}")
                else:
                    entry_strs.append(entry_line)
            out.append(",\n".join(entry_strs) + ";\n")
            if custom_enum:
                out.append("        private final int mValue;\n")
                out.append(f"        {enum_ir['name']}(int value) {{")
                out.append("            mValue = value;")
                out.append("        }\n")
                out.append("        /** @return the value of this enum constant as used by the native Filament engine. */")
                out.append("        public int toFilamentNative() { return mValue; }\n")
                out.append(f"        public static {enum_ir['name']} from(int value) {{")
                out.append("            switch (value) {")
                seen_values = set()
                for entry in enum_ir.get("entries", []):
                    name = format_enum_entry_name(enum_ir['name'], entry['name'])
                    val = entry.get("value", 0)
                    if val in seen_values:
                        continue
                    seen_values.add(val)
                    out.append(f"                case {val}: return {name};")
                out.append(f"                default: throw new IllegalArgumentException(\"Unknown {enum_ir['name']} value: \" + value);")
                out.append("            }")
                out.append("        }")
            else:
                out.append("        /** @return the value of this enum constant as used by the native Filament engine. */")
                out.append("        public int toFilamentNative() { return ordinal(); }")
            out.append("    }")
            out.append("")

        if self.name == "View":
            out.append("    /**")
            out.append("     * List of available tone-mapping operators")
            out.append("     *")
            out.append("     * @deprecated Use ColorGrading instead")
            out.append("     */")
            out.append("    @Deprecated")
            out.append("    public enum ToneMapping {")
            out.append("        /**")
            out.append("         * Equivalent to disabling tone-mapping.")
            out.append("         */")
            out.append("        LINEAR,")
            out.append("")
            out.append("        /**")
            out.append("         * The Academy Color Encoding System (ACES).")
            out.append("         */")
            out.append("        ACES")
            out.append("    }")
            out.append("")
            out.append("    /**")
            out.append("     * Used to select buffers.")
            out.append("     */")
            out.append("    public enum TargetBufferFlags {")
            out.append("        /**")
            out.append("         * Color 0 buffer selected.")
            out.append("         */")
            out.append("        COLOR0(0x1),")
            out.append("        /**")
            out.append("         * Color 1 buffer selected.")
            out.append("         */")
            out.append("        COLOR1(0x2),")
            out.append("        /**")
            out.append("         * Color 2 buffer selected.")
            out.append("         */")
            out.append("        COLOR2(0x4),")
            out.append("        /**")
            out.append("         * Color 3 buffer selected.")
            out.append("         */")
            out.append("        COLOR3(0x8),")
            out.append("        /**")
            out.append("         * Depth buffer selected.")
            out.append("         */")
            out.append("        DEPTH(0x10),")
            out.append("        /**")
            out.append("         * Stencil buffer selected.")
            out.append("         */")
            out.append("        STENCIL(0x20);")
            out.append("")
            out.append("        /*")
            out.append("         * No buffer selected")
            out.append("         */")
            out.append("        public static EnumSet<TargetBufferFlags> NONE = EnumSet.noneOf(TargetBufferFlags.class);")
            out.append("")
            out.append("        /*")
            out.append("         * All color buffers selected")
            out.append("         */")
            out.append("        public static EnumSet<TargetBufferFlags> ALL_COLOR =")
            out.append("                EnumSet.of(COLOR0, COLOR1, COLOR2, COLOR3);")
            out.append("        /**")
            out.append("         * Depth and stencil buffer selected.")
            out.append("         */")
            out.append("        public static EnumSet<TargetBufferFlags> DEPTH_STENCIL = EnumSet.of(DEPTH, STENCIL);")
            out.append("        /**")
            out.append("         * All buffers are selected.")
            out.append("         */")
            out.append("        public static EnumSet<TargetBufferFlags> ALL = EnumSet.range(COLOR0, STENCIL);")
            out.append("")
            out.append("        private int mFlags;")
            out.append("")
            out.append("        TargetBufferFlags(int flags) {")
            out.append("            mFlags = flags;")
            out.append("        }")
            out.append("")
            out.append("        static int flags(EnumSet<TargetBufferFlags> flags) {")
            out.append("            int result = 0;")
            out.append("            for (TargetBufferFlags flag : flags) {")
            out.append("                result |= flag.mFlags;")
            out.append("            }")
            out.append("            return result;")
            out.append("        }")
            out.append("    }")
            out.append("")

        
        # Step 9: Invocable Callback Functional Interfaces
        interfaces = []
        seen_interfaces = set()
        for method in self.methods:
            if not self._expand_method(method) or self.is_async_callback_method(method):
                continue
            m_name = method["name"]
            for arg in method.get("arguments", []):
                inv_info = self.get_invocable_info(arg["type"], m_name)
                if inv_info:
                    if inv_info["interface_name"] in seen_interfaces:
                        continue
                    seen_interfaces.add(inv_info["interface_name"])
                    p_decls = []
                    for p in inv_info["params"]:
                        ann_str = f"{p['annotation']} " if p["annotation"] else ""
                        p_decls.append(f"{ann_str}{p['java_type']} {p['name']}".strip())
                    
                    interfaces.append("    @FunctionalInterface")
                    interfaces.append(f"    public interface {inv_info['interface_name']} {{")
                    interfaces.append(f"        {inv_info['java_ret']} accept({', '.join(p_decls)});\n    }}")
                    
        if interfaces:
            out.append("\n".join(interfaces))
            out.append("")

        # Step 10: Public Constants & Public Fields
        constants = self.constants
        if constants:
            for const in constants:
                const_name = const["name"]
                const_doc = generate_javadoc(const.get("doc"), indent_spaces=4)
                type_info, _ = self.resolve_type_info(const["type"])
                j_type = type_info.get("java", "int")
                raw_val = const.get("value", "")
                
                val_str = self.format_constant_value(raw_val, j_type, constants)
                if const_doc:
                    out.append(const_doc)
                out.append(f"    public static final {j_type} {const_name} = {val_str};")
            out.append("")

        for field in self.fields:
            fname = field["name"]
            ftype = field["type"]
            jtype = self.get_java_type(ftype)
            ann = self.get_annotation(ftype)
            
            if ann:
                out.append(f"    {ann}")
            out.append(f"    public {jtype} {fname};")
        
        if self.fields:
            out.append("")
        
        # Step 11: Class Constructors & Static Factory wrap(...)
        if self.archetype == "utility":
            out.append(f"    private {self.name}() {{")
            out.append("    }")
            out.append("")
        elif self.name == "Engine":
            out.append(self._generate_engine_methods_java())
            out.append("")
        elif self.archetype == "handle":
            if self.retained_references:
                ctor_params = ["long nativeObject"]
                assignments = ["        mNativeObject = nativeObject;"]
                for ref in self.retained_references.values():
                    # MaterialInstance allows a @Nullable Material in its constructor because
                    # legacy/internal callers (e.g. RenderableManager, gltfio) can instantiate
                    # instances via MaterialInstance.wrap(long) where no Java Material wrapper
                    # is available, leaving mMaterial null.
                    ann = "@Nullable " if (ref.get("is_nullable") or self.name == "MaterialInstance") else f"{ref['nullability_annotation']} "
                    ctor_params.append(f"{ann}{ref['java_type']} {ref['param_name']}")
                    assignments.append(f"        {ref['field_name']} = {ref['param_name']};")

                out.append(f"    {self.name}({', '.join(ctor_params)}) {{")
                for a in assignments:
                    out.append(a)
                out.append("    }")
                out.append("")

                if self.name == "MaterialInstance":
                    out.append("    /* package */ MaterialInstance(long nativeMaterialInstance) {")
                    out.append("        this(nativeMaterialInstance, null);")
                    out.append("    }")
                    out.append("")
            elif not self.base_class:
                if self.name == "Material":
                    out.append("    Material(long nativeObject) {")
                    out.append("        mNativeObject = nativeObject;")
                    out.append("        mDefaultInstance = new MaterialInstance(nGetDefaultInstance(nativeObject), this);")
                    out.append("    }")
                    out.append("")
                else:
                    out.append(f"    {self.name}(long nativeObject) {{")
                    out.append(f"        mNativeObject = nativeObject;")
                    out.append("    }")
                    out.append("")

            # Step 11b: Static Factory wrap(...) Method
            if not self.base_class and self.name != "Engine":
                if self.retained_references:
                    wrap_params = ["long nativeObject"]
                    wrap_args = ["nativeObject"]
                    all_nullable = True
                    for ref in self.retained_references.values():
                        is_nullable = bool(ref.get("is_nullable") or self.name == "MaterialInstance")
                        if not is_nullable:
                            all_nullable = False
                        ann = "@Nullable " if is_nullable else f"{ref['nullability_annotation']} "
                        wrap_params.append(f"{ann}{ref['java_type']} {ref['param_name']}")
                        wrap_args.append(ref['param_name'])

                    out.append("    @NonNull")
                    out.append("    @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)")
                    out.append(f"    public static {self.name} wrap({', '.join(wrap_params)}) {{")
                    out.append(f"        return new {self.name}({', '.join(wrap_args)});")
                    out.append("    }")
                    out.append("")

                    if all_nullable:
                        null_args = ["nativeObject"] + ["null"] * len(self.retained_references)
                        out.append("    @NonNull")
                        out.append("    @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)")
                        out.append(f"    public static {self.name} wrap(long nativeObject) {{")
                        out.append(f"        return new {self.name}({', '.join(null_args)});")
                        out.append("    }")
                        out.append("")
                else:
                    out.append("    @NonNull")
                    out.append("    @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)")
                    out.append(f"    public static {self.name} wrap(long nativeObject) {{")
                    out.append(f"        return new {self.name}(nativeObject);")
                    out.append("    }")
                    out.append("")

        # Step 12: Inner Builder Class Generation
        if self.builder:
            out.append(self.builder.generate_builder_class_java())
            out.append("")

        # Step 13: Backward-Compatibility Type Aliases
        for alias in self.raw_aliases:
            alias_name = alias.get("name")
            target_type = alias.get("type", {})
            target_cpp = target_type.get("cpp_name", "")
            target_qname = target_type.get("qualified_name", "")
            if alias_name == "PixelBufferDescriptor" or target_cpp.endswith("PixelBufferDescriptor") or target_qname.endswith("PixelBufferDescriptor"):
                out.append("    /**")
                out.append("     * @deprecated Use {@link com.google.android.filament.PixelBufferDescriptor} instead.")
                out.append("     */")
                out.append("    @Deprecated")
                out.append("    public static class PixelBufferDescriptor extends com.google.android.filament.PixelBufferDescriptor {")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type,")
                out.append("                @IntRange(from = 1, to = 8) int alignment,")
                out.append("                @IntRange(from = 0) int left, @IntRange(from = 0) int top,")
                out.append("                @IntRange(from = 0) int stride,")
                out.append("                @Nullable Object handler, @Nullable Runnable callback) {")
                out.append("            super(storage, format, type, alignment, left, top, stride, handler, callback);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type) {")
                out.append("            super(storage, format, type);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type,")
                out.append("                @IntRange(from = 1, to = 8) int alignment) {")
                out.append("            super(storage, format, type, alignment);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type,")
                out.append("                @IntRange(from = 1, to = 8) int alignment,")
                out.append("                @IntRange(from = 0) int left, @IntRange(from = 0) int top) {")
                out.append("            super(storage, format, type, alignment, left, top);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type,")
                out.append("                @Nullable Object handler, @Nullable Runnable callback) {")
                out.append("            this(storage, format, type, 1, 0, 0, 0, handler, callback);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull Buffer storage,")
                out.append("                @NonNull Texture.Format format, @NonNull Texture.Type type,")
                out.append("                @IntRange(from = 1, to = 8) int alignment,")
                out.append("                @Nullable Object handler, @Nullable Runnable callback) {")
                out.append("            this(storage, format, type, alignment, 0, 0, 0, handler, callback);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull ByteBuffer storage,")
                out.append("                @NonNull Texture.CompressedType compressedFormat,")
                out.append("                @IntRange(from = 0) int compressedSizeInBytes,")
                out.append("                @Nullable Object handler, @Nullable Runnable callback) {")
                out.append("            this(storage, compressedFormat, compressedSizeInBytes);")
                out.append("            setCallback(handler, callback);")
                out.append("        }")
                out.append("")
                out.append("        public PixelBufferDescriptor(@NonNull ByteBuffer storage,")
                out.append("                @NonNull Texture.CompressedType compressedFormat,")
                out.append("                @IntRange(from = 0) int compressedSizeInBytes) {")
                out.append("            super(storage, compressedFormat, compressedSizeInBytes);")
                out.append("        }")
                out.append("    }")
                out.append("")

        # Step 14: Method Emission & Native Declarations
        native_decls = []
        
        for method in self.methods:
            self.current_method = method["name"]
            generated_methods = self._expand_method(method)
            for m in generated_methods:
                m_java = self._generate_java_method(m)
                if m_java:
                    out.append(m_java)
                    out.append("")
                decl = self._generate_native_decl(m)
                if decl:
                    native_decls.append(decl)
            self.current_method = None

        if self.builder:
            native_decls.extend(self.builder.generate_builder_native_decls())
        if self.name == "Engine":
            native_decls.extend(self._get_engine_native_decls())

        # Step 15: Handle Lifetime Boilerplate & Cached Struct Getters
        if self.name == "ToneMapper":
            out.append("    @Override")
            out.append("    protected void finalize() throws Throwable {")
            out.append("        try {")
            out.append("            super.finalize();")
            out.append("        } finally {")
            out.append("            nDestroyToneMapper(mNativeObject);")
            out.append("        }")
            out.append("    }")
            out.append("")
            native_decls.append("    private static native void nDestroyToneMapper(long nativeObject);")

        # Synthetic getters for cached struct fields without C++ getters
        for cached in self.cached_fields.values():
            if cached.get("is_struct") and not any(m["name"] == cached["getter"] for m in self.methods):
                out.append("    @NonNull")
                out.append(f"    public {cached['type']} {cached['getter']}() {{")
                out.append(f"        if ({cached['field_name']} == null) {{")
                out.append(f"            {cached['field_name']} = new {cached['type']}();")
                out.append("        }")
                out.append(f"        return {cached['field_name']};")
                out.append("    }")
                out.append("")

        if self.archetype == "handle" and not self.base_class:
            if self.is_used_by_native or self.name == "Engine":
                out.append('    @UsedByNative("Engine.cpp")')
            out.append("    public long getNativeObject() {")
            out.append("        if (mNativeObject == 0) {")
            out.append(f"            throw new IllegalStateException(\"Calling method on destroyed {self.name}\");")
            out.append("        }")
            out.append("        return mNativeObject;")
            out.append("    }")
            out.append("")
            
            out.append("    void clearNativeObject() {")
            out.append("        mNativeObject = 0;")
            out.append("    }")
        
        if self.name == "View":
            out.append("    /**")
            out.append("     * @deprecated Use {@link #setColorGrading(ColorGrading)}")
            out.append("     */")
            out.append("    @Deprecated")
            out.append("    public void setToneMapping(@NonNull ToneMapping type) {")
            out.append("    }")
            out.append("")
            out.append("    /**")
            out.append("     * @deprecated Use {@link #getColorGrading()}. This always returns {@link ToneMapping#ACES}")
            out.append("     */")
            out.append("    @Deprecated")
            out.append("    @NonNull")
            out.append("    public ToneMapping getToneMapping() {")
            out.append("        return ToneMapping.ACES;")
            out.append("    }")
            out.append("")
            out.append("    /**")
            out.append("     * An interface to implement a custom class to receive results of picking queries.")
            out.append("     */")
            out.append("    public interface OnPickCallback {")
            out.append("        /**")
            out.append("         * onPick() is called by the specified Handler in {@link View#pick} when the picking query")
            out.append("         * result is available.")
            out.append("         * @param result An instance of {@link PickingQueryResult}.")
            out.append("         */")
            out.append("        void onPick(@NonNull PickingQueryResult result);")
            out.append("    }")
            out.append("")
            out.append("    /**")
            out.append("     * Creates a picking query. Multiple queries can be created (e.g.: multi-touch).")
            out.append("     * Picking queries are all executed when {@link Renderer#render} is called on this View.")
            out.append("     * The provided callback is guaranteed to be called at some point in the future.")
            out.append("     *")
            out.append("     * <p>Typically it takes a couple frames to receive the result of a picking query.</p>")
            out.append("     *")
            out.append("     * @param x        Horizontal coordinate to query in the viewport with origin on the left.")
            out.append("     * @param y        Vertical coordinate to query on the viewport with origin at the bottom.")
            out.append("     * @param handler  An {@link java.util.concurrent.Executor Executor}.")
            out.append("     *                 On Android this can also be a {@link android.os.Handler Handler}.")
            out.append("     * @param callback User callback executed by <code>handler</code> when the picking query")
            out.append("     *                 result is available.")
            out.append("     */")
            out.append("    public void pick(int x, int y,")
            out.append("            @Nullable Object handler, @Nullable OnPickCallback callback) {")
            out.append("        InternalOnPickCallback internalCallback = new InternalOnPickCallback(callback);")
            out.append("        nPick(getNativeObject(), x, y, handler, internalCallback);")
            out.append("    }")
            out.append("")
            out.append("    @UsedByNative(\"View.cpp\")")
            out.append("    private static class InternalOnPickCallback implements Runnable {")
            out.append("        private final OnPickCallback mUserCallback;")
            out.append("        private final PickingQueryResult mPickingQueryResult = new PickingQueryResult();")
            out.append("")
            out.append("        @UsedByNative(\"View.cpp\")")
            out.append("        @Entity")
            out.append("        int mRenderable;")
            out.append("")
            out.append("        @UsedByNative(\"View.cpp\")")
            out.append("        float mDepth;")
            out.append("")
            out.append("        @UsedByNative(\"View.cpp\")")
            out.append("        float mFragCoordsX;")
            out.append("        @UsedByNative(\"View.cpp\")")
            out.append("        float mFragCoordsY;")
            out.append("        @UsedByNative(\"View.cpp\")")
            out.append("        float mFragCoordsZ;")
            out.append("")
            out.append("        public InternalOnPickCallback(OnPickCallback mUserCallback) {")
            out.append("            this.mUserCallback = mUserCallback;")
            out.append("        }")
            out.append("")
            out.append("        @Override")
            out.append("        public void run() {")
            out.append("            mPickingQueryResult.renderable = mRenderable;")
            out.append("            mPickingQueryResult.depth = mDepth;")
            out.append("            mPickingQueryResult.fragCoords[0] = mFragCoordsX;")
            out.append("            mPickingQueryResult.fragCoords[1] = mFragCoordsY;")
            out.append("            mPickingQueryResult.fragCoords[2] = mFragCoordsZ;")
            out.append("            if (mUserCallback != null) {")
            out.append("                mUserCallback.onPick(mPickingQueryResult);")
            out.append("            }")
            out.append("        }")
            out.append("    }")
            out.append("")
            native_decls.append("    private static native void nPick(long nativeView, int x, int y, @Nullable Object handler, @NonNull InternalOnPickCallback internalCallback);")

        # Step 16: Nested Struct Classes & Native Declarations Collection
        if self.nested_struct_contexts:
            for nested_ctx in self.nested_struct_contexts:
                if nested_ctx.name in ("Ssct", "Gtao", "FroxelConfigurationInfo", "FroxelConfigurationInfoWithAge", "PickingQuery"):
                    continue
                if nested_ctx.is_pojo_struct():
                    out.append(nested_ctx._generate_pojo_struct(indent="    "))
                else:
                    out.append(nested_ctx._generate_aggregate_java(is_nested=True, indent="    "))
                out.append("")

        if native_decls:
           out.append("")
           out.append("\n".join(native_decls))

        out.append("}")
        out.append("")
        out.append(GENERATED_FILE_WARNING)
        return "\n".join(out) + "\n"

    def _generate_pojo_struct(self, indent="    ") -> str:
        """Generate Java source code for a POJO options struct class.

        POJO structs (e.g. DynamicResolutionOptions, AmbientOcclusionOptions, etc.)
        feature public mutable fields with default values, nested enums, and
        flattened sub-struct fields (e.g. ssct, gtao) directly matching the
        public Java API.
        """
        out = []
        doc = generate_javadoc(self.doc, indent_spaces=len(indent))
        if doc:
            out.append(doc)
        out.append(f"{indent}public static class {self.name} {{")
        is_engine_config = (self.name == "Config" and getattr(self.parent_context, "name", "") == "Engine") or (self.name == "Config" and getattr(self, "ctx", None) and getattr(self.ctx, "name", "") == "Engine") or (self.name == "Config" and any(m.get("name") == "create" for m in getattr(self.ctx, "methods", []))) or (self.name == "Config" and getattr(self.ctx, "parent_class", "") == "Engine") or (self.name == "Config" and getattr(self, "ir", {}).get("parent_class") == "Engine")
        if is_engine_config:
            out.append(f"{indent}    // #defines in Engine.h")
            out.append(f"{indent}    private static final long FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB = 3;")
            out.append(f"{indent}    private static final long FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB = 2;")
            out.append(f"{indent}    private static final long FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB = 1;")
            out.append(f"{indent}    private static final long FILAMENT_COMMAND_BUFFER_SIZE_IN_MB =")
            out.append(f"{indent}            FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3;")
            out.append(f"{indent}")
            out.append(f"{indent}    /**")
            out.append(f"{indent}     * Special value for jobSystemThreadCount, forcing the JobSystem to be single-threaded.")
            out.append(f"{indent}     */")
            out.append(f"{indent}    public static final long SINGLE_THREADED = 0xFFFFFFFFL;")
            out.append(f"{indent}")

        # Nested enums
        for enum_ir in self.enums:
            edoc = generate_javadoc(enum_ir.get("doc", {}), indent_spaces=len(indent) + 4)
            if edoc:
                out.append(edoc)
            out.append(f"{indent}    public enum {enum_ir['name']} {{")
            custom_enum = is_custom_enum(enum_ir)
            entry_strs = []
            for entry in enum_ir.get("entries", []):
                vdoc = generate_javadoc(entry.get("doc", {}), indent_spaces=len(indent) + 8)
                name = format_enum_entry_name(enum_ir['name'], entry['name'])
                if custom_enum:
                    val = entry.get("value", 0)
                    entry_line = f"{indent}        {name}({val})"
                else:
                    entry_line = f"{indent}        {name}"
                if vdoc:
                    entry_strs.append(f"{vdoc}\n{entry_line}")
                else:
                    entry_strs.append(entry_line)
            out.append(",\n".join(entry_strs) + ";\n")
            if custom_enum:
                out.append(f"{indent}        private final int mValue;\n")
                out.append(f"{indent}        {enum_ir['name']}(int value) {{")
                out.append(f"{indent}            mValue = value;")
                out.append(f"{indent}        }}\n")
                out.append(f"{indent}        public int toFilamentNative() {{ return mValue; }}\n")
            else:
                out.append(f"{indent}        public int toFilamentNative() {{ return ordinal(); }}")
            out.append(f"{indent}    }}")
            out.append("")

        # Fields
        for f in self.fields:
            fname = f["name"]
            attrs = f.get("attributes", [])
            # Skip padding/reserved/rfu fields
            if is_reserved_or_padding_field(fname):
                continue

            # Check if this field is a flattened sub-struct (e.g. ssct, gtao)
            if "apigen:flatten" in attrs:
                f_type = f.get("type", {})
                cpp_type_name = f_type.get("cpp_name", "") if isinstance(f_type, dict) else str(f_type)
                clean_cpp_type = self._strip_namespaces(cpp_type_name).replace("*", "").replace("&", "").replace("const", "").strip()
                sub_cls = self.is_pojo_struct(clean_cpp_type)
                if not sub_cls:
                    sub_cls = KNOWN_CLASSES.get(clean_cpp_type)
                if sub_cls:
                    for sf in sub_cls.get("fields", []):
                        sf_name = sf["name"]
                        if is_reserved_or_padding_field(sf_name):
                            continue
                        prefixed_name = fname + sf_name[0].upper() + sf_name[1:]
                        sf_doc = generate_javadoc(sf.get("doc", {}), indent_spaces=len(indent) + 4)
                        if sf_doc:
                            out.append(sf_doc)
                        sf_ann_list = self.get_pojo_field_annotation(sf, sub_cls.get("name"))
                        for ann in sf_ann_list:
                            out.append(f"{indent}    {ann}")
                        sf_type_info, _ = self.resolve_type_info(sf.get("type"), silent=True)
                        sf_jtype = sf_type_info.get("java", "float")
                        arr_in = self.get_array_input_info(sf.get("type"))
                        if arr_in:
                            sf_jtype = arr_in["java"]
                        default_val = self.translate_default_value(sf, sub_cls.get("name"))
                        init_str = f" = {default_val}" if default_val is not None else ""
                        out.append(f"{indent}    public {sf_jtype} {prefixed_name}{init_str};")
                continue

            fdoc = generate_javadoc(f.get("doc", {}), indent_spaces=len(indent) + 4)
            if fdoc:
                out.append(fdoc)

            ann_list = self.get_pojo_field_annotation(f, self.name)
            if is_engine_config:
                ann_list = []
            for ann in ann_list:
                out.append(f"{indent}    {ann}")

            # Check type
            if "apigen:java_type:float" in attrs:
                jtype = "float"
            else:
                f_type = f.get("type", {})
                arr_in = self.get_array_input_info(f_type)
                if arr_in:
                    jtype = arr_in["java"]
                else:
                    t_info, _ = self.resolve_type_info(f_type, silent=True)
                    jtype = t_info.get("java", "float")
                    if any(e["name"] == jtype for e in self.enums):
                        jtype = f"{self.name}.{jtype}"

            if is_engine_config:
                if jtype in ("int", "short", "byte") or any(it in str(f.get("type", {})).lower() for it in ("uint", "size_t", "int32", "int64", "uint32", "uint8")):
                    jtype = "long"

            default_val = self.translate_default_value(f, self.name)
            if self.name == "PickingQueryResult" and fname == "fragCoords" and not default_val:
                default_val = "new float[3]"
            if is_engine_config:
                config_defaults = {
                    "commandBufferSizeMB": "FILAMENT_COMMAND_BUFFER_SIZE_IN_MB",
                    "perRenderPassArenaSizeMB": "FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB",
                    "driverHandleArenaSizeMB": "0",
                    "minCommandBufferSizeMB": "FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB",
                    "perFrameCommandsSizeMB": "FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB",
                    "jobSystemThreadCount": "0",
                    "metalUploadBufferSizeBytes": "512 * 1024",
                    "metalDisablePanicOnDrawableFailure": "false",
                    "disableParallelShaderCompile": "false",
                    "stereoscopicType": "StereoscopicType.NONE",
                    "stereoscopicEyeCount": "2",
                    "resourceAllocatorCacheSizeMB": "64",
                    "resourceAllocatorCacheMaxAge": "1",
                    "disableHandleUseAfterFreeCheck": "false",
                    "preferredShaderLanguage": "ShaderLanguage.DEFAULT",
                    "forceGLES2Context": "false",
                    "assertNativeWindowIsValid": "false",
                    "gpuContextPriority": "GpuContextPriority.DEFAULT",
                    "sharedUboInitialSizeInBytes": "256 * 64",
                    "asynchronousMode": "AsynchronousMode.NONE",
                    "materialCacheCapacity": "0",
                    "programCacheCapacity": "0",
                    "enableMultipleDirectionalLights": "false",
                }
                if fname in config_defaults:
                    default_val = config_defaults[fname]
            init_str = f" = {default_val}" if default_val is not None else ""
            out.append(f"{indent}    public {jtype} {fname}{init_str};")

        out.append(f"{indent}}}")
        return "\n".join(out)


    def _generate_aggregate_java(self, is_nested=False, indent=""):
        """Generate Java source code for an aggregate Plain-Old-Data (POD) struct class.

        Aggregate structs (e.g. `Box`, `Aabb`, `Viewport`, `Frustum`) map C++ POD structures
        directly to idiomatic Java classes. They are emitted with exploded internal storage
        fields to enable high-efficiency AAPCS64 register-passed C++ reconstruction.

        Emits:
        1. **Storage Fields**:
           - Math Vectors/Arrays: Exploded into individual primitive scalar fields
             (e.g., `float3 center` -> `private float mCenterX, mCenterY, mCenterZ`).
           - Nested Aggregate Structs: Recursively flattened into leaf scalar fields.
           - Slices: Stored as `@Nullable <Type>[]`.
           - Scalars: Declared as `public <Type> <name>`.
        2. **Constructors**:
           - Default zero-initializing constructor.
           - All-leaves primitive constructor accepting all exploded scalars directly.
           - Composite constructor accepting high-level arrays and sub-struct objects.
        3. **Accessors (Getters & Setters)**:
           - Exploded Setters: `setCenter(float x, float y, float z)`.
           - Array Setters: `setCenter(@NonNull @Size(min = 3) float[] center)`.
           - Zero-Allocation Out-Parameter Getters: `getCenter(@Nullable float[] out)`
             which populates the supplied array or allocates a new one if null.
           - Convenience Parameterless Getters: `getCenter()` delegating to `getCenter(null)`.
           - Component-Level Accessors: `getCenterX()`, `setCenterX(float x)`.
        4. **Nested Structs & Member Methods**:
           - Recursively processes nested struct contexts.
           - Emits member methods and JNI native declarations.

        Args:
            is_nested: Whether this struct is an inner class inside an outer enclosing class.
            indent: Whitespace indentation prefix for inner class formatting.

        Returns:
            Generated Java source code for the aggregate struct.
        """
        # Step 1: Package and Import Headers (if top-level file)
        out = []
        has_int_range = any(self.get_annotation(f["type"]) and "@IntRange" in self.get_annotation(f["type"]) for f in self.fields)
        if not is_nested:
            out.append(LICENSE_HEADER)
            out.append("")
            out.append(GENERATED_FILE_WARNING)
            out.append("")
            out.append(f"package {self.package};")
            out.append("")
            if has_int_range:
                out.append("import androidx.annotation.IntRange;")
            out.append("import androidx.annotation.NonNull;")
            out.append("import androidx.annotation.Nullable;")
            out.append("import androidx.annotation.Size;")
            out.append("")
        
        # Step 2: Class Docstring and Declaration
        if self.doc:
            javadoc = generate_javadoc(self.doc, indent_spaces=len(indent))
            if javadoc:
                out.append(javadoc)
                
        class_decl = f"{indent}public static class {self.name} {{" if is_nested else f"public class {self.name} {{"
        out.append(class_decl)
        
        # Step 3: Class Constants
        if self.constants:
            for const in self.constants:
                c_name = const["name"]
                c_doc = generate_javadoc(const.get("doc"), indent_spaces=len(indent) + 4)
                if c_doc:
                    out.append(c_doc)
                raw_val = const.get("value")
                j_type = self.get_java_type(const["type"])
                val_str = self.format_constant_value(raw_val, j_type, self.constants)
                out.append(f"{indent}    public static final {j_type} {c_name} = {val_str};")
            out.append("")

        leaves = self.get_flattened_leaves(self.ir)
        
        # Step 4: Storage Field Declarations (Exploded Primitives)
        for f in self.fields:
            fname = f["name"]
            fcap = fname[0].upper() + fname[1:]
            arr_in = self.get_array_input_info(f["type"])
            f_info, _ = self.resolve_type_info(f["type"], silent=True)
            if arr_in:
                size = arr_in["size"]
                scalar = arr_in["scalar"]
                is_fixed_arr = arr_in.get("is_fixed_array", False)
                comps = [str(i) for i in range(size)] if (is_fixed_arr or size > 4) else ["X", "Y", "Z", "W"][:size]
                for c in comps:
                    out.append(f"{indent}    private {scalar} m{fcap}{c};")
            elif self.is_aggregate_struct(f["type"]):
                sub_leaves = self.get_flattened_leaves(f["type"], prefix=fcap)
                for sl in sub_leaves:
                    out.append(f"{indent}    private {sl['java_type']} m{sl['name']};")
            elif f_info.get("is_slice"):
                out.append(f"{indent}    private @Nullable {f_info['java']} m{fcap};")
            else:
                ann = self.get_annotation(f["type"])
                ann_str = f"{ann}\n{indent}    " if ann else ""
                jtype = self.get_java_type(f["type"])
                out.append(f"{indent}    {ann_str}public {jtype} {fname};")
        out.append("")
        
        # Step 5: Default Zero-Initializing Constructor
        out.append(f"{indent}    public {self.name}() {{")
        out.append(f"{indent}    }}")
        out.append("")
        
        # Step 6: All-Leaves Primitive Constructor (Exploded Components)
        has_slice = any(self.resolve_type_info(f["type"], silent=True)[0].get("is_slice") for f in self.fields)
        if not has_slice:
            leaf_params = []
            for l in leaves:
                ann = self.get_annotation(l.get("type")) if l.get("type") else ""
                ann_prefix = f"{ann} " if ann else ""
                leaf_params.append(f"{ann_prefix}{l['java_type']} {l['param_name']}")
            out.append(f"{indent}    public {self.name}({', '.join(leaf_params)}) {{")
            for f in self.fields:
                fname = f["name"]
                fcap = fname[0].upper() + fname[1:]
                arr_in = self.get_array_input_info(f["type"])
                if arr_in:
                    size = arr_in["size"]
                    is_fixed_arr = arr_in.get("is_fixed_array", False)
                    comps = [str(i) for i in range(size)] if (is_fixed_arr or size > 4) else ["X", "Y", "Z", "W"][:size]
                    f_args = [f"{fname}{c}" for c in comps]
                    out.append(f"{indent}        set{fcap}({', '.join(f_args)});")
                elif self.is_aggregate_struct(f["type"]):
                    sub_struct = self.is_aggregate_struct(f["type"])
                    sub_name = sub_struct["name"]
                    sub_leaves = self.get_flattened_leaves(f["type"])
                    sub_args = [f"{fname}{l['name']}" for l in sub_leaves]
                    out.append(f"{indent}        set{fcap}(new {sub_name}({', '.join(sub_args)}));")
                else:
                    out.append(f"{indent}        set{fcap}({fname});")
            out.append(f"{indent}    }}")
            out.append("")
        
        # Step 7: Composite Constructor (High-level Objects and Arrays)
        has_composite = any(self.get_array_input_info(f["type"]) or self.is_aggregate_struct(f["type"]) or self.resolve_type_info(f["type"], silent=True)[0].get("is_slice") for f in self.fields)
        if has_composite:
            comp_params = []
            for f in self.fields:
                fname = f["name"]
                arr_in = self.get_array_input_info(f["type"])
                f_info, _ = self.resolve_type_info(f["type"], silent=True)
                if arr_in:
                    comp_params.append(f"@NonNull @Size(min = {arr_in['size']}) {arr_in['java']} {fname}")
                elif self.is_aggregate_struct(f["type"]):
                    scls = self.is_aggregate_struct(f["type"])
                    comp_params.append(f"@NonNull {scls['name']} {fname}")
                elif f_info.get("is_slice"):
                    comp_params.append(f"@Nullable {f_info['java']} {fname}")
                else:
                    ann = self.get_annotation(f["type"])
                    ann_prefix = f"{ann} " if ann else ""
                    comp_params.append(f"{ann_prefix}{self.get_java_type(f['type'])} {fname}")
            out.append(f"{indent}    public {self.name}({', '.join(comp_params)}) {{")
            for f in self.fields:
                fname = f["name"]
                fcap = fname[0].upper() + fname[1:]
                out.append(f"{indent}        set{fcap}({fname});")
            out.append(f"{indent}    }}")
            out.append("")
            
        # Step 8: Multi-Tier Field Getters and Setters
        for f in self.fields:
            fname = f["name"]
            fcap = fname[0].upper() + fname[1:]
            f_info, _ = self.resolve_type_info(f["type"], silent=True)
            arr_in = self.get_array_input_info(f["type"])
            if arr_in:
                size = arr_in["size"]
                j_arr_type = arr_in["java"]
                scalar = arr_in["scalar"]
                is_fixed_arr = arr_in.get("is_fixed_array", False)
                comps = [str(i) for i in range(size)] if (is_fixed_arr or size > 4) else ["X", "Y", "Z", "W"][:size]
                
                # Exploded Setter (e.g. setCenter(float x, float y, float z))
                expl_params = [f"{scalar} {fname}{c}" for c in comps]
                out.append(f"{indent}    public void set{fcap}({', '.join(expl_params)}) {{")
                for c in comps:
                    out.append(f"{indent}        m{fcap}{c} = {fname}{c};")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Array Setter (e.g. setCenter(float[] center))
                out.append(f"{indent}    public void set{fcap}(@NonNull @Size(min = {size}) {j_arr_type} {fname}) {{")
                for idx, c in enumerate(comps):
                    out.append(f"{indent}        m{fcap}{c} = {fname}[{idx}];")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Out-Parameter Zero-Allocation Getter (e.g. getCenter(float[] out))
                out.append(f"{indent}    @NonNull @Size(min = {size})")
                out.append(f"{indent}    public {j_arr_type} get{fcap}(@Nullable @Size(min = {size}) {j_arr_type} out) {{")
                out.append(f"{indent}        if (out == null) {{")
                out.append(f"{indent}            out = new {scalar}[{size}];")
                out.append(f"{indent}        }}")
                for idx, c in enumerate(comps):
                    out.append(f"{indent}        out[{idx}] = m{fcap}{c};")
                out.append(f"{indent}        return out;")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Convenience Parameterless Getter (e.g. getCenter())
                out.append(f"{indent}    @NonNull @Size(min = {size})")
                out.append(f"{indent}    public {j_arr_type} get{fcap}() {{")
                out.append(f"{indent}        return get{fcap}(null);")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Individual Component Scalar Getters / Setters (e.g. getCenterX(), setCenterX(float))
                for c in comps:
                    out.append(f"{indent}    public {scalar} get{fcap}{c}() {{")
                    out.append(f"{indent}        return m{fcap}{c};")
                    out.append(f"{indent}    }}")
                    out.append("")
                    out.append(f"{indent}    public void set{fcap}{c}({scalar} {fname}{c}) {{")
                    out.append(f"{indent}        m{fcap}{c} = {fname}{c};")
                    out.append(f"{indent}    }}")
                    out.append("")
                    
            elif self.is_aggregate_struct(f["type"]):
                sub_struct = self.is_aggregate_struct(f["type"])
                sub_name = sub_struct["name"]
                sub_leaves = self.get_flattened_leaves(f["type"])
                
                # Struct Setter
                out.append(f"{indent}    public void set{fcap}(@NonNull {sub_name} {fname}) {{")
                for sl in sub_leaves:
                    out.append(f"{indent}        m{fcap}{sl['name']} = {fname}.get{sl['name']}();")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Out-Parameter Struct Getter
                out.append(f"{indent}    @NonNull")
                out.append(f"{indent}    public {sub_name} get{fcap}(@Nullable {sub_name} out) {{")
                out.append(f"{indent}        if (out == null) {{")
                out.append(f"{indent}            out = new {sub_name}();")
                out.append(f"{indent}        }}")
                for sl in sub_leaves:
                    out.append(f"{indent}        out.set{sl['name']}(m{fcap}{sl['name']});")
                out.append(f"{indent}        return out;")
                out.append(f"{indent}    }}")
                out.append("")
                
                # Convenience Parameterless Struct Getter
                out.append(f"{indent}    @NonNull")
                out.append(f"{indent}    public {sub_name} get{fcap}() {{")
                out.append(f"{indent}        return get{fcap}(null);")
                out.append(f"{indent}    }}")
                out.append("")
            elif f_info.get("is_slice"):
                out.append(f"{indent}    public void set{fcap}(@Nullable {f_info['java']} {fname}) {{")
                out.append(f"{indent}        m{fcap} = {fname};")
                out.append(f"{indent}    }}")
                out.append("")
                out.append(f"{indent}    @Nullable")
                out.append(f"{indent}    public {f_info['java']} get{fcap}() {{")
                out.append(f"{indent}        return m{fcap};")
                out.append(f"{indent}    }}")
                out.append("")
            else:
                jtype = self.get_java_type(f["type"])
                ann = self.get_annotation(f["type"])
                ann_str = f"{ann} " if ann else ""
                out.append(f"{indent}    public void set{fcap}({ann_str}{jtype} {fname}) {{")
                out.append(f"{indent}        this.{fname} = {fname};")
                out.append(f"{indent}    }}")
                out.append("")
                out.append(f"{indent}    {ann_str}public {jtype} get{fcap}() {{")
                out.append(f"{indent}        return {fname};")
                out.append(f"{indent}    }}")
                out.append("")
                
        # Step 9: Recursive Nested Struct Generation
        if self.nested_struct_contexts:
            for nested_ctx in self.nested_struct_contexts:
                out.append(nested_ctx._generate_aggregate_java(is_nested=True, indent=indent + "    "))
                out.append("")

        # Step 10: Aggregate Member Methods and Native Declarations
        native_decls = []
        for method in self.methods:
            if method.get("is_constructor"):
                continue
            self.current_method = method["name"]
            generated_methods = self._expand_method(method)
            for m in generated_methods:
                m_code = self._generate_java_method(m)
                if m_code:
                    if is_nested:
                        m_code = indent_lines(m_code, spaces=len(indent))
                    out.append(m_code)
                    out.append("")
                decl = self._generate_native_decl(m)
                if decl:
                    if is_nested:
                        decl = indent_lines(decl, spaces=len(indent))
                    native_decls.append(decl)
        self.current_method = None
        
        if native_decls:
            out.append("")
            out.append("\n".join(native_decls))
            
        out.append(f"{indent}}}")
        if not is_nested:
            out.append("")
            out.append(GENERATED_FILE_WARNING)
            return "\n".join(out) + "\n"
        return "\n".join(out)


    def _generate_vector_array_overloads(self, method, effective_doc, method_prefix, ret_type, ret_ann, is_builder=False):
        """Generate array-based overloads for methods accepting unrolled math vectors.

        When a C++ API accepts a fixed math vector (e.g. `math::float3`), the primary Java binding
        unrolls the vector into primitive components (`float x, float y, float z`) to avoid
        object/array allocation. This method synthesizes a complementary public overload that accepts
        a primitive array (`@NonNull @Size(min = 3) float[] value`) and unpacks `value[0], value[1], ...`
        directly into the unrolled method call.

        Algorithm:
        1. **Vector Detection**: Scans arguments to verify if any parameter is an unrolled vector
           (excluding matrices, pointers, or size-parameter consumers).
        2. **Ambiguity Audit**: Checks against all sibling overloads to prevent Java signature
           collisions when another method shares the same name and parameter types but differs only
           in vector dimensionality.
        3. **Signature Synthesis**: Constructs parameter list with `@NonNull @Size(min = N)` annotations,
           extracts component call expressions (`arr[0], arr[1]`), clones Javadoc parameter docs,
           and checks `emitted_signatures` to prevent duplicate emissions.

        Args:
            method: C++ method IR dictionary.
            effective_doc: Parsed Javadoc documentation dictionary.
            method_prefix: Access modifier prefix (e.g. 'public ' or 'public static ').
            ret_type: Java return type string.
            ret_ann: Return type annotation string (e.g. '@NonNull'), or None.
            is_builder: Whether this overload is being emitted inside an inner Builder class.

        Returns:
            List of generated Java method strings.
        """
        c_args = method.get("arguments", [])
        name = get_effective_method_name(method)
        
        # Step 1: Detect Unrolled Vector Arguments
        has_unrolled_vector = False
        for arg in c_args:
            req_type = arg["type"]
            arr_in = self.get_array_input_info(req_type)
            if arr_in:
                req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = arg["type"].get("is_pointer", False) if isinstance(arg["type"], dict) else False
                size_param_name = get_size_param_attr(arg)
                if not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                    has_unrolled_vector = True
                    break

        if not has_unrolled_vector:
            return []

        # Step 2: Sibling Overload Ambiguity Check
        is_ambiguous = False
        for other_m in self.methods:
            for exp_other in self._expand_method(other_m):
                other_ir = exp_other["ir"]
                if other_ir is method:
                    continue
                if get_effective_method_name(other_ir) != name:
                    continue
                other_args = other_ir.get("arguments", [])
                if len(other_args) != len(c_args):
                    continue
                diff_size_same_scalar = False
                all_others_match = True
                for a1, a2 in zip(c_args, other_args):
                    v1 = self.get_array_input_info(a1["type"])
                    v2 = self.get_array_input_info(a2["type"])
                    if v1 and v2:
                        if v1["scalar"] == v2["scalar"]:
                            if v1["size"] != v2["size"]:
                                diff_size_same_scalar = True
                        else:
                            all_others_match = False
                            break
                    elif not v1 and not v2:
                        if self.get_java_type(a1["type"]) != self.get_java_type(a2["type"]):
                            all_others_match = False
                            break
                    else:
                        all_others_match = False
                        break
                if all_others_match and diff_size_same_scalar:
                    is_ambiguous = True
                    break
            if is_ambiguous:
                break

        if is_ambiguous:
            return []

        # Step 3: Parameter List and Element Unpacking Generation
        params_list = []
        call_args = []
        doc_params = {}
        overload_param_types = {}
        for arg in c_args:
            arg_name = arg["name"]
            req_type = arg["type"]
            arr_in = self.get_array_input_info(req_type)
            is_matrix = False
            is_pointer = False
            size_param_name = None
            if arr_in:
                req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = arg["type"].get("is_pointer", False) if isinstance(arg["type"], dict) else False
                size_param_name = get_size_param_attr(arg)

            if arr_in and not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                scalar = arr_in["scalar"]
                size = arr_in["size"]
                j_type = f"{scalar}[]"
                ann = f"@NonNull @Size(min = {size})"
                safe_name = sanitize_identifier(arg_name)
                params_list.append(f"{ann} {j_type} {safe_name}")
                for idx in range(size):
                    call_args.append(f"{safe_name}[{idx}]")
                doc_params[safe_name] = effective_doc.get("params", {}).get(arg_name, "")
                overload_param_types[safe_name] = j_type
            else:
                safe_name = sanitize_identifier(arg_name)
                t_info, _ = self.resolve_type_info(req_type, silent=True)
                jtype = self.get_java_type(req_type)
                if t_info.get("is_string") or t_info.get("java") == "String":
                    nullability = req_type.get("nullability", "unspecified") if isinstance(req_type, dict) else "unspecified"
                    ann = "@Nullable" if nullability == "nullable" else "@NonNull"
                    overload_param_types[safe_name] = "String"
                else:
                    ann = self.get_annotation(req_type)
                    overload_param_types[safe_name] = jtype
                ann_str = f"{ann} " if ann else ""
                params_list.append(f"{ann_str}{jtype} {safe_name}")
                call_args.append(safe_name)
                doc_params[safe_name] = effective_doc.get("params", {}).get(arg_name, "")

        overload_doc = copy.deepcopy(effective_doc)
        overload_doc["params"] = doc_params

        # Step 4: Signature Deduplication and Code Emission
        indent = 8 if is_builder else 4
        indent_str = " " * indent
        sig = self._get_method_sig(name, params_list)
        if sig in self.emitted_signatures:
            return []
        self.emitted_signatures.add(sig)

        javadoc = generate_javadoc(overload_doc, indent_spaces=indent, argument_names=[p.split()[-1] for p in params_list], param_types=overload_param_types)
        lines = []
        if javadoc:
            lines.append(javadoc)
        if ret_ann:
            lines.append(f"{indent_str}{ret_ann}")
        lines.append(f"{indent_str}{method_prefix}{ret_type} {name}({', '.join(params_list)}) {{")
        call_pfx = "return " if ret_type != "void" else ""
        lines.append(f"{indent_str}    {call_pfx}{name}({', '.join(call_args)});")
        lines.append(f"{indent_str}}}")
        return ["\n".join(lines)]

    def _generate_convenience_overloads(self, method, effective_doc, method_prefix, ret_type, ret_ann, math_ret_info, size_param_consumers, indent=4):
        """Generate telescopic Java convenience overloads for methods with trailing default values.

        Because Java lacks default parameter support, C++ methods with default parameters
        (e.g., `void setFoo(int a, int b = 0, int c = 1)`) require telescopic convenience
        overloads (`setFoo(int a)` and `setFoo(int a, int b)`) that forward down to the primary
        method with the default values inserted.

        Specialized Overloads:
        1. **VsyncTick Unpacking**: `setupFrame` methods unwrapping frame timestamps and timeline counts.
        2. **Trailing Default Arguments**: Iterates through parameter lengths, projecting omitted
           arguments to translated C++ default literals.
        3. **Out-Parameter Convenience**: Emits parameterless getters delegating to `getFoo(null)`.

        Args:
            method: C++ method IR dictionary.
            effective_doc: Parsed Javadoc documentation dictionary.
            method_prefix: Access modifier prefix (e.g. 'public ' or 'public static ').
            ret_type: Java return type string.
            ret_ann: Return type annotation string, or None.
            math_ret_info: Resolved math return info if method returns math vector/matrix.
            size_param_consumers: Mapping of size argument names to their parent array arguments.
            indent: Number of spaces for Java code indentation (default: 4, builders: 8).

        Returns:
            List of generated Java convenience method strings.
        """
        # Step 1: Pre-condition Guard Checks
        if self.is_async_callback_method(method):
            return []
        ret_info, _ = self.resolve_type_info(method.get("return_type"), silent=True)
        if ret_info.get("is_container") and not math_ret_info:
            return []

        c_args = method.get("arguments", [])
        name = get_effective_method_name(method)
        is_constructor = method.get("is_constructor", False)
        
        # Step 2: Count Trailing Default Arguments
        trailing_defaults_count = 0
        for arg in reversed(c_args):
            if arg.get("default_value") is not None:
                trailing_defaults_count += 1
            else:
                break

        indent_str = " " * indent
        body_indent_str = " " * (indent + 4)
        clean_prefix = method_prefix.strip()
        prefix_str = f"{clean_prefix} " if clean_prefix else ""

        convenience_methods = []
        
        # Step 3: Specialized VsyncTick Unpacking Overloads
        if len(c_args) == 1 and self.resolve_type_info(c_args[0]["type"], silent=True)[0].get("is_struct"):
            struct_cls = self.resolve_type_info(c_args[0]["type"], silent=True)[0]["struct_cls"]
            if struct_cls["name"] == "VsyncTick":
                c_lines = []
                c_lines.append(f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos, @Nullable long[] hardwareTimelines, int timelineCount) {{")
                c_lines.append(f"{body_indent_str}return FrameStatus.from(nSetupFrame(getNativeObject(), frameTimeNanos, vsyncPeriodNanos, 0, hardwareTimelines, timelineCount));")
                c_lines.append(f"{indent_str}}}")
                c_lines.append("")
                c_lines.append(f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos, long vsyncPeriodNanos) {{")
                c_lines.append(f"{body_indent_str}return setupFrame(frameTimeNanos, vsyncPeriodNanos, null, 0);")
                c_lines.append(f"{indent_str}}}")
                c_lines.append("")
                c_lines.append(f"{indent_str}public FrameStatus setupFrame(long frameTimeNanos) {{")
                c_lines.append(f"{body_indent_str}return setupFrame(frameTimeNanos, 16666666L);")
                c_lines.append(f"{indent_str}}}")
                convenience_methods.append("\n".join(c_lines))

        # Step 4: Telescopic Overloads for Trailing Default Arguments
        if trailing_defaults_count > 0:
            for plen in range(len(c_args) - trailing_defaults_count, len(c_args)):
                p_args = c_args[:plen]
                o_args = c_args[plen:]
                
                conv_params_list = []
                f_args = []
                conv_names_list = []
                conv_param_types = {}

                for arg in p_args:
                    arg_name = arg["name"]
                    req_type = arg["type"]
                    
                    if arg_name in size_param_consumers and len(size_param_consumers[arg_name]) == 1:
                        continue
                        
                    size_param_name = get_size_param_attr(arg)

                    arr_in = self.get_array_input_info(req_type)
                    if arr_in:
                        req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                        is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                        is_pointer = arg["type"].get("is_pointer", False)

                        if not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                            scalar = arr_in["scalar"]
                            size = arr_in["size"]
                            components = ["x", "y", "z", "w"][:size]
                            comp_names = [sanitize_identifier(f"{arg_name}{c}") for c in components]
                            for comp, comp_name in zip(components, comp_names):
                                conv_params_list.append(f"{scalar} {comp_name}")
                                f_args.append(comp_name)
                                conv_names_list.append(comp_name)
                                conv_param_types[comp_name] = scalar
                        else:
                            j_type = arr_in["java"]
                            size = arr_in["size"]
                            nullability = arg["type"].get("nullability", "unspecified")
                            
                            if arr_in.get("is_custom_array"):
                                ann_prefix = f"{arr_in['annotation']} " if arr_in.get("annotation") else ""
                                if arr_in.get("is_slice"):
                                    ann = f"@NonNull {ann_prefix}".strip()
                                elif nullability == "nonnull":
                                    ann = f"@NonNull {ann_prefix}".strip()
                                elif nullability == "nullable":
                                    ann = f"@Nullable {ann_prefix}".strip()
                                else:
                                    ann = ann_prefix.strip()
                            else:
                                ann = f"@NonNull @Size(min = {size})"
                                if is_pointer:
                                    if nullability == "nullable":
                                        ann = f"@Nullable @Size(min = {size})"
                                    elif nullability == "unspecified":
                                        ann = f"@Size(min = {size})"

                            safe_name = sanitize_identifier(arg_name)
                            p_str = f"{ann} {j_type} {safe_name}".strip()
                            conv_params_list.append(p_str)
                            f_args.append(safe_name)
                            conv_names_list.append(safe_name)
                            conv_param_types[safe_name] = j_type
                    else:
                        safe_name = sanitize_identifier(arg_name)
                        conv_names_list.append(safe_name)
                        inv_info = self.get_invocable_info(req_type, name)
                        if inv_info:
                            conv_params_list.append(f"@NonNull {inv_info['interface_name']} {safe_name}")
                            f_args.append(safe_name)
                            conv_param_types[safe_name] = inv_info["interface_name"]
                            continue

                        type_info, _ = self.resolve_type_info(req_type)
                        if type_info.get("is_struct"):
                            conv_params_list.append(f"@NonNull {type_info['java']} {safe_name}")
                            f_args.append(safe_name)
                            conv_param_types[safe_name] = type_info["java"]
                            continue

                        if type_info.get("is_string") or type_info.get("java") == "String":
                            nullability = arg["type"].get("nullability", "unspecified") if isinstance(arg["type"], dict) else "unspecified"
                            ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                            conv_params_list.append(f"{ann}String {safe_name}")
                            f_args.append(safe_name)
                            conv_param_types[safe_name] = "String"
                            continue

                        if type_info.get("is_enum"):
                            nullability = arg["type"].get("nullability", "unspecified")
                            ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                            conv_params_list.append(f"{ann}{type_info['java']} {safe_name}")
                            f_args.append(safe_name)
                            conv_param_types[safe_name] = type_info["java"]
                            continue

                        j_type = self.get_java_type(req_type)
                        ann = self.get_annotation(req_type)
                        if type_info.get("is_filament_type"):
                            nullability = arg["type"].get("nullability", "unspecified")
                            if nullability == "nullable":
                                ann = "@Nullable"
                            elif nullability == "nonnull" or arg["type"].get("is_reference", False):
                                ann = "@NonNull"
                            else:
                                ann = None
                        p_str = ""
                        if ann:
                            p_str += f"{ann} "
                        p_str += f"{j_type} {safe_name}"
                        conv_params_list.append(p_str)
                        f_args.append(safe_name)
                        conv_param_types[safe_name] = j_type

                # Translate omitted default parameters to their C++ literals
                for arg in o_args:
                    def_raw = arg.get("default_value")
                    arr_in = self.get_array_input_info(arg["type"])
                    req_type_str = arg["type"].get("qualified_name") or arg["type"]["cpp_name"] if isinstance(arg["type"], dict) else arg["type"]
                    is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                    is_pointer = arg["type"].get("is_pointer", False)

                    if arr_in and not is_matrix and not is_pointer and not arr_in.get("is_slice"):
                        match = re.search(r"\{([^}]+)\}", def_raw)
                        if match:
                            vals = [e.strip() for e in match.group(1).split(",")]
                        else:
                            vals = [def_raw] * arr_in["size"]
                        for v in vals:
                            f_args.append(v)
                    else:
                        def_val = self.translate_default_value(def_raw, arg["type"])
                        f_args.append(def_val)

                # Emit constructor chaining or method invocation
                if is_constructor:
                    conv_doc = generate_javadoc(effective_doc, indent_spaces=indent, argument_names=conv_names_list, param_types=conv_param_types)
                    conv_lines = []
                    if conv_doc:
                        conv_lines.append(conv_doc)
                    ctor_prefix = prefix_str if prefix_str else "public "
                    conv_lines.append(f"{indent_str}{ctor_prefix}{self.name}({', '.join(conv_params_list)}) {{")
                    conv_lines.append(f"{body_indent_str}this({', '.join(f_args)});")
                    conv_lines.append(f"{indent_str}}}")
                    convenience_methods.append("\n".join(conv_lines))
                elif math_ret_info:
                    j_arr_type = math_ret_info["java"]
                    size = math_ret_info["size"]
                    conv_params_list.append(f"@Nullable @Size(min = {size}) {j_arr_type} out")
                    f_args.append("out")
                    conv_names_list.append("out")
                    conv_param_types["out"] = j_arr_type

                    conv_doc_dict = copy.deepcopy(effective_doc)
                    conv_doc_dict.setdefault("params", {})["out"] = "optional array to store the result, or null to allocate a new one"

                    conv_doc = generate_javadoc(conv_doc_dict, indent_spaces=indent, argument_names=conv_names_list, param_types=conv_param_types)
                    conv_lines = []
                    if conv_doc:
                        conv_lines.append(conv_doc)
                    conv_lines.append(f"{indent_str}@NonNull @Size(min = {size})")
                    conv_lines.append(f"{indent_str}{prefix_str}{j_arr_type} {name}({', '.join(conv_params_list)}) {{")
                    conv_lines.append(f"{body_indent_str}return {name}({', '.join(f_args)});")
                    conv_lines.append(f"{indent_str}}}")
                    convenience_methods.append("\n".join(conv_lines))
                elif ret_info.get("is_struct") and not self.is_aggregate:
                    struct_cls = ret_info.get("struct_cls") or self.is_aggregate_struct(ret_info.get("cpp_type"))
                    struct_name = struct_cls["name"] if struct_cls else ret_info["java"]
                    conv_params_list.append(f"@Nullable {struct_name} out")
                    f_args.append("out")
                    conv_names_list.append("out")
                    conv_param_types["out"] = struct_name

                    conv_doc_dict = copy.deepcopy(effective_doc)
                    conv_doc_dict.setdefault("params", {})["out"] = "optional object to store the result, or null to allocate a new one"

                    conv_doc = generate_javadoc(conv_doc_dict, indent_spaces=indent, argument_names=conv_names_list, param_types=conv_param_types)
                    conv_lines = []
                    if conv_doc:
                        conv_lines.append(conv_doc)
                    conv_lines.append(f"{indent_str}@NonNull")
                    conv_lines.append(f"{indent_str}{prefix_str}{struct_name} {name}({', '.join(conv_params_list)}) {{")
                    conv_lines.append(f"{body_indent_str}return {name}({', '.join(f_args)});")
                    conv_lines.append(f"{indent_str}}}")
                    convenience_methods.append("\n".join(conv_lines))
                else:
                    if len(conv_params_list) == 0 and name in JAVA_OBJECT_FINAL_NOARG_METHODS:
                        continue
                    conv_doc = generate_javadoc(effective_doc, indent_spaces=indent, argument_names=conv_names_list, param_types=conv_param_types)
                    conv_lines = []
                    if conv_doc:
                        conv_lines.append(conv_doc)
                    if ret_ann:
                        conv_lines.append(f"{indent_str}{ret_ann}")
                    conv_lines.append(f"{indent_str}{prefix_str}{ret_type} {name}({', '.join(conv_params_list)}) {{")
                    call_pfx = "return " if ret_type != "void" else ""
                    conv_lines.append(f"{body_indent_str}{call_pfx}{name}({', '.join(f_args)});")
                    conv_lines.append(f"{indent_str}}}")
                    convenience_methods.append("\n".join(conv_lines))

        return convenience_methods

    def _generate_java_method(self, m_ctx):
        """Generates public Java method definitions, overloads, and JNI bridges.

        Translates an intermediate representation (IR) method definition or expanded
        overload context into complete, idiomatic Java source code. This includes:
        - Interception of cached getters and specialized engine properties.
        - Material class reflection and instance querying helpers.
        - BufferDescriptor and PixelBufferDescriptor streaming overloads.
        - Tagged scalar array overloads with Element enum tags.
        - Packed buffer overloads supporting both NIO Direct Buffers and float arrays.
        - Value class inline buffer getters (`mPlanes`) and value object returns.
        - Asynchronous callback handlers and dispatchers.
        - Constructor method emission with class hierarchy initialization.
        - Slice, Container, and POD struct returns with pre-allocated buffer reuse.
        - Math vector and matrix methods with `Asserts.*` bounds validation.
        - Standard method calls with enum conversions, object wrapping, and convenience overloads.

        Args:
            m_ctx (Dict[str, Any]): Expanded method execution context containing:
                - 'ir' (Dict[str, Any]): Method AST node dictionary.
                - 'specialization_type' (Optional[str]): Template specialization argument
                  or container element subtype if applicable.
                - 'is_tagged_array' (bool): True if expanded into a tagged scalar family overload.
                - 'family' (str): Tagged family identifier (e.g., 'float', 'int').
                - 'tagged_info' (Dict[str, Any]): Parameter binding metadata for tagged arrays.
                - 'buffer_mode' (str): Either 'buffer' (NIO Buffer) or 'array' (float array).
                - 'is_async_callback' (bool): True if method dispatches asynchronous callbacks.

        Returns:
            Optional[str]: Formatted Java method source string (or multiple overloaded
            method definitions separated by double newlines); or None if the signature has
            already been emitted or is intercepted as a non-emitted synthesized helper.

        Raises:
            JavaGenerationError: If parameter extraction, type mapping, or AST structure is invalid.

        Side Effects:
            Mutates `self.emitted_signatures` by registering newly generated method
            signatures to guarantee strict overload deduplication.
        """
        # Step 0: Extract method IR node and calculate effective method name and Javadoc.
        method = m_ctx["ir"]
        spec_type = m_ctx["specialization_type"]
        
        name = get_effective_method_name(method)
        doc = method.get("doc", {})
        
        import copy
        effective_doc = copy.deepcopy(doc)

        if self.name == "Engine":
            intercepted_engine_methods = (
                "destroy",
                "createSwapChain",
            )
            if name in intercepted_engine_methods:
                return None

        # Step 1: Intercept retained reference getters (e.g. getEngine, getMaterial).
        # Synthesizes direct accessor for retained field without native call overhead.
        if name in self.retained_references:
            sig = self._get_method_sig(name, [])
            if sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(sig)
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=[])
            lines = []
            if javadoc:
                lines.append(javadoc)
            ref = self.retained_references[name]
            ann = ref.get("nullability_annotation", "")
            # MaterialInstance.getMaterial() returns null if created via the single-arg constructor
            if self.name == "MaterialInstance":
                ann = ""
            if ann:
                lines.append(f"    {ann}")
            lines.append(f"    public {ref['java_type']} {name}() {{")
            lines.append(f"        return {ref['field_name']};")
            lines.append("    }")
            return "\n".join(lines)

        # Step 2: Intercept cached field getters backed by synthetic instance fields.
        # Lazily allocates or directly returns cached Java wrapper instance to preserve identity.
        if name in [c["getter"] for c in self.cached_fields.values()]:
            sig = self._get_method_sig(name, [])
            if sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(sig)
            info = next(c for c in self.cached_fields.values() if c["getter"] == name)
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=[])
            getter_lines = []
            if javadoc:
                getter_lines.append(javadoc)
            if info.get("is_struct"):
                getter_lines.append("    @NonNull")
                getter_lines.append(f"    public {info['type']} {name}() {{")
                getter_lines.append(f"        if ({info['field_name']} == null) {{")
                getter_lines.append(f"            {info['field_name']} = new {info['type']}();")
                getter_lines.append("        }")
                getter_lines.append(f"        return {info['field_name']};")
                getter_lines.append("    }")
            elif info.get("is_handle_reference"):
                native_name = "n" + name[0].upper() + name[1:]
                target_type = info["type"]
                field_name = info["field_name"]
                native_this = "getNativeObject()" if not method.get("is_static") else ""
                getter_lines.append("    @NonNull")
                getter_lines.append(f"    public {target_type} {name}() {{")
                getter_lines.append(f"        if ({field_name} == null) {{")
                getter_lines.append(f"            long native{target_type} = {native_name}({native_this});")
                getter_lines.append(f'            if (native{target_type} == 0) throw new IllegalStateException("Couldn\'t get {target_type}");')
                getter_lines.append(f"            {field_name} = new {target_type}(native{target_type});")
                getter_lines.append("        }")
                getter_lines.append(f"        return {field_name};")
                getter_lines.append("    }")
            else:
                getter_lines.append("    @Nullable")
                getter_lines.append(f"    public {info['type']} {name}() {{")
                getter_lines.append(f"        return {info['field_name']};")
                getter_lines.append("    }")
            return "\n".join(getter_lines)

        # Step 3A: Material.getDefaultInstance() specialization.
        # Returns cached mDefaultInstance singleton instance associated with this Material.
        if self.name == "Material" and name == "getDefaultInstance":
            sig = self._get_method_sig("getDefaultInstance", [])
            if sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(sig)
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=[])
            lines = []
            if javadoc:
                lines.append(javadoc)
            lines.append("    @NonNull")
            lines.append("    public MaterialInstance getDefaultInstance() {")
            lines.append("        return mDefaultInstance;")
            lines.append("    }")
            return "\n".join(lines)

        # Step 3B: Material.getParameters() reflection specialization.
        # Synthesizes both self-allocating and caller-allocated array populator variants.
        if self.name == "Material" and name == "getParameters":
            sig = self._get_method_sig("getParameters", [])
            if sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(sig)
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=[])
            lines = []
            if javadoc:
                lines.append(javadoc)
            lines.append("    @NonNull")
            lines.append("    public ParameterInfo[] getParameters() {")
            lines.append("        int count = getParameterCount();")
            lines.append("        ParameterInfo[] parameters = new ParameterInfo[count];")
            lines.append("        if (count > 0) {")
            lines.append("            for (int i = 0; i < count; i++) {")
            lines.append("                parameters[i] = new ParameterInfo();")
            lines.append("            }")
            lines.append("            nGetParameters(getNativeObject(), parameters, count);")
            lines.append("        }")
            lines.append("        return parameters;")
            lines.append("    }")
            lines.append("")
            lines.append("    @IntRange(from = 0)")
            lines.append("    public int getParameters(@NonNull ParameterInfo[] parameters, @IntRange(from = 0) int count) {")
            lines.append("        return nGetParameters(getNativeObject(), parameters, count);")
            lines.append("    }")
            return "\n".join(lines)

        # Step 4: Extract return type metadata and assess math return compatibility.
        ret_type_cpp = method["return_type"]
        
        # Check if it is a Math Type Return via resolved info
        ret_info, _ = self.resolve_type_info(ret_type_cpp)
        math_ret_info = ret_info if "assert" in ret_info else None
        
        is_static = method.get("is_static", False) or self.is_utility_class
        method_prefix = "public static " if is_static else "public "

        # Step 5: BufferDescriptor streaming overloads (BufferDescriptor -> java.nio.Buffer).
        # Generates a trio of overloaded methods for direct buffer streaming:
        # - Method 1: Minimal overload forwarding to full method with 0 offset and null callbacks.
        # - Method 2: Offset and count range overload with null callbacks.
        # - Method 3: Full asynchronous overload accepting Handler and Runnable callbacks.
        if self.is_buffer_descriptor_method(method):
            lines = []
            args = method.get("arguments", [])
            buf_idx = -1
            for i, a in enumerate(args):
                t = a["type"]
                q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
                if "BufferDescriptor" in q and "PixelBufferDescriptor" not in q:
                    buf_idx = i
                    break
            pre_args = args[1:buf_idx]
            pre_params = []
            pre_call_args = []
            for a in pre_args:
                p_t = self.get_java_type(a["type"])
                p_ann = self.get_annotation(a["type"])
                p_name = sanitize_identifier(a["name"])
                ann_str = f"{p_ann} " if p_ann else ""
                pre_params.append(f"{ann_str}{p_t} {p_name}")
                pre_call_args.append(p_name)

            pre_p_str = ", ".join(pre_params) + ", " if pre_params else ""
            pre_c_str = ", ".join(pre_call_args) + ", " if pre_call_args else ""
            native_name = f"n{name[0].upper() + name[1:]}"

            doc_params = dict(effective_doc.get("params", {}))
            if "destOffsetInBytes" not in doc_params:
                doc_params["destOffsetInBytes"] = doc_params.get("byteOffset", "offset in *bytes* into the destination buffer")
            if "count" not in doc_params:
                doc_params["count"] = "number of bytes to copy"
            if "buffer" not in doc_params:
                doc_params["buffer"] = "buffer containing the data"
            if "handler" not in doc_params:
                doc_params["handler"] = "handler to dispatch the callback or null for the default handler"
            if "callback" not in doc_params:
                doc_params["callback"] = "runnable called upon completion of the operation"
            buffer_effective_doc = dict(effective_doc)
            buffer_effective_doc["params"] = doc_params

            buf_param_types = {a["name"]: self.get_java_type(a["type"]) for a in pre_args}
            buf_param_types["engine"] = "Engine"
            buf_param_types["buffer"] = "Buffer"
            buf_param_types["destOffsetInBytes"] = "int"
            buf_param_types["count"] = "int"
            buf_param_types["handler"] = "Object"
            buf_param_types["callback"] = "Runnable"

            # Method 1: (engine, ..., buffer) -> forwards with 0, 0, null, null
            m1_doc = generate_javadoc(buffer_effective_doc, indent_spaces=4, argument_names=["engine"] + [a["name"] for a in pre_args] + ["buffer"], param_types=buf_param_types)
            if m1_doc:
                lines.append(m1_doc)
            lines.append(f"    public void {name}(@NonNull Engine engine, {pre_p_str}@NonNull Buffer buffer) {{")
            lines.append(f"        {name}(engine, {pre_c_str}buffer, 0, 0, null, null);")
            lines.append("    }")
            lines.append("")

            # Method 2: (engine, ..., buffer, destOffsetInBytes, count) -> forwards with null, null
            m2_doc = generate_javadoc(buffer_effective_doc, indent_spaces=4, argument_names=["engine"] + [a["name"] for a in pre_args] + ["buffer", "destOffsetInBytes", "count"], param_types=buf_param_types)
            if m2_doc:
                lines.append(m2_doc)
            lines.append(f"    public void {name}(@NonNull Engine engine, {pre_p_str}@NonNull Buffer buffer,")
            lines.append(f"            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count) {{")
            lines.append(f"        {name}(engine, {pre_c_str}buffer, destOffsetInBytes, count, null, null);")
            lines.append("    }")
            lines.append("")

            # Method 3: Full native dispatch with bounds check and asynchronous callback handling
            m3_doc = generate_javadoc(buffer_effective_doc, indent_spaces=4, argument_names=["engine"] + [a["name"] for a in pre_args] + ["buffer", "destOffsetInBytes", "count", "handler", "callback"], param_types=buf_param_types)
            if m3_doc:
                lines.append(m3_doc)
            lines.append(f"    public void {name}(@NonNull Engine engine, {pre_p_str}@NonNull Buffer buffer,")
            lines.append(f"            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count,")
            lines.append(f"            @Nullable Object handler, @Nullable Runnable callback) {{")
            lines.append(f"        int result = {native_name}(getNativeObject(), engine.getNativeObject(), {pre_c_str}buffer, buffer.remaining(),")
            lines.append(f"                destOffsetInBytes, count == 0 ? buffer.remaining() : count, handler, callback);")
            lines.append("        if (result < 0) {")
            lines.append("            throw new BufferOverflowException();")
            lines.append("        }")
            lines.append("    }")
            return "\n".join(lines)

        # Step 6: PixelBufferDescriptor streaming overloads (Texture pixel streaming).
        # Translates texture image transfer calls, unpacking pixel formats, compressed strides,
        # and checking read-only buffer constraints for readPixels operations.
        if self.is_pixel_buffer_descriptor_method(method):
            lines = []
            args = method.get("arguments", [])
            buf_idx = -1
            for i, a in enumerate(args):
                t = a["type"]
                q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
                if "PixelBufferDescriptor" in q:
                    buf_idx = i
                    break
            pre_args = args[:buf_idx]
            pre_params = []
            pre_call_args = []
            for a in pre_args:
                a_t = a["type"]
                type_info, _ = self.resolve_type_info(a_t, silent=True)
                safe_name = sanitize_identifier(a["name"])
                if type_info.get("is_enum"):
                    nullability = a_t.get("nullability", "unspecified")
                    ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                    pre_params.append(f"{ann}{type_info['java']} {safe_name}")
                    if nullability == "nullable":
                        pre_call_args.append(f"{safe_name} != null ? {safe_name}.toFilamentNative() : 0")
                    else:
                        pre_call_args.append(f"{safe_name}.toFilamentNative()")
                elif type_info.get("is_filament_type"):
                    nullability = a_t.get("nullability", "unspecified")
                    if nullability == "nullable":
                        pre_call_args.append(f"{safe_name} != null ? {safe_name}.getNativeObject() : 0")
                        ann = "@Nullable "
                    elif nullability == "nonnull" or a_t.get("is_reference", False):
                        pre_call_args.append(f"{safe_name}.getNativeObject()")
                        ann = "@NonNull "
                    else:
                        pre_call_args.append(f"{safe_name}.getNativeObject()")
                        ann = ""
                    pre_params.append(f"{ann}{type_info['java']} {safe_name}")
                else:
                    j_type = self.get_java_type(a_t)
                    ann = self.get_annotation(a_t)
                    ann_str = f"{ann} " if ann else ""
                    pre_params.append(f"{ann_str}{j_type} {safe_name}")
                    pre_call_args.append(safe_name)

            main_sig = f"{name}({', '.join(pre_params + ['PixelBufferDescriptor buffer'])})"
            if main_sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(main_sig)

            pre_p_str = ", ".join(pre_params) + ", " if pre_params else ""
            pre_c_str = ", ".join(pre_call_args) + ", " if pre_call_args else ""
            native_name = f"n{name[0].upper() + name[1:]}"

            arg_names = [a["name"] for a in pre_args] + ["buffer"]
            m_doc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names)
            if m_doc:
                lines.append(m_doc)
            lines.append(f"    public void {name}({pre_p_str}@NonNull PixelBufferDescriptor buffer) {{")
            is_read = "read" in name.lower()
            if is_read:
                lines.append("        if (buffer.storage.isReadOnly()) {")
                lines.append("            throw new ReadOnlyBufferException();")
                lines.append("        }")
            lines.append("        int type = (buffer.type != null) ? buffer.type.ordinal() : 0;")
            lines.append("        int format = (buffer.type == Texture.Type.COMPRESSED) ?")
            lines.append("                (buffer.compressedFormat != null ? buffer.compressedFormat.ordinal() : 0) :")
            lines.append("                (buffer.format != null ? buffer.format.ordinal() : 0);")
            lines.append("        int stride = (buffer.type == Texture.Type.COMPRESSED) ? buffer.compressedSizeInBytes : buffer.stride;")
            lines.append(f"        {native_name}(getNativeObject(), {pre_c_str}buffer.storage, buffer.storage.remaining(),")
            lines.append("                buffer.left, buffer.top, type, buffer.alignment,")
            lines.append("                stride, format,")
            lines.append("                buffer.handler, buffer.callback);")
            lines.append("    }")
            return "\n".join(lines)

        # Step 7: Tagged Array Scalar Family Overloads (e.g., setParameter(name, Element, float[], ...)).
        # Emits dual overloads:
        # - Full windowed overload: (..., Element type, scalar[] values, int offset, int count)
        # - Convenience overload: (..., Element type, scalar[] values, int count) forwarding with offset = 0
        if m_ctx.get("is_tagged_array"):
            method = m_ctx["ir"]
            family = m_ctx["family"]
            tagged_info = m_ctx["tagged_info"]
            specs = m_ctx["specializations"]
            family_cfg = TAGGED_SCALAR_FAMILIES[family]
            
            name = get_effective_method_name(method)
            cap_family = family_cfg["family"]
            native_name = f"n{name[0].upper() + name[1:]}{cap_family}Array"
            enum_name = family_cfg["enum_name"]
            
            pre_params = []
            pre_call_args = []
            pre_native_args = []
            for a in tagged_info["pre_args"]:
                p_t = self.get_java_type(a["type"])
                p_ann = self.get_annotation(a["type"])
                nullability = a["type"].get("nullability", "unspecified") if isinstance(a["type"], dict) else "unspecified"
                a_info, _ = self.resolve_type_info(a["type"], silent=True)
                if not p_ann:
                    if a_info.get("is_string") or a_info.get("java") == "String":
                        p_ann = "@Nullable" if nullability == "nullable" else "@NonNull"
                    elif nullability == "nonnull":
                        p_ann = "@NonNull"
                    elif nullability == "nullable":
                        p_ann = "@Nullable"
                p_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    if nullability == "nullable":
                        pre_native_args.append(f"{p_name} != null ? {p_name}.getNativeObject() : 0")
                    else:
                        pre_native_args.append(f"{p_name}.getNativeObject()")
                elif a_info.get("is_enum"):
                    pre_native_args.append(f"{p_name}.ordinal()")
                else:
                    pre_native_args.append(p_name)
                ann_str = f"{p_ann} " if p_ann else ""
                pre_params.append(f"{ann_str}{p_t} {p_name}")
                pre_call_args.append(p_name)
            
            values_name = sanitize_identifier(tagged_info["tagged_arg"]["name"])
            count_name = sanitize_identifier(tagged_info["cnt_arg"]["name"])
            scalar_arr = family_cfg["java_array"]
            
            effective_doc = copy.deepcopy(method.get("doc", {}))
            doc_params = effective_doc.setdefault("params", {})
            if "type" not in doc_params:
                doc_params["type"] = "the number of components for each individual parameter"
            if "offset" not in doc_params:
                doc_params["offset"] = f"the number of elements in <code>{values_name}</code> to skip"
                
            lines = []
            
            # Method 1: (..., Element type, scalar[] values, int offset, int count)
            sig1_params = pre_params + [
                f"@NonNull {enum_name} type",
                f"@NonNull {scalar_arr} {values_name}",
                "@IntRange(from = 0) int offset",
                f"@IntRange(from = 1) int {count_name}"
            ]
            sig1 = self._get_method_sig(name, sig1_params)
            if sig1 not in self.emitted_signatures:
                self.emitted_signatures.add(sig1)
                arg_names_1 = [a["name"] for a in tagged_info["pre_args"]] + ["type", values_name, "offset", count_name]
                doc1 = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names_1, param_types={values_name: scalar_arr})
                if doc1:
                    lines.append(doc1)
                call_args_1 = ["getNativeObject()"] + pre_native_args + ["type.ordinal()", values_name, "offset", count_name]
                lines.append(f"    public void {name}({', '.join(sig1_params)}) {{")
                lines.append(f"        {native_name}({', '.join(call_args_1)});")
                lines.append("    }")
                lines.append("")
                
            # Method 2: (..., Element type, scalar[] values, int count) forwarding with offset = 0
            sig2_params = pre_params + [
                f"@NonNull {enum_name} type",
                f"@NonNull {scalar_arr} {values_name}",
                f"@IntRange(from = 1) int {count_name}"
            ]
            sig2 = self._get_method_sig(name, sig2_params)
            if sig2 not in self.emitted_signatures:
                self.emitted_signatures.add(sig2)
                arg_names_2 = [a["name"] for a in tagged_info["pre_args"]] + ["type", values_name, count_name]
                doc2 = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names_2, param_types={values_name: scalar_arr})
                if doc2:
                    lines.append(doc2)
                call_args_2 = pre_call_args + ["type", values_name, "0", count_name]
                lines.append(f"    public void {name}({', '.join(sig2_params)}) {{")
                lines.append(f"        {name}({', '.join(call_args_2)});")
                lines.append("    }")
                
            return "\n".join(lines)

        # Step 8: Packed Buffer Dual-Mode Overloads (NIO Direct Buffer vs primitive float array).
        # Depending on `mode`, synthesizes either:
        # - Direct java.nio.Buffer dispatch with BufferOverflowException checking.
        # - Windowed primitive float[] methods with strict ArrayIndexOutOfBoundsException checks
        #   and cascading zero-offset / full-length forwarders.
        if self.is_packed_buffer_method(method):
            info = self.get_packed_buffer_info(method)
            mode = m_ctx.get("buffer_mode", "buffer")
            name = get_effective_method_name(method)
            native_name = f"n{name[0].upper() + name[1:]}"
            buf_name = info["buf_name"]
            cnt_name = info["cnt_name"]
            has_existing_offset = info["has_existing_offset"]
            array_offset_name = info["array_offset_name"]
            stride = info["stride"]
            java_type = info.get("java_type", "float[]")

            pre_params = []
            pre_call_args = []
            pre_native_args = []
            for a in info["pre_args"]:
                p_t = self.get_java_type(a["type"])
                p_ann = self.get_annotation(a["type"])
                p_name = sanitize_identifier(a["name"])
                a_info, _ = self.resolve_type_info(a["type"], silent=True)
                if a_info.get("is_filament_type"):
                    nullability = a["type"].get("nullability", "unspecified") if isinstance(a["type"], dict) else "unspecified"
                    is_ref = a["type"].get("is_reference", False) if isinstance(a["type"], dict) else False
                    if nullability == "nullable":
                        p_ann = "@Nullable"
                        pre_native_args.append(f"{p_name} != null ? {p_name}.getNativeObject() : 0")
                    else:
                        p_ann = "@NonNull"
                        pre_native_args.append(f"{p_name}.getNativeObject()")
                else:
                    pre_native_args.append(p_name)
                ann_str = f"{p_ann} " if p_ann else ""
                pre_params.append(f"{ann_str}{p_t} {p_name}")
                pre_call_args.append(p_name)

            pre_p_str = ", ".join(pre_params) + ", " if pre_params else ""
            pre_c_str = ", ".join(pre_call_args) + ", " if pre_call_args else ""
            pre_n_str = ", ".join(pre_native_args) + ", " if pre_native_args else ""

            doc_params = dict(effective_doc.get("params", {}))
            doc_params[cnt_name] = f"number of elements (structured element count) in <code>{buf_name}</code>"
            if has_existing_offset:
                doc_params["arrayOffset"] = f"offset in elements (structured element count) in <code>{buf_name}</code> to skip"
                if "offset" not in doc_params:
                    doc_params["offset"] = "offset in elements (structured element count) in the destination buffer or component"
            else:
                doc_params["offset"] = f"offset in elements (structured element count) in <code>{buf_name}</code> to skip"

            for a in info["pre_args"]:
                p_name = sanitize_identifier(a["name"])
                if p_name not in doc_params and a["name"] not in doc_params:
                    if a["name"] == "instance":
                        doc_params[p_name] = "instance of the component obtained from getInstance()"
                    elif a["name"] == "engine":
                        doc_params[p_name] = "the {@link Engine} instance"
                    elif a["name"] == "entity":
                        doc_params[p_name] = "the entity"
                    else:
                        doc_params[p_name] = f"the {a['name']}"

            if buf_name not in doc_params:
                if mode == "buffer":
                    doc_params[buf_name] = f"buffer containing {buf_name} data"
                else:
                    doc_params[buf_name] = f"array containing {buf_name} data"

            tailored_doc = dict(effective_doc)
            tailored_doc["params"] = doc_params

            buf_param_types = {buf_name: "Buffer"}
            arr_param_types = {buf_name: java_type}

            lines = []
            pre_arg_names = [a["name"] for a in info["pre_args"]]

            if mode == "buffer":
                if has_existing_offset:
                    # Method 1: (..., Buffer {buf_name}, int {cnt_name}, int offset)
                    sig1 = self._get_method_sig(name, pre_params + [f"Buffer {buf_name}", f"int {cnt_name}", "int offset"])
                    if sig1 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig1)
                        m1_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=True)
                        m1_doc = generate_javadoc(m1_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, cnt_name, "offset"], param_types=buf_param_types)
                        if m1_doc:
                            lines.append(m1_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull Buffer {buf_name}, @IntRange(from = 0) int {cnt_name}, @IntRange(from = 0) int offset) {{")
                        lines.append(f"        int result = {native_name}(getNativeObject(), {pre_n_str}{buf_name}, {buf_name}.remaining(), {cnt_name}, offset);")
                        lines.append("        if (result < 0) {")
                        lines.append("            throw new BufferOverflowException();")
                        lines.append("        }")
                        lines.append("    }")
                        lines.append("")

                    # Method 2: (..., Buffer {buf_name}, int {cnt_name}) -> (..., {buf_name}, {cnt_name}, 0)
                    sig2 = self._get_method_sig(name, pre_params + [f"Buffer {buf_name}", f"int {cnt_name}"])
                    if sig2 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig2)
                        m2_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                        m2_doc = generate_javadoc(m2_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, cnt_name], param_types=buf_param_types)
                        if m2_doc:
                            lines.append(m2_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull Buffer {buf_name}, @IntRange(from = 0) int {cnt_name}) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, {cnt_name}, 0);")
                        lines.append("    }")
                else:
                    # Method 1 (no existing offset): (..., int {cnt_name}, Buffer {buf_name})
                    sig1 = self._get_method_sig(name, pre_params + [f"int {cnt_name}", f"Buffer {buf_name}"])
                    if sig1 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig1)
                        m1_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                        m1_doc = generate_javadoc(m1_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [cnt_name, buf_name], param_types=buf_param_types)
                        if m1_doc:
                            lines.append(m1_doc)
                        lines.append(f"    public void {name}({pre_p_str}@IntRange(from = 0) int {cnt_name}, @NonNull Buffer {buf_name}) {{")
                        lines.append(f"        int result = {native_name}(getNativeObject(), {pre_n_str}{buf_name}, {buf_name}.remaining(), {cnt_name});")
                        lines.append("        if (result < 0) {")
                        lines.append("            throw new BufferOverflowException();")
                        lines.append("        }")
                        lines.append("    }")
                return "\n".join(lines)

            else: # mode == "array"
                if has_existing_offset:
                    # Method 3 (windowed): (..., {java_type} {buf_name}, int arrayOffset, int {cnt_name}, int offset)
                    sig3 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}", "int arrayOffset", f"int {cnt_name}", "int offset"])
                    if sig3 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig3)
                        m3_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=True)
                        m3_doc = generate_javadoc(m3_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, "arrayOffset", cnt_name, "offset"], param_types=arr_param_types)
                        if m3_doc:
                            lines.append(m3_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int arrayOffset, @IntRange(from = 0) int {cnt_name}, @IntRange(from = 0) int offset) {{")
                        lines.append(f"        if ({buf_name}.length < (arrayOffset + {cnt_name}) * {stride}) {{")
                        lines.append(f"            throw new ArrayIndexOutOfBoundsException(\"Array length must be at least \" + ((arrayOffset + {cnt_name}) * {stride}));")
                        lines.append("        }")
                        lines.append(f"        {native_name}(getNativeObject(), {pre_n_str}{buf_name}, {cnt_name}, offset, arrayOffset);")
                        lines.append("    }")
                        lines.append("")

                    # Method 4: (..., {java_type} {buf_name}, int {cnt_name}, int offset) -> forwards to arrayOffset = 0
                    sig4 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}", f"int {cnt_name}", "int offset"])
                    if sig4 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig4)
                        m4_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=True)
                        m4_doc = generate_javadoc(m4_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, cnt_name, "offset"], param_types=arr_param_types)
                        if m4_doc:
                            lines.append(m4_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int {cnt_name}, @IntRange(from = 0) int offset) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, 0, {cnt_name}, offset);")
                        lines.append("    }")
                        lines.append("")

                    # Method 5: (..., {java_type} {buf_name}, int {cnt_name}) -> forwards to arrayOffset = 0, offset = 0
                    sig5 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}", f"int {cnt_name}"])
                    if sig5 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig5)
                        m5_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                        m5_doc = generate_javadoc(m5_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, cnt_name], param_types=arr_param_types)
                        if m5_doc:
                            lines.append(m5_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int {cnt_name}) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, 0, {cnt_name}, 0);")
                        lines.append("    }")
                        lines.append("")

                    # Method 6: (..., {java_type} {buf_name}) -> forwards to arrayOffset = 0, count = length / stride, offset = 0
                    sig6 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}"])
                    if sig6 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig6)
                        m6_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, f"{buf_name}.length / {stride}", has_offset_arg=False)
                        m6_doc = generate_javadoc(m6_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name], param_types=arr_param_types)
                        if m6_doc:
                            lines.append(m6_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, 0, {buf_name}.length / {stride}, 0);")
                        lines.append("    }")
                else:
                    # Method 3 (windowed, no existing offset): (..., {java_type} {buf_name}, int offset, int {cnt_name})
                    sig3 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}", "int offset", f"int {cnt_name}"])
                    if sig3 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig3)
                        m3_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                        m3_doc = generate_javadoc(m3_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, "offset", cnt_name], param_types=arr_param_types)
                        if m3_doc:
                            lines.append(m3_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int offset, @IntRange(from = 0) int {cnt_name}) {{")
                        lines.append(f"        if ({buf_name}.length < (offset + {cnt_name}) * {stride}) {{")
                        lines.append(f"            throw new ArrayIndexOutOfBoundsException(\"Array length must be at least \" + ((offset + {cnt_name}) * {stride}));")
                        lines.append("        }")
                        lines.append(f"        {native_name}(getNativeObject(), {pre_n_str}{buf_name}, {cnt_name}, offset);")
                        lines.append("    }")
                        lines.append("")

                    # Method 4: (..., {java_type} {buf_name}, int {cnt_name}) -> forwards to offset = 0
                    sig4 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}", f"int {cnt_name}"])
                    if sig4 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig4)
                        m4_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                        m4_doc = generate_javadoc(m4_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name, cnt_name], param_types=arr_param_types)
                        if m4_doc:
                            lines.append(m4_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int {cnt_name}) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, 0, {cnt_name});")
                        lines.append("    }")
                        lines.append("")

                    # Method 5: (..., {java_type} {buf_name}) -> forwards to offset = 0, count = length / stride
                    sig5 = self._get_method_sig(name, pre_params + [f"{java_type} {buf_name}"])
                    if sig5 not in self.emitted_signatures:
                        self.emitted_signatures.add(sig5)
                        m5_doc_dict = self._adapt_packed_doc(tailored_doc, buf_name, f"{buf_name}.length / {stride}", has_offset_arg=False)
                        m5_doc = generate_javadoc(m5_doc_dict, indent_spaces=4, argument_names=pre_arg_names + [buf_name], param_types=arr_param_types)
                        if m5_doc:
                            lines.append(m5_doc)
                        lines.append(f"    public void {name}({pre_p_str}@NonNull @Size(min = {stride}) {java_type} {buf_name}) {{")
                        lines.append(f"        {name}({pre_c_str}{buf_name}, 0, {buf_name}.length / {stride});")
                        lines.append("    }")
                return "\n".join(lines)

        # Step 9: Value Class Direct Buffer Backing Getter (e.g., Frustum.getPlanes()).
        # Synthesizes caller-allocated array populator using System.arraycopy and a zero-arg allocator.
        if self.is_value_class and name == self.value_type_info.get("buffer_getter"):
            size = self.value_type_info.get("size", 24)
            field_name = self.value_type_info.get("field_name", "mPlanes")
            elem_type = self.value_type_info.get("element_type", "float")
            doc_params = dict(effective_doc.get("params", {}))
            if "out" not in doc_params:
                doc_params["out"] = f"pre-allocated array to receive the data, or null."
            out_doc = dict(effective_doc)
            out_doc["params"] = doc_params
            javadoc = generate_javadoc(out_doc, indent_spaces=4, argument_names=["out"], param_types={"out": f"{elem_type}[]"})
            lines = []
            if javadoc:
                lines.append(javadoc)
            lines.append(f"    @NonNull @Size(min = {size})")
            lines.append(f"    public {elem_type}[] {name}(@Nullable @Size(min = {size}) {elem_type}[] out) {{")
            lines.append("        if (out == null) {")
            lines.append(f"            out = new {elem_type}[{size}];")
            lines.append("        }")
            lines.append(f"        System.arraycopy({field_name}, 0, out, 0, {size});")
            lines.append("        return out;")
            lines.append("    }")
            lines.append("")
            lines.append(f"    @NonNull @Size(min = {size})")
            lines.append(f"    public {elem_type}[] {name}() {{")
            lines.append(f"        return {name}(null);")
            lines.append("    }")
            return "\n".join(lines)

        # Step 10: Value Object Return Overloads (e.g., Camera.getFrustum(out)).
        # Populates the underlying backing array of the provided value object, returning it.
        if ret_info.get("is_value_object"):
            ret_type = ret_info["java"]
            doc_params = dict(effective_doc.get("params", {}))
            if "out" not in doc_params:
                doc_params["out"] = f"pre-allocated {ret_type} to receive the data, or null."
            out_doc = dict(effective_doc)
            out_doc["params"] = doc_params
            javadoc = generate_javadoc(out_doc, indent_spaces=4, argument_names=["out"], param_types={"out": ret_type})
            lines = []
            if javadoc:
                lines.append(javadoc)
            lines.append("    @NonNull")
            lines.append(f"    public {ret_type} {name}(@Nullable {ret_type} out) {{")
            lines.append("        if (out == null) {")
            lines.append(f"            out = new {ret_type}();")
            lines.append("        }")
            call_obj = "getNativeObject()" if not is_static else ""
            native_call_args = [call_obj] if call_obj else []
            native_call_args.append(f"out.{ret_info.get('field_name', 'mPlanes')}")
            lines.append(f"        n{name[0].upper() + name[1:]}({', '.join(native_call_args)});")
            lines.append("        return out;")
            lines.append("    }")
            lines.append("")
            lines.append("    @NonNull")
            lines.append(f"    public {ret_type} {name}() {{")
            lines.append(f"        return {name}(null);")
            lines.append("    }")
            return "\n".join(lines)

        # Step 11: Asynchronous Callback & Handler Overloads (e.g., fence, async buffer releases).
        # Marshals platform CallbackHandler (Object) and std::function / Invocable (Runnable)
        # into native JNI execution blocks.
        if m_ctx.get("is_async_callback"):
            name = get_effective_method_name(method)
            native_name = f"n{name[0].upper() + name[1:]}"
            params_list = []
            call_args = []
            arg_names = []
            async_param_types = {}
            for arg in method.get("arguments", []):
                arg_name = sanitize_identifier(arg["name"])
                t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "")
                if "CallbackHandler" in t_str:
                    params_list.append(f"@Nullable Object {arg_name}")
                    call_args.append(arg_name)
                    arg_names.append(arg_name)
                    async_param_types[arg_name] = "Object"
                elif any(cb in t_str for cb in ("Invocable", "std::function", "Callback")):
                    params_list.append(f"@Nullable Runnable {arg_name}")
                    call_args.append(arg_name)
                    arg_names.append(arg_name)
                    async_param_types[arg_name] = "Runnable"
                else:
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    j_type = self.get_java_type(arg["type"])
                    ann = self.get_annotation(arg["type"])
                    if t_info.get("is_enum"):
                        ann = "@NonNull "
                        call_method = "ordinal()" if ("." in t_info['java'] and t_info['java'].split(".")[0] != self.name) else "toFilamentNative()"
                        call_args.append(f"{arg_name}.{call_method}")
                    elif t_info.get("is_filament_type"):
                        ann = "@NonNull "
                        call_args.append(f"{arg_name}.getNativeObject()")
                    elif t_info.get("archetype") == "bitfield":
                        ann = "@NonNull "
                        field_name = t_info.get("field_name", "mSampler")
                        call_args.append(f"{arg_name}.{field_name}")
                    else:
                        ann = f"{ann} " if ann else ""
                        call_args.append(arg_name)
                    params_list.append(f"{ann}{j_type} {arg_name}".strip())
                    arg_names.append(arg_name)
                    async_param_types[arg_name] = j_type

            main_sig = self._get_method_sig(name, params_list)
            if main_sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(main_sig)

            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=async_param_types)
            out = []
            if javadoc:
                out.append(javadoc)
            out.append(f"    public void {name}({', '.join(params_list)}) {{")
            out.append(f"        {native_name}(getNativeObject(), {', '.join(call_args)});")
            out.append("    }")

            # Step 11B: Convenience overloads for trailing arguments with default values
            # (e.g. setFrameScheduledCallback(handler, callback) defaulting flags = 0)
            callback_idx = -1
            c_args = method.get("arguments", [])
            for idx, arg in enumerate(c_args):
                t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "")
                if any(cb in t_str for cb in ("Invocable", "std::function", "Callback")):
                    callback_idx = idx

            if callback_idx != -1 and callback_idx < len(c_args) - 1:
                trailing_defaults = 0
                for arg in reversed(c_args[callback_idx + 1:]):
                    if arg.get("default_value") is not None:
                        trailing_defaults += 1
                    else:
                        break

                if trailing_defaults > 0:
                    for plen in range(len(c_args) - trailing_defaults, len(c_args)):
                        p_params = params_list[:plen]
                        p_names = arg_names[:plen]
                        f_call = list(arg_names[:plen])
                        for o_arg in c_args[plen:]:
                            def_raw = o_arg.get("default_value")
                            def_val = self.translate_default_value(def_raw, o_arg["type"])
                            if def_val is None:
                                def_val = def_raw if def_raw else "0"
                            f_call.append(def_val)

                        conv_sig = self._get_method_sig(name, p_params)
                        if conv_sig not in self.emitted_signatures:
                            self.emitted_signatures.add(conv_sig)
                            out.append("")
                            conv_doc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=p_names, param_types=async_param_types)
                            if conv_doc:
                                out.append(conv_doc)
                            out.append(f"    public void {name}({', '.join(p_params)}) {{")
                            out.append(f"        {name}({', '.join(f_call)});")
                            out.append("    }")

            return "\n".join(out)

        # Step 12A: Target Pointer Resolution & Receiver Marshalling.
        # Determines native dispatch arguments: instance pointer (`getNativeObject()`),
        # archetype backing field (`mPlanes`, `mSampler`), or flattened POD leaves.
        params_list = []
        if is_static or method.get("is_constructor"):
            call_args = []
        elif self.archetype == "inline_buffer":
            call_args = [self.value_type_info.get("field_name", "mPlanes")]
        elif self.archetype == "bitfield":
            call_args = [self.value_type_info.get("field_name", "mSampler")]
        elif self.is_aggregate:
            call_args = []
            for l in self.get_flattened_leaves(self.name):
                if l.get("is_math") or l.get("is_fixed_array") or l.get("is_slice"):
                    call_args.append(f"m{l['name']}")
                else:
                    call_args.append(l["param_name"])
        else:
            call_args = ["getNativeObject()"]
        arg_names = []
        
        # Validation logic to prepend to body
        validation_code = []

        # Step 12B: Cached Setter State Tracking.
        # Prepend field assignment if this method acts as a cached property setter.
        if name in [c["setter"] for c in self.cached_fields.values()]:
            info = next(c for c in self.cached_fields.values() if c["setter"] == name)
            validation_code.insert(0, f"{info['field_name']} = {info['arg_name']};")

        # Step 12C: Index Size Parameter Consumers.
        # Map: count_arg_name -> list of (ptr_arg_name, stride, is_nullable)
        size_param_consumers = {}
        for arg in method.get("arguments", []):
            cnt_name = get_size_param_attr(arg)
            if cnt_name:
                arr_in = self.get_array_input_info(arg["type"])
                if arr_in is not None:
                    stride = arr_in["size"] if arr_in else 1
                    nullability = arg["type"].get("nullability", "unspecified")
                    is_nullable = (nullability == "nullable")
                    size_param_consumers.setdefault(cnt_name, []).append((arg["name"], stride, is_nullable))

        # Step 12D: Parameter Iteration & Marshalling.
        method_param_types = {}
        for arg in method.get("arguments", []):
            arg_name = arg["name"]
            req_type = arg["type"]
            
            # If method returns a container, drop the size/count argument because it is inferred from the out-array
            if ret_info.get("is_container") and not math_ret_info and arg_name in ("historySize", "size", "count", "capacity", "maxCount"):
                continue

            # Check if this arg is a count parameter consumed by a single array
            if arg_name in size_param_consumers and len(size_param_consumers[arg_name]) == 1:
                ptr_name, stride, is_nullable = size_param_consumers[arg_name][0]
                if stride == 1:
                    cnt_expr = f"{ptr_name} != null ? {ptr_name}.length : 0" if is_nullable else f"{ptr_name}.length"
                else:
                    cnt_expr = f"{ptr_name} != null ? {ptr_name}.length / {stride} : 0" if is_nullable else f"{ptr_name}.length / {stride}"
                call_args.append(cnt_expr)
                continue

            # Check for size_param on this arg
            size_param_name = get_size_param_attr(arg)
            
            arr_in = self.get_array_input_info(req_type)
            if arr_in:
                # Array or Vector Input
                req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = arg["type"].get("is_pointer", False)
                is_slice = arr_in.get("is_slice", False)

                if not is_matrix and not is_pointer and not size_param_name and not is_slice:
                    # Unroll Vector (e.g. float3 -> vx, vy, vz)
                    scalar = arr_in["scalar"]
                    size = arr_in["size"]
                    components = ["x", "y", "z", "w"][:size]
                    comp_names = [sanitize_identifier(f"{arg_name}{c}") for c in components]
                    
                    original_desc = effective_doc.get("params", {}).get(arg_name, "")
                    
                    for comp, comp_name in zip(components, comp_names):
                        params_list.append(f"{scalar} {comp_name}")
                        call_args.append(comp_name)
                        arg_names.append(comp_name)
                        method_param_types[comp_name] = scalar
                        
                        if original_desc:
                            new_desc = f"({comp} component) {original_desc}"
                        else:
                            new_desc = f"({comp} component)"
                        
                        if "params" not in effective_doc:
                            effective_doc["params"] = {}
                        effective_doc["params"][comp_name] = new_desc
                else:
                    # Array parameter
                    j_type = arr_in["java"]
                    size = arr_in["size"]
                    
                    nullability = arg["type"].get("nullability", "unspecified")
                    is_nullable = False
                    
                    if arr_in.get("is_custom_array"):
                        ann_prefix = f"{arr_in['annotation']} " if arr_in.get("annotation") else ""
                        if is_slice:
                            ann = f"@NonNull {ann_prefix}".strip()
                        elif nullability == "nonnull":
                            ann = f"@NonNull {ann_prefix}".strip()
                        elif nullability == "nullable":
                            ann = f"@Nullable {ann_prefix}".strip()
                            is_nullable = True
                        else:
                            ann = ann_prefix.strip()
                    else:
                        ann = f"@NonNull @Size(min = {size})"
                        if is_pointer:
                            if nullability == "nullable":
                                ann = f"@Nullable @Size(min = {size})"
                                is_nullable = True
                            elif nullability == "unspecified":
                                ann = f"@Size(min = {size})"
                                is_nullable = True

                    safe_name = sanitize_identifier(arg_name)
                    p_str = f"{ann} {j_type} {safe_name}".strip()
                    params_list.append(p_str)
                    call_args.append(safe_name)
                    if is_slice:
                        stride = arr_in.get("size", 1)
                        if stride > 1:
                            cnt_expr = f"{safe_name} != null ? {safe_name}.length / {stride} : 0" if is_nullable else f"{safe_name}.length / {stride}"
                        else:
                            cnt_expr = f"{safe_name} != null ? {safe_name}.length : 0" if is_nullable else f"{safe_name}.length"
                        call_args.append(cnt_expr)
                    arg_names.append(safe_name)
                    method_param_types[safe_name] = j_type
                    
                    # Validation Generation
                    if size_param_name and (size_param_name not in size_param_consumers or len(size_param_consumers[size_param_name]) > 1):
                        mult_str = f"{size} * " if size > 1 else ""
                        check = f"if ({safe_name}.length < {mult_str}{size_param_name}) {{\n"
                        check += f"            throw new ArrayIndexOutOfBoundsException(\"Array length must be at least {mult_str}{size_param_name}\");\n"
                        check += "        }"
                        
                        if is_nullable:
                             check = f"if ({safe_name} != null) {{\n            {check.strip()}\n        }}"
                        
                        validation_code.append(check)

            else:
                safe_name = sanitize_identifier(arg_name)
                arg_names.append(safe_name)
                req_type = arg["type"]
                
                inv_info = self.get_invocable_info(req_type, name)
                if inv_info:
                    params_list.append(f"@NonNull {inv_info['interface_name']} {safe_name}")
                    call_args.append(safe_name)
                    method_param_types[safe_name] = inv_info['interface_name']
                    continue

                type_info, _ = self.resolve_type_info(req_type)
                if type_info.get("is_pojo_struct"):
                    struct_cls = type_info["struct_cls"]
                    params_list.append(f"@NonNull {type_info['java']} {safe_name}")
                    method_param_types[safe_name] = type_info['java']
                    leaves = self.get_pojo_leaves(struct_cls, var_name=safe_name)
                    for leaf in leaves:
                        call_args.append(leaf["java_expr"])
                    continue

                if type_info.get("is_struct"):
                    struct_cls = type_info["struct_cls"]
                    params_list.append(f"@NonNull {type_info['java']} {safe_name}")
                    method_param_types[safe_name] = type_info['java']
                    leaves = self.get_flattened_leaves(struct_cls)
                    struct_slice_leaves = [l for l in leaves if l.get("is_struct_slice")]
                    for sl in struct_slice_leaves:
                        slice_info = sl.get("slice_info", {})
                        inner_info = slice_info.get("inner_info", {})
                        inner_java_name = inner_info.get("java", "Object")
                        inner_cls = inner_info.get("struct_cls") or self.is_aggregate_struct(inner_info.get("cpp_type"))
                        inner_leaves = self.get_flattened_leaves(inner_cls) if inner_cls else []
                        stride = max(1, len(inner_leaves))
                        sf_name = sl["name"][0].lower() + sl["name"][1:]
                        sf_cap = sl["name"]
                        validation_code.append(f"long[] raw{sf_cap} = null;")
                        validation_code.append(f"int {sf_name}Count = 0;")
                        getter_chain = ".".join([safe_name] + sl.get("getter_chain", [f"get{sf_cap}()"]))
                        validation_code.append(f"{inner_java_name}[] {sf_name} = {getter_chain};")
                        validation_code.append(f"if ({sf_name} != null && {sf_name}.length > 0) {{")
                        validation_code.append(f"    {sf_name}Count = {sf_name}.length;")
                        stride_mult = f" * {stride}" if stride > 1 else ""
                        validation_code.append(f"    raw{sf_cap} = new long[{sf_name}Count{stride_mult}];")
                        validation_code.append(f"    for (int i = 0; i < {sf_name}Count; ++i) {{")
                        validation_code.append(f"        {inner_java_name} item = {sf_name}[i];")
                        validation_code.append(f"        if (item != null) {{")
                        if inner_leaves:
                            for idx, ileaf in enumerate(inner_leaves):
                                leaf_getter = ".".join(["item"] + ileaf.get("getter_chain", [f"get{ileaf['name']}()"]))
                                offset_str = f"i * {stride} + {idx}" if idx > 0 else (f"i * {stride}" if stride > 1 else "i")
                                validation_code.append(f"            raw{sf_cap}[{offset_str}] = {leaf_getter};")
                        else:
                            validation_code.append(f"            raw{sf_cap}[i] = item.getNativeObject();")
                        validation_code.append(f"        }}")
                        validation_code.append(f"    }}")
                        validation_code.append(f"}}")

                    for leaf in leaves:
                        if leaf.get("is_struct_slice"):
                            sf_cap = leaf["name"]
                            call_args.append(f"raw{sf_cap}")
                        elif leaf.get("is_slice_count"):
                            base_name = leaf["name"][:-5] if leaf["name"].endswith("Count") else leaf["name"]
                            sf_name = base_name[0].lower() + base_name[1:]
                            call_args.append(f"{sf_name}Count")
                        else:
                            getter_call = ".".join([safe_name] + leaf.get("getter_chain", [f"get{leaf['name']}()"]))
                            call_args.append(getter_call)
                    continue

                if type_info.get("is_string") or type_info.get("java") == "String":
                    nullability = arg["type"].get("nullability", "unspecified") if isinstance(arg["type"], dict) else "unspecified"
                    ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                    params_list.append(f"{ann}String {safe_name}")
                    call_args.append(safe_name)
                    method_param_types[safe_name] = "String"
                    continue

                if type_info.get("is_enum"):
                    nullability = arg["type"].get("nullability", "unspecified")
                    ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                    j_type = self.get_java_type(req_type)
                    params_list.append(f"{ann}{j_type} {safe_name}")
                    call_method = "ordinal()" if ("." in type_info['java'] and type_info['java'].split(".")[0] != self.name) else "toFilamentNative()"
                    if nullability == "nullable":
                        call_args.append(f"{safe_name} != null ? {safe_name}.{call_method} : 0")
                    else:
                        call_args.append(f"{safe_name}.{call_method}")
                    method_param_types[safe_name] = j_type
                    continue

                j_type = self.get_java_type(req_type)
                ann = self.get_annotation(req_type)
                
                # Check for Filament Type
                if type_info.get("is_filament_type"):
                    nullability = arg["type"].get("nullability", "unspecified")
                    if nullability == "nullable":
                        call_args.append(f"{safe_name} != null ? {safe_name}.getNativeObject() : 0")
                        ann = "@Nullable"
                    elif nullability == "nonnull" or arg["type"].get("is_reference", False):
                        call_args.append(f"{safe_name}.getNativeObject()")
                        ann = "@NonNull"
                    else:
                        call_args.append(f"{safe_name}.getNativeObject()")
                        ann = None
                elif type_info.get("archetype") == "bitfield":
                    nullability = arg["type"].get("nullability", "unspecified")
                    field_name = type_info.get("field_name", "mSampler")
                    if nullability == "nullable":
                        call_args.append(f"{safe_name} != null ? {safe_name}.{field_name} : 0")
                        ann = "@Nullable"
                    elif nullability == "nonnull" or arg["type"].get("is_reference", False):
                        call_args.append(f"{safe_name}.{field_name}")
                        ann = "@NonNull"
                    else:
                        call_args.append(f"{safe_name} != null ? {safe_name}.{field_name} : 0")
                        ann = None
                else:
                    call_args.append(safe_name)
                    
                p_str = ""
                if ann:
                    p_str += f"{ann} "
                p_str += f"{j_type} {safe_name}"
                
                params_list.append(p_str)
                method_param_types[safe_name] = j_type

        out = []

        # Step 13: Constructor Method Emission & Hierarchy Chaining.
        # Synthesizes constructors for bitfield archetypes, inline buffer archetypes,
        # or subclasses extending native-backed base classes.
        if method.get("is_constructor"):
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
            if javadoc:
                out.append(javadoc)
            signature = f"public {self.name}({', '.join(params_list)})"
            out.append(f"    {signature} {{")
            if self.archetype == "bitfield":
                clean_types = "".join(self._get_clean_type_name(a["type"]) for a in method.get("arguments", []))
                native_name = f"nCreate{self.name}{clean_types}" if clean_types else f"nCreate{self.name}"
                field_name = self.value_type_info.get("field_name", "mSampler")
                call_str = f"{native_name}({', '.join(call_args)})" if call_args else f"{native_name}()"
                out.append(f"        {field_name} = {call_str};")
            elif self.archetype == "inline_buffer":
                c_args = method.get("arguments", [])
                if len(c_args) == 0:
                    pass
                else:
                    setter_match = None
                    for m in self.methods:
                        if m["name"].startswith("set") and not m.get("is_constructor"):
                            m_args = m.get("arguments", [])
                            if len(m_args) == len(c_args):
                                if all(self.get_java_type(a1["type"]) == self.get_java_type(a2["type"]) for a1, a2 in zip(m_args, c_args)):
                                    setter_match = m["name"]
                                    break
                    if setter_match:
                        out.append(f"        {setter_match}({', '.join(arg_names)});")
                    else:
                        clean_types = "".join(self._get_clean_type_name(a["type"]) for a in c_args)
                        native_name = f"nInit{self.name}{clean_types}"
                        field_name = self.value_type_info.get("field_name", "mPlanes")
                        out.append(f"        {native_name}({field_name}, {', '.join(call_args)});")
            elif self.base_class:
                clean_types = "".join(self._get_clean_type_name(a["type"]) for a in method.get("arguments", []))
                native_name = f"nCreate{self.name}{clean_types}" if clean_types else f"nCreate{self.name}"
                call_str = f"{native_name}({', '.join(call_args)})" if call_args else f"{native_name}()"
                out.append(f"        super({call_str});")
            out.append("    }")
            convenience_methods = self._generate_convenience_overloads(
                method, effective_doc, "", "void", None, None, size_param_consumers
            )
            return "\n\n".join(convenience_methods + ["\n".join(out)])

        # Step 14: Slice Return Logic (e.g., utils::Slice<const Entity> -> int[]).
        # Emits dual overloads:
        # - Primary method accepting pre-allocated array `@Nullable int[] out` to avoid GC churn.
        # - Convenience overload with no `out` argument forwarding `null` to allocate dynamically.
        if ret_info.get("is_slice") and not ret_info.get("is_struct_slice"):
            out_param_name = "out"
            java_elem_type = ret_info.get("java_elem_type", "int")
            elem_ann = ""
            if java_elem_type.startswith("@Entity "):
                elem_ann = "@Entity "
                clean_elem = "int"
            else:
                clean_elem = java_elem_type
            
            out_arg_def = f"@Nullable {elem_ann}{clean_elem}[] {out_param_name}"
            params_list.append(out_arg_def)
            call_args.append(out_param_name)
            arg_names.append(out_param_name)
            
            # Clean effective doc params if size arg was documented
            if "params" in effective_doc:
                for s_key in ("historySize", "size", "count", "capacity", "maxCount"):
                    effective_doc["params"].pop(s_key, None)
            effective_doc.setdefault("params", {})[out_param_name] = f"pre-allocated array to receive the items, or null."
            if "return" in effective_doc:
                effective_doc["return"] = f"An array containing the items, or null on error."

            method_param_types[out_param_name] = f"{clean_elem}[]"
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
            if javadoc:
                out.append(javadoc)
            
            ret_type_str = f"{elem_ann}{clean_elem}[]"
            out.append("    @NonNull")
            signature = f"{method_prefix}{ret_type_str} {name}({', '.join(params_list)})"
            main_sig = self._get_method_sig(name, params_list)
            if main_sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(main_sig)
            
            out.append(f"    {signature} {{")
            for code in validation_code:
                out.append(f"        {code}")
            
            native_name = "n" + name[0].upper() + name[1:]
            out.append(f"        return {native_name}({', '.join(call_args)});")
            out.append("    }")
            
            # Convenience method with no out param
            c_args_before = [p for p in params_list if p != out_arg_def]
            c_call_before = [re.sub(r"^.*?(\w+)$", r"\1", p.strip()) for p in c_args_before] + ["null"]
            see_types = []
            for p in params_list:
                p_clean = re.sub(r"@[A-Za-z0-9_]+(?:\([^)]*\))?\s*", "", p).strip()
                p_type = p_clean.split()[0]
                see_types.append(p_type)
            see_doc = generate_javadoc({"see": f"#{name}({', '.join(see_types)})"}, indent_spaces=4)
            
            c_lines = []
            if see_doc:
                c_lines.append(see_doc)
            c_lines.append("    @NonNull")
            c_lines.append(f"    {method_prefix}{ret_type_str} {name}({', '.join(c_args_before)}) {{")
            c_lines.append(f"        return {name}({', '.join(c_call_before)});")
            c_lines.append("    }")
            
            return "\n\n".join(["\n".join(out), "\n".join(c_lines)])

        # Step 15: Container Return Logic (FixedCapacityVector, std::vector, std::array).
        # Populates a caller-provided destination array and returns the number of elements written.
        if ret_info.get("is_container") and not math_ret_info:
            out_param_name = "outHistory" if "History" in name else "out"
            java_elem_type = ret_info.get("java_elem_type", "Object")
            
            # Additional 'out' arg
            out_arg_def = f"@NonNull {java_elem_type}[] {out_param_name}"
            params_list.append(out_arg_def)
            call_args.append(out_param_name)
            arg_names.append(out_param_name)
            
            # Clean effective doc params if size arg was documented
            if "params" in effective_doc:
                for s_key in ("historySize", "size", "count", "capacity", "maxCount"):
                    effective_doc["params"].pop(s_key, None)
                effective_doc["params"][out_param_name] = f"pre-allocated array of {java_elem_type}."
            if "return" in effective_doc:
                effective_doc["return"] = f"the number of {java_elem_type} populated."

            method_param_types[out_param_name] = f"{java_elem_type}[]"
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
            if javadoc:
                out.append(javadoc)
            
            signature = f"{method_prefix}int {name}({', '.join(params_list)})"
            main_sig = self._get_method_sig(name, params_list)
            if main_sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(main_sig)
            
            out.append(f"    {signature} {{")
            for code in validation_code:
                out.append(f"        {code}")
            native_name = "n" + name[0].upper() + name[1:]
            out.append(f"        return {native_name}({', '.join(call_args)});")
            out.append("    }")
            return "\n".join(out)

        # Step 16: Aggregate POD Struct Return Logic (e.g., Box, Aabb).
        # Emits dual overloads:
        # - Primary method accepting `@Nullable Struct out`, lazily instantiating if null.
        # - Zero-argument convenience method forwarding `null` to allocate.
        if ret_info.get("is_struct") and not self.is_aggregate:
            struct_cls = ret_info.get("struct_cls") or self.is_aggregate_struct(ret_info.get("cpp_type"))
            struct_name = struct_cls["name"] if struct_cls else ret_info["java"]
            out_param_name = "out"
            out_arg_def = f"@Nullable {struct_name} {out_param_name}"
            params_list.append(out_arg_def)
            call_args.append(out_param_name)
            arg_names.append(out_param_name)

            if "params" in effective_doc:
                effective_doc.setdefault("params", {})[out_param_name] = f"pre-allocated {struct_name} to receive the data, or null."
            if "return" in effective_doc:
                effective_doc["return"] = f"{out_param_name}, or a newly allocated {struct_name} if {out_param_name} was null."

            method_param_types[out_param_name] = struct_name
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
            if javadoc:
                out.append(javadoc)

            signature = f"{method_prefix}{struct_name} {name}({', '.join(params_list)})"
            main_sig = self._get_method_sig(name, params_list)
            if main_sig in self.emitted_signatures:
                return None
            self.emitted_signatures.add(main_sig)

            out.append("    @NonNull")
            out.append(f"    {signature} {{")
            for code in validation_code:
                out.append(f"        {code}")

            out.append(f"        if ({out_param_name} == null) {{")
            out.append(f"            {out_param_name} = new {struct_name}();")
            out.append(f"        }}")

            native_name = "n" + name[0].upper() + name[1:]
            out.append(f"        {native_name}({', '.join(call_args)});")
            out.append(f"        return {out_param_name};")
            out.append("    }")

            # Convenience method with no out param
            c_args_before = [p for p in params_list if p != out_arg_def]
            c_call_before = [re.sub(r"^.*?(\w+)$", r"\1", p.strip()) for p in c_args_before] + ["null"]
            see_types = []
            for p in params_list:
                p_clean = re.sub(r"@[A-Za-z0-9_]+(?:\([^)]*\))?\s*", "", p).strip()
                p_type = p_clean.split()[0]
                see_types.append(p_type)
            see_doc = generate_javadoc({"see": f"#{name}({', '.join(see_types)})"}, indent_spaces=4)

            c_lines = []
            if see_doc:
                c_lines.append(see_doc)
            c_lines.append("    @NonNull")
            c_lines.append(f"    {method_prefix}{struct_name} {name}({', '.join(c_args_before)}) {{")
            c_lines.append(f"        return {name}({', '.join(c_call_before)});")
            c_lines.append("    }")

            convenience_methods = self._generate_convenience_overloads(
                method, effective_doc, method_prefix, struct_name, "@NonNull", None, size_param_consumers
            )
            return "\n\n".join(convenience_methods + ["\n".join(out), "\n".join(c_lines)])

        # Step 17: Math Type Return Logic (math vectors, matrices, LinearColor).
        # Enforces Asserts.* bounds validation on both inputs and caller-allocated output buffers,
        # with automatic synthesization of unrolled scalar convenience overloads.
        if math_ret_info:
            j_arr_type = math_ret_info["java"]
            size = math_ret_info["size"]
            
            # Additional 'out' arg
            out_arg_def = f"@Nullable @Size(min = {size}) {j_arr_type} out"
            params_list.append(out_arg_def)
            call_args.append("out")
            arg_names.append("out")
            
            method_param_types["out"] = j_arr_type
            javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
            if javadoc:
                out.append(javadoc)
            
            is_linear_ret = ret_info.get("is_linear_color") or ret_type_cpp.get("cpp_name") in ("LinearColor", "LinearColorA", "const LinearColor &", "const LinearColorA &")
            ret_ann_parts = [f"@NonNull @Size(min = {size})"]
            if is_linear_ret:
                ret_ann_parts.append("@LinearColor")
            ret_ann_str = " ".join(ret_ann_parts)
            out.append(f"    {ret_ann_str}")
            
            signature = f"{method_prefix}{j_arr_type} {name}({', '.join(params_list)})"
            main_sig = self._get_method_sig(name, params_list)
            self.emitted_signatures.add(main_sig)
            out.append(f"    {signature} {{")

            for code in validation_code:
                out.append(f"        {code}")

            for arg in method.get("arguments", []):
                size_param_name = get_size_param_attr(arg)
                if size_param_name:
                    continue
                
                m_in = self.get_array_input_info(arg["type"])
                is_pointer = arg["type"].get("is_pointer", False)
                if m_in and "assert" in m_in and ("::mat" in arg["type"]["cpp_name"] or is_pointer):
                    assert_method = m_in["assert"]
                    nullability = arg["type"].get("nullability", "unspecified")
                    arg_name = arg["name"]
                    
                    if is_pointer and nullability != "nonnull":
                         out.append(f"        if ({arg_name} != null) {{")
                         out.append(f"            Asserts.{assert_method}In({arg_name});")
                         out.append(f"        }}")
                    else:
                         out.append(f"        Asserts.{assert_method}In({arg_name});")
            
            assert_method = math_ret_info["assert"]
            out.append(f"        out = Asserts.{assert_method}(out);")
            
            native_name = "n" + name[0].upper() + name[1:]
            out.append(f"        {native_name}({', '.join(call_args)});")
            out.append("        return out;")
            out.append("    }")
            
            convenience_methods = self._generate_convenience_overloads(
                method, effective_doc, method_prefix, j_arr_type, None, math_ret_info, size_param_consumers
            )

            # Array input overloads if arguments were unrolled
            c_args = method.get("arguments", [])
            has_unrolled = False
            arr_params = []
            arr_call = []
            arr_doc_names = []
            for arg in c_args:
                arr_in = self.get_array_input_info(arg["type"])
                req_type_str = arg["type"].get("qualified_name") or arg["type"]["cpp_name"] if isinstance(arg["type"], dict) else arg["type"]
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = arg["type"].get("is_pointer", False)
                size_param_name = bool(get_size_param_attr(arg))
                if arr_in and not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                    has_unrolled = True
                    sz = arr_in["size"]
                    is_lin = arg["type"].get("cpp_name") in ("LinearColor", "LinearColorA", "const LinearColor &", "const LinearColorA &")
                    ann_p = [f"@NonNull @Size(min = {sz})"]
                    if is_lin:
                        ann_p.append("@LinearColor")
                    safe_name = sanitize_identifier(arg["name"])
                    arr_params.append(f"{' '.join(ann_p)} float[] {safe_name}")
                    for idx in range(sz):
                        arr_call.append(f"{safe_name}[{idx}]")
                    arr_doc_names.append(safe_name)
                else:
                    safe_name = sanitize_identifier(arg["name"])
                    arr_doc_names.append(safe_name)
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    j_t = self.get_java_type(arg['type'])
                    if t_info.get("is_enum"):
                        arr_params.append(f"@NonNull {j_t} {safe_name}")
                    else:
                        arr_params.append(f"{j_t} {safe_name}")
                    arr_call.append(safe_name)

            if has_unrolled:
                arr_with_out_params = arr_params + [out_arg_def]
                arr_with_out_call = arr_call + ["out"]
                arr_with_out_names = arr_doc_names + ["out"]
                sig_with_out = self._get_method_sig(name, arr_with_out_params)
                if sig_with_out not in self.emitted_signatures:
                    self.emitted_signatures.add(sig_with_out)
                    doc_with_out = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arr_with_out_names, param_types=method_param_types)
                    lines_with_out = []
                    if doc_with_out:
                        lines_with_out.append(doc_with_out)
                    lines_with_out.append(f"    {ret_ann_str}")
                    lines_with_out.append(f"    {method_prefix}{j_arr_type} {name}({', '.join(arr_with_out_params)}) {{")
                    lines_with_out.append(f"        return {name}({', '.join(arr_with_out_call)});")
                    lines_with_out.append("    }")
                    convenience_methods.append("\n".join(lines_with_out))

            return "\n\n".join(convenience_methods + ["\n".join(out)])
        
        # Step 18: Standard Instance / Static Method Dispatch.
        # Handles standard C++ member function invocations:
        # - Resolves Java return type and nullability annotations.
        # - Emits input validation bounds checks.
        # - Maps return values from native handles, enums (via EnumCache), or primitive types.
        # - Attaches convenience and vector array overloads.
        ret_type = self.get_java_type(ret_type_cpp)
        ret_ann = self.get_annotation(ret_type_cpp)
        if self.is_attribute_bitset(ret_type_cpp):
            ret_type = "Set<VertexBuffer.VertexAttribute>"
            ret_ann = "@NonNull"
        elif ret_info.get("is_string") or ret_type == "String":
            nullability = ret_type_cpp.get("nullability", "unspecified") if isinstance(ret_type_cpp, dict) else "unspecified"
            ret_ann = "@Nullable" if nullability == "nullable" else "@NonNull"
        
        native_name = "n" + name[0].upper() + name[1:]
        
        # Generate Javadoc
        javadoc = generate_javadoc(effective_doc, indent_spaces=4, argument_names=arg_names, param_types=method_param_types)
        if javadoc:
            out.append(javadoc)
        
        main_sig = self._get_method_sig(name, params_list)
        if main_sig in self.emitted_signatures:
            return None
        self.emitted_signatures.add(main_sig)

        if ret_ann:
            out.append(f"    {ret_ann}")
            
        signature = f"{method_prefix}{ret_type} {name}({', '.join(params_list)})"
        out.append(f"    {signature} {{")

        # Validation Code
        for code in validation_code:
            out.append(f"        {code}")

        # Input asserts
        for arg in method.get("arguments", []):
            size_param_name = get_size_param_attr(arg)
            if size_param_name:
                continue

            m_in = self.get_array_input_info(arg["type"])
            is_pointer = arg["type"].get("is_pointer", False)
            if m_in and "assert" in m_in and ("::mat" in arg["type"]["cpp_name"] or is_pointer):
                assert_method = m_in["assert"]
                nullability = arg["type"].get("nullability", "unspecified")
                arg_name = arg["name"]
                
                if is_pointer and nullability != "nonnull":
                     out.append(f"        if ({arg_name} != null) {{")
                     out.append(f"            Asserts.{assert_method}In({arg_name});")
                     out.append(f"        }}")
                else:
                     out.append(f"        Asserts.{assert_method}In({arg_name});")
        
        if self.name == "MaterialInstance" and name == "duplicate":
            out.append(f"        long nativeInstance = {native_name}({', '.join(call_args)});")
            out.append('        if (nativeInstance == 0) throw new IllegalStateException("Couldn\'t duplicate MaterialInstance");')
            out.append("        return new MaterialInstance(nativeInstance, other.getMaterial());")
        elif self.archetype == "bitfield" and (ret_type == "void" or ret_info.get("cpp_type") in (self.name, f"{self.name} &", f"const {self.name} &")):
            field_name = self.value_type_info.get("field_name", "mSampler")
            out.append(f"        {field_name} = {native_name}({', '.join(call_args)});")
            if ret_type != "void":
                out.append("        return this;")
        elif ret_info.get("is_enum"):
            enum_name = ret_info['java']
            if "." in enum_name:
                owner_class, simple_enum = enum_name.rsplit(".", 1)
                if owner_class == self.name:
                    enum_name = simple_enum
            enum_ir = next((e for e in self.enums if e["name"] == enum_name), None)
            is_custom = False
            if enum_ir:
                for idx, entry in enumerate(enum_ir.get("entries", [])):
                    val = entry.get("value")
                    if val is not None and val != idx:
                        is_custom = True
                        break
            if is_custom:
                out.append(f"        return {enum_name}.from({native_name}({', '.join(call_args)}));")
            else:
                if "." in ret_info['java']:
                    owner_class, simple_enum = ret_info['java'].rsplit(".", 1)
                    out.append(f"        return EnumCache.s{simple_enum}Values[{native_name}({', '.join(call_args)})];")
                else:
                    out.append(f"        return EnumCache.s{enum_name}Values[{native_name}({', '.join(call_args)})];")
        elif ret_info.get("is_filament_type"):
            nullability = method["return_type"].get("nullability", "unspecified")
            target_class = ret_info['java']
            extra_args = ", this" if self.retains_parent_handle(target_class) else ""
            result_var = f"native{target_class}" if f"native{target_class}" not in arg_names else "result"
            out.append(f"        long {result_var} = {native_name}({', '.join(call_args)});")
            if nullability == "nonnull":
                verb = "duplicate" if name == "duplicate" else "create"
                out.append(f'        if ({result_var} == 0) throw new IllegalStateException("Couldn\'t {verb} {target_class}");')
                out.append(f"        return new {target_class}({result_var}{extra_args});")
            else:
                out.append(f"        return {result_var} == 0 ? null : new {target_class}({result_var}{extra_args});")
        elif self.is_attribute_bitset(ret_type_cpp):
            out.append(f"        return getAttributes({native_name}({', '.join(call_args)}));")
        else:
            call_prefix = ""
            if ret_type != "void":
                call_prefix = "return "
            if self.name == "Engine" and name == "getFeatureFlag":
                out.append('        if (!hasFeatureFlag(name)) throw new IllegalArgumentException("Feature flag " + name + " does not exist");')
            out.append(f"        {call_prefix}{native_name}({', '.join(call_args)});")
        out.append("    }")
        
        convenience_methods = self._generate_convenience_overloads(
            method, effective_doc, method_prefix, ret_type, ret_ann, None, size_param_consumers
        )
        vec_overloads = self._generate_vector_array_overloads(
            method, effective_doc, method_prefix, ret_type, ret_ann, is_builder=False
        )
        return "\n\n".join(convenience_methods + ["\n".join(out)] + vec_overloads)

    def _generate_native_decl(self, m_ctx, dry_run=False, return_sig=False):
        """Synthesizes private static native JNI method declarations.

        Generates the low-level `private static native ... nMethodName(...)` declaration
        corresponding to a high-level Java method. Marshals parameters into their C++ JNI
        counterparts (objects -> native long handles, enums -> ints, strings -> String,
        POD structs -> flattened scalars, buffers -> direct Buffer / arrays).

        Args:
            m_ctx (Dict[str, Any]): Expanded method context IR dictionary.
            dry_run (bool): If True, computes the declaration string without registering
                the signature into `self.emitted_native_signatures`.
            return_sig (bool): If True, returns only the normalized method signature string
                `nName(arg_types...)` instead of the full Java declaration.

        Returns:
            Optional[str]: Formatted single-line (or multi-line annotated) Java native
            method declaration; or signature string if `return_sig` is True; or None if
            the declaration is already emitted or intercepted as a non-native helper.

        Raises:
            JavaGenerationError: If parameter extraction or type mapping fails.

        Side Effects:
            Registers the generated native signature in `self.emitted_native_signatures`
            unless `dry_run` or `return_sig` is True.
        """
        method = m_ctx["ir"]
        name = get_effective_method_name(method)

        # Step 1: Interception check for pure Java synthesized methods.
        # Cached getters, inline value getters, getEngine, and standard constructors
        # have no direct native dispatch entry points here.
        if name in [c["getter"] for c in self.cached_fields.values()]:
            info = next(c for c in self.cached_fields.values() if c["getter"] == name)
            if not info.get("is_handle_reference"):
                return None
        if self.is_value_class and name == self.value_type_info.get("buffer_getter"):
            return None
        if name in self.retained_references:
            return None
        if method.get("is_constructor") and self.archetype != "bitfield" and not self.base_class:
            return None
        if self.name == "Engine":
            intercepted_engine_methods = (
                "destroy",
                "createSwapChain",
            )
            if name in intercepted_engine_methods:
                return None

        # Step 2: Material.getParameters() reflection specialization.
        if self.name == "Material" and name == "getParameters":
            decl = "    private static native int nGetParameters(long nativeMaterial, @NonNull ParameterInfo[] parameters, @IntRange(from = 0) int count);"
            if return_sig:
                return "nGetParameters(long,ParameterInfo[],int)"
            if not dry_run:
                if "nGetParameters" in self.emitted_native_signatures:
                    return None
                self.emitted_native_signatures.add("nGetParameters")
            return decl

        # Step 3: Asynchronous callback native declaration.
        # Marshals CallbackHandler to Object and Invocable/std::function to Runnable.
        if m_ctx.get("is_async_callback"):
            native_name = f"n{name[0].upper() + name[1:]}"
            native_args = [f"long native{self.name}"]
            for arg in method.get("arguments", []):
                arg_name = sanitize_identifier(arg["name"])
                t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "")
                if "CallbackHandler" in t_str:
                    native_args.append(f"Object {arg_name}")
                elif any(cb in t_str for cb in ("Invocable", "std::function", "Callback")):
                    native_args.append(f"Runnable {arg_name}")
                else:
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    if t_info.get("is_enum"):
                        native_args.append(f"int {arg_name}")
                    elif t_info.get("is_filament_type"):
                        native_args.append(f"long {arg_name}")
                    else:
                        native_args.append(f"{self.get_java_type(arg['type'])} {arg_name}")
            return f"    private static native void {native_name}({', '.join(native_args)});"

        # Step 4: BufferDescriptor direct streaming native declaration.
        if self.is_buffer_descriptor_method(method):
            args = method.get("arguments", [])
            buf_idx = -1
            for i, a in enumerate(args):
                t = a["type"]
                q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
                if "BufferDescriptor" in q and "PixelBufferDescriptor" not in q:
                    buf_idx = i
                    break
            pre_args = args[1:buf_idx]
            decl_params = [f"long native{self.name}", "long nativeEngine"]
            for a in pre_args:
                decl_params.append(f"{self.get_java_type(a['type'])} {sanitize_identifier(a['name'])}")
            decl_params.extend([
                "Buffer buffer",
                "int remaining",
                "int destOffsetInBytes",
                "int count",
                "Object handler",
                "Runnable callback"
            ])
            native_name = f"n{name[0].upper() + name[1:]}"
            decl_sig = self._get_method_sig(native_name, decl_params)
            if return_sig:
                return decl_sig
            if not dry_run:
                if decl_sig in self.emitted_native_signatures:
                    return None
                self.emitted_native_signatures.add(decl_sig)
            return f"    private static native int {native_name}({', '.join(decl_params)});"

        # Step 5: PixelBufferDescriptor texture read/write native declaration.
        if self.is_pixel_buffer_descriptor_method(method):
            args = method.get("arguments", [])
            buf_idx = -1
            for i, a in enumerate(args):
                t = a["type"]
                q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
                if "PixelBufferDescriptor" in q:
                    buf_idx = i
                    break
            pre_args = args[:buf_idx]
            decl_params = [f"long native{self.name}"]
            for a in pre_args:
                a_t = a["type"]
                type_info, _ = self.resolve_type_info(a_t, silent=True)
                safe_name = sanitize_identifier(a["name"])
                if type_info.get("is_filament_type"):
                    decl_params.append(f"long native{safe_name[0].upper()}{safe_name[1:]}")
                elif type_info.get("is_enum"):
                    decl_params.append(f"int {safe_name}")
                else:
                    decl_params.append(f"{self.get_java_type(a_t)} {safe_name}")
            decl_params.extend([
                "Buffer storage",
                "int remaining",
                "int left",
                "int top",
                "int type",
                "int alignment",
                "int stride",
                "int format",
                "Object handler",
                "Runnable callback"
            ])
            native_name = f"n{name[0].upper() + name[1:]}"
            decl_sig = self._get_method_sig(native_name, decl_params)
            if return_sig:
                return decl_sig
            if not dry_run:
                if decl_sig in self.emitted_native_signatures:
                    return None
                self.emitted_native_signatures.add(decl_sig)
            return f"    private static native void {native_name}({', '.join(decl_params)});"

        # Step 6: Tagged array scalar family native declaration.
        if m_ctx.get("is_tagged_array"):
            method = m_ctx["ir"]
            family = m_ctx["family"]
            tagged_info = m_ctx["tagged_info"]
            family_cfg = TAGGED_SCALAR_FAMILIES[family]
            
            name = get_effective_method_name(method)
            cap_family = family_cfg["family"]
            native_name = f"n{name[0].upper() + name[1:]}{cap_family}Array"
            
            decl_params = [f"long native{self.name}"]
            for a in tagged_info["pre_args"]:
                a_info, _ = self.resolve_type_info(a["type"], silent=True)
                p_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    decl_params.append(f"long native{p_name[0].upper()}{p_name[1:]}")
                elif a_info.get("is_string"):
                    decl_params.append(f"@NonNull String {p_name}")
                elif a_info.get("is_enum"):
                    decl_params.append(f"int {p_name}")
                else:
                    decl_params.append(f"{self.get_java_type(a['type'])} {p_name}")
                    
            values_name = sanitize_identifier(tagged_info["tagged_arg"]["name"])
            count_name = sanitize_identifier(tagged_info["cnt_arg"]["name"])
            scalar_arr = family_cfg["java_array"]
            
            decl_params.extend([
                "int type",
                f"@NonNull @Size(min = 1) {scalar_arr} {values_name}",
                "@IntRange(from = 0) int offset",
                f"@IntRange(from = 1) int {count_name}"
            ])
            
            decl_sig = self._get_method_sig(native_name, decl_params)
            if return_sig:
                return decl_sig
            if not dry_run:
                if decl_sig in self.emitted_native_signatures:
                    return None
                self.emitted_native_signatures.add(decl_sig)
            return f"    private static native void {native_name}({', '.join(decl_params)});"

        # Step 7: Packed buffer native declarations (dual buffer/array modes).
        if self.is_packed_buffer_method(method):
            info = self.get_packed_buffer_info(method)
            mode = m_ctx.get("buffer_mode", "buffer")
            name = get_effective_method_name(method)
            is_bld = (self.archetype == "builder")
            native_name = f"nBuilder{name[0].upper() + name[1:]}" if is_bld else f"n{name[0].upper() + name[1:]}"
            stride = info["stride"]
            buf_name = info["buf_name"]
            cnt_name = info["cnt_name"]
            has_existing_offset = info["has_existing_offset"]
            array_offset_name = info["array_offset_name"]
            java_type = info.get("java_type", "float[]")

            decl_params = [f"long native{self.name}"]
            for a in info["pre_args"]:
                a_info, _ = self.resolve_type_info(a["type"], silent=True)
                p_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    decl_params.append(f"long native{p_name[0].upper()}{p_name[1:]}")
                else:
                    decl_params.append(f"{self.get_java_type(a['type'])} {p_name}")

            if mode == "buffer":
                if is_bld:
                    decl_params.extend([
                        f"int {cnt_name}",
                        f"@NonNull Buffer {buf_name}",
                        "int remaining"
                    ])
                elif has_existing_offset:
                    decl_params.extend([
                        f"@NonNull Buffer {buf_name}",
                        "int remaining",
                        f"@IntRange(from = 0) int {cnt_name}",
                        "@IntRange(from = 0) int offset"
                    ])
                else:
                    decl_params.extend([
                        f"@NonNull Buffer {buf_name}",
                        "int remaining",
                        f"@IntRange(from = 0) int {cnt_name}"
                    ])
                decl_sig = self._get_method_sig(native_name, decl_params)
                if return_sig:
                    return decl_sig
                if not dry_run:
                    if decl_sig in self.emitted_native_signatures:
                        return None
                    self.emitted_native_signatures.add(decl_sig)
                return f"    private static native int {native_name}({', '.join(decl_params)});"
            else:
                if is_bld:
                    decl_params.extend([
                        f"@NonNull @Size(min = {stride}) {java_type} {buf_name}",
                        f"int {cnt_name}",
                        f"int {array_offset_name}"
                    ])
                elif has_existing_offset:
                    decl_params.extend([
                        f"@NonNull @Size(min = {stride}) {java_type} {buf_name}",
                        f"@IntRange(from = 0) int {cnt_name}",
                        "@IntRange(from = 0) int offset",
                        "@IntRange(from = 0) int arrayOffset"
                    ])
                else:
                    decl_params.extend([
                        f"@NonNull @Size(min = {stride}) {java_type} {buf_name}",
                        f"@IntRange(from = 0) int {cnt_name}",
                        "@IntRange(from = 0) int offset"
                    ])
                decl_sig = self._get_method_sig(native_name, decl_params)
                if return_sig:
                    return decl_sig
                if not dry_run:
                    if decl_sig in self.emitted_native_signatures:
                        return None
                    self.emitted_native_signatures.add(decl_sig)
                return f"    private static native void {native_name}({', '.join(decl_params)});"
        
        # Step 8: Standard native parameter marshalling and receiver resolution.
        ret_type_cpp = method["return_type"]
        ret_info, _ = self.resolve_type_info(ret_type_cpp)
        math_ret_info = ret_info if "assert" in ret_info else None
        
        is_static = method.get("is_static", False) or self.is_utility_class
        if is_static or method.get("is_constructor"):
            params_list = []
        elif self.archetype == "inline_buffer":
            elem_type = self.value_type_info.get("element_type", "float")
            field_name = self.value_type_info.get("field_name", "mPlanes")
            params_list = [f"{elem_type}[] {field_name[1:].lower()}"]
        elif self.archetype == "bitfield":
            storage_type = self.value_type_info.get("storage_type", "long")
            field_name = self.value_type_info.get("field_name", "mSampler")
            params_list = [f"{storage_type} {field_name[1:].lower()}"]
        elif self.is_aggregate:
            params_list = [f"{l['java_type']} {l['param_name']}" for l in self.get_flattened_leaves(self.name)]
        else:
            params_list = [f"long native{self.name}"]

        if self.archetype == "builder":
            if method.get("is_constructor"):
                clean_types = "".join(self._get_clean_type_name(a["type"]) for a in method.get("arguments", []))
                native_name = f"nCreateBuilder{clean_types}" if clean_types else "nCreateBuilder"
            elif name == "build":
                native_name = "nBuilderBuild"
            else:
                native_name = "nBuilder" + name[0].upper() + name[1:]
        elif method.get("is_constructor"):
            clean_types = "".join(self._get_clean_type_name(a["type"]) for a in method.get("arguments", []))
            native_name = f"nCreate{self.name}{clean_types}" if clean_types else f"nCreate{self.name}"
        else:
            native_name = "n" + name[0].upper() + name[1:]

        for arg in method.get("arguments", []):
            arg_name = arg["name"]
            req_type = arg["type"]
            
            # If method returns a container, drop the size/count argument because it is inferred from the out-array
            if ret_info.get("is_container") and not math_ret_info and arg_name in ("historySize", "size", "count", "capacity", "maxCount"):
                continue

            # Check for size_param
            size_param_name = get_size_param_attr(arg)

            arr_in = self.get_array_input_info(req_type)
            if arr_in:
                req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = arg["type"].get("is_pointer", False)
                is_slice = arr_in.get("is_slice", False)
                
                if not is_matrix and not is_pointer and not size_param_name and not is_slice:
                    # Unroll
                    scalar = arr_in["scalar"]
                    size = arr_in["size"]
                    components = ["x", "y", "z", "w"][:size]
                    for comp in components:
                         params_list.append(f"{scalar} {sanitize_identifier(f'{arg_name}{comp}')}")
                else:
                    j_type = arr_in["java"]
                    size = arr_in["size"]
                    nullability = arg["type"].get("nullability", "unspecified")
                    
                    if arr_in.get("is_custom_array"):
                        ann_prefix = f"{arr_in['annotation']} " if arr_in.get("annotation") else ""
                        if is_slice:
                            ann = f"@NonNull {ann_prefix}".strip()
                        elif nullability == "nonnull":
                            ann = f"@NonNull {ann_prefix}".strip()
                        elif nullability == "nullable":
                            ann = f"@Nullable {ann_prefix}".strip()
                        else:
                            ann = ann_prefix.strip()
                    else:
                        ann = f"@NonNull @Size(min = {size})"
                        if is_pointer:
                            ann = f"@Size(min = {size})"
                            if nullability == "nonnull":
                                ann = f"@NonNull @Size(min = {size})"
                    
                    safe_name = sanitize_identifier(arg_name)
                    p_str = f"{ann} {j_type} {safe_name}".strip()
                    params_list.append(p_str)
                    if is_slice:
                        params_list.append(f"int {safe_name}Count")
            else:
                safe_name = sanitize_identifier(arg_name)
                inv_info = self.get_invocable_info(req_type, name)
                if inv_info:
                    params_list.append(f"@NonNull {inv_info['interface_name']} {safe_name}")
                    continue

                type_info, _ = self.resolve_type_info(req_type)
                if type_info.get("is_pojo_struct"):
                    struct_cls = type_info["struct_cls"]
                    leaves = self.get_pojo_leaves(struct_cls)
                    for leaf in leaves:
                        params_list.append(f"{leaf['java_type']} {leaf['name']}")
                    continue

                if type_info.get("is_struct"):
                    struct_cls = type_info["struct_cls"]
                    leaves = self.get_flattened_leaves(struct_cls, prefix=arg_name)
                    for leaf in leaves:
                        params_list.append(f"{leaf['java_type']} {leaf['param_name']}")
                    continue

                if type_info.get("is_string") or type_info.get("java") == "String":
                    nullability = req_type.get("nullability", "unspecified") if isinstance(req_type, dict) else "unspecified"
                    ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                    params_list.append(f"{ann}String {safe_name}")
                    continue

                if type_info.get("is_enum"):
                    params_list.append(f"int {safe_name}")
                    continue

                if type_info.get("is_filament_type"):
                    j_type = "long"
                    p_name = f"native{arg_name[0].upper()}{arg_name[1:]}"
                elif type_info.get("archetype") == "bitfield":
                    j_type = type_info.get("storage_type", "int")
                    p_name = safe_name
                else:
                    j_type = self.get_java_type(req_type)
                    p_name = safe_name
                ann = self.get_annotation(req_type)
                
                p_str = ""
                if ann:
                    p_str += f"{ann} "
                p_str += f"{j_type} {p_name}"
                params_list.append(p_str)
            
        # Step 9: Native return type resolution and signature deduplication.
        if method.get("is_constructor"):
            if self.archetype == "bitfield":
                storage_type = self.value_type_info.get("storage_type", "int")
                decl = f"    private static native {storage_type} {native_name}({', '.join(params_list)});"
            else:
                decl = f"    private static native long {native_name}({', '.join(params_list)});"
        elif math_ret_info:
            j_arr_type = math_ret_info["java"]
            size = math_ret_info["size"]
            params_list.append(f"@Size(min = {size}) {j_arr_type} out")
            decl = f"    private static native void {native_name}({', '.join(params_list)});"
        elif ret_info.get("is_slice") and not ret_info.get("is_struct_slice"):
            java_elem = ret_info.get("java_elem_type", "int")
            clean_elem = java_elem.replace("@Entity", "").strip()
            params_list.append(f"{clean_elem}[] out")
            decl = f"    @NonNull\n    private static native {clean_elem}[] {native_name}({', '.join(params_list)});"
        elif ret_info.get("is_value_object"):
            size = ret_info.get("size", 24)
            params_list.append(f"@NonNull @Size(min = {size}) float[] out")
            decl = f"    private static native void {native_name}({', '.join(params_list)});"
        elif ret_info.get("is_container"):
            out_param_name = "outHistory" if "History" in name else "out"
            java_elem = ret_info.get("java_elem_type", "Object")
            params_list.append(f"{java_elem}[] {out_param_name}")
            decl = f"    private static native int {native_name}({', '.join(params_list)});"
        elif ret_info.get("is_struct") and not self.is_aggregate:
            struct_cls = ret_info.get("struct_cls") or self.is_aggregate_struct(ret_info.get("cpp_type"))
            struct_name = struct_cls["name"] if struct_cls else ret_info["java"]
            params_list.append(f"@NonNull {struct_name} out")
            decl = f"    private static native void {native_name}({', '.join(params_list)});"
        else:
            # Standard return type mapping
            ret_ann = None
            if self.archetype == "bitfield" and (self.get_java_type(ret_type_cpp) == "void" or ret_info.get("cpp_type") in (self.name, f"{self.name} &", f"const {self.name} &")):
                ret_type = self.value_type_info.get("storage_type", "int")
            elif ret_info.get("archetype") == "bitfield":
                ret_type = ret_info.get("storage_type", "int")
            elif ret_info.get("is_enum"):
                ret_type = "int"
            elif ret_info.get("is_filament_type"):
                ret_type = "long"
            elif ret_info.get("is_string") or ret_info.get("java") == "String":
                ret_type = "String"
                nullability = ret_type_cpp.get("nullability", "unspecified") if isinstance(ret_type_cpp, dict) else "unspecified"
                ret_ann = "@Nullable" if nullability == "nullable" else "@NonNull"
            else:
                ret_type = self.get_java_type(ret_type_cpp)
                ret_ann = self.get_annotation(ret_type_cpp)
            
            ret_prefix = ""
            if ret_ann:
                ret_prefix = f"{ret_ann}\n    "
                
            decl = f"    {ret_prefix}private static native {ret_type} {native_name}({', '.join(params_list)});"

        native_sig = self._get_method_sig(native_name, params_list)
        if return_sig:
            return native_sig
        if not dry_run:
            if native_sig in self.emitted_native_signatures:
                return None
            self.emitted_native_signatures.add(native_sig)
        return decl

    def generate_builder_class_java(self):
        """Generates static inner Builder class for fluent object construction.

        Synthesizes the complete `public static class Builder` definition for classes
        employing the builder pattern in Filament (e.g., VertexBuffer.Builder,
        Texture.Builder). This includes:
        - Memory management lifecycle via `BuilderFinalizer` and `nDestroyBuilder`.
        - Buffer retention container (`mRetainedBuffers`) to guard against premature
          garbage collection during native resource staging.
        - Nested builder-scoped enums and bitmask flags.
        - Builder constructor variants initializing native peer builder pointers.
        - Fluent setter methods and terminal `build()` execution methods.

        Returns:
            str: Complete Java source code for the static inner Builder class.

        Raises:
            JavaGenerationError: If builder context initialization or method expansion fails.
        """
        if self.parent_context and self.parent_context.name == "Engine":
            return self._generate_engine_builder_java()

        out = []

        # Step 1: Emit Builder class header, Javadoc, and lifecycle state fields.
        builder_doc = self.doc or {"brief": f"Use <code>Builder</code> to construct a <code>{self.parent_context.name}</code> object instance."}
        b_doc_str = generate_javadoc(builder_doc, indent_spaces=4)
        if b_doc_str:
            out.append(b_doc_str)
        out.append("    public static class Builder {")
        out.append("        @SuppressWarnings({\"FieldCanBeLocal\", \"UnusedDeclaration\"}) // Keep to finalize native resources")
        out.append("        private final BuilderFinalizer mFinalizer;")
        out.append("        private final long mNativeBuilder;")
        if self.has_retained_buffers:
            out.append("        private final java.util.List<Object> mRetainedBuffers = new java.util.ArrayList<>();")
        out.append("")

        # Step 2: Inner Enums & Flag Bitmask Classes within Builder scope.
        for enum_ir in self.enums:
            edoc = generate_javadoc(enum_ir.get("doc", {}), indent_spaces=8)
            if edoc:
                out.append(edoc)
            is_flags = enum_ir.get("is_flags", False) or any(a in ("filament:apigen:flags", "filament:flags", "apigen:flags", "flags", "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask") for a in enum_ir.get("attributes", []))
            if is_flags:
                out.append(f"        public static class {enum_ir['name']} {{")
                out.append(f"            private {enum_ir['name']}() {{}}")
                out.append("")
                for entry in enum_ir.get("entries", []):
                    edoc = generate_javadoc(entry.get("doc", {}), indent_spaces=12)
                    if edoc:
                        out.append(edoc)
                    val = entry.get("value", 0)
                    name = format_enum_entry_name(enum_ir['name'], entry['name'])
                    hex_val = f"0x{val:X}" if val > 0 else "0"
                    out.append(f"            public static final int {name} = {hex_val};")
                out.append("        }")
                out.append("")
                continue
            out.append(f"        public enum {enum_ir['name']} {{")
            custom_enum = is_custom_enum(enum_ir)
            entry_strs = []
            for entry in enum_ir.get("entries", []):
                edoc = generate_javadoc(entry.get("doc", {}), indent_spaces=12)
                name = format_enum_entry_name(enum_ir['name'], entry['name'])
                if custom_enum:
                    val = entry.get("value", 0)
                    entry_line = f"            {name}({val})"
                else:
                    entry_line = f"            {name}"
                if edoc:
                    entry_strs.append(f"{edoc}\n{entry_line}")
                else:
                    entry_strs.append(entry_line)
            out.append(",\n".join(entry_strs) + ";\n")
            if custom_enum:
                out.append("            private final int mValue;\n")
                out.append(f"            {enum_ir['name']}(int value) {{")
                out.append("                mValue = value;")
                out.append("            }\n")
                out.append("            /** @return the value of this enum constant as used by the native Filament engine. */")
                out.append("            public int toFilamentNative() { return mValue; }\n")
                out.append(f"            public static {enum_ir['name']} from(int value) {{")
                out.append("                switch (value) {")
                seen_values = set()
                for entry in enum_ir.get("entries", []):
                    name = format_enum_entry_name(enum_ir['name'], entry['name'])
                    val = entry.get("value", 0)
                    if val in seen_values:
                        continue
                    seen_values.add(val)
                    out.append(f"                    case {val}: return {name};")
                out.append(f"                    default: throw new IllegalArgumentException(\"Unknown {enum_ir['name']} value: \" + value);")
                out.append("                }")
                out.append("            }")
            else:
                out.append("            /** @return the value of this enum constant as used by the native Filament engine. */")
                out.append("            public int toFilamentNative() { return ordinal(); }")
            out.append("        }")
            out.append("")

        # Step 3: Builder Constructors & Native Peer Allocation.
        has_default_ctor = False
        for m in self.methods:
            if m.get("is_constructor"):
                args = m.get("arguments", [])
                if len(args) == 0:
                    has_default_ctor = True
                    ctor_doc = generate_javadoc(m.get("doc") or {"brief": f"Use <code>Builder</code> to construct a <code>{self.parent_context.name}</code> object instance."}, indent_spaces=8)
                    if ctor_doc:
                        out.append(ctor_doc)
                    out.append("        public Builder() {")
                    out.append("            mNativeBuilder = nCreateBuilder();")
                    out.append("            mFinalizer = new BuilderFinalizer(mNativeBuilder);")
                    out.append("        }")
                    out.append("")
                else:
                    ctor_doc = generate_javadoc(m.get("doc"), indent_spaces=8)
                    if ctor_doc:
                        out.append(ctor_doc)
                    param_decls = []
                    call_args = []
                    for arg in args:
                        req_t = arg["type"]
                        j_type = self.get_java_type(req_t)
                        ann = self.get_annotation(req_t)
                        ann_str = f"{ann} " if ann else ""
                        param_decls.append(f"{ann_str}{j_type} {arg['name']}")
                        t_info, _ = self.resolve_type_info(req_t, silent=True)
                        if t_info.get("is_enum"):
                            call_args.append(f"{arg['name']}.toFilamentNative()")
                        else:
                            call_args.append(arg['name'])
                    out.append(f"        public Builder({', '.join(param_decls)}) {{")
                    out.append(f"            mNativeBuilder = nCreateBuilder({', '.join(call_args)});")
                    out.append("            mFinalizer = new BuilderFinalizer(mNativeBuilder);")
                    out.append("        }")
                    out.append("")

        if not has_default_ctor and not any(m.get("is_constructor") for m in self.methods):
            out.append("        public Builder() {")
            out.append("            mNativeBuilder = nCreateBuilder();")
            out.append("            mFinalizer = new BuilderFinalizer(mNativeBuilder);")
            out.append("        }")
            out.append("")

        # Step 4: Package-private native builder pointer accessor.
        out.append("        long getNativeBuilder() {")
        out.append("            return mNativeBuilder;")
        out.append("        }")
        out.append("")

        # Step 5: Expand and generate fluent Builder methods.
        for method in self.methods:
            if method.get("is_constructor"):
                continue
            for exp_m in self._expand_method(method):
                m_java = self._generate_builder_method_java(exp_m)
                if m_java:
                    out.append(m_java)
                    out.append("")

        # Step 6: BuilderFinalizer inner class for safe C++ builder destruction.
        out.append("        private static class BuilderFinalizer {")
        out.append("            private final long mNativeObject;")
        out.append("")
        out.append("            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }")
        out.append("")
        out.append("            @Override")
        out.append("            public void finalize() {")
        out.append("                try {")
        out.append("                    super.finalize();")
        out.append("                } catch (Throwable t) { // Ignore")
        out.append("                } finally {")
        out.append("                    nDestroyBuilder(mNativeObject);")
        out.append("                }")
        out.append("            }")
        out.append("        }")
        out.append("    }")
        return "\n".join(out)

    def _generate_builder_method_java(self, m_ctx):
        """Generates fluent setter and terminal execution methods within Builder class.

        Handles the emission of builder methods including:
        - Terminal `build()` variants (creating engine-managed instances or attaching components).
        - Packed buffer methods supporting direct NIO buffers and primitive float arrays.
        - Chained setter methods returning `@NonNull Builder` (`return this;`).
        - Buffer retention tracking (`mRetainedBuffers.add(buffer)`) preventing GC cleanup.
        - Automatic size parameter inference and vector array convenience overloads.

        Args:
            m_ctx (Dict[str, Any]): Expanded method context IR dictionary.

        Returns:
            Optional[str]: Formatted Java method source string for the Builder class;
            or None if the method was suppressed or unexpanded.

        Raises:
            JavaGenerationError: If parameter extraction or type mapping fails.
        """
        method = m_ctx["ir"]
        name = get_effective_method_name(method)
        doc = method.get("doc", {})
        import copy
        effective_doc = copy.deepcopy(doc)
        
        # Step 1: Terminal build() execution methods.
        if name == "build":
            parent_name = self.parent_context.name
            args = method.get("arguments", [])
            
            # Case 1: build(Engine& engine) returning TargetClass*
            if len(args) == 1 and ("Engine" in args[0]["type"].get("cpp_name", "") or "Engine" in args[0]["type"].get("qualified_name", "")):
                javadoc = generate_javadoc(effective_doc, indent_spaces=8, argument_names=[a["name"] for a in args], param_types={args[0]["name"]: "Engine"})
                lines = []
                if javadoc:
                    lines.append(javadoc)
                lines.append("        @NonNull")
                lines.append(f"        public {parent_name} build(@NonNull Engine engine) {{")
                lines.append(f"            long native{parent_name} = nBuilderBuild(mNativeBuilder, engine.getNativeObject());")
                lines.append(f"            if (native{parent_name} == 0) throw new IllegalStateException(\"Couldn't create {parent_name}\");")
                lines.append(f"            return new {parent_name}(native{parent_name});")
                lines.append("        }")
                return "\n".join(lines)
            
            # Case 2: build(Engine& engine, Entity entity) component builder
            elif len(args) == 2:
                b_types = {args[0]["name"]: "Engine", args[1]["name"]: "int"}
                javadoc = generate_javadoc(effective_doc, indent_spaces=8, argument_names=[a["name"] for a in args], param_types=b_types)
                lines = []
                if javadoc:
                    lines.append(javadoc)
                lines.append("        public void build(@NonNull Engine engine, @Entity int entity) {")
                lines.append("            if (!nBuilderBuild(mNativeBuilder, engine.getNativeObject(), entity)) {")
                lines.append(f"                throw new IllegalStateException(\"Couldn't create {parent_name}\");")
                lines.append("            }")
                lines.append("        }")
                return "\n".join(lines)
            
            # Case 3: build() with no args
            elif len(args) == 0:
                javadoc = generate_javadoc(effective_doc, indent_spaces=8, argument_names=[])
                lines = []
                if javadoc:
                    lines.append(javadoc)
                lines.append("        @Nullable")
                lines.append(f"        public {parent_name} build() {{")
                lines.append(f"            long native{parent_name} = nBuilderBuild(mNativeBuilder);")
                lines.append(f"            if (native{parent_name} == 0) return null;")
                lines.append(f"            return new {parent_name}(native{parent_name});")
                lines.append("        }")
                return "\n".join(lines)

        # Step 2: Packed buffer methods in builder context (NIO Buffer vs float array).
        if self.is_packed_buffer_method(method):
            info = self.get_packed_buffer_info(method)
            mode = m_ctx.get("buffer_mode", "buffer")
            stride = info["stride"]
            buf_name = info["buf_name"]
            cnt_name = info["cnt_name"]
            has_existing_offset = info["has_existing_offset"]
            array_offset_name = info["array_offset_name"]
            native_name = f"nBuilder{name[0].upper() + name[1:]}"

            doc_params = dict(effective_doc.get("params", {}))
            doc_params[cnt_name] = f"number of elements (structured element count) in <code>{buf_name}</code>"
            doc_params[array_offset_name] = f"offset in elements (structured element count) in <code>{buf_name}</code> to skip"
            if buf_name not in doc_params:
                if mode == "buffer":
                    doc_params[buf_name] = f"buffer containing {buf_name} data"
                else:
                    doc_params[buf_name] = f"array containing {buf_name} data"
            tailored_doc = dict(effective_doc)
            tailored_doc["params"] = doc_params

            buf_param_types = {buf_name: "Buffer"}
            arr_param_types = {buf_name: info.get("java_type", "float[]")}

            lines = []
            if mode == "buffer":
                sig_buf = self._get_method_sig(name, [f"int {cnt_name}", f"Buffer {buf_name}"])
                if sig_buf not in self.emitted_signatures:
                    self.emitted_signatures.add(sig_buf)
                    doc_buf_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                    doc_buf = generate_javadoc(doc_buf_dict, indent_spaces=8, argument_names=[cnt_name, buf_name], param_types=buf_param_types)
                    if doc_buf:
                        lines.append(doc_buf)
                    lines.append("        @NonNull")
                    lines.append(f"        public Builder {name}(@IntRange(from = 0) int {cnt_name}, @NonNull Buffer {buf_name}) {{")
                    lines.append(f"            int result = {native_name}(mNativeBuilder, {cnt_name}, {buf_name}, {buf_name}.remaining());")
                    lines.append("            if (result < 0) {")
                    lines.append("                throw new BufferOverflowException();")
                    lines.append("            }")
                    lines.append("            return this;")
                    lines.append("        }")
            else:
                java_type = info.get("java_type", "float[]")
                # Windowed overload: ({java_type} buf, int offset, int count)
                sig_win = self._get_method_sig(name, [f"{java_type} {buf_name}", f"int {array_offset_name}", f"int {cnt_name}"])
                if sig_win not in self.emitted_signatures:
                    self.emitted_signatures.add(sig_win)
                    doc_win_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                    doc_win = generate_javadoc(doc_win_dict, indent_spaces=8, argument_names=[buf_name, array_offset_name, cnt_name], param_types=arr_param_types)
                    if doc_win:
                        lines.append(doc_win)
                    lines.append("        @NonNull")
                    lines.append(f"        public Builder {name}(@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int {array_offset_name}, @IntRange(from = 0) int {cnt_name}) {{")
                    lines.append(f"            if ({buf_name}.length < ({array_offset_name} + {cnt_name}) * {stride}) {{")
                    lines.append(f"                throw new ArrayIndexOutOfBoundsException(\"Array length must be at least \" + (({array_offset_name} + {cnt_name}) * {stride}));")
                    lines.append("            }")
                    lines.append(f"            {native_name}(mNativeBuilder, {buf_name}, {cnt_name}, {array_offset_name});")
                    lines.append("            return this;")
                    lines.append("        }")
                    lines.append("")

                # Convenience with count: ({java_type} buf, int count)
                sig_cnt = self._get_method_sig(name, [f"{java_type} {buf_name}", f"int {cnt_name}"])
                if sig_cnt not in self.emitted_signatures:
                    self.emitted_signatures.add(sig_cnt)
                    doc_cnt_dict = self._adapt_packed_doc(tailored_doc, buf_name, cnt_name, has_offset_arg=False)
                    doc_cnt = generate_javadoc(doc_cnt_dict, indent_spaces=8, argument_names=[buf_name, cnt_name], param_types=arr_param_types)
                    if doc_cnt:
                        lines.append(doc_cnt)
                    lines.append("        @NonNull")
                    lines.append(f"        public Builder {name}(@NonNull @Size(min = {stride}) {java_type} {buf_name}, @IntRange(from = 0) int {cnt_name}) {{")
                    lines.append(f"            return {name}({buf_name}, 0, {cnt_name});")
                    lines.append("        }")
                    lines.append("")

                # Convenience full-array: ({java_type} buf)
                sig_arr = self._get_method_sig(name, [f"{java_type} {buf_name}"])
                if sig_arr not in self.emitted_signatures:
                    self.emitted_signatures.add(sig_arr)
                    doc_arr_dict = self._adapt_packed_doc(tailored_doc, buf_name, f"{buf_name}.length / {stride}", has_offset_arg=False)
                    doc_arr = generate_javadoc(doc_arr_dict, indent_spaces=8, argument_names=[buf_name], param_types=arr_param_types)
                    if doc_arr:
                        lines.append(doc_arr)
                    lines.append("        @NonNull")
                    lines.append(f"        public Builder {name}(@NonNull @Size(min = {stride}) {java_type} {buf_name}) {{")
                    lines.append(f"            return {name}({buf_name}, 0, {buf_name}.length / {stride});")
                    lines.append("        }")

            return "\n".join(lines)

        # Step 3: Chained fluent setter methods.
        native_name = f"nBuilder{name[0].upper() + name[1:]}"
        
        params_list = []
        call_args = ["mNativeBuilder"]
        arg_names = []
        
        size_param_consumers = {}
        for arg in method.get("arguments", []):
            cnt_name = get_size_param_attr(arg)
            if cnt_name:
                arr_in = self.get_array_input_info(arg["type"])
                if arr_in is not None:
                    stride = arr_in["size"] if arr_in else 1
                    nullability = arg["type"].get("nullability", "unspecified")
                    is_nullable = (nullability == "nullable")
                    size_param_consumers.setdefault(cnt_name, []).append((arg["name"], stride, is_nullable))

        overload_code = self._generate_convenience_overloads(
            method, effective_doc, "public ", "Builder", "@NonNull", None, size_param_consumers, indent=8
        )

        input_checks = []
        builder_param_types = {}
        # Step 4: Parameter iteration, unrolling vector components, and array parameter formatting.
        for arg in method.get("arguments", []):
            arg_name = arg["name"]
            req_type = arg["type"]
            
            if arg_name in size_param_consumers and len(size_param_consumers[arg_name]) == 1:
                ptr_name, stride, is_nullable = size_param_consumers[arg_name][0]
                safe_ptr_name = sanitize_identifier(ptr_name)
                div_str = f" / {stride}" if stride > 1 else ""
                if is_nullable:
                    call_args.append(f"{safe_ptr_name} != null ? {safe_ptr_name}.length{div_str} : 0")
                else:
                    call_args.append(f"{safe_ptr_name}.length{div_str}")
                continue

            if self.is_builder_buffer_arg(arg):
                safe_name = sanitize_identifier(arg_name)
                arg_names.append(safe_name)
                builder_param_types[safe_name] = "Buffer"
                arg_t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "") if isinstance(arg["type"], dict) else str(arg["type"])
                is_slice = "Slice" in arg_t_str
                if is_slice:
                    params_list.append(f"@NonNull Buffer {safe_name}")
                    params_list.append("@IntRange(from = 0) int size")
                    call_args.append(safe_name)
                    call_args.append("size")
                    arg_names.append("size")
                    builder_param_types["size"] = "int"
                else:
                    params_list.append(f"Buffer {safe_name}")
                    call_args.append(safe_name)
                continue

            size_param_name = get_size_param_attr(arg)
            arr_in = self.get_array_input_info(req_type)
            if arr_in:
                req_type_str = req_type.get("qualified_name") or req_type["cpp_name"] if isinstance(req_type, dict) else req_type
                is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                is_pointer = req_type.get("is_pointer", False)
                if not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                    sz = arr_in["size"]
                    for c in ["x", "y", "z", "w"][:sz]:
                        comp_name = sanitize_identifier(f"{arg_name}{c}")
                        scalar_java = arr_in["scalar"]
                        ann = "@LinearColor " if "LinearColor" in str(req_type) else ""
                        params_list.append(f"{ann}{scalar_java} {comp_name}")
                        call_args.append(comp_name)
                        arg_names.append(comp_name)
                        builder_param_types[comp_name] = scalar_java
                    continue

                j_type = arr_in["java"]
                size = arr_in["size"]
                nullability = req_type.get("nullability", "unspecified")
                
                if arr_in.get("is_custom_array"):
                    ann_prefix = f"{arr_in['annotation']} " if arr_in.get("annotation") else ""
                    if arr_in.get("is_slice"):
                        ann = f"@NonNull {ann_prefix}".strip()
                    elif nullability == "nonnull":
                        ann = f"@NonNull {ann_prefix}".strip()
                    elif nullability == "nullable":
                        ann = f"@Nullable {ann_prefix}".strip()
                    else:
                        ann = ann_prefix.strip()
                else:
                    ann = f"@NonNull @Size(min = {size})"
                    if is_pointer:
                        if nullability == "nullable":
                            ann = f"@Nullable @Size(min = {size})"
                        elif nullability == "unspecified":
                            ann = f"@Size(min = {size})"

                safe_name = sanitize_identifier(arg_name)
                p_str = f"{ann} {j_type} {safe_name}".strip()
                params_list.append(p_str)
                call_args.append(safe_name)
                arg_names.append(safe_name)
                builder_param_types[safe_name] = j_type
                continue

            safe_name = sanitize_identifier(arg_name)
            arg_names.append(safe_name)
            t_info, _ = self.resolve_type_info(req_type, silent=True)
            if t_info.get("is_struct"):
                struct_cls = t_info["struct_cls"]
                params_list.append(f"@NonNull {t_info['java']} {safe_name}")
                builder_param_types[safe_name] = t_info['java']
                leaves = self.get_flattened_leaves(struct_cls)
                for leaf in leaves:
                    getter_call = ".".join([safe_name] + leaf.get("getter_chain", [f"get{leaf['name']}()"]))
                    call_args.append(getter_call)
            elif t_info.get("is_string") or t_info.get("java") == "String":
                nullability = req_type.get("nullability", "unspecified") if isinstance(req_type, dict) else "unspecified"
                ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                params_list.append(f"{ann}String {safe_name}")
                call_args.append(safe_name)
                builder_param_types[safe_name] = "String"
            elif t_info.get("is_enum"):
                nullability = req_type.get("nullability", "unspecified")
                ann = "@Nullable " if nullability == "nullable" else "@NonNull "
                params_list.append(f"{ann}{t_info['java']} {safe_name}")
                if nullability == "nullable":
                    call_args.append(f"{safe_name} != null ? {safe_name}.toFilamentNative() : 0")
                else:
                    call_args.append(f"{safe_name}.toFilamentNative()")
                builder_param_types[safe_name] = t_info['java']
            elif t_info.get("is_filament_type"):
                nullability = req_type.get("nullability", "unspecified")
                if nullability == "nullable":
                    ann = "@Nullable "
                    call_args.append(f"{safe_name} != null ? {safe_name}.getNativeObject() : 0")
                elif nullability == "nonnull" or req_type.get("is_reference", False):
                    ann = "@NonNull "
                    call_args.append(f"{safe_name}.getNativeObject()")
                else:
                    ann = ""
                    call_args.append(f"{safe_name}.getNativeObject()")
                j_type = self.get_java_type(req_type)
                params_list.append(f"{ann}{j_type} {safe_name}".strip())
                builder_param_types[safe_name] = j_type
            elif t_info.get("is_value_object") or t_info.get("archetype") == "bitfield":
                field_name = t_info.get("field_name", "mSampler")
                j_type = self.get_java_type(req_type)
                params_list.append(f"@NonNull {j_type} {safe_name}")
                call_args.append(f"{safe_name}.{field_name}")
                builder_param_types[safe_name] = j_type
            else:
                j_type = self.get_java_type(req_type)
                ann = self.get_annotation(req_type)
                ann_str = f"{ann} " if ann else ""
                params_list.append(f"{ann_str}{j_type} {safe_name}")
                call_args.append(safe_name)
                builder_param_types[safe_name] = j_type

        javadoc = generate_javadoc(effective_doc, indent_spaces=8, argument_names=arg_names, param_types=builder_param_types)
        
        # Step 5: Javadoc generation, fluent chaining body, and buffer retention tracking.
        lines = []
        if overload_code:
            for oc in overload_code:
                lines.append(oc)
                lines.append("")
        if javadoc:
            lines.append(javadoc)
        lines.append("        @NonNull")
        lines.append(f"        public Builder {name}({', '.join(params_list)}) {{")
        for check in input_checks:
            lines.append(f"            {check}")
        for a in method.get("arguments", []):
            if self.is_builder_buffer_arg(a):
                safe_a = sanitize_identifier(a["name"])
                lines.append(f"            mRetainedBuffers.add({safe_a});")
        lines.append(f"            {native_name}({', '.join(call_args)});")
        lines.append("            return this;")
        lines.append("        }")

        # Step 6: Vector array overloads and single-buffer convenience overloads.
        vec_overloads = self.parent_context._generate_vector_array_overloads(
            method, effective_doc, "public ", "Builder", "@NonNull", is_builder=True
        )
        if vec_overloads:
            for vo in vec_overloads:
                lines.append("")
                lines.append(vo)
        buf_args = [a for a in method.get("arguments", []) if self.is_builder_buffer_arg(a)]
        if buf_args and len(buf_args) == 1:
            buf_arg = buf_args[0]
            buf_type_str = buf_arg["type"].get("qualified_name") or buf_arg["type"].get("cpp_name", "") if isinstance(buf_arg["type"], dict) else str(buf_arg["type"])
            is_slice_buf = "Slice" in buf_type_str
            size_param_name = get_size_param_attr(buf_arg)
            arg_names_in_m = [a["name"] for a in method.get("arguments", [])]
            if is_slice_buf or (size_param_name and size_param_name in arg_names_in_m):
                overload_params = []
                overload_call_args = []
                for a in method.get("arguments", []):
                    a_name = a["name"]
                    safe_a = sanitize_identifier(a_name)
                    if not is_slice_buf and a_name == size_param_name:
                        safe_buf = sanitize_identifier(buf_arg["name"])
                        overload_call_args.append(f"{safe_buf}.remaining()")
                    elif a_name == buf_arg["name"]:
                        overload_params.append(f"@NonNull Buffer {safe_a}")
                        overload_call_args.append(safe_a)
                        if is_slice_buf:
                            overload_call_args.append(f"{safe_a}.remaining()")
                    else:
                        j_type = self.get_java_type(a["type"])
                        ann = self.get_annotation(a["type"])
                        ann_str = f"{ann} " if ann else ""
                        overload_params.append(f"{ann_str}{j_type} {safe_a}".strip())
                        overload_call_args.append(safe_a)
                lines.append("")
                lines.append("        @NonNull")
                lines.append(f"        public Builder {name}({', '.join(overload_params)}) {{")
                lines.append(f"            return {name}({', '.join(overload_call_args)});")
                lines.append("        }")
        return "\n".join(lines)

    def generate_builder_native_decls(self):
        """Generates private static native JNI declarations for inner Builder class.

        Synthesizes the native methods utilized by Builder constructors, finalizers,
        setters, and terminal build methods:
        - `nCreateBuilder(...)` native allocator functions.
        - `nDestroyBuilder(long nativeBuilder)` native destructor.
        - `nBuilderBuild(...)` terminal construction functions.
        - `nBuilderMethodName(...)` fluent property mutators.

        Returns:
            List[str]: List of formatted single-line Java native declaration statements.
        """
        if self.parent_context and self.parent_context.name == "Engine":
            return []

        decls = []

        # Step 1: Builder creation constructors (nCreateBuilder).
        for m in self.methods:
            if m.get("is_constructor"):
                args = m.get("arguments", [])
                params = []
                for arg in args:
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    j_type = "int" if t_info.get("is_enum") else self.get_java_type(arg["type"])
                    params.append(f"{j_type} {sanitize_identifier(arg['name'])}")
                decls.append(f"    private static native long nCreateBuilder({', '.join(params)});")
        if not any(m.get("is_constructor") for m in self.methods):
            decls.append("    private static native long nCreateBuilder();")

        # Step 2: Builder destruction (nDestroyBuilder).
        decls.append("    private static native void nDestroyBuilder(long nativeBuilder);")

        # Step 3: Builder terminal build() and method native declarations.
        for method in self.methods:
            if method.get("is_constructor"):
                continue
            for exp_m in self._expand_method(method):
                m_ir = exp_m["ir"]
                m_name = m_ir["name"]
                eff_name = get_effective_method_name(m_ir)
                if m_name == "build":
                    args = m_ir.get("arguments", [])
                    if len(args) == 1 and ("Engine" in args[0]["type"].get("cpp_name", "") or "Engine" in args[0]["type"].get("qualified_name", "")):
                        decls.append("    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);")
                    elif len(args) == 2:
                        decls.append("    private static native boolean nBuilderBuild(long nativeBuilder, long nativeEngine, int entity);")
                    elif len(args) == 0:
                        decls.append("    private static native long nBuilderBuild(long nativeBuilder);")
                elif self.is_packed_buffer_method(m_ir):
                    info = self.get_packed_buffer_info(m_ir)
                    mode = exp_m.get("buffer_mode", "buffer")
                    native_name = f"nBuilder{eff_name[0].upper() + eff_name[1:]}"
                    stride = info["stride"]
                    buf_name = info["buf_name"]
                    cnt_name = info["cnt_name"]
                    array_offset_name = info["array_offset_name"]
                    if mode == "buffer":
                        decls.append(f"    private static native int {native_name}(long nativeBuilder, int {cnt_name}, @NonNull Buffer {buf_name}, int remaining);")
                    else:
                        java_type = info.get("java_type", "float[]")
                        decls.append(f"    private static native void {native_name}(long nativeBuilder, @NonNull @Size(min = {stride}) {java_type} {buf_name}, int {cnt_name}, int {array_offset_name});")
                else:
                    native_name = f"nBuilder{eff_name[0].upper() + eff_name[1:]}"
                    params = ["long nativeBuilder"]
                    for arg in m_ir.get("arguments", []):
                        arg_name = sanitize_identifier(arg["name"])
                        cpp_type = arg["type"]
                        if self.is_builder_buffer_arg(arg):
                            params.append(f"Buffer {arg_name}")
                            arg_type_str = cpp_type.get("qualified_name") or cpp_type.get("cpp_name", "") if isinstance(cpp_type, dict) else str(cpp_type)
                            if "Slice" in arg_type_str:
                                params.append("int size")
                            continue
                        size_param_name = get_size_param_attr(arg)
                        arr_in = self.get_array_input_info(cpp_type)
                        if arr_in:
                            req_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else cpp_type
                            is_matrix = re.search(r"::mat\d", req_type_str) or "::TMat" in req_type_str
                            is_pointer = cpp_type.get("is_pointer", False)
                            if not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                                sz = arr_in["size"]
                                scalar_java = arr_in["scalar"]
                                for c in ["x", "y", "z", "w"][:sz]:
                                    params.append(f"{scalar_java} {arg_name}{c}")
                                continue
                            else:
                                j_arr_type = arr_in["java"]
                                ann = self.get_annotation(cpp_type)
                                ann_str = f"{ann} " if ann else ""
                                params.append(f"{ann_str}{j_arr_type} {arg_name}")
                        else:
                            t_info, _ = self.resolve_type_info(cpp_type, silent=True)
                            if t_info.get("is_struct"):
                                struct_cls = t_info["struct_cls"]
                                leaves = self.get_flattened_leaves(struct_cls)
                                for leaf in leaves:
                                    params.append(f"{leaf['java_type']} {leaf['param_name']}")
                            elif t_info.get("is_enum"):
                                params.append(f"int {arg_name}")
                            elif t_info.get("archetype") == "bitfield":
                                storage_type = t_info.get("storage_type", "int")
                                params.append(f"{storage_type} {arg_name}")
                            elif t_info.get("is_filament_type") or t_info.get("is_handle") or t_info.get("is_value_object"):
                                params.append(f"long {arg_name}")
                            else:
                                j_type = self.get_java_type(cpp_type)
                                params.append(f"{j_type} {arg_name}")
                    decls.append(f"    private static native void {native_name}({', '.join(params)});")
        return decls

    def _generate_engine_methods_java(self):
        return """    private Engine(long nativeEngine, @Nullable Config config) {
        mNativeObject = nativeEngine;
        mConfig = config;
    }

    @NonNull
    public static Engine create() {
        return new Builder().build();
    }

    @NonNull
    public static Engine create(@NonNull Backend backend) {
        return new Builder()
            .backend(backend)
            .build();
    }

    @NonNull
    public static Engine create(@NonNull Object sharedContext) {
        return new Builder()
            .sharedContext(sharedContext)
            .build();
    }

    public void destroy() {
        nDestroyEngine(getNativeObject());
        clearNativeObject();
    }

    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface) {
        return createSwapChain(surface, SwapChainFlags.CONFIG_DEFAULT);
    }

    @NonNull
    public SwapChain createSwapChain(@NonNull Object surface, long flags) {
        if (Platform.get().validateSurface(surface)) {
            long nativeSwapChain = nCreateSwapChain(getNativeObject(), surface, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, surface);
        }
        throw new IllegalArgumentException("Invalid surface " + surface);
    }

    @NonNull
    public SwapChain createSwapChain(int width, int height, long flags) {
        if (width >= 0 && height >= 0) {
            long nativeSwapChain =
                nCreateSwapChainHeadless(getNativeObject(), width, height, flags);
            if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
            return new SwapChain(nativeSwapChain, null);
        }
        throw new IllegalArgumentException("Invalid parameters");
    }

    @NonNull
    public SwapChain createSwapChain(int width, int height) {
        return createSwapChain(width, height, SwapChainFlags.CONFIG_DEFAULT);
    }

    @NonNull
    public SwapChain createSwapChainFromNativeSurface(@NonNull NativeSurface surface, long flags) {
        long nativeSwapChain =
                nCreateSwapChainFromRawPointer(getNativeObject(), surface.getNativeObject(), flags);
        if (nativeSwapChain == 0) throw new IllegalStateException("Couldn't create SwapChain");
        return new SwapChain(nativeSwapChain, surface);
    }


    public void destroySwapChain(@NonNull SwapChain swapChain) {
        assertDestroy(nDestroySwapChain(getNativeObject(), swapChain.getNativeObject()));
        swapChain.clearNativeObject();
    }

    public void destroyView(@NonNull View view) {
        assertDestroy(nDestroyView(getNativeObject(), view.getNativeObject()));
        view.clearNativeObject();
    }

    public void destroyRenderer(@NonNull Renderer renderer) {
        assertDestroy(nDestroyRenderer(getNativeObject(), renderer.getNativeObject()));
        renderer.clearNativeObject();
    }

    public void destroyScene(@NonNull Scene scene) {
        assertDestroy(nDestroyScene(getNativeObject(), scene.getNativeObject()));
        scene.clearNativeObject();
    }

    public void destroyStream(@NonNull Stream stream) {
        assertDestroy(nDestroyStream(getNativeObject(), stream.getNativeObject()));
        stream.clearNativeObject();
    }

    public void destroyFence(@NonNull Fence fence) {
        assertDestroy(nDestroyFence(getNativeObject(), fence.getNativeObject()));
        fence.clearNativeObject();
    }

    public void destroyFramePacer(@NonNull FramePacer framePacer) {
        assertDestroy(nDestroyFramePacer(getNativeObject(), framePacer.getNativeObject()));
        framePacer.clearNativeObject();
    }

    public void destroyBufferObject(@NonNull BufferObject bufferObject) {
        assertDestroy(nDestroyBufferObject(getNativeObject(), bufferObject.getNativeObject()));
        bufferObject.clearNativeObject();
    }

    public void destroyIndexBuffer(@NonNull IndexBuffer indexBuffer) {
        assertDestroy(nDestroyIndexBuffer(getNativeObject(), indexBuffer.getNativeObject()));
        indexBuffer.clearNativeObject();
    }

    public void destroyInstanceBuffer(@NonNull InstanceBuffer instanceBuffer) {
        assertDestroy(nDestroyInstanceBuffer(getNativeObject(), instanceBuffer.getNativeObject()));
        instanceBuffer.clearNativeObject();
    }

    public void destroyVertexBuffer(@NonNull VertexBuffer vertexBuffer) {
        assertDestroy(nDestroyVertexBuffer(getNativeObject(), vertexBuffer.getNativeObject()));
        vertexBuffer.clearNativeObject();
    }

    public void destroySkinningBuffer(@NonNull SkinningBuffer skinningBuffer) {
        assertDestroy(nDestroySkinningBuffer(getNativeObject(), skinningBuffer.getNativeObject()));
        skinningBuffer.clearNativeObject();
    }

    public void destroyMorphTargetBuffer(@NonNull MorphTargetBuffer morphTargetBuffer) {
        assertDestroy(nDestroyMorphTargetBuffer(getNativeObject(), morphTargetBuffer.getNativeObject()));
        morphTargetBuffer.clearNativeObject();
    }

    public void destroyIndirectLight(@NonNull IndirectLight ibl) {
        assertDestroy(nDestroyIndirectLight(getNativeObject(), ibl.getNativeObject()));
        ibl.clearNativeObject();
    }

    public void destroyMaterial(@NonNull Material material) {
        assertDestroy(nDestroyMaterial(getNativeObject(), material.getNativeObject()));
        material.clearNativeObject();
    }

    public void destroyMaterialInstance(@NonNull MaterialInstance materialInstance) {
        assertDestroy(nDestroyMaterialInstance(getNativeObject(), materialInstance.getNativeObject()));
        materialInstance.clearNativeObject();
    }

    public void destroySkybox(@NonNull Skybox skybox) {
        assertDestroy(nDestroySkybox(getNativeObject(), skybox.getNativeObject()));
        skybox.clearNativeObject();
    }

    public void destroyColorGrading(@NonNull ColorGrading colorGrading) {
        assertDestroy(nDestroyColorGrading(getNativeObject(), colorGrading.getNativeObject()));
        colorGrading.clearNativeObject();
    }

    public void destroyTexture(@NonNull Texture texture) {
        assertDestroy(nDestroyTexture(getNativeObject(), texture.getNativeObject()));
        texture.clearNativeObject();
    }

    public void destroyRenderTarget(@NonNull RenderTarget target) {
        assertDestroy(nDestroyRenderTarget(getNativeObject(), target.getNativeObject()));
        target.clearNativeObject();
    }

    public void destroyEntity(@Entity int entity) {
        nDestroyEntity(getNativeObject(), entity);
    }

    public void destroy(@NonNull SwapChain swapChain) { destroySwapChain(swapChain); }
    public void destroy(@NonNull View view) { destroyView(view); }
    public void destroy(@NonNull Renderer renderer) { destroyRenderer(renderer); }
    public void destroy(@NonNull Scene scene) { destroyScene(scene); }
    public void destroy(@NonNull Stream stream) { destroyStream(stream); }
    public void destroy(@NonNull Fence fence) { destroyFence(fence); }
    public void destroy(@NonNull FramePacer framePacer) { destroyFramePacer(framePacer); }
    public void destroy(@NonNull BufferObject bufferObject) { destroyBufferObject(bufferObject); }
    public void destroy(@NonNull IndexBuffer indexBuffer) { destroyIndexBuffer(indexBuffer); }
    public void destroy(@NonNull InstanceBuffer instanceBuffer) { destroyInstanceBuffer(instanceBuffer); }
    public void destroy(@NonNull VertexBuffer vertexBuffer) { destroyVertexBuffer(vertexBuffer); }
    public void destroy(@NonNull SkinningBuffer skinningBuffer) { destroySkinningBuffer(skinningBuffer); }
    public void destroy(@NonNull MorphTargetBuffer morphTargetBuffer) { destroyMorphTargetBuffer(morphTargetBuffer); }
    public void destroy(@NonNull IndirectLight ibl) { destroyIndirectLight(ibl); }
    public void destroy(@NonNull Material material) { destroyMaterial(material); }
    public void destroy(@NonNull MaterialInstance materialInstance) { destroyMaterialInstance(materialInstance); }
    public void destroy(@NonNull Skybox skybox) { destroySkybox(skybox); }
    public void destroy(@NonNull ColorGrading colorGrading) { destroyColorGrading(colorGrading); }
    public void destroy(@NonNull Texture texture) { destroyTexture(texture); }
    public void destroy(@NonNull RenderTarget target) { destroyRenderTarget(target); }
    public void destroy(@Entity int entity) { destroyEntity(entity); }

    private static void assertDestroy(boolean success) {
        if (!success) {
            throw new IllegalStateException("Object could not be destroyed (or already destroyed)");
        }
    }"""

    def _get_engine_native_decls(self):
        return [
            "    private static native void nDestroyEngine(long nativeEngine);",
            "    private static native long nCreateSwapChain(long nativeEngine, Object nativeWindow, long flags);",
            "    private static native long nCreateSwapChainHeadless(long nativeEngine, int width, int height, long flags);",
            "    private static native long nCreateSwapChainFromRawPointer(long nativeEngine, long pointer, long flags);",
            "    private static native boolean nDestroyBufferObject(long nativeEngine, long nativeBufferObject);",
            "    private static native boolean nDestroyColorGrading(long nativeEngine, long nativeColorGrading);",
            "    private static native boolean nDestroyFence(long nativeEngine, long nativeFence);",
            "    private static native boolean nDestroyFramePacer(long nativeEngine, long nativeFramePacer);",
            "    private static native boolean nDestroyIndexBuffer(long nativeEngine, long nativeIndexBuffer);",
            "    private static native boolean nDestroyIndirectLight(long nativeEngine, long nativeIndirectLight);",
            "    private static native boolean nDestroyInstanceBuffer(long nativeEngine, long nativeInstanceBuffer);",
            "    private static native boolean nDestroyMaterial(long nativeEngine, long nativeMaterial);",
            "    private static native boolean nDestroyMaterialInstance(long nativeEngine, long nativeMaterialInstance);",
            "    private static native boolean nDestroyMorphTargetBuffer(long nativeEngine, long nativeMorphTargetBuffer);",
            "    private static native boolean nDestroyRenderTarget(long nativeEngine, long nativeTarget);",
            "    private static native boolean nDestroyRenderer(long nativeEngine, long nativeRenderer);",
            "    private static native boolean nDestroyScene(long nativeEngine, long nativeScene);",
            "    private static native boolean nDestroySkinningBuffer(long nativeEngine, long nativeSkinningBuffer);",
            "    private static native boolean nDestroySkybox(long nativeEngine, long nativeSkybox);",
            "    private static native boolean nDestroyStream(long nativeEngine, long nativeStream);",
            "    private static native boolean nDestroySwapChain(long nativeEngine, long nativeSwapChain);",
            "    private static native boolean nDestroyTexture(long nativeEngine, long nativeTexture);",
            "    private static native boolean nDestroyVertexBuffer(long nativeEngine, long nativeVertexBuffer);",
            "    private static native boolean nDestroyView(long nativeEngine, long nativeView);",
            "    private static native void nDestroyEntity(long nativeEngine, int entity);",
            "    private static native long nCreateBuilder();",
            "    private static native void nDestroyBuilder(long nativeBuilder);",
            "    private static native void nSetBuilderBackend(long nativeBuilder, long backend);",
            "    private static native void nSetBuilderConfig(long nativeBuilder, long commandBufferSizeMB,",
            "            long perRenderPassArenaSizeMB, long driverHandleArenaSizeMB,",
            "            long minCommandBufferSizeMB, long perFrameCommandsSizeMB, long jobSystemThreadCount,",
            "            boolean disableParallelShaderCompile, int stereoscopicType, long stereoscopicEyeCount,",
            "            long resourceAllocatorCacheSizeMB, long resourceAllocatorCacheMaxAge,",
            "            boolean disableHandleUseAfterFreeCheck,",
            "            int preferredShaderLanguage,",
            "            boolean forceGLES2Context, boolean assertNativeWindowIsValid,",
            "            int gpuContextPriority,",
            "            long sharedUboInitialSizeInBytes,",
            "            boolean enableMultipleDirectionalLights);",
            "    private static native void nSetBuilderFeatureLevel(long nativeBuilder, int ordinal);",
            "    private static native void nSetBuilderSharedContext(long nativeBuilder, long sharedContext);",
            "    private static native void nSetBuilderPaused(long nativeBuilder, boolean paused);",
            "    private static native void nSetBuilderFeature(long nativeBuilder, String name, boolean value);",
            "    private static native void nSetBuilderColorGrading(long nativeBuilder, long nativeColorGradingBuilder);",
            "    private static native long nBuilderBuild(long nativeBuilder);",
        ]

    def _generate_engine_builder_java(self):
        return """    /**
     * Use <code>Builder</code> to construct an <code>Engine</code> object instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;
        private Config mConfig;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Sets the {@link Backend} for the Engine.
         *
         * @param backend Driver backend to use
         * @return A reference to this Builder for chaining calls.
         */
        public Builder backend(Backend backend) {
            nSetBuilderBackend(mNativeBuilder, backend.ordinal());
            return this;
        }

        /**
         * Sets a sharedContext for the Engine.
         *
         * @param sharedContext  A platform-dependant OpenGL context used as a shared context
         *                       when creating filament's internal context. On Android this parameter
         *                       <b>must be</b> an instance of {@link android.opengl.EGLContext}.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder sharedContext(Object sharedContext) {
            if (Platform.get().validateSharedContext(sharedContext)) {
                nSetBuilderSharedContext(mNativeBuilder,
                        Platform.get().getSharedContextNativeHandle(sharedContext));
                return this;
            }
            throw new IllegalArgumentException("Invalid shared context " + sharedContext);
        }

        /**
         * Configure the Engine with custom parameters.
         *
         * @param config A {@link Config} object
         * @return A reference to this Builder for chaining calls.
         */
        public Builder config(Config config) {
            mConfig = config;
            nSetBuilderConfig(mNativeBuilder, config.commandBufferSizeMB,
                    config.perRenderPassArenaSizeMB, config.driverHandleArenaSizeMB,
                    config.minCommandBufferSizeMB, config.perFrameCommandsSizeMB,
                    config.jobSystemThreadCount, config.disableParallelShaderCompile,
                    config.stereoscopicType.ordinal(), config.stereoscopicEyeCount,
                    config.resourceAllocatorCacheSizeMB, config.resourceAllocatorCacheMaxAge,
                    config.disableHandleUseAfterFreeCheck,
                    config.preferredShaderLanguage.ordinal(),
                    config.forceGLES2Context, config.assertNativeWindowIsValid,
                    config.gpuContextPriority.ordinal(),
                    config.sharedUboInitialSizeInBytes,
                    config.enableMultipleDirectionalLights);
            return this;
        }

        /**
         * Sets the initial featureLevel for the Engine.
         *
         * @param featureLevel The feature level at which initialize Filament.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder featureLevel(FeatureLevel featureLevel) {
            nSetBuilderFeatureLevel(mNativeBuilder, featureLevel.ordinal());
            return this;
        }

        /**
         * Sets the initial paused state of the rendering thread.
         *
         * @param paused Whether to start the rendering thread paused.
         * @return A reference to this Builder for chaining calls.
         */
        public Builder paused(boolean paused) {
            nSetBuilderPaused(mNativeBuilder, paused);
            return this;
        }

        /**
         * Set a feature flag value. This is the only way to set constant feature flags.
         * @param name feature name
         * @param value true to enable, false to disable
         * @return A reference to this Builder for chaining calls.
         */
        public Builder feature(@NonNull String name, boolean value) {
            nSetBuilderFeature(mNativeBuilder, name, value);
            return this;
        }

        /**
         * Sets the builder used to create the default ColorGrading object.
         * @param colorGrading Builder used to create the default color grading.
         * @return A reference to this Builder for chaining calls.
         */
        @NonNull
        public Builder colorGrading(@NonNull ColorGrading.Builder colorGrading) {
            nSetBuilderColorGrading(mNativeBuilder, colorGrading.getNativeBuilder());
            return this;
        }

        /**
         * Creates an instance of Engine
         *
         * @return A newly created <code>Engine</code>, or <code>null</code> if the GPU driver couldn't
         *         be initialized, for instance if it doesn't support the right version of OpenGL or
         *         OpenGL ES.
         *
         * @throws Error if there isn't enough memory to allocate the command buffer.
         */
        public Engine build() {
            long nativeEngine = nBuilderBuild(mNativeBuilder);
            if (nativeEngine == 0) throw new IllegalStateException("Couldn't create Engine");
            return new Engine(nativeEngine, mConfig);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }"""


