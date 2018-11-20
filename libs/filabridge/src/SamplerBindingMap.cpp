/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "filament/SamplerBindingMap.h"

#include "private/filament/SamplerInterfaceBlock.h"

#include <private/filament/SibGenerator.h>

#include <utils/Log.h>

namespace filament {

void SamplerBindingMap::populate(SamplerInterfaceBlock* perMaterialSib, const char* materialName) {
    // To avoid collision, the sampler bindings start after the last UBO binding.
    const uint8_t numUniformBlockBindings = filament::BindingPoints::COUNT;
    uint8_t offset = numUniformBlockBindings;
    bool overflow = false;
    for (uint8_t blockIndex = 0; blockIndex < filament::BindingPoints::COUNT; blockIndex++) {
        mSamplerBlockOffsets[blockIndex] = offset;
        filament::SamplerInterfaceBlock const* sib;
        if (blockIndex == filament::BindingPoints::PER_MATERIAL_INSTANCE) {
            sib = perMaterialSib;
        } else {
            sib = filament::SibGenerator::getSib(blockIndex);
        }
        if (sib) {
            auto sibFields = sib->getSamplerInfoList();
            for (auto sInfo : sibFields) {
                if (offset - numUniformBlockBindings >= filament::MAX_SAMPLER_COUNT) {
                    overflow = true;
                }
                addSampler({
                    .blockIndex = blockIndex,
                    .localOffset = sInfo.offset,
                    .globalOffset = offset++,
                });
            }
        }
    }

    // If an overflow occurred, go back through and list all sampler names. This is helpful to
    // material authors who need to understand where the samplers are coming from.
    if (overflow) {
        utils::slog.e << "WARNING: Exceeded max sampler count of " << filament::MAX_SAMPLER_COUNT;
        if (materialName) {
            utils::slog.e << " (" << materialName << ")";
        }
        utils::slog.e << utils::io::endl;
        offset = 0;
        for (uint8_t blockIndex = 0; blockIndex < filament::BindingPoints::COUNT; blockIndex++) {
            filament::SamplerInterfaceBlock const* sib;
            if (blockIndex == filament::BindingPoints::PER_MATERIAL_INSTANCE) {
                sib = perMaterialSib;
            } else {
                sib = filament::SibGenerator::getSib(blockIndex);
            }
            if (sib) {
                auto sibFields = sib->getSamplerInfoList();
                for (auto sInfo : sibFields) {
                    utils::slog.e << "  " << offset << " " << sInfo.name.c_str() << utils::io::endl;
                    offset++;
                }
            }
        }
    }
}

void SamplerBindingMap::addSampler(SamplerBindingInfo info) {
    if (info.globalOffset < mSamplerBlockOffsets[info.blockIndex]) {
        mSamplerBlockOffsets[info.blockIndex] = info.globalOffset;
    }
    mBindingList.push_back(info);
    mBindingMap[getBindingKey(info.blockIndex, info.localOffset)] = info;
}

} // namespace filament
