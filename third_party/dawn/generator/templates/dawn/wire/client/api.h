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
//*
//*
{% set API = metadata.c_prefix %}
{% set api = metadata.api.lower() %}
#ifndef DAWN_WIRE_CLIENT_{{metadata.api.upper()}}_H_
#define DAWN_WIRE_CLIENT_{{metadata.api.upper()}}_H_

#include "{{api}}/{{api}}.h"
#include "dawn/wire/dawn_wire_export.h"

#ifdef __cplusplus
extern "C" {
#endif

{% for function in by_category["function"] %}
    DAWN_WIRE_EXPORT {{as_cType(function.return_type.name)}} {{as_cMethodNamespaced(None, function.name, Name('dawn wire client'))}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif -%}
                {%- if arg.optional %}{{API}}_NULLABLE {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
{% endfor %}

{% for type in by_category["object"] if len(c_methods(type)) > 0 %}
    // Methods of {{type.name.CamelCase()}}
    {% for method in c_methods(type) %}
        DAWN_WIRE_EXPORT {{as_cType(method.return_type.name)}} {{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                ,{{" "}}
                {%- if arg.optional %}{{API}}_NULLABLE {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {{API}}_FUNCTION_ATTRIBUTE;
    {% endfor %}

{% endfor %}

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // DAWN_WIRE_CLIENT_{{metadata.api.upper()}}_H_
