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

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::alignment(
        BufferInterfaceBlock::Alignment alignment) {
    mAlignment = alignment;
    return *this;
}

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::target(
        BufferInterfaceBlock::Target target) {
    mTarget = target;
    return *this;
}

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::qualifier(
        BufferInterfaceBlock::Qualifier qualifier) {
    mQualifiers |= uint8_t(qualifier);
    return *this;
}

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::add(
        std::initializer_list<InterfaceBlockEntry> list) {
    mEntries.reserve(mEntries.size() + list.size());
    for (auto const& item : list) {
        mEntries.push_back({
                { item.name.data(), item.name.size() },
                0, uint8_t(item.stride), item.type, item.size > 0, item.size, item.precision, item.minFeatureLevel,
                { item.structName.data(), item.structName.size() },
                { item.sizeName.data(), item.sizeName.size() }
        });
    }
    return *this;
}

BufferInterfaceBlock::Builder& BufferInterfaceBlock::Builder::addVariableSizedArray(
        BufferInterfaceBlock::InterfaceBlockEntry const& item) {
    mHasVariableSizeArray = true;
    mEntries.push_back({
            { item.name.data(), item.name.size() },
            0, uint8_t(item.stride), item.type, true, 0, item.precision, item.minFeatureLevel,
            { item.structName.data(), item.structName.size() },
            { item.sizeName.data(), item.sizeName.size() }
    });
    return *this;
}

BufferInterfaceBlock BufferInterfaceBlock::Builder::build() {
    // look for the first variable-size array
    auto pos = std::find_if(mEntries.begin(), mEntries.end(),
            [](FieldInfo const& item) -> bool {
        return item.isArray && !item.size;
    });

    // if there is one, check it's the last entry
    FILAMENT_CHECK_PRECONDITION(pos == mEntries.end() || pos == mEntries.end() - 1)
            << "the variable-size array must be the last entry";

    // if we have a variable size array, we can't be a UBO
    FILAMENT_CHECK_PRECONDITION(pos == mEntries.end() || mTarget == Target::SSBO)
            << "variable size arrays not supported for UBOs";

    // std430 not available for UBOs
    FILAMENT_CHECK_PRECONDITION(mAlignment == Alignment::std140 || mTarget == Target::SSBO)
            << "UBOs must use std140";

    return BufferInterfaceBlock(*this);
}

bool BufferInterfaceBlock::Builder::hasVariableSizeArray() const {
    return mHasVariableSizeArray;
}

// --------------------------------------------------------------------------------------------------------------------

BufferInterfaceBlock::BufferInterfaceBlock() = default;
BufferInterfaceBlock::BufferInterfaceBlock(BufferInterfaceBlock&& rhs) noexcept = default;
BufferInterfaceBlock& BufferInterfaceBlock::operator=(BufferInterfaceBlock&& rhs) noexcept = default;
BufferInterfaceBlock::~BufferInterfaceBlock() noexcept = default;

BufferInterfaceBlock::BufferInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName),
      mFieldInfoList(builder.mEntries.size()),
      mAlignment(builder.mAlignment),
      mTarget(builder.mTarget)
{
    auto& infoMap = mInfoMap;
    infoMap.reserve(builder.mEntries.size());

    auto& uniformsInfoList = mFieldInfoList;

    uint32_t i = 0;
    uint16_t offset = 0;
    for (auto const& e : builder.mEntries) {
        size_t alignment = baseAlignmentForType(e.type);
        size_t stride = strideForType(e.type, e.stride);

        if (e.isArray) { // this is an array
            if (builder.mAlignment == Alignment::std140) {
                // in std140 arrays are aligned to float4
                alignment = 4;
            }
            // the stride of an array is always rounded to its alignment (which is POT)
            stride = (stride + alignment - 1) & ~(alignment - 1);
        }

        // calculate the offset for this uniform
        size_t padding = (alignment - (offset % alignment)) % alignment;
        offset += padding;

        FieldInfo& info = uniformsInfoList[i];
        info = { e.name, offset, uint8_t(stride), e.type, e.isArray, e.size,
                 e.precision, e.minFeatureLevel, e.structName, e.sizeName };

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
    FILAMENT_CHECK_PRECONDITION(pos != mInfoMap.end()) << "uniform named \""
            << name << "\" not found";
    return &mFieldInfoList[pos->second];
}

bool BufferInterfaceBlock::isEmptyForFeatureLevel(
        backend::FeatureLevel featureLevel) const noexcept {
    return std::all_of(mFieldInfoList.begin(), mFieldInfoList.end(),
                       [featureLevel](auto const &info) {
                           return featureLevel < info.minFeatureLevel;
                       });
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
