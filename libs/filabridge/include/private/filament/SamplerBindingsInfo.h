/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILABRIDGE_SAMPLERBINDINGS_INFO_H
#define TNT_FILABRIDGE_SAMPLERBINDINGS_INFO_H

#include <backend/DriverEnums.h>

#include <private/filament/EngineEnums.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <array>

namespace filament {

// binding information about a sampler group
struct SamplerGroupBindingInfo {
    constexpr static uint8_t UNKNOWN_OFFSET = 0xff;
    // global binding of this block, or UNKNOWN_OFFSET if not used.
    uint8_t bindingOffset = UNKNOWN_OFFSET;
    // shader stage flags for samplers in this block
    backend::ShaderStageFlags shaderStageFlags = backend::ShaderStageFlags::NONE;
    // number of samplers in this block. Can be zero.
    uint8_t count = 0;
};

// list of binding information for all known binding points
using SamplerGroupBindingInfoList =
        std::array<SamplerGroupBindingInfo, utils::Enum::count<SamplerBindingPoints>()>;

// map of sampler shader binding to sampler shader name
using SamplerBindingToNameMap =
        utils::FixedCapacityVector<utils::CString>;

} // namespace filament

#endif //TNT_FILABRIDGE_SAMPLERBINDINGS_INFO_H
