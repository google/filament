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

#include <private/filament/EngineEnums.h>

namespace filament {

class SamplerInterfaceBlock;

// Lookup table from (BlockIndex,LocalOffset) to (GlobalOffset,GroupIndex).
// Also stores the mapping as a flat vector of 4-tuples to make it easy to [de]serialize.
class SamplerBindingMap {
public:
    // Assigns a range of finalized binding points to each sampler block.
    // If a per-material SIB is provided, then material samplers are also inserted (always at the
    // end). The optional material name is used for error reporting only.
    void populate(const SamplerInterfaceBlock* perMaterialSib = nullptr,
            const char* materialName = nullptr);

    // Given a valid Filament binding point and an offset within the block, returns a global unique
    // binding index.
    uint8_t getSamplerBinding(uint8_t bindingPoint, uint8_t localOffset) const {
        assert_invariant(mSamplerBlockOffsets[bindingPoint] != UNKNOWN_OFFSET);
        return mSamplerBlockOffsets[bindingPoint] + localOffset;
    }

    // Gets the global offset of the first sampler in the given sampler block.
    uint8_t getBlockOffset(uint8_t bindingPoint) const {
        assert_invariant(mSamplerBlockOffsets[bindingPoint] != UNKNOWN_OFFSET);
        return mSamplerBlockOffsets[bindingPoint];
    }

private:
    constexpr static uint8_t UNKNOWN_OFFSET = 0xff;
    uint8_t mSamplerBlockOffsets[filament::BindingPoints::COUNT] = { UNKNOWN_OFFSET };
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_SAMPLERBINDINGMAP_H
