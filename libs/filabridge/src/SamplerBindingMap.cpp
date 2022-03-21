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
    size_t vertexSamplerCount = 0;
    size_t fragmentSamplerCount = 0;

    UTILS_NOUNROLL
    for (uint8_t blockIndex = 0; blockIndex < BindingPoints::COUNT; blockIndex++) {
        mSamplerBlockOffsets[blockIndex] = offset;
        SamplerInterfaceBlock const* const sib =
                (blockIndex == BindingPoints::PER_MATERIAL_INSTANCE) ?
                perMaterialSib : SibGenerator::getSib(blockIndex, dummyVariant);
        if (sib) {
            const auto& sibFields = sib->getSamplerInfoList();
            const auto stageFlags = sib->getStageFlags();
            const size_t samplerCount = sibFields.size();
            offset += samplerCount;
            if (stageFlags.vertex) {
                vertexSamplerCount += samplerCount;
            }
            if (stageFlags.fragment) {
                fragmentSamplerCount += samplerCount;
            }
        }
    }

    const bool isOverflow = vertexSamplerCount > backend::MAX_VERTEX_SAMPLER_COUNT ||
                            fragmentSamplerCount > backend::MAX_FRAGMENT_SAMPLER_COUNT;

    // If an overflow occurred, go back through and list all sampler names. This is helpful to
    // material authors who need to understand where the samplers are coming from.
    if (UTILS_UNLIKELY(isOverflow)) {
        utils::slog.e << "WARNING: Exceeded max sampler count of " << backend::MAX_SAMPLER_COUNT;
        if (materialName) {
            utils::slog.e << " (" << materialName << ")";
        }
        utils::slog.e << utils::io::endl;
        offset = 0;
        for (uint8_t blockIndex = 0; blockIndex < BindingPoints::COUNT; blockIndex++) {
            SamplerInterfaceBlock const* const sib =
                    (blockIndex == BindingPoints::PER_MATERIAL_INSTANCE) ?
                    perMaterialSib : SibGenerator::getSib(blockIndex, dummyVariant);
            if (sib) {
                auto const& sibFields = sib->getSamplerInfoList();
                for (auto const& sInfo : sibFields) {
                    utils::slog.e << "  " << (int) offset << " " << sInfo.name.c_str()
                        << " " << sib->getStageFlags() << utils::io::endl;
                    offset++;
                }
            }
        }
    }
}

} // namespace filament
