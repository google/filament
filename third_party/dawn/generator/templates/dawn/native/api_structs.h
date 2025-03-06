//* Copyright 2017 The Dawn & Tint Authors
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
{% from 'dawn/cpp_macros.tmpl' import wgpu_string_members with context %}

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
{% set namespace = metadata.namespace %}
#ifndef {{DIR}}_{{namespace.upper()}}_STRUCTS_H_
#define {{DIR}}_{{namespace.upper()}}_STRUCTS_H_

#include "absl/strings/string_view.h"
{% set api = metadata.api.lower() %}
{% set CAPI = metadata.c_prefix %}
#include "dawn/{{api}}_cpp.h"
{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/Forward.h"

#include <cmath>
#include <optional>
#include <string_view>

namespace {{native_namespace}} {

{% macro render_cpp_default_value(member, forced_default_value="") -%}
    {%- if forced_default_value -%}
        {{" "}}= {{forced_default_value}}
    {%- elif member.annotation in ["*", "const*"] and member.optional or member.default_value == "nullptr" -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "object" and member.optional -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "callback info" -%}
        {{" "}}= {{CAPI}}_{{member.name.SNAKE_CASE()}}_INIT
    {%- elif member.type.category in ["enum", "bitmask"] and member.default_value != None -%}
        {{" "}}= {{namespace}}::{{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
    {%- elif member.type.category == "native" and member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- elif member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- else -%}
        {{assert(member.default_value == None)}}
    {%- endif -%}
{%- endmacro %}

    using {{namespace}}::ChainedStruct;
    using {{namespace}}::ChainedStructOut;

    //* Special structures that are manually written.
    {% set SpecialStructures = ["string view"] %}

    struct StringView {
        char const * data = nullptr;
        size_t length = WGPU_STRLEN;

        {{wgpu_string_members("StringView")}}

        // Equality operators, mostly for testing. Note that this tests
        // strict pointer-pointer equality if the struct contains member pointers.
        bool operator==(const StringView& rhs) const;

        #ifndef ABSL_USES_STD_STRING_VIEW
        // NOLINTNEXTLINE(runtime/explicit) allow implicit conversion
        operator absl::string_view() const {
            if (this->length == wgpu::kStrlen) {
                if (IsUndefined()) {
                    return {};
                }
                return {this->data};
            }
            return {this->data, this->length};
        }
        #endif
    };

    {% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
        {% set CppType = as_cppType(type.name) %}
        {% if type.chained %}
            {% set chainedStructType = "ChainedStructOut" if type.chained == "out" else "ChainedStruct" %}
            struct {{CppType}} : {{chainedStructType}} {
                {{CppType}}() {
                    sType = {{namespace}}::SType::{{type.name.CamelCase()}};
                }
        {% else %}
            struct {{CppType}} {
                {% if type.has_free_members_function %}
                    {{CppType}}() = default;
                {% endif %}
        {% endif %}
            {% if type.has_free_members_function %}
                ~{{CppType}}();
                {{CppType}}(const {{CppType}}&) = delete;
                {{CppType}}& operator=(const {{CppType}}&) = delete;
                {{CppType}}({{CppType}}&&);
                {{CppType}}& operator=({{CppType}}&&);

            {% endif %}
            {% if type.extensible %}
                {% set chainedStructType = "ChainedStructOut" if type.output else "ChainedStruct const" %}
                {{chainedStructType}} * nextInChain = nullptr;
            {% endif %}
            {% for member in type.members %}
                {% if type.name.get() == "bind group layout entry" %}
                    {% if member.name.canonical_case() == "buffer" %}
                        {% set forced_default_value = "{ nullptr, wgpu::BufferBindingType::BindingNotUsed, false, 0 }" %}
                    {% elif member.name.canonical_case() == "sampler" %}
                        {% set forced_default_value = "{ nullptr, wgpu::SamplerBindingType::BindingNotUsed }" %}
                    {% elif member.name.canonical_case() == "texture" %}
                        {% set forced_default_value = "{ nullptr, wgpu::TextureSampleType::BindingNotUsed, wgpu::TextureViewDimension::e2D, false }" %}
                    {% elif member.name.canonical_case() == "storage texture" %}
                        {% set forced_default_value = "{ nullptr, wgpu::StorageTextureAccess::BindingNotUsed, wgpu::TextureFormat::Undefined, wgpu::TextureViewDimension::e2D }" %}
                    {% endif %}
                {% endif %}
                {% set member_declaration = as_annotated_frontendType(member) + render_cpp_default_value(member, forced_default_value) %}
                {% if type.chained and loop.first %}
                    //* Align the first member after ChainedStruct to match the C struct layout.
                    //* It has to be aligned both to its natural and ChainedStruct's alignment.
                    alignas({{namespace}}::{{CppType}}::kFirstMemberAlignment) {{member_declaration}};
                {% else %}
                    {{member_declaration}};
                {% endif %}
            {% endfor %}

            {% if type.any_member_requires_struct_defaulting %}
                // This method makes a copy of the struct, then, for any enum members with trivial
                // defaulting (where something like "Undefined" is replaced with a default), applies
                // all of the defaults for the struct, and recursively its by-value substructs (but
                // NOT by-pointer substructs since they are const*). It must be called in an
                // appropriate place in Dawn.
                [[nodiscard]] {{CppType}} WithTrivialFrontendDefaults() const;
            {% endif %}
            // Equality operators, mostly for testing. Note that this tests
            // strict pointer-pointer equality if the struct contains member pointers.
            bool operator==(const {{CppType}}& rhs) const;

            {% if type.has_free_members_function %}
              private:
                inline void FreeMembers();
            {% endif %}
        };

    {% endfor %}

    {% for typeDef in by_category["typedef"] if typeDef.type.category == "structure" %}
        using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        // {{as_cppType(type.name)}}
        void API{{as_MethodSuffix(type.name, Name("free members"))}}({{as_cType(type.name)}});
    {% endfor %}

} // namespace {{native_namespace}}

#endif  // {{DIR}}_{{namespace.upper()}}_STRUCTS_H_
