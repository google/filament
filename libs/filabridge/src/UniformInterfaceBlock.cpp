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

#include "private/filament/UniformInterfaceBlock.h"

#include <utils/Panic.h>
#include <utils/compiler.h>

using namespace utils;

namespace filament {

UniformInterfaceBlock::Builder&
UniformInterfaceBlock::Builder::name(utils::CString const& interfaceBlockName) {
    mName = interfaceBlockName;
    return *this;
}

UniformInterfaceBlock::Builder&
UniformInterfaceBlock::Builder::name(utils::CString&& interfaceBlockName) {
    interfaceBlockName.swap(mName);
    return *this;
}

UniformInterfaceBlock::Builder&
UniformInterfaceBlock::Builder::name(utils::StaticString const& interfaceBlockName) {
    mName = CString{ interfaceBlockName };
    return *this;
}

UniformInterfaceBlock::Builder& UniformInterfaceBlock::Builder::add(
        utils::CString const& uniformName, size_t size, UniformInterfaceBlock::Type type,
        UniformInterfaceBlock::Precision precision) {
    mEntries.emplace_back(uniformName, (uint32_t)size, type, precision);
    return *this;
}

UniformInterfaceBlock::Builder& UniformInterfaceBlock::Builder::add(
        utils::CString&& uniformName, size_t size, UniformInterfaceBlock::Type type,
        UniformInterfaceBlock::Precision precision) {
    mEntries.emplace_back(std::move(uniformName), (uint32_t)size, type, precision);
    return *this;
}

UniformInterfaceBlock::Builder& UniformInterfaceBlock::Builder::add(
        utils::StaticString const& uniformName, size_t size, UniformInterfaceBlock::Type type,
        UniformInterfaceBlock::Precision precision) {
    mEntries.emplace_back(uniformName, (uint32_t)size, type, precision);
    return *this;
}

UniformInterfaceBlock::Builder& UniformInterfaceBlock::Builder::add(
        utils::CString const& uniformName, size_t size,
        utils::CString const& structName, size_t stride) {
    mEntries.emplace_back(uniformName, (uint32_t)size, structName, stride);
    return *this;
}


UniformInterfaceBlock UniformInterfaceBlock::Builder::build() {
    return UniformInterfaceBlock(*this);
}

// --------------------------------------------------------------------------------------------------------------------

UniformInterfaceBlock::UniformInterfaceBlock() = default;
UniformInterfaceBlock::UniformInterfaceBlock(const UniformInterfaceBlock& rhs) = default;
UniformInterfaceBlock::UniformInterfaceBlock(UniformInterfaceBlock&& rhs) noexcept /* = default */ {};
UniformInterfaceBlock& UniformInterfaceBlock::operator=(const UniformInterfaceBlock& rhs) = default;
UniformInterfaceBlock& UniformInterfaceBlock::operator=(UniformInterfaceBlock&& rhs) /*noexcept*/ = default;
UniformInterfaceBlock::~UniformInterfaceBlock() noexcept = default;

UniformInterfaceBlock::UniformInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName)
{
    auto& infoMap = mInfoMap;
    auto& uniformsInfoList = mUniformsInfoList;
    infoMap.reserve(builder.mEntries.size());
    uniformsInfoList.resize(builder.mEntries.size());

    uint32_t i = 0;
    uint16_t offset = 0;
    for (auto const& e : builder.mEntries) {
        size_t alignment = baseAlignmentForType(e.type);
        uint8_t stride = strideForType(e.type, e.stride);
        if (e.size > 1) { // this is an array
            // round the alignment up to that of a float4
            alignment = (alignment + 3) & ~3;
            stride = (stride + uint8_t(3)) & ~uint8_t(3);
        }

        // calculate the offset for this uniform
        size_t padding = (alignment - (offset % alignment)) % alignment;
        offset += padding;

        UniformInfo& info = uniformsInfoList[i];
        info = { e.name, offset, stride, e.type, e.size, e.precision, e.structName };

        // record this uniform info
        infoMap[info.name.c_str()] = i;

        // advance offset to next slot
        offset += stride * e.size;
        ++i;
    }

    // round size to the next multiple of 4 and convert to bytes
    mSize = sizeof(uint32_t) * ((offset + 3) & ~3);
}

ssize_t UniformInterfaceBlock::getUniformOffset(const char* name, size_t index) const {
    auto const* info = getUniformInfo(name);
    if (!info) {
        return -1;
    }
    return info->getBufferOffset(index);
}

UniformInterfaceBlock::UniformInfo const* UniformInterfaceBlock::getUniformInfo(const char* name) const {
    auto const& pos = mInfoMap.find(name);
    if (!ASSERT_PRECONDITION_NON_FATAL(pos != mInfoMap.end(), "uniform named \"%s\" not found", name)) {
        return nullptr;
    }
    return &mUniformsInfoList[pos->second];
}


uint8_t UTILS_NOINLINE UniformInterfaceBlock::baseAlignmentForType(UniformInterfaceBlock::Type type) noexcept {
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

uint8_t UTILS_NOINLINE UniformInterfaceBlock::strideForType(UniformInterfaceBlock::Type type, uint32_t stride) noexcept {
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

