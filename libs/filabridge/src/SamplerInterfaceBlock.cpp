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

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::name(utils::CString const& interfaceBlockName) {
    mName = interfaceBlockName;
    return *this;
}

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::name(utils::CString&& interfaceBlockName) {
    interfaceBlockName.swap(mName);
    return *this;
}

SamplerInterfaceBlock::Builder&
SamplerInterfaceBlock::Builder::name(utils::StaticString const& interfaceBlockName) {
    mName = CString{ interfaceBlockName };
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        utils::CString const& samplerName, Type type, Format format,
        Precision precision, bool multisample) noexcept {
    mEntries.emplace_back(samplerName, type, format, precision, multisample);
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        utils::CString&& samplerName, Type type, Format format,
        Precision precision, bool multisample) noexcept {
    mEntries.emplace_back(std::move(samplerName), type, format, precision, multisample);
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add(
        utils::StaticString const& samplerName, Type type, Format format,
        Precision precision, bool multisample) noexcept {
    mEntries.emplace_back(samplerName, type, format, precision, multisample);
    return *this;
}

SamplerInterfaceBlock SamplerInterfaceBlock::Builder::build() {
    return SamplerInterfaceBlock(*this);
}

SamplerInterfaceBlock::Builder::~Builder() noexcept = default;

// -------------------------------------------------------------------------------------------------

SamplerInterfaceBlock::SamplerInterfaceBlock() = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs) = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(const SamplerInterfaceBlock& rhs) = default;
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(SamplerInterfaceBlock&& rhs) /*noexcept*/ = default;
SamplerInterfaceBlock::~SamplerInterfaceBlock() noexcept = default;

SamplerInterfaceBlock::SamplerInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName)
{
    auto& infoMap = mInfoMap;
    auto& samplersInfoList = mSamplersInfoList;
    infoMap.reserve(builder.mEntries.size());
    samplersInfoList.resize(builder.mEntries.size());

    uint32_t i = 0;
    for (auto const& e : builder.mEntries) {
        SamplerInfo& info = samplersInfoList[i];
        info = { e.name, uint8_t(i), e.type, e.format, e.precision, e.multisample };

        // record this uniform info
        infoMap[info.name.c_str()] = i;

        // advance offset to next slot
        ++i;
    }

    mSize = i;
}

const SamplerInterfaceBlock::SamplerInfo* SamplerInterfaceBlock::getSamplerInfo(
        const char* name) const {
    auto const& pos = mInfoMap.find(name);
    if (!ASSERT_PRECONDITION_NON_FATAL(pos != mInfoMap.end(), "sampler named \"%s\" not found", name)) {
        return nullptr;
    }
    return &mSamplersInfoList[pos->second];
}

utils::CString SamplerInterfaceBlock::getUniformName(const char* group, const char* sampler) noexcept {
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
