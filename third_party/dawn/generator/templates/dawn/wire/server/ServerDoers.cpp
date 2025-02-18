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

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {
    //* Implementation of the command doers
    {% for command in cmd_records["command"] %}
        {% set type = command.derived_object %}
        {% set method = command.derived_method %}
        {% set is_method = method is not none %}

        {% set Suffix = command.name.CamelCase() %}
        {% if Suffix not in client_side_commands %}
            {% if is_method %}
                WireResult Server::Do{{Suffix}}(
                    {%- for member in command.members -%}
                        {%- if member.is_return_value -%}
                            {%- if member.handle_type -%}
                                {{as_cType(member.handle_type.name)}}* {{as_varName(member.name)}}
                            {%- else -%}
                                {{as_cType(member.type.name)}}* {{as_varName(member.name)}}
                            {%- endif -%}
                        {%- else -%}
                            {{as_annotated_cType(member)}}
                        {%- endif -%}
                        {%- if not loop.last -%}, {% endif %}
                    {%- endfor -%}
                ) {
                    {% set ret = command.members|selectattr("is_return_value")|list %}
                    //* If there is a return value, assign it.
                    {% if ret|length == 1 %}
                        *{{as_varName(ret[0].name)}} =
                    {% else %}
                        //* Only one member should be a return value.
                        {{ assert(ret|length == 0) }}
                    {% endif %}
                    mProcs.{{as_varName(type.name, method.name)}}(
                        {%- for member in command.members if not member.is_return_value -%}
                            {{as_varName(member.name)}}
                            {%- if not loop.last -%}, {% endif %}
                        {%- endfor -%}
                    );
                    {% if ret|length == 1 %}
                        //* WebGPU error handling guarantees that no null object can be returned by
                        //* object creation functions.
                        DAWN_ASSERT(*{{as_varName(ret[0].name)}} != nullptr);
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
                    Reserved<{{cType}}> obj;
                    WIRE_TRY(Objects<{{cType}}>().Get(objectId, &obj));

                    if (obj->state == AllocationState::Allocated) {
                        DAWN_ASSERT(obj->handle != nullptr);
                        {% if type.name.get() == "device" %}
                            //* Deregisters uncaptured error and device lost callbacks since
                            //* they should not be forwarded if the device no longer exists on the wire.
                            ClearDeviceCallbacks(obj->handle);
                        {% endif %}
                        Release(mProcs, obj->handle);
                    }
                    Objects<{{cType}}>().Free(objectId);
                    return WireResult::Success;
                }
            {% endfor %}
            default:
                return WireResult::FatalError;
        }
    }

}  // namespace dawn::wire::server
