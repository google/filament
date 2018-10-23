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

using namespace utils;

namespace filament {

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::name(const std::string& interfaceBlockName) {
    mName = CString(interfaceBlockName.c_str());
    return *this;
}

SamplerInterfaceBlock::Builder& SamplerInterfaceBlock::Builder::add( const std::string& samplerName, Type type, Format format,
        Precision precision, bool multisample) noexcept {
    mEntries.emplace_back(CString(samplerName.c_str()), type, format, precision, multisample);
    return *this;
}

SamplerInterfaceBlock SamplerInterfaceBlock::Builder::build() {
    return SamplerInterfaceBlock(*this);
}

// -------------------------------------------------------------------------------------------------

SamplerInterfaceBlock::SamplerInterfaceBlock() = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs) = default;
SamplerInterfaceBlock::SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept /* = default */ {};
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(const SamplerInterfaceBlock& rhs) = default;
SamplerInterfaceBlock& SamplerInterfaceBlock::operator=(SamplerInterfaceBlock&& rhs) noexcept = default;
SamplerInterfaceBlock::~SamplerInterfaceBlock() noexcept = default;

SamplerInterfaceBlock::SamplerInterfaceBlock(Builder& builder) noexcept
    : mName(std::move(builder.mName))
{
    auto& infoMap = mInfoMap;
    auto& samplersInfoList = mSamplersInfoList;
    infoMap.reserve(builder.mEntries.size());
    samplersInfoList.resize(builder.mEntries.size());

    uint32_t i = 0;
    for (auto const& e : builder.mEntries) {
        SamplerInfo& info = samplersInfoList[i];
        info = { std::move(e.name), uint8_t(i), e.type, e.format, e.precision, e.multisample };

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

} // namespace filament
