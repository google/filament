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
package {{ kotlin_package }}

{% set callbackName = 'on' + function_pointer.name.chunks[:-1] | map('title') | join %}

//* Generating KDocs
{% set all_callback_info = kdocs.callbacks %}
{% set funtion_pointer_info = all_callback_info.get(function_pointer.name.get()) %}
{% set main_doc = funtion_pointer_info.doc if funtion_pointer_info else "" %}
{% set arg_docs_map =  funtion_pointer_info.args if funtion_pointer_info else {} %}

{% set function_pointer_args = function_pointer.arguments | list %}
public fun interface {{ function_pointer.name.CamelCase() }} {
    {% if check_if_doc_present(main_doc, "", arg_docs_map, function_pointer_args) == 'True' %}
    {{ generate_kdoc(main_doc, return_str, arg_docs_map, function_pointer_args , indent_prefix = "    ",line_wrap_prefix = "\n     * ") }}
    {%- endif %}
    @Suppress("INAPPLICABLE_JVM_NAME")  //* Required for @JvmName on global function.
    @JvmName("{{ callbackName }}")  //* Required to access Inline Value Class parameters via JNI.
    public fun {{ callbackName }}(
    {%- for arg in kotlin_record_members(function_pointer.arguments) -%}
        {{ kotlin_annotation(arg) }} {{ as_varName(arg.name) }}: {{ kotlin_declaration(arg) }},{{ ' ' }}
    {%- endfor -%});
}

{% set args_list = kotlin_record_members(function_pointer.arguments) | list %}

internal class {{ function_pointer.name.CamelCase() }}Runnable(
private val callback: {{ function_pointer.name.CamelCase() }},
{% for arg in args_list %}
    private val {{ as_varName(arg.name) }}: {{ kotlin_declaration(arg) }}{{ ','
    if not loop.last }}
{% endfor %}
) : Runnable {
    override fun run() {
        callback.{{ callbackName }}(
            {%- for arg in args_list -%}
            {{ as_varName(arg.name) }}{{ ', ' if not loop.last }}
            {%- endfor -%}
        )
    }
}
