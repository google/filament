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

#include "src/dawn/wire/server/Server.h"
#include "src/utils/assert.h"

namespace dawn::wire::server {
    //* Implementation of the command doers
    {% for command in cmd_records["command"] %}
        {% set type = command.derived_object %}
        {% set method = command.derived_method %}
        {% set is_method = method is not none %}

        {% set Suffix = command.name.CamelCase() %}
        {% set CmdName = Suffix + "Cmd" %}
        {% if Suffix not in client_side_commands %}
            {% if is_method %}
                WireResult Server::Do{{Suffix}}(
                    {%- for member in command.members if not member.is_length -%}
                        {%- if not loop.first -%}, {% endif %}
                        {%- if member.is_return_value -%}
                            {%- if member.handle_type -%}
                                {{as_cType(member.handle_type.name)}}* {{as_varName(member.name)}}
                            {%- else -%}
                                {{as_cType(member.type.name)}}* {{as_varName(member.name)}}
                            {%- endif -%}
                        {%- elif member.length and member.constant_length != 1 -%}
                            {% set length = "dawn::detail::DynamicExtent<size_t>" %}
                            {% if member.length == "constant" %}
                                {% set length = member.constant_length %}
                            {% endif %}
                            {% set element_type = "std::remove_pointer_t<" + decorate(as_cType(member.type.name, True), member) + ">" %}
                            {% if is_wire_data_only(member) %}
                                //* If the member is data only, we do not copy the data, so it will be volatile.
                                {% set element_type = "volatile " + element_type %}
                            {% endif %}
                            ityp::span<size_t, {{element_type}}, {{length}}> {{as_varName(member.name)}}
                        {%- else -%}
                            {{as_annotated_cType(member)}}
                        {%- endif -%}
                    {%- endfor -%}
                ) {
                    {% set ret = command.members|selectattr("is_return_value")|list %}
                    //* If there is a return value, assign it.
                    {% if ret|length == 1 %}
                        *{{as_varName(ret[0].name)}} =
                    {% elif method.returns and method.returns.type.name.canonical_case() == "status" %}
                        WGPUStatus status =
                    {% else %}
                        //* Only one member should be a return value.
                        {{ assert(ret|length == 0) }}
                        {{ assert(not method.returns) }}
                    {% endif %}
                    mProcs->{{as_varName(type.name, method.name)}}(
                        {%- for member in command.members if not member.is_return_value -%}
                            {%- if not loop.first -%}, {% endif %}
                            {%- if member.is_length -%}
                                {%- set span_members = command.members | selectattr("length", "equalto", member) | list -%}
                                {{as_varName(span_members[0].name)}}.size()
                            {%- elif member.length and member.constant_length != 1 -%}
                                {% if is_wire_data_only(member) %}
                                    //* For wire data types, we cast away the volatile here. This
                                    //* is fine since the data is not sensitive to TOCTOU attacks.
                                    const_cast<const std::byte*>({{as_varName(member.name)}}.data())
                                {%- else -%}
                                    {{as_varName(member.name)}}.data()
                                {%- endif -%}
                            {%- else -%}
                                {{as_varName(member.name)}}
                            {%- endif -%}
                        {%- endfor -%}
                    );
                    {% if ret|length == 1 %}
                        //* WebGPU error handling guarantees that no null object can be returned by
                        //* object creation functions.
                        DAWN_ASSERT(*{{as_varName(ret[0].name)}} != nullptr);
                    {% endif %}

                    //* The client is responsible for making sure what it does isn't an invalid API
                    //* usage that will cause a WGPUStatus_Error.
                    {% if method.returns and method.returns.type.name.canonical_case() == "status" %}
                        if (status != WGPUStatus_Success) {
                            return WireResult::FatalError;
                        }
                    {% endif %}

                    return WireResult::Success;
                }
            {% endif %}
        {% endif %}
    {% endfor %}

    WireResult Server::DoUnregisterObject(ObjectType objectType, ObjectId objectId) {
        switch(objectType) {
            {% for type in by_category["object"] %}
                {% set cType = as_cType(type.name) %}
                case ObjectType::{{type.name.CamelCase()}}: {
                    ObjectData<{{cType}}> data;
                    WIRE_TRY(Free<{{cType}}>(objectId, &data));

                    //* Handle actually releasing the object after untracking it.
                    if (data.state == AllocationState::Allocated) {
                        DAWN_ASSERT(data.handle != nullptr);
                        {% if type.name.get() == "device" %}
                            //* Destroy the device to ensure that the spontaneous callbacks, i.e.
                            //* the uncaptured error and logging callbacks, are cleared, and the
                            //* device lost callback is fired. This is important because once we
                            //* deallocate the ObjectData, those callbacks reference freed memory.
                            mProcs->deviceDestroy(data.handle);
                        {% endif %}
                        Release(data.handle);
                    }
                    return WireResult::Success;
                }
            {% endfor %}
            default:
                return WireResult::FatalError;
        }
    }

}  // namespace dawn::wire::server
