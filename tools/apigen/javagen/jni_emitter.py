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

"""Zero-cost JNI C++ bridge code emission engine for Filament.

This module provides :class:`JniEmitter`, which synthesizes high-performance C++ JNI bridge
files (`.cpp`) corresponding to generated Java wrapper classes. It handles native pointer
unwrapping, stack struct reconstruction for AAPCS64 register passing, direct buffer pinning
and unpinning, slice forwarding, asynchronous callback dispatch, and overloaded JNI symbol
name mangling.

Architectural Position and Bridge Model:
----------------------------------------
1. **Zero-Cost Abstraction Principle**:
   - Bridge functions are declared `extern "C" JNIEXPORT ... JNICALL` with internal linkage
     wrappers.
   - Wherever possible, native method calls avoid heap allocation by passing Plain-Old-Data (POD)
     structs exploded as scalar primitives directly in CPU registers (s0-s7 for floating-point,
     x0-x7 for integer/pointer values under the AAPCS64 standard).
2. **Exception Safety & wrapJni Invariants**:
   - All C++ API calls that can potentially throw or encounter errors are wrapped inside
     `filament::android::wrapJni(env, [=]() { ... })`.
   - `wrapJni` catches C++ standard exceptions and rethrows them as appropriate Java exceptions
     (e.g. `IllegalArgumentException`, `IllegalStateException`), preventing process aborts.
   - Methods declared `noexcept` bypass `wrapJni` for optimal performance.
3. **Direct Buffer & Memory Management**:
   - `AutoBuffer`: Manages direct `java.nio.Buffer` instances, obtaining native pointers via
     `env->GetDirectBufferAddress` and enforcing capacity/stride safety limits.
   - Primitive Arrays: Pinned via `env->Get<Type>ArrayElements` and safely unpinned in `body_post`
     via `env->Release<Type>ArrayElements` using `JNI_ABORT` for read-only buffers to avoid
     unnecessary cache-flush writes.
   - Builder Buffers: In Builders where buffers must remain valid until `build()` completes,
     buffers are captured inside a heap-allocated `BuilderWrapper` maintaining a `retainedBuffers`
     vector of `AutoBuffer` instances.
4. **Asynchronous Callbacks & Invocables**:
   - Functional interfaces (`utils::Invocable<void(args)>`) are bound to Java callback objects.
   - `JniCallback` and `JniBufferCallback` manage global JNI references and thread-safe dispatch
     back to the Android main thread handler via `postToJavaAndDestroy`.
5. **JNI Method Name Mangling**:
   - Implements Oracle JNI specification naming rules:
     `Java_<escaped_pkg>_<class>_<nativeMethod>`
   - When multiple overloads share the same native base name in Java, appends double-underscore
     signature mangling (`__<mangled_types>`) to ensure unique symbol link resolution.
"""

import collections
import copy
import os
import re
import sys
from typing import Any, Dict, List, Optional, Set, Tuple, Union

from .config import (
    GENERATED_FILE_WARNING,
    KNOWN_CLASSES,
    LICENSE_HEADER,
    MATH_TYPES,
    PACKAGE_MAP,
    TAGGED_SCALAR_FAMILIES,
    TYPE_MAP,
    TYPE_PATTERNS,
    VALUE_TYPES,
)
from .utils import (
    classify_scalar_family,
    format_enum_entry_name,
    get_effective_method_name,
    get_element_entry_name,
    get_jni_scalar_type,
    get_scoped_cpp_name,
    get_size_param_attr,
    get_tagged_array_info,
    indent_lines,
    is_custom_enum,
    is_tagged_array_method,
    sanitize_identifier,
    to_camel_case,
)


class JniEmitter:
    """Emits zero-cost JNI C++ bridge implementations (.cpp) for a given ClassContext.

    Coordinates include header discovery, method generation, builder wrapper synthesis,
    memory pinning, AAPCS64 exploded struct marshalling, and overloaded symbol mangling.

    Attributes:
        ctx: The parent :class:`~javagen.context.ClassContext` representing the active class.
    """

    def __init__(self, context: Any) -> None:
        """Initialize JniEmitter bound to an active ClassContext.

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
        """Whether this class is a 64-bit primitive bitfield wrapper."""
        return self.ctx.is_bitfield_class

    @property
    def is_utility_class(self) -> bool:
        """Whether this class is a static-only utility namespace."""
        return self.ctx.is_utility_class

    @property
    def is_builder_class(self) -> bool:
        """Whether this class implements the Builder pattern."""
        return self.ctx.is_builder_class

    @property
    def value_type_info(self) -> Any:
        """Configuration dictionary for value types or bitfields."""
        return self.ctx.value_type_info

    @property
    def parent_context(self) -> Any:
        """Enclosing ClassContext if this is an inner/nested class."""
        return self.ctx.parent_context

    @property
    def builder(self) -> Any:
        """Child ClassContext for nested Builder if present."""
        return self.ctx.builder

    @property
    def package(self) -> str:
        """Java package name."""
        return self.ctx.package

    @property
    def jni_package(self) -> str:
        """Escaped package name suitable for JNI function names."""
        return self.ctx.jni_package

    @property
    def base_class(self) -> Optional[str]:
        """Primary base class name if single inheritance applies."""
        return self.ctx.base_class

    @property
    def cached_fields(self) -> Dict[str, Any]:
        """Dictionary of Java-side cached getter/setter field pairs."""
        return self.ctx.cached_fields

    @property
    def emitted_signatures(self) -> Set[Any]:
        """Set of emitted method signatures for deduplication."""
        return self.ctx.emitted_signatures

    @property
    def emitted_native_signatures(self) -> Set[Any]:
        """Set of emitted native method signatures."""
        return self.ctx.emitted_native_signatures

    @property
    def emitted_jni_funcs(self) -> Set[str]:
        """Set of emitted C++ JNI export function symbol names."""
        return self.ctx.emitted_jni_funcs

    @property
    def native_counts(self) -> Dict[str, int]:
        """Dictionary mapping native method base names to overload counts."""
        return self.ctx.native_counts

    @property
    def is_used_by_native(self) -> bool:
        """Whether the class carries the used_by_native attribute."""
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

    @property
    def has_retained_buffers(self) -> bool:
        """Whether this builder retains native buffer memory across invocations."""
        return self.ctx.has_retained_buffers

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

    def _get_clean_type_name(self, *a: Any, **kw: Any) -> str:
        """Forward clean type name formatting to parent context."""
        return self.ctx._get_clean_type_name(*a, **kw)

    def _get_method_sig(self, *a: Any, **kw: Any) -> Tuple[str, Tuple[str, ...]]:
        """Forward method signature deduplication helper to parent context."""
        return self.ctx._get_method_sig(*a, **kw)

    def translate_default_value(self, *a: Any, **kw: Any) -> Optional[str]:
        """Forward default value translation to parent context."""
        return self.ctx.translate_default_value(*a, **kw)

    def format_constant_value(self, *a: Any, **kw: Any) -> str:
        """Forward constant value formatting to parent context."""
        return self.ctx.format_constant_value(*a, **kw)

    def get_jni_mangled_type(self, *a: Any, **kw: Any) -> str:
        """Forward JNI signature type mangling to parent context."""
        return self.ctx.get_jni_mangled_type(*a, **kw)

    def _has_annotations(self) -> bool:
        """Forward annotation presence check to parent context."""
        return self.ctx._has_annotations()

    def _expand_method(self, *a: Any, **kw: Any) -> List[Dict[str, Any]]:
        """Forward method expansion and SFINAE unrolling to parent context."""
        return self.ctx._expand_method(*a, **kw)

    def generate_cpp_struct_construction(self, *a: Any, **kw: Any) -> List[str]:
        """Forward C++ struct reconstruction generation to parent context."""
        return self.ctx.generate_cpp_struct_construction(*a, **kw)

    def generate_cpp_struct_cleanup(self, *a: Any, **kw: Any) -> List[str]:
        """Forward C++ struct cleanup generation to parent context."""
        return self.ctx.generate_cpp_struct_cleanup(*a, **kw)

    # -------------------------------------------------------------------------
    # Builder JNI Method Emission
    # -------------------------------------------------------------------------

    def generate_builder_jni_methods(self) -> str:
        """Emit JNI C++ bridge implementations for the nested Builder class.

        Generates:
        1. **Wrapper Struct (`<Class>BuilderWrapper`)**: If the builder accepts retained direct
           buffers, wraps the C++ builder alongside a `std::vector<std::unique_ptr<AutoBuffer>>`
           to ensure native buffer allocations remain pinned until `build()` completes.
        2. **Builder Allocation (`nCreateBuilder`)**: Instantiates `new Builder()` on the native
           heap and returns the raw pointer address as a `jlong`.
        3. **Builder Destruction (`nDestroyBuilder`)**: Invokes `delete` on the native builder
           or wrapper instance when discarded.
        4. **Build Execution (`nBuilderBuild`)**: Dispatches the final `build(*engine)` call,
           wrapping it in `wrapJni` to convert native C++ errors into Java exceptions.
        5. **Fluid Setter Dispatchers (`nBuilder<Setter>`)**: Unpacks JNI arguments and forwards
           them to builder methods, unrolling vector arguments or pinning array elements.

        Returns:
            C++ source string containing all JNI functions for the builder.
        """
        if self.parent_context and self.parent_context.name == "Engine":
            return ""

        out = []
        parent_name = self.parent_context.name
        parent_cpp = self.parent_context.cpp_name

        wrapper_name = f"{parent_name}BuilderWrapper"
        if self.has_retained_buffers:
            out.append(f"struct {wrapper_name} {{")
            out.append(f"    {parent_cpp}::Builder builder;")
            out.append("    std::vector<std::unique_ptr<AutoBuffer>> retainedBuffers;")
            out.append("};")
            out.append("")

        # Step 1: Emit Constructor bridges (nCreateBuilder)
        has_ctor = False
        for m in self.methods:
            if m.get("is_constructor"):
                has_ctor = True
                args = m.get("arguments", [])
                params = ["JNIEnv *env", "jclass clazz"]
                param_jni_types = []
                cpp_call_args = []
                for arg in args:
                    arg_name = sanitize_identifier(arg["name"])
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    if t_info.get("is_enum"):
                        params.append(f"jint {arg_name}")
                        param_jni_types.append("jint")
                        cpp_enum_type = self._strip_namespaces(t_info.get("cpp_enum_type") or t_info.get("cpp_type") or arg["type"]["cpp_name"])
                        cpp_call_args.append(f"({cpp_enum_type}){arg_name}")
                    else:
                        jni_t = self.get_jni_type(arg["type"])
                        params.append(f"{jni_t} {arg_name}")
                        param_jni_types.append(jni_t)
                        cpp_call_args.append(arg_name)
                if self.native_counts.get("nCreateBuilder", 0) > 1:
                    mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                    jni_func_name = f"Java_{self.jni_package}_{parent_name}_nCreateBuilder__{mangled_sig}"
                else:
                    jni_func_name = f"Java_{self.jni_package}_{parent_name}_nCreateBuilder"
                out.append(f"extern \"C\" JNIEXPORT jlong JNICALL")
                out.append(f"{jni_func_name}({', '.join(params)}) {{")
                if self.has_retained_buffers:
                    call_expr = f"{parent_cpp}::Builder({', '.join(cpp_call_args)})" if cpp_call_args else f"{parent_cpp}::Builder{{}}"
                    out.append(f"    return (jlong) new {wrapper_name}{{{call_expr}}};")
                else:
                    call_expr = f"{parent_cpp}::Builder({', '.join(cpp_call_args)})" if cpp_call_args else f"{parent_cpp}::Builder{{}}"
                    out.append(f"    return (jlong) new {call_expr};")
                out.append("}")
                out.append("")
        if not has_ctor:
            out.append(f"extern \"C\" JNIEXPORT jlong JNICALL")
            out.append(f"Java_{self.jni_package}_{parent_name}_nCreateBuilder(JNIEnv *env, jclass clazz) {{")
            if self.has_retained_buffers:
                out.append(f"    return (jlong) new {wrapper_name}();")
            else:
                out.append(f"    return (jlong) new {parent_cpp}::Builder{{}};")
            out.append("}")
            out.append("")

        # Step 2: Emit Builder Destructor bridge (nDestroyBuilder)
        out.append(f"extern \"C\" JNIEXPORT void JNICALL")
        out.append(f"Java_{self.jni_package}_{parent_name}_nDestroyBuilder(JNIEnv *env, jclass clazz, jlong nativeBuilder) {{")
        if self.has_retained_buffers:
            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
            out.append("    for (auto& buf : wrapper->retainedBuffers) {")
            out.append("        buf->attachToJniThread(env);")
            out.append("    }")
            out.append("    delete wrapper;")
        else:
            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
            out.append("    delete builder;")
        out.append("}")
        out.append("")

        # Step 3: Emit Builder Member Methods and Setter bridges
        for method in self.methods:
            if method.get("is_constructor"):
                continue
            for exp_m in self._expand_method(method):
                m_ir = exp_m["ir"]
                m_name = m_ir["name"]
                is_noexcept = m_ir.get("is_noexcept", False)
                args = m_ir.get("arguments", [])
                
                # Handle Builder.build(...) methods
                if m_name == "build":
                    if len(args) == 1 and ("Engine" in args[0]["type"].get("cpp_name", "") or "Engine" in args[0]["type"].get("qualified_name", "")):
                        param_jni_types = ["jlong", "jlong"]
                        if self.native_counts.get("nBuilderBuild", 0) > 1:
                            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild__{mangled_sig}"
                        else:
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild"
                        out.append(f"extern \"C\" JNIEXPORT jlong JNICALL")
                        out.append(f"{jni_func_name}(JNIEnv *env, jclass clazz, jlong nativeBuilder, jlong nativeEngine) {{")
                        if self.has_retained_buffers:
                            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                            out.append(f"    Engine* engine = (Engine*) nativeEngine;")
                            out.append("    jlong result = filament::android::wrapJni<jlong>(env, [=]() {")
                            out.append(f"        return (jlong) wrapper->builder.build(*engine);")
                            out.append("    });")
                            out.append("    wrapper->retainedBuffers.clear();")
                            out.append("    return result;")
                        else:
                            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                            out.append(f"    Engine* engine = (Engine*) nativeEngine;")
                            out.append("    return filament::android::wrapJni<jlong>(env, [=]() {")
                            out.append(f"        return (jlong) builder->build(*engine);")
                            out.append("    });")
                        out.append("}")
                        out.append("")
                    elif len(args) == 2:
                        param_jni_types = ["jlong", "jlong", "jint"]
                        if self.native_counts.get("nBuilderBuild", 0) > 1:
                            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild__{mangled_sig}"
                        else:
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild"
                        out.append(f"extern \"C\" JNIEXPORT jboolean JNICALL")
                        out.append(f"{jni_func_name}(JNIEnv *env, jclass clazz, jlong nativeBuilder, jlong nativeEngine, jint entity) {{")
                        if self.has_retained_buffers:
                            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                            out.append(f"    Engine* engine = (Engine*) nativeEngine;")
                            out.append("    jboolean result = filament::android::wrapJni<jboolean>(env, [=]() {")
                            out.append(f"        return (jboolean) (wrapper->builder.build(*engine, utils::Entity::import(entity)) == {parent_cpp}::Builder::Result::Success);")
                            out.append("    });")
                            out.append("    wrapper->retainedBuffers.clear();")
                            out.append("    return result;")
                        else:
                            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                            out.append(f"    Engine* engine = (Engine*) nativeEngine;")
                            out.append("    return filament::android::wrapJni<jboolean>(env, [=]() {")
                            out.append(f"        return (jboolean) (builder->build(*engine, utils::Entity::import(entity)) == {parent_cpp}::Builder::Result::Success);")
                            out.append("    });")
                        out.append("}")
                        out.append("")
                    elif len(args) == 0:
                        param_jni_types = ["jlong"]
                        if self.native_counts.get("nBuilderBuild", 0) > 1:
                            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild__{mangled_sig}"
                        else:
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_nBuilderBuild"
                        out.append(f"extern \"C\" JNIEXPORT jlong JNICALL")
                        out.append(f"{jni_func_name}(JNIEnv *env, jclass clazz, jlong nativeBuilder) {{")
                        if self.has_retained_buffers:
                            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                            out.append("    jlong result = filament::android::wrapJni<jlong>(env, [=]() {")
                            out.append(f"        return (jlong) wrapper->builder.build();")
                            out.append("    });")
                            out.append("    wrapper->retainedBuffers.clear();")
                            out.append("    return result;")
                        else:
                            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                            out.append("    return filament::android::wrapJni<jlong>(env, [=]() {")
                            out.append(f"        return (jlong) builder->build();")
                            out.append("    });")
                        out.append("}")
                        out.append("")
                    continue

                eff_name = get_effective_method_name(m_ir)
                native_name = f"nBuilder{eff_name[0].upper() + eff_name[1:]}"
                
                # Handle Packed Buffer Builder Methods (Dual Buffer & Array Emission)
                if self.is_packed_buffer_method(m_ir):
                    info = self.get_packed_buffer_info(m_ir)
                    mode = exp_m.get("buffer_mode", "buffer")
                    stride = info["stride"]
                    buf_name = info["buf_name"]
                    cnt_name = info["cnt_name"]
                    cpp_type = info["cpp_type"]
                    cpp_method_name = m_ir["name"]
                    array_offset_name = info["array_offset_name"]

                    if mode == "buffer":
                        params = ["JNIEnv *env", "jclass clazz", "jlong nativeBuilder", f"jint {cnt_name}", f"jobject {buf_name}_", "jint remaining"]
                        param_jni_types = ["jlong", "jint", "Buffer", "jint"]
                        if self.native_counts.get(native_name, 0) > 1:
                            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}__{mangled_sig}"
                        else:
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}"
                        out.append('extern "C" JNIEXPORT jint JNICALL')
                        out.append(f"{jni_func_name}({', '.join(params)}) {{")
                        if self.has_retained_buffers:
                            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                            out.append(f"    auto* builder = &wrapper->builder;")
                        else:
                            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                        out.append(f"    AutoBuffer nioBuffer(env, {buf_name}_, {cnt_name} * {stride});")
                        out.append(f"    void* {buf_name} = nioBuffer.getData();")
                        out.append("    size_t sizeInBytes = nioBuffer.getSize();")
                        out.append("    if (sizeInBytes > (remaining << nioBuffer.getShift())) {")
                        out.append("        return -1;")
                        out.append("    }")
                        if info.get("is_slice"):
                            cpp_call_args = [f"utils::Slice<const {cpp_type}>(reinterpret_cast<const {cpp_type} *>({buf_name}), (size_t){cnt_name})"]
                        else:
                            cpp_call_args = []
                            for a in m_ir.get("arguments", []):
                                if a["name"] == info["buf_arg"]["name"]:
                                    cpp_call_args.append(f"reinterpret_cast<const {cpp_type} *>({buf_name})")
                                elif a["name"] == info["cnt_arg"]["name"]:
                                    cpp_call_args.append(f"(size_t){cnt_name}")
                        out.append(f"    builder->{cpp_method_name}({', '.join(cpp_call_args)});")
                        out.append("    return 0;")
                        out.append("}")
                        out.append("")
                    else: # mode == "array"
                        scalar = info.get("scalar", "float")
                        cap_scalar = scalar.capitalize()
                        jni_type = info.get("jni_type", f"j{scalar}Array")
                        jni_scalar = f"j{scalar}"
                        params = ["JNIEnv *env", "jclass clazz", "jlong nativeBuilder", f"{jni_type} {buf_name}_", f"jint {cnt_name}", f"jint {array_offset_name}"]
                        param_jni_types = ["jlong", jni_type, "jint", "jint"]
                        if self.native_counts.get(native_name, 0) > 1:
                            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}__{mangled_sig}"
                        else:
                            jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}"
                        out.append('extern "C" JNIEXPORT void JNICALL')
                        out.append(f"{jni_func_name}({', '.join(params)}) {{")
                        if self.has_retained_buffers:
                            out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                            out.append(f"    auto* builder = &wrapper->builder;")
                        else:
                            out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                        out.append(f"    {jni_scalar} const * const {buf_name} = env->Get{cap_scalar}ArrayElements({buf_name}_, nullptr);")
                        if info.get("is_slice"):
                            cpp_call_args = [f"utils::Slice<const {cpp_type}>(reinterpret_cast<const {cpp_type} *>({buf_name}) + {array_offset_name}, (size_t){cnt_name})"]
                        else:
                            cpp_call_args = []
                            for a in m_ir.get("arguments", []):
                                if a["name"] == info["buf_arg"]["name"]:
                                    cpp_call_args.append(f"reinterpret_cast<const {cpp_type} *>({buf_name}) + {array_offset_name}")
                                elif a["name"] == info["cnt_arg"]["name"]:
                                    cpp_call_args.append(f"(size_t){cnt_name}")
                        out.append(f"    builder->{cpp_method_name}({', '.join(cpp_call_args)});")
                        out.append(f"    env->Release{cap_scalar}ArrayElements({buf_name}_, const_cast<{jni_scalar} *>({buf_name}), JNI_ABORT);")
                        out.append("}")
                        out.append("")
                    continue

                # Standard Fluid Setter emission
                params = ["JNIEnv *env", "jclass clazz", "jlong nativeBuilder"]
                param_jni_types = ["jlong"]
                cpp_call_args = []
                body_pre = []
                body_post = []

                for arg in args:
                    arg_name = arg["name"]
                    cpp_type = arg["type"]
                    if self.is_builder_buffer_arg(arg):
                        params.append(f"jobject {arg_name}")
                        param_jni_types.append("jobject")
                        arg_type_str = cpp_type.get("qualified_name") or cpp_type.get("cpp_name", "") if isinstance(cpp_type, dict) else str(cpp_type)
                        is_slice = "Slice" in arg_type_str
                        if is_slice:
                            size_param_name = "size"
                            params.append(f"jint {size_param_name}")
                            param_jni_types.append("jint")
                        else:
                            size_param_name = get_size_param_attr(arg)
                        body_pre.append(f"wrapper->retainedBuffers.push_back(std::make_unique<AutoBuffer>(env, {arg_name}, {size_param_name}));")
                        if is_slice:
                            inner_match = re.search(r"Slice<\s*(.*?)\s*>", arg_type_str)
                            inner_t = inner_match.group(1).strip() if inner_match else "const uint8_t"
                            cpp_call_args.append(f"utils::Slice<{inner_t}>(static_cast<{inner_t}*>(wrapper->retainedBuffers.back()->getData()), (size_t){size_param_name})")
                        else:
                            cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else str(cpp_type)
                            if cpp_type_str.strip() in ("const void*", "void*"):
                                cpp_call_args.append("wrapper->retainedBuffers.back()->getData()")
                            else:
                                cpp_call_args.append(f"static_cast<{cpp_type_str}>(wrapper->retainedBuffers.back()->getData())")
                        continue
                    arr_in = self.get_array_input_info(cpp_type)
                    if arr_in:
                        cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else cpp_type
                        is_matrix = re.search(r"::mat\d", cpp_type_str) or "::TMat" in cpp_type_str
                        is_pointer = cpp_type.get("is_pointer", False)
                        size_param_name = get_size_param_attr(arg)
                        
                        if not is_matrix and not is_pointer and not size_param_name and not arr_in.get("is_slice"):
                            scalar_jni = get_jni_scalar_type(arr_in["scalar"])
                            sz = arr_in["size"]
                            components = ["x", "y", "z", "w"][:sz]
                            comp_args = []
                            for comp in components:
                                comp_arg_name = sanitize_identifier(f"{arg_name}{comp}")
                                params.append(f"{scalar_jni} {comp_arg_name}")
                                param_jni_types.append(scalar_jni)
                                comp_args.append(comp_arg_name)
                            type_cstr = self._strip_namespaces(arr_in["cpp_type"])
                            cpp_call_args.append(f"{type_cstr}{{{', '.join(comp_args)}}}")
                        else:
                            jni_arr_type = arr_in["jni"]
                            jni_arg_name = f"{arg_name}_"
                            params.append(f"{jni_arr_type} {jni_arg_name}")
                            param_jni_types.append(jni_arr_type)
                            scalar = arr_in["scalar"]
                            elem_accessor = "GetFloatArrayElements" if scalar == "float" else ("GetDoubleArrayElements" if scalar == "double" else "GetIntArrayElements")
                            elem_releaser = "ReleaseFloatArrayElements" if scalar == "float" else ("ReleaseDoubleArrayElements" if scalar == "double" else "ReleaseIntArrayElements")
                            scalar_jni_type = "jfloat" if scalar == "float" else ("jdouble" if scalar == "double" else "jint")
                            
                            is_const = cpp_type.get("is_const", False)
                            body_pre.append(f"{scalar_jni_type} *{arg_name} = env->{elem_accessor}({jni_arg_name}, nullptr);")
                            if is_const:
                                body_post.append(f"env->{elem_releaser}({jni_arg_name}, {arg_name}, JNI_ABORT);")
                            else:
                                body_post.append(f"env->{elem_releaser}({jni_arg_name}, {arg_name}, 0);")
                            
                            type_cstr = self._strip_namespaces(arr_in["cpp_type"])
                            if is_matrix and not is_pointer:
                                cpp_call_args.append(f"*reinterpret_cast<const {type_cstr} *>({arg_name})")
                            else:
                                ptr_cast = f"const {type_cstr} *" if is_const else f"{type_cstr} *"
                                cpp_call_args.append(f"reinterpret_cast<{ptr_cast}>({arg_name})")
                    else:
                        t_info, _ = self.resolve_type_info(cpp_type, silent=True)
                        if t_info.get("is_struct"):
                            struct_cls = t_info["struct_cls"]
                            leaves = self.get_flattened_leaves(struct_cls)
                            for leaf in leaves:
                                params.append(f"{leaf['jni_type']} {leaf['param_name']}")
                                param_jni_types.append(leaf['jni_type'])
                            var_name = f"{arg_name}Cpp"
                            body_pre.extend(self.generate_cpp_struct_construction(struct_cls, var_name))
                            body_post.extend(self.generate_cpp_struct_cleanup(struct_cls))
                            cpp_call_args.append(var_name)
                        elif t_info.get("is_string"):
                            jni_arg_name = f"{arg_name}_"
                            params.append(f"jstring {jni_arg_name}")
                            param_jni_types.append("jstring")
                            nullability = cpp_type.get("nullability", "unspecified") if isinstance(cpp_type, dict) else "unspecified"
                            if nullability == "nullable":
                                body_pre.append(f"char const * const {arg_name} = {jni_arg_name} ? env->GetStringUTFChars({jni_arg_name}, nullptr) : nullptr;")
                                body_post.append(f"if ({jni_arg_name}) {{")
                                body_post.append(f"    env->ReleaseStringUTFChars({jni_arg_name}, {arg_name});")
                                body_post.append(f"}}")
                            else:
                                body_pre.append(f"char const * const {arg_name} = env->GetStringUTFChars({jni_arg_name}, nullptr);")
                                body_post.append(f"env->ReleaseStringUTFChars({jni_arg_name}, {arg_name});")
                            
                            cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else str(cpp_type)
                            clean_type = self._strip_namespaces(cpp_type_str).replace("&", "").replace("const", "").strip()
                            if "string_view" in clean_type or "basic_string_view" in clean_type:
                                cpp_call_args.append(f"std::string_view({arg_name})")
                            elif clean_type == "CString":
                                cpp_call_args.append(f"utils::CString({arg_name})")
                            elif clean_type == "StaticString":
                                cpp_call_args.append(f"utils::StaticString({arg_name})")
                            elif clean_type == "ImmutableCString":
                                cpp_call_args.append(f"utils::ImmutableCString({arg_name})")
                            elif clean_type == "string":
                                cpp_call_args.append(f"std::string({arg_name})")
                            else:
                                cpp_call_args.append(arg_name)
                        elif t_info.get("is_enum"):
                            params.append(f"jint {arg_name}")
                            param_jni_types.append("jint")
                            cpp_enum_type = self._strip_namespaces(t_info.get("cpp_enum_type") or t_info.get("cpp_type") or cpp_type["cpp_name"])
                            cpp_call_args.append(f"({cpp_enum_type}){arg_name}")
                        elif t_info.get("is_filament_type") or t_info.get("is_handle"):
                            params.append(f"jlong {arg_name}")
                            param_jni_types.append("jlong")
                            raw_cpp_type = self._strip_namespaces(t_info.get("cpp_type") or cpp_type["cpp_name"]).replace("*", "").replace("&", "").replace("const", "").strip()
                            is_ref = cpp_type.get("is_reference", False) if isinstance(cpp_type, dict) else False
                            is_const = cpp_type.get("is_const", False) if isinstance(cpp_type, dict) else False
                            const_prefix = "const " if is_const else ""
                            if is_ref:
                                cpp_call_args.append(f"*({const_prefix}{raw_cpp_type}*){arg_name}")
                            else:
                                cpp_call_args.append(f"({const_prefix}{raw_cpp_type}*){arg_name}")
                        elif t_info.get("is_value_object"):
                            params.append(f"jlong {arg_name}")
                            param_jni_types.append("jlong")
                            cpp_call_args.append(f"filament::JniUtils::from_long({arg_name})")
                        elif t_info.get("is_custom_lut_buffer"):
                            params.append(f"jobject {arg_name}")
                            param_jni_types.append("jobject")
                            dim_arg = next((a["name"] for a in args if a["name"] != arg_name and self.resolve_type_info(a["type"], silent=True)[0].get("java") in ("int", "byte", "short", "long")), "dimension")
                            lut_var = f"{arg_name}Lut"
                            body_pre.append(f"if ({dim_arg} == 0) {{")
                            body_pre.append(f"    return;")
                            body_pre.append(f"}}")
                            body_pre.append(f"math::float3* {arg_name}Data = (math::float3*) env->GetDirectBufferAddress({arg_name});")
                            body_pre.append(f"size_t {arg_name}Count = size_t({dim_arg}) * {dim_arg} * {dim_arg};")
                            body_pre.append(f"utils::FixedCapacityVector<math::float3> {lut_var} = utils::FixedCapacityVector<math::float3>::with_capacity({arg_name}Count);")
                            body_pre.append(f"for (size_t i = 0; i < {arg_name}Count; ++i) {{")
                            body_pre.append(f"    {lut_var}.push_back({arg_name}Data[i]);")
                            body_pre.append(f"}}")
                            cpp_call_args.append(f"std::move({lut_var})")
                        else:
                            jni_t = self.get_jni_type(cpp_type)
                            params.append(f"{jni_t} {arg_name}")
                            param_jni_types.append(jni_t)
                            expr = self.get_to_cpp_expr(cpp_type, arg_name)
                            cpp_call_args.append(expr)

                if self.native_counts.get(native_name, 0) > 1:
                    mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                    jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}__{mangled_sig}"
                else:
                    jni_func_name = f"Java_{self.jni_package}_{parent_name}_{native_name}"

                out.append(f"extern \"C\" JNIEXPORT void JNICALL")
                out.append(f"{jni_func_name}({', '.join(params)}) {{")
                if self.has_retained_buffers:
                    out.append(f"    {wrapper_name}* wrapper = ({wrapper_name}*) nativeBuilder;")
                    out.append(f"    {parent_cpp}::Builder* builder = &wrapper->builder;")
                else:
                    out.append(f"    {parent_cpp}::Builder* builder = ({parent_cpp}::Builder*) nativeBuilder;")
                for pre in body_pre:
                    out.append(f"    {pre}")
                orig_name = m_ir.get("orig_name", m_name)
                spec_type = exp_m.get("specialization_type")
                target_name = f"{orig_name}<{spec_type}>" if spec_type else orig_name
                call_expr = f"builder->{target_name}({', '.join(cpp_call_args)});"
                if is_noexcept:
                    out.append(f"    {call_expr}")
                else:
                    out.append("    wrapJni(env, [=]() {")
                    out.append(f"        {call_expr}")
                    out.append("    });")
                for post in body_post:
                    out.append(f"    {post}")
                out.append("}")
                out.append("")
        return "\n".join(out)

    # -------------------------------------------------------------------------
    # Main File Generation Entry Point
    # -------------------------------------------------------------------------

    def generate(self) -> str:
        """Generate complete zero-cost JNI C++ bridge implementation file content.

        Walkthrough of Generation Steps:
        --------------------------------
        1. **License & Warning Headers**: Formats Apache 2.0 license and generated warning.
        2. **Standard Includes**: Emits `<jni.h>`, `<common/JniUtils.h>`, and the declaring header.
        3. **Referenced Header Discovery**:
           - Scans all method signatures (and nested Builder signatures) for types referencing
             external headers (`string_view`, `math/mat4.h`, `backend/BufferDescriptor.h`,
             `utils/Entity.h`, `ToneMapper.h`, etc.).
           - Sorts headers deterministically.
        4. **Namespace Declarations**: Emits `using namespace filament;`, `using namespace filament::android;`,
           and conditionally `using namespace filament::math;` if math types are present.
        5. **Member Method Bridges**: Expands compressed SFINAE trait methods and generates individual
           JNI export functions (`_generate_jni_method`).
        6. **Builder Bridges**: Emits builder JNI methods if a nested Builder exists.

        Returns:
            Complete C++ JNI bridge file as a string.
        """
        # Step 1: Emit Standard License and Warning Headers
        out = []
        out.append(LICENSE_HEADER)
        out.append("")
        out.append(GENERATED_FILE_WARNING)
        out.append("")
        out.append("#include <jni.h>")
        out.append("#include <common/JniUtils.h>")
        out.append(f"#include <{self.source_header}>")
        
        # Step 2: Discover and Collect All Referenced Headers
        included_headers = set()
        if self.archetype == "bitfield":
            included_headers.add("utils/algorithm.h")
        all_methods_for_headers = list(self.methods)
        if self.builder:
            all_methods_for_headers.extend(self.builder.methods)
        for m in all_methods_for_headers:
            all_types = [m.get("return_type", {})] + [a["type"] for a in m.get("arguments", [])]
            for t in all_types:
                if not t:
                    continue
                t_str = t.get("qualified_name") or t.get("cpp_name", "") if isinstance(t, dict) else str(t)
                clean_t = self._strip_namespaces(t_str).replace("&", "").replace("*", "").replace("const", "").strip()
                scls = self.is_aggregate_struct(clean_t)
                if scls:
                    hdr = scls.get("location", {}).get("file")
                    if hdr and hdr != self.source_header:
                        included_headers.add(hdr)
                if "string_view" in t_str or "basic_string_view" in t_str:
                    included_headers.add("string_view")
                if "std::string" in t_str:
                    included_headers.add("string")
                if "chrono" in t_str:
                    included_headers.add("chrono")
                if "optional" in t_str:
                    included_headers.add("optional")
                if "Slice" in t_str:
                    included_headers.add("utils/Slice.h")
                if "CString" in t_str:
                    included_headers.add("utils/CString.h")
                if "StaticString" in t_str:
                    included_headers.add("utils/StaticString.h")
                if "ImmutableCString" in t_str:
                    included_headers.add("utils/ImmutableCString.h")
                if "tribool" in t_str:
                    included_headers.add("utils/tribool.h")
                if "bitset" in t_str:
                    included_headers.add("utils/bitset.h")
                if "Engine" in t_str and "Engine.h" not in self.source_header:
                    included_headers.add("filament/Engine.h")
                if "EntityInstance" in t_str:
                    included_headers.add("utils/EntityInstance.h")
                elif "Entity" in t_str:
                    included_headers.add("utils/Entity.h")
                for vt_name, vt_data in VALUE_TYPES.items():
                    if vt_data.get("header") and (vt_name in t_str) and vt_data["header"] not in self.source_header:
                        included_headers.add(vt_data["header"])
                if "Camera" in t_str and "Camera.h" not in self.source_header:
                    included_headers.add("filament/Camera.h")
                if "mat4" in t_str or "mat3" in t_str or "mat2" in t_str or "TMat" in t_str:
                    included_headers.add("math/mat4.h")
                if "BufferDescriptor" in t_str and "PixelBufferDescriptor" not in t_str:
                    included_headers.add("backend/BufferDescriptor.h")
                    included_headers.add("common/CallbackUtils.h")
                    included_headers.add("common/NioUtils.h")
                if "PixelBufferDescriptor" in t_str:
                    included_headers.add("backend/PixelBufferDescriptor.h")
                    included_headers.add("common/CallbackUtils.h")
                    included_headers.add("common/NioUtils.h")
                if "float4" in t_str or "float3" in t_str or "float2" in t_str or "TVec" in t_str:
                    included_headers.add("math/vec4.h")
                if "FixedCapacityVector" in t_str:
                    included_headers.add("utils/FixedCapacityVector.h")
                if "ToneMapper" in t_str and "ToneMapper.h" not in self.source_header:
                    included_headers.add("filament/ToneMapper.h")
                if "Viewport" in t_str and "Viewport.h" not in self.source_header:
                    included_headers.add("filament/Viewport.h")
                if re.search(r'\b(filament::)?TextureSampler\b', t_str) and "TextureSampler.h" not in self.source_header:
                    included_headers.add("filament/TextureSampler.h")
        if any(self.is_async_callback_method(m) for m in all_methods_for_headers):
            included_headers.add("common/CallbackUtils.h")
        if any(self.is_packed_buffer_method(m) for m in all_methods_for_headers):
            included_headers.add("common/NioUtils.h")
            for m in all_methods_for_headers:
                info = self.get_packed_buffer_info(m)
                if info and "Bone" in info.get("cpp_type", "") and "RenderableManager.h" not in self.source_header:
                    included_headers.add("filament/RenderableManager.h")
        if self.has_retained_buffers:
            included_headers.add("common/NioUtils.h")
            included_headers.add("memory")
            included_headers.add("vector")
        if any(any(self.resolve_type_info(a["type"], silent=True)[0].get("java") in ("Buffer", "ByteBuffer") for a in m.get("arguments", [])) for m in all_methods_for_headers):
            included_headers.add("common/NioUtils.h")
            included_headers.add("memory")
        if any(any("vector" in (a["type"].get("qualified_name") or a["type"].get("cpp_name", "")) for a in m.get("arguments", [])) for m in all_methods_for_headers) or any("vector" in (m.get("return_type", {}).get("qualified_name") or m.get("return_type", {}).get("cpp_name", "")) for m in all_methods_for_headers):
            included_headers.add("vector")
        if self.name == "Material" and any(m["name"] == "getParameters" for m in self.methods):
            included_headers.add("vector")
        for m in all_methods_for_headers:
            tagged_info = get_tagged_array_info(m)
            if tagged_info:
                tp_name = tagged_info["tagged_arg"].get("type", {}).get("template_param_name", "T")
                for spec in m.get("specializations", []):
                    conc = spec.get(tp_name, "")
                    if "mat" in conc:
                        included_headers.add("math/mat4.h")
                    if any(v in conc for v in ("float2", "float3", "float4", "int2", "int3", "int4", "bool2", "bool3", "bool4")):
                        included_headers.add("math/vec4.h")

        if self.name == "View":
            included_headers.add("filament/Options.h")
            included_headers.add("filament/Color.h")
            included_headers.add("filament/Viewport.h")
            included_headers.add("common/CallbackUtils.h")
            included_headers.add("private/backend/VirtualMachineEnv.h")

        if self.name == "Engine":
            included_headers.update([
                "filament/BufferObject.h",
                "filament/Camera.h",
                "filament/ColorGrading.h",
                "filament/Fence.h",
                "filament/FramePacer.h",
                "filament/IndexBuffer.h",
                "filament/IndirectLight.h",
                "filament/InstanceBuffer.h",
                "filament/LightManager.h",
                "filament/Material.h",
                "filament/MaterialInstance.h",
                "filament/MorphTargetBuffer.h",
                "filament/RenderableManager.h",
                "filament/RenderTarget.h",
                "filament/Renderer.h",
                "filament/Scene.h",
                "filament/SkinningBuffer.h",
                "filament/Skybox.h",
                "filament/Stream.h",
                "filament/SwapChain.h",
                "filament/Texture.h",
                "filament/TransformManager.h",
                "filament/VertexBuffer.h",
                "filament/View.h",
                "utils/Entity.h",
                "utils/EntityManager.h",
                "utils/tribool.h",
                "common/CallbackUtils.h",
            ])

        # Step 3: Format Includes and Namespaces
        has_math = any(h.startswith("math/") for h in included_headers) or self.source_header.startswith("math/")
        for hdr in sorted(included_headers):
            if hdr.startswith("common/"):
                out.append(f'#include "{hdr}"')
            else:
                out.append(f"#include <{hdr}>")
        out.append("")
        out.append("using namespace filament;")
        out.append("using namespace filament::android;")
        if has_math:
            out.append("using namespace filament::math;")
        out.append("using namespace utils;")
        out.append("")
        
        # Step 4: Handle Special Custom Functions (e.g. ToneMapper destructor, View.pick)
        if self.name == "ToneMapper":
            out.append(f'extern "C" JNIEXPORT void JNICALL')
            out.append(f'Java_{self.jni_package}_ToneMapper_nDestroyToneMapper(JNIEnv *env, jclass clazz, jlong toneMapper_) {{')
            out.append(f'    ToneMapper* toneMapper = (ToneMapper*) toneMapper_;')
            out.append(f'    delete toneMapper;')
            out.append(f'}}')
            out.append('')

        if self.name == "View":
            jni_func_name = f"Java_{self.jni_package}_{self.name}_nPick"
            if jni_func_name not in self.emitted_jni_funcs:
                self.emitted_jni_funcs.add(jni_func_name)
                out.append(f'''extern "C" JNIEXPORT void JNICALL
{jni_func_name}(JNIEnv* env, jclass clazz,
        jlong nativeView,
        jint x, jint y, jobject handler, jobject internalCallback) {{

    static const struct JniState {{
        jclass internalOnPickCallbackClass;
        jfieldID renderableFieldId;
        jfieldID depthFieldId;
        jfieldID fragCoordXFieldId;
        jfieldID fragCoordYFieldId;
        jfieldID fragCoordZFieldId;
        explicit JniState(JNIEnv* env) noexcept {{
            internalOnPickCallbackClass = env->FindClass("com/google/android/filament/View$InternalOnPickCallback");
            renderableFieldId = env->GetFieldID(internalOnPickCallbackClass, "mRenderable", "I");
            depthFieldId = env->GetFieldID(internalOnPickCallbackClass, "mDepth", "F");
            fragCoordXFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsX", "F");
            fragCoordYFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsY", "F");
            fragCoordZFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsZ", "F");
        }}
    }} jniState(env);

    View* view = (View*) nativeView;
    JniCallback *callback = JniCallback::make(env, handler, internalCallback);
    view->pick(x, y, [callback](View::PickingQueryResult const& result) {{
        jobject obj = callback->getCallbackObject();
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        env->SetIntField(obj, jniState.renderableFieldId, (jint)result.renderable.getId());
        env->SetFloatField(obj, jniState.depthFieldId, result.depth);
        env->SetFloatField(obj, jniState.fragCoordXFieldId, result.fragCoords.x);
        env->SetFloatField(obj, jniState.fragCoordYFieldId, result.fragCoords.y);
        env->SetFloatField(obj, jniState.fragCoordZFieldId, result.fragCoords.z);
        JniCallback::postToJavaAndDestroy(callback);
    }}, callback->getHandler());
}}''')
                out.append('')

        # Step 5: Emit JNI Function Implementations for all member methods
        for method in self.methods:
            self.current_method = method["name"]
            generated_methods = self._expand_method(method)
            for m in generated_methods:
                jni_func = self._generate_jni_method(m)
                if jni_func:
                    out.append(jni_func)
                    out.append("")

        # Step 6: Emit Nested Builder JNI Functions
        if self.builder:
            builder_jni = self.builder.generate_builder_jni_methods()
            if builder_jni:
                out.append(builder_jni)
                out.append("")

        if self.name == "Engine":
            out.append(self._get_engine_jni_methods())
            out.append("")
        
        out.append(GENERATED_FILE_WARNING)
        return "\n".join(out) + "\n"

    # -------------------------------------------------------------------------
    # Individual JNI Method Implementation Emission
    # -------------------------------------------------------------------------

    def _generate_jni_method(self, m_ctx: Dict[str, Any]) -> Optional[str]:
        """Generate C++ JNI bridge function implementation for a single method variant.

        Handles:
        1. **Special APIs**: Handcrafted bridges for `Material::getParameters` reflection queries.
        2. **Asynchronous Callbacks**: Converts Android handler/runnable pairs into native callbacks.
        3. **BufferDescriptor**: Handles asynchronous buffer release callbacks (`JniBufferCallback`).
        4. **PixelBufferDescriptor**: Emits dual compressed / uncompressed pixel descriptor dispatch.
        5. **Tagged Arrays**: Emits enum-based switch-case scalar family dispatch.
        6. **Packed Buffers**: Emits dual direct Buffer and primitive array overloads.
        7. **AAPCS64 Struct Flattening**: Reconstructs C++ structs from scalar leaves and generates cleanup.
        8. **Standard Native Calls**: Unwraps native handles, converts strings, and calls native methods
           inside `wrapJni`.

        Args:
            m_ctx: Method context dictionary from `_expand_method`.

        Returns:
            C++ JNI function implementation string, or None if method should be skipped.
        """
        method = m_ctx["ir"]
        name = method["name"]
        eff_name = get_effective_method_name(method)

        # Step 1: Discard cached fields, duplicate engine getters, and unbindable constructors
        if name in [c["getter"] for c in self.cached_fields.values()]:
            info = next(c for c in self.cached_fields.values() if c["getter"] == name)
            if not info.get("is_handle_reference"):
                return None
        if name in self.retained_references:
            return None
        if self.is_value_class and name == self.value_type_info.get("buffer_getter"):
            return None
        if method.get("is_constructor") and self.archetype != "bitfield" and not self.base_class:
            return None
        if self.name == "Engine":
            intercepted_engine_methods = (
                "destroy",
                "createSwapChain",
            )
            if name in intercepted_engine_methods or eff_name in intercepted_engine_methods:
                return None
        ret_info, _ = self.resolve_type_info(method["return_type"], silent=True)
        if ret_info.get("is_pojo_struct"):
            return None

        # Step 2: Handle Material.getParameters Reflection API Bridge
        if self.name == "Material" and eff_name == "getParameters":
            native_name = "nGetParameters"
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"
            if jni_func_name in self.emitted_jni_funcs:
                return None
            self.emitted_jni_funcs.add(jni_func_name)
            out = []
            out.append(f'extern "C" JNIEXPORT jint JNICALL')
            out.append(f'{jni_func_name}(JNIEnv *env, jclass clazz, jlong nativeMaterial, jobjectArray parameters, jint count) {{')
            out.append(f'    Material const * const that = (Material const *) nativeMaterial;')
            out.append(f'    if (!parameters || count <= 0) {{')
            out.append(f'        return 0;')
            out.append(f'    }}')
            out.append(f'    std::vector<Material::ParameterInfo> info(count);')
            out.append(f'    size_t const received = that->getParameters(info.data(), (size_t)count);')
            out.append(f'')
            out.append(f'    jclass const infoClass = env->FindClass("com/google/android/filament/Material$ParameterInfo");')
            out.append(f'    jmethodID const infoCtor = env->GetMethodID(infoClass, "<init>", "()V");')
            out.append(f'    jfieldID const nameField = env->GetFieldID(infoClass, "name", "Ljava/lang/String;");')
            out.append(f'    jfieldID const isSamplerField = env->GetFieldID(infoClass, "isSampler", "Z");')
            out.append(f'    jfieldID const isSubpassField = env->GetFieldID(infoClass, "isSubpass", "Z");')
            out.append(f'    jfieldID const typeField = env->GetFieldID(infoClass, "type", "Lcom/google/android/filament/Material$ParameterType;");')
            out.append(f'    jfieldID const samplerTypeField = env->GetFieldID(infoClass, "samplerType", "Lcom/google/android/filament/Material$SamplerType;");')
            out.append(f'    jfieldID const subpassTypeField = env->GetFieldID(infoClass, "subpassType", "Lcom/google/android/filament/Material$SubpassType;");')
            out.append(f'    jfieldID const countField = env->GetFieldID(infoClass, "count", "I");')
            out.append(f'    jfieldID const precisionField = env->GetFieldID(infoClass, "precision", "Lcom/google/android/filament/Material$Precision;");')
            out.append(f'')
            out.append(f'    jclass const pTypeClass = env->FindClass("com/google/android/filament/Material$ParameterType");')
            out.append(f'    jmethodID const pTypeValues = env->GetStaticMethodID(pTypeClass, "values", "()[Lcom/google/android/filament/Material$ParameterType;");')
            out.append(f'    jobjectArray const pTypeArray = (jobjectArray)env->CallStaticObjectMethod(pTypeClass, pTypeValues);')
            out.append(f'')
            out.append(f'    jclass const sTypeClass = env->FindClass("com/google/android/filament/Material$SamplerType");')
            out.append(f'    jmethodID const sTypeValues = env->GetStaticMethodID(sTypeClass, "values", "()[Lcom/google/android/filament/Material$SamplerType;");')
            out.append(f'    jobjectArray const sTypeArray = (jobjectArray)env->CallStaticObjectMethod(sTypeClass, sTypeValues);')
            out.append(f'')
            out.append(f'    jclass const spTypeClass = env->FindClass("com/google/android/filament/Material$SubpassType");')
            out.append(f'    jmethodID const spTypeValues = env->GetStaticMethodID(spTypeClass, "values", "()[Lcom/google/android/filament/Material$SubpassType;");')
            out.append(f'    jobjectArray const spTypeArray = (jobjectArray)env->CallStaticObjectMethod(spTypeClass, spTypeValues);')
            out.append(f'')
            out.append(f'    jclass const precClass = env->FindClass("com/google/android/filament/Material$Precision");')
            out.append(f'    jmethodID const precValues = env->GetStaticMethodID(precClass, "values", "()[Lcom/google/android/filament/Material$Precision;");')
            out.append(f'    jobjectArray const precArray = (jobjectArray)env->CallStaticObjectMethod(precClass, precValues);')
            out.append(f'')
            out.append(f'    for (size_t i = 0; i < received; ++i) {{')
            out.append(f'        jobject elem = env->GetObjectArrayElement(parameters, (jsize)i);')
            out.append(f'        if (!elem) {{')
            out.append(f'            elem = env->NewObject(infoClass, infoCtor);')
            out.append(f'            env->SetObjectArrayElement(parameters, (jsize)i, elem);')
            out.append(f'        }}')
            out.append(f'        if (elem) {{')
            out.append(f'            jstring const jname = env->NewStringUTF(info[i].name);')
            out.append(f'            env->SetObjectField(elem, nameField, jname);')
            out.append(f'            env->DeleteLocalRef(jname);')
            out.append(f'')
            out.append(f'            env->SetBooleanField(elem, isSamplerField, (jboolean)info[i].isSampler);')
            out.append(f'            env->SetBooleanField(elem, isSubpassField, (jboolean)info[i].isSubpass);')
            out.append(f'            env->SetIntField(elem, countField, (jint)info[i].count);')
            out.append(f'')
            out.append(f'            if (info[i].isSampler) {{')
            out.append(f'                jobject const sTypeObj = env->GetObjectArrayElement(sTypeArray, (jsize)info[i].samplerType);')
            out.append(f'                env->SetObjectField(elem, samplerTypeField, sTypeObj);')
            out.append(f'                env->DeleteLocalRef(sTypeObj);')
            out.append(f'                env->SetObjectField(elem, typeField, nullptr);')
            out.append(f'                env->SetObjectField(elem, subpassTypeField, nullptr);')
            out.append(f'            }} else if (info[i].isSubpass) {{')
            out.append(f'                jobject const spTypeObj = env->GetObjectArrayElement(spTypeArray, (jsize)info[i].subpassType);')
            out.append(f'                env->SetObjectField(elem, subpassTypeField, spTypeObj);')
            out.append(f'                env->DeleteLocalRef(spTypeObj);')
            out.append(f'                env->SetObjectField(elem, typeField, nullptr);')
            out.append(f'                env->SetObjectField(elem, samplerTypeField, nullptr);')
            out.append(f'            }} else {{')
            out.append(f'                jobject const pTypeObj = env->GetObjectArrayElement(pTypeArray, (jsize)info[i].type);')
            out.append(f'                env->SetObjectField(elem, typeField, pTypeObj);')
            out.append(f'                env->DeleteLocalRef(pTypeObj);')
            out.append(f'                env->SetObjectField(elem, samplerTypeField, nullptr);')
            out.append(f'                env->SetObjectField(elem, subpassTypeField, nullptr);')
            out.append(f'            }}')
            out.append(f'')
            out.append(f'            jobject const precObj = env->GetObjectArrayElement(precArray, (jsize)info[i].precision);')
            out.append(f'            env->SetObjectField(elem, precisionField, precObj);')
            out.append(f'            env->DeleteLocalRef(precObj);')
            out.append(f'')
            out.append(f'            env->DeleteLocalRef(elem);')
            out.append(f'        }}')
            out.append(f'    }}')
            out.append(f'    env->DeleteLocalRef(precArray);')
            out.append(f'    env->DeleteLocalRef(spTypeArray);')
            out.append(f'    env->DeleteLocalRef(sTypeArray);')
            out.append(f'    env->DeleteLocalRef(pTypeArray);')
            out.append(f'')
            out.append(f'    return (jint)received;')
            out.append(f'}}')
            return "\n".join(out)

        # Step 3: Handle Asynchronous Callback Methods (JniCallback)
        if m_ctx.get("is_async_callback"):
            name = get_effective_method_name(method)
            native_name = f"n{name[0].upper() + name[1:]}"
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"
            if jni_func_name in self.emitted_jni_funcs:
                return None
            self.emitted_jni_funcs.add(jni_func_name)

            is_static = method.get("is_static", False)
            params = ["JNIEnv *env", "jclass clazz"]
            if not is_static:
                params.append(f"jlong native{self.name}")

            handler_name = "handler"
            callback_name = "callback"

            # Pass 1: Assemble JNI parameter list and identify handler/callback names
            for arg in method.get("arguments", []):
                arg_name = sanitize_identifier(arg["name"])
                t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "")
                if "CallbackHandler" in t_str:
                    handler_name = arg_name
                    params.append(f"jobject {arg_name}")
                elif any(cb in t_str for cb in ("Invocable", "std::function", "Callback")):
                    callback_name = arg_name
                    params.append(f"jobject {arg_name}")
                else:
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    if t_info.get("is_enum"):
                        params.append(f"jint {arg_name}")
                    elif t_info.get("is_filament_type"):
                        params.append(f"jlong {arg_name}")
                    else:
                        jni_t = self.get_jni_type(arg["type"])
                        params.append(f"{jni_t} {arg_name}")

            # Pass 2: Assemble C++ call arguments in exact positional sequence
            cpp_call_args = []
            for arg in method.get("arguments", []):
                arg_name = sanitize_identifier(arg["name"])
                t_str = arg["type"].get("qualified_name") or arg["type"].get("cpp_name", "")
                if "CallbackHandler" in t_str:
                    cpp_call_args.append("jniCallback->getHandler()")
                elif any(cb in t_str for cb in ("Invocable", "std::function", "Callback")):
                    cpp_call_args.append("[jniCallback](auto&&...) { JniCallback::postToJavaAndDestroy(jniCallback); }")
                else:
                    t_info, _ = self.resolve_type_info(arg["type"], silent=True)
                    if t_info.get("is_enum"):
                        cpp_enum = t_info.get("cpp_enum_type") or self._strip_namespaces(arg["type"].get("qualified_name") or arg["type"].get("cpp_name", ""))
                        cpp_call_args.append(f"({self._strip_namespaces(cpp_enum)}){arg_name}")
                    elif t_info.get("is_filament_type"):
                        cpp_class = t_info.get("cpp_class") or t_info.get("java")
                        cpp_call_args.append(f"({self._strip_namespaces(cpp_class)}*){arg_name}")
                    else:
                        cpp_type_name = arg["type"].get("cpp_name") or self._strip_namespaces(arg["type"].get("qualified_name", ""))
                        cpp_call_args.append(f"({cpp_type_name}){arg_name}")

            out = []
            out.append(f'extern "C" JNIEXPORT void JNICALL')
            out.append(f'{jni_func_name}({", ".join(params)}) {{')
            if not is_static:
                out.append(f'    {self.name}* that = ({self.name}*) native{self.name};')
            out.append(f'    JniCallback* jniCallback = JniCallback::make(env, {handler_name}, {callback_name});')

            target_expr = f'that->{method["name"]}' if not is_static else f'{self.name}::{method["name"]}'
            call_stmt = f'{target_expr}({", ".join(cpp_call_args)});'

            if method.get("is_noexcept", False):
                out.append(f'    {call_stmt}')
            else:
                out.append('    filament::android::wrapJni<void>(env, [=]() {')
                out.append(f'        {call_stmt}')
                out.append('    });')
            out.append('}')
            return "\n".join(out)

        # Step 4: Handle BufferDescriptor Methods (Asynchronous Buffer Release)
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

            params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}", "jlong nativeEngine"]
            pre_call_args = []
            for a in pre_args:
                a_t = a["type"]
                a_info, _ = self.resolve_type_info(a_t, silent=True)
                jni_t = self.get_jni_type(a_t)
                a_name = sanitize_identifier(a["name"])
                params.append(f"{jni_t} {a_name}")
                if a_info.get("is_enum"):
                    pre_call_args.append(f"({self._strip_namespaces(a_info['cpp_enum_type'])}){a_name}")
                else:
                    expr = self.get_to_cpp_expr(a_t, a_name)
                    pre_call_args.append(expr)

            params.extend([
                "jobject buffer",
                "jint remaining",
                "jint destOffsetInBytes",
                "jint count",
                "jobject handler",
                "jobject runnable"
            ])

            native_name = f"n{eff_name[0].upper() + eff_name[1:]}"
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"

            if jni_func_name in self.emitted_jni_funcs:
                return None
            self.emitted_jni_funcs.add(jni_func_name)

            out = []
            out.append(f"extern \"C\" JNIEXPORT jint JNICALL")
            out.append(f"{jni_func_name}({', '.join(params)}) {{")
            out.append(f"    {self.name} *that = ({self.name} *) native{self.name};")
            out.append(f"    Engine *engine = (Engine *) nativeEngine;")
            out.append("")
            out.append("    return filament::android::wrapJni<jint>(env, [=]() {")
            out.append("        AutoBuffer nioBuffer(env, buffer, count);")
            out.append("        void* data = nioBuffer.getData();")
            out.append("        size_t sizeInBytes = nioBuffer.getSize();")
            out.append("        if (sizeInBytes > (remaining << nioBuffer.getShift())) {")
            out.append("            // BufferOverflowException")
            out.append("            return -1;")
            out.append("        }")
            out.append("")
            out.append("        auto* callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));")
            out.append("")
            out.append("        backend::BufferDescriptor desc(data, sizeInBytes,")
            out.append("                callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);")
            out.append("")
            pre_args_str = ", ".join(pre_call_args) + ", " if pre_call_args else ""
            out.append(f"        that->{name}(*engine, {pre_args_str}std::move(desc), (uint32_t) destOffsetInBytes);")
            out.append("")
            out.append("        return 0;")
            out.append("    });")
            out.append("}")
            return "\n".join(out)

        # Step 5: Handle PixelBufferDescriptor Methods (Compressed / Uncompressed)
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

            params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"]
            param_jni_types = ["jlong"]
            body_pre = []
            cpp_call_args = []

            for a in pre_args:
                a_t = a["type"]
                a_info, _ = self.resolve_type_info(a_t, silent=True)
                safe_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    param_name = f"native{safe_name[0].upper()}{safe_name[1:]}"
                    params.append(f"jlong {param_name}")
                    param_jni_types.append("jlong")
                    cpp_class = a_info["cpp_class"]
                    is_const_arg = a_t.get("is_const", False) if isinstance(a_t, dict) else False
                    is_ref = a_t.get("is_reference", False) if isinstance(a_t, dict) else False
                    cpp_type_str = a_t.get("qualified_name") or a_t.get("cpp_name", "") if isinstance(a_t, dict) else str(a_t)
                    if "&" in cpp_type_str:
                        is_ref = True
                    const_str = " const" if is_const_arg else ""
                    body_pre.append(f"{cpp_class}{const_str}* const {safe_name} = ({cpp_class}{const_str}*) {param_name};")
                    if is_ref:
                        cpp_call_args.append(f"*{safe_name}")
                    else:
                        cpp_call_args.append(safe_name)
                elif a_info.get("is_enum"):
                    params.append(f"jint {safe_name}")
                    param_jni_types.append("jint")
                    cpp_call_args.append(f"({self._strip_namespaces(a_info['cpp_enum_type'])}){safe_name}")
                else:
                    jni_t = self.get_jni_type(a_t)
                    params.append(f"{jni_t} {safe_name}")
                    param_jni_types.append(jni_t)
                    expr = self.get_to_cpp_expr(a_t, safe_name)
                    cpp_call_args.append(expr)

            params.extend([
                "jobject storage",
                "jint remaining",
                "jint left",
                "jint top",
                "jint type",
                "jint alignment",
                "jint stride",
                "jint format",
                "jobject handler",
                "jobject callback",
            ])
            param_jni_types.extend([
                "Buffer",
                "jint",
                "jint",
                "jint",
                "jint",
                "jint",
                "jint",
                "jint",
                "jobject",
                "Runnable",
            ])

            native_name = f"n{eff_name[0].upper() + eff_name[1:]}"
            if self.native_counts.get(native_name, 0) > 1:
                mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}__{mangled_sig}"
            else:
                jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"

            if jni_func_name in self.emitted_jni_funcs:
                return None
            self.emitted_jni_funcs.add(jni_func_name)

            out = []
            out.append(f"extern \"C\" JNIEXPORT void JNICALL")
            out.append(f"{jni_func_name}({', '.join(params)}) {{")
            out.append(f"    {self.name} *that = ({self.name} *) native{self.name};")
            for line in body_pre:
                out.append(f"    {line}")
            out.append("")
            out.append("    AutoBuffer nioBuffer(env, storage, remaining);")
            out.append("    void* buffer = nioBuffer.getData();")
            out.append("    size_t sizeInBytes = nioBuffer.getSize();")
            out.append("    auto* bufferCallback = JniBufferCallback::make(nullptr, env, handler, callback, std::move(nioBuffer));")
            out.append("")
            out.append("    backend::PixelBufferDescriptor desc = (type == (jint) backend::PixelDataType::COMPRESSED) ?")
            out.append("            backend::PixelBufferDescriptor(buffer, sizeInBytes,")
            out.append("                    (backend::CompressedPixelDataType) format, (uint32_t) stride,")
            out.append("                    bufferCallback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, bufferCallback) :")
            out.append("            backend::PixelBufferDescriptor(buffer, sizeInBytes,")
            out.append("                    (backend::PixelDataFormat) format, (backend::PixelDataType) type,")
            out.append("                    (uint8_t) alignment, (uint32_t) left, (uint32_t) top, (uint32_t) stride,")
            out.append("                    bufferCallback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, bufferCallback);")
            out.append("")
            cpp_call_args_str = ", ".join(cpp_call_args) + ", " if cpp_call_args else ""
            cpp_method_name = method["name"]
            out.append("    filament::android::wrapJni(env, [&]() {")
            out.append(f"        that->{cpp_method_name}({cpp_call_args_str}std::move(desc));")
            out.append("    });")
            out.append("}")
            return "\n".join(out)

        # Step 6: Handle Tagged Array Buffer Methods (Scalar Family Enum Switch)
        if m_ctx.get("is_tagged_array"):
            method = m_ctx["ir"]
            family = m_ctx["family"]
            tagged_info = m_ctx["tagged_info"]
            specs = m_ctx["specializations"]
            family_cfg = TAGGED_SCALAR_FAMILIES[family]
            
            name = method["name"]
            eff_name = get_effective_method_name(method)
            cap_family = family_cfg["family"]
            native_name = f"n{eff_name[0].upper() + eff_name[1:]}{cap_family}Array"
            
            pre_jni_params = []
            pre_param_jni_types = []
            pre_body_lines = []
            post_body_lines = []
            cpp_pre_call_args = []
            
            for a in tagged_info["pre_args"]:
                a_t = a["type"]
                a_info, _ = self.resolve_type_info(a_t, silent=True)
                safe_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    param_name = f"native{safe_name[0].upper()}{safe_name[1:]}"
                    pre_jni_params.append(f"jlong {param_name}")
                    pre_param_jni_types.append("jlong")
                    cpp_class = a_info["cpp_class"]
                    pre_body_lines.append(f"{cpp_class}* const {safe_name} = ({cpp_class}*) {param_name};")
                    if a_t.get("is_reference"):
                        cpp_pre_call_args.append(f"*{safe_name}")
                    else:
                        cpp_pre_call_args.append(safe_name)
                elif a_info.get("is_string"):
                    jni_arg_name = f"{safe_name}_"
                    pre_jni_params.append(f"jstring {jni_arg_name}")
                    pre_param_jni_types.append("jstring")
                    pre_body_lines.append(f"const char* const {safe_name} = env->GetStringUTFChars({jni_arg_name}, nullptr);")
                    post_body_lines.append(f"env->ReleaseStringUTFChars({jni_arg_name}, {safe_name});")
                    cpp_pre_call_args.append(safe_name)
                elif a_info.get("is_enum"):
                    pre_jni_params.append(f"jint {safe_name}")
                    pre_param_jni_types.append("jint")
                    cpp_pre_call_args.append(f"({self._strip_namespaces(a_info['cpp_enum_type'])}){safe_name}")
                else:
                    jni_t = self.get_jni_type(a_t)
                    pre_jni_params.append(f"{jni_t} {safe_name}")
                    pre_param_jni_types.append(jni_t)
                    cpp_pre_call_args.append(self.get_to_cpp_expr(a_t, safe_name))
            
            values_name = sanitize_identifier(tagged_info["tagged_arg"]["name"])
            count_name = sanitize_identifier(tagged_info["cnt_arg"]["name"])
            jni_arr = family_cfg["jni_array"]
            scalar_ptr = family_cfg["jni_scalar_ptr"]
            get_elems = family_cfg["get_elements"]
            rel_elems = family_cfg["release_elements"]
            
            params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"] + pre_jni_params + [
                "jint element",
                f"{jni_arr} {values_name}_",
                "jint offset",
                f"jint {count_name}"
            ]
            
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"
            if jni_func_name in self.emitted_jni_funcs:
                return None
            self.emitted_jni_funcs.add(jni_func_name)
            
            out = []
            out.append(f'extern "C" JNIEXPORT void JNICALL')
            out.append(f"{jni_func_name}({', '.join(params)}) {{")
            out.append(f"    {self.cpp_name}* const that = ({self.cpp_name}*) native{self.name};")
            for line in pre_body_lines:
                out.append(f"    {line}")
            out.append(f"    {scalar_ptr} values = env->{get_elems}({values_name}_, nullptr);")
            out.append("")
            
            enum_name = family_cfg["enum_name"]
            enum_ir = self.enum_map.get(enum_name)
            enum_entries = [e["name"] for e in enum_ir.get("entries", [])] if enum_ir else []
            
            tp_name = tagged_info["tagged_arg"].get("type", {}).get("template_param_name", "T")
            cpp_pre_str = ", ".join(cpp_pre_call_args) + ", " if cpp_pre_call_args else ""
            
            out.append("    wrapJni(env, [=]() {")
            out.append("        switch (element) {")
            
            seen_cases = set()
            for spec in specs:
                concrete_type = spec.get(tp_name)
                entry_name = get_element_entry_name(concrete_type)
                case_idx = enum_entries.index(entry_name) if entry_name in enum_entries else specs.index(spec)
                if case_idx in seen_cases:
                    continue
                seen_cases.add(case_idx)
                out.append(f"            case {case_idx}: // {entry_name}")
                if tagged_info.get("is_slice"):
                    out.append(f"                that->{name}({cpp_pre_str}utils::Slice<const {concrete_type}>(((const {concrete_type}*) values) + offset, (size_t){count_name}));")
                else:
                    out.append(f"                that->{name}({cpp_pre_str}((const {concrete_type}*) values) + offset, {count_name});")
                out.append("                break;")
                
            out.append("        }")
            out.append("    });")
            out.append("")
            out.append(f"    env->{rel_elems}({values_name}_, values, JNI_ABORT);")
            for line in post_body_lines:
                out.append(f"    {line}")
            out.append("}")
            return "\n".join(out)

        # Step 7: Handle Packed Buffer Methods (Dual Buffer & Array Emission)
        if self.is_packed_buffer_method(method):
            info = self.get_packed_buffer_info(method)
            mode = m_ctx.get("buffer_mode", "buffer")
            name = method["name"]
            eff_name = get_effective_method_name(method)
            native_name = f"n{eff_name[0].upper() + eff_name[1:]}"
            stride = info["stride"]
            cpp_elem_type = info["cpp_elem_type"]
            buf_name = info["buf_name"]
            cnt_name = info["cnt_name"]
            has_existing_offset = info["has_existing_offset"]
            array_offset_name = info["array_offset_name"]

            pre_jni_params = []
            pre_param_jni_types = []
            pre_body_lines = []
            cpp_pre_call_args = []
            for a in info["pre_args"]:
                a_t = a["type"]
                a_info, _ = self.resolve_type_info(a_t, silent=True)
                safe_name = sanitize_identifier(a["name"])
                if a_info.get("is_filament_type"):
                    param_name = f"native{safe_name[0].upper()}{safe_name[1:]}"
                    pre_jni_params.append(f"jlong {param_name}")
                    pre_param_jni_types.append("jlong")
                    cpp_class = a_info["cpp_class"]
                    pre_body_lines.append(f"{cpp_class}* const {safe_name} = ({cpp_class}*) {param_name};")
                    if a_t.get("is_reference"):
                        cpp_pre_call_args.append(f"*{safe_name}")
                    else:
                        cpp_pre_call_args.append(safe_name)
                elif a_info.get("is_enum"):
                    pre_jni_params.append(f"jint {safe_name}")
                    pre_param_jni_types.append("jint")
                    cpp_pre_call_args.append(f"({self._strip_namespaces(a_info['cpp_enum_type'])}){safe_name}")
                else:
                    jni_t = self.get_jni_type(a_t)
                    pre_jni_params.append(f"{jni_t} {safe_name}")
                    pre_param_jni_types.append(jni_t)
                    cpp_pre_call_args.append(self.get_to_cpp_expr(a_t, safe_name))

            cpp_pre_call_str = ", ".join(cpp_pre_call_args) + ", " if cpp_pre_call_args else ""

            if mode == "buffer":
                if has_existing_offset:
                    params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"] + pre_jni_params + [
                        f"jobject {buf_name}_",
                        "jint remaining",
                        f"jint {cnt_name}",
                        "jint offset"
                    ]
                    param_jni_types = ["jlong"] + pre_param_jni_types + ["Buffer", "jint", "jint", "jint"]
                else:
                    params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"] + pre_jni_params + [
                        f"jobject {buf_name}_",
                        "jint remaining",
                        f"jint {cnt_name}"
                    ]
                    param_jni_types = ["jlong"] + pre_param_jni_types + ["Buffer", "jint", "jint"]

                if self.native_counts.get(native_name, 0) > 1:
                    mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                    jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}__{mangled_sig}"
                else:
                    jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"

                if jni_func_name in self.emitted_jni_funcs:
                    return None
                self.emitted_jni_funcs.add(jni_func_name)

                out = []
                out.append(f"extern \"C\" JNIEXPORT jint JNICALL")
                out.append(f"{jni_func_name}({', '.join(params)}) {{")
                out.append(f"    {self.name}* const that = ({self.name}*) native{self.name};")
                for bl in pre_body_lines:
                    out.append(f"    {bl}")
                out.append("    return filament::android::wrapJni<jint>(env, [=]() {")
                out.append(f"        AutoBuffer nioBuffer(env, {buf_name}_, {cnt_name} * {stride});")
                out.append("        void* data = nioBuffer.getData();")
                out.append("        size_t sizeInBytes = nioBuffer.getSize();")
                out.append("        if (sizeInBytes > (remaining << nioBuffer.getShift())) {")
                out.append("            // BufferOverflowException")
                out.append("            return -1;")
                out.append("        }")
                if info.get("is_slice"):
                    slice_expr = f"utils::Slice<{cpp_elem_type} const>(static_cast<{cpp_elem_type} const *>(data), (size_t){cnt_name})"
                    if has_existing_offset:
                        out.append(f"        that->{name}({cpp_pre_call_str}{slice_expr}, (size_t)offset);")
                    else:
                        out.append(f"        that->{name}({cpp_pre_call_str}{slice_expr});")
                else:
                    if has_existing_offset:
                        out.append(f"        that->{name}({cpp_pre_call_str}static_cast<{cpp_elem_type} const *>(data), (size_t){cnt_name}, (size_t)offset);")
                    else:
                        out.append(f"        that->{name}({cpp_pre_call_str}static_cast<{cpp_elem_type} const *>(data), (size_t){cnt_name});")
                out.append("        return 0;")
                out.append("    });")
                out.append("}")
                return "\n".join(out)

            else: # mode == "array"
                scalar = info.get("scalar", "float")
                cap_scalar = scalar.capitalize()
                jni_type = info.get("jni_type", f"j{scalar}Array")
                jni_scalar = f"j{scalar}"

                if has_existing_offset:
                    params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"] + pre_jni_params + [
                        f"{jni_type} {buf_name}_",
                        f"jint {cnt_name}",
                        "jint offset",
                        "jint arrayOffset"
                    ]
                    param_jni_types = ["jlong"] + pre_param_jni_types + [jni_type, "jint", "jint", "jint"]
                else:
                    params = ["JNIEnv *env", "jclass clazz", f"jlong native{self.name}"] + pre_jni_params + [
                        f"{jni_type} {buf_name}_",
                        f"jint {cnt_name}",
                        "jint offset"
                    ]
                    param_jni_types = ["jlong"] + pre_param_jni_types + [jni_type, "jint", "jint"]

                if self.native_counts.get(native_name, 0) > 1:
                    mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
                    jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}__{mangled_sig}"
                else:
                    jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"

                if jni_func_name in self.emitted_jni_funcs:
                    return None
                self.emitted_jni_funcs.add(jni_func_name)

                out = []
                out.append(f"extern \"C\" JNIEXPORT void JNICALL")
                out.append(f"{jni_func_name}({', '.join(params)}) {{")
                out.append(f"    {self.name}* const that = ({self.name}*) native{self.name};")
                for bl in pre_body_lines:
                    out.append(f"    {bl}")
                out.append(f"    {jni_scalar} const * const {buf_name} = env->Get{cap_scalar}ArrayElements({buf_name}_, nullptr);")
                out.append("    wrapJni(env, [=]() {")
                if info.get("is_slice"):
                    if has_existing_offset:
                        slice_expr = f"utils::Slice<{cpp_elem_type} const>(reinterpret_cast<{cpp_elem_type} const *>({buf_name}) + arrayOffset, (size_t){cnt_name})"
                        out.append(f"        that->{name}({cpp_pre_call_str}{slice_expr}, (size_t)offset);")
                    else:
                        slice_expr = f"utils::Slice<{cpp_elem_type} const>(reinterpret_cast<{cpp_elem_type} const *>({buf_name}) + offset, (size_t){cnt_name})"
                        out.append(f"        that->{name}({cpp_pre_call_str}{slice_expr});")
                else:
                    if has_existing_offset:
                        out.append(f"        that->{name}({cpp_pre_call_str}reinterpret_cast<{cpp_elem_type} const *>({buf_name}) + arrayOffset, (size_t){cnt_name}, (size_t)offset);")
                    else:
                        out.append(f"        that->{name}({cpp_pre_call_str}reinterpret_cast<{cpp_elem_type} const *>({buf_name}) + offset, (size_t){cnt_name});")
                out.append("    });")
                out.append(f"    env->Release{cap_scalar}ArrayElements({buf_name}_, const_cast<{jni_scalar} *>({buf_name}), JNI_ABORT);")
                out.append("}")
                return "\n".join(out)

        # Step 8: Standard Method Parameter and Return Marshalling
        is_static = method.get("is_static", False) or self.is_utility_class
        is_noexcept = method.get("is_noexcept", False)
        
        math_ret_info = None
        is_value_obj_ret = False
        is_slice_ret = False
        is_container_ret = False
        is_struct_ret = False
        ret_info = {}
        if method.get("is_constructor"):
            clean_types = "".join(self._get_clean_type_name(a["type"]) for a in method.get("arguments", []))
            native_name = f"nCreate{self.name}{clean_types}" if clean_types else f"nCreate{self.name}"
            if self.archetype == "bitfield":
                jni_ret = self.value_type_info.get("jni_type", "jint")
            else:
                jni_ret = "jlong"
        else:
            native_name = "n" + eff_name[0].upper() + eff_name[1:]
            cpp_ret = method["return_type"]
            ret_info, _ = self.resolve_type_info(cpp_ret)
            math_ret_info = ret_info if "assert" in ret_info else None
            is_value_obj_ret = ret_info.get("is_value_object", False)
            is_slice_ret = ret_info.get("is_slice", False) and not ret_info.get("is_struct_slice", False)
            is_container_ret = ret_info.get("is_container", False) and not math_ret_info and not is_slice_ret
            is_struct_ret = ret_info.get("is_struct", False) and not self.is_aggregate
            if self.archetype == "bitfield" and (self.get_java_type(cpp_ret) == "void" or ret_info.get("cpp_type") in (self.name, f"{self.name} &", f"const {self.name} &")):
                jni_ret = self.value_type_info.get("jni_type", "jint")
            elif is_slice_ret:
                scalar = ret_info.get("scalar", "int")
                jni_ret = ret_info.get("jni", f"j{scalar}Array")
                if not jni_ret.endswith("Array"):
                    jni_ret = f"{jni_ret}Array"
            elif is_container_ret:
                jni_ret = "jint"
            elif ret_info.get("is_enum"):
                jni_ret = "jint"
            elif math_ret_info or is_value_obj_ret or is_struct_ret:
                jni_ret = "void"
            else:
                jni_ret = self.get_jni_type(cpp_ret)

        params = ["JNIEnv *env", "jclass clazz"]
        param_jni_types = []
        if method.get("is_constructor"):
            pass
        elif self.archetype == "inline_buffer":
            params.append("jfloatArray planes_")
            param_jni_types.append("jfloatArray")
        elif self.archetype == "bitfield":
            field_name = self.value_type_info.get("field_name", "mSampler")
            jni_prim = self.value_type_info.get("jni_type", "jint")
            params.append(f"{jni_prim} {field_name[1:].lower()}_")
            param_jni_types.append(jni_prim)
        elif self.is_aggregate and not is_static:
            leaves = self.get_flattened_leaves(self.name)
            for leaf in leaves:
                params.append(f"{leaf['jni_type']} {leaf['param_name']}")
                param_jni_types.append(leaf['jni_type'])
        elif not is_static:
            params.append(f"jlong native{self.name}")
            param_jni_types.append("jlong")
            
        cpp_call_args = []
        body_pre = []
        body_post = []
        
        for arg in method.get("arguments", []):
            arg_name = arg["name"]
            cpp_type = arg["type"]
            
            if is_container_ret and arg_name in ("historySize", "size", "count", "capacity", "maxCount"):
                continue

            arr_in = self.get_array_input_info(cpp_type)
            if arr_in:
                 cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else cpp_type
                 is_matrix = re.search(r"::mat\d", cpp_type_str) or "::TMat" in cpp_type_str
                 is_pointer = cpp_type.get("is_pointer", False)
                 
                 size_param_name = get_size_param_attr(arg)
                 is_slice = arr_in.get("is_slice", False)

                 if not is_matrix and not is_pointer and not size_param_name and not is_slice:
                     # Unroll fixed math vector components directly into primitive parameters
                     scalar_jni = get_jni_scalar_type(arr_in["scalar"])
                     size = arr_in["size"]
                     components = ["x", "y", "z", "w"][:size]
                     
                     comp_args = []
                     for comp in components:
                         comp_arg_name = sanitize_identifier(f"{arg_name}{comp}")
                         params.append(f"{scalar_jni} {comp_arg_name}")
                         param_jni_types.append(scalar_jni)
                         comp_args.append(comp_arg_name)
                         
                     type_cstr = self._strip_namespaces(arr_in["cpp_type"])
                     cpp_call_args.append(f"{type_cstr}{{{', '.join(comp_args)}}}")
                     
                 else:
                     # Matrix or Pointer Input passed as primitive array
                     jni_arr_type = arr_in["jni"]
                     jni_arg_name = f"{arg_name}_"
                     params.append(f"{jni_arr_type} {jni_arg_name}")
                     param_jni_types.append(jni_arr_type)
                     if is_slice:
                         count_arg_name = sanitize_identifier(f"{arg_name}Count")
                         params.append(f"jint {count_arg_name}")
                         param_jni_types.append("jint")
                     
                     scalar = arr_in["scalar"]
                     if scalar == "float":
                         elem_accessor = "GetFloatArrayElements"
                         elem_releaser = "ReleaseFloatArrayElements"
                         scalar_jni_type = "jfloat"
                     elif scalar == "double":
                         elem_accessor = "GetDoubleArrayElements"
                         elem_releaser = "ReleaseDoubleArrayElements"
                         scalar_jni_type = "jdouble"
                     elif scalar == "int":
                         elem_accessor = "GetIntArrayElements"
                         elem_releaser = "ReleaseIntArrayElements"
                         scalar_jni_type = "jint"
                     elif scalar == "byte":
                         elem_accessor = "GetByteArrayElements"
                         elem_releaser = "ReleaseByteArrayElements"
                         scalar_jni_type = "jbyte"
                     elif scalar == "short":
                         elem_accessor = "GetShortArrayElements"
                         elem_releaser = "ReleaseShortArrayElements"
                         scalar_jni_type = "jshort"
                     elif scalar == "long":
                         elem_accessor = "GetLongArrayElements"
                         elem_releaser = "ReleaseLongArrayElements"
                         scalar_jni_type = "jlong"
                     elif scalar == "boolean":
                         elem_accessor = "GetBooleanArrayElements"
                         elem_releaser = "ReleaseBooleanArrayElements"
                         scalar_jni_type = "jboolean"
                     elif scalar == "char":
                         elem_accessor = "GetCharArrayElements"
                         elem_releaser = "ReleaseCharArrayElements"
                         scalar_jni_type = "jchar"
                     else:
                         elem_accessor = "GetIntArrayElements"
                         elem_releaser = "ReleaseIntArrayElements"
                         scalar_jni_type = "jint"
                     
                     is_pointer = arg["type"].get("is_pointer", False)
                     is_const_ptr = arg["type"].get("is_const", False) or arr_in.get("is_const", False)
                     
                     ptr_var = arg_name
                     
                     is_nullable = False
                     if is_pointer:
                         nullability = arg["type"].get("nullability", "unspecified")
                         if nullability in ["nullable", "unspecified"]:
                             is_nullable = True

                     if is_const_ptr:
                         ptr_type = f"{scalar_jni_type} const * const"
                     else:
                         ptr_type = f"{scalar_jni_type} *"

                     release_mode = "JNI_ABORT" if is_const_ptr else "0"
                     cast_str = f"const_cast<{scalar_jni_type} *>" if is_const_ptr else ""
                     cast_expr = f"{cast_str}({ptr_var})" if cast_str else ptr_var

                     if is_nullable:
                          body_pre.append(f"{ptr_type} {ptr_var} = {jni_arg_name} ? env->{elem_accessor}({jni_arg_name}, nullptr) : nullptr;")
                          body_post.append(f"if ({ptr_var}) {{")
                          body_post.append(f"    env->{elem_releaser}({jni_arg_name}, {cast_expr}, {release_mode});")
                          body_post.append(f"}}")
                     else:
                          body_pre.append(f"{ptr_type} {ptr_var} = env->{elem_accessor}({jni_arg_name}, nullptr);")
                          body_post.append(f"env->{elem_releaser}({jni_arg_name}, {cast_expr}, {release_mode});")
                     
                     if is_slice:
                         cpp_target_type = arr_in["cpp_type"]
                         const_qual = " const" if is_const_ptr else ""
                         count_arg_name = sanitize_identifier(f"{arg_name}Count")
                         cpp_call_args.append(f"utils::Slice<{cpp_target_type}{const_qual}>({ptr_var} ? reinterpret_cast<{cpp_target_type}{const_qual} *>({ptr_var}) : nullptr, (size_t){count_arg_name})")
                     elif is_pointer:
                         if arr_in.get("is_custom_array"):
                             cpp_target_type = arr_in["cpp_type"]
                             const_qual = " const" if is_const_ptr else ""
                             cpp_call_args.append(f"({cpp_target_type}{const_qual} *){ptr_var}")
                         else:
                             full_cpp_type = self._strip_namespaces(arg["type"]["cpp_name"])
                             cpp_call_args.append(f"reinterpret_cast<{full_cpp_type}>({ptr_var})")
                     else:
                         cpp_target_type = self._strip_namespaces(arr_in["cpp_type"])
                         const_qual = " const" if is_const_ptr else ""
                         cpp_call_args.append(f"*reinterpret_cast<{cpp_target_type}{const_qual} *>({ptr_var})")
            
            else:
                inv_info = self.get_invocable_info(cpp_type, name)
                if inv_info:
                    jni_arg_name = f"{arg_name}_"
                    params.append(f"jobject {jni_arg_name}")
                    param_jni_types.append("jobject")
                    
                    body_pre.append(f"jclass const {arg_name}Class = env->GetObjectClass({jni_arg_name});")
                    body_pre.append(f"jmethodID const {arg_name}Method = env->GetMethodID({arg_name}Class, \"accept\", \"{inv_info['jni_method_sig']}\");")
                    
                    lambda_params = [f"{self._strip_namespaces(p['cpp_type'])} {p['name']}" for p in inv_info['params']]
                    call_args_jni = [p['to_jni'] for p in inv_info['params']]
                    
                    if inv_info['java_ret'] == "void":
                        lambda_body = f"""[env, {jni_arg_name}, {arg_name}Method]({', '.join(lambda_params)}) {{
        if (env->ExceptionCheck()) {{
            return;
        }}
        env->{inv_info['jni_call']}({jni_arg_name}, {arg_name}Method{', ' + ', '.join(call_args_jni) if call_args_jni else ''});
    }}"""
                    else:
                        lambda_body = f"""[env, {jni_arg_name}, {arg_name}Method]({', '.join(lambda_params)}) {{
        if (env->ExceptionCheck()) {{
            return {inv_info['default_ret']};
        }}
        return ({inv_info['cpp_ret']})env->{inv_info['jni_call']}({jni_arg_name}, {arg_name}Method{', ' + ', '.join(call_args_jni) if call_args_jni else ''});
    }}"""
                    cpp_call_args.append(lambda_body)
                else:
                    type_info, _ = self.resolve_type_info(cpp_type)
                    if type_info.get("is_pojo_struct"):
                        struct_cls = type_info["struct_cls"]
                        leaves = self.get_pojo_leaves(struct_cls)
                        for leaf in leaves:
                            params.append(f"{leaf['jni_type']} {leaf['jni_param_name']}")
                            param_jni_types.append(leaf['jni_type'])

                        scoped_type = f"View::{struct_cls['name']}"
                        body_pre.append(f"{scoped_type} options;")

                        vector_comps = {}
                        for leaf in leaves:
                            if leaf.get("is_vector_comp"):
                                vfield = leaf["vector_field"]
                                if vfield not in vector_comps:
                                    vector_comps[vfield] = {
                                        "cpp_type": leaf["cpp_type"],
                                        "comps": []
                                    }
                                vector_comps[vfield]["comps"].append((leaf["comp_idx"], leaf["jni_param_name"]))
                            elif leaf.get("cpp_assign"):
                                body_pre.append(leaf["cpp_assign"])

                        for vfield, vinfo in vector_comps.items():
                            sorted_comps = [cname for idx, cname in sorted(vinfo["comps"], key=lambda x: x[0])]
                            cpp_t = vinfo["cpp_type"]
                            if not cpp_t.startswith("filament::") and not cpp_t.startswith("math::"):
                                if "LinearColor" in cpp_t:
                                    cpp_t = f"filament::{cpp_t}"
                                elif "float" in cpp_t:
                                    cpp_t = f"filament::math::{cpp_t}"
                            body_pre.append(f"options.{vfield} = {cpp_t}{{ {', '.join(sorted_comps)} }};")

                        cpp_call_args.append("options")
                        continue
                    elif type_info.get("is_struct"):
                        struct_cls = type_info["struct_cls"]
                        leaves = self.get_flattened_leaves(struct_cls, prefix=arg_name)
                        for leaf in leaves:
                            params.append(f"{leaf['jni_type']} {leaf['param_name']}")
                            param_jni_types.append(leaf['jni_type'])
                        
                        cpp_arg_type = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else str(cpp_type)
                        construct_lines = self.generate_cpp_struct_construction(struct_cls, arg_name, leaf_prefix=arg_name, type_override=cpp_arg_type)
                        for cl in construct_lines:
                            body_pre.append(cl)
                        cleanup_lines = self.generate_cpp_struct_cleanup(struct_cls, leaf_prefix=arg_name)
                        for cl in cleanup_lines:
                            body_post.append(cl)
                        cpp_call_args.append(arg_name)
                        continue

                    if type_info.get("is_string"):
                        jni_arg_name = f"{arg_name}_"
                        params.append(f"jstring {jni_arg_name}")
                        param_jni_types.append("jstring")
                        
                        nullability = cpp_type.get("nullability", "unspecified") if isinstance(cpp_type, dict) else "unspecified"
                        if nullability == "nullable":
                            body_pre.append(f"char const * const {arg_name} = {jni_arg_name} ? env->GetStringUTFChars({jni_arg_name}, nullptr) : nullptr;")
                            body_post.append(f"if ({jni_arg_name}) {{")
                            body_post.append(f"    env->ReleaseStringUTFChars({jni_arg_name}, {arg_name});")
                            body_post.append(f"}}")
                        else:
                            body_pre.append(f"char const * const {arg_name} = env->GetStringUTFChars({jni_arg_name}, nullptr);")
                            body_post.append(f"env->ReleaseStringUTFChars({jni_arg_name}, {arg_name});")
                        
                        cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else str(cpp_type)
                        clean_type = self._strip_namespaces(cpp_type_str).replace("&", "").replace("const", "").strip()
                        if "string_view" in clean_type or "basic_string_view" in clean_type:
                            cpp_call_args.append(f"std::string_view({arg_name})")
                        elif clean_type == "CString":
                            cpp_call_args.append(f"utils::CString({arg_name})")
                        elif clean_type == "StaticString":
                            cpp_call_args.append(f"utils::StaticString({arg_name})")
                        elif clean_type == "ImmutableCString":
                            cpp_call_args.append(f"utils::ImmutableCString({arg_name})")
                        elif clean_type == "string":
                            cpp_call_args.append(f"std::string({arg_name})")
                        else:
                            cpp_call_args.append(arg_name)
                    elif type_info.get("is_enum"):
                        params.append(f"jint {arg_name}")
                        param_jni_types.append("jint")
                        cpp_call_args.append(f"({self._strip_namespaces(type_info['cpp_enum_type'])}){arg_name}")
                    elif type_info.get("is_filament_type"):
                        param_name = f"native{arg_name[0].upper()}{arg_name[1:]}"
                        params.append(f"jlong {param_name}")
                        param_jni_types.append("jlong")
                        cpp_class = type_info["cpp_class"]
                        is_const_arg = cpp_type.get("is_const", False) if isinstance(cpp_type, dict) else False
                        is_ref = cpp_type.get("is_reference", False) if isinstance(cpp_type, dict) else False
                        cpp_type_str = cpp_type.get("qualified_name") or cpp_type["cpp_name"] if isinstance(cpp_type, dict) else str(cpp_type)
                        if "&" in cpp_type_str:
                            is_ref = True
                        const_str = " const" if is_const_arg else ""
                        body_pre.append(f"{cpp_class}{const_str}* const {arg_name} = ({cpp_class}{const_str}*) {param_name};")
                        if is_ref:
                            cpp_call_args.append(f"*{arg_name}")
                        else:
                            cpp_call_args.append(arg_name)
                    else:
                        jni_arg_type = self.get_jni_type(cpp_type)
                        params.append(f"{jni_arg_type} {arg_name}")
                        param_jni_types.append(jni_arg_type)
                        expr = self.get_to_cpp_expr(cpp_type, arg_name)
                        cpp_call_args.append(expr)
            
        if math_ret_info:
            jni_arr_type = math_ret_info["jni"]
            params.append(f"{jni_arr_type} out_")
            param_jni_types.append(jni_arr_type)
        elif is_value_obj_ret:
            params.append("jfloatArray out_")
            param_jni_types.append("jfloatArray")
        elif is_slice_ret:
            scalar = ret_info.get("scalar", "int")
            jni_arr = ret_info.get("jni", f"j{scalar}Array")
            if not jni_arr.endswith("Array"):
                jni_arr = f"{jni_arr}Array"
            params.append(f"{jni_arr} out_")
            param_jni_types.append(jni_arr)
        elif is_container_ret:
            out_param_name = "outHistory" if "History" in name else "out"
            if ret_info.get("is_struct_container"):
                params.append(f"jobjectArray {out_param_name}")
                param_jni_types.append("jobjectArray")
            else:
                jni_arr = ret_info.get("jni", "jlongArray")
                params.append(f"{jni_arr} {out_param_name}")
                param_jni_types.append(jni_arr)
        elif is_struct_ret:
            params.append("jobject out_")
            param_jni_types.append("jobject")
        
        # Step 9: JNI Signature Symbol Mangling
        if self.native_counts.get(native_name, 0) > 1:
            mangled_sig = "".join(self.get_jni_mangled_type(t) for t in param_jni_types)
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}__{mangled_sig}"
        else:
            jni_func_name = f"Java_{self.jni_package}_{self.name}_{native_name}"
        
        if jni_func_name in self.emitted_jni_funcs:
            return None
        self.emitted_jni_funcs.add(jni_func_name)
        
        out = []
        out.append(f"extern \"C\" JNIEXPORT {jni_ret} JNICALL")
        out.append(f"{jni_func_name}({', '.join(params)}) {{")
        
        # Step 10: Native Target Pointer Resolution
        if method.get("is_constructor"):
            for line in body_pre:
                out.append(f"    {line}")
            if self.base_class or self.archetype == "handle":
                call_args = ", ".join(cpp_call_args)
                out.append(f"    return (jlong) new {self.cpp_name}({call_args});")
            else:
                call_expr = f"{self.cpp_name}({', '.join(cpp_call_args)})" if cpp_call_args else f"{self.cpp_name}{{}}"
                if self.archetype == "bitfield":
                    from_cpp_expr = self.value_type_info.get("from_cpp", "filament::JniUtils::to_int({value})").format(value=call_expr)
                    out.append(f"    return {from_cpp_expr};")
                else:
                    out.append(f"    return filament::JniUtils::to_long({call_expr});")
            out.append("}")
            return "\n".join(out)

        if self.archetype == "inline_buffer":
            is_const = method.get("is_const", False)
            field_name = self.value_type_info.get("field_name", "mPlanes")
            if is_const:
                out.append(f"    jfloat const * const {field_name[1:].lower()} = env->GetFloatArrayElements(planes_, nullptr);")
                out.append(f"    {self.cpp_name} const * const that = reinterpret_cast<{self.cpp_name} const *>({field_name[1:].lower()});")
                body_post.append(f"env->ReleaseFloatArrayElements(planes_, const_cast<jfloat *>({field_name[1:].lower()}), JNI_ABORT);")
            else:
                out.append(f"    jfloat *{field_name[1:].lower()} = env->GetFloatArrayElements(planes_, nullptr);")
                out.append(f"    {self.cpp_name}* const that = reinterpret_cast<{self.cpp_name}*>({field_name[1:].lower()});")
                body_post.append(f"env->ReleaseFloatArrayElements(planes_, {field_name[1:].lower()}, 0);")
        elif self.archetype == "bitfield":
            field_name = self.value_type_info.get("field_name", "mSampler")
            to_cpp_expr = self.value_type_info.get("to_cpp", "filament::JniUtils::from_int({value})").format(value=f"{field_name[1:].lower()}_")
            out.append(f"    {self.cpp_name} that = {to_cpp_expr};")
        elif self.is_aggregate and not is_static:
            construct_lines = self.generate_cpp_struct_construction({"name": self.cpp_name, "fields": self.fields}, "that")
            for cl in construct_lines:
                out.append(f"    {cl}")
        elif not is_static:
            is_const = method.get("is_const", False)
            if is_const:
                out.append(f"    {self.cpp_name} const * const that = ({self.cpp_name} const *) native{self.name};")
            else:
                out.append(f"    {self.cpp_name}* const that = ({self.cpp_name}*) native{self.name};")
        
        for line in body_pre:
            out.append(f"    {line}")
            
        orig_name = method.get("orig_name", name)
        spec_type = m_ctx.get("specialization_type")
        target_name = f"{orig_name}<{spec_type}>" if spec_type else orig_name

        if is_static:
            call_target = f"{self.cpp_name}::{target_name}"
        elif self.is_aggregate or self.archetype == "bitfield":
            call_target = f"that.{target_name}"
        else:
            call_target = f"that->{target_name}"

        # Step 11: Call Invocation & Return Value Marshalling
        if math_ret_info:
            scalar = math_ret_info["scalar"]
            cpp_target_type = self._strip_namespaces(math_ret_info["cpp_type"])
            
            elem_accessor = "GetFloatArrayElements" if scalar == "float" else "GetDoubleArrayElements"
            elem_releaser = "ReleaseFloatArrayElements" if scalar == "float" else "ReleaseDoubleArrayElements"
            ptr_type = "jfloat *" if scalar == "float" else "jdouble *"
            
            out.append(f"    {ptr_type}out = env->{elem_accessor}(out_, nullptr);")
            if is_noexcept:
                out.append(f"    *reinterpret_cast<{cpp_target_type} *>(out) = {call_target}({', '.join(cpp_call_args)});")
            else:
                out.append("    wrapJni(env, [=]() {")
                out.append(f"        *reinterpret_cast<{cpp_target_type} *>(out) = {call_target}({', '.join(cpp_call_args)});")
                out.append("    });")
            out.append(f"    env->{elem_releaser}(out_, out, 0);")
            for line in body_post:
                out.append(f"    {line}")
        elif is_value_obj_ret:
            cpp_target_type = self._strip_namespaces(ret_info["cpp_type"])
            out.append("    jfloat *out = env->GetFloatArrayElements(out_, nullptr);")
            if is_noexcept:
                out.append(f"    *reinterpret_cast<{cpp_target_type} *>(out) = {call_target}({', '.join(cpp_call_args)});")
            else:
                out.append("    wrapJni(env, [=]() {")
                out.append(f"        *reinterpret_cast<{cpp_target_type} *>(out) = {call_target}({', '.join(cpp_call_args)});")
                out.append("    });")
            out.append("    env->ReleaseFloatArrayElements(out_, out, 0);")
            for line in body_post:
                out.append(f"    {line}")
        elif is_slice_ret:
            scalar = ret_info.get("scalar", "int")
            jtype_cap = scalar[0].upper() + scalar[1:]
            call_expr = f"{call_target}({', '.join(cpp_call_args)})"
            if is_noexcept:
                out.append(f"    auto const slice = {call_expr};")
                out.append(f"    jsize const count = static_cast<jsize>(slice.size());")
                out.append(f"    if (out_ == nullptr || env->GetArrayLength(out_) < count) {{")
                out.append(f"        out_ = env->New{jtype_cap}Array(count);")
                out.append(f"    }}")
                out.append(f"    if (count > 0) {{")
                out.append(f"        env->Set{jtype_cap}ArrayRegion(out_, 0, count, reinterpret_cast<const j{scalar}*>(slice.data()));")
                out.append(f"    }}")
                for line in body_post:
                    out.append(f"    {line}")
                out.append("    return out_;")
            else:
                out.append(f"    return wrapJni<{jni_ret}>(env, [&]() {{")
                out.append(f"        auto const slice = {call_expr};")
                out.append(f"        jsize const count = static_cast<jsize>(slice.size());")
                out.append(f"        if (out_ == nullptr || env->GetArrayLength(out_) < count) {{")
                out.append(f"            out_ = env->New{jtype_cap}Array(count);")
                out.append(f"        }}")
                out.append(f"        if (count > 0) {{")
                out.append(f"            env->Set{jtype_cap}ArrayRegion(out_, 0, count, reinterpret_cast<const j{scalar}*>(slice.data()));")
                out.append(f"        }}")
                for line in body_post:
                    out.append(f"    {line}")
                out.append("        return out_;")
                out.append("    });")
        elif is_container_ret:
            out_param_name = "outHistory" if "History" in name else "out"
            if ret_info.get("is_struct_container"):
                struct_cls = ret_info.get("struct_cls") or {}
                struct_name = struct_cls.get("name", ret_info.get("java_elem_type", "Object"))
                jni_state_name = f"Jni{struct_name}State"
                
                is_nested_in_self = any(c.name == struct_name for c in self.nested_struct_contexts) or struct_name in self.nested_structs_map.get(self.name, []) or any(child.get("name") == struct_name for child in self.nested_structs_map.get(self.name, []))
                if is_nested_in_self:
                    class_path = f"{self.package.replace('.', '/')}/{self.name}${struct_name}"
                else:
                    class_path = f"{self.package.replace('.', '/')}/{struct_name}"
                    
                fields = struct_cls.get("fields", [])
                out.append(f"    static const struct {jni_state_name} {{")
                for f in fields:
                    out.append(f"        jfieldID {f['name']};")
                out.append(f"        explicit {jni_state_name}(JNIEnv* env) noexcept {{")
                out.append(f"            jclass frameInfoClass = env->FindClass(\"{class_path}\");")
                for f in fields:
                    f_info, _ = self.resolve_type_info(f["type"], silent=True)
                    f_type_str = f["type"].get("cpp_name", "") if isinstance(f["type"], dict) else str(f["type"])
                    if f_info.get("is_enum") or f_type_str in ("int", "int32_t", "uint32_t", "size_t"):
                        sig = "I"
                    elif "64" in f_type_str or "long" in f_type_str:
                        sig = "J"
                    elif f_type_str == "float":
                        sig = "F"
                    elif f_type_str == "double":
                        sig = "D"
                    elif f_type_str == "bool":
                        sig = "Z"
                    elif "16" in f_type_str or f_type_str == "short":
                        sig = "S"
                    elif "8" in f_type_str or f_type_str in ("byte", "char"):
                        sig = "B"
                    else:
                        sig = "J"
                    out.append(f"            {f['name']} = env->GetFieldID(frameInfoClass, \"{f['name']}\", \"{sig}\");")
                out.append("        }")
                out.append("    } jniState(env);")
                out.append("")
                out.append(f"    jsize const arrayLength = env->GetArrayLength({out_param_name});")
                out.append("    if (arrayLength <= 0) {")
                out.append("        return 0;")
                out.append("    }")
                out.append("")
                call_args_str = ", ".join(cpp_call_args + ["static_cast<size_t>(arrayLength)"]) if any(a["name"] in ("historySize", "size", "count", "capacity", "maxCount") for a in method.get("arguments", [])) else ", ".join(cpp_call_args)
                out.append(f"    auto const history = {call_target}({call_args_str});")
                out.append("    jsize const count = static_cast<jsize>(history.size());")
                out.append("")
                out.append("    for (jsize i = 0; i < count; ++i) {")
                out.append(f"        jobject obj = env->GetObjectArrayElement({out_param_name}, i);")
                out.append("        if (!obj) {")
                out.append("            continue;")
                out.append("        }")
                out.append("")
                out.append("        auto const& info = history[i];")
                for f in fields:
                    f_info, _ = self.resolve_type_info(f["type"], silent=True)
                    f_type_str = f["type"].get("cpp_name", "") if isinstance(f["type"], dict) else str(f["type"])
                    if f_info.get("is_enum") or f_type_str in ("int", "int32_t", "uint32_t", "size_t"):
                        setter = "SetIntField"
                        cast = "static_cast<jint>("
                        cast_end = ")"
                    elif "64" in f_type_str or "long" in f_type_str:
                        setter = "SetLongField"
                        cast = ""
                        cast_end = ""
                    elif f_type_str == "float":
                        setter = "SetFloatField"
                        cast = "(jfloat) "
                        cast_end = ""
                    elif f_type_str == "double":
                        setter = "SetDoubleField"
                        cast = "(jdouble) "
                        cast_end = ""
                    elif f_type_str == "bool":
                        setter = "SetBooleanField"
                        cast = "(jboolean) "
                        cast_end = ""
                    elif "16" in f_type_str or f_type_str == "short":
                        setter = "SetShortField"
                        cast = "static_cast<jshort>("
                        cast_end = ")"
                    elif "8" in f_type_str or f_type_str in ("byte", "char"):
                        setter = "SetByteField"
                        cast = "static_cast<jbyte>("
                        cast_end = ")"
                    else:
                        setter = "SetLongField"
                        cast = "(jlong) "
                        cast_end = ""
                    out.append(f"        env->{setter}(obj, jniState.{f['name']}, {cast}info.{f['name']}{cast_end});")
                out.append("        env->DeleteLocalRef(obj);")
                out.append("    }")
                out.append("")
                out.append("    return count;")
            else:
                out.append(f"    jsize const arrayLength = env->GetArrayLength({out_param_name});")
                out.append("    if (arrayLength <= 0) {")
                out.append("        return 0;")
                out.append("    }")
                call_args_str = ", ".join(cpp_call_args + ["static_cast<size_t>(arrayLength)"]) if any(a["name"] in ("historySize", "size", "count", "capacity", "maxCount") for a in method.get("arguments", [])) else ", ".join(cpp_call_args)
                out.append(f"    auto const history = {call_target}({call_args_str});")
                out.append("    jsize const count = static_cast<jsize>(std::min((size_t)arrayLength, (size_t)history.size()));")
                scalar = ret_info.get("scalar", "int")
                jtype_cap = scalar[0].upper() + scalar[1:]
                out.append("    if (count > 0) {")
                out.append(f"        env->Set{jtype_cap}ArrayRegion({out_param_name}, 0, count, reinterpret_cast<const j{scalar}*>(history.data()));")
                out.append("    }")
                out.append("    return count;")
        elif is_struct_ret:
            struct_cls = ret_info.get("struct_cls") or self.is_aggregate_struct(ret_info.get("cpp_type"))
            struct_name = struct_cls["name"] if struct_cls else ret_info["java"]
            jni_state_name = f"Jni{struct_name}State"
            is_nested_in_self = any(c.name == struct_name for c in self.nested_struct_contexts) or struct_name in self.nested_structs_map.get(self.name, []) or any(child.get("name") == struct_name for child in self.nested_structs_map.get(self.name, []))
            if is_nested_in_self:
                class_path = f"{self.package.replace('.', '/')}/{self.name}${struct_name}"
            else:
                class_path = f"{self.package.replace('.', '/')}/{struct_name}"
            
            accessors = self.get_struct_leaf_accessors(struct_cls)
            out.append(f"    static const struct {jni_state_name} {{")
            for field_id_name, _, _, _, _, _, _ in accessors:
                out.append(f"        jfieldID {field_id_name};")
            out.append(f"        explicit {jni_state_name}(JNIEnv* env) noexcept {{")
            out.append(f"            jclass clazz = env->FindClass(\"{class_path}\");")
            for field_id_name, java_field_name, sig, _, _, _, _ in accessors:
                out.append(f"            {field_id_name} = env->GetFieldID(clazz, \"{java_field_name}\", \"{sig}\");")
            out.append("        }")
            out.append("    } jniState(env);")
            out.append("")
            call_expr = f"{call_target}({', '.join(cpp_call_args)})"
            if is_noexcept:
                out.append(f"    auto const& res = {call_expr};")
            else:
                out.append(f"    auto const res = wrapJni(env, [=]() {{")
                out.append(f"        return {call_expr};")
                out.append("    });")
            for field_id_name, _, _, setter, cast, cast_end, cpp_access in accessors:
                out.append(f"    env->{setter}(out_, jniState.{field_id_name}, {cast}res.{cpp_access}{cast_end});")
            for line in body_post:
                out.append(f"    {line}")
        elif self.archetype == "bitfield" and (self.get_java_type(cpp_ret) == "void" or ret_info.get("cpp_type") in (self.name, f"{self.name} &", f"const {self.name} &")):
            call_expr = f"{call_target}({', '.join(cpp_call_args)})"
            if is_noexcept:
                out.append(f"    {call_expr};")
            else:
                out.append("    wrapJni(env, [=]() {")
                out.append(f"        {call_expr};")
                out.append("    });")
            for line in body_post:
                out.append(f"    {line}")
            from_cpp_expr = self.value_type_info.get("from_cpp", "filament::JniUtils::to_int({value})").format(value="that")
            out.append(f"    return {from_cpp_expr};")
        else:
            call_expr = f"{call_target}({', '.join(cpp_call_args)})"
            is_deprecated = bool(
                method.get("doc", {}).get("meta", {}).get("deprecated")
                or any("deprecated" in str(a).lower() for a in method.get("attributes", []))
                or eff_name in ("setSampleCount", "getSampleCount", "setAmbientOcclusion", "getAmbientOcclusion")
            )

            if is_deprecated:
                out.append("#pragma clang diagnostic push")
                out.append('#pragma clang diagnostic ignored "-Wdeprecated-declarations"')

            if jni_ret != "void":
                from_cpp = self.get_from_cpp_expr(cpp_ret, call_expr, jni_ret)
                if body_post:
                    if is_noexcept:
                        out.append(f"    {jni_ret} const result = {from_cpp};")
                    else:
                        out.append(f"    {jni_ret} const result = wrapJni<{jni_ret}>(env, [=]() {{")
                        out.append(f"        return {from_cpp};")
                        out.append("    });")
                    for line in body_post:
                        out.append(f"    {line}")
                    out.append("    return result;")
                else:
                    if is_noexcept:
                        out.append(f"    return {from_cpp};")
                    else:
                        out.append(f"    return wrapJni<{jni_ret}>(env, [=]() {{")
                        out.append(f"        return {from_cpp};")
                        out.append("    });")
            else:
                if is_noexcept:
                    out.append(f"    {call_expr};")
                else:
                    out.append("    wrapJni(env, [=]() {")
                    out.append(f"        {call_expr};")
                    out.append("    });")
                for line in body_post:
                    out.append(f"    {line}")

            if is_deprecated:
                out.append("#pragma clang diagnostic pop")
            
        out.append("}")
        return "\n".join(out)

    def _get_engine_jni_methods(self) -> str:
        types = [
            "BufferObject", "ColorGrading", "Fence", "FramePacer",
            "IndexBuffer", "IndirectLight", "InstanceBuffer", "Material",
            "MaterialInstance", "MorphTargetBuffer", "RenderTarget",
            "Renderer", "Scene", "SkinningBuffer", "Skybox", "Stream",
            "SwapChain", "Texture", "VertexBuffer", "View"
        ]
        destroy_funcs = []
        for t in types:
            destroy_funcs.append(f"""extern "C" JNIEXPORT jboolean JNICALL
Java_{self.jni_package}_Engine_nDestroy{t}(JNIEnv *env, jclass,
        jlong nativeEngine, jlong nativeObject) {{
    Engine* engine = (Engine*) nativeEngine;
    {t}* obj = ({t}*) nativeObject;
    return wrapJni<jboolean>(env, [=]() {{
        return engine->destroy(obj);
    }});
}}""")

        destroy_code = "\n\n".join(destroy_funcs)

        return f"""extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nDestroyEngine(JNIEnv *env, jclass, jlong nativeEngine) {{
    wrapJni(env, [=]() {{
        Engine* engine = (Engine*) nativeEngine;
        Engine::destroy(&engine);
    }});
}}

extern "C" {{
extern void* getNativeWindow(JNIEnv* env, jclass, jobject surface);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{self.jni_package}_Engine_nCreateSwapChain(JNIEnv* env,
        jclass klass, jlong nativeEngine, jobject surface, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    void* win = getNativeWindow(env, klass, surface);
    return (jlong) engine->createSwapChain(win, (uint64_t) flags);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{self.jni_package}_Engine_nCreateSwapChainHeadless(JNIEnv*,
        jclass, jlong nativeEngine, jint width, jint height, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createSwapChain(width, height, (uint64_t) flags);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{self.jni_package}_Engine_nCreateSwapChainFromRawPointer(JNIEnv*,
        jclass, jlong nativeEngine, jlong pointer, jlong flags) {{
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) engine->createSwapChain((void*) pointer, (uint64_t) flags);
}}


extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nDestroyEntity(JNIEnv*, jclass,
        jlong nativeEngine, jint entity_) {{
    Engine* engine = (Engine*) nativeEngine;
    Entity& entity = *reinterpret_cast<Entity*>(&entity_);
    engine->destroy(entity);
}}

{destroy_code}

extern "C" JNIEXPORT jlong JNICALL
Java_{self.jni_package}_Engine_nCreateBuilder(JNIEnv*, jclass) {{
    return (jlong) new Engine::Builder{{}};
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nDestroyBuilder(JNIEnv*, jclass, jlong nativeBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    delete builder;
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderBackend(JNIEnv*, jclass,
        jlong nativeBuilder, jlong backend) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->backend((Engine::Backend) backend);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderConfig(JNIEnv*, jclass,
        jlong nativeBuilder, jlong commandBufferSizeMB, jlong perRenderPassArenaSizeMB,
        jlong driverHandleArenaSizeMB, jlong minCommandBufferSizeMB, jlong perFrameCommandsSizeMB,
        jlong jobSystemThreadCount, jboolean disableParallelShaderCompile,
        jint stereoscopicType, jlong stereoscopicEyeCount,
        jlong resourceAllocatorCacheSizeMB, jlong resourceAllocatorCacheMaxAge,
        jboolean disableHandleUseAfterFreeCheck,
        jint preferredShaderLanguage,
        jboolean forceGLES2Context, jboolean assertNativeWindowIsValid,
        jint gpuContextPriority,
        jlong sharedUboInitialSizeInBytes,
        jboolean enableMultipleDirectionalLights) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    Engine::Config config = {{
            .commandBufferSizeMB = (uint32_t) commandBufferSizeMB,
            .perRenderPassArenaSizeMB = (uint32_t) perRenderPassArenaSizeMB,
            .driverHandleArenaSizeMB = (uint32_t) driverHandleArenaSizeMB,
            .minCommandBufferSizeMB = (uint32_t) minCommandBufferSizeMB,
            .perFrameCommandsSizeMB = (uint32_t) perFrameCommandsSizeMB,
            .jobSystemThreadCount = (uint32_t) jobSystemThreadCount,
            .disableParallelShaderCompile = (bool) disableParallelShaderCompile,
            .stereoscopicType = (Engine::StereoscopicType) stereoscopicType,
            .stereoscopicEyeCount = (uint8_t) stereoscopicEyeCount,
            .resourceAllocatorCacheSizeMB = (uint32_t) resourceAllocatorCacheSizeMB,
            .resourceAllocatorCacheMaxAge = (uint8_t) resourceAllocatorCacheMaxAge,
            .disableHandleUseAfterFreeCheck = (bool) disableHandleUseAfterFreeCheck,
            .preferredShaderLanguage = (Engine::Config::ShaderLanguage) preferredShaderLanguage,
            .forceGLES2Context = (bool) forceGLES2Context,
            .assertNativeWindowIsValid = (bool) assertNativeWindowIsValid,
            .gpuContextPriority = (Engine::GpuContextPriority) gpuContextPriority,
            .sharedUboInitialSizeInBytes = (uint32_t) sharedUboInitialSizeInBytes,
            .enableMultipleDirectionalLights = (bool) enableMultipleDirectionalLights,
    }};
    builder->config(&config);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderFeatureLevel(JNIEnv*, jclass,
        jlong nativeBuilder, jint ordinal) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->featureLevel((Engine::FeatureLevel) ordinal);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderSharedContext(JNIEnv*, jclass,
        jlong nativeBuilder, jlong sharedContext) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->sharedContext((void*) sharedContext);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderPaused(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean paused) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    builder->paused((bool) paused);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderFeature(JNIEnv *env, jclass,
        jlong nativeBuilder, jstring name_, jboolean value) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    const char *name = env->GetStringUTFChars(name_, 0);
    builder->feature(name, (bool) value);
    env->ReleaseStringUTFChars(name_, name);
}}

extern "C" JNIEXPORT void JNICALL
Java_{self.jni_package}_Engine_nSetBuilderColorGrading(JNIEnv *env, jclass,
        jlong nativeBuilder, jlong nativeColorGradingBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    ColorGrading::Builder const* colorGradingBuilder = (ColorGrading::Builder const*) nativeColorGradingBuilder;
    builder->colorGrading(*colorGradingBuilder);
}}

extern "C" JNIEXPORT jlong JNICALL
Java_{self.jni_package}_Engine_nBuilderBuild(JNIEnv *env, jclass, jlong nativeBuilder) {{
    Engine::Builder* builder = (Engine::Builder*) nativeBuilder;
    return wrapJniBackend<jlong>(env, [=]() {{
        return (jlong) builder->build();
    }});
}}"""
