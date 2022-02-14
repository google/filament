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

#include "private/filament/SamplerBindingMap.h"

#include "private/filament/SamplerInterfaceBlock.h"

#include <private/filament/SibGenerator.h>

#include <utils/Log.h>

namespace filament {

void SamplerBindingMap::populate(const SamplerInterfaceBlock* perMaterialSib,
            const char* materialName) {
    // TODO: Calculate SamplerBindingMap with a material variant.
    // The dummy variant isn't enough for calculating the binding map.
    // The material variant affects sampler bindings.
    const Variant dummyVariant{};
    uint8_t offset = 0;
    for (uint8_t blockIndex = 0; blockIndex < filament::BindingPoints::COUNT; blockIndex++) {
        mSamplerBlockOffsets[blockIndex] = offset;
        filament::SamplerInterfaceBlock const* sib;
        if (blockIndex == filament::BindingPoints::PER_MATERIAL_INSTANCE) {
            sib = perMaterialSib;
        } else {
            sib = filament::SibGenerator::getSib(blockIndex, dummyVariant);
        }
        if (sib) {
            auto sibFields = sib->getSamplerInfoList();
            for (const auto& sInfo : sibFields) {
                addSampler({
                    .blockIndex = blockIndex,
                    .localOffset = sInfo.offset,
                    .globalOffset = offset++,
                });
            }
        }
    }

    auto isOverflow = [&perMaterialSib, &dummyVariant]() {
        size_t numVertexSampler = 0, numFragmentSampler = 0;
        for (uint8_t blockIndex = 0; blockIndex < filament::BindingPoints::COUNT; blockIndex++) {
            filament::SamplerInterfaceBlock const* sib;
            if (blockIndex == filament::BindingPoints::PER_MATERIAL_INSTANCE) {
                sib = perMaterialSib;
            } else {
                sib = filament::SibGenerator::getSib(blockIndex, dummyVariant);
            }
            if (sib) {
                // Shader stage flags is only needed to check if MAX_SAMPLER_COUNT is exceeded.
                // Somehow if we can get shader stage flags from Program then we can remove it in SamplerInterfaceBlock.
                const auto stageFlags = sib->getStageFlags();
                if (stageFlags.vertex) {
                    numVertexSampler += sib->getSamplerInfoList().size();
                }
                if (stageFlags.fragment) {
                    numFragmentSampler += sib->getSamplerInfoList().size();
                }
                if (numVertexSampler > backend::MAX_VERTEX_SAMPLER_COUNT ||
                    numFragmentSampler > backend::MAX_FRAGMENT_SAMPLER_COUNT) {
                    return true;
                }
            }
        }
        return false;
    };

    // If an overflow occurred, go back through and list all sampler names. This is helpful to
    // material authors who need to understand where the samplers are coming from.
    if (isOverflow()) {
        utils::slog.e << "WARNING: Exceeded max sampler count of " << backend::MAX_SAMPLER_COUNT;
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
                sib = filament::SibGenerator::getSib(blockIndex, dummyVariant);
            }
            if (sib) {
                auto sibFields = sib->getSamplerInfoList();
                for (auto sInfo : sibFields) {
                    utils::slog.e << "  " << (int) offset << " " << sInfo.name.c_str()
                        << " " << sib->getStageFlags() << utils::io::endl;
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
    mBindingMap[getBindingKey(info.blockIndex, info.localOffset)] = info;
}

} // namespace filament
