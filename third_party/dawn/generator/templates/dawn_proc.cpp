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

{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include "dawn/{{prefix}}_proc.h"

// The sanitizer is disabled for calls to procs.* since those functions may be
// dynamically loaded.
#include "dawn/common/Compiler.h"
#include "dawn/common/Log.h"

// A fake wgpuCreateInstance that prints a warning so folks know that they are using dawn_procs and
// should either use a different target to link against, or call dawnProcSetProcs.
WGPUInstance CreateInstanceThatWarns(const WGPUInstanceDescriptor* desc) {
    dawn::ErrorLog() <<
        R"(The \"null\" {{metadata.namespace}}CreateInstance from {{prefix}}_proc was called which always returns nullptr. You either need to:
  - call {{prefix}}ProcSetProcs with a {{Prefix}}ProcTable object retrieved somewhere else, or
  - (most likely) link against a different target that implements {{metadata.api}} directly, for example {{metadata.api.lower()}}_dawn)";

    return nullptr;
}

constexpr {{Prefix}}ProcTable MakeNullProcTable() {
    {{Prefix}}ProcTable procs = {};
    procs.createInstance = CreateInstanceThatWarns;
    return procs;
}

static {{Prefix}}ProcTable kNullProcs = MakeNullProcTable();
static {{Prefix}}ProcTable procs = MakeNullProcTable();

void {{prefix}}ProcSetProcs(const {{Prefix}}ProcTable* procs_) {
    if (procs_) {
        procs = *procs_;
    } else {
        procs = kNullProcs;
    }
}

{% for function in by_category["function"] %}
    DAWN_NO_SANITIZE("cfi-icall")
    {{as_annotated_cType(function.returns)}} {{as_cMethod(None, function.name)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{as_annotated_cType(arg)}}
        {%- endfor -%}
    ) {
        {% if function.returns %}return {% endif %}
        procs.{{as_varName(function.name)}}(
            {%- for arg in function.arguments -%}
                {% if not loop.first %}, {% endif %}{{as_varName(arg.name)}}
            {%- endfor -%}
        );
    }
{% endfor %}

{% for (type, methods) in c_methods_sorted_by_parent %}
    {% for method in methods %}
        DAWN_NO_SANITIZE("cfi-icall")
        {{as_annotated_cType(method.returns)}} {{as_cMethod(type.name, method.name)}}(
            {{-as_cType(type.name)}} {{as_varName(type.name)}}
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% if method.returns %}return {% endif %}
            procs.{{as_varName(type.name, method.name)}}({{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_varName(arg.name)}}
                {%- endfor -%}
            );
        }
    {% endfor %}

{% endfor %}
