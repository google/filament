// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/{{metadata.api.lower()}}.h"

namespace dawn::native {

// This file should be kept in sync with generator/templates/dawn/native/ProcTable.cpp

{% for function in by_category["function"] %}
    extern {{as_cType(function.return_type.name)}} Native{{as_cppType(function.name)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
        {%- endfor -%}
    );
{% endfor %}
{% for type in by_category["object"] %}
    {% for method in c_methods(type) %}
        extern {{as_cType(method.return_type.name)}} Native{{as_MethodSuffix(type.name, method.name)}}(
            {{-as_cType(type.name)}} cSelf
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        );
    {% endfor %}
{% endfor %}

}

extern "C" {
    using namespace dawn::native;

    {% for function in by_category["function"] %}
        {{as_cType(function.return_type.name)}} {{metadata.namespace}}{{as_cppType(function.name)}} (
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            return Native{{as_cppType(function.name)}}(
                {%- for arg in function.arguments -%}
                    {% if not loop.first %}, {% endif %}{{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }
    {% endfor %}

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_cType(method.return_type.name)}} {{metadata.namespace}}{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                return Native{{as_MethodSuffix(type.name, method.name)}}(
                    cSelf
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}
    {% endfor %}
}
