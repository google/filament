/*
 * Copyright 2025 The Android Open Source Project
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
 */

{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition, check_if_doc_present, generate_kdoc, generate_simple_kdoc, add_kdoc_disclaimer with context %}
{% from 'art/api_kotlin_async_helpers.kt' import async_wrapper, analyze_callback with context %}
{{ add_kdoc_disclaimer() }}
package {{ kotlin_package }}

import dalvik.annotation.optimization.FastNative
import java.nio.ByteBuffer
import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.suspendCancellableCoroutine


//* Generating KDocs
{% set all_objects_info = kdocs.objects%}
{% set object_info = all_objects_info.get(obj.name.get()) %}
{% set doc_str = object_info.doc if object_info else "" %}
{% if doc_str | trim %}
    {{ generate_simple_kdoc(doc_str) }}
{% endif %}
public class {{ kotlin_name(obj) }} private constructor(public val handle: Long): AutoCloseable {
    {% set all_method_info = object_info.methods if object_info else {} %}
    {% for method in obj.methods if include_method(obj, method) %}
        {% set _kotlin_return = kotlin_return(method) %}
        //* Generating KDocs
        {% set method_info = all_method_info.get(method.name.snake_case()) %}
        {% set main_doc = method_info.doc if method_info else "" %}
        {% set return_doc = method_info.returns_doc if method_info else "" %}
        {% set arg_docs_map = method_info.args if method_info else {} %}
        {% set method_args = kotlin_record_members(method.arguments) | list %}
        {% if check_if_doc_present(main_doc, return_doc, arg_docs_map, method_args) == 'True' %}
        {{ generate_kdoc(main_doc, return_doc, arg_docs_map, method_args, "\n     * ", indent_prefix = "    ") }}

        {%- endif %}
        @FastNative
        @JvmName("{{ method.name.camelCase() }}")
        {% for arg in kotlin_record_members(method.arguments) %}
            {% if kotlin_default(arg) is not none %}
                @JvmOverloads
            {% break %}{% endif %}
        {% endfor %}
        {% if method.returns and method.returns.type.name.canonical_case() == 'status' %}
            @Throws({{"WebGpuException::class"}})
        {% endif %}
        {{ kotlin_annotation(_kotlin_return) if _kotlin_return else '' }} public external fun {{ method.name.camelCase() }}(
        //* TODO(b/341923892): rework async methods to use futures.
        {%- for arg in kotlin_record_members(method.arguments) %}
            {%- if arg.type.category == 'callback function' -%}
                {% set ns = namespace() %}
                {{ analyze_callback(arg, ns) }}
                {% set generic_T = kotlin_declaration(ns.payload_arg, true) if ns.payload_arg else 'Unit' %}
                callback: GPURequestCallback<{{kotlin_annotation(ns.payload_arg)}} {{ generic_T }}>,{{ ' ' }}
            {%- else -%}
                {{- kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},{{ ' ' }}
            {%- endif -%}
        {%- endfor -%}): {{ kotlin_declaration(_kotlin_return) if _kotlin_return else 'Unit' }}

        {% if method.name.chunks[0] == 'get' and not method.arguments %}
            //* For the Kotlin getter, strip word 'get' from name and convert the remainder to
            //* camelCase() (lower case first word). E.g. "get foo bar" translated to fooBar.
            {% set name = method.name.chunks[1] + method.name.chunks[2:] | map('title') | join %}
            @get:JvmName("{{ name }}")
            public val {{ name }}: {{ kotlin_declaration(_kotlin_return) if _kotlin_return else 'Unit' }} get() = {{ method.name.camelCase() }}()

        {% endif %}

        //* Every method that is identified as using callbacks is given a helper method that wraps the
        //* call with a suspend function.
        {%- for arg in kotlin_record_members(method.arguments) %}
            {% if arg.type.category == 'callback function' %}
                {{- async_wrapper(obj, method, arg) -}}
                {{ continue }}
            {% endif %}
        {%- endfor -%}

    {% endfor %}
    /**
     * Decrements the reference count of the object and frees resources when the count reaches zero.
     *
     * This is the standard way to manage object lifetimes and should be used in `use` blocks.
     * After calling this, the object is no longer usable.
     */
    external override fun close()

    //* By default, the equals() function implements referential equality.
    //* see: https://kotlinlang.org/docs/equality.html#structural-equality
    //* A structural comparison of the wrapper object is equivalent to a referential comparison of
    //* the wrapped object.
    override fun equals(other: Any?): Boolean =
        other is {{ kotlin_name(obj) }} && other.handle == handle
    override fun hashCode(): Int = handle.hashCode()
}
