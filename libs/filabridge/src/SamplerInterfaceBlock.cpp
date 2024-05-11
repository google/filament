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

#include "private/filament/SamplerInterfaceBlock.h"

#include <utils/Panic.h>
#include <utils/compiler.h>

#include <utility>

#include <stdint.h>

using namespace utils;

namespace filament {

SamplerInterfaceBlock::Builder::Builder() = default;
SamplerInterfaceBlock::Builder::~Builder() noexcept = default;

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::name(std::string_view interfaceBlockName) {
    mName = { interfaceBlockName.data(), interfaceBlockName.size() };
    return *this;
}

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::stageFlags(backend::ShaderStageFlags stageFlags) {
    mStageFlags = stageFlags;
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        std::string_view samplerName, Type type, Format format,
        Precision precision, bool multisample) noexcept {
    mEntries.push_back({
            { samplerName.data(), samplerName.size() }, { },
            uint8_t(mEntries.size()), type, format, precision, multisample });
    return *this;
}

SamplerInterfaceBlock SamplerInterfaceBlock::Builder::build() {
    return SamplerInterfaceBlock(*this);
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        std::initializer_list<ListEntry> list) noexcept {
    for (auto& e : list) {
        add(e.name, e.type, e.format, e.precision, e.multisample);
    }
    return *this;
}

// -------------------------------------------------------------------------------------------------

SamplerInterfaceBlock::SamplerInterfaceBlock() = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock::~SamplerInterfaceBlock() noexcept = default;

SamplerInterfaceBlock::SamplerInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName), mStageFlags(builder.mStageFlags),
    mSamplersInfoList(builder.mEntries.size())
{
    auto& infoMap = mInfoMap;
    infoMap.reserve(builder.mEntries.size());

    auto& samplersInfoList = mSamplersInfoList;

    size_t i = 0;
    for (auto const& e : builder.mEntries) {
        assert_invariant(i == e.offset);
        SamplerInfo& info = samplersInfoList[i++];
        info = e;
        info.uniformName = generateUniformName(mName.c_str(), e.name.c_str());
        infoMap[{ info.name.data(), info.name.size() }] = info.offset; // info.name.c_str() guaranteed constant
    }
    assert_invariant(i == samplersInfoList.size());
}

const SamplerInterfaceBlock::SamplerInfo* SamplerInterfaceBlock::getSamplerInfo(
        std::string_view name) const {
    auto pos = mInfoMap.find(name);
    ASSERT_PRECONDITION(pos != mInfoMap.end(), "sampler named \"%.*s\" not found",
            name.size(), name.data());
    return &mSamplersInfoList[pos->second];
}

utils::CString SamplerInterfaceBlock::generateUniformName(const char* group, const char* sampler) noexcept {
    char uniformName[256];

    // sampler interface block name
    char* const prefix = std::copy_n(group,
            std::min(sizeof(uniformName) / 2, strlen(group)), uniformName);
    if (uniformName[0] >= 'A' && uniformName[0] <= 'Z') {
        uniformName[0] |= 0x20; // poor man's tolower()
    }
    *prefix = '_';

    char* last = std::copy_n(sampler,
            std::min(sizeof(uniformName) / 2 - 2, strlen(sampler)),
            prefix + 1);
    *last++ = 0; // null terminator
    assert(last <= std::end(uniformName));

    return CString{ uniformName, size_t(last - uniformName) - 1u };
}

} // namespace filament
