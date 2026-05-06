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

{% from 'art/api_kotlin_types.kt' import kotlin_declaration, generate_simple_kdoc, add_kdoc_disclaimer with context %}
{{ add_kdoc_disclaimer() }}
package {{ kotlin_package }}
{% set all_constants_info = kdocs.constants %}

public object Constants {
    /**
     * -1 to max int is resolved at compile time
     */
    private const val UINT32_MAX: Int = -1

    /**
     * -1L to max long is resolved at compile time
     */
    private const val UINT64_MAX: Long = -1L
    private const val SIZE_MAX = UINT64_MAX
    {% for constant in by_category['constant'] %}
        //* Generating KDocs
        {% set constant_doc = all_constants_info.get(constant.name.get()) %}
        {% if constant_doc %}

            {{ generate_simple_kdoc(constant_doc, indent_prefix = "    ", line_wrap_prefix = "\n     * ") }}
        {% endif %}
        public const val {{ as_ktName(constant.name.SNAKE_CASE() ) }}:{{ ' ' }}
        {{- kotlin_declaration(constant) }} =
        {%- if constant.value == 'NAN' %}
            {% if constant.type.name.get() == 'float' -%}   {{ ' ' }}Float.NaN
            {% elif constant.type.name.get() == 'double' %} {{ ' ' }}Double.NaN
            {% else %} {{ assert(false) }}
            {% endif %}
        {%- else %} {{ constant.value }}
        {% endif %}
    {% endfor %}
}
