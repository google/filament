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
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/{{Prefix}}Native.h"

#include <algorithm>
#include <vector>

{% for type in by_category["object"] %}
    {% if type.name.canonical_case() not in ["texture view"] %}
        #include "{{native_dir}}/{{type.name.CamelCase()}}.h"
    {% endif %}
{% endfor %}

namespace {{native_namespace}} {

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {% set suffix = as_MethodSuffix(type.name, method.name) %}

            {{as_cType(method.return_type.name)}} Native{{suffix}}(
                {{-as_cType(type.name)}} cSelf
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                //* Perform conversion between C types and frontend types
                auto self = FromAPI(cSelf);

                {% for arg in method.arguments %}
                    {% set varName = as_varName(arg.name) %}
                    {% if arg.type.category in ["enum", "bitmask"] and arg.annotation == "value" %}
                        auto {{varName}}_ = static_cast<{{as_frontendType(arg.type)}}>({{varName}});
                    {% elif arg.type.category == "structure" and arg.annotation == "value" %}
                        auto {{varName}}_ = *reinterpret_cast<{{as_frontendType(arg.type)}}*>(&{{varName}});
                    {% elif arg.annotation != "value" or arg.type.category == "object" %}
                        auto {{varName}}_ = reinterpret_cast<{{decorate("", as_frontendType(arg.type), arg)}}>({{varName}});
                    {% else %}
                        auto {{varName}}_ = {{as_varName(arg.name)}};
                    {% endif %}
                {%- endfor-%}

                {% if method.autolock and method.return_type.name.get() != 'future' %}
                    {% if type.name.get() != "device" %}
                        auto device = self->GetDevice();
                    {% else %}
                        auto device = self;
                    {% endif %}
                    auto deviceLock(device->GetScopedLock());
                {% else %}
                    // This method is specified to not use AutoLock in json script or it returns a future.
                {% endif %}

                {% if method.return_type.name.canonical_case() != "void" %}
                    auto result =
                {%- endif %}
                self->API{{method.name.CamelCase()}}(
                    {%- for arg in method.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        {{as_varName(arg.name)}}_
                    {%- endfor -%}
                );
                {% if method.return_type.name.canonical_case() != "void" %}
                    {% if method.return_type.category in ["object", "enum", "bitmask"] %}
                        return ToAPI(result);
                    {% elif method.return_type.category in ["structure"] %}
                        return *ToAPI(&result);
                    {% else %}
                        return result;
                    {% endif %}
                {% endif %}
            }
        {% endfor %}
    {% endfor %}

    {% for function in by_category["function"] if function.name.canonical_case() != "get proc address" and function.name.canonical_case() != "get proc address 2" %}
        {{as_cType(function.return_type.name)}} Native{{function.name.CamelCase()}}(
            {%- for arg in function.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}
        ) {
            {% for arg in function.arguments %}
                {% set varName = as_varName(arg.name) %}
                {% if arg.type.category in ["enum", "bitmask"] and arg.annotation == "value" %}
                    auto {{varName}}_ = static_cast<{{as_frontendType(arg.type)}}>({{varName}});
                {% elif arg.annotation != "value" or arg.type.category == "object" %}
                    auto {{varName}}_ = reinterpret_cast<{{decorate("", as_frontendType(arg.type), arg)}}>({{varName}});
                {% else %}
                    auto {{varName}}_ = {{as_varName(arg.name)}};
                {% endif %}
            {%- endfor-%}

            {% if function.return_type.name.canonical_case() != "void" %}
                auto result =
            {%- endif %}
            API{{function.name.CamelCase()}}(
                {%- for arg in function.arguments -%}
                    {%- if not loop.first %}, {% endif -%}
                    {{as_varName(arg.name)}}_
                {%- endfor -%}
            );
            {% if function.return_type.name.canonical_case() != "void" %}
                {% if function.return_type.category in ["object", "enum", "bitmask"] %}
                    return ToAPI(result);
                {% else %}
                    return result;
                {% endif %}
            {% endif %}
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
        {% for function in by_category["function"] %}
            procs.{{as_varName(function.name)}} = Native{{as_cppType(function.name)}};
        {% endfor %}
        {% for type in by_category["object"] %}
            {% for method in c_methods(type) %}
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
