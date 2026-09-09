// Copyright 2026 The Dawn & Tint Authors
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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set include_dir = namespace_name.Dirs() %}
{% set namespace = metadata.namespace %}
#include "{{include_dir}}/{{namespace}}_structs_defaults_autogen.h"

namespace {{native_namespace}} {

{% for type in by_category["structure"] %}
    {% set CppType = as_cppType(type.name) %}
    {% if type.any_member_requires_struct_defaulting %}
        {{CppType}} WithTrivialFrontendDefaults(const {{CppType}}& in) {
            {{CppType}} copy;
            {% if type.extensible %}
                copy.nextInChain = in.nextInChain;
            {% endif %}
            {% if type.chained %}
                copy.nextInChain = in.nextInChain;
                copy.sType = in.sType;
            {% endif %}
            {% for member in type.members %}
                {% set memberName = member.name.camelCase() %}
                {% if (member.spanify | default(True)) and member.is_length %}
                    //* Skip as the member is included in the span member.
                {% elif member.requires_struct_defaulting %}
                    {% if member.type.category == "structure" %}
                        copy.{{memberName}} = WithTrivialFrontendDefaults(in.{{memberName}});
                    {% elif member.type.category == "enum" %}
                        {% set Enum = namespace + "::" + as_cppType(member.type.name) %}
                        copy.{{memberName}} = (in.{{memberName}} == {{Enum}}::Undefined)
                            ? {{Enum}}::{{as_cppEnum(Name(member.default_value))}}
                            : in.{{memberName}};
                    {% else %}
                        {{assert(False, "other types do not currently support defaulting")}}
                    {% endif %}
                {% else %}
                    copy.{{memberName}} = in.{{memberName}};
                {% endif %}
            {% endfor %}
            return copy;
        }

    {% endif %}
{% endfor %}
} // namespace {{native_namespace}}
