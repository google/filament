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

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
#ifndef MOCK_{{API}}_H
#define MOCK_{{API}}_H

{% set Prefix = metadata.proc_table_prefix %}
{% set prefix = Prefix.lower() %}
#include "dawn/{{prefix}}_proc_table.h"
#include "dawn/{{api}}.h"
#include <gmock/gmock.h>

#include <atomic>
#include <memory>

#include "dawn/common/FutureUtils.h"

// An abstract base class representing a proc table so that API calls can be mocked. Most API calls
// are directly represented by a delete virtual method but others need minimal state tracking to be
// useful as mocks.
class ProcTableAsClass {
    public:
        virtual ~ProcTableAsClass();

        void GetProcTable({{Prefix}}ProcTable* table);

        // Creates an object that can be returned by a mocked call as in WillOnce(Return(foo)).
        // It returns an object of the write type that isn't equal to any previously returned object.
        // Otherwise some mock expectation could be triggered by two different objects having the same
        // value.
        {% for type in by_category["object"] %}
            {{as_cType(type.name)}} GetNew{{type.name.CamelCase()}}();
        {% endfor %}

        {%- for type in by_category["object"] %}

            virtual void {{as_MethodSuffix(type.name, Name("add ref"))}}({{as_cType(type.name)}} self) = 0;
            virtual void {{as_MethodSuffix(type.name, Name("release"))}}({{as_cType(type.name)}} self) = 0;
            {% for method in type.methods %}
                {% set Suffix = as_CppMethodSuffix(type.name, method.name) %}
                {% if not has_callbackInfoStruct(method) %}
                    virtual {{as_cType(method.return_type.name)}} {{Suffix}}(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ) = 0;
                {% else %}
                    //* For functions with callbacks, store callback and userdata and call the On* method.
                    {{as_cType(method.return_type.name)}} {{Suffix}}(
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    );
                {% endif %}
            {% endfor %}

            {% for method in type.methods if has_callbackInfoStruct(method) %}
                {% set Suffix = as_CppMethodSuffix(type.name, method.name) %}
                //* The virtual function to call after saving the callback and userdata in the proc.
                //* This function can be mocked.
                virtual void On{{Suffix}}(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in method.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                ) = 0;
                {% set CallbackInfoType = (method.arguments|last).type %}
                {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
                void Call{{Suffix}}Callback(
                    {{-as_cType(type.name)}} {{as_varName(type.name)}}
                    {%- for arg in CallbackType.arguments -%}
                        , {{as_annotated_cType(arg)}}
                    {%- endfor -%}
                );
            {% endfor %}

        {% endfor %}

        // Manually implement some callback helpers for testing.
        void CallDeviceLostCallback(WGPUDevice device, WGPUDeviceLostReason reason, WGPUStringView message);
        void CallDeviceUncapturedErrorCallback(WGPUDevice device, WGPUErrorType type, WGPUStringView message);

        struct Object {
            ProcTableAsClass* procs = nullptr;
            {% for type in by_category["object"] %}
                {% for method in type.methods if has_callbackInfoStruct(method) %}
                    {% set CallbackInfoType = (method.arguments|last).type %}
                    {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
                    void* m{{as_CppMethodSuffix(type.name, method.name)}}Userdata1 = 0;
                    void* m{{as_CppMethodSuffix(type.name, method.name)}}Userdata2 = 0;
                    {{as_cType(CallbackType.name)}} m{{as_CppMethodSuffix(type.name, method.name)}}Callback = nullptr;
                {% endfor %}
            {% endfor %}
            // Manually implement some callback helpers for testing.
            WGPUDeviceLostCallback mDeviceLostCallback = nullptr;
            void* mDeviceLostUserdata1 = 0;
            void* mDeviceLostUserdata2 = 0;
            WGPUUncapturedErrorCallback mUncapturedErrorCallback = nullptr;
            void* mUncapturedErrorUserdata1 = 0;
            void* mUncapturedErrorUserdata2 = 0;
        };

    private:
        // Remembers the values returned by GetNew* so they can be freed.
        std::vector<std::unique_ptr<Object>> mObjects;
        // Increasing futureID for testing purposes.
        std::atomic<dawn::FutureID> mNextFutureID = 1;
};

class MockProcTable : public ProcTableAsClass {
    public:
        MockProcTable();
        ~MockProcTable() override;

        void IgnoreAllReleaseCalls();

        {%- for type in by_category["object"] %}

            MOCK_METHOD(void, {{as_MethodSuffix(type.name, Name("add ref"))}}, ({{as_cType(type.name)}} self), (override));
            MOCK_METHOD(void, {{as_MethodSuffix(type.name, Name("release"))}}, ({{as_cType(type.name)}} self), (override));
            {% for method in type.methods if not has_callbackInfoStruct(method) %}
                MOCK_METHOD({{as_cType(method.return_type.name)}},{{" "}}
                    {{-as_MethodSuffix(type.name, method.name)}}, (
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ), (override));
            {% endfor %}

            {% for method in type.methods if has_callbackInfoStruct(method) %}
                MOCK_METHOD(void,{{" "-}}
                    On{{as_CppMethodSuffix(type.name, method.name)}}, (
                        {{-as_cType(type.name)}} {{as_varName(type.name)}}
                        {%- for arg in method.arguments -%}
                            , {{as_annotated_cType(arg)}}
                        {%- endfor -%}
                    ), (override));
            {% endfor %}
        {% endfor %}
};

#endif  // MOCK_{{API}}_H
