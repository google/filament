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

{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_declaration, kotlin_definition, check_if_doc_present, generate_kdoc, add_kdoc_disclaimer with context %}
{{ add_kdoc_disclaimer() }}
@file:JvmName("Functions")

package {{ kotlin_package }}

import dalvik.annotation.optimization.FastNative

public object GPU {

    {% set all_functions_info = kdocs.functions %}
    {% for function in by_category['function'] if include_method(None, function) %}
        {% set _kotlin_return = kotlin_return(function) %}
        //* Generating KDocs
        {% set function_info = all_functions_info.get(function.name.get()) %}
        {% set main_doc = function_info.doc if function_info else "" %}
        {% set return_doc = function_info.returns_doc if function_info else "" %}
        {% set arg_docs_map = function_info.args if function_info else {} %}
        {% set function_args = function.arguments | list %}
        {% if check_if_doc_present(main_doc, return_doc, arg_docs_map, function_args) == 'True' %}
            {{ generate_kdoc(main_doc, return_doc, arg_docs_map, function_args , line_wrap_prefix = "\n * ") }}

        {%- endif %}
        {% if function.has_default %}
            @JvmOverloads
        {% endif %}
        @FastNative
        {% if function.returns and function.returns.type.name.canonical_case() == 'status' %}
            @Throws({{"WebGpuException::class"}})
        {% endif %}
        {{ kotlin_annotation(_kotlin_return) if _kotlin_return else '' }} public external fun {{ function.name.camelCase() }}(
            {%- for arg in function.arguments -%}
                {{- kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_definition(arg) }},{{' '}}
            {%- endfor %}): {{ kotlin_declaration(_kotlin_return) if _kotlin_return else 'Unit' }}
{% endfor %}}