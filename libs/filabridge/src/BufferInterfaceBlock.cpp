/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "private/filament/BufferInterfaceBlock.h"

#include <utils/Panic.h>
#include <utils/compiler.h>

#include <utility>

using namespace utils;

namespace filament {

BufferInterfaceBlock::Builder::Builder() noexcept = default;
BufferInterfaceBlock::Builder::~Builder() noexcept = default;

BufferInterfaceBlock::Builder&
BufferInterfaceBlock::Builder::name(std::string_view interfaceBlockName) {
    mName = { interfaceBlockName.data(), interfaceBlockName.size() };
    return *this;
}

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::add(
        std::initializer_list<InterfaceBlockEntry> list) {
    mEntries.reserve(mEntries.size() + list.size());
    for (auto const& item : list) {
        mEntries.push_back({
                { item.name.data(), item.name.size() },
                0, uint8_t(item.stride), item.type, item.size, item.precision,
                { item.structName.data(), item.structName.size() },
                { item.sizeName.data(), item.sizeName.size() }
        });
    }
    return *this;
}

BufferInterfaceBlock BufferInterfaceBlock::Builder::build() {
    return BufferInterfaceBlock(*this);
}

// --------------------------------------------------------------------------------------------------------------------

BufferInterfaceBlock::BufferInterfaceBlock() = default;
BufferInterfaceBlock::BufferInterfaceBlock(BufferInterfaceBlock&& rhs) noexcept = default;
BufferInterfaceBlock& BufferInterfaceBlock::operator=(BufferInterfaceBlock&& rhs) noexcept = default;
BufferInterfaceBlock::~BufferInterfaceBlock() noexcept = default;

BufferInterfaceBlock::BufferInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName), mFieldInfoList(builder.mEntries.size())
{
    auto& infoMap = mInfoMap;
    infoMap.reserve(builder.mEntries.size());

    auto& uniformsInfoList = mFieldInfoList;

    uint32_t i = 0;
    uint16_t offset = 0;
    for (auto const& e : builder.mEntries) {
        size_t alignment = baseAlignmentForType(e.type);
        uint8_t stride = strideForType(e.type, e.stride);
        if (e.size > 0) { // this is an array
            // round the alignment up to that of a float4
            alignment = (alignment + 3) & ~3;
            stride = (stride + uint8_t(3)) & ~uint8_t(3);
        }

        // calculate the offset for this uniform
        size_t padding = (alignment - (offset % alignment)) % alignment;
        offset += padding;

        FieldInfo& info = uniformsInfoList[i];
        info = { e.name, offset, stride, e.type, e.size, e.precision, e.structName, e.sizeName };

        // record this uniform info
        infoMap[{ info.name.data(), info.name.size() }] = i;

        // advance offset to next slot
        offset += stride * std::max(1u, e.size);
        ++i;
    }

    // round size to the next multiple of 4 and convert to bytes
    mSize = sizeof(uint32_t) * ((offset + 3) & ~3);
}

ssize_t BufferInterfaceBlock::getFieldOffset(std::string_view name, size_t index) const {
    auto const* info = getFieldInfo(name);
    assert_invariant(info);
    return (ssize_t)info->getBufferOffset(index);
}

BufferInterfaceBlock::FieldInfo const* BufferInterfaceBlock::getFieldInfo(
        std::string_view name) const {
    auto pos = mInfoMap.find(name);
    ASSERT_PRECONDITION(pos != mInfoMap.end(),
            "uniform named \"%.*s\" not found", name.size(), name.data());
    return &mFieldInfoList[pos->second];
}

uint8_t UTILS_NOINLINE BufferInterfaceBlock::baseAlignmentForType(BufferInterfaceBlock::Type type) noexcept {
    switch (type) {
        case Type::BOOL:
        case Type::FLOAT:
        case Type::INT:
        case Type::UINT:
            return 1;
        case Type::BOOL2:
        case Type::FLOAT2:
        case Type::INT2:
        case Type::UINT2:
            return 2;
        case Type::BOOL3:
        case Type::BOOL4:
        case Type::FLOAT3:
        case Type::FLOAT4:
        case Type::INT3:
        case Type::INT4:
        case Type::UINT3:
        case Type::UINT4:
        case Type::MAT3:
        case Type::MAT4:
        case Type::STRUCT:
            return 4;
    }
}

uint8_t UTILS_NOINLINE BufferInterfaceBlock::strideForType(BufferInterfaceBlock::Type type, uint32_t stride) noexcept {
    switch (type) {
        case Type::BOOL:
        case Type::INT:
        case Type::UINT:
        case Type::FLOAT:
            return 1;
        case Type::BOOL2:
        case Type::INT2:
        case Type::UINT2:
        case Type::FLOAT2:
            return 2;
        case Type::BOOL3:
        case Type::INT3:
        case Type::UINT3:
        case Type::FLOAT3:
            return 3;
        case Type::BOOL4:
        case Type::INT4:
        case Type::UINT4:
        case Type::FLOAT4:
            return 4;
        case Type::MAT3:
            return 12;
        case Type::MAT4:
            return 16;
        case Type::STRUCT:
            return stride;
    }
}

} // namespace filament

