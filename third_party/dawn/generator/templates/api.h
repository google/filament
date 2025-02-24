//* Copyright 2020 The Dawn & Tint Authors
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
//*
//*
{% include 'BSD_LICENSE' %}

{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
    #endif
{% endif %}

#ifndef {{metadata.api.upper()}}_H_
#define {{metadata.api.upper()}}_H_

#define WGPU_BREAKING_CHANGE_STRING_VIEW_LABELS
#define WGPU_BREAKING_CHANGE_STRING_VIEW_OUTPUT_STRUCTS
#define WGPU_BREAKING_CHANGE_STRING_VIEW_CALLBACKS
#define WGPU_BREAKING_CHANGE_FUTURE_CALLBACK_TYPES
#define WGPU_BREAKING_CHANGE_LOGGING_CALLBACK_TYPE

{% set API = metadata.c_prefix %}
#if defined({{API}}_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined({{API}}_IMPLEMENTATION)
#            define {{API}}_EXPORT __declspec(dllexport)
#        else
#            define {{API}}_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined({{API}}_IMPLEMENTATION)
#            define {{API}}_EXPORT __attribute__((visibility("default")))
#        else
#            define {{API}}_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined({{API}}_SHARED_LIBRARY)
#    define {{API}}_EXPORT
#endif  // defined({{API}}_SHARED_LIBRARY)

#if !defined({{API}}_OBJECT_ATTRIBUTE)
#define {{API}}_OBJECT_ATTRIBUTE
#endif
#if !defined({{API}}_ENUM_ATTRIBUTE)
#define {{API}}_ENUM_ATTRIBUTE
#endif
#if !defined({{API}}_STRUCTURE_ATTRIBUTE)
#define {{API}}_STRUCTURE_ATTRIBUTE
#endif
#if !defined({{API}}_FUNCTION_ATTRIBUTE)
#define {{API}}_FUNCTION_ATTRIBUTE
#endif
#if !defined({{API}}_NULLABLE)
#define {{API}}_NULLABLE
#endif

#define WGPU_BREAKING_CHANGE_DROP_DESCRIPTOR

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
#  if __cplusplus >= 201103L
#    define {{API}}_MAKE_INIT_STRUCT(type, value) (type value)
#  else
#    define {{API}}_MAKE_INIT_STRUCT(type, value) value
#  endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define {{API}}_MAKE_INIT_STRUCT(type, value) ((type) value)
#else
#  define {{API}}_MAKE_INIT_STRUCT(type, value) value
#endif

{% for constant in by_category["constant"] %}
    #define {{API}}_{{constant.name.SNAKE_CASE()}} {{constant.value}}
{% endfor %}

typedef uint64_t {{API}}Flags;
typedef uint32_t {{API}}Bool;

{% for type in by_category["object"] %}
    typedef struct {{as_cType(type.name)}}Impl* {{as_cType(type.name)}} {{API}}_OBJECT_ATTRIBUTE;
{% endfor %}

// Structure forward declarations
{% for type in by_category["structure"] %}
    struct {{as_cType(type.name)}};
{% endfor %}

{% for type in by_category["enum"] %}
    typedef enum {{as_cType(type.name)}} {
        {% for value in type.values %}
            {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "08X")}},
        {% endfor %}
        {{as_cEnum(type.name, Name("force32"))}} = 0x7FFFFFFF
    } {{as_cType(type.name)}} {{API}}_ENUM_ATTRIBUTE;
{% endfor %}

{% for type in by_category["bitmask"] %}
    typedef {{API}}Flags {{as_cType(type.name)}};
    {% for value in type.values %}
        static const {{as_cType(type.name)}} {{as_cEnum(type.name, value.name)}} = 0x{{format(value.value, "016X")}};
    {% endfor %}
{% endfor -%}

{% for type in by_category["function pointer"] %}
    typedef {{as_cType(type.return_type.name)}} (*{{as_cType(type.name)}})(
        {%- if type.arguments == [] -%}
            void
        {%- else -%}
            {%- for arg in type.arguments -%}
                {% if not loop.first %}, {% endif %}
                {% if arg.type.category == "structure" %}struct {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        {%- endif -%}
    ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

// Callback function pointers
{% for type in by_category["callback function"] %}
    typedef {{as_cType(type.return_type.name)}} (*{{as_cType(type.name)}})(
        {%- for arg in type.arguments -%}
            {% if arg.type.category == "structure" %}struct {% endif %}{{as_annotated_cType(arg)}}{{", "}}
        {%- endfor -%}
    void* userdata1, void* userdata2) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

typedef struct {{API}}ChainedStruct {
    struct {{API}}ChainedStruct * next;
    {{API}}SType sType;
} {{API}}ChainedStruct {{API}}_STRUCTURE_ATTRIBUTE;

{% macro render_c_default_value(member) -%}
    {%- if member.annotation in ["*", "const*"] and member.optional or member.default_value == "nullptr" -%}
        NULL
    {%- elif member.type.category == "object" and member.optional -%}
        NULL
    {%- elif member.type.category == "callback function" -%}
        NULL
    {%- elif member.type.category in ["enum", "bitmask"] and member.default_value != None -%}
        {{as_cEnum(member.type.name, Name(member.default_value))}}
    {%- elif member.default_value != None -%}
        {{member.default_value}}
    {%- elif member.type.category == "structure" and member.annotation == "value" -%}
        {{API}}_{{member.type.name.SNAKE_CASE()}}_INIT
    {%- else -%}
        {{- assert(member.json_data.get("no_default", false) == false) -}}
        {{- assert(member.default_value == None) -}}
        {}
    {%- endif -%}
{% endmacro %}
{% macro nullable_annotation(record) -%}
    {% if record.optional and (record.type.category == "object" or record.annotation != "value") -%}
        {{API}}_NULLABLE{{" "}}
    {%- endif %}
{%- endmacro %}

#define {{API}}_COMMA ,

{% for type in by_category["callback info"] %}
    typedef struct {{as_cType(type.name)}} {
        {{API}}ChainedStruct* nextInChain;
        {% for member in type.members %}
            {{as_annotated_cType(member)}};
        {% endfor %}
        void* userdata1;
        void* userdata2;
    } {{as_cType(type.name)}} {{API}}_STRUCTURE_ATTRIBUTE;

    #define {{API}}_{{type.name.SNAKE_CASE()}}_INIT {{API}}_MAKE_INIT_STRUCT({{as_cType(type.name)}}, { \
        /*.nextInChain=*/NULL {{API}}_COMMA \
        {% for member in type.members %}
            /*.{{as_varName(member.name)}}=*/{{render_c_default_value(member)}} {{API}}_COMMA \
        {% endfor %}
        /*.userdata1=*/NULL {{API}}_COMMA \
        /*.userdata2=*/NULL {{API}}_COMMA \
    })

{% endfor %}

{% for type in by_category["structure"] %}
    {% for root in type.chain_roots %}
        // Can be chained in {{as_cType(root.name)}}
    {% endfor %}
    typedef struct {{as_cType(type.name)}} {
        {% if type.extensible %}
            {{API}}ChainedStruct* nextInChain;
        {% endif %}
        {% if type.chained %}
            {{API}}ChainedStruct chain;
        {% endif %}
        {% for member in type.members %}
            {{nullable_annotation(member)}}{{as_annotated_cType(member)}};
        {% endfor %}
    } {{as_cType(type.name)}} {{API}}_STRUCTURE_ATTRIBUTE;

    #define {{API}}_{{type.name.SNAKE_CASE()}}_INIT {{API}}_MAKE_INIT_STRUCT({{as_cType(type.name)}}, { \
        {% if type.extensible %}
            /*.nextInChain=*/NULL {{API}}_COMMA \
        {% endif %}
        {% if type.chained %}
            /*.chain=*/{/*.nextInChain*/NULL {{API}}_COMMA /*.sType*/{{API}}SType_{{type.name.CamelCase()}}} {{API}}_COMMA \
        {% endif %}
        {% for member in type.members %}
            /*.{{as_varName(member.name)}}=*/{{render_c_default_value(member)}} {{API}}_COMMA \
        {% endfor %}
    })

{% endfor %}
{% for typeDef in by_category["typedef"] %}
    // {{as_cType(typeDef.name)}} is deprecated.
    // Use {{as_cType(typeDef.type.name)}} instead.
    typedef {{as_cType(typeDef.type.name)}} {{as_cType(typeDef.name)}};

{% endfor %}
#ifdef __cplusplus
extern "C" {
#endif

#if !defined({{API}}_SKIP_PROCS)

// TODO(374150686): Remove these Emscripten specific declarations from the
// header once they are fully deprecated.
#ifdef __EMSCRIPTEN__
{{API}}_EXPORT WGPUDevice emscripten_webgpu_get_device(void);
#endif

{% for function in by_category["function"] %}
    typedef {{as_cType(function.return_type.name)}} (*{{as_cProc(None, function.name)}})(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}
                {{nullable_annotation(arg)}}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Procs of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        typedef {{as_cType(method.return_type.name)}} (*{{as_cProc(type.name, method.name)}})(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{nullable_annotation(arg)}}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
    {% endfor %}

{% endfor %}

#endif  // !defined({{API}}_SKIP_PROCS)

#if !defined({{API}}_SKIP_DECLARATIONS)

{% for function in by_category["function"] %}
    {{API}}_EXPORT {{as_cType(function.return_type.name)}} {{as_cMethod(None, function.name)}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif -%}
                {{nullable_annotation(arg)}}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        {{API}}_EXPORT {{as_cType(method.return_type.name)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{nullable_annotation(arg)}}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
    {% endfor %}

{% endfor %}

#endif  // !defined({{API}}_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // {{metadata.api.upper()}}_H_
