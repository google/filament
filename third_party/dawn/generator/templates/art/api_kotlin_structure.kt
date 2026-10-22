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

{% from 'art/api_kotlin_types.kt' import kotlin_annotation, kotlin_definition, kotlin_declaration, generate_simple_kdoc, add_kdoc_disclaimer, kotlin_member_optin, kotlin_class_optin, item_requires_optin, item_is_experimental, kotlin_parameter, kotlin_chain_parameter, kotlin_property, kotlin_chain_property with context %}
{{ add_kdoc_disclaimer() }}
package {{ kotlin_package }}

{% set ns = namespace(callback_count = 0, default_count = 0) %}
{%- set members = [] -%}
{%- for member in kotlin_record_members(structure.members, structure.name.get()) -%}
    {%- if member.name.camelCase().endswith('Callback') -%}
        {%- set ns.callback_count = ns.callback_count + 1 -%}
    {%- endif -%}
    {%- if kotlin_default(member) is not none -%}
        {%- set ns.default_count = ns.default_count + 1 -%}
    {%- endif -%}
    {%- do members.append(member) -%}
{%- endfor -%}

{%- set chain_children_list = chain_children[structure.name.get()] -%}
{%- for child in chain_children_list -%}
    {%- set ns.default_count = ns.default_count + 1 -%}
{%- endfor -%}

{%- set struct_config = customize_structures.get(structure.name.get(), {}) -%}
{%- set graduated_batches = struct_config.get('graduated_batches', []) -%}
{%- set class_is_experimental = item_is_experimental(structure) == 'True' -%}

{%- set stable_members = [] -%}
{%- set experimental_members = [] -%}
{%- for member in members -%}
    {%- if not class_is_experimental and item_requires_optin(member, chain_children=chain_children) == 'True' -%}
        {%- do experimental_members.append(member) -%}
    {%- else -%}
        {%- do stable_members.append(member) -%}
    {%- endif -%}
{%- endfor -%}

{%- set stable_chain_children = [] -%}
{%- set experimental_chain_children = [] -%}
{%- for child in chain_children_list -%}
    {%- if not class_is_experimental and item_requires_optin(child, chain_children=chain_children) == 'True' -%}
        {%- do experimental_chain_children.append(child) -%}
    {%- else -%}
        {%- do stable_chain_children.append(child) -%}
    {%- endif -%}
{%- endfor -%}

{%- set has_experimental = (experimental_members or experimental_chain_children) and not class_is_experimental -%}

{%- set stable_ctor_has_defaults = namespace(val=false) -%}
{%- for member in stable_members -%}
    {%- if kotlin_default(member) is not none -%}
        {%- set stable_ctor_has_defaults.val = true -%}
        {%- break -%}
    {%- endif -%}
{%- endfor -%}
{%- if not stable_ctor_has_defaults.val -%}
    {%- if stable_chain_children -%}
        {%- set stable_ctor_has_defaults.val = true -%}
    {%- endif -%}
{%- endif -%}

//* Generating KDocs
{% set all_structs_info = kdocs.structs -%}
{%- set struct_info = all_structs_info.get(structure.name.get()) -%}
{%- set main_doc = struct_info.doc if struct_info else "" -%}
{%- if main_doc %}
    {{ generate_simple_kdoc(main_doc) }}
{% endif %}
{%- set experimental = kotlin_class_optin(structure) -%}
{%- if experimental %}
    {{ experimental }}
{% endif %}
//* Generates legacy constructors for Android ABI compatibility.
//* Omits newly added non-nullable fields to maintain stable APIs.
{% macro render_legacy_constructors(graduated_batches, stable_members, stable_chain_children, structure) %}
    {% if graduated_batches %}
        {% for batch in graduated_batches %}
            {% set batch_index = loop.index0 %}
            {% set current_legacy_members = [] %}
            {% for m in stable_members %}
                {% set m_name = m.name.get() %}
                {% set in_current_or_future_batch = namespace(val=false) %}
                {% for future_batch in graduated_batches[batch_index:] %}
                    {% if m_name in future_batch %}
                        {% set in_current_or_future_batch.val = true %}
                    {% endif %}
                {% endfor %}
                {% if not in_current_or_future_batch.val %}
                    {% do current_legacy_members.append(m) %}
                {% endif %}
            {% endfor %}
            {% set current_legacy_chain_children = [] %}
            {% for child in stable_chain_children %}
                {% set child_name = child.name.get() %}
                {% set in_current_or_future_batch = namespace(val=false) %}
                {% for future_batch in graduated_batches[batch_index:] %}
                    {% if child_name in future_batch %}
                        {% set in_current_or_future_batch.val = true %}
                    {% endif %}
                {% endfor %}
                {% if not in_current_or_future_batch.val %}
                    {% do current_legacy_chain_children.append(child) %}
                {% endif %}
            {% endfor %}

            {% set legacy_params = [] %}
            {% for m in current_legacy_members %}
                {% do legacy_params.append(kotlin_parameter(m, parent=structure)) %}
            {%- endfor %}
            {% for child in current_legacy_chain_children %}
                {% do legacy_params.append(kotlin_chain_parameter(child, parent=structure)) %}
            {%- endfor %}

            @Deprecated(
                message = "Hidden constructor for binary compatibility",
                level = DeprecationLevel.HIDDEN,
            )
            public constructor(
                {% for param in legacy_params %}
                    {{ param }}{{ "," if not loop.last }}
                {% endfor %}
            ) : this(
                {% for m in stable_members %}
                    {{ m.name.camelCase() }} = {{ m.name.camelCase() if m in current_legacy_members else (kotlin_default(m) if kotlin_default(m) is not none else "null") }},
                {% endfor %}
                {% for child in stable_chain_children %}
                    {{ child.name.camelCase() }} = {{ child.name.camelCase() if child in current_legacy_chain_children else "null" }},
                {% endfor %}
            )
        {%- endfor %}
    {%- endif %}
{% endmacro %}
//* Generates explicit `set<CallbackName>` methods.
//* Required for Java interop when multiple callbacks exist in a struct.
{% macro render_callbacks(members, callback_count) %}
    {% if callback_count > 1 %}
        {% for member in members %}
            {% if member.name.camelCase().endswith('Callback') %}
                {% set callback_name = member.name.camelCase() %}
                {% set executor_name = callback_name + 'Executor' %}
                {% set function_name = 'set' + member.name.CamelCase() %}
                public fun {{ function_name }}(
                    {{ executor_name }}: java.util.concurrent.Executor,
                    {{ callback_name }}: {{ kotlin_definition(member) }}
                ): Unit {
                    this.{{ executor_name }} = {{ executor_name }}
                    this.{{ callback_name }} = {{ callback_name }}
                }

            {% endif %}
        {% endfor %}
    {% endif %}
{% endmacro %}
//* Generates the Builder inner class for flexible initialization.
{% macro render_builder(structure, members, chain_children_list, default_count, include_optins, has_experimental, chain_children) %}
    /**
     * Builder for [{{ kotlin_name(structure) }}].
     */
    public class Builder(
        {% for member in members %}
            {% if kotlin_default(member) is none %}
                {% set optin_str = kotlin_member_optin(member, parent=structure, chain_children=chain_children) if include_optins else "" %}
                {{ kotlin_annotation(member) }}{{ " " ~ optin_str if optin_str else "" }} private val {{ member.name.camelCase() }}: {{ kotlin_declaration(member) }}{{ "," if not loop.last or default_count > 0 }}
            {% endif %}
        {% endfor %}
    ) {
        {% for member in members %}
            {% if kotlin_default(member) is not none %}
                {% set optin_str = kotlin_member_optin(member, parent=structure, chain_children=chain_children) if include_optins else "" %}
                {{ kotlin_annotation(member) }}{{ " " ~ optin_str if optin_str else "" }} private var {{ member.name.camelCase() }}: {{ kotlin_definition(member) }}
            {% endif %}
        {% endfor %}
        {% for child in chain_children_list %}
            {% set optin_str = kotlin_member_optin(child, parent=structure, chain_children=chain_children) if include_optins else "" %}
            {{ optin_str ~ " " if optin_str else "" }}private var {{ child.name.camelCase() }}: {{ kotlin_name(child) }}? = null
        {% endfor %}

        {% for member in members %}
            {% if kotlin_default(member) is not none %}
                {% set optin_str = kotlin_member_optin(member, parent=structure, chain_children=chain_children) if include_optins else "" %}
                {{ optin_str ~ " " if optin_str else "" }}public fun set{{ member.name.CamelCase() }}({{ kotlin_annotation(member) }} {{ member.name.camelCase() }}: {{ kotlin_declaration(member) }}): Builder = apply {
                    this.{{ member.name.camelCase() }} = {{ member.name.camelCase() }}
                }
            {% endif %}
        {% endfor %}
        {% for child in chain_children_list %}
            {% set optin_str = kotlin_member_optin(child, parent=structure, chain_children=chain_children) if include_optins else "" %}
            {{ optin_str ~ " " if optin_str else "" }}public fun set{{ child.name.CamelCase() }}({{ child.name.camelCase() }}: {{ kotlin_name(child) }}?): Builder = apply {
                this.{{ child.name.camelCase() }} = {{ child.name.camelCase() }}
            }
        {% endfor %}

        /**
         * Builds the [{{ kotlin_name(structure) }}].
         */
        {% if include_optins and has_experimental %}
            @OptIn(ExperimentalWebGpuApi::class)
        {% endif %}
        public fun build(): {{ kotlin_name(structure) }} = {{ kotlin_name(structure) }}(
            {% for member in members %}
                {{ member.name.camelCase() }} = {{ member.name.camelCase() }},
            {% endfor %}
            {% for child in chain_children_list %}
                {{ child.name.camelCase() }} = {{ child.name.camelCase() }},
            {% endfor %}
        )
    }
{% endmacro %}
{%- if not has_experimental -%}
    public class {{ kotlin_name(structure) }}(
        {% for member in members %}
            //* Generating KDocs
            {% set member_doc = struct_info.members.get(member.name.get(), "") if struct_info and struct_info.members else "" %}
            {% if member_doc %}
                {{ generate_simple_kdoc(member_doc, indent_prefix = "    ", line_wrap_prefix = '\n     * ') }}
            {% endif %}
            {% if member.type.name.get() == "bool" %}{{ "    " }}@get:JvmName("is{{ member.name.CamelCase() }}") {% else %}{{ "    " }}{% endif %}{{ kotlin_annotation(member) }} public var {{ member.name.camelCase() }}: {{ kotlin_definition(member) }},
        {% endfor %}
        {% for child in chain_children_list %}
            //* Generating KDocs
            {% set chain_struct_doc = all_structs_info.get(child.name.get()).doc if all_structs_info.get(child.name.get()) else "" %}
            {% if chain_struct_doc %}
                {{ generate_simple_kdoc(chain_struct_doc, indent_prefix = "    ", line_wrap_prefix = '\n     * ') }}
            {% endif %}
            public var {{ child.name.camelCase() }}: {{ kotlin_name(child) }}? = null,
        {% endfor %}
) {
{% set legacy_code = render_legacy_constructors(graduated_batches, stable_members, stable_chain_children, structure) -%}
{%- if legacy_code.strip() %}
    {{ legacy_code | indent(4, True) }}
{%- endif -%}
{% set callbacks_code = render_callbacks(members, ns.callback_count) -%}
{%- if callbacks_code.strip() %}
    {{ callbacks_code | indent(4, True) }}
{%- endif -%}
{% set builder_code = render_builder(structure, members, chain_children_list, ns.default_count, false, has_experimental, chain_children) -%}
{%- if builder_code.strip() %}
    {{ builder_code | indent(4, True) }}
{%- endif -%}
}
{%- else -%}
    public class {{ kotlin_name(structure) }} {
        {% for member in members %}
            //* Generating KDocs
            {% set member_doc = struct_info.members.get(member.name.get(), "") if struct_info and struct_info.members else "" %}
            {% if member_doc %}
                {{ generate_simple_kdoc(member_doc, indent_prefix = "        ", line_wrap_prefix = '\n         * ') }}
            {% endif %}
            {{ kotlin_property(member, parent=structure) }}
        {% endfor %}
        {% for child in chain_children_list %}
            //* Generating KDocs
            {% set chain_struct_doc = all_structs_info.get(child.name.get()).doc if all_structs_info.get(child.name.get()) else "" %}
            {% if chain_struct_doc %}
                {{ generate_simple_kdoc(chain_struct_doc, indent_prefix = "        ", line_wrap_prefix = '\n         * ') }}
            {% endif %}
            {{ kotlin_chain_property(child, parent=structure) }}
        {% endfor %}

        {% set stable_params = [] -%}
        {%- for member in stable_members -%}
            {%- do stable_params.append(kotlin_parameter(member, parent=structure)) -%}
        {%- endfor -%}
        {%- for child in stable_chain_children -%}
            {%- do stable_params.append(kotlin_chain_parameter(child, parent=structure)) -%}
        {%- endfor -%}

        {%- set all_stable_have_defaults = namespace(val=true) -%}
        {%- for member in stable_members -%}
            {%- if kotlin_default(member) is none -%}
                {%- set all_stable_have_defaults.val = false -%}
                {%- break -%}
            {%- endif -%}
        {% endfor %}
        {%- if all_stable_have_defaults.val %}
            //* Explicit no-args constructor to maintain Java binary compatibility.
            //* Kotlin does not automatically generate one for secondary constructors.
            public constructor() {
                {% for member in stable_members %}
                    this.{{ member.name.camelCase() }} = {{ kotlin_default(member) }}
                {% endfor %}
                {% for child in stable_chain_children %}
                    this.{{ child.name.camelCase() }} = null
                {% endfor %}
            }
        {% endif %}

        public constructor(
            {% for param in stable_params %}
                {{ param }}{{ "," if not loop.last }}
            {% endfor %}
        ) {
            {% for member in stable_members %}
                this.{{ member.name.camelCase() }} = {{ member.name.camelCase() }}
            {% endfor %}
            {% for child in stable_chain_children %}
                this.{{ child.name.camelCase() }} = {{ child.name.camelCase() }}
            {% endfor %}
        }
{% set legacy_code = render_legacy_constructors(graduated_batches, stable_members, stable_chain_children, structure) -%}
{%- if legacy_code.strip() %}
    {{ legacy_code | indent(4, True) }}
{%- endif -%}
        {% if experimental_members or experimental_chain_children %}
            {% set exp_params = [] -%}
            {%- for member in members -%}
                {%- set is_exp = member in experimental_members -%}
                {%- do exp_params.append(kotlin_parameter(member, parent=structure, include_default=(not is_exp))) -%}
            {%- endfor -%}
            {%- for child in chain_children_list -%}
                {%- set is_exp = child in experimental_chain_children -%}
                {%- do exp_params.append(kotlin_chain_parameter(child, parent=structure, include_default=(not is_exp))) -%}
            {%- endfor %}

            // TODO: When stabilizing, fold the experimental parameters into the stable constructor. The previous stable constructor should be marked with @Deprecated(level = DeprecationLevel.HIDDEN) to maintain backward compatibility.
            @ExperimentalWebGpuApi
            public constructor(
                {% for param in exp_params %}
                    {{ param }}{{ "," if not loop.last }}
                {% endfor %}
            ) {
                {% for member in members %}
                    this.{{ member.name.camelCase() }} = {{ member.name.camelCase() }}
                {% endfor %}
                {% for child in chain_children_list %}
                    this.{{ child.name.camelCase() }} = {{ child.name.camelCase() }}
                {% endfor %}
            }
        {% endif %}
{% set callbacks_code = render_callbacks(members, ns.callback_count) -%}
{% if callbacks_code.strip() %}

    {{ callbacks_code | indent(8, True) }}
{%- endif -%}
{% set builder_code = render_builder(structure, members, chain_children_list, ns.default_count, true, has_experimental, chain_children) -%}
{% if builder_code.strip() %}

    {{ builder_code | indent(4, True) }}
{%- endif -%}
}
{%- endif -%}