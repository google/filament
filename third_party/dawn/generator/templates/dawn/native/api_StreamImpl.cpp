//* Copyright 2022 The Dawn & Tint Authors
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
{% set prefix = metadata.proc_table_prefix.lower() %}
#include "{{native_dir}}/CacheKey.h"
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/{{metadata.namespace}}_structs_autogen.h"

#include <cstring>

namespace {{native_namespace}} {

//*
//* Streaming readers for wgpu structures.
//*
{% macro render_reader(member) %}
    {%- set name = member.name.camelCase() -%}
    DAWN_TRY(StreamOut(source, &t->{{name}}));
{% endmacro %}

//*
//* Streaming writers for wgpu structures.
//*
{% macro render_writer(member) %}
    {%- set name = member.name.camelCase() -%}
    {% if member.length == None %}
        StreamIn(sink, t.{{name}});
    {% else %}
        StreamIn(sink, Iterable(t.{{name}}, t.{{member.length.name.camelCase()}}));
    {% endif %}
{% endmacro %}

{# Helper macro to render readers and writers. Should be used in a call block to provide additional custom
   handling when necessary. The optional `omit` field can be used to omit fields that are either
   handled in the custom code, or unnecessary in the serialized output.
   Example:
       {% call render_streaming_impl("struct name", writer=true, reader=false, omits=["omit field"]) %}
           // Custom C++ code to handle special types/members that are hard to generate code for
       {% endcall %}
   One day we should probably make the generator smart enough to generate everything it can
   instead of manually adding streaming implementations here.
#}
{% macro render_streaming_impl(json_type, writer, reader, omits=[]) %}
    {%- set cpp_type = types[json_type].name.CamelCase() -%}
    {% if reader %}
        template <>
        MaybeError stream::Stream<{{cpp_type}}>::Read(stream::Source* source, {{cpp_type}}* t) {
        {{ caller() }}
        {% for member in types[json_type].members %}
            {% if not member.name.get() in omits %}
                    {{render_reader(member)}}
            {% endif %}
        {% endfor %}
            return {};
        }
    {% endif %}
    {% if writer %}
        template <>
        void stream::Stream<{{cpp_type}}>::Write(stream::Sink* sink, const {{cpp_type}}& t) {
        {{ caller() }}
        {% for member in types[json_type].members %}
            {% if not member.name.get() in omits %}
                    {{render_writer(member)}}
            {% endif %}
        {% endfor %}
        }
    {% endif %}
{% endmacro %}

// Custom stream operator for special bool type that doesn't have the same size as C++'s bool.
{% set BoolCppType = metadata.namespace + "::" + as_cppType(types["bool"].name) %}
template <>
void stream::Stream<{{BoolCppType}}>::Write(stream::Sink* sink, const {{BoolCppType}}& t) {
    StreamIn(sink, static_cast<bool>(t));
}

// Custom stream operator for StringView.
{% set StringViewType = as_cppType(types["string view"].name) %}
template <>
void stream::Stream<{{StringViewType}}>::Write(stream::Sink* sink, const {{StringViewType}}& t) {
    bool undefined = t.IsUndefined();
    std::string_view sv = t;
    StreamIn(sink, undefined, sv);
}


{% call render_streaming_impl("adapter info", true, false) %}
{% endcall %}

{% call render_streaming_impl("dawn cache device descriptor", true, false,
                              omits=["load data function", "store data function", "function userdata"]) %}
{% endcall %}

{% call render_streaming_impl("extent 3D", true, true) %}
{% endcall %}

} // namespace {{native_namespace}}
