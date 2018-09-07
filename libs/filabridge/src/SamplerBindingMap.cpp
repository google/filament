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

#include "filament/SamplerInterfaceBlock.h"

#include <private/filament/SibGenerator.h>

namespace filament {

void SamplerBindingMap::populate(SamplerInterfaceBlock* perMaterialSib) {
    // To avoid collision, the sampler bindings start after the last UBO binding.
    uint8_t offset = filament::BindingPoints::COUNT;
    for (uint8_t blockIndex = 0; blockIndex < filament::BindingPoints::COUNT; blockIndex++) {
        mSamplerBlockOffsets[blockIndex] = offset;
        filament::SamplerInterfaceBlock* sib;
        if (blockIndex == filament::BindingPoints::PER_MATERIAL_INSTANCE) {
            sib = perMaterialSib;
        } else {
            sib = filament::SibGenerator::getSib(blockIndex);
        }
        if (sib) {
            auto sibFields = sib->getSamplerInfoList();
            for (auto sInfo : sibFields) {
				filament::SamplerBindingInfo samplerBindingInfo;
				{
					samplerBindingInfo.blockIndex = blockIndex;
					samplerBindingInfo.localOffset = sInfo.offset;
					samplerBindingInfo.globalOffset = offset++;
				}
                addSampler(samplerBindingInfo);
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
