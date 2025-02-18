//* Copyright 2018 The Dawn & Tint Authors
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

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set namespace = metadata.namespace %}
#include "{{native_dir}}/{{namespace}}_structs_autogen.h"

#include <cstring>
#include <tuple>

#include "dawn/common/Assert.h"

#if defined(__GNUC__) || defined(__clang__)
// error: 'offsetof' within non-standard-layout type '{{namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

namespace {{native_namespace}} {

    {% set c_prefix = metadata.c_prefix %}
    static_assert(sizeof(ChainedStruct) == sizeof({{c_prefix}}ChainedStruct),
            "sizeof mismatch for ChainedStruct");
    static_assert(alignof(ChainedStruct) == alignof({{c_prefix}}ChainedStruct),
            "alignof mismatch for ChainedStruct");
    static_assert(offsetof(ChainedStruct, nextInChain) == offsetof({{c_prefix}}ChainedStruct, next),
            "offsetof mismatch for ChainedStruct::nextInChain");
    static_assert(offsetof(ChainedStruct, sType) == offsetof({{c_prefix}}ChainedStruct, sType),
            "offsetof mismatch for ChainedStruct::sType");

    //* Special structures that are manually written.
    {% set SpecialStructures = ["string view"] %}

    bool StringView::operator==(const StringView& rhs) const {
        return data == rhs.data && length == rhs.length;
    }

    {% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
        {% set CppType = as_cppType(type.name) %}
        {% set CType = as_cType(type.name) %}

        static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
        static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

        {% if type.extensible %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                    "offsetof mismatch for {{CppType}}::nextInChain");
        {% endif %}
        {% if type.chained %}
            static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, chain) + offsetof(WGPUChainedStruct, next),
                    "offsetof mismatch for {{CppType}}::nextInChain");
            static_assert(offsetof({{CppType}}, sType) == offsetof({{CType}}, chain) + offsetof(WGPUChainedStruct, sType),
                    "offsetof mismatch for {{CppType}}::sType");
        {% endif %}
        {% for member in type.members %}
            {% set memberName = member.name.camelCase() %}
            static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                         "offsetof mismatch for {{CppType}}::{{memberName}}");
        {% endfor %}

        {% if type.any_member_requires_struct_defaulting %}
            {{CppType}} {{CppType}}::WithTrivialFrontendDefaults() const {
                {{CppType}} copy;
                {% if type.extensible %}
                    copy.nextInChain = nextInChain;
                {% endif %}
                {% if type.chained %}
                    copy.nextInChain = nextInChain;
                    copy.sType = sType;
                {% endif %}
                {% for member in type.members %}
                    {% set memberName = member.name.camelCase() %}
                    {% if member.requires_struct_defaulting %}
                        {% if member.type.category == "structure" %}
                            copy.{{memberName}} = {{memberName}}.WithTrivialFrontendDefaults();
                        {% elif member.type.category == "enum" %}
                            {% set Enum = namespace + "::" + as_cppType(member.type.name) %}
                            copy.{{memberName}} = ({{memberName}} == {{Enum}}::Undefined)
                                ? {{Enum}}::{{as_cppEnum(Name(member.default_value))}}
                                : {{memberName}};
                        {% else %}
                            {{assert(False, "other types do not currently support defaulting")}}
                        {% endif %}
                    {% else %}
                        copy.{{memberName}} = {{memberName}};
                    {% endif %}
                {% endfor %}
                return copy;
            }
        {% endif %}
        bool {{CppType}}::operator==(const {{CppType}}& rhs) const {
            return {% if type.extensible or type.chained -%}
                (nextInChain == rhs.nextInChain) &&
            {%- endif %} std::tie(
                {% for member in type.members if member.type.category != 'callback info' %}
                    {{member.name.camelCase()-}}
                    {{ "," if not loop.last else "" }}
                {% endfor %}
            ) == std::tie(
                {% for member in type.members if member.type.category != 'callback info' %}
                    rhs.{{member.name.camelCase()-}}
                    {{ "," if not loop.last else "" }}
                {% endfor %}
            );
        }

    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        // {{as_cppType(type.name)}}
        {{as_cppType(type.name)}}::~{{as_cppType(type.name)}}() {
            FreeMembers();
        }

        {{as_cppType(type.name)}}::{{as_cppType(type.name)}}({{as_cppType(type.name)}}&& rhs)
        : {% for member in type.members %}
            {%- set memberName = member.name.camelCase() -%}
            {{memberName}}(rhs.{{memberName}}){% if not loop.last %},{{"\n      "}}{% endif %}
        {% endfor -%}
        {
            {% for member in type.members %}
                rhs.{{member.name.camelCase()}} = {};
            {% endfor %}
        }

        {{as_cppType(type.name)}}& {{as_cppType(type.name)}}::operator=({{as_cppType(type.name)}}&& rhs) {
            if (&rhs == this) {
                return *this;
            }
            FreeMembers();
            {% for member in type.members %}
                this->{{member.name.camelCase()}} = std::move(rhs.{{member.name.camelCase()}});
            {% endfor %}
            {% for member in type.members %}
                rhs.{{member.name.camelCase()}} = {};
            {% endfor %}
            return *this;
        }

        void {{as_cppType(type.name)}}::FreeMembers() {
            bool needsFreeing = false;
            {%- for member in type.members if member.annotation != 'value' %}
                if (this->{{member.name.camelCase()}} != nullptr) { needsFreeing = true; }
            {%- endfor -%}
            {%- for member in type.members if member.type.name.canonical_case() == 'string view' %}
                if (this->{{member.name.camelCase()}}.data != nullptr) { needsFreeing = true; }
            {%- endfor -%}
            if (needsFreeing) {
                API{{as_MethodSuffix(type.name, Name("free members"))}}(*reinterpret_cast<{{as_cType(type.name)}}*>(this));
            }
        }

    {% endfor %}

} // namespace {{native_namespace}}
