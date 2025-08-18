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

#include "dawn/wire/WireCmd_autogen.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/Numeric.h"
#include "dawn/wire/BufferConsumer_impl.h"
#include "dawn/wire/Wire.h"

#include <algorithm>
#include <cstring>
#include <limits>

#if defined(__GNUC__) || defined(__clang__)
// error: 'offsetof' within non-standard-layout type 'wgpu::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

//* Helper macros so that the main [de]serialization functions can be written in a generic manner.

//* Outputs an rvalue that's the number of elements a pointer member points to.
{%- macro member_length(member, record_accessor) -%}
    {%- if member.length == "constant" -%}
        {{member.constant_length}}u
    {%- else -%}
        {{record_accessor}}{{as_varName(member.length.name)}}
    {%- endif -%}
{%- endmacro -%}

//* Outputs the type that will be used on the wire for the member
{%- macro member_transfer_type(type) -%}
    {%- if type.category == "object" -%}
        ObjectId
    {%- elif type.category == "structure" -%}
        {{as_cType(type.name)}}Transfer
    {%- elif as_cType(type.name) == "size_t" -%}
        {{as_cType(types["uint64_t"].name)}}
    {%- else -%}
        {%- do assert(type.is_wire_transparent, 'wire transparent') -%}
        {{as_cType(type.name)}}
    {%- endif -%}
{%- endmacro -%}

//* Outputs the size of one element of the type that will be used on the wire for the member
{%- macro member_transfer_sizeof(type) -%}
    sizeof({{member_transfer_type(type)}})
{%- endmacro -%}

//* Outputs the serialization code to put `in` in `out`
{%- macro serialize_member(type, optional, in, out) -%}
    {%- if type.category == "object" -%}
        {%- set Optional = "Optional" if optional else "" -%}
        WIRE_TRY(provider.Get{{Optional}}Id({{in}}, &{{out}}));
    {%- elif type.category == "structure" -%}
        //* Do not memcpy or we may serialize padding bytes which can leak information across a
        //* trusted boundary.
        {%- set Provider = ", provider" if type.may_have_dawn_object else "" -%}
        WIRE_TRY({{as_cType(type.name)}}Serialize({{in}}, &{{out}}, buffer{{Provider}}));
    {%- elif not is_wire_serializable(type) -%}
        if ({{in}} != nullptr) return WireResult::FatalError;
    {%- else -%}
        {{out}} = {{in}};
    {%- endif -%}
{%- endmacro -%}

//* Outputs the deserialization code to put `in` in `out`
{%- macro deserialize_member(type, optional, in, out) -%}
    {%- if type.category == "object" -%}
        {%- set Optional = "Optional" if optional else "" -%}
        WIRE_TRY(resolver.Get{{Optional}}FromId({{in}}, &{{out}}));
    {%- elif type.category == "structure" %}
        {% if type.is_wire_transparent %}
            static_assert(sizeof({{out}}) == sizeof({{in}}), "Deserialize memcpy size must match.");
                memcpy(&{{out}}, const_cast<const {{member_transfer_type(type)}}*>(&{{in}}), {{member_transfer_sizeof(type)}});
        {%- else %}
            WIRE_TRY({{as_cType(type.name)}}Deserialize(&{{out}}, &{{in}}, deserializeBuffer, allocator
                {%- if type.may_have_dawn_object -%}
                    , resolver
                {%- endif -%}
            ));
        {%- endif -%}
    {%- elif type.category == 'callback info' %}
        {{out}} = WGPU_{{type.name.SNAKE_CASE()}}_INIT;
    {%- elif not is_wire_serializable(type) %}
        {{out}} = nullptr;
    {%- elif type.name.get() == "size_t" -%}
        //* Deserializing into size_t requires check that the uint64_t used on the wire won't narrow.
        if ({{in}} > std::numeric_limits<size_t>::max()) return WireResult::FatalError;
            {{out}} = checked_cast<size_t>({{in}});
    {%- else -%}
        static_assert(sizeof({{out}}) >= sizeof({{in}}), "Deserialize assignment may not narrow.");
            {{out}} = {{in}};
    {%- endif -%}
{%- endmacro -%}

//* The main [de]serialization macro
//* Methods are very similar to structures that have one member corresponding to each arguments.
//* This macro takes advantage of the similarity to output [de]serialization code for a record
//* that is either a structure or a method, with some special cases for each.
{%- macro write_record_serialization_helpers(record, name, members, is_cmd=False, is_return_command=False) -%}
    {%- set Return = "Return" if is_return_command else "" -%}
    {%- set Cmd = "Cmd" if is_cmd else "" -%}
    {%- set Inherits = " : CmdHeader" if is_cmd else "" %}

    //* Structure for the wire format of each of the records. Members that are values
    //* are embedded directly in the structure. Other members are assumed to be in the
    //* memory directly following the structure in the buffer.
    struct {{Return}}{{name}}Transfer{{Inherits}} {
        static_assert({{[is_cmd, record.extensible, record.chained].count(True)}} <= 1,
                      "Record must be at most one of is_cmd, extensible, and chained.");
        {% if is_cmd %}
            //* Start the transfer structure with the command ID, so that casting to WireCmd gives the ID.
            {{Return}}WireCmd commandId;
        {% elif record.extensible %}
            WGPUBool hasNextInChain;
        {% elif record.chained %}
            WGPUChainedStructTransfer chain;
        {% endif %}

        {% for member in members %}
            {% if not is_wire_serializable(member.type) %}
                {% continue %}
            {% endif %}
            //* Value types are directly in the command, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {{member_transfer_type(member.type)}} {{as_varName(member.name)}};
                {% continue %}
            {% endif %}
            //* Optional members additionally come with a boolean to indicate whether they were set.
            {% if member.optional and member.type.category != "object" %}
                WGPUBool has_{{as_varName(member.name)}};
            {% endif %}
        {% endfor %}
    };

    {% if is_cmd %}
        static_assert(offsetof({{Return}}{{name}}Transfer, commandSize) == 0);
        static_assert(offsetof({{Return}}{{name}}Transfer, commandId) == sizeof(CmdHeader));
    {% endif -%}

    {% if record.chained %}
        static_assert(offsetof({{Return}}{{name}}Transfer, chain) == 0);
    {% endif %}

    //* Returns the required transfer size for `record` in addition to the transfer structure.
    [[maybe_unused]] size_t {{Return}}{{name}}GetExtraRequiredSize([[maybe_unused]] const {{Return}}{{name}}{{Cmd}}& record) {
        size_t result = 0;

        //* Gather how much space will be needed for the extension chain.
        {% if record.extensible %}
            const WGPUChainedStruct* next = record.nextInChain;
            while (next != nullptr) {
                switch (next->sType) {
                    {% for extension in record.extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CType = as_cType(extension.name) %}
                        case {{as_cEnum(types["s type"].name, extension.name)}}: {
                            const auto& typedStruct = *reinterpret_cast<{{CType}} const *>(next);
                            result += WireAlignSizeof<{{CType}}Transfer>();
                            result += {{CType}}GetExtraRequiredSize(typedStruct);
                            break;
                        }
                    {% endfor %}
                    default: {
                        result += WireAlignSizeof<WGPUDawnInjectedInvalidSTypeTransfer>();
                        break;
                    }
                }
                next = next->next;
            }
        {% endif %}
        //* Gather space needed for pointer members.
        {% for member in members %}
            {%- set memberName = as_varName(member.name) -%}
            //* Skip size computation if we are skipping serialization.
            {% if member.skip_serialize %}
                {% continue %}
            {% endif %}
            //* Normal handling for pointer members and structs.
            {% if member.annotation != "value" %}
                {% if member.type.category != "object" and member.optional %}
                    if (record.{{as_varName(member.name)}} != nullptr)
                {% endif %}
                {
                    {% do assert(member.annotation != "const*const*", "const*const* not valid here") %}
                    auto memberLength = {{member_length(member, "record.")}};
                    auto size = WireAlignSizeofN<{{member_transfer_type(member.type)}}>(memberLength);
                    DAWN_ASSERT(size);
                    result += *size;
                    //* Structures might contain more pointers so we need to add their extra size as well.
                    {% if member.type.category == "structure" %}
                        for (decltype(memberLength) i = 0; i < memberLength; ++i) {
                            {% do assert(member.annotation == "const*" or member.annotation == "*", "unhandled annotation: " + member.annotation)%}
                            result += {{as_cType(member.type.name)}}GetExtraRequiredSize(record.{{as_varName(member.name)}}[i]);
                        }
                    {% endif %}
                }
            {% elif member.type.category == "structure" %}
                result += {{as_cType(member.type.name)}}GetExtraRequiredSize(record.{{as_varName(member.name)}});
            {% endif %}
        {% endfor %}
        return result;
    }
    // GetExtraRequiredSize isn't used for structures that are value members of other structures
    // because we assume they cannot contain pointers themselves.
    DAWN_UNUSED_FUNC({{Return}}{{name}}GetExtraRequiredSize);

    //* Serializes `record` into `transfer`, using `buffer` to get more space for pointed-to data
    //* and `provider` to serialize objects.
    [[maybe_unused]] WireResult {{Return}}{{name}}Serialize(
        const {{Return}}{{name}}{{Cmd}}& record,
        {{Return}}{{name}}Transfer* transfer,
        [[maybe_unused]] SerializeBuffer* buffer
        {%- if record.may_have_dawn_object -%}
            , const ObjectIdProvider& provider
        {%- endif -%}
    ) {
        //* Handle special transfer members of methods.
        {% if is_cmd %}
            transfer->commandId = {{Return}}WireCmd::{{name}};
        {% endif %}

        {% if record.extensible %}
            const WGPUChainedStruct* next = record.nextInChain;
            transfer->hasNextInChain = false;
            while (next != nullptr) {
                transfer->hasNextInChain = true;
                switch (next->sType) {
                    {% for extension in record.extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CType = as_cType(extension.name) %}
                        case {{as_cEnum(types["s type"].name, extension.name)}}: {
                            {{CType}}Transfer* chainTransfer;
                            WIRE_TRY(buffer->Next(&chainTransfer));
                            chainTransfer->chain.sType = next->sType;
                            chainTransfer->chain.hasNext = next->next != nullptr;

                            WIRE_TRY({{CType}}Serialize(*reinterpret_cast<{{CType}} const*>(next), chainTransfer, buffer, provider));
                            break;
                        }
                    {% endfor %}
                    default: {
                        // Invalid enum. Serialize just the invalid sType for validation purposes.
                        dawn::WarningLog() << "Unknown sType " << next->sType << " discarded.";

                        WGPUDawnInjectedInvalidSTypeTransfer* chainTransfer;
                        WIRE_TRY(buffer->Next(&chainTransfer));
                        chainTransfer->chain.sType = WGPUSType_DawnInjectedInvalidSType;
                        chainTransfer->chain.hasNext = next->next != nullptr;
                        chainTransfer->invalidSType = next->sType;
                        break;
                    }
                }
                next = next->next;
            }
        {% endif %}
        {% if record.chained %}
            //* Should be set by the root descriptor's call to SerializeChainedStruct.
            DAWN_ASSERT(transfer->chain.sType == {{as_cEnum(types["s type"].name, record.name)}});
            DAWN_ASSERT(transfer->chain.hasNext == (record.chain.next != nullptr));
        {% endif %}

        //* Iterate members, sorted in reverse on "attribute" so that "value" types are serialized first.
        //* Note this is important because some array pointer members rely on another "value" for their
        //* "length", but order is not always given.
        {% for member in members | sort(reverse=true, attribute="annotation") %}
            {% set memberName = as_varName(member.name) %}
            //* Skip serialization for custom serialized members and callback infos.
            {% if member.skip_serialize or member.type.category == 'callback info' %}
                {% continue %}
            {% endif %}
            //* Value types are directly in the transfer record, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {{serialize_member(member.type, member.optional, "record." + memberName, "transfer->" + memberName)}}
                {% continue %}
            {% endif %}
            //* Allocate space and write the non-value arguments in it.
            {% do assert(member.annotation != "const*const*") %}
            {% if member.type.category != "object" and member.optional %}
                bool has_{{memberName}} = record.{{memberName}} != nullptr;
                transfer->has_{{memberName}} = has_{{memberName}};
                if (has_{{memberName}}) {
            {% else %}
                {
            {% endif %}
                auto memberLength = {{member_length(member, "record.")}};

                {{member_transfer_type(member.type)}}* memberBuffer;
                WIRE_TRY(buffer->NextN(memberLength, &memberBuffer));

                {% if member.type.is_wire_transparent %}
                    //* memcpy is not defined for null pointers, even when the length is zero.
                    //* conflicts with the common practice to use (nullptr, 0) to represent an
                    //* span. Guard memcpy with a zero check to work around this language bug.
                    if (memberLength != 0) {
                        memcpy(
                            memberBuffer, record.{{memberName}},
                            {{member_transfer_sizeof(member.type)}} * memberLength);
                    }
                {% else %}
                    //* This loop cannot overflow because it iterates up to |memberLength|. Even if
                    //* memberLength were the maximum integer value, |i| would become equal to it
                    //* just before exiting the loop, but not increment past or wrap around.
                    for (decltype(memberLength) i = 0; i < memberLength; ++i) {
                        {{serialize_member(member.type, member.array_element_optional, "record." + memberName + "[i]", "memberBuffer[i]" )}}
                    }
                {% endif %}
            }
        {% endfor %}

        return WireResult::Success;
    }
    DAWN_UNUSED_FUNC({{Return}}{{name}}Serialize);

    //* Deserializes `transfer` into `record` getting more serialized data from `buffer` and `size`
    //* if needed, using `allocator` to store pointed-to values and `resolver` to translate object
    //* Ids to actual objects.
    [[maybe_unused]] WireResult {{Return}}{{name}}Deserialize(
        {{Return}}{{name}}{{Cmd}}* record,
        const volatile {{Return}}{{name}}Transfer* transfer,
        DeserializeBuffer* deserializeBuffer,
        [[maybe_unused]] DeserializeAllocator* allocator
        {%- if record.may_have_dawn_object -%}
            , const ObjectIdResolver& resolver
        {%- endif -%}) {
        {% if is_cmd %}
            DAWN_ASSERT(transfer->commandId == {{Return}}WireCmd::{{name}});
        {% endif %}
        {% if record.derived_method %}
            record->selfId = transfer->self;
        {% endif %}

        {% if record.extensible %}
            WGPUChainedStruct** outChainNext = &record->nextInChain;
            bool hasNext = transfer->hasNextInChain;
            while (hasNext) {
                const volatile WGPUChainedStructTransfer* header;
                WIRE_TRY(deserializeBuffer->Peek(&header));
                WGPUSType sType = header->sType;
                hasNext = header->hasNext;

                switch (sType) {
                    //* All extensible types need to be able to handle deserializing the invalid
                    //* sType struct.
                    {% set extensions = record.extensions + [types['dawn injected invalid s type']] %}
                    {% for extension in extensions if extension.name.CamelCase() not in client_side_structures %}
                        {% set CType = as_cType(extension.name) %}
                        case {{as_cEnum(types["s type"].name, extension.name)}}: {
                            const volatile {{CType}}Transfer* chainTransfer;
                            WIRE_TRY(deserializeBuffer->Read(&chainTransfer));

                            {{CType}}* typedOutStruct;
                            WIRE_TRY(GetSpace(allocator, 1u, &typedOutStruct));
                            typedOutStruct->chain.sType = sType;
                            typedOutStruct->chain.next = nullptr;
                            WIRE_TRY({{CType}}Deserialize(typedOutStruct, chainTransfer,
                                                          deserializeBuffer, allocator, resolver));
                            *outChainNext = &typedOutStruct->chain;
                            outChainNext = &typedOutStruct->chain.next;
                            break;
                        }
                    {% endfor %}
                    default: {
                        //* For invalid sTypes, it's a fatal error since this implies a compromised
                        //* or corrupt client.
                        return WireResult::FatalError;
                    }
                }
            }
            *outChainNext = nullptr;
        {% endif %}
        {% if record.chained %}
            //* Should be set by the root descriptor's call to DeserializeChainedStruct.
            //* Don't check |record->chain.next| matches because it is not set until the
            //* next iteration inside DeserializeChainedStruct.
            DAWN_ASSERT(record->chain.sType == {{as_cEnum(types["s type"].name, record.name)}});
            DAWN_ASSERT(record->chain.next == nullptr);
        {% endif %}

        //* Iterate members, sorted in reverse on "attribute" so that "value" types are serialized first.
        //* Note this is important because some array pointer members rely on another "value" for their
        //* "length", but order is not always given.
        {% for member in members | sort(reverse=true, attribute="annotation") %}
            {% set memberName = as_varName(member.name) %}
            //* Value types are directly in the transfer record, objects being replaced with their IDs.
            {% if member.annotation == "value" %}
                {{deserialize_member(member.type, member.optional, "transfer->" + memberName, "record->" + memberName)}}
                {% continue %}
            {% endif %}
            //* Get extra buffer data, and copy pointed to values in extra allocated space. Note that
            //* currently, there is an implicit restriction that "skip_serialize" members must not be be
            //* a "value" type, and that they are the last non-"value" type specified in the list in
            //* dawn_wire.json.
            {% do assert(member.annotation != "const*const*") %}
            {% if member.type.category != "object" and member.optional %}
                //* Non-constant length optional members use length=0 to denote they aren't present.
                //* Otherwise we could have length=N and has_member=false, causing reads from an
                //* uninitialized pointer.
                {% do assert(member.length == "constant") %}
                bool has_{{memberName}} = transfer->has_{{memberName}};
                record->{{memberName}} = nullptr;
                if (has_{{memberName}}) {
            {% else %}
                {
            {% endif %}
                auto memberLength = {{member_length(member, "record->")}};
                const volatile {{member_transfer_type(member.type)}}* memberBuffer;
                WIRE_TRY(deserializeBuffer->ReadN(memberLength, &memberBuffer));

                //* For data-only members (e.g. "data" in WriteBuffer and WriteTexture), they are
                //* not security sensitive so we can directly refer the data inside the transfer
                //* buffer in dawn_native. For other members, as prevention of TOCTOU attacks is an
                //* important feature of the wire, we must make sure every single value returned to
                //* dawn_native must be a copy of what's in the wire.
                {% if member.json_data["wire_is_data_only"] %}
                    record->{{memberName}} =
                        const_cast<const {{member_transfer_type(member.type)}}*>(memberBuffer);

                {% else %}
                    {{as_cType(member.type.name)}}* copiedMembers;
                    WIRE_TRY(GetSpace(allocator, memberLength, &copiedMembers));
                    record->{{memberName}} = copiedMembers;

                    {% if member.type.is_wire_transparent %}
                        //* memcpy is not defined for null pointers, even when the length is zero.
                        //* conflicts with the common practice to use (nullptr, 0) to represent an
                        //* span. Guard memcpy with a zero check to work around this language bug.
                        if (memberLength != 0) {
                            //* memcpy is not allowed to copy from volatile objects. However, these
                            //* arrays are just used as plain data, and don't impact control flow.
                            //* So if the underlying data were changed while the copy was still
                            //* executing, we would get different data - but it wouldn't cause
                            //* unexpected downstream effects.
                            memcpy(
                                copiedMembers,
                                const_cast<const {{member_transfer_type(member.type)}}*>(memberBuffer),
                              {{member_transfer_sizeof(member.type)}} * memberLength);
                        }
                    {% else %}
                        //* This loop cannot overflow because it iterates up to |memberLength|. Even
                        //* if memberLength were the maximum integer value, |i| would become equal
                        //* to it just before exiting the loop, but not increment past or wrap
                        //* around.
                        for (decltype(memberLength) i = 0; i < memberLength; ++i) {
                            {{deserialize_member(member.type, member.array_element_optional, "memberBuffer[i]", "copiedMembers[i]")}}
                        }
                    {% endif %}
                {% endif %}
            }
        {% endfor %}

        return WireResult::Success;
    }
    DAWN_UNUSED_FUNC({{Return}}{{name}}Deserialize);
{%- endmacro -%}

{%- macro write_command_serialization_methods(command, is_return) -%}
    {% set Return = "Return" if is_return else "" %}
    {% set Name = Return + command.name.CamelCase() %}
    {% set Cmd = Name + "Cmd" %}
    size_t {{Cmd}}::GetRequiredSize() const {
        return WireAlignSizeof<{{Name}}Transfer>() + {{Name}}GetExtraRequiredSize(*this);
    }

    {% if command.may_have_dawn_object %}
        WireResult {{Cmd}}::Serialize(
            size_t commandSize,
            SerializeBuffer* serializeBuffer,
            const ObjectIdProvider& provider) const {
            {{Name}}Transfer* transfer;
            WIRE_TRY(serializeBuffer->Next(&transfer));
            transfer->commandSize = commandSize;
            return ({{Name}}Serialize(*this, transfer, serializeBuffer, provider));
        }
        WireResult {{Cmd}}::Serialize(size_t commandSize, SerializeBuffer* serializeBuffer) const {
            ErrorObjectIdProvider provider;
            return Serialize(commandSize, serializeBuffer, provider);
        }

        WireResult {{Cmd}}::Deserialize(
            DeserializeBuffer* deserializeBuffer,
            DeserializeAllocator* allocator,
            const ObjectIdResolver& resolver) {
            const volatile {{Name}}Transfer* transfer;
            WIRE_TRY(deserializeBuffer->Read(&transfer));
            return {{Name}}Deserialize(this, transfer, deserializeBuffer, allocator, resolver);
        }
        WireResult {{Cmd}}::Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator) {
            ErrorObjectIdResolver resolver;
            return Deserialize(deserializeBuffer, allocator, resolver);
        }
    {% else %}
        WireResult {{Cmd}}::Serialize(size_t commandSize, SerializeBuffer* serializeBuffer) const {
            {{Name}}Transfer* transfer;
            WIRE_TRY(serializeBuffer->Next(&transfer));
            transfer->commandSize = commandSize;
            return ({{Name}}Serialize(*this, transfer, serializeBuffer));
        }
        WireResult {{Cmd}}::Serialize(
            size_t commandSize,
            SerializeBuffer* serializeBuffer,
            const ObjectIdProvider&) const {
            return Serialize(commandSize, serializeBuffer);
        }

        WireResult {{Cmd}}::Deserialize(DeserializeBuffer* deserializeBuffer, DeserializeAllocator* allocator) {
            const volatile {{Name}}Transfer* transfer;
            WIRE_TRY(deserializeBuffer->Read(&transfer));
            return {{Name}}Deserialize(this, transfer, deserializeBuffer, allocator);
        }
        WireResult {{Cmd}}::Deserialize(
            DeserializeBuffer* deserializeBuffer,
            DeserializeAllocator* allocator,
            const ObjectIdResolver&) {
            return Deserialize(deserializeBuffer, allocator);
        }
    {% endif %}
{% endmacro %}

namespace dawn::wire {
namespace {

// Allocates enough space from allocator to countain T[count] and return it in out.
// Return FatalError if the allocator couldn't allocate the memory.
// Always writes to |out| on success.
template <typename T, typename N>
WireResult GetSpace(DeserializeAllocator* allocator, N count, T** out) {
    // Because we use this function extensively when `count` == 1, we can optimize the
    // size computations a bit more for those cases via constexpr version of the
    // alignment computation.
    constexpr size_t kSizeofT = WireAlignSizeof<T>();
    size_t size = 0;
    if (count == 1) {
      size = kSizeofT;
    } else {
      auto sizeN = WireAlignSizeofN<T>(count);
      // A size of 0 indicates an overflow, so return an error.
      if (!sizeN) {
        return WireResult::FatalError;
      }
      size = *sizeN;
    }

    *out = static_cast<T*>(allocator->GetSpace(size));
    if (*out == nullptr) {
        return WireResult::FatalError;
    }

    return WireResult::Success;
}

struct WGPUChainedStructTransfer {
    WGPUSType sType;
    bool hasNext;
};

//* Structs that need special handling for [de]serialization code generation.
{% set SpecialSerializeStructs = ["string view", "dawn injected invalid s type"] %}

// Manually define serialization and deserialization for WGPUStringView because
// it has a special encoding where:
//  { .data = nullptr, .length = WGPU_STRLEN }  --> nil
//  { .data = non-null, .length = WGPU_STRLEN } --> null-terminated, use strlen
//  { .data = ..., .length = 0 }             --> ""
//  { .data = ..., .length > 0 }             --> string of size `length`
struct WGPUStringViewTransfer {
    bool has_data;
    uint64_t length;
};

size_t WGPUStringViewGetExtraRequiredSize(const WGPUStringView& record) {
    size_t size = record.length;
    if (size == WGPU_STRLEN) {
        // This is a null-terminated string, or it's nil.
        size = record.data ? std::strlen(record.data) : 0;
    }
    return Align(size, kWireBufferAlignment);
}

WireResult WGPUStringViewSerialize(
    const WGPUStringView& record,
    WGPUStringViewTransfer* transfer,
    SerializeBuffer* buffer) {

    bool has_data = record.data != nullptr;
    uint64_t length = record.length;
    transfer->has_data = has_data;

    if (!has_data) {
        transfer->length = length;
        return WireResult::Success;
    }
    if (length == WGPU_STRLEN) {
        length = std::strlen(record.data);
    }
    if (length > 0) {
        char* memberBuffer;
        WIRE_TRY(buffer->NextN(length, &memberBuffer));
        memcpy(memberBuffer, record.data, length);
    }
    transfer->length = length;
    return WireResult::Success;
}

WireResult WGPUStringViewDeserialize(
    WGPUStringView* record,
    const volatile WGPUStringViewTransfer* transfer,
    DeserializeBuffer* deserializeBuffer,
    DeserializeAllocator* allocator) {

    bool has_data = transfer->has_data;
    uint64_t length = transfer->length;

    if (length > WGPU_STRLEN) {
        return WireResult::FatalError;
    }
    if (!has_data) {
        record->data = nullptr;
        if (length != 0 && length != WGPU_STRLEN) {
            // Invalid string.
            return WireResult::FatalError;
        }
        record->length = static_cast<size_t>(length);
        return WireResult::Success;
    }
    if (length == 0) {
        record->data = "";
        record->length = 0;
        return WireResult::Success;
    }

    size_t stringLength = static_cast<size_t>(length);
    const volatile char* stringInBuffer;
    WIRE_TRY(deserializeBuffer->ReadN(stringLength, &stringInBuffer));

    char* copiedString;
    WIRE_TRY(GetSpace(allocator, stringLength, &copiedString));
    memcpy(copiedString, const_cast<const char*>(stringInBuffer), stringLength);

    record->data = copiedString;
    record->length = stringLength;
    return WireResult::Success;
}

//* Force generation of de[serialization] methods for WGPUDawnInjectedInvalidSType early.
{% set type = types["dawn injected invalid s type"] %}
{%- set name = as_cType(type.name) -%}
{{write_record_serialization_helpers(type, name, type.members, is_cmd=False)}}

//* Output structure [de]serialization first because it is used by commands.
{% for type in by_category["structure"] %}
    {%- set name = as_cType(type.name) -%}
    {% if type.name.CamelCase() not in client_side_structures and type.name.get() not in SpecialSerializeStructs -%}
        {{write_record_serialization_helpers(type, name, type.members, is_cmd=False)}}
    {% endif %}
{% endfor %}

//* Generate the list of sTypes that we need to handle.
{% set sTypes = [] %}
{% for sType in types["s type"].values %}
    {% if not sType.valid %}
        {% continue %}
    {% elif sType.name.CamelCase() in client_side_structures %}
        {% continue %}
    {% endif %}
    {% do sTypes.append(sType) %}
{% endfor %}

//* Output [de]serialization helpers for commands
{% for command in cmd_records["command"] %}
    {%- set name = command.name.CamelCase() -%}
    {{write_record_serialization_helpers(command, name, command.members, is_cmd=True)}}
{% endfor %}

//* Output [de]serialization helpers for return commands
{% for command in cmd_records["return command"] %}
    {%- set name = command.name.CamelCase() -%}
    {{write_record_serialization_helpers(command, name, command.members,
                                         is_cmd=True, is_return_command=True)}}
{% endfor %}

// Implementation of ObjectIdResolver that always errors.
// Used when the generator adds a provider argument because of a chained
// struct, but in practice, a chained struct in that location is invalid.
class ErrorObjectIdResolver final : public ObjectIdResolver {
    public:
    {% for type in by_category["object"] %}
      WireResult GetFromId(ObjectId id, {{as_cType(type.name)}}* out) const override {
          return WireResult::FatalError;
      }
      WireResult GetOptionalFromId(ObjectId id, {{as_cType(type.name)}}* out) const override {
          return WireResult::FatalError;
      }
    {% endfor %}
};

// Implementation of ObjectIdProvider that always errors.
// Used when the generator adds a provider argument because of a chained
// struct, but in practice, a chained struct in that location is invalid.
class ErrorObjectIdProvider final : public ObjectIdProvider {
    public:
    {% for type in by_category["object"] %}
      WireResult GetId({{as_cType(type.name)}} object, ObjectId* out) const override {
          return WireResult::FatalError;
      }
      WireResult GetOptionalId({{as_cType(type.name)}} object, ObjectId* out) const override {
          return WireResult::FatalError;
      }
    {% endfor %}
};

}  // anonymous namespace

{% for command in cmd_records["command"] -%}
    {{write_command_serialization_methods(command, False)}}
{% endfor %}

{% for command in cmd_records["return command"] -%}
    {{write_command_serialization_methods(command, True)}}
{% endfor %}

}  // namespace dawn::wire
