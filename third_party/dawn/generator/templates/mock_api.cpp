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

{% set api = metadata.api.lower() %}
#include "mock_{{api}}.h"

#include <algorithm>

#include "dawn/dawn_version.h"
#include "src/utils/assert.h"
#include "src/utils/log.h"

using namespace testing;

namespace {
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{as_annotated_cType(method.returns)}} Forward{{as_MethodSuffix(type.name, method.name)}}(
                {{-as_cType(type.name)}} self
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                auto object = reinterpret_cast<ProcTableAsClass::Object*>(self);
                return object->procs->{{as_CppMethodSuffix(type.name, method.name)}}(self
                    {%- for arg in method.arguments -%}
                        , {{as_varName(arg.name)}}
                    {%- endfor -%}
                );
            }
        {% endfor %}

    {% endfor %}
}

ProcTableAsClass::~ProcTableAsClass() {
}

WGPUFuture ProcTableAsClass::GetLastFuture() {
    return {mNextFutureID - 1};
}

{% set Prefix = metadata.proc_table_prefix %}
void ProcTableAsClass::GetProcTable({{Prefix}}ProcTable* table) {
    std::ranges::copy(dawn::kDawnVersion, table->version);

    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            table->{{as_varName(type.name, method.name)}} = reinterpret_cast<{{as_cProc(type.name, method.name)}}>(Forward{{as_MethodSuffix(type.name, method.name)}});
        {% endfor %}
    {% endfor %}

    {% for type in by_category["structure"] if type.has_free_members_function %}
        table->{{as_varName(type.name, Name("free members"))}} = []({{as_cType(type.name)}} {{as_varName(type.name)}}) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No mock available for {{as_varName(type.name, Name('free members'))}}";
            }
        };
    {% endfor %}
}

{% for type in by_category["object"] %}
    {% for method in type.methods %}
        {% set Suffix = as_CppMethodSuffix(type.name, method.name) %}
        {% if has_callbackInfoStruct(method.arguments) %}
            {{as_annotated_cType(method.returns)}} ProcTableAsClass::{{Suffix}}(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in method.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                {% if method.returns and method.returns.type.name.get() == "future" %}
                    dawn::FutureID futureID = mNextFutureID++;
                    object->m{{Suffix}}Requests[futureID] = {callbackInfo.callback, callbackInfo.userdata1, callbackInfo.userdata2};

                    On{{Suffix}}(
                        {{-as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_varName(arg.name)}}
                        {%- endfor -%}
                    );
                    return {futureID};
                {% else %}
                    object->m{{Suffix}}Callback = callbackInfo.callback;
                    object->m{{Suffix}}Userdata1 = callbackInfo.userdata1;
                    object->m{{Suffix}}Userdata2 = callbackInfo.userdata2;

                    On{{Suffix}}(
                        {{-as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_varName(arg.name)}}
                        {%- endfor -%}
                    );
                {% endif %}
            }
            {% set CallbackInfoType = (method.arguments|last).type %}
            {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
            void ProcTableAsClass::Call{{Suffix}}Callback(
                {{-as_cType(type.name)}} {{as_varName(type.name)}}
                {%- for arg in CallbackType.arguments -%}
                    , {{as_annotated_cType(arg)}}
                {%- endfor -%}
                {%- if method.returns and method.returns.type.name.get() == "future" -%}
                    , WGPUFuture future
                {%- endif -%}
            ) {
                ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>({{as_varName(type.name)}});
                {% if method.returns and method.returns.type.name.get() == "future" %}
                    if (future.id == dawn::kNullFutureID) {
                        // TODO(crbug.com/514400091): We provide this default version for emulating
                        // callbacks to avoid breaking existing tests that only ever have one
                        // outstanding pending callback at a time. Consider updating all tests,
                        // and/or adding better utility functions to make this process easier.
                        DAWN_ASSERT(object->m{{Suffix}}Requests.size() == 1);
                        auto it = object->m{{Suffix}}Requests.begin();
                        auto data = it->second;
                        object->m{{Suffix}}Requests.erase(it);
                        data.callback(
                            {%- for arg in CallbackType.arguments -%}
                                {{as_varName(arg.name)}}{{", "}}
                            {%- endfor -%}
                            data.userdata1, data.userdata2);
                    } else {
                        auto it = object->m{{Suffix}}Requests.find(future.id);
                        DAWN_ASSERT(it != object->m{{Suffix}}Requests.end());
                        auto data = it->second;
                        object->m{{Suffix}}Requests.erase(it);
                        data.callback(
                            {%- for arg in CallbackType.arguments -%}
                                {{as_varName(arg.name)}}{{", "}}
                            {%- endfor -%}
                            data.userdata1, data.userdata2);
                    }
                {% else %}
                    object->m{{Suffix}}Callback(
                        {%- for arg in CallbackType.arguments -%}
                            {{as_varName(arg.name)}}{{", "}}
                        {%- endfor -%}
                        object->m{{Suffix}}Userdata1, object->m{{Suffix}}Userdata2);
                {% endif %}
            }
        {% endif %}
    {% endfor %}
{% endfor %}

// Manually implement some callback helpers for testing.
void ProcTableAsClass::CallDeviceLostCallback(WGPUDevice device, WGPUDeviceLostReason reason, WGPUStringView message) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mDeviceLostCallback(&device, reason, message, object->mDeviceLostUserdata1,
                                object->mDeviceLostUserdata2);
}
void ProcTableAsClass::CallDeviceUncapturedErrorCallback(WGPUDevice device, WGPUErrorType type, WGPUStringView message) {
    ProcTableAsClass::Object* object = reinterpret_cast<ProcTableAsClass::Object*>(device);
    object->mUncapturedErrorCallback(&device, type, message, object->mUncapturedErrorUserdata1,
                                     object->mUncapturedErrorUserdata2);
}

{% for type in by_category["object"] %}
    {{as_cType(type.name)}} ProcTableAsClass::GetNew{{type.name.CamelCase()}}() {
        mObjects.emplace_back(new Object);
        mObjects.back()->procs = this;
        return reinterpret_cast<{{as_cType(type.name)}}>(mObjects.back().get());
    }
{% endfor %}

MockProcTable::MockProcTable() = default;

MockProcTable::~MockProcTable() = default;
