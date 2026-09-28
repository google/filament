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

"""C++ to Java and JNI type resolution, marshalling, and memory layout engine.

This module provides :class:`TypeResolver`, the primary type translation and memory
interoperability engine for the JavaGen binding generator. It evaluates raw C++ AST
type structures extracted from headers by libclang, maps them to target Java language
primitives and AndroidX annotations, and generates zero-cost C++ JNI marshalling expressions.

Architectural Position and Core Responsibilities:
-------------------------------------------------
1. **Type Canonicalization & Namespace Normalization**:
   - Strips nested C++ engine namespaces (`filament::`, `utils::`, `math::`, `backend::`).
   - Recursively resolves `typedef` and C++11 `using` aliases against class-level and
     file-level symbol tables.
   - Detects and preserves specialized semantic types, such as `@LinearColor` vectors.
2. **Type Pattern Matching & Multi-Tier Resolution Pipeline**:
   - Tier 1: Static exact table lookups in :data:`~javagen.config.TYPE_MAP` and
     :data:`~javagen.config.MATH_TYPES`.
   - Tier 2: Regular expression pattern matchers (:data:`~javagen.config.TYPE_PATTERNS`)
     for parameterized STL templates and pointer/reference combinations.
   - Tier 3: Modifier stripping (cv-qualifiers `const`, `volatile`, reference modifiers `&`, `&&`).
   - Tier 4: Fixed-size C-style arrays (`float[3]`, `math::mat4[4]`).
   - Tier 5: Functional callback templates (`utils::Invocable<void(int)>`).
   - Tier 6: View slices (`utils::Slice<T>`, `utils::Slice<const T>`).
   - Tier 7: Standard containers (`std::vector<T>`, `utils::FixedCapacityVector<T>`).
   - Tier 8: Scoped and unscoped C++ enums (with automatic bitmask/flags classification).
   - Tier 9: Value classes and bitfields (:data:`~javagen.config.VALUE_TYPES`).
   - Tier 10: Flattened aggregate POD structs (`Box`, `Viewport`).
   - Tier 11: Filament engine entity handles passed as opaque native pointers (`jlong`).
3. **AAPCS64 Register-Passed Struct Flattening**:
   - On 64-bit ARM (AAPCS64), small Plain-Old-Data (POD) aggregate structs cannot be efficiently
     passed across the JNI boundary as Java object instances without heap allocation and JNI
     GetFieldID overhead.
   - :class:`TypeResolver` recursively decomposes aggregate structs into primitive scalar leaves
     (`get_flattened_leaves`), allowing them to be passed directly in floating-point (s0-s7)
     and integer (x0-x7) CPU registers across the JNI boundary.
   - Emits C++ reconstruction expressions (`generate_cpp_struct_construction`) and JNI
     cleanup routines (`generate_cpp_struct_cleanup`).
4. **Dual-Mode Buffer Overloads**:
   - Analyzes packed buffer methods (e.g. `setBones`, `setTransforms`) accepting continuous
     memory strides.
   - Automatically provisions dual overloads: one accepting direct `java.nio.Buffer` instances
     for zero-copy native consumption, and one accepting primitive Java arrays (`float[]`)
     with array offsets.
"""

import re
import sys
from typing import Any, Dict, List, Optional, Tuple, Union

from .config import (
    KNOWN_CLASSES,
    MATH_TYPES,
    TYPE_MAP,
    TYPE_PATTERNS,
    VALUE_TYPES,
)
from .utils import (
    get_effective_method_name,
    get_scoped_cpp_name,
    get_size_param_attr,
    is_reserved_or_padding_field,
    sanitize_identifier,
)


class TypeResolver:
    """Resolves C++ IR types to target Java and JNI constructs.

    Coordinates type canonicalization, alias unwrapping, aggregate struct decomposition,
    and native marshalling expressions for an active :class:`~javagen.context.ClassContext`.

    Attributes:
        context: The parent :class:`~javagen.context.ClassContext` representing the active class.
    """

    def __init__(self, context: Any) -> None:
        """Initialize the TypeResolver bound to a ClassContext.

        Args:
            context: The parent ClassContext representing the active class.
        """
        self.context = context

    @property
    def name(self) -> str:
        """Simple name of the active class."""
        return self.context.name

    @property
    def cpp_name(self) -> str:
        """C++ identifier of the active class."""
        return self.context.cpp_name

    @property
    def aliases(self) -> Dict[str, str]:
        """Class-level and file-level type alias mapping."""
        return self.context.aliases

    @property
    def enum_map(self) -> Dict[str, Dict[str, Any]]:
        """Dictionary of registered enums in this class context."""
        return self.context.enum_map

    @property
    def current_method(self) -> Optional[str]:
        """Name of the method currently being emitted, if any."""
        return getattr(self.context, "current_method", None)

    @property
    def diagnostic_mode(self) -> bool:
        """Whether diagnostic warnings are enabled for unmapped types."""
        return getattr(self.context, "diagnostic_mode", False)

    def strip_namespaces(self, t: str) -> str:
        """Strip root Filament and standard namespaces from a C++ type string.

        Normalizes qualified names such as `filament::Engine`, `utils::Entity`,
        and `filament::math::float3` into their unqualified identifiers (`Engine`,
        `Entity`, `float3`) while preserving nested sub-namespaces like `backend::`.

        Input-to-Output Examples:
            - `"filament::Engine"` -> `"Engine"`
            - `"filament::backend::BufferDescriptor"` -> `"backend::BufferDescriptor"`
            - `"utils::Entity"` -> `"Entity"`
            - `"filament::math::float3"` -> `"float3"`

        Args:
            t: Fully-qualified or scoped C++ type string.

        Returns:
            Type string with root engine namespaces removed.
        """
        # Step 1: Normalize nested backend namespace prefix
        t = t.replace("filament::backend::", "backend::")

        # Step 2: Strip engine and utility namespaces
        t = t.replace("filament::", "")
        t = t.replace("utils::", "")
        t = t.replace("math::", "")
        return t

    def is_unsupported_type(self, type_dict: Dict[str, Any]) -> bool:
        """Check if a type is fundamentally unsupported by Java bindings.

        Identifies types that cannot be safely or practically marshalled across the
        JNI boundary, such as low-level raw texture sampler parameters, non-standard
        completion handlers, raw `std::function` objects without `Invocable` wrappers,
        or unresolved unknown types.

        Args:
            type_dict: IR type representation dictionary.

        Returns:
            True if type cannot be bound to Java, False otherwise.
        """
        # Step 1: Extract qualified and C++ type names
        t_name = type_dict.get("qualified_name") or type_dict.get("cpp_name", "")

        # Step 2: Suppress low-level internal engine handlers and raw callbacks
        if "SamplerParams" in t_name or "CallbackHandler" in t_name or "AsyncCompletionCallback" in t_name:
            return True

        # Step 3: Suppress un-wrapped std::function objects (require utils::Invocable)
        if "std::function" in t_name or "std::__1::function" in t_name:
            return True

        # Step 4: Check if resolution flags this type as unknown
        t_info, _ = self.resolve_type_info(type_dict, silent=True)
        return t_info.get("is_unknown", False)

    def is_buffer_descriptor_method(self, method: Dict[str, Any]) -> bool:
        """Check if any method argument accepts a raw backend::BufferDescriptor.

        Filament's `BufferDescriptor` requires specialized JNI handling because the native
        engine takes asynchronous ownership of the memory and signals completion via a
        release callback. (PixelBufferDescriptor is handled separately as an inline value class).

        Args:
            method: Method dictionary from IR.

        Returns:
            True if method accepts BufferDescriptor (excluding PixelBufferDescriptor).
        """
        for arg in method.get("arguments", []):
            t = arg["type"]
            q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
            if "BufferDescriptor" in q and "PixelBufferDescriptor" not in q:
                return True
        return False

    def is_pixel_buffer_descriptor_method(self, method: Dict[str, Any]) -> bool:
        """Check if any method argument accepts a backend::PixelBufferDescriptor.

        Args:
            method: Method dictionary from IR.

        Returns:
            True if method accepts PixelBufferDescriptor, False otherwise.
        """
        for arg in method.get("arguments", []):
            t = arg["type"]
            q = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
            if "PixelBufferDescriptor" in q:
                return True
        return False

    def is_async_callback_method(self, method: Dict[str, Any]) -> bool:
        """Check if a method follows the asynchronous handler + callback completion pattern.

        Recognizes asynchronous methods returning `void` that accept both a `CallbackHandler*`
        (an executor/handler dispatch token) and a `utils::Invocable<...>` callback.

        Args:
            method: Method dictionary from IR.

        Returns:
            True if method matches the async callback pattern.
        """
        # Step 1: Verify return type is void
        ret = method.get("return_type", {})
        ret_cat = ret.get("category") if isinstance(ret, dict) else ""
        ret_name = ret.get("cpp_name", "") if isinstance(ret, dict) else ""
        if ret_cat != "void" and ret_name != "void":
            return False

        # Step 2: Verify presence of CallbackHandler argument
        args = method.get("arguments", [])
        has_handler = any(
            "CallbackHandler" in (a["type"].get("qualified_name") or a["type"].get("cpp_name", ""))
            for a in args
        )
        if not has_handler:
            return False

        # Step 3: Verify presence of Invocable callback argument
        has_invocable = any(
            "Invocable" in (a["type"].get("qualified_name") or a["type"].get("cpp_name", ""))
            for a in args
        )
        if not has_invocable:
            return False

        # Step 4: Discard if method also accepts BufferDescriptor or raw user void pointers
        for a in args:
            t_str = a["type"].get("qualified_name") or a["type"].get("cpp_name", "")
            if "AsyncCompletionCallback" in t_str or "BufferDescriptor" in t_str:
                return False
            if a["type"].get("is_pointer") and ("void" in t_str or a.get("name") == "user"):
                return False

        return True

    def get_struct_float_stride(self, struct_cls: Optional[Dict[str, Any]]) -> Optional[int]:
        """Compute the total contiguous float element stride for a pure-float struct.

        Inspects struct fields to verify if all members are homogeneous 32-bit floats
        or standard math vectors/matrices composed of floats (e.g. `float3`, `mat4f`).
        If homogeneous, computes the total scalar stride so that packed array buffers
        can be marshalled directly as a continuous `float[]` buffer.

        Stride Mappings:
            - `float`                 -> 1
            - `float2` / `TVec2<float>` -> 2
            - `float3` / `TVec3<float>` -> 3
            - `float4` / `TVec4<float>` -> 4
            - `quatf` / `TQuaternion<float>` -> 4
            - `mat3f` / `TMat33<float>` -> 9
            - `mat4f` / `TMat44<float>` -> 16

        Args:
            struct_cls: Struct class dictionary from IR.

        Returns:
            Total float scalar stride count, or None if struct contains non-float fields.
        """
        if not struct_cls or not isinstance(struct_cls, dict):
            return None
        total = 0
        for f in struct_cls.get("fields", []):
            ft = f["type"]
            q = ft.get("qualified_name") or ft.get("cpp_name", "") if isinstance(ft, dict) else str(ft)
            clean_q = self.strip_namespaces(q).replace("const ", "").replace("&", "").strip()
            if clean_q == "float" or q == "float":
                total += 1
            elif "TVec2<float>" in q or clean_q in ("float2", "math::float2"):
                total += 2
            elif "TVec3<float>" in q or clean_q in ("float3", "math::float3"):
                total += 3
            elif "TVec4<float>" in q or clean_q in ("float4", "math::float4"):
                total += 4
            elif "TQuaternion<float>" in q or clean_q in ("quatf", "math::quatf"):
                total += 4
            elif "TMat33<float>" in q or clean_q in ("mat3f", "math::mat3f"):
                total += 9
            elif "TMat44<float>" in q or clean_q in ("mat4f", "math::mat4f"):
                total += 16
            else:
                return None
        return total if total > 0 else None

    def is_attribute_bitset(self, type_dict: Optional[Dict[str, Any]]) -> bool:
        """Check if a type represents Filament's internal AttributeBitset.

        Args:
            type_dict: IR type representation dictionary.

        Returns:
            True if type is AttributeBitset, False otherwise.
        """
        if not type_dict or not isinstance(type_dict, dict):
            return False
        cpp_name = type_dict.get("cpp_name", "")
        return cpp_name == "AttributeBitset" or cpp_name.endswith("::AttributeBitset")

    def is_builder_buffer_arg(self, arg: Dict[str, Any]) -> bool:
        """Check if a builder setter argument passes a raw buffer pointer.

        When a Builder setter accepts a buffer pointer paired with a size parameter,
        the native C++ builder stores the raw pointer internally until `build()` is called.
        The Java binding must retain a reference to the incoming Java `Buffer` object
        on the Builder instance to ensure the garbage collector does not prematurely
        reclaim the memory before `build()` executes.

        Args:
            arg: Argument dictionary from method IR.

        Returns:
            True if argument is a buffer pointer requiring AutoBuffer retention.
        """
        t = arg.get("type", {})
        cpp_name = t.get("cpp_name", "") if isinstance(t, dict) else str(t)
        if re.search(r"\bSlice<\s*(?:const\s+)?(uint8_t|char|unsigned char|void)\s*>", cpp_name):
            return True
        size_param = get_size_param_attr(arg)
        if not size_param:
            return False
        if cpp_name in ("const char*", "char*", "std::string_view", "string_view"):
            return False
        is_pointer = t.get("is_pointer", False) if isinstance(t, dict) else ("*" in cpp_name)
        if not is_pointer:
            return False
        if self.get_array_input_info(t) is not None:
            return False
        return True

    def is_packed_buffer_method(self, method: Dict[str, Any]) -> bool:
        """Check if a method qualifies for packed buffer overload emission.

        Args:
            method: Method dictionary from IR.

        Returns:
            True if packed buffer metadata is present, False otherwise.
        """
        return self.get_packed_buffer_info(method) is not None

    def get_packed_buffer_info(self, method: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Extract packed buffer metadata (buffer argument, size parameter, element stride).

        Used for dual-mode APIs like `setBones`, where both `Buffer` and `float[]` overloads
        with automatic element stride calculations are generated.

        Args:
            method: Method dictionary from IR.

        Returns:
            Dictionary containing packed buffer arguments and stride, or None.
        """
        args = method.get("arguments", [])

        # Step 1A: Check for Slice parameter with stride > 1 and float scalar (e.g. Bone, mat4f)
        for idx, a in enumerate(args):
            t = a.get("type", {})
            nullability = t.get("nullability", "unspecified") if isinstance(t, dict) else "unspecified"
            if nullability == "nullable":
                continue

            arr_in = self.get_array_input_info(t)
            if arr_in and arr_in.get("is_slice"):
                stride = arr_in.get("size", 1)
                if stride > 1 and arr_in.get("scalar") in ("float", "double", "int", "long", "short", "byte"):
                    name = get_effective_method_name(method)
                    buf_name = sanitize_identifier(a["name"])
                    cnt_name = "boneCount" if "skinning" in name else "count"

                    offset_arg = next((arg for arg in args if arg["name"] in ("offset", "byteOffset")), None)
                    offset_idx = args.index(offset_arg) if offset_arg else -1
                    has_existing_offset = (offset_arg is not None)
                    array_offset_name = "arrayOffset" if has_existing_offset else "offset"

                    matched_indices = {idx}
                    if offset_idx != -1:
                        matched_indices.add(offset_idx)
                    if max(matched_indices) < len(args) - 1:
                        continue

                    pre_args = [arg for i, arg in enumerate(args) if i not in matched_indices]

                    if getattr(self.context, "archetype", "") == "builder":
                        native_name = f"nBuilder{name[0].upper() + name[1:]}"
                        native_buffer_name = f"nBuilder{name[0].upper() + name[1:]}Buffer"
                    else:
                        native_name = f"n{name[0].upper() + name[1:]}"
                        native_buffer_name = f"n{name[0].upper() + name[1:]}"

                    cnt_arg = {"name": cnt_name, "type": {"cpp_name": "size_t"}}
                    return {
                        "buf_arg": a,
                        "buf_idx": idx,
                        "cnt_arg": cnt_arg,
                        "cnt_idx": -1,
                        "offset_arg": offset_arg,
                        "offset_idx": offset_idx,
                        "has_existing_offset": has_existing_offset,
                        "array_offset_name": array_offset_name,
                        "size_param": None,
                        "stride": stride,
                        "cpp_type": arr_in.get("cpp_type", ""),
                        "cpp_elem_type": arr_in.get("cpp_type", ""),
                        "java_type": arr_in.get("java", "float[]"),
                        "scalar": arr_in.get("scalar", "float"),
                        "jni_type": arr_in.get("jni", "jfloatArray"),
                        "native_name": native_name,
                        "native_buffer_name": native_buffer_name,
                        "buf_name": buf_name,
                        "cnt_name": cnt_name,
                        "pre_args": pre_args,
                        "is_slice": True,
                    }

        # Step 1B: Legacy candidate buffer argument paired with a size parameter
        for idx, a in enumerate(args):
            size_param = get_size_param_attr(a)
            if not size_param:
                continue

            # Packed buffer method must have exactly one argument referencing size_param
            if len([arg for arg in args if get_size_param_attr(arg) == size_param]) != 1:
                continue

            t = a.get("type", {})
            nullability = t.get("nullability", "unspecified") if isinstance(t, dict) else "unspecified"
            if nullability == "nullable":
                continue

            cpp_name = t.get("cpp_name", "") if isinstance(t, dict) else str(t)
            clean_t = re.sub(r'\bconst\b', '', cpp_name).replace("*", "").strip()
            if clean_t in ("char", "const char", "void", "const void"):
                continue
            is_ptr = (isinstance(t, dict) and t.get("is_pointer", False)) or "*" in cpp_name
            if not is_ptr:
                continue

            # Step 2: Determine element stride (from array input info or homogeneous float struct)
            arr_in = self.get_array_input_info(t)
            if not arr_in:
                struct_cls = self.is_aggregate_struct(clean_t)
                float_stride = self.get_struct_float_stride(struct_cls) if struct_cls else None
                if float_stride is not None and float_stride > 1:
                    stride = float_stride
                    cpp_type = clean_t
                    arr_type = "float[]"
                    scalar = "float"
                    jni_type = "jfloatArray"
                else:
                    continue
            else:
                if arr_in.get("scalar") not in ("float", "double", "int", "long", "short", "byte"):
                    continue
                stride = arr_in.get("size", 1)
                if stride <= 1:
                    continue
                cpp_type = arr_in.get("cpp_type", clean_t)
                arr_type = arr_in.get("java", "float[]")
                scalar = arr_in.get("scalar", "float")
                jni_type = arr_in.get("jni", "jfloatArray")

            # Step 3: Locate matching count parameter
            cnt_arg = next((arg for arg in args if arg["name"] == size_param), None)
            if not cnt_arg:
                continue
            cnt_idx = args.index(cnt_arg)

            name = get_effective_method_name(method)
            buf_name = sanitize_identifier(a["name"])
            cnt_name = sanitize_identifier(cnt_arg["name"])

            # Step 4: Check for existing offset parameters
            offset_arg = next((arg for arg in args if arg["name"] in ("offset", "byteOffset")), None)
            offset_idx = args.index(offset_arg) if offset_arg else -1
            has_existing_offset = (offset_arg is not None)
            array_offset_name = "arrayOffset" if has_existing_offset else "offset"

            matched_indices = {idx, cnt_idx}
            if offset_idx != -1:
                matched_indices.add(offset_idx)
            if max(matched_indices) < len(args) - 1:
                continue

            pre_args = [arg for i, arg in enumerate(args) if i not in matched_indices]

            # Step 5: Format native method names
            if getattr(self.context, "archetype", "") == "builder":
                native_name = f"nBuilder{name[0].upper() + name[1:]}"
                native_buffer_name = f"nBuilder{name[0].upper() + name[1:]}Buffer"
            else:
                native_name = f"n{name[0].upper() + name[1:]}"
                native_buffer_name = f"n{name[0].upper() + name[1:]}"

            return {
                "buf_arg": a,
                "buf_idx": idx,
                "cnt_arg": cnt_arg,
                "cnt_idx": cnt_idx,
                "offset_arg": offset_arg,
                "offset_idx": offset_idx,
                "has_existing_offset": has_existing_offset,
                "array_offset_name": array_offset_name,
                "size_param": size_param,
                "stride": stride,
                "cpp_type": cpp_type,
                "cpp_elem_type": cpp_type,
                "java_type": arr_type,
                "scalar": scalar,
                "jni_type": jni_type,
                "native_name": native_name,
                "native_buffer_name": native_buffer_name,
                "buf_name": buf_name,
                "cnt_name": cnt_name,
                "pre_args": pre_args,
            }
        return None

    def get_builder_packed_buffer_info(self, method: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Extract packed buffer metadata for a builder setter method.

        Args:
            method: Builder method AST dictionary.

        Returns:
            Packed buffer metadata dictionary if matched, else None.
        """
        return self.get_packed_buffer_info(method)

    def is_bone_buffer_method(self, method: Dict[str, Any]) -> bool:
        """Check if method accepts a bone matrix or transform buffer.

        Args:
            method: Method AST dictionary.

        Returns:
            True if method accepts bone buffers.
        """
        return self.is_packed_buffer_method(method)

    def get_bone_buffer_info(self, method: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Extract bone buffer metadata for a method.

        Args:
            method: Method AST dictionary.

        Returns:
            Packed buffer metadata dictionary if matched, else None.
        """
        return self.get_packed_buffer_info(method)

    def get_clean_type_name(self, type_dict: Dict[str, Any]) -> str:
        """Produce a clean capitalized type name suitable for method suffix unmangling.

        Args:
            type_dict: Type AST dictionary.

        Returns:
            Clean capitalized type string (e.g. `Float`, `Int`, `Boolean`).
        """
        t_info, _ = self.resolve_type_info(type_dict, silent=True)
        if t_info.get("is_enum"):
            return t_info.get("enum_name") or t_info.get("java", "").split(".")[-1] or "Enum"
        j_type = t_info.get("java", "Object")
        j_type = re.sub(r'[^A-Za-z0-9]', '', j_type)
        return j_type[0].upper() + j_type[1:]

    def resolve_type_info(
        self,
        type_input: Union[str, Dict[str, Any]],
        silent: bool = False
    ) -> Tuple[Dict[str, Any], Tuple[Any, ...]]:
        """Resolve any C++ type to its Java type, JNI type, annotations, and marshalling rules.

        Comprehensive Type Resolution Strategy:
        ---------------------------------------
        - **Step 1: Raw Type String Extraction**: Extract either `qualified_name` or `cpp_name`.
        - **Step 2: Typedef & Using Alias Unwrapping**: Resolve typedefs (`using Type = ...`).
          Detects semantic aliases such as `LinearColor` / `LinearColorA`.
        - **Step 3: Exact Table Matching**: Evaluates :data:`~javagen.config.TYPE_MAP` and
          :data:`~javagen.config.MATH_TYPES`.
        - **Step 4: Regular Expression Pattern Evaluation**: Evaluates :data:`~javagen.config.TYPE_PATTERNS`.
        - **Step 5: Modifier Stripping**: Strips `const`, `&`, and `&&` qualifiers and re-checks.
        - **Step 6: Fixed-Size Arrays**: Detects `float[4]`, `mat4[4]` fixed arrays and generates
          `@Size(min = N)` annotations.
        - **Step 7: Invocable Callbacks**: Maps `utils::Invocable<void(args)>` to custom functional
          Java callback interfaces.
        - **Step 8: View Slices**: Maps `utils::Slice<T>` to Java arrays and JNI pointer arrays.
        - **Step 9: Standard Containers**: Maps `std::vector<T>` and `FixedCapacityVector<T>`.
        - **Step 10: Scoped & Unscoped Enums**: Resolves enum mappings, checking for bitmask flags.
        - **Step 11: Value Classes & Bitfields**: Maps inline buffers (`Box`, `Frustum`) and `Sampler`.
        - **Step 12: Aggregate Structs**: Identifies POD structs for AAPCS64 register flattening.
        - **Step 13: Filament Engine Handles**: Identifies opaque C++ engine pointer objects (`jlong`).
        - **Step 14: Fallback Opaque Pointer**: Emits diagnostic warning (if enabled) and returns `jlong`.

        Args:
            type_input: C++ type string or type dictionary from IR.
            silent: When True, suppresses diagnostic warnings for unknown types.

        Returns:
            Tuple of (type_info_dict, regex_capture_groups_tuple).
        """
        # Step 1: Extract raw type string from AST node or string input
        cpp_type = type_input
        if isinstance(type_input, dict):
            cpp_type = type_input.get("qualified_name") or type_input["cpp_name"]

        def strip_modifiers(t: str) -> str:
            """Strip const and reference modifiers from type."""
            t = re.sub(r'\bconst\b', '', t).strip()
            if t.endswith("&"):
                t = t[:-1].strip()
            if t.endswith("&&"):
                t = t[:-2].strip()
            return t.strip()

        def strip_pointer(t: str) -> str:
            """Strip trailing pointer asterisk."""
            t = t.strip()
            if t.endswith("*"):
                t = t[:-1].strip()
            return t.strip()

        # Step 2: Resolve type aliases (typedefs, using declarations)
        is_linear_color = (
            cpp_type in ("LinearColor", "LinearColorA", "const LinearColor &", "const LinearColorA &")
            or (
                isinstance(type_input, dict)
                and type_input.get("cpp_name") in (
                    "LinearColor", "LinearColorA", "const LinearColor &", "const LinearColorA &"
                )
            )
        )
        resolved_type = cpp_type
        if cpp_type in self.aliases:
            resolved_type = self.aliases[cpp_type]
        elif isinstance(type_input, dict) and type_input.get("cpp_name") in self.aliases:
            resolved_type = self.aliases[type_input["cpp_name"]]
        elif cpp_type.split("::")[-1] in self.aliases:
            resolved_type = self.aliases[cpp_type.split("::")[-1]]
        else:
            stripped = strip_modifiers(cpp_type)
            if stripped in self.aliases:
                resolved_type = self.aliases[stripped]
            elif stripped.split("::")[-1] in self.aliases:
                resolved_type = self.aliases[stripped.split("::")[-1]]

        if resolved_type in ("LinearColor", "LinearColorA"):
            is_linear_color = True

        def check_exact(t: str) -> Optional[Tuple[Dict[str, Any], Tuple[Any, ...]]]:
            clean_t = self.strip_namespaces(t)
            if t in self.enum_map or clean_t in self.enum_map:
                return None
            if t in TYPE_MAP:
                return TYPE_MAP[t], ()
            if t in MATH_TYPES:
                return MATH_TYPES[t], ()
            if t.startswith("filament::math::"):
                math_t = t.replace("filament::math::", "math::")
                if math_t in MATH_TYPES:
                    return MATH_TYPES[math_t], ()
            return None

        def with_linear(
            res_tuple: Optional[Tuple[Dict[str, Any], Tuple[Any, ...]]]
        ) -> Optional[Tuple[Dict[str, Any], Tuple[Any, ...]]]:
            if not res_tuple or not is_linear_color:
                return res_tuple
            info, extra = res_tuple
            info_copy = dict(info)
            info_copy["is_linear_color"] = True
            return info_copy, extra

        # Step 3: Exact mapping table checks (TYPE_MAP, MATH_TYPES)
        res = check_exact(resolved_type)
        if res:
            return with_linear(res)  # type: ignore

        # Step 4: Check regex pattern matchers (TYPE_PATTERNS)
        for pattern, info in TYPE_PATTERNS:
            m = re.match(pattern, resolved_type)
            if m:
                return with_linear((info, m.groups()))  # type: ignore

        # Step 5: Check modifiers stripping (const, &, &&)
        if '*' not in resolved_type:
            stripped = strip_modifiers(resolved_type)
            if stripped != resolved_type:
                res = check_exact(stripped)
                if res:
                    return with_linear(res)  # type: ignore

                for pattern, info in TYPE_PATTERNS:
                    m = re.match(pattern, stripped)
                    if m:
                        return with_linear((info, m.groups()))  # type: ignore
        else:
            stripped_ptr = strip_pointer(strip_modifiers(resolved_type))
            if stripped_ptr in MATH_TYPES:
                math_info = dict(MATH_TYPES[stripped_ptr])
                math_info["is_pointer"] = True
                return with_linear((math_info, ()))  # type: ignore

        # Step 6: Check for fixed-size arrays (e.g. float[3], math::mat4[4])
        clean_check = resolved_type.strip()
        if clean_check.startswith("const "):
            clean_check = clean_check[6:].strip()

        fixed_arr_match = re.match(r"^([a-zA-Z0-9_:]+)\s*\[\s*(\d+)\s*\]$", clean_check)
        if fixed_arr_match:
            elem_type_str = fixed_arr_match.group(1)
            size = int(fixed_arr_match.group(2))
            elem_info, _ = self.resolve_type_info(elem_type_str, silent=True)
            if "assert" in elem_info:
                math_info = dict(elem_info)
                math_info["is_array_of_math"] = True
                math_info["array_length"] = size
                math_info["total_size"] = size * elem_info["size"]
                return with_linear((math_info, ()))  # type: ignore
            scalar = elem_info.get("scalar") or elem_info.get("java")
            if scalar:
                jni_scalar = elem_info.get("jni", f"j{scalar}")
                return {
                    "java": f"{elem_info['java']}[]",
                    "jni": f"{jni_scalar}Array",
                    "jni_type": f"{jni_scalar}Array",
                    "scalar": scalar,
                    "size": size,
                    "annotation": f"@Size(min = {size})",
                    "cpp_type": f"{elem_type_str}[{size}]",
                    "elem_cpp_type": elem_type_str,
                    "is_fixed_array": True,
                }, ()

        # Step 7: Check for Invocable callback interfaces (utils::Invocable<...>)
        inv_match = re.match(
            r"^(?:const\s+)?(?:filament::)?(?:utils::)?Invocable<\s*([^(]+)\((.*)\)\s*>\s*(?:&&|&)?$",
            clean_check
        )
        if inv_match:
            interface_name = (
                f"{self.current_method[0].upper() + self.current_method[1:]}Callback"
                if self.current_method
                else "Callback"
            )
            return {
                "java": interface_name,
                "jni": "jobject",
                "jni_type": "jobject",
                "is_invocable": True,
            }, ()

        # Step 8: Check for Slices (utils::Slice<const T>)
        slice_match = re.match(
            r"^(?:const\s+)?(?:filament::)?(?:utils::)?Slice<\s*(?:const\s+)?(.*?)(?:\s+const)?\s*>\s*(?:&)?$",
            clean_check
        )
        if slice_match:
            inner_type_str = slice_match.group(1).strip()
            inner_info, _ = self.resolve_type_info(inner_type_str, silent=True)
            if inner_info.get("is_struct"):
                return {
                    "java": f"{inner_info['java']}[]",
                    "jni": "jlongArray",
                    "jni_type": "jlongArray",
                    "is_slice": True,
                    "is_struct_slice": True,
                    "inner_info": inner_info,
                    "cpp_type": f"utils::Slice<{inner_info.get('cpp_type', inner_type_str)}>",
                }, ()
            else:
                scalar = inner_info.get("scalar") or inner_info.get("java", "long")
                jni_scalar = inner_info.get("jni", f"j{scalar}")
                is_entity = inner_info.get("is_entity") or inner_info.get("annotation") == "@Entity"
                elem_ann = "@Entity " if is_entity else ""
                return {
                    "java": f"{elem_ann}{inner_info.get('java', scalar)}[]",
                    "jni": f"{jni_scalar}Array",
                    "jni_type": f"{jni_scalar}Array",
                    "is_slice": True,
                    "is_container": True,
                    "is_entity_container": is_entity,
                    "container_kind": "Slice",
                    "inner_info": inner_info,
                    "scalar": scalar,
                    "java_elem_type": f"{elem_ann}{inner_info.get('java', scalar)}",
                    "elem_cpp_type": inner_info.get("cpp_type", inner_type_str),
                    "cpp_type": f"utils::Slice<{inner_info.get('cpp_type', inner_type_str)}>",
                    "annotation": "@NonNull",
                }, ()

        # Step 9: Check for Containers (std::vector, FixedCapacityVector, std::array)
        container_match = re.match(
            r"^(?:const\s+)?(?:filament::)?(?:utils::)?(?:std::|std::__1::)?(FixedCapacityVector|vector|array)<\s*([^,>]+?)(?:\s*,\s*(\d+))?\s*>\s*(?:&|const\s*&)?$",
            clean_check
        )
        if container_match:
            container_kind = container_match.group(1)
            inner_type_str = container_match.group(2).strip()
            inner_info, _ = self.resolve_type_info(inner_type_str, silent=True)
            if inner_info.get("is_struct") or self.is_aggregate_struct(inner_type_str):
                struct_cls = inner_info.get("struct_cls") or self.is_aggregate_struct(inner_type_str)
                java_elem = inner_info.get("java") or (struct_cls["name"] if struct_cls else "Object")
                return {
                    "java": f"{java_elem}[]",
                    "jni": "jobjectArray",
                    "jni_type": "jobjectArray",
                    "is_container": True,
                    "container_kind": container_kind,
                    "is_struct_container": True,
                    "inner_info": inner_info,
                    "struct_cls": struct_cls,
                    "java_elem_type": java_elem,
                    "elem_cpp_type": inner_type_str,
                    "cpp_type": clean_check,
                    "annotation": "@NonNull",
                }, ()
            elif "assert" in inner_info:
                math_info = dict(inner_info)
                math_info["is_container"] = True
                math_info["container_kind"] = container_kind
                return math_info, ()
            else:
                scalar = inner_info.get("scalar") or inner_info.get("java", "long")
                jni_scalar = inner_info.get("jni", f"j{scalar}")
                return {
                    "java": f"{inner_info.get('java', scalar)}[]",
                    "jni": f"{jni_scalar}Array",
                    "jni_type": f"{jni_scalar}Array",
                    "is_container": True,
                    "container_kind": container_kind,
                    "inner_info": inner_info,
                    "scalar": scalar,
                    "java_elem_type": inner_info.get('java', scalar),
                    "elem_cpp_type": inner_type_str,
                    "cpp_type": clean_check,
                    "annotation": "@NonNull",
                }, ()


        # Step 10: Check for Enums and Bitmask Flags
        clean_no_ns = self.strip_namespaces(clean_check)
        raw_cpp_name = type_input.get("cpp_name") if isinstance(type_input, dict) else ""
        raw_qual_name = type_input.get("qualified_name") if isinstance(type_input, dict) else ""
        enum_ir = (
            self.enum_map.get(clean_check)
            or self.enum_map.get(clean_no_ns)
            or self.enum_map.get(raw_cpp_name)
            or self.enum_map.get(raw_qual_name)
        )
        if enum_ir:
            enum_name = enum_ir["name"]
            cpp_enum_type = self.strip_namespaces(enum_ir["qualified_name"])
            is_flags = enum_ir.get("is_flags", False) or any(
                a in (
                    "filament:apigen:flags", "filament:flags", "apigen:flags", "flags",
                    "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask"
                )
                for a in enum_ir.get("attributes", [])
            )
            if is_flags:
                return {
                    "java": "int",
                    "jni": "jint",
                    "jni_type": "jint",
                    "is_enum": False,
                    "is_flags": True,
                    "enum_name": enum_name,
                    "cpp_enum_type": cpp_enum_type,
                    "to_cpp": f"({cpp_enum_type}){{value}}",
                    "from_cpp": f"(jint){{value}}",
                    "annotation": "@IntRange(from = 0)",
                }, ()
            return {
                "java": enum_name,
                "jni": "jint",
                "jni_type": "jint",
                "is_enum": True,
                "enum_name": enum_name,
                "cpp_enum_type": cpp_enum_type,
                "to_cpp": f"({cpp_enum_type}){{value}}",
                "from_cpp": f"(jint){{value}}",
            }, ()
        elif isinstance(type_input, dict) and type_input.get("category") == "enum":
            enum_name = strip_modifiers(type_input.get("cpp_name", ""))
            cpp_enum_type = self.strip_namespaces(
                strip_modifiers(type_input.get("qualified_name") or type_input.get("cpp_name", ""))
            )
            is_flags = type_input.get("is_flags", False) or any(
                a in (
                    "filament:apigen:flags", "filament:flags", "apigen:flags", "flags",
                    "filament:apigen:bitmask", "filament:bitmask", "apigen:bitmask", "bitmask"
                )
                for a in type_input.get("attributes", [])
            )
            if is_flags:
                return {
                    "java": "int",
                    "jni": "jint",
                    "jni_type": "jint",
                    "is_enum": False,
                    "is_flags": True,
                    "enum_name": enum_name,
                    "cpp_enum_type": cpp_enum_type,
                    "to_cpp": f"({cpp_enum_type}){{value}}",
                    "from_cpp": f"(jint){{value}}",
                    "annotation": "@IntRange(from = 0)",
                }, ()
            return {
                "java": enum_name,
                "jni": "jint",
                "jni_type": "jint",
                "is_enum": True,
                "enum_name": enum_name,
                "cpp_enum_type": cpp_enum_type,
                "to_cpp": f"({cpp_enum_type}){{value}}",
                "from_cpp": f"(jint){{value}}",
            }, ()

        # Step 11: Check for Value Types (Box, Frustum, PixelBufferDescriptor, Sampler)
        clean_val = clean_check.replace("const ", "").replace("&", "").replace("*", "").strip()
        if clean_val in VALUE_TYPES:
            return VALUE_TYPES[clean_val], ()
        clean_no_ns = self.strip_namespaces(clean_val)
        if clean_no_ns in VALUE_TYPES:
            return VALUE_TYPES[clean_no_ns], ()

        # Check for dynamic Bitfield archetype classes (e.g. TextureSampler)
        known_cls = (
            KNOWN_CLASSES.get(clean_val)
            or KNOWN_CLASSES.get(clean_no_ns)
            or KNOWN_CLASSES.get(clean_check)
        )
        if known_cls and (
            known_cls.get("archetype") == "bitfield"
            or known_cls.get("category") == "bitfield"
            or any(
                a in ("filament:apigen:bitfield", "filament:bitfield", "apigen:bitfield", "bitfield")
                for a in known_cls.get("attributes", [])
            )
        ):
            primitive = known_cls.get("bitfield_primitive", "int")
            jni_prim = f"j{primitive}"
            return {
                "java": known_cls["name"],
                "jni": jni_prim,
                "jni_type": jni_prim,
                "archetype": "bitfield",
                "storage_type": primitive,
                "field_name": "mSampler" if "Sampler" in known_cls["name"] else f"m{known_cls['name']}",
                "cpp_type": known_cls.get("qualified_name") or known_cls["name"],
                "header": known_cls.get("location", {}).get("file", ""),
                "to_cpp": f"filament::JniUtils::from_{primitive}({{value}})",
                "from_cpp": f"filament::JniUtils::to_{primitive}({{value}})",
            }, ()

        # Step 12: Check for Aggregate Structs (Box, Viewport, etc.)
        struct_cls = self.is_aggregate_struct(type_input if isinstance(type_input, dict) else clean_check)
        if struct_cls:
            is_pojo = bool(struct_cls.get("is_pojo_struct") or struct_cls.get("archetype") == "pojo_struct" or self.is_pojo_struct(struct_cls))
            return {
                "java": struct_cls["name"],
                "jni": "void",
                "jni_type": "void",
                "is_struct": True,
                "is_pojo_struct": is_pojo,
                "struct_cls": struct_cls,
                "cpp_type": get_scoped_cpp_name(struct_cls) or struct_cls.get("qualified_name") or struct_cls["name"]
            }, ()

        # Step 12b: Check for POJO Structs (DynamicResolutionOptions, AmbientOcclusionOptions, etc.)
        pojo_cls = self.is_pojo_struct(type_input if isinstance(type_input, dict) else clean_check)
        if pojo_cls:
            return {
                "java": pojo_cls["name"],
                "jni": "void",
                "jni_type": "void",
                "is_struct": True,
                "is_pojo_struct": True,
                "struct_cls": pojo_cls,
                "cpp_type": get_scoped_cpp_name(pojo_cls) or pojo_cls.get("qualified_name") or pojo_cls["name"]
            }, ()

        # Step 13: Check for Filament engine object handles (Camera, Scene, View, etc.)
        clean_type_name = (
            self.strip_namespaces(clean_check)
            .replace("const ", "")
            .replace("*", "")
            .replace("&", "")
            .strip()
        )
        known_cls_entry = KNOWN_CLASSES.get(clean_type_name) or KNOWN_CLASSES.get(clean_no_ns)
        is_struct_type = known_cls_entry and (
            known_cls_entry.get("is_aggregate")
            or (
                len(known_cls_entry.get("fields", [])) > 0
                and (
                    known_cls_entry.get("category") == "struct"
                    or known_cls_entry.get("type", {}).get("category") == "struct"
                )
            )
        )
        is_ptr_or_ref = (
            "*" in clean_check
            or "&" in clean_check
            or (
                isinstance(type_input, dict)
                and (type_input.get("is_pointer") or type_input.get("is_reference"))
            )
        )
        if (
            not is_struct_type
            and is_ptr_or_ref
            and (
                clean_check.startswith("filament::")
                or clean_check.startswith("utils::")
                or clean_type_name in KNOWN_CLASSES
                or clean_type_name in (
                    "ToneMapper", "Engine", "Scene", "Camera", "View", "IndirectLight",
                    "IndexBuffer", "VertexBuffer", "BufferObject", "InstanceBuffer",
                    "SkinningBuffer", "MorphTargetBuffer", "ColorGrading"
                )
            )
        ):
            if (
                not clean_check.startswith("filament::math::")
                and not clean_check.startswith("math::")
                and "<" not in clean_check
            ):
                clean = (
                    clean_type_name
                    .replace("filament::", "")
                    .replace("utils::", "")
                    .replace("*", "")
                    .replace("&", "")
                    .strip()
                )
                return {
                    "java": clean,
                    "jni": "jlong",
                    "jni_type": "jlong",
                    "is_filament_type": True,
                    "cpp_class": clean
                }, ()

        # Step 14: Fallback for generic opaque pointer objects (jlong handle)
        if self.diagnostic_mode and not silent:
            method_context = f" in method '{self.current_method}'" if self.current_method else ""
            print(
                f"[WARNING] Unknown type '{cpp_type}' in class '{self.name}'{method_context}. Defaulting to 'jlong'.",
                file=sys.stderr
            )
        clean_java = self.strip_namespaces(resolved_type).replace("*", "").replace("&", "").strip()
        if not clean_java or "::" in clean_java or resolved_type.endswith("*") or resolved_type.endswith("&"):
            clean_java = "long"
        return {"java": clean_java, "jni": "jlong", "jni_type": "jlong"}, ()

    def get_invocable_info(
        self,
        t: Union[str, Dict[str, Any]],
        method_name: str = ""
    ) -> Optional[Dict[str, Any]]:
        """Extract Invocable lambda/std::function callback interface signatures and parameters.

        Parses the C++ signature of `utils::Invocable<ReturnType(ArgTypes...)>` into:
        - Target Java interface name (e.g. `MyMethodCallback`).
        - JNI method invocation call (e.g. `CallVoidMethod`, `CallBooleanMethod`).
        - JNI method signature string (e.g. `(I)V`).
        - Parameter list with JNI cast expressions.

        Args:
            t: Type input string or dictionary.
            method_name: Name of the enclosing method (for interface naming).

        Returns:
            Dictionary containing callback interface name, return signatures, and arguments.
        """
        # Step 1: Extract type string and test regex match
        t_str = t.get("qualified_name") or t["cpp_name"] if isinstance(t, dict) else str(t)
        clean_type = t_str.strip()
        m = re.match(
            r"^(?:const\s+)?(?:filament::)?(?:utils::)?Invocable<\s*([^(]+)\((.*)\)\s*>\s*(?:&&|&)?$",
            clean_type
        )
        if not m:
            return None

        ret_str = m.group(1).strip()
        params_str = m.group(2).strip()

        interface_name = (
            f"{method_name[0].upper() + method_name[1:]}Callback" if method_name else "Callback"
        )

        # Step 2: Map return type and JNI method invocation call
        ret_info, _ = self.resolve_type_info(ret_str, silent=True)
        java_ret = ret_info.get("java", "void")
        jni_ret = ret_info.get("jni", "void")
        cpp_ret = self.strip_namespaces(ret_str)

        if java_ret == "void":
            jni_call = "CallVoidMethod"
            ret_sig = "V"
            default_ret = ""
        elif java_ret == "boolean":
            jni_call = "CallBooleanMethod"
            ret_sig = "Z"
            default_ret = "false"
        elif java_ret == "byte":
            jni_call = "CallByteMethod"
            ret_sig = "B"
            default_ret = "0"
        elif java_ret == "char":
            jni_call = "CallCharMethod"
            ret_sig = "C"
            default_ret = "0"
        elif java_ret == "short":
            jni_call = "CallShortMethod"
            ret_sig = "S"
            default_ret = "0"
        elif java_ret == "int":
            jni_call = "CallIntMethod"
            ret_sig = "I"
            default_ret = "0"
        elif java_ret == "long":
            jni_call = "CallLongMethod"
            ret_sig = "J"
            default_ret = "0"
        elif java_ret == "float":
            jni_call = "CallFloatMethod"
            ret_sig = "F"
            default_ret = "0.0f"
        elif java_ret == "double":
            jni_call = "CallDoubleMethod"
            ret_sig = "D"
            default_ret = "0.0"
        else:
            jni_call = "CallObjectMethod"
            if "/" in java_ret:
                ret_sig = f"L{java_ret};"
            elif "." in java_ret:
                ret_sig = f"L{java_ret.replace('.', '/')};"
            elif java_ret == "String":
                ret_sig = "Ljava/lang/String;"
            elif java_ret == "Object":
                ret_sig = "Ljava/lang/Object;"
            else:
                ret_sig = f"Lcom/google/android/filament/{java_ret};"
            default_ret = "nullptr"

        # Step 3: Parse parameter list and construct JNI method signature
        parsed_params = []
        if params_str:
            raw_params = [p.strip() for p in params_str.split(",") if p.strip()]
            for idx, p_str in enumerate(raw_params):
                parts = p_str.split()
                if (
                    len(parts) > 1
                    and not parts[0].endswith("::")
                    and parts[-1].isidentifier()
                    and parts[-1] not in (
                        "int", "float", "double", "char", "short", "long", "bool", "void", "size_t"
                    )
                ):
                    p_type_only = " ".join(parts[:-1])
                    p_name = parts[-1]
                else:
                    p_type_only = p_str
                    p_name = f"arg{idx}"

                p_info, _ = self.resolve_type_info(p_type_only, silent=True)
                p_java = p_info.get("java", "int")
                p_ann = p_info.get("annotation", "")
                if p_ann == "@Entity" and p_name == f"arg{idx}":
                    p_name = "entity" if len(raw_params) == 1 else f"entity{idx}"

                if p_java == "boolean":
                    sig = "Z"
                    to_jni = f"(jboolean){p_name}"
                elif p_java == "byte":
                    sig = "B"
                    to_jni = f"(jbyte){p_name}"
                elif p_java == "char":
                    sig = "C"
                    to_jni = f"(jchar){p_name}"
                elif p_java == "short":
                    sig = "S"
                    to_jni = f"(jshort){p_name}"
                elif p_java == "int":
                    sig = "I"
                    if p_info.get("annotation") == "@Entity":
                        to_jni = f"(jint){p_name}.getId()"
                    else:
                        to_jni = f"(jint){p_name}"
                elif p_java == "long":
                    sig = "J"
                    to_jni = f"(jlong){p_name}"
                elif p_java == "float":
                    sig = "F"
                    to_jni = f"(jfloat){p_name}"
                elif p_java == "double":
                    sig = "D"
                    to_jni = f"(jdouble){p_name}"
                else:
                    if "/" in p_java:
                        sig = f"L{p_java};"
                    elif "." in p_java:
                        sig = f"L{p_java.replace('.', '/')};"
                    elif p_java == "String":
                        sig = "Ljava/lang/String;"
                    elif p_java == "Object":
                        sig = "Ljava/lang/Object;"
                    else:
                        sig = f"Lcom/google/android/filament/{p_java};"
                    to_jni = f"{p_name}"

                parsed_params.append({
                    "name": p_name,
                    "cpp_type": p_type_only,
                    "java_type": p_java,
                    "annotation": p_ann,
                    "sig": sig,
                    "to_jni": to_jni,
                })

        jni_method_sig = f"({''.join(p['sig'] for p in parsed_params)}){ret_sig}"

        return {
            "interface_name": interface_name,
            "java_ret": java_ret,
            "jni_ret": jni_ret,
            "cpp_ret": cpp_ret,
            "jni_call": jni_call,
            "ret_sig": ret_sig,
            "default_ret": default_ret,
            "params": parsed_params,
            "jni_method_sig": jni_method_sig,
        }

    def is_aggregate_struct(
        self,
        type_name_or_info: Union[str, Dict[str, Any]]
    ) -> Optional[Dict[str, Any]]:
        """Determine if a type represents a pure data aggregate struct (e.g. Box, Viewport).

        Aggregate structs are Plain-Old-Data (POD) types with public member fields,
        no polymorphic base classes, and no opaque native pointers.

        Args:
            type_name_or_info: Type name string or type dictionary.

        Returns:
            The struct class dictionary from KNOWN_CLASSES if aggregate, else None.
        """
        # Step 1: Reject pointer types immediately
        if isinstance(type_name_or_info, dict):
            if type_name_or_info.get("is_pointer"):
                return None
            qname = type_name_or_info.get("qualified_name") or type_name_or_info.get("cpp_name", "")
        else:
            qname = str(type_name_or_info)
        if "*" in qname:
            return None

        # Step 2: Clean namespace and modifier qualifiers
        clean = (
            self.strip_namespaces(qname)
            .replace("&", "")
            .replace("*", "")
            .replace("const", "")
            .replace("struct ", "")
            .strip()
        )

        # Step 2b: Exclude POJO structs
        if self.is_pojo_struct(type_name_or_info):
            return None

        # Step 3: Look up in KNOWN_CLASSES and check aggregate criteria
        cls = KNOWN_CLASSES.get(clean) or KNOWN_CLASSES.get(qname)
        if cls and (
            cls.get("is_aggregate")
            or (
                len(cls.get("fields", [])) > 0
                and (
                    cls.get("category") == "struct"
                    or cls.get("type", {}).get("category") == "struct"
                )
            )
        ):
            return cls
        return None

    def is_pojo_struct(
        self,
        type_name_or_info: Optional[Union[str, Dict[str, Any]]] = None
    ) -> Optional[Dict[str, Any]]:
        """Determine if a type represents a public POJO struct (e.g. DynamicResolutionOptions).

        POJO structs are Plain-Old-Java-Object classes with public mutable fields,
        nested enums, exact default values, and symmetrical option caching.

        Args:
            type_name_or_info: Type name string, type dictionary, or None (checks self.context).

        Returns:
            The struct class dictionary from KNOWN_CLASSES or nested structs if POJO, else None.
        """
        if type_name_or_info is None:
            if hasattr(self.context, "ir"):
                cls = self.context.ir
                if cls.get("is_pojo_struct") or cls.get("archetype") == "pojo_struct":
                    return cls
                loc_file = cls.get("location", {}).get("file", "")
                if loc_file.endswith("Options.h"):
                    return cls
            type_name_or_info = self.context.name

        if isinstance(type_name_or_info, dict):
            if (type_name_or_info.get("is_pojo_struct") or type_name_or_info.get("archetype") == "pojo_struct") and "name" in type_name_or_info and "fields" in type_name_or_info:
                return type_name_or_info
            loc_file = type_name_or_info.get("location", {}).get("file", "")
            if loc_file.endswith("Options.h") and "name" in type_name_or_info:
                return type_name_or_info
            qname = type_name_or_info.get("qualified_name") or type_name_or_info.get("cpp_name", "")
        else:
            qname = str(type_name_or_info)

        clean = (
            self.strip_namespaces(qname)
            .replace("&", "")
            .replace("*", "")
            .replace("const", "")
            .replace("struct ", "")
            .strip()
        )

        cls = KNOWN_CLASSES.get(clean) or KNOWN_CLASSES.get(qname)
        if not cls and hasattr(self.context, "nested_structs_map"):
            for nlist in self.context.nested_structs_map.values():
                for s in nlist:
                    if s.get("name") == clean:
                        cls = s
                        break
                if cls:
                    break
        if not cls and hasattr(self.context, "nested_struct_contexts"):
            for nc in self.context.nested_struct_contexts:
                if nc.name == clean:
                    cls = nc.ir
                    break

        if cls:
            if cls.get("is_pojo_struct") or cls.get("archetype") == "pojo_struct":
                return cls
            loc_file = cls.get("location", {}).get("file", "")
            if loc_file.endswith("Options.h"):
                return cls
            if cls.get("parent_class") == "View" and cls.get("name") == "PickingQueryResult":
                return cls
            if cls.get("parent_class") == "Engine" and cls.get("name") == "Config":
                return cls

        return None

    def translate_default_value(
        self,
        raw_val: Any,
        type_info: Any = None,
        attributes: Optional[List[str]] = None,
        enclosing_struct_name: Optional[str] = None
    ) -> Optional[str]:
        """Translate a C++ default value expression to an idiomatic Java literal."""
        if isinstance(raw_val, dict):
            field_dict = raw_val
            raw_val = field_dict.get("default_value")
            if type_info is None or isinstance(type_info, str):
                enclosing_struct_name = enclosing_struct_name or (type_info if isinstance(type_info, str) else None)
                type_info = field_dict.get("type", {})
            attributes = attributes or field_dict.get("attributes", [])

        if raw_val is None:
            return None
        val = str(raw_val).strip()
        attributes = attributes or []

        # Case 1: Forced java_float
        if "apigen:java_type:float" in attributes:
            inner = val.strip(" {}[]()")
            if "," in inner:
                inner = inner.split(",")[0].strip()
            if not inner.endswith("f"):
                inner += "f"
            return inner

        # Case 2: Nullptr
        if val == "nullptr":
            return "null"

        # Case 3: Infinity
        if val == "INFINITY":
            return "Float.POSITIVE_INFINITY"
        if val in ("-INFINITY", "- INFINITY"):
            return "Float.NEGATIVE_INFINITY"

        # Case 4: Empty braces (default zero initialization)
        if val in ("{}", "{ }"):
            return None

        # Case 5: Vector array initializers { ... } or [ ... ]
        if (val.startswith("{") and val.endswith("}")) or (val.startswith("[") and val.endswith("]")):
            inner = val[1:-1].strip()
            parts = [p.strip() for p in inner.split(",") if p.strip()]
            fixed_parts = []
            for p in parts:
                p_clean = p.replace("- ", "-")
                if not p_clean.endswith("f") and ("." in p_clean or p_clean.isdigit() or (p_clean.startswith("-") and p_clean[1:].isdigit())):
                    p_clean += "f"
                fixed_parts.append(p_clean)
            return "{" + ", ".join(fixed_parts) + "}"

        # Case 6: Enum values with ::
        if "::" in val:
            clean_enum = val.replace("::", ".")
            parts = clean_enum.split(".")
            enum_cls = parts[-2]
            enum_const = parts[-1]
            if enclosing_struct_name:
                nested_enums = getattr(self.context, "enums", [])
                if any(e["name"] == enum_cls for e in nested_enums):
                    return f"{enclosing_struct_name}.{enum_cls}.{enum_const}"
            return f"{enum_cls}.{enum_const}"

        # Case 7: Scalar float without f suffix
        cpp_t = type_info.get("cpp_name", "") if isinstance(type_info, dict) else str(type_info)
        if "float" in cpp_t:
            val = val.replace("- ", "-")
            if not val.endswith("f") and not val.startswith("Float."):
                val += "f"
            return val

        return val

    def get_pojo_field_annotation(
        self,
        field: Dict[str, Any],
        enclosing_name: Optional[str] = None
    ) -> List[str]:
        """Compute AndroidX / deprecation annotations for a POJO struct field."""
        annotations = []
        doc = field.get("doc", {})
        if isinstance(doc, dict):
            meta = doc.get("meta", {})
            details = doc.get("details", "")
            brief = doc.get("brief", "")
            if "deprecated" in meta or "@deprecated" in details.lower() or "@deprecated" in brief.lower():
                annotations.append("@Deprecated")

        attrs = field.get("attributes", [])
        if "apigen:java_type:float" not in attrs:
            default_val = field.get("default_value")
            cpp_t = field.get("type", {}).get("cpp_name", "") if isinstance(field.get("type"), dict) else str(field.get("type"))
            if default_val == "nullptr":
                annotations.append("@Nullable")
            elif cpp_t == "math::float2":
                annotations.append("@NonNull @Size(min = 2)")
            elif cpp_t in ("math::float3", "LinearColor"):
                annotations.append("@NonNull @Size(min = 3)")
            elif cpp_t in ("math::float4", "LinearColorA"):
                annotations.append("@NonNull @Size(min = 4)")
            elif default_val and "::" in default_val:
                annotations.append("@NonNull")

        if enclosing_name == "PickingQueryResult":
            if field.get("name") == "renderable":
                annotations.append("@Entity")

        return annotations

    def get_pojo_leaves(
        self,
        struct_cls: Dict[str, Any],
        var_name: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """Extract unrolled leaf descriptors for a POJO struct for JNI marshalling.

        Args:
            struct_cls: Dictionary representing the POJO struct class IR.
            var_name: Optional variable name in Java (e.g. 'options').

        Returns:
            List of leaf descriptor dictionaries detailing names, Java expressions,
            JNI parameter names, JNI types, and C++ reconstruction assignments.
        """
        leaves = []
        fields = struct_cls.get("fields", [])
        for f in fields:
            fname = f["name"]
            attrs = f.get("attributes", [])
            f_type = f.get("type", {})
            cpp_type_name = f_type.get("cpp_name", "") if isinstance(f_type, dict) else str(f_type)
            clean_cpp_type = self.strip_namespaces(cpp_type_name).replace("*", "").replace("&", "").replace("const", "").strip()

            # Skip padding/reserved/rfu fields
            if is_reserved_or_padding_field(fname):
                continue

            # Case 1: Flattened sub-struct (e.g. Ssct, Gtao)
            if "apigen:flatten" in attrs:
                # Resolve child struct
                sub_cls = self.is_pojo_struct(clean_cpp_type)
                if not sub_cls:
                    sub_cls = KNOWN_CLASSES.get(clean_cpp_type)
                if sub_cls:
                    sub_fields = sub_cls.get("fields", [])
                    for sf in sub_fields:
                        sf_name = sf["name"]
                        if is_reserved_or_padding_field(sf_name):
                            continue
                        prefixed_name = fname + sf_name[0].upper() + sf_name[1:]
                        sf_type = sf.get("type", {})
                        sf_cpp_type = sf_type.get("cpp_name", "") if isinstance(sf_type, dict) else str(sf_type)
                        sf_clean_type = self.strip_namespaces(sf_cpp_type).replace("*", "").replace("&", "").replace("const", "").strip()

                        # Check if subfield is math vector (e.g. lightDirection: math::float3)
                        arr_in = self.get_array_input_info(sf_type)
                        if arr_in and not sf.get("attributes", []):
                            size = arr_in["size"]
                            comps = ["X", "Y", "Z", "W"][:size]
                            for idx, c in enumerate(comps):
                                comp_name = f"{prefixed_name}{c}"
                                java_expr = f"{var_name}.{prefixed_name}[{idx}]" if var_name else comp_name
                                leaves.append({
                                    "name": comp_name,
                                    "java_type": "float",
                                    "java_expr": java_expr,
                                    "jni_type": "jfloat",
                                    "jni_param_name": comp_name,
                                    "is_vector_comp": True,
                                    "vector_field": f"{fname}.{sf_name}",
                                    "comp_idx": idx,
                                    "comp_count": size,
                                    "cpp_type": sf_cpp_type,
                                    "cpp_assign": None,
                                })
                        elif sf_cpp_type == "bool":
                            java_expr = f"{var_name}.{prefixed_name}" if var_name else prefixed_name
                            leaves.append({
                                "name": prefixed_name,
                                "java_type": "boolean",
                                "java_expr": java_expr,
                                "jni_type": "jboolean",
                                "jni_param_name": prefixed_name,
                                "cpp_assign": f"options.{fname}.{sf_name} = (bool){prefixed_name};",
                            })
                        elif sf_clean_type in ("uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t", "int"):
                            java_expr = f"{var_name}.{prefixed_name}" if var_name else prefixed_name
                            leaves.append({
                                "name": prefixed_name,
                                "java_type": "int",
                                "java_expr": java_expr,
                                "jni_type": "jint",
                                "jni_param_name": prefixed_name,
                                "cpp_assign": f"options.{fname}.{sf_name} = ({sf_cpp_type}){prefixed_name};",
                            })
                        else:  # float
                            java_expr = f"{var_name}.{prefixed_name}" if var_name else prefixed_name
                            leaves.append({
                                "name": prefixed_name,
                                "java_type": "float",
                                "java_expr": java_expr,
                                "jni_type": "jfloat",
                                "jni_param_name": prefixed_name,
                                "cpp_assign": f"options.{fname}.{sf_name} = {prefixed_name};",
                            })
                continue

            # Case 2: Float broadcast (%codegen_java_float%)
            if "apigen:java_type:float" in attrs:
                java_expr = f"{var_name}.{fname}" if var_name else fname
                leaves.append({
                    "name": fname,
                    "java_type": "float",
                    "java_expr": java_expr,
                    "jni_type": "jfloat",
                    "jni_param_name": fname,
                    "cpp_assign": f"options.{fname} = filament::math::float2{{ {fname} }};",
                })
                continue

            # Case 3: Texture pointer
            if "Texture" in cpp_type_name and "*" in cpp_type_name:
                param_name = f"native{fname[0].upper()}{fname[1:]}"
                java_expr = f"{var_name}.{fname} != null ? {var_name}.{fname}.getNativeObject() : 0" if var_name else param_name
                leaves.append({
                    "name": param_name,
                    "java_type": "long",
                    "java_expr": java_expr,
                    "jni_type": "jlong",
                    "jni_param_name": param_name,
                    "cpp_assign": f"options.{fname} = (Texture*){param_name};",
                })
                continue

            # Case 4: Enum
            enum_info, _ = self.resolve_type_info(f_type, silent=True)
            if enum_info.get("is_enum") or any(e["name"] == clean_cpp_type for e in struct_cls.get("enums", [])):
                parent = struct_cls.get("parent_class") or "View"
                call_meth = ".toFilamentNative()"
                java_expr = f"{var_name}.{fname}{call_meth}" if var_name else fname
                scoped_enum = f"{parent}::{struct_cls['name']}::{clean_cpp_type}" if any(e["name"] == clean_cpp_type for e in struct_cls.get("enums", [])) else f"{parent}::{clean_cpp_type}"
                leaves.append({
                    "name": fname,
                    "java_type": "int",
                    "java_expr": java_expr,
                    "jni_type": "jint",
                    "jni_param_name": fname,
                    "cpp_assign": f"options.{fname} = ({scoped_enum}){fname};",
                })
                continue

            # Case 5: Vector array (e.g. LinearColor color = {1.0f, 1.0f, 1.0f})
            arr_in = self.get_array_input_info(f_type)
            if arr_in:
                size = arr_in["size"]
                comps = ["X", "Y", "Z", "W"][:size]
                if "LinearColorA" in cpp_type_name:
                    cpp_target_type = "filament::LinearColorA"
                elif "LinearColor" in cpp_type_name:
                    cpp_target_type = "filament::LinearColor"
                else:
                    cpp_target_type = arr_in["cpp_type"]
                for idx, c in enumerate(comps):
                    comp_name = f"{fname}{c}"
                    java_expr = f"{var_name}.{fname}[{idx}]" if var_name else comp_name
                    leaves.append({
                        "name": comp_name,
                        "java_type": "float",
                        "java_expr": java_expr,
                        "jni_type": "jfloat",
                        "jni_param_name": comp_name,
                        "is_vector_comp": True,
                        "vector_field": fname,
                        "comp_idx": idx,
                        "comp_count": size,
                        "cpp_type": cpp_target_type,
                        "cpp_assign": None,
                    })
                continue

            # Case 6: Boolean
            if cpp_type_name == "bool":
                java_expr = f"{var_name}.{fname}" if var_name else fname
                leaves.append({
                    "name": fname,
                    "java_type": "boolean",
                    "java_expr": java_expr,
                    "jni_type": "jboolean",
                    "jni_param_name": fname,
                    "cpp_assign": f"options.{fname} = (bool){fname};",
                })
                continue

            # Case 7: Integer types
            if clean_cpp_type in ("size_t", "uint64_t", "int64_t", "long", "unsigned long", "long long", "unsigned long long") or (struct_cls.get("parent_class") == "Engine" and struct_cls.get("name") == "Config" and clean_cpp_type in ("uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t", "int")):
                java_expr = f"{var_name}.{fname}" if var_name else fname
                leaves.append({
                    "name": fname,
                    "java_type": "long",
                    "java_expr": java_expr,
                    "jni_type": "jlong",
                    "jni_param_name": fname,
                    "cpp_assign": f"options.{fname} = ({cpp_type_name}){fname};",
                })
                continue

            if clean_cpp_type in ("uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t", "int"):
                java_expr = f"{var_name}.{fname}" if var_name else fname
                leaves.append({
                    "name": fname,
                    "java_type": "int",
                    "java_expr": java_expr,
                    "jni_type": "jint",
                    "jni_param_name": fname,
                    "cpp_assign": f"options.{fname} = ({cpp_type_name}){fname};",
                })
                continue

            # Case 8: Default float scalar
            java_expr = f"{var_name}.{fname}" if var_name else fname
            leaves.append({
                "name": fname,
                "java_type": "float",
                "java_expr": java_expr,
                "jni_type": "jfloat",
                "jni_param_name": fname,
                "cpp_assign": f"options.{fname} = {fname};",
            })

        return leaves

    def get_flattened_leaves(
        self,
        type_info: Union[str, Dict[str, Any]],
        prefix: str = ""
    ) -> List[Dict[str, Any]]:
        """Flatten a complex struct or vector type into primitive scalar leaves.

        Used for register-passed JNI method signatures (AAPCS64 s0-s7 parameter passing).
        Recursively unpacks nested aggregate structs and math vectors into individual scalar
        parameters so they can be passed in CPU registers across the JNI boundary.

        Examples:
            - `Box` -> `[mCenterX, mCenterY, mCenterZ, mHalfExtentX, mHalfExtentY, mHalfExtentZ]`
            - `Viewport` -> `[mBottom, mLeft, mWidth, mHeight]`

        Args:
            type_info: Type string or dictionary.
            prefix: Property prefix for nested member names.

        Returns:
            List of leaf descriptor dictionaries detailing names, types, and getter chains.
        """
        # Step 1: Math vector or array (e.g. float3, mat4f)
        arr_info = self.get_array_input_info(type_info)
        if arr_info:
            size = arr_info["size"]
            scalar = arr_info["scalar"]
            jni_scalar = arr_info.get("jni_scalar") or (
                "jfloat" if scalar == "float"
                else "jdouble" if scalar == "double"
                else "jint" if scalar in (
                    "int", "uint32_t", "uint8_t", "int8_t", "short",
                    "uint16_t", "int16_t", "byte", "char"
                )
                else "jlong"
            )
            is_fixed_arr = arr_info.get("is_fixed_array", False)
            if is_fixed_arr or size > 4:
                comps = [str(i) for i in range(size)]
            else:
                comps = ["X", "Y", "Z", "W"][:size]
            leaves = []
            for comp in comps:
                leaf_name = f"{prefix}{comp}"
                leaf_param = f"{prefix[0].lower() + prefix[1:] if prefix else ''}{comp}"
                leaves.append({
                    "name": leaf_name,
                    "param_name": leaf_param,
                    "getter_chain": [f"get{leaf_name}()"],
                    "java_type": scalar,
                    "jni_type": jni_scalar,
                    "comp": comp,
                    "is_math": not is_fixed_arr,
                    "is_fixed_array": is_fixed_arr,
                    "cpp_type": arr_info["cpp_type"],
                    "size": size,
                    "scalar": scalar
                })
            return leaves

        # Step 2: Struct slice (utils::Slice<const T>)
        slice_info, _ = self.resolve_type_info(type_info, silent=True)
        if slice_info.get("is_slice"):
            param_name = prefix[0].lower() + prefix[1:] if prefix else "slice"
            java_type = "long[]" if slice_info.get("is_struct_slice") else slice_info["java"]
            return [
                {
                    "name": prefix,
                    "param_name": param_name,
                    "getter_chain": [f"get{prefix}()"],
                    "java_type": java_type,
                    "jni_type": slice_info["jni_type"],
                    "is_slice": True,
                    "is_struct_slice": slice_info.get("is_struct_slice", False),
                    "slice_info": slice_info,
                    "cpp_type": slice_info["cpp_type"],
                },
                {
                    "name": f"{prefix}Count",
                    "param_name": f"{param_name}Count",
                    "getter_chain": [f"get{prefix}() != null ? get{prefix}().length : 0"],
                    "java_type": "int",
                    "jni_type": "jint",
                    "is_slice_count": True,
                    "cpp_type": "size_t",
                }
            ]

        # Step 3: Nested aggregate struct
        struct_cls = self.is_aggregate_struct(type_info)
        if struct_cls:
            leaves = []
            for field in struct_cls.get("fields", []):
                fname = field["name"]
                if is_reserved_or_padding_field(fname):
                    continue
                cap_name = fname[0].upper() + fname[1:]
                sub_prefix = f"{prefix}{cap_name}"
                sub_leaves = self.get_flattened_leaves(field["type"], prefix=sub_prefix)
                for sl in sub_leaves:
                    sl_copy = dict(sl)
                    if not prefix:
                        if self.is_aggregate_struct(field["type"]):
                            sub_field_name = sl['name'][len(cap_name):]
                            sl_copy["getter_chain"] = [f"get{cap_name}()", f"get{sub_field_name}()"]
                        else:
                            sl_copy["getter_chain"] = [f"get{sl['name']}()"]
                    leaves.append(sl_copy)
            return leaves

        # Step 4: Scalar primitive
        jtype = self.get_java_type(type_info)
        jnitype = self.get_jni_type(type_info)
        param_name = prefix[0].lower() + prefix[1:] if prefix else "val"
        return [{
            "name": prefix,
            "param_name": param_name,
            "getter_chain": [f"get{prefix}()"],
            "java_type": jtype,
            "jni_type": jnitype,
            "is_math": False,
            "type": type_info,
            "cpp_type": type_info.get("cpp_name", "") if isinstance(type_info, dict) else str(type_info)
        }]

    def get_struct_leaf_accessors(
        self,
        struct_cls: Dict[str, Any],
        prefix: str = "",
        cpp_prefix: str = ""
    ) -> List[Tuple[str, str, str, str, str, str, str]]:
        """Generate JNI leaf accessor metadata for populating Java struct fields from C++.

        Args:
            struct_cls: Struct class dictionary from IR.
            prefix: Java field prefix for nested structs.
            cpp_prefix: C++ field access prefix.

        Returns:
            List of tuples: (field_id_name, java_field_name, sig, setter, cast, cast_end, cpp_access).
        """
        accessors = []
        for field in struct_cls.get("fields", []):
            fname = field["name"]
            if is_reserved_or_padding_field(fname):
                continue
            fcap = fname[0].upper() + fname[1:]
            ftype = field["type"]
            arr_in = self.get_array_input_info(ftype)
            f_info, _ = self.resolve_type_info(ftype, silent=True)
            f_type_str = ftype.get("cpp_name", "") if isinstance(ftype, dict) else str(ftype)

            if arr_in:
                size = arr_in["size"]
                scalar = arr_in["scalar"]
                is_fixed_arr = arr_in.get("is_fixed_array", False)
                comps = (
                    [str(i) for i in range(size)]
                    if (is_fixed_arr or size > 4)
                    else ["X", "Y", "Z", "W"][:size]
                )
                for c in comps:
                    leaf_name = f"{fcap}{c}"
                    field_id_name = f"{prefix}{fname}_{c}" if prefix else f"{fname}_{c}"
                    java_field_name = f"m{prefix}{leaf_name}"
                    sig = "F" if scalar == "float" else "D" if scalar == "double" else "I"
                    setter = (
                        "SetFloatField" if scalar == "float"
                        else "SetDoubleField" if scalar == "double"
                        else "SetIntField"
                    )
                    cast = (
                        "(jfloat) " if scalar == "float"
                        else "(jdouble) " if scalar == "double"
                        else "static_cast<jint>("
                    )
                    cast_end = ")" if scalar not in ("float", "double") else ""
                    comp_access = f"[{c}]" if (is_fixed_arr or size > 4) else f".{c.lower()}"
                    cpp_access = f"{cpp_prefix}{fname}{comp_access}"
                    accessors.append(
                        (field_id_name, java_field_name, sig, setter, cast, cast_end, cpp_access)
                    )
            elif self.is_aggregate_struct(ftype):
                sub_s = self.is_aggregate_struct(ftype)
                sub_accs = self.get_struct_leaf_accessors(
                    sub_s,  # type: ignore
                    prefix=f"{prefix}{fcap}",
                    cpp_prefix=f"{cpp_prefix}{fname}."
                )
                accessors.extend(sub_accs)
            else:
                field_id_name = f"{prefix}{fname}" if prefix else fname
                java_field_name = f"m{prefix}{fcap}" if prefix else fname
                java_t = f_info.get("java", "")
                if f_info.get("is_enum") or java_t == "int":
                    sig = "I"
                    setter = "SetIntField"
                    cast = "static_cast<jint>("
                    cast_end = ")"
                elif java_t == "long":
                    sig = "J"
                    setter = "SetLongField"
                    cast = "static_cast<jlong>("
                    cast_end = ")"
                elif java_t == "float":
                    sig = "F"
                    setter = "SetFloatField"
                    cast = "(jfloat) "
                    cast_end = ""
                elif java_t == "double":
                    sig = "D"
                    setter = "SetDoubleField"
                    cast = "(jdouble) "
                    cast_end = ""
                elif java_t == "boolean":
                    sig = "Z"
                    setter = "SetBooleanField"
                    cast = "(jboolean) "
                    cast_end = ""
                elif java_t == "byte":
                    sig = "B"
                    setter = "SetByteField"
                    cast = "static_cast<jbyte>("
                    cast_end = ")"
                elif java_t == "short":
                    sig = "S"
                    setter = "SetShortField"
                    cast = "static_cast<jshort>("
                    cast_end = ")"
                else:
                    sig = "I"
                    setter = "SetIntField"
                    cast = "static_cast<jint>("
                    cast_end = ")"
                cpp_access = f"{cpp_prefix}{fname}"
                accessors.append(
                    (field_id_name, java_field_name, sig, setter, cast, cast_end, cpp_access)
                )
        return accessors

    def generate_cpp_struct_construction(
        self,
        struct_cls: Dict[str, Any],
        var_name: str,
        leaf_prefix: str = "",
        type_override: Optional[str] = None
    ) -> List[str]:
        """Generate C++ statements reconstructing an aggregate struct from primitive leaf arguments.

        Generates local assignment lines that pack primitive arguments back into the native C++
        struct before calling engine APIs.

        Args:
            struct_cls: Struct class dictionary from IR.
            var_name: Target local C++ variable name.
            leaf_prefix: Prefix for leaf parameter names.
            type_override: Optional explicit C++ type override.

        Returns:
            List of C++ assignment and instantiation statements.
        """
        # Step 1: Declare local struct instance
        if type_override:
            scoped_cpp_type = (
                self.strip_namespaces(type_override)
                .replace("&", "")
                .replace("const", "")
                .strip()
            )
        else:
            scoped_cpp_type = get_scoped_cpp_name(struct_cls) or struct_cls['name']
        lines = [f"{scoped_cpp_type} {var_name};"]

        # Step 2: Populate each field from primitive leaf parameters
        for f in struct_cls.get("fields", []):
            fname = f["name"]
            if is_reserved_or_padding_field(fname):
                continue
            fcap = fname[0].upper() + fname[1:]
            sub_prefix = f"{leaf_prefix}{fcap}"

            arr_info = self.get_array_input_info(f["type"])
            if arr_info:
                size = arr_info["size"]
                if arr_info.get("is_fixed_array"):
                    for idx in range(size):
                        comp_arg = f"{sub_prefix[0].lower() + sub_prefix[1:]}{idx}"
                        lines.append(f"{var_name}.{fname}[{idx}] = {comp_arg};")
                else:
                    comp_args = [
                        f"{sub_prefix[0].lower() + sub_prefix[1:]}{c.upper()}"
                        for c in ["x", "y", "z", "w"][:size]
                    ]
                    lines.append(f"{var_name}.{fname} = {{ {', '.join(comp_args)} }};")
            elif self.is_aggregate_struct(f["type"]):
                sub_struct = self.is_aggregate_struct(f["type"])
                sub_lines = self.generate_cpp_struct_construction(
                    sub_struct,  # type: ignore
                    f"{var_name}.{fname}",
                    leaf_prefix=sub_prefix
                )
                lines.extend(sub_lines[1:])
            else:
                param_name = f"{sub_prefix[0].lower() + sub_prefix[1:]}"
                f_info, _ = self.resolve_type_info(f["type"], silent=True)
                if f_info.get("is_slice"):
                    count_param = f"{param_name}Count"
                    if f_info.get("is_struct_slice"):
                        inner_cls = f_info["inner_info"].get("struct_cls")
                        inner_cpp = get_scoped_cpp_name(inner_cls) or f_info["inner_info"]["java"]
                        elem_var = f"{param_name}Elements"
                        lines.append(f"jlong* {elem_var} = nullptr;")
                        lines.append(f"if ({param_name} != nullptr && {count_param} > 0) {{")
                        lines.append(f"    {elem_var} = env->GetLongArrayElements({param_name}, nullptr);")
                        lines.append(f"    auto* structs = reinterpret_cast<{inner_cpp}*>({elem_var});")
                        lines.append(f"    {var_name}.{fname} = {{ structs, (size_t) {count_param} }};")
                        lines.append(f"}}")
                elif f_info.get("to_cpp"):
                    val_expr = f_info["to_cpp"].format(value=param_name)
                    lines.append(f"{var_name}.{fname} = {val_expr};")
                elif f_info.get("is_enum"):
                    cpp_enum_type = (
                        get_scoped_cpp_name(f_info.get("enum_ir", {}))
                        or self.strip_namespaces(f_info.get("cpp_type", ""))
                    )
                    lines.append(f"{var_name}.{fname} = ({cpp_enum_type}){param_name};")
                else:
                    lines.append(f"{var_name}.{fname} = {param_name};")
        return lines

    def generate_cpp_struct_cleanup(
        self,
        struct_cls: Dict[str, Any],
        leaf_prefix: str = ""
    ) -> List[str]:
        """Generate C++ cleanup statements releasing pinned arrays after native dispatch.

        Args:
            struct_cls: Struct class dictionary from IR.
            leaf_prefix: Prefix for leaf parameter names.

        Returns:
            List of C++ JNI cleanup release statements.
        """
        cleanup_lines = []
        for f in struct_cls.get("fields", []):
            fname = f["name"]
            if is_reserved_or_padding_field(fname):
                continue
            fcap = fname[0].upper() + fname[1:]
            sub_prefix = f"{leaf_prefix}{fcap}"
            if self.is_aggregate_struct(f["type"]):
                sub_struct = self.is_aggregate_struct(f["type"])
                cleanup_lines.extend(
                    self.generate_cpp_struct_cleanup(sub_struct, leaf_prefix=sub_prefix)  # type: ignore
                )
            else:
                param_name = f"{sub_prefix[0].lower() + sub_prefix[1:]}"
                f_info, _ = self.resolve_type_info(f["type"], silent=True)
                if f_info.get("is_slice") and f_info.get("is_struct_slice"):
                    elem_var = f"{param_name}Elements"
                    cleanup_lines.append(f"if ({elem_var} != nullptr) {{")
                    cleanup_lines.append(
                        f"    env->ReleaseLongArrayElements({param_name}, {elem_var}, JNI_ABORT);"
                    )
                    cleanup_lines.append(f"}}")
        return cleanup_lines

    def get_array_input_info(
        self,
        t: Union[str, Dict[str, Any]]
    ) -> Optional[Dict[str, Any]]:
        """Resolve array input information (math types, bone buffers, primitive pointers).

        Identifies types that map to primitive Java arrays (`float[]`, `int[]`, etc.)
        and computes their size, component type, and `@Size` validation annotations.

        Args:
            t: Type string or dictionary from IR.

        Returns:
            Dictionary detailing array sizes, scalar types, and annotations, or None.
        """
        # Step 1: Discard string types immediately
        if isinstance(t, dict) and (t.get("category") == "string" or t.get("is_string")):
            return None
        t_str = t.get("qualified_name") or t["cpp_name"] if isinstance(t, dict) else str(t)
        t_clean = t_str.replace("const ", "").strip()
        if t_clean in ("char*", "char *", "char const*", "const char*"):
            return None

        # Step 2: Check resolved math vectors and fixed arrays
        i, _ = self.resolve_type_info(t, silent=True)
        if "assert" in i:
            return i
        if i.get("is_fixed_array"):
            return i

        # Step 3: Handle raw pointer types
        t_clean = re.sub(r'\bconst\b', '', t_str).strip()
        t_strip = t_clean
        if t_strip.endswith("*") or (isinstance(t, dict) and t.get("is_pointer", False)):
            if t_strip.endswith("*"):
                t_strip = t_strip[:-1].strip()
            clean_elem = self.strip_namespaces(t_strip)
            if clean_elem == "Bone" or t_strip.endswith("::Bone"):
                return {
                    "java": "float[]",
                    "jni": "jfloatArray",
                    "jni_type": "jfloatArray",
                    "scalar": "float",
                    "size": 8,
                    "annotation": "@Size(min = 8)",
                    "cpp_type": "RenderableManager::Bone",
                    "is_custom_array": True
                }
            i, _ = self.resolve_type_info(t_strip, silent=True)
            if "assert" in i:
                return i
            if i.get("is_filament_type"):
                return None

            if i.get("is_struct"):
                elem_cast = self.strip_namespaces(t_strip)
                struct_cls = i.get("struct_cls") or self.is_aggregate_struct(t_strip)
                stride = self.get_struct_float_stride(struct_cls)
                if stride is not None:
                    return {
                        "java": "float[]",
                        "jni": "jfloatArray",
                        "jni_type": "jfloatArray",
                        "scalar": "float",
                        "size": stride,
                        "annotation": f"@Size(min = {stride})",
                        "cpp_type": elem_cast,
                        "is_custom_array": True
                    }
                return {
                    "java": "long[]",
                    "jni": "jlongArray",
                    "jni_type": "jlongArray",
                    "scalar": "long",
                    "size": 1,
                    "annotation": i.get("annotation"),
                    "cpp_type": elem_cast,
                    "is_custom_array": True
                }

            scalar = i.get("scalar") or i.get("java")
            if scalar in ["byte", "short", "int", "long", "float", "double", "boolean"]:
                jni_scalar = i.get("jni", f"j{scalar}")
                elem_cast = self.strip_namespaces(t_strip)
                return {
                    "java": f"{i['java']}[]",
                    "jni": f"{jni_scalar}Array",
                    "jni_type": f"{jni_scalar}Array",
                    "scalar": scalar,
                    "size": 1,
                    "annotation": i.get("annotation"),
                    "cpp_type": elem_cast,
                    "is_custom_array": True
                }

        # Step 4: Handle utils::Slice types
        slice_match = re.match(
            r"^(?:const\s+)?(?:filament::)?(?:utils::)?Slice<\s*(?:const\s+)?(.*?)(?:\s+const)?\s*>\s*(?:&)?$",
            t_str.strip()
        )
        if slice_match:
            inner_type_str = slice_match.group(1).strip()
            is_const = bool(re.search(r"Slice<\s*const\b", t_str) or re.search(r"\bconst\s*>", t_str))
            clean_elem = self.strip_namespaces(inner_type_str)
            if clean_elem == "Bone" or inner_type_str.endswith("::Bone"):
                return {
                    "java": "float[]",
                    "jni": "jfloatArray",
                    "jni_type": "jfloatArray",
                    "scalar": "float",
                    "size": 8,
                    "annotation": "@Size(min = 8)",
                    "cpp_type": "RenderableManager::Bone",
                    "is_custom_array": True,
                    "is_slice": True,
                    "is_const": is_const,
                }
            i, _ = self.resolve_type_info(inner_type_str, silent=True)
            if "assert" in i:
                res = dict(i)
                res["is_slice"] = True
                res["is_const"] = is_const
                res["cpp_type"] = i.get("cpp_type") or inner_type_str
                return res
            if i.get("is_struct"):
                elem_cast = self.strip_namespaces(inner_type_str)
                struct_cls = i.get("struct_cls") or self.is_aggregate_struct(inner_type_str)
                stride = self.get_struct_float_stride(struct_cls)
                if stride is not None:
                    return {
                        "java": "float[]",
                        "jni": "jfloatArray",
                        "jni_type": "jfloatArray",
                        "scalar": "float",
                        "size": stride,
                        "annotation": f"@Size(min = {stride})",
                        "cpp_type": elem_cast,
                        "is_custom_array": True,
                        "is_slice": True,
                        "is_const": is_const,
                    }
                return None

            scalar = i.get("scalar") or i.get("java")
            if scalar in ["byte", "short", "int", "long", "float", "double", "boolean"]:
                jni_scalar = i.get("jni", f"j{scalar}")
                elem_cast = self.strip_namespaces(inner_type_str)
                return {
                    "java": f"{i['java']}[]",
                    "jni": f"{jni_scalar}Array",
                    "jni_type": f"{jni_scalar}Array",
                    "scalar": scalar,
                    "size": 1,
                    "annotation": i.get("annotation"),
                    "cpp_type": elem_cast,
                    "is_custom_array": True,
                    "is_slice": True,
                    "is_const": is_const,
                }
        return None

    def get_java_type(self, type_input: Union[str, Dict[str, Any]]) -> str:
        """Resolve the simple Java type string for an input C++ type.

        Strips outer enclosing class prefix if type refers to an inner member of the active class.

        Args:
            type_input: Type string or dictionary.

        Returns:
            Java type string (e.g. 'int', 'String', 'Buffer', 'ToneMapper').
        """
        info, _ = self.resolve_type_info(type_input, silent=True)
        java_type = info["java"]
        if java_type.startswith(f"{self.name}."):
            return java_type[len(self.name) + 1:]
        return java_type

    def get_jni_type(self, type_input: Union[str, Dict[str, Any]]) -> str:
        """Resolve the JNI C++ type string for an input C++ type.

        Args:
            type_input: Type string or dictionary.

        Returns:
            JNI type string (e.g. 'jint', 'jlong', 'jfloatArray', 'jobject').
        """
        t = type_input
        if isinstance(t, str):
            t = t.strip()
        info, _ = self.resolve_type_info(t, silent=True)
        return info.get("jni_type", info.get("jni", "void"))

    def get_annotation(self, type_input: Union[str, Dict[str, Any]]) -> Optional[str]:
        """Resolve the Java AndroidX annotation for a type (e.g. @IntRange, @Entity).

        Args:
            type_input: Type string or dictionary.

        Returns:
            Annotation string if applicable, or None.
        """
        info, _ = self.resolve_type_info(type_input, silent=True)
        return info.get("annotation")

    def get_to_cpp_expr(self, type_input: Union[str, Dict[str, Any]], val_name: str) -> str:
        """Generate C++ expression marshalling a JNI value to native C++ invocation arguments.

        Args:
            type_input: Type string or dictionary.
            val_name: Name of the JNI parameter variable.

        Returns:
            Formatted C++ expression string.
        """
        cpp_type = (
            type_input.get("qualified_name") or type_input["cpp_name"]
            if isinstance(type_input, dict)
            else type_input
        )
        resolved_type = self.strip_namespaces(cpp_type)
        if cpp_type in self.aliases:
            resolved_type = self.aliases[cpp_type]

        info, groups = self.resolve_type_info(type_input)
        tmpl = info.get("to_cpp")
        if tmpl:
            stripped_groups = [self.strip_namespaces(g) for g in groups]
            return tmpl.format(*stripped_groups, type=resolved_type, value=val_name)

        if info.get("is_filament_type"):
            return f"({resolved_type}){val_name}"

        return f"({resolved_type}){val_name}"

    def get_from_cpp_expr(
        self,
        type_input: Union[str, Dict[str, Any]],
        val_expr: str,
        jni_ret: str
    ) -> str:
        """Generate C++ expression converting a native C++ return value to JNI representation.

        Args:
            type_input: Type string or dictionary.
            val_expr: Native C++ return expression.
            jni_ret: Target JNI return type string.

        Returns:
            Formatted C++ expression returning to JNI caller.
        """
        cpp_type = (
            type_input.get("qualified_name") or type_input["cpp_name"]
            if isinstance(type_input, dict)
            else type_input
        )
        resolved_type = self.strip_namespaces(cpp_type)
        info, groups = self.resolve_type_info(type_input)
        if info.get("is_string"):
            clean_type = (
                self.strip_namespaces(cpp_type)
                .replace("&", "")
                .replace("const", "")
                .strip()
            )
            if "string_view" in clean_type or "basic_string_view" in clean_type:
                return f"env->NewStringUTF(std::string({val_expr}).c_str())"
            elif clean_type in ("CString", "StaticString", "ImmutableCString", "string"):
                return f"env->NewStringUTF(({val_expr}).c_str())"
            else:
                return f"({val_expr}) ? env->NewStringUTF({val_expr}) : nullptr"
        tmpl = info.get("from_cpp")
        stripped_groups = [self.strip_namespaces(g) for g in groups]
        if tmpl:
            return tmpl.format(*stripped_groups, type=resolved_type, value=val_expr)
        if jni_ret != "void":
            if jni_ret == "jlong" and isinstance(type_input, dict):
                is_ref = type_input.get("is_reference", False) or "&" in cpp_type
                is_ptr = type_input.get("is_pointer", False) or "*" in cpp_type
                cat = type_input.get("category", "")
                if is_ref and not is_ptr and cat != "primitive":
                    return f"({jni_ret})&({val_expr})"
            return f"({jni_ret}){val_expr}"
        return val_expr
