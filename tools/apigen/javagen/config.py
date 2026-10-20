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

"""Constants, type registries, and configuration tables for JavaGen.

This module encapsulates all static mappings between C++ primitives, math constructs,
engine handles, and their corresponding Java and JNI types. It also maintains the
global registry of known classes discovered during multi-file IR analysis.
"""

import re
from typing import Any, Dict, Iterable, List, Pattern, Set, Tuple


def _expand_keys(entries: Iterable[Tuple[Iterable[str], Dict[str, Any]]]) -> Dict[str, Dict[str, Any]]:
    """Expand grouped key aliases into a flat lookup dictionary."""
    result: Dict[str, Dict[str, Any]] = {}
    for keys, val in entries:
        for k in keys:
            result[k] = val
    return result


# -----------------------------------------------------------------------------
# C++ TO JAVA / JNI TYPE MAPPING TABLES
# -----------------------------------------------------------------------------

# Maps fundamental C++ types to Java types, JNI types, and optional annotations
TYPE_MAP: Dict[str, Dict[str, Any]] = _expand_keys([
    (("void",), {"java": "void", "jni": "void", "jni_type": "void"}),
    (("bool",), {"java": "boolean", "jni": "jboolean", "jni_type": "jboolean"}),

    # Signed integer types (8, 16, 32-bit -> int / jint)
    (("char", "signed char", "int8_t", "short", "int16_t", "int32_t", "ssize_t"),
     {"java": "int", "jni": "jint", "jni_type": "jint"}),
    (("int",),
     {"java": "int", "jni": "jint", "jni_type": "jint", "to_cpp": "{value}"}),

    # Unsigned integer types (8, 16, 32-bit, size_t -> @IntRange(from = 0) int / jint)
    (("unsigned char", "uint8_t", "unsigned short", "uint16_t", "unsigned int", "uint32_t", "size_t"),
     {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"}),

    # 64-bit integer and pointer types (-> long / jlong)
    (("long", "long long", "int64_t", "intptr_t", "void*", "const void*", "void *", "const void *"),
     {"java": "long", "jni": "jlong", "jni_type": "jlong"}),
    (("unsigned long", "unsigned long long", "uint64_t", "uintptr_t"),
     {"java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)"}),

    # Floating-point types
    (("float",), {"java": "float", "jni": "jfloat", "jni_type": "jfloat", "to_cpp": "{value}"}),
    (("double",), {"java": "double", "jni": "jdouble", "jni_type": "jdouble", "to_cpp": "{value}"}),

    # Strings and string views
    (("const char*", "const char *"),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "const char*"}),
    (("char*", "char *"),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "char*"}),
    (("std::string_view",),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view"}),
    (("std::string",),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string"}),
    (("utils::CString",),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::CString"}),
    (("utils::StaticString",),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::StaticString"}),
    (("utils::ImmutableCString",),
     {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::ImmutableCString"}),

    # Tribool and optional<bool>
    (("utils::tribool",), {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "utils::tribool((bool){value})",
        "from_cpp": "(jboolean)(({value}).is_true())",
    }),
    (("std::optional<bool>",), {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "std::make_optional((bool){value})",
        "from_cpp": "(jboolean)({value}.value_or(false))",
    }),

    # PixelBufferDescriptor
    (("backend::PixelBufferDescriptor", "filament::backend::PixelBufferDescriptor",
      "PixelBufferDescriptor", "Texture::PixelBufferDescriptor"),
     {"java": "PixelBufferDescriptor", "jni": "jobject", "jni_type": "jobject"}),

    # ExternalImageHandle
    (("backend::Platform::ExternalImageHandle", "backend::Platform::ExternalImageHandleRef",
      "filament::backend::Platform::ExternalImageHandle", "filament::backend::Platform::ExternalImageHandleRef",
      "ExternalImageHandle", "ExternalImageHandleRef",
      "Texture::ExternalImageHandle", "Texture::ExternalImageHandleRef"), {
        "java": "long", "jni": "jlong", "jni_type": "jlong",
        "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})",
    }),

    # Cross-header Enums
    (("filament::backend::CullingMode", "backend::CullingMode", "CullingMode"), {
        "java": "Material.CullingMode", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "cpp_enum_type": "backend::CullingMode",
        "to_cpp": "(backend::CullingMode){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::TransparencyMode",), {
        "java": "Material.TransparencyMode", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "cpp_enum_type": "TransparencyMode",
        "to_cpp": "(TransparencyMode){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::backend::SamplerCompareFunc", "backend::SamplerCompareFunc",
      "SamplerCompareFunc", "DepthFunc", "StencilCompareFunc"), {
        "java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc",
        "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::backend::CompilerPriorityQueue", "backend::CompilerPriorityQueue", "CompilerPriorityQueue"), {
        "java": "Material.CompilerPriorityQueue", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "cpp_enum_type": "backend::CompilerPriorityQueue",
        "to_cpp": "(backend::CompilerPriorityQueue){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::backend::FeatureLevel", "backend::FeatureLevel", "FeatureLevel"), {
        "java": "Engine.FeatureLevel", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "enum_name": "FeatureLevel", "cpp_enum_type": "backend::FeatureLevel",
        "to_cpp": "(backend::FeatureLevel){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::RgbType", "RgbType", "filament::Color::RgbType", "Color::RgbType"), {
        "java": "Colors.RgbType", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "cpp_enum_type": "RgbType",
        "to_cpp": "(RgbType){value}", "from_cpp": "(jint){value}",
    }),
    (("filament::RgbaType", "RgbaType", "filament::Color::RgbaType", "Color::RgbaType"), {
        "java": "Colors.RgbaType", "jni": "jint", "jni_type": "jint",
        "is_enum": True, "cpp_enum_type": "RgbaType",
        "to_cpp": "(RgbaType){value}", "from_cpp": "(jint){value}",
    }),
])


def _build_math_types() -> Dict[str, Dict[str, Any]]:
    """Build the lookup table for Filament math vectors, matrices, and quaternions."""
    table: Dict[str, Dict[str, Any]] = {}

    vec_families = [
        # (short_prefix, scalar_java, jni_arr, assert_prefix, tvec_types)
        ("float", "float", "jfloatArray", "assertFloat", ("float",)),
        ("double", "double", "jdoubleArray", "assertDouble", ("double",)),
        ("bool", "boolean", "jbooleanArray", "assertBoolean", ()),
        ("short", "short", "jshortArray", "assertShort", ("short", "int16_t")),
        ("ushort", "int", "jintArray", "assertInt", ()),
        ("int", "int", "jintArray", "assertInt", ("int", "int32_t")),
        ("uint", "int", "jintArray", "assertInt", ("uint32_t",)),
        ("byte", "byte", "jbyteArray", "assertByte", ()),
        ("ubyte", "byte", "jbyteArray", "assertByte", ()),
    ]

    for prefix, scalar, jni_arr, assert_prefix, tvec_scalars in vec_families:
        for dim in (2, 3, 4):
            cpp_type = f"math::{prefix}{dim}"
            entry = {
                "java": f"{scalar}[]",
                "jni": jni_arr,
                "jni_type": jni_arr,
                "scalar": scalar,
                "size": dim,
                "assert": f"{assert_prefix}{dim}",
                "cpp_type": cpp_type,
            }
            table[cpp_type] = entry
            for t_scalar in tvec_scalars:
                table[f"filament::math::details::TVec{dim}<{t_scalar}>"] = entry
                if t_scalar == "int16_t" and dim == 4:
                    table["math::details::TVec4<int16_t>"] = entry
                    table["details::TVec4<int16_t>"] = entry

    for dim in (2, 3, 4):
        size = dim * dim
        matf_entry = {
            "java": "float[]",
            "jni": "jfloatArray",
            "jni_type": "jfloatArray",
            "scalar": "float",
            "size": size,
            "assert": f"assertMat{dim}f",
            "cpp_type": f"math::mat{dim}f",
        }
        table[f"math::mat{dim}f"] = matf_entry
        table[f"filament::math::details::TMat{dim}{dim}<float>"] = matf_entry

        matd_entry = {
            "java": "double[]",
            "jni": "jdoubleArray",
            "jni_type": "jdoubleArray",
            "scalar": "double",
            "size": size,
            "assert": f"assertMat{dim}d",
            "cpp_type": f"math::mat{dim}",
        }
        table[f"math::mat{dim}"] = matd_entry
        table[f"filament::math::details::TMat{dim}{dim}<double>"] = matd_entry

    quatf_entry = {
        "java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray",
        "scalar": "float", "size": 4, "assert": "assertFloat4",
        "cpp_type": "math::quatf", "is_quaternion": True,
    }
    for k in ("math::quatf", "filament::math::details::TQuaternion<float>", "math::details::TQuaternion<float>"):
        table[k] = quatf_entry

    quatd_entry = {
        "java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray",
        "scalar": "double", "size": 4, "assert": "assertDouble4",
        "cpp_type": "math::quat", "is_quaternion": True,
    }
    for k in ("math::quat", "filament::math::details::TQuaternion<double>", "math::details::TQuaternion<double>"):
        table[k] = quatd_entry

    return table


# Math Types Configuration (Vectors, Matrices, and Quaternions)
MATH_TYPES: Dict[str, Dict[str, Any]] = _build_math_types()

# Value Types Configuration (Inline Buffer Objects and Descriptors)
VALUE_TYPES: Dict[str, Dict[str, Any]] = _expand_keys([
    (("filament::Frustum", "Frustum"), {
        "java": "Frustum",
        "jni": "jfloatArray",
        "jni_type": "jfloatArray",
        "archetype": "inline_buffer",
        "is_value_object": True,
        "element_type": "float",
        "size": 24,
        "field_name": "mPlanes",
        "cpp_type": "filament::Frustum",
        "header": "filament/Frustum.h",
        "buffer_getter": "getNormalizedPlanes",
    }),
    (("filament::backend::PixelBufferDescriptor", "backend::PixelBufferDescriptor",
      "PixelBufferDescriptor", "Texture::PixelBufferDescriptor"), {
        "java": "PixelBufferDescriptor",
        "is_value_object": True,
    }),
])

# Tagged Scalar Array Families Configuration (for UTILS_APIGEN_TAGGED_ARRAY)
TAGGED_SCALAR_FAMILIES: Dict[str, Dict[str, str]] = {
    "float": {
        "family": "Float",
        "enum_name": "FloatElement",
        "java_scalar": "float",
        "java_array": "float[]",
        "jni_array": "jfloatArray",
        "jni_scalar_ptr": "jfloat*",
        "get_elements": "GetFloatArrayElements",
        "release_elements": "ReleaseFloatArrayElements",
    },
    "int": {
        "family": "Int",
        "enum_name": "IntElement",
        "java_scalar": "int",
        "java_array": "int[]",
        "jni_array": "jintArray",
        "jni_scalar_ptr": "jint*",
        "get_elements": "GetIntArrayElements",
        "release_elements": "ReleaseIntArrayElements",
    },
    "bool": {
        "family": "Boolean",
        "enum_name": "BooleanElement",
        "java_scalar": "boolean",
        "java_array": "boolean[]",
        "jni_array": "jbooleanArray",
        "jni_scalar_ptr": "jboolean*",
        "get_elements": "GetBooleanArrayElements",
        "release_elements": "ReleaseBooleanArrayElements",
    },
}

# Custom enum entry renames for Java API compatibility
ENUM_ENTRY_RENAMES: Dict[str, Dict[str, str]] = {
    "StencilOperation": {
        "INCR": "INCR_CLAMP",
        "DECR": "DECR_CLAMP",
    }
}

# Regex-based type matching patterns for template and qualified types
TYPE_PATTERNS: List[Tuple[str, Dict[str, Any]]] = [
    (r"^(?:filament::)?(?:utils::)?Entity$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@Entity",
        "to_cpp": "Entity::import({value})",
        "from_cpp": "(jint)Entity::smuggle({value})",
    }),
    (r"^(?:filament::)?(?:utils::)?EntityInstance<(.*)>$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@EntityInstance",
        "to_cpp": "EntityInstance<{0}>({value})",
        "from_cpp": "(jint){value}.asValue()",
    }),
    (r"^(?:const\s+)?(?:std::)?basic_string_view<char>(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view",
    }),
    (r"^(?:const\s+)?(?:std::)?string_view(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view",
    }),
    (r"^(?:const\s+)?(?:std::)?string(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string",
    }),
    (r"^(?:const\s+)?(?:utils::)?CString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::CString",
    }),
    (r"^(?:const\s+)?(?:utils::)?StaticString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::StaticString",
    }),
    (r"^(?:const\s+)?(?:utils::)?ImmutableCString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::ImmutableCString",
    }),
    (r"^(?:const\s+)?(?:filament::)?(?:backend::)?BufferDescriptor(?:\s*&&|\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_buffer_descriptor": True, "cpp_type": "backend::BufferDescriptor",
    }),
    (r"^(?:const\s+)?(?:filament::)?(?:BufferObject::|IndexBuffer::|VertexBuffer::)?BufferDescriptor(?:\s*&&|\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_buffer_descriptor": True, "cpp_type": "backend::BufferDescriptor",
    }),
    (r"^(?:const\s+)?(?:utils::)?FixedCapacityVector<(?:(?:filament::)?math::float3|(?:filament::)?math::details::TVec3<float>|details::TVec3<float>|float3)>(?:\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_custom_lut_buffer": True, "cpp_type": "utils::FixedCapacityVector<math::float3>",
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::duration<.*>(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::nanoseconds({value})",
        "from_cpp": "(jlong){value}.count()",
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::time_point<.*>(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::steady_clock::time_point(std::chrono::nanoseconds({value}))",
        "from_cpp": "(jlong){value}.time_since_epoch().count()",
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::nanoseconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::nanoseconds({value})",
        "from_cpp": "(jlong){value}.count()",
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::milliseconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::milliseconds({value})",
        "from_cpp": "(jlong){value}.count()",
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::seconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::seconds({value})",
        "from_cpp": "(jlong){value}.count()",
    }),
    (r"^(?:const\s+)?(?:utils::)?tribool(?:\s*&)?$", {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "utils::tribool((bool){value})",
        "from_cpp": "(jboolean)(({value}).is_true())",
    }),
    (r"^(?:const\s+)?(?:std::)?optional<bool>(?:\s*&)?$", {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "std::make_optional((bool){value})",
        "from_cpp": "(jboolean)({value}.value_or(false))",
    }),
    (r"^(?:const\s+)?utils::bitset<.*>(?:\s*&)?$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)",
        "to_cpp": "{type}({value})",
        "from_cpp": "(jint){value}.getValue()",
    }),
    (r"^(?:const\s+)?(?:std::)?bitset<.*>(?:\s*&)?$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)",
        "to_cpp": "{type}({value})",
        "from_cpp": "(jint){value}.to_ulong()",
    }),
]

# Pre-compiled regex type patterns for O(1) compiled matching in TypeResolver
COMPILED_TYPE_PATTERNS: List[Tuple[Pattern[str], Dict[str, Any]]] = [
    (re.compile(pat), info) for pat, info in TYPE_PATTERNS
]

# Package mapping for C++ headers
PACKAGE_MAP: Dict[str, str] = {
    "filament/View.h": "com.google.android.filament.View",
}

# Standard Android Open Source Project Apache 2.0 license header for generated sources
LICENSE_HEADER: str = """/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */"""

# Generated file warning banner
GENERATED_FILE_WARNING: str = "// THIS FILE IS GENERATED BY APIGEN AND MUST NOT BE HAND-MODIFIED."

# Reserved Java keywords to prevent illegal method or parameter identifier emission
JAVA_KEYWORDS: Set[str] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char",
    "class", "const", "continue", "default", "do", "double", "else", "enum",
    "extends", "final", "finally", "float", "for", "goto", "if", "implements",
    "import", "instanceof", "int", "interface", "long", "native", "new",
    "package", "private", "protected", "public", "return", "short", "static",
    "strictfp", "super", "switch", "synchronized", "this", "throw", "throws",
    "transient", "try", "void", "volatile", "while", "true", "false", "null",
}

# Global Registry of Known Classes / Structs across IR files
KNOWN_CLASSES: Dict[str, Dict[str, Any]] = {}


def register_known_classes(classes: List[Dict[str, Any]]) -> None:
    """Register extracted C++ classes and structs into the global known classes registry.

    Architectural Purpose:
        C++ AST parsing occurs on a per-header basis, meaning that when generating
        bindings for a class like `Frustum` that interacts with an aggregate struct
        like `Box` (declared in `Box.h`), `Frustum`'s JSON IR may only contain a
        forward declaration or reference to `Box`.
        To resolve composite field layouts and unroll struct parameters into
        primitive CPU registers across JNI, `JavaGen` indexes all classes and structs
        across both the primary IR file and sibling IR files in the workspace.

    Multi-Key Indexing Strategy:
        A single C++ struct may be referenced via various naming styles depending
        on using-directives, namespace qualifiers, or nested scopes. To ensure O(1)
        discovery regardless of caller qualification, each class is indexed under:
        1. Simple Name (e.g. `Box`, `ShadowOptions`)
        2. Fully-Qualified Name (e.g. `filament::Box`, `filament::LightManager::ShadowOptions`)
        3. Stripped Namespace Variant (e.g. `Box` with `filament::`, `backend::`, etc. removed)
        4. Cross-Namespace Backend Aliases (e.g. `backend::BufferDescriptor` mapped
           interchangeably with `filament::backend::BufferDescriptor`).

    Walkthrough Example:
        Given `{"name": "Box", "qualified_name": "filament::Box"}`:
        - `KNOWN_CLASSES["Box"]` -> `cls`
        - `KNOWN_CLASSES["filament::Box"]` -> `cls`
        - `KNOWN_CLASSES["Box"]` (clean) -> `cls`
        - `KNOWN_CLASSES["backend::Box"]` -> `cls`
        - `KNOWN_CLASSES["filament::backend::Box"]` -> `cls`

    Args:
        classes: List of class or struct dictionary definitions parsed from JSON IR.

    Side Effects:
        Mutates the module-global `KNOWN_CLASSES` dictionary in place.
    """
    for cls in classes:
        # Step 1: Index under unqualified simple name
        KNOWN_CLASSES[cls["name"]] = cls

        # Step 2: If qualified name is available, index across namespace aliases
        qname = cls.get("qualified_name")
        if qname:
            KNOWN_CLASSES[qname] = cls

            # Step 2a: Strip top-level engine namespace prefixes for unqualified fallback
            clean = (
                qname.replace("filament::", "")
                .replace("backend::", "")
                .replace("utils::", "")
                .replace("math::", "")
            )
            KNOWN_CLASSES[clean] = cls

            # Step 2b: Handle filament:: vs backend:: / filament::backend:: re-exports
            if "filament::" in qname:
                KNOWN_CLASSES[qname.replace("filament::", "backend::")] = cls
                KNOWN_CLASSES[qname.replace("filament::", "filament::backend::")] = cls


# -----------------------------------------------------------------------------
# RESERVED JAVA OBJECT METHODS
# -----------------------------------------------------------------------------

# Set of zero-argument final methods defined on java.lang.Object that must not be
# synthesized as zero-argument convenience overloads to prevent compiler collision.
JAVA_OBJECT_FINAL_NOARG_METHODS: Set[str] = {
    "wait",
    "notify",
    "notifyAll",
    "getClass",
}
