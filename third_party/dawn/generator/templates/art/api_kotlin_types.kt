//* Copyright 2024 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{%- macro declaration_with_defaults(arg, emit_defaults) -%}
    {%- set type = arg.type %}
    {%- set optional = arg.optional %}
    {%- set default_value = arg.default_value %}
    {%- if arg.type.name.get() == 'string view' -%}
        String{{ '?' if optional }}
        {%- if emit_defaults and optional-%}
            {{ ' ' }}= null
        {%- endif %}
    {% elif type.name.get() == 'void' %}
        {%- if arg.length and arg.constant_length != 1 -%}  {# void with length is binary data #}
            java.nio.ByteBuffer{{ ' = java.nio.ByteBuffer.allocateDirect(0)' if emit_defaults }}
        {%- else -%}
            Unit  {# raw void is C type for no return type; Kotlin equivalent is Unit #}
        {%- endif -%}
    {%- elif arg.length and arg.length != 'constant' %}
        {# * annotation can mean an array, e.g. an output argument #}
        {%- if type.category in ['bitmask', 'callback function', 'callback info', 'enum', 'function pointer', 'object', 'structure'] -%}
            Array<{{ type.name.CamelCase() }}>{{ ' = arrayOf()' if emit_defaults }}
        {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            IntArray{{ ' = intArrayOf()' if emit_defaults }}
        {%- else -%}
            {{ unreachable_code() }}
        {% endif %}
    {%- elif type.category in ['callback function', 'function pointer', 'object'] %}
        {{- type.name.CamelCase() }}
        {%- if optional or default_value %}?{{ ' = null' if emit_defaults }}{% endif %}
    {%- elif type.category == 'structure' or type.category == 'callback info' %}
        {{- type.name.CamelCase() }}{{ '?' if optional }}
        {%- if emit_defaults -%}
            {%- if type.has_basic_constructor -%}
                {{ ' ' }}= {{ type.name.CamelCase() }}()
            {%- elif optional -%}
                {{ ' ' }}= null
            {%- endif %}
        {%- endif %}
    {%- elif type.category in ['bitmask', 'enum'] -%}
        {{ type.name.CamelCase() }}
        {%- if default_value %}
            {%- for value in type.values if value.name.name == default_value -%}
                {{ ' ' }}= {{ type.name.CamelCase() }}.{{ as_ktName(value.name.CamelCase()) }}
            {%- endfor %}
        {%- endif %}
    {%- elif type.name.get() == 'bool' -%}
        Boolean{{ '?' if optional }}{% if default_value %} = {{ default_value }}{% endif %}
    {%- elif type.name.get() in ['void *', 'void const *'] %}
        //* Hack: void* for a return value is a ByteBuffer.
        {% if not arg.name %}
            ByteBuffer
        {% else %}
            Long
        {% endif %}
    {%- elif type.category == 'native' -%}
        {%- set ns = namespace(type_name='', default_value=default_value) -%}
        {%- if type.name.get() == 'float' -%}
            {%- set ns.type_name = 'Float' -%}
        {%- elif type.name.get() == 'double' -%}
            {%- set ns.type_name = 'Double' -%}
        {%- elif type.name.get() in ['int8_t', 'uint8_t'] -%}
            {%- set ns.type_name = 'Byte' -%}
        {%- elif type.name.get() in ['int16_t', 'uint16_t'] -%}
            {%- set ns.type_name = 'Short' -%}
        {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            {%- set ns.type_name = 'Int' -%}
            {%- if ns.default_value == '0xFFFFFFFF' -%}
                {%- set ns.default_value = '-0x7FFFFFFF' -%}
            {%- endif -%}
        {%- elif type.name.get() in ['int64_t', 'uint64_t', 'size_t'] -%}
            {%- set ns.type_name = 'Long' -%}
            {%- if ns.default_value == '0xFFFFFFFFFFFFFFFF' -%}
                {%- set ns.default_value = '-0x7FFFFFFFFFFFFFFF' -%}
            {%- endif -%}
        {%- else -%}
            {{ unreachable_code('Unsupported native type: ' + type.name.get()) }}
        {%- endif -%}
        {%- if optional -%}
            {%- set ns.type_name = ns.type_name + '?' -%}
        {%- endif -%}
        {{ ns.type_name }}
        {%- if ns.default_value not in [None, undefined] -%} {{ ' ' }}={{ ' ' }}
            {%- set constant = find_by_name(by_category["constant"], ns.default_value) -%}
            {%- if constant -%}
                Constants.{{ as_ktName(constant.name.SNAKE_CASE()) }}
            {%- else -%}
                {{ ns.default_value }}
            {%- endif -%}
        {%- endif -%}
    {%- else -%}
        {{ unreachable_code('Unsupported type: ' + type.name.get()) }}
    {%- endif %}
{% endmacro %}

{% macro kotlin_definition(arg) -%}
    {{ declaration_with_defaults(arg, true) }}
{%- endmacro %}

{% macro kotlin_declaration(arg) -%}
    {{ declaration_with_defaults(arg, false) }}
{%- endmacro %}

