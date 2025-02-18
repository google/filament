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
    {% for command in cmd_records["command"] %}
        {% set method = command.derived_method %}
        {% set is_method = method != None %}
        {% set returns = is_method and method.return_type.name.canonical_case() != "void" %}

        {% set Suffix = command.name.CamelCase() %}
        //* The generic command handlers
        WireResult Server::Handle{{Suffix}}(DeserializeBuffer* deserializeBuffer) {
            {{Suffix}}Cmd cmd;
            WIRE_TRY(cmd.Deserialize(deserializeBuffer, &mAllocator
                {%- if command.may_have_dawn_object -%}
                    , *this
                {%- endif -%}
            ));
            {% if Suffix in server_custom_pre_handler_commands %}
                WIRE_TRY(PreHandle{{Suffix}}(cmd));
            {%- endif -%}

            //* Allocate any result objects
            {% for member in command.members if member.is_return_value -%}
                {{ assert(member.handle_type) }}
                {% set cType = as_cType(member.handle_type.name) %}
                {% set name = as_varName(member.name) %}
                Reserved<{{cType}}> {{name}}Data;
                WIRE_TRY(Objects<{{cType}}>().Allocate(&{{name}}Data, cmd.{{name}}));
                {{name}}Data->generation = cmd.{{name}}.generation;
            {%- endfor %}

            //* Get any input objects
            {% for member in command.members if member.id_type != None -%}
                {% set cType = as_cType(member.id_type.name) %}
                {% set name = as_varName(member.name) %}
                Known<{{cType}}> {{name}}Handle;
                WIRE_TRY(Objects<{{cType}}>().Get(cmd.{{name}}, &{{name}}Handle));
            {% endfor %}

            //* Do command
            WIRE_TRY(Do{{Suffix}}(
                {%- for member in command.members -%}
                    {%- if member.is_return_value -%}
                        {%- if member.handle_type -%}
                            &{{as_varName(member.name)}}Data->handle //* Pass the handle of the output object to be written by the doer
                        {%- else -%}
                            &cmd.{{as_varName(member.name)}}
                        {%- endif -%}
                    {%- elif member.id_type != None -%}
                        {{as_varName(member.name)}}Handle
                    {%- else -%}
                        cmd.{{as_varName(member.name)}}
                    {%- endif -%}
                    {%- if not loop.last -%}, {% endif %}
                {%- endfor -%}
            ));

            return WireResult::Success;
        }
    {% endfor %}

    const volatile char* Server::HandleCommandsImpl(const volatile char* commands, size_t size) {
        DeserializeBuffer deserializeBuffer(commands, size);

        while (deserializeBuffer.AvailableSize() >= sizeof(CmdHeader) + sizeof(WireCmd)) {
            // Start by chunked command handling, if it is done, then it means the whole buffer
            // was consumed by it, so we return a pointer to the end of the commands.
            switch (HandleChunkedCommands(deserializeBuffer.Buffer(), deserializeBuffer.AvailableSize())) {
                case ChunkedCommandsResult::Consumed:
                    return commands + size;
                case ChunkedCommandsResult::Error:
                    return nullptr;
                case ChunkedCommandsResult::Passthrough:
                    break;
            }

            WireCmd cmdId = *static_cast<const volatile WireCmd*>(static_cast<const volatile void*>(
                deserializeBuffer.Buffer() + sizeof(CmdHeader)));
            WireResult result;
            switch (cmdId) {
                {% for command in cmd_records["command"] %}
                    case WireCmd::{{command.name.CamelCase()}}:
                        result = Handle{{command.name.CamelCase()}}(&deserializeBuffer);
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

        // After the server handles all the commands from the stream, we additionally run
        // ProcessEvents on all known Instances so that any work done on the server side can be
        // forwarded through to the client.
        for (auto instance : Objects<WGPUInstance>().GetAllHandles()) {
            if (DoInstanceProcessEvents(instance) != WireResult::Success) {
                return nullptr;
            }
        }

        if (deserializeBuffer.AvailableSize() != 0) {
            return nullptr;
        }

        return commands;
    }

}  // namespace dawn::wire::server
