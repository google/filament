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
#include "dawn/wire/client/Client.h"

#include <string>

namespace dawn::wire::client {
    {% for command in cmd_records["return command"] %}
        WireResult Client::Handle{{command.name.CamelCase()}}(DeserializeBuffer* deserializeBuffer) {
            Return{{command.name.CamelCase()}}Cmd cmd;
            WIRE_TRY(cmd.Deserialize(deserializeBuffer, &mAllocator));

            {% for member in command.members if member.handle_type %}
                {% set Type = member.handle_type.name.CamelCase() %}
                {% set name = as_varName(member.name) %}

                {% if member.type.dict_name == "ObjectHandle" %}
                    {{Type}}* {{name}} = Get<{{Type}}>(cmd.{{name}}.id);
                    if ({{name}} != nullptr && {{name}}->GetWireHandle(this).generation != cmd.{{name}}.generation) {
                        {{name}} = nullptr;
                    }
                {% endif %}
            {% endfor %}

            return Do{{command.name.CamelCase()}}(
                {%- for member in command.members -%}
                    {%- if member.handle_type -%}
                        {{as_varName(member.name)}}
                    {%- else -%}
                        cmd.{{as_varName(member.name)}}
                    {%- endif -%}
                    {%- if not loop.last -%}, {% endif %}
                {%- endfor -%}
            );
        }
    {% endfor %}

    const volatile char* Client::HandleCommands(const volatile char* commands, size_t size) {
        DeserializeBuffer deserializeBuffer(commands, size);

        while (deserializeBuffer.AvailableSize() >= sizeof(CmdHeader) + sizeof(WireCmd)) {
            WireCmd cmdId = *static_cast<const volatile WireCmd*>(static_cast<const volatile void*>(
                deserializeBuffer.Buffer() + sizeof(CmdHeader)));
            WireResult result = WireResult::FatalError;
            switch (cmdId) {
                {% for command in cmd_records["special command"] %}
                    {% set Suffix = command.name.CamelCase() %}
                    case WireCmd::{{Suffix}}:
                        result = Handle{{Suffix}}(&deserializeBuffer);
                        break;
                {% endfor %}
                {% for command in cmd_records["return command"] %}
                    {% set Suffix = command.name.CamelCase() %}
                    case WireCmd::Return{{Suffix}}:
                        result = Handle{{Suffix}}(&deserializeBuffer);
                        break;
                {% endfor %}
                default:
                    result = WireResult::FatalError;
            }

            if (result != WireResult::Success) {
                return nullptr;
            }
            mAllocator.Reset();
        }

        if (deserializeBuffer.AvailableSize() != 0) {
            return nullptr;
        }

        return commands;
    }
}  // namespace dawn::wire::client
