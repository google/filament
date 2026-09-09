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
{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set include_dir = namespace_name.Dirs() %}
#include <algorithm>
#include <vector>

#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{include_dir}}/{{Prefix}}Native.h"
#include "dawn/dawn_version.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"

{% for type in by_category["object"] %}
    {% if type.name.canonical_case() not in ["texture view"] %}
        #include "{{native_dir}}/{{type.name.CamelCase()}}.h"
    {% endif %}
{% endfor %}

{%- macro convert_arguments_and_call(function, suffix, call_receiver, first_arg = None) -%}
    {% for arg in function.arguments %}
        {% set varName = as_varName(arg.name) %}
        {% if (arg.spanify | default(True)) and arg.is_length %}
            //* Skip as it's included in the span just below.
        {% elif (arg.spanify | default(True)) and arg.length and arg.length != "constant" %}
            // TODO(https://crbug.com/524405497): Support fixed-length spans.
            {% if arg.type.name.canonical_case() == "void" %}
                using {{varName}}SpanT = {% if arg.annotation == "const*" %}const {% endif %}std::byte;
            {% else %}
                using {{varName}}SpanT = std::remove_pointer_t<{{decorate(as_frontendType(arg.type), arg)}}>;
            {% endif %}
            {% set IndexType = arg.index_type | default('size_t') %}
            auto {{varName}}Size_ = checked_cast<{{IndexType}}>({{as_varName(arg.length.name)}});
            auto {{varName}}Ptr = reinterpret_cast<{{varName}}SpanT*>({{varName}});
            // SAFETY: The webgpu.h user is required to pass valid ranges of objects.
            auto {{varName}}_ = DAWN_UNSAFE_BUFFERS(ityp::span<{{IndexType}}, {{varName}}SpanT>({{varName}}Ptr, {{varName}}Size_));
        {% elif arg.type.category in ["enum", "bitmask"] and arg.annotation == "value" %}
            auto {{varName}}_ = static_cast<{{as_frontendType(arg.type)}}>({{varName}});
        {% elif arg.type.category == "structure" and arg.annotation == "value" %}
            auto {{varName}}_ = *reinterpret_cast<{{as_frontendType(arg.type)}}*>(&{{varName}});
        {% elif arg.annotation != "value" or arg.type.category == "object" %}
            auto {{varName}}_ = reinterpret_cast<{{decorate(as_frontendType(arg.type), arg)}}>({{varName}});
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
        {%- for arg in function.arguments if (not (arg.spanify | default(True)) or not arg.is_length) -%}
            {%- if not loop.first %}, {% endif -%}
            {{as_varName(arg.name)}}_
        {%- endfor -%}
    );
    {% if function.returns %}
        {% if function.returns.type.category in ["object", "enum", "bitmask"] %}
            return ToAPI(result);
        {% elif function.returns.type.category in ["structure"] %}
            return *ToAPI(&result);
        {% else %}
            return result;
        {% endif %}
    {% endif %}
{%- endmacro -%}

namespace {{native_namespace}} {
    {% for (type, methods) in c_methods_sorted_by_parent %}
        {% for method in methods %}
            {% set suffix = as_MethodSuffix(type.name, method.name) %}

            {{as_annotated_cType(method.returns)}} Native{{suffix}}(
                {{-as_cType(type.name)}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                //* Perform conversion between C types and frontend types
                {% if type.category == "object" %}
                    auto self = FromAPI(cSelf);

                    {% if method.autolock and not (method.returns and method.returns.type.name.get() == 'future') %}
                        {% if type.name.get() != "device" %}
                            auto device = self->GetDevice();
                        {% else %}
                            auto device = self;
                        {% endif %}
                        auto deviceGuard = device->GetGuard();
                    {% else %}
                        // This method is specified to not use AutoLock in json script or it returns a future.
                    {% endif %}

                    {{convert_arguments_and_call(method, suffix, "self->API" + method.name.CamelCase())}}
                {% else %}
                    {{assert(type.category == "structure")}}
                    {{convert_arguments_and_call(method, suffix, "API" + suffix, first_arg="cSelf")}}
                {% endif %}
            }
        {% endfor %}
    {% endfor %}

    {% for function in by_category["function"] if function.name.canonical_case() != "get proc address" and function.name.canonical_case() != "get proc address 2" %}
        {% set suffix = function.name.CamelCase() %}
        {{as_annotated_cType(function.returns)}} Native{{suffix}}(
            {%- for arg in function.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {{convert_arguments_and_call(function, suffix, "API" + suffix)}}
        }
    {% endfor %}

    namespace {

        {% set c_prefix = metadata.c_prefix %}
        struct ProcEntry {
            {{c_prefix}}Proc proc;
            std::string_view name;
        };
        static const ProcEntry sProcMap[] = {
            {% for (type, method) in c_methods_sorted_by_name %}
                { reinterpret_cast<{{c_prefix}}Proc>(Native{{as_MethodSuffix(type.name, method.name)}}), "{{as_cMethod(type.name, method.name)}}" },
            {% endfor %}
        };
        static constexpr size_t sProcMapSize = sizeof(sProcMap) / sizeof(sProcMap[0]);

    }  // anonymous namespace

    WGPUProc NativeGetProcAddress(WGPUStringView cProcName) {
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
                return reinterpret_cast<{{c_prefix}}Proc>(Native{{as_cppType(function.name)}});
            }

        {% endfor %}
        return nullptr;
    }

    std::vector<std::string_view> GetProcMapNamesForTestingInternal() {
        std::vector<std::string_view> result;
        result.reserve(sProcMapSize);
        for (const ProcEntry& entry : sProcMap) {
            result.push_back(entry.name);
        }
        return result;
    }

    constexpr {{Prefix}}ProcTable MakeProcTable() {
        {{Prefix}}ProcTable procs = {};
        std::ranges::copy(dawn::kDawnVersion, procs.version);
        {% for function in by_category["function"] %}
            procs.{{as_varName(function.name)}} = Native{{as_cppType(function.name)}};
        {% endfor %}
        {% for (type, methods) in c_methods_sorted_by_parent %}
            {% for method in methods %}
                procs.{{as_varName(type.name, method.name)}} = Native{{as_MethodSuffix(type.name, method.name)}};
            {% endfor %}
        {% endfor %}
        return procs;
    }

    static {{Prefix}}ProcTable gProcTable = MakeProcTable();

    const {{Prefix}}ProcTable& GetProcsAutogen() {
        return gProcTable;
    }
}
