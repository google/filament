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

#include "src/dawn/wire/client/Client.h"
#include "src/utils/assert.h"

#include <string>

namespace dawn::wire::client {
    {% for command in cmd_records["return command"] %}
        {% set CmdName = "Return" + command.name.CamelCase() + "Cmd" %}
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
                {%- for member in command.members if not member.is_length -%}
                    {%- if not loop.first -%}, {% endif %}
                    {%- if member.handle_type -%}
                        {{as_varName(member.name)}}
                    {%- else -%}
                        cmd.{{as_varName(member.name)}}
                    {%- endif -%}
                {%- endfor -%}
            );
        }
    {% endfor %}

    bool Client::HandleCommands(Span<const volatile std::byte> commands) {
        DeserializeBuffer deserializeBuffer(commands);

        const volatile CmdHeader* cmdHeader;
        while (deserializeBuffer.Peek(&cmdHeader) != WireResult::FatalError) {
            WireCmd cmdId = cmdHeader->commandId;
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
                return false;
            }
            mAllocator.Reset();
        }

        if (!deserializeBuffer.Empty()) {
            return false;
        }

        return true;
    }
}  // namespace dawn::wire::client
