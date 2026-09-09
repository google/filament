//* Copyright 2019 The Dawn & Tint Authors
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

{% from 'dawn/cpp_macros.tmpl' import as_dawnType with context %}
{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "dawn/wire/client/{{prefix}}_platform.h"
#include "dawn/wire/client/webgpu.h"
#include "dawn/dawn_version.h"
#include "src/dawn/wire/client/Client.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"

{%- macro convert_arguments_and_call(function, suffix, call_receiver, first_arg = None) -%}
    {% set client = "dawn::wire::client" %}

    {% for arg in function.arguments %}
        {% set varName = as_varName(arg.name) %}
        {% if arg.is_length %}
            //* Skip as it's included in the span just below.
        {% elif arg.length and arg.length != "constant" %}
            {% if arg.type.name.canonical_case() == "void" %}
                using {{varName}}SpanT = {% if arg.annotation == "const*" %}const {% endif %}std::byte;
                auto {{varName}}Ptr = reinterpret_cast<{{varName}}SpanT*>({{varName}});
            {% else %}
                using {{varName}}SpanT = std::remove_pointer_t<{% if arg.type.category in ["object", "structure"] %}{{client}}::{% endif %}{{decorate(as_dawnType(arg.type), arg)}}>;
                auto {{varName}}Ptr = {{client}}::FromAPI({{varName}});
            {% endif %}
            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
            auto {{varName}}_ = DAWN_UNSAFE_BUFFERS(dawn::Span<{{varName}}SpanT>({{varName}}Ptr, {{as_varName(arg.length.name)}}));
        {% elif arg.type.category == "structure" %}
            auto {{varName}}_ = {{client}}::FromAPI({{varName}});
        {% elif arg.type.category in ["enum", "bitmask"] and arg.annotation == "value" %}
            auto {{varName}}_ = static_cast<{{as_dawnType(arg.type)}}>({{varName}});
        {% elif arg.type.category == "object" %}
            auto {{varName}}_ = {{client}}::FromAPI({{varName}});
        {% elif arg.annotation != "value" %}
            auto {{varName}}_ = reinterpret_cast<{{decorate(as_dawnType(arg.type), arg)}}>({{varName}});
        {% else %}
            auto {{varName}}_ = {{as_varName(arg.name)}};
        {% endif %}
    {%- endfor-%}

    {% if function.returns %}
        auto result =
    {%- endif %}
    {{call_receiver}}(
        {%- if first_arg -%}
            {{first_arg}} {%- if len(function.arguments) != 0 %}, {% endif -%}
        {%- endif -%}
        {%- for arg in function.arguments if not arg.is_length -%}
            {%- if not loop.first %}, {% endif -%}
            {{as_varName(arg.name)}}_
        {%- endfor -%}
    );
    {% if function.returns %}
        {% if function.returns.type.category in ["object", "enum", "bitmask"] %}
            return {{client}}::ToAPI(result);
        {% elif function.returns.type.category in ["structure"] %}
            return *{{client}}::ToAPI(&result);
        {% else %}
            return result;
        {% endif %}
    {% endif %}
{%- endmacro %}

namespace dawn::wire::client {

    // Template function for constexpr branching when creating new objects.
    template <typename Parent, typename Child, typename... Args>
    Child* Create(Parent p, Args... args) {
        if constexpr (std::is_constructible_v<Child, const ObjectBaseParams&, decltype(args)...>) {
            return p->GetClient()->template Make<Child>(args...).Detach();
        } else if constexpr (std::is_constructible_v<Child, const ObjectBaseParams&, Ref<Instance>, decltype(args)...>) {
            return p->GetClient()->template Make<Child>(p->GetInstance(), args...).Detach();
        } else {
            if constexpr (std::is_base_of_v<ObjectWithEventsBase, Child>) {
                return p->GetClient()->template Make<Child>(p->GetInstance()).Detach();
            } else {
                return p->GetClient()->template Make<Child>().Detach();
            }
        }
    }

}  // namespace dawn::wire::client

//* Implementation of the client API functions.
{% for (type, methods) in c_methods_sorted_by_parent %}
    {%- set Type = type.name.CamelCase() -%}
    {%- set cType = as_cType(type.name) -%}
    {%- set client = "dawn::wire::client" -%}

    {% for method in methods %}
        {% set Suffix = as_MethodSuffix(type.name, method.name) %}

        DAWN_WIRE_EXPORT {{as_annotated_cType(method.returns)}} {{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}}(
            {{-cType}} cSelf
            {%- for arg in method.arguments -%}
                , {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% if Suffix not in client_handwritten_commands %}
                auto self = {{client}}::FromAPI(cSelf);
                dawn::wire::{{Suffix}}Cmd cmd;

                //* Create the structure going on the wire on the stack and fill it with the value
                //* arguments so it can compute its size.
                cmd.self = cSelf;

                //* For object creation, store the object ID the client will use for the result.
                {% if method.returns and method.returns.type.category == "object" %}
                    {% set ReturnObj = client + "::" + method.returns.type.name.CamelCase() %}
                    {{ReturnObj}}* returnObject = {{client}}::Create<{{client}}::{{as_wireType(type)}}, {{ReturnObj}}>(self
                        {%- for arg in method.arguments -%}
                                , {{as_varName(arg.name)}}
                        {%- endfor -%}
                    );
                    cmd.result = returnObject->GetWireHandle(self->GetClient());
                {% endif %}

                {% for arg in method.arguments %}
                    //* Commands with mutable pointers should not be autogenerated.
                    {{assert(arg.annotation != "*")}}
                    {% set varName = as_varName(arg.name) %}
                    {% if arg.is_length %}
                        //* Skipped as it is included in the span below.
                    {% elif arg.length and arg.constant_length != 1 %}
                        using {{varName}}SpanT = std::remove_pointer_t<{{decorate(as_cType(arg.type.name, True), arg)}}>;
                        auto* {{varName}}Ptr = reinterpret_cast<{{varName}}SpanT*>({{varName}});
                        {% if arg.length == "constant" %}
                            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
                            cmd.{{varName}} = DAWN_UNSAFE_BUFFERS(dawn::Span<{{varName}}SpanT, {{arg.constant_length}}>({{varName}}Ptr));
                        {% else %}
                            size_t {{varName}}SizeV = dawn::checked_cast<size_t>({{as_varName(arg.length.name)}});
                            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
                            cmd.{{varName}} = DAWN_UNSAFE_BUFFERS(dawn::Span<{{varName}}SpanT>({{varName}}Ptr, {{varName}}SizeV));
                        {% endif %}
                    {% else %}
                        cmd.{{varName}} = {{varName}};
                    {% endif %}
                {% endfor %}

                //* Allocate space to send the command and copy the value args over.
                self->GetClient()->SerializeCommand(cmd);

                {% if method.returns and method.returns.type.category == "object" %}
                    return {{client}}::ToAPI(returnObject);
                {% endif %}
            {% elif type.category == "object" %}
                auto self = {{client}}::FromAPI(cSelf);
                {{convert_arguments_and_call(method, Suffix, "self->API" + method.name.CamelCase())}}
            {% elif type.category == "structure" %}
                return {{client}}::API{{Suffix}}(cSelf);
            {% endif %}
        }
    {% endfor %}

{% endfor %}

namespace {
    struct ProcEntry {
        WGPUProc proc;
        std::string_view name;
    };
    static const ProcEntry sProcMap[] = {
        {% for (type, method) in c_methods_sorted_by_name %}
            { reinterpret_cast<WGPUProc>({{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}}), "{{as_cMethod(type.name, method.name)}}" },
        {% endfor %}
    };
    static constexpr size_t sProcMapSize = sizeof(sProcMap) / sizeof(sProcMap[0]);
}  // anonymous namespace

DAWN_WIRE_EXPORT WGPUProc {{as_cMethodNamespaced(None, Name('get proc address'), Name('dawn wire client'))}}(WGPUStringView cProcName) {
    if (cProcName.data == nullptr) {
        return nullptr;
    }

    std::string_view procName(cProcName.data, cProcName.length != WGPU_STRLEN ? cProcName.length : strlen(cProcName.data));

    const ProcEntry* entry = std::lower_bound(&sProcMap[0], &sProcMap[sProcMapSize], procName,
        [](const ProcEntry &a, const std::string_view& b) -> bool {
            return a.name.compare(b) < 0;
        }
    );

    if (entry != &sProcMap[sProcMapSize] && entry->name == procName) {
        return entry->proc;
    }

    // Special case the free-standing functions of the API.
    // TODO(dawn:1238) Checking string one by one is slow, it needs to be optimized.
    {% for function in by_category["function"] %}
        if (procName == "{{as_cMethod(None, function.name)}}") {
            return reinterpret_cast<WGPUProc>({{as_cMethodNamespaced(None, function.name, Name('dawn wire client'))}});
        }

    {% endfor %}
    return nullptr;
}

namespace dawn::wire::client {

    std::vector<std::string_view> GetProcMapNamesForTesting() {
        std::vector<std::string_view> result;
        result.reserve(sProcMapSize);
        for (const ProcEntry& entry : sProcMap) {
            result.push_back(entry.name);
        }
        return result;
    }

    {% set Prefix = metadata.proc_table_prefix %}

    constexpr {{Prefix}}ProcTable MakeProcTable() {
        {{Prefix}}ProcTable procs = {};
        std::ranges::copy(dawn::kDawnVersion, procs.version);
        {% for function in by_category["function"] %}
            procs.{{as_varName(function.name)}} = {{as_cMethodNamespaced(None, function.name, Name('dawn wire client'))}};
        {% endfor %}
        {% for (type, methods) in c_methods_sorted_by_parent %}
            {% for method in methods %}
                procs.{{as_varName(type.name, method.name)}} = {{as_cMethodNamespaced(type.name, method.name, Name('dawn wire client'))}};
            {% endfor %}
        {% endfor %}
        return procs;
    }

    static {{Prefix}}ProcTable gProcTable = MakeProcTable();

    const {{Prefix}}ProcTable& GetProcs() {
        return gProcTable;
    }

}  // namespace dawn::wire::client
