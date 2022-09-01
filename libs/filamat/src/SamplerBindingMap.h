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

#ifndef TNT_FILAMENT_DRIVER_SAMPLERBINDINGMAP_H
#define TNT_FILAMENT_DRIVER_SAMPLERBINDINGMAP_H

#include <private/filament/SamplerBindingsInfo.h>

#include <filament/MaterialEnums.h>

namespace filament {

class SamplerInterfaceBlock;

/*
 * SamplerBindingMap maps filament's (BindingPoints, offset) to a global offset.
 * This global offset is used in shaders to set the `layout(binding=` of each sampler.
 *
 * It also keeps a map of global offsets to the sampler name in the shader.
 *
 * SamplerBindingMap is flattened into the material file and used on the filament side to
 * create the backend's programs.
 */
class SamplerBindingMap {
public:

    using SamplerGroupBindingInfo = SamplerGroupBindingInfo;

    // Initializes the SamplerBindingMap.
    // Assigns a range of finalized binding points to each sampler block.
    // If a per-material SIB is provided, then material samplers are also inserted (always at the
    // end).
    void init(MaterialDomain materialDomain,
            SamplerInterfaceBlock const& perMaterialSib);

    SamplerGroupBindingInfo const& getSamplerGroupBindingInfo(
            SamplerBindingPoints bindingPoint) const noexcept {
        return mSamplerBlockOffsets[+bindingPoint];
    }

    // Gets the global offset of the first sampler in the given sampler block.
    inline uint8_t getBlockOffset(SamplerBindingPoints bindingPoint) const noexcept {
        assert_invariant(mSamplerBlockOffsets[+bindingPoint].bindingOffset != UNKNOWN_OFFSET);
        return getSamplerGroupBindingInfo(bindingPoint).bindingOffset;
    }

    size_t getActiveSamplerCount() const noexcept {
        return mActiveSamplerCount;
    }

    utils::CString const& getSamplerName(size_t binding) const noexcept {
        return mSamplerNamesBindingMap[binding];
    }

private:
    constexpr static uint8_t UNKNOWN_OFFSET = SamplerGroupBindingInfo::UNKNOWN_OFFSET;
    SamplerGroupBindingInfoList mSamplerBlockOffsets{};
    SamplerBindingToNameMap mSamplerNamesBindingMap{};
    uint8_t mActiveSamplerCount = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_SAMPLERBINDINGMAP_H
