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

from typing import Any, Dict, List, Set, Tuple

# -----------------------------------------------------------------------------
# C++ TO JAVA / JNI TYPE MAPPING TABLES
# -----------------------------------------------------------------------------

# Maps fundamental C++ types to Java types, JNI types, and optional annotations
TYPE_MAP: Dict[str, Dict[str, Any]] = {
    "void": {"java": "void", "jni": "void", "jni_type": "void"},
    "bool": {"java": "boolean", "jni": "jboolean", "jni_type": "jboolean"},

    # Char types
    "char": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "unsigned char": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},
    "signed char": {"java": "int", "jni": "jint", "jni_type": "jint"},

    # 8-bit integers
    "int8_t": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "uint8_t": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},

    # 16-bit integers
    "short": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "unsigned short": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},
    "int16_t": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "uint16_t": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},

    # 32-bit integers
    "int": {"java": "int", "jni": "jint", "jni_type": "jint", "to_cpp": "{value}"},
    "unsigned int": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},
    "int32_t": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "uint32_t": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},

    # 64-bit integers
    "long": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "unsigned long": {"java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)"},
    "long long": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "unsigned long long": {"java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)"},
    "int64_t": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "uint64_t": {"java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)"},

    # Architecture-dependent pointer/size types
    "size_t": {"java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)"},
    "ssize_t": {"java": "int", "jni": "jint", "jni_type": "jint"},
    "intptr_t": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "uintptr_t": {"java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)"},
    "void*": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "const void*": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "void *": {"java": "long", "jni": "jlong", "jni_type": "jlong"},
    "const void *": {"java": "long", "jni": "jlong", "jni_type": "jlong"},

    # Floating-point types
    "float": {"java": "float", "jni": "jfloat", "jni_type": "jfloat", "to_cpp": "{value}"},
    "double": {"java": "double", "jni": "jdouble", "jni_type": "jdouble", "to_cpp": "{value}"},

    # Strings and string views
    "const char*": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "const char*"},
    "const char *": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "const char*"},
    "char*": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "char*"},
    "char *": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "char*"},
    "std::string_view": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view"},
    "std::string": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string"},
    "utils::CString": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::CString"},
    "utils::StaticString": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::StaticString"},
    "utils::ImmutableCString": {"java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::ImmutableCString"},

    # Tribool and optional
    "utils::tribool": {"java": "boolean", "jni": "jboolean", "jni_type": "jboolean", "to_cpp": "utils::tribool((bool){value})", "from_cpp": "(jboolean)(({value}).is_true())"},
    "std::optional<bool>": {"java": "boolean", "jni": "jboolean", "jni_type": "jboolean", "to_cpp": "std::make_optional((bool){value})", "from_cpp": "(jboolean)({value}.value_or(false))"},

    # Buffer descriptors
    "backend::PixelBufferDescriptor": {"java": "PixelBufferDescriptor", "jni": "jobject", "jni_type": "jobject"},
    "filament::backend::PixelBufferDescriptor": {"java": "PixelBufferDescriptor", "jni": "jobject", "jni_type": "jobject"},
    "PixelBufferDescriptor": {"java": "PixelBufferDescriptor", "jni": "jobject", "jni_type": "jobject"},
    "Texture::PixelBufferDescriptor": {"java": "PixelBufferDescriptor", "jni": "jobject", "jni_type": "jobject"},

    # External image handles
    "backend::Platform::ExternalImageHandle": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "backend::Platform::ExternalImageHandleRef": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "filament::backend::Platform::ExternalImageHandle": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "filament::backend::Platform::ExternalImageHandleRef": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "ExternalImageHandle": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "ExternalImageHandleRef": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "Texture::ExternalImageHandle": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},
    "Texture::ExternalImageHandleRef": {"java": "long", "jni": "jlong", "jni_type": "jlong", "to_cpp": "backend::Platform::ExternalImageHandle((backend::Platform::ExternalImage*){value})"},

    # Special Enums
    "filament::backend::CullingMode": {"java": "Material.CullingMode", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CullingMode", "to_cpp": "(backend::CullingMode){value}", "from_cpp": "(jint){value}"},
    "backend::CullingMode": {"java": "Material.CullingMode", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CullingMode", "to_cpp": "(backend::CullingMode){value}", "from_cpp": "(jint){value}"},
    "CullingMode": {"java": "Material.CullingMode", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CullingMode", "to_cpp": "(backend::CullingMode){value}", "from_cpp": "(jint){value}"},
    "filament::TransparencyMode": {"java": "Material.TransparencyMode", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "TransparencyMode", "to_cpp": "(TransparencyMode){value}", "from_cpp": "(jint){value}"},
    "filament::backend::SamplerCompareFunc": {"java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc", "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}"},
    "backend::SamplerCompareFunc": {"java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc", "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}"},
    "SamplerCompareFunc": {"java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc", "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}"},
    "DepthFunc": {"java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc", "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}"},
    "StencilCompareFunc": {"java": "TextureSampler.CompareFunc", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "CompareFunc", "cpp_enum_type": "backend::SamplerCompareFunc", "to_cpp": "(backend::SamplerCompareFunc){value}", "from_cpp": "(jint){value}"},
    "filament::backend::CompilerPriorityQueue": {"java": "Material.CompilerPriorityQueue", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CompilerPriorityQueue", "to_cpp": "(backend::CompilerPriorityQueue){value}", "from_cpp": "(jint){value}"},
    "backend::CompilerPriorityQueue": {"java": "Material.CompilerPriorityQueue", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CompilerPriorityQueue", "to_cpp": "(backend::CompilerPriorityQueue){value}", "from_cpp": "(jint){value}"},
    "CompilerPriorityQueue": {"java": "Material.CompilerPriorityQueue", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "backend::CompilerPriorityQueue", "to_cpp": "(backend::CompilerPriorityQueue){value}", "from_cpp": "(jint){value}"},
    "filament::backend::FeatureLevel": {"java": "Engine.FeatureLevel", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "FeatureLevel", "cpp_enum_type": "backend::FeatureLevel", "to_cpp": "(backend::FeatureLevel){value}", "from_cpp": "(jint){value}"},
    "backend::FeatureLevel": {"java": "Engine.FeatureLevel", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "FeatureLevel", "cpp_enum_type": "backend::FeatureLevel", "to_cpp": "(backend::FeatureLevel){value}", "from_cpp": "(jint){value}"},
    "FeatureLevel": {"java": "Engine.FeatureLevel", "jni": "jint", "jni_type": "jint", "is_enum": True, "enum_name": "FeatureLevel", "cpp_enum_type": "backend::FeatureLevel", "to_cpp": "(backend::FeatureLevel){value}", "from_cpp": "(jint){value}"},
    "filament::RgbType": {"java": "Colors.RgbType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbType", "to_cpp": "(RgbType){value}", "from_cpp": "(jint){value}"},
    "RgbType": {"java": "Colors.RgbType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbType", "to_cpp": "(RgbType){value}", "from_cpp": "(jint){value}"},
    "filament::Color::RgbType": {"java": "Colors.RgbType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbType", "to_cpp": "(RgbType){value}", "from_cpp": "(jint){value}"},
    "Color::RgbType": {"java": "Colors.RgbType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbType", "to_cpp": "(RgbType){value}", "from_cpp": "(jint){value}"},
    "filament::RgbaType": {"java": "Colors.RgbaType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbaType", "to_cpp": "(RgbaType){value}", "from_cpp": "(jint){value}"},
    "RgbaType": {"java": "Colors.RgbaType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbaType", "to_cpp": "(RgbaType){value}", "from_cpp": "(jint){value}"},
    "filament::Color::RgbaType": {"java": "Colors.RgbaType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbaType", "to_cpp": "(RgbaType){value}", "from_cpp": "(jint){value}"},
    "Color::RgbaType": {"java": "Colors.RgbaType", "jni": "jint", "jni_type": "jint", "is_enum": True, "cpp_enum_type": "RgbaType", "to_cpp": "(RgbaType){value}", "from_cpp": "(jint){value}"},
}

# Math Types Configuration (Vectors, Matrices, and Quaternions)
MATH_TYPES: Dict[str, Dict[str, Any]] = {
    "math::float2": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 2, "assert": "assertFloat2", "cpp_type": "math::float2"},
    "math::float3": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 3, "assert": "assertFloat3", "cpp_type": "math::float3"},
    "math::float4": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertFloat4", "cpp_type": "math::float4"},
    "math::double2": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 2, "assert": "assertDouble2", "cpp_type": "math::double2"},
    "math::double3": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 3, "assert": "assertDouble3", "cpp_type": "math::double3"},
    "math::double4": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertDouble4", "cpp_type": "math::double4"},
    "math::bool2": {"java": "boolean[]", "jni": "jbooleanArray", "jni_type": "jbooleanArray", "scalar": "boolean", "size": 2, "assert": "assertBoolean2", "cpp_type": "math::bool2"},
    "math::bool3": {"java": "boolean[]", "jni": "jbooleanArray", "jni_type": "jbooleanArray", "scalar": "boolean", "size": 3, "assert": "assertBoolean3", "cpp_type": "math::bool3"},
    "math::bool4": {"java": "boolean[]", "jni": "jbooleanArray", "jni_type": "jbooleanArray", "scalar": "boolean", "size": 4, "assert": "assertBoolean4", "cpp_type": "math::bool4"},

    "math::mat2f": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertMat2f", "cpp_type": "math::mat2f"},
    "math::mat3f": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 9, "assert": "assertMat3f", "cpp_type": "math::mat3f"},
    "math::mat4f": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 16, "assert": "assertMat4f", "cpp_type": "math::mat4f"},

    # mat2, mat3, mat4 (no suffix) map to double[] unless specified otherwise
    "math::mat2": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertMat2d", "cpp_type": "math::mat2"},
    "math::mat3": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 9, "assert": "assertMat3d", "cpp_type": "math::mat3"},
    "math::mat4": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 16, "assert": "assertMat4d", "cpp_type": "math::mat4"},

    # Canonical FQNs (Qualified Names)
    # Vectors (Float)
    "filament::math::details::TVec2<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 2, "assert": "assertFloat2", "cpp_type": "math::float2"},
    "filament::math::details::TVec3<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 3, "assert": "assertFloat3", "cpp_type": "math::float3"},
    "filament::math::details::TVec4<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertFloat4", "cpp_type": "math::float4"},
    # Vectors (Double)
    "filament::math::details::TVec2<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 2, "assert": "assertDouble2", "cpp_type": "math::double2"},
    "filament::math::details::TVec3<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 3, "assert": "assertDouble3", "cpp_type": "math::double3"},
    "filament::math::details::TVec4<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertDouble4", "cpp_type": "math::double4"},
    # Matrices (Float)
    "filament::math::details::TMat22<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertMat2f", "cpp_type": "math::mat2f"},
    "filament::math::details::TMat33<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 9, "assert": "assertMat3f", "cpp_type": "math::mat3f"},
    "filament::math::details::TMat44<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 16, "assert": "assertMat4f", "cpp_type": "math::mat4f"},
    # Matrices (Double)
    "filament::math::details::TMat22<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertMat2d", "cpp_type": "math::mat2"},
    "filament::math::details::TMat33<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 9, "assert": "assertMat3d", "cpp_type": "math::mat3"},
    "filament::math::details::TMat44<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 16, "assert": "assertMat4d", "cpp_type": "math::mat4"},

    # Quaternions
    "math::quatf": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertFloat4", "cpp_type": "math::quatf"},
    "math::quat": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertDouble4", "cpp_type": "math::quat"},
    "filament::math::details::TQuaternion<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertFloat4", "cpp_type": "math::quatf"},
    "filament::math::details::TQuaternion<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertDouble4", "cpp_type": "math::quat"},
    "math::details::TQuaternion<float>": {"java": "float[]", "jni": "jfloatArray", "jni_type": "jfloatArray", "scalar": "float", "size": 4, "assert": "assertFloat4", "cpp_type": "math::quatf"},
    "math::details::TQuaternion<double>": {"java": "double[]", "jni": "jdoubleArray", "jni_type": "jdoubleArray", "scalar": "double", "size": 4, "assert": "assertDouble4", "cpp_type": "math::quat"},

    # Math Vectors (short, ushort, int, uint, byte, ubyte)
    "math::short2": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 2, "assert": "assertShort2", "cpp_type": "math::short2"},
    "math::short3": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 3, "assert": "assertShort3", "cpp_type": "math::short3"},
    "math::short4": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 4, "assert": "assertShort4", "cpp_type": "math::short4"},
    "math::ushort2": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::ushort2"},
    "math::ushort3": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::ushort3"},
    "math::ushort4": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::ushort4"},
    "math::int2": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::int2"},
    "math::int3": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::int3"},
    "math::int4": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::int4"},
    "math::uint2": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::uint2"},
    "math::uint3": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::uint3"},
    "math::uint4": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::uint4"},
    "math::byte2": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 2, "assert": "assertByte2", "cpp_type": "math::byte2"},
    "math::byte3": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 3, "assert": "assertByte3", "cpp_type": "math::byte3"},
    "math::byte4": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 4, "assert": "assertByte4", "cpp_type": "math::byte4"},
    "math::ubyte2": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 2, "assert": "assertByte2", "cpp_type": "math::ubyte2"},
    "math::ubyte3": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 3, "assert": "assertByte3", "cpp_type": "math::ubyte3"},
    "math::ubyte4": {"java": "byte[]", "jni": "jbyteArray", "jni_type": "jbyteArray", "scalar": "byte", "size": 4, "assert": "assertByte4", "cpp_type": "math::ubyte4"},

    # Qualified versions of Math Vectors
    "filament::math::details::TVec2<short>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 2, "assert": "assertShort2", "cpp_type": "math::short2"},
    "filament::math::details::TVec3<short>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 3, "assert": "assertShort3", "cpp_type": "math::short3"},
    "filament::math::details::TVec4<short>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 4, "assert": "assertShort4", "cpp_type": "math::short4"},
    "filament::math::details::TVec2<int16_t>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 2, "assert": "assertShort2", "cpp_type": "math::short2"},
    "filament::math::details::TVec3<int16_t>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 3, "assert": "assertShort3", "cpp_type": "math::short3"},
    "filament::math::details::TVec4<int16_t>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 4, "assert": "assertShort4", "cpp_type": "math::short4"},
    "math::details::TVec4<int16_t>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 4, "assert": "assertShort4", "cpp_type": "math::short4"},
    "details::TVec4<int16_t>": {"java": "short[]", "jni": "jshortArray", "jni_type": "jshortArray", "scalar": "short", "size": 4, "assert": "assertShort4", "cpp_type": "math::short4"},
    "filament::math::details::TVec2<int>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::int2"},
    "filament::math::details::TVec3<int>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::int3"},
    "filament::math::details::TVec4<int>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::int4"},
    "filament::math::details::TVec2<int32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::int2"},
    "filament::math::details::TVec3<int32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::int3"},
    "filament::math::details::TVec4<int32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::int4"},
    "filament::math::details::TVec2<uint32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 2, "assert": "assertInt2", "cpp_type": "math::uint2"},
    "filament::math::details::TVec3<uint32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 3, "assert": "assertInt3", "cpp_type": "math::uint3"},
    "filament::math::details::TVec4<uint32_t>": {"java": "int[]", "jni": "jintArray", "jni_type": "jintArray", "scalar": "int", "size": 4, "assert": "assertInt4", "cpp_type": "math::uint4"},
}

# Value Types Configuration (Value Objects, Geometry, and Inline Backing Buffers)
VALUE_TYPES: Dict[str, Dict[str, Any]] = {
    "filament::Frustum": {
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
    },
    "Frustum": {
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
    },
    "filament::backend::PixelBufferDescriptor": {
        "java": "PixelBufferDescriptor",
        "is_value_object": True,
    },
    "backend::PixelBufferDescriptor": {
        "java": "PixelBufferDescriptor",
        "is_value_object": True,
    },
    "PixelBufferDescriptor": {
        "java": "PixelBufferDescriptor",
        "is_value_object": True,
    },
    "Texture::PixelBufferDescriptor": {
        "java": "PixelBufferDescriptor",
        "is_value_object": True,
    },
}

# Tagged scalar families for dual-mode buffer marshalling
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

# Specific enum entry overrides to avoid keyword collisions or adhere to API conventions
ENUM_ENTRY_RENAMES: Dict[str, Dict[str, str]] = {
    "StencilOperation": {
        "INCR": "INCR_CLAMP",
        "DECR": "DECR_CLAMP",
    }
}

# Regex patterns for entities, durations, bitsets, and string-like types
TYPE_PATTERNS: List[Tuple[str, Dict[str, Any]]] = [
    (r"^(?:filament::)?(?:utils::)?Entity$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@Entity",
        "to_cpp": "Entity::import({value})",
        "from_cpp": "(jint)Entity::smuggle({value})"
    }),
    (r"^(?:filament::)?(?:utils::)?EntityInstance<(.*)>$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@EntityInstance",
        "to_cpp": "EntityInstance<{0}>({value})",
        "from_cpp": "(jint){value}.asValue()"
    }),
    (r"^(?:const\s+)?(?:std::)?basic_string_view<char>(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view"
    }),
    (r"^(?:const\s+)?(?:std::)?string_view(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string_view"
    }),
    (r"^(?:const\s+)?(?:std::)?string(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "std::string"
    }),
    (r"^(?:const\s+)?(?:utils::)?CString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::CString"
    }),
    (r"^(?:const\s+)?(?:utils::)?StaticString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::StaticString"
    }),
    (r"^(?:const\s+)?(?:utils::)?ImmutableCString(?:\s*&)?$", {
        "java": "String", "jni": "jstring", "jni_type": "jstring", "is_string": True, "cpp_type": "utils::ImmutableCString"
    }),
    (r"^(?:const\s+)?(?:filament::)?(?:backend::)?BufferDescriptor(?:\s*&&|\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_buffer_descriptor": True, "cpp_type": "backend::BufferDescriptor"
    }),
    (r"^(?:const\s+)?(?:filament::)?(?:BufferObject::|IndexBuffer::|VertexBuffer::)?BufferDescriptor(?:\s*&&|\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_buffer_descriptor": True, "cpp_type": "backend::BufferDescriptor"
    }),
    (r"^(?:const\s+)?(?:utils::)?FixedCapacityVector<(?:(?:filament::)?math::float3|(?:filament::)?math::details::TVec3<float>|details::TVec3<float>|float3)>(?:\s*&)?$", {
        "java": "Buffer", "jni": "jobject", "jni_type": "jobject", "is_custom_lut_buffer": True, "cpp_type": "utils::FixedCapacityVector<math::float3>"
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::duration<.*>(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::nanoseconds({value})",
        "from_cpp": "(jlong){value}.count()"
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::time_point<.*>(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::steady_clock::time_point(std::chrono::nanoseconds({value}))",
        "from_cpp": "(jlong){value}.time_since_epoch().count()"
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::nanoseconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::nanoseconds({value})",
        "from_cpp": "(jlong){value}.count()"
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::milliseconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::milliseconds({value})",
        "from_cpp": "(jlong){value}.count()"
    }),
    (r"^(?:const\s+)?(?:std::)?chrono::seconds(?:\s*&)?$", {
        "java": "long", "jni": "jlong", "jni_type": "jlong", "annotation": "@IntRange(from = 0)",
        "to_cpp": "std::chrono::seconds({value})",
        "from_cpp": "(jlong){value}.count()"
    }),
    (r"^(?:const\s+)?(?:utils::)?tribool(?:\s*&)?$", {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "utils::tribool((bool){value})",
        "from_cpp": "(jboolean)(({value}).is_true())"
    }),
    (r"^(?:const\s+)?(?:std::)?optional<bool>(?:\s*&)?$", {
        "java": "boolean", "jni": "jboolean", "jni_type": "jboolean",
        "to_cpp": "std::make_optional((bool){value})",
        "from_cpp": "(jboolean)({value}.value_or(false))"
    }),
    (r"^(?:const\s+)?utils::bitset<.*>(?:\s*&)?$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)",
        "to_cpp": "{type}({value})",
        "from_cpp": "(jint){value}.getValue()"
    }),
    (r"^(?:const\s+)?(?:std::)?bitset<.*>(?:\s*&)?$", {
        "java": "int", "jni": "jint", "jni_type": "jint", "annotation": "@IntRange(from = 0)",
        "to_cpp": "{type}({value})",
        "from_cpp": "(jint){value}.to_ulong()"
    })
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
    "transient", "try", "void", "volatile", "while", "true", "false", "null"
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
        if "qualified_name" in cls:
            qname = cls["qualified_name"]
            KNOWN_CLASSES[qname] = cls

            # Step 2a: Strip top-level engine namespace prefixes for unqualified fallback
            clean = (
                qname
                .replace("filament::", "")
                .replace("backend::", "")
                .replace("utils::", "")
                .replace("math::", "")
            )
            KNOWN_CLASSES[clean] = cls

            # Step 2b: Handle filament:: vs backend:: / filament::backend:: re-exports
            if "filament::" in qname:
                backend_clean = qname.replace("filament::", "backend::")
                KNOWN_CLASSES[backend_clean] = cls
                filament_backend_clean = qname.replace(
                    "filament::", "filament::backend::"
                )
                KNOWN_CLASSES[filament_backend_clean] = cls


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
