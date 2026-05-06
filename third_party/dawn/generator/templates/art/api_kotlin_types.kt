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

{%- macro kotlin_declaration(arg, strip_optional = False) -%}
    {%- set type = arg.type %}
    {%- set optional = arg.optional and not strip_optional %}
    {%- set default_value = arg.default_value %}
    {%- if type.category == 'kotlin type' -%}
        {{ type.name.get() }}  {#- The name *is* the type #}
    {%- elif arg.type.name.get() == 'string view' -%}
        String{{ '?' if optional }}
    {%- elif type.name.get() == 'void' %}
        {{- assert(arg.length and arg.constant_length != 1) -}}  {# void with length is binary data #}
        java.nio.ByteBuffer
    {%- elif arg.length and arg.length != 'constant' %}
        {# * annotation can mean an array, e.g. an output argument #}
        {%- if type.category in ['callback function', 'callback info', 'function pointer', 'object', 'structure'] -%}
            Array<{{ kotlin_name(type) }}>
        {%- elif type.category in ['bitmask', 'enum'] or type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            IntArray
        {%- else -%}
            {{ unreachable_code() }}
        {% endif %}
    {%- elif type.category in ['callback function', 'function pointer', 'object'] %}
        {{- kotlin_name(type) }}
        {%- if optional or default_value %}?{% endif %}
    {%- elif type.category == 'structure' or type.category == 'callback info' %}
        {{- kotlin_name(type) }}{{ '?' if optional }}
    {%- elif type.category in ['bitmask', 'enum'] -%}
        Int
    {%- elif type.name.get() == 'bool' -%}
        Boolean{{ '?' if optional }}
    {%- elif type.name.get() in ['void *', 'void const *'] %}
        //* Hack: void* for a return value is a ByteBuffer.
        {% if not arg.name %}
            ByteBuffer
        {% else %}
            Long
        {% endif %}
    {%- elif type.category == 'native' -%}
        {%- if type.name.get() == 'float' -%}
            Float
        {%- elif type.name.get() == 'double' -%}
            Double
        {%- elif type.name.get() in ['int8_t', 'uint8_t'] -%}
            Byte
        {%- elif type.name.get() in ['int16_t', 'uint16_t'] -%}
            Short
        {%- elif type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            Int
        {%- elif type.name.get() in ['int64_t', 'uint64_t', 'size_t'] -%}
            Long
        {%- else -%}
            {{ unreachable_code('Unsupported native type: ' + type.name.get()) }}
        {%- endif -%}
        {%- if optional -%}
            {{ unreachable_code('Optional natives not supported: ' + type.name.get()) }}
        {%- endif -%}
    {%- else -%}
        {{ unreachable_code('Unsupported type: ' + type.name.get()) }}
    {%- endif %}
{% endmacro %}

{% macro kotlin_definition(arg) -%}
    {{- kotlin_declaration(arg) -}}
    {%- if kotlin_default(arg) is not none %} = {{ kotlin_default(arg) }}{% endif -%}
{%- endmacro %}

{% macro kotlin_annotation(arg) -%}
    {%- if arg != None -%}
        {% set type = arg.type %}
        {% if type.category in ['bitmask', 'enum'] -%}
            @{{ type.name.CamelCase() -}}.Type
        {% endif -%}
    {% endif -%}
{% endmacro %}

{% macro do_all_args_have_doc(all_arg_docs, code_args) %}
    //* Ensure inputs are not None
    {% set all_arg_docs = all_arg_docs if all_arg_docs else {} %}
    {% set code_args = code_args if code_args else {} %}
    {% if not code_args %}
        {{- false -}}
    {% else %}
        {% set all_found = namespace(value=true) %}
        {% for arg in code_args %}
            //* Check for docs under both camelCase and snake_case versions of the arg name
            {% set camel_name = as_varName(arg.name) %}
            {% set snake_name = arg.name.snake_case() %}
            {% set arg_name = camel_name if camel_name in all_arg_docs else (snake_name if snake_name in all_arg_docs else None) %}
            {% set arg_doc = all_arg_docs.get(arg_name) | trim if arg_name else "" %}
            {% if not arg_doc %}
                //* If docs are not found for any arg, set all_found to false and exit early
                {% set all_found.value = false %}
                {% break %}
            {% endif %}
        {% endfor %}
        {{- all_found.value -}}
    {% endif %}
{% endmacro %}

{% macro check_if_doc_present(doc_str, return_str, all_arg_docs, code_args) %}
    {% set all_arg_docs = all_arg_docs if all_arg_docs else {} %}
    {% set code_args = code_args if code_args else {} %}
    {% if (doc_str | trim) or (return_str | trim) %}
        {{- true -}}
    {% else %}
        //* Call the generalized helper for arg docs
        {{- do_all_args_have_doc(all_arg_docs, code_args) -}}
    {% endif %}
{% endmacro %}

{%- macro generate_kdoc(main_doc, return_doc, arg_docs_map, function_args, line_wrap_prefix, indent_prefix = "") -%}
    {%- set main_doc = main_doc -%}
    {%- set return_doc = return_doc -%}
    {%- set line_wrap_prefix = line_wrap_prefix -%}
    {%- set arg_docs_map = arg_docs_map if arg_docs_map else {} -%}
    {%- set indent_prefix = indent_prefix %}
    //* Check if all of the class's arguments have doc available in the map.
    {%- set all_args_have_doc = do_all_args_have_doc(arg_docs_map, function_args) -%}
    {{indent_prefix}}/**
    {%- if main_doc %}

        {{indent_prefix}} * {{ main_doc | trim | wordwrap(90, break_long_words=False, break_on_hyphens=False, wrapstring = line_wrap_prefix) }}
    {%- endif %}
    //* Iterate over each argument in the function's signature to generate its @param tag.
    {%- for arg in function_args %}
        //* To find the doc, we need to check for the argument's name in the docs map. The name in the map might be camelCase or snake_case, so we try both.
        {%- set arg_name_camel_case = as_varName(arg.name) -%}
        {%- set arg_name_snake_case = arg.name.snake_case() -%}
        {%- set arg_doc_lookup_key = arg_name_camel_case if arg_name_camel_case in arg_docs_map else (arg_name_snake_case if arg_name_snake_case in arg_docs_map else None) -%}
        {%- set current_arg_doc = arg_docs_map.get(arg_doc_lookup_key) | trim if arg_doc_lookup_key else "" -%}
        //* Only add the @param tag if doc was found for this specific argument, and if all arguments have doc overall (to keep the block clean).
        {%- if all_args_have_doc == 'True' and current_arg_doc %}

            {{indent_prefix}} * @param {{ as_varName(arg.name) + " " +  current_arg_doc | wordwrap(90, break_long_words=False, break_on_hyphens=False, wrapstring = line_wrap_prefix) }}
        {%- endif %}
    {%- endfor %}
    {%- if return_doc %}

        {{indent_prefix}} * @return {{ return_doc | trim | wordwrap(90, break_long_words=False, break_on_hyphens=False, wrapstring = line_wrap_prefix) }}
    {%- endif %}

    {{indent_prefix}} */
{% endmacro -%}

{% macro generate_simple_kdoc(doc_str, indent_prefix = "", line_wrap_prefix = "\n * ") %}
    /**
    {{indent_prefix}} * {{ doc_str | wordwrap(80, break_long_words=False, break_on_hyphens=False, wrapstring = line_wrap_prefix) }}
    {{indent_prefix}} */
{%- endmacro -%}

{% macro add_kdoc_disclaimer(indent_prefix = "") %}
    {{indent_prefix}}/*
    {{indent_prefix}} * Note: The KDocs in this file was generated by Google Gemini AI.
    {{indent_prefix}} * For the definitive source of truth, please refer to the source code and the official
    {{indent_prefix}} * WebGPU JS Specification (https://www.w3.org/TR/webgpu/).
    {{indent_prefix}} * If you find any inaccuracies, please file an issue with the Google Dawn team at
    {{indent_prefix}} * https://issuetracker.google.com/issues/new?component=1960262
    {{indent_prefix}} */
{%- endmacro -%}
