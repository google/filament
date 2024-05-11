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

#include "SamplerBindingMap.h"

#include "shaders/SibGenerator.h"

#include <filament/MaterialEnums.h>

#include <private/filament/SamplerInterfaceBlock.h>

#include <backend/DriverEnums.h>

namespace filament {

using namespace utils;
using namespace backend;

void SamplerBindingMap::init(MaterialDomain materialDomain,
        SamplerInterfaceBlock const& perMaterialSib) {

    assert_invariant(mActiveSamplerCount == 0);

    mSamplerNamesBindingMap.reserve(MAX_SAMPLER_COUNT);
    mSamplerNamesBindingMap.resize(MAX_SAMPLER_COUNT);

    // Note: the material variant affects only the sampler types, but cannot affect
    // the actual bindings. For this reason it is okay to use the dummyVariant here.
    uint8_t offset = 0;
    size_t vertexSamplerCount = 0;
    size_t fragmentSamplerCount = 0;

    auto processSamplerGroup = [&](SamplerBindingPoints bindingPoint){
        SamplerInterfaceBlock const* const sib =
                (bindingPoint == SamplerBindingPoints::PER_MATERIAL_INSTANCE) ?
                &perMaterialSib : SibGenerator::getSib(bindingPoint, {});
        if (sib) {
            const auto stageFlags = sib->getStageFlags();
            auto const& list = sib->getSamplerInfoList();
            const size_t samplerCount = list.size();

            if (any(stageFlags & ShaderStageFlags::VERTEX)) {
                vertexSamplerCount += samplerCount;
            }
            if (any(stageFlags & ShaderStageFlags::FRAGMENT)) {
                fragmentSamplerCount += samplerCount;
            }

            mSamplerBlockOffsets[+bindingPoint] = { offset, stageFlags, uint8_t(samplerCount) };
            for (size_t i = 0; i < samplerCount; i++) {
                assert_invariant(mSamplerNamesBindingMap[offset + i].empty());
                mSamplerNamesBindingMap[offset + i] = list[i].uniformName;
            }

            offset += samplerCount;
        }
    };

    switch(materialDomain) {
        case MaterialDomain::SURFACE:
            UTILS_NOUNROLL
            for (size_t i = 0; i < Enum::count<SamplerBindingPoints>(); i++) {
                processSamplerGroup((SamplerBindingPoints)i);
            }
            break;
        case MaterialDomain::POST_PROCESS:
        case MaterialDomain::COMPUTE:
            processSamplerGroup(SamplerBindingPoints::PER_MATERIAL_INSTANCE);
            break;
    }

    mActiveSamplerCount = offset;

    // we shouldn't be using more total samplers than supported
    assert_invariant(vertexSamplerCount + fragmentSamplerCount <= MAX_SAMPLER_COUNT);

    // Here we cannot check for overflow for a given feature level because we don't know
    // what feature level the backend will support. We only know the feature level declared
    // by the material. However, we can at least assert for the highest feature level.

    constexpr size_t MAX_VERTEX_SAMPLER_COUNT =
            backend::FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3].MAX_VERTEX_SAMPLER_COUNT;

    assert_invariant(vertexSamplerCount <= MAX_VERTEX_SAMPLER_COUNT);

    constexpr size_t MAX_FRAGMENT_SAMPLER_COUNT =
            backend::FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3].MAX_FRAGMENT_SAMPLER_COUNT;

    assert_invariant(fragmentSamplerCount <= MAX_FRAGMENT_SAMPLER_COUNT);
}

} // namespace filament
