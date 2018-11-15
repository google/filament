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

#include <filament/EngineEnums.h>

#include <tsl/robin_map.h>
#include <vector>

namespace filament {

// Binding information for a single sampler.
//
// This identifies the parent block, the offset within the parent, and
// the finalized "global" offset, which uniquely identifies the
// sampler across all sampler blocks in the shader program.
//
// Also contains a grouping index, which is a hint to the backend about
// how to batch the updates. In Vulkan, this could be used to assign
// samplers to desciptor sets.
//
struct SamplerBindingInfo {
    uint8_t blockIndex;   // Binding point of the parent block (see filament::BindingPoints)
    uint8_t localOffset;  // Index of this sampler within the block
    uint8_t globalOffset; // Finalized binding point for the sampler
    uint8_t groupIndex;   // Group index for the sampler
};

class SamplerInterfaceBlock;

// Lookup table from (BlockIndex,LocalOffset) to (GlobalOffset,GroupIndex).
// Also stores the mapping as a flat vector of 4-tuples to make it easy to [de]serialize.
class SamplerBindingMap {
public:
    // Assigns a range of finalized binding points to each sampler block. If a per-material SIB
    // is provided, then material samplers are also inserted (always at the end).
    void populate(SamplerInterfaceBlock* perMaterialSib = nullptr);

    // Given a valid Filament binding point and an offset with the block, returns true and sets
    // the output arguments: (1) the globally unique binding index, and (2) the grouping index.
    bool getSamplerBinding(uint8_t blockIndex, uint8_t localOffset, uint8_t* globalOffset,
            uint8_t* groupIndex) const {
        assert(globalOffset);
        assert(groupIndex);
        auto iter = mBindingMap.find(getBindingKey(blockIndex, localOffset));
        if (iter == mBindingMap.end()) {
            return false;
        }
        *globalOffset = iter->second.globalOffset;
        *groupIndex = iter->second.groupIndex;
        return true;
    }

    // Adds the given sampler to the mapping. Useful for deserialization.
    void addSampler(SamplerBindingInfo info);

    // Retrieves all samplers as a flat list. Useful for serialization.
    const std::vector<SamplerBindingInfo>& getBindingList() const {
        return mBindingList;
    }

    // Gets the global offset of the first sampler in the given sampler block.
    uint8_t getBlockOffset(uint8_t bindingPoint) const {
        assert(UNKNOWN_OFFSET != mSamplerBlockOffsets[bindingPoint]);
        return mSamplerBlockOffsets[bindingPoint];
    }

private:
    constexpr static uint8_t UNKNOWN_OFFSET = 0xff;
    typedef uint32_t BindingKey;
    static BindingKey getBindingKey(uint8_t blockIndex, uint8_t localOffset) {
        return ((uint32_t) blockIndex << 8) + localOffset;
    }
    std::vector<SamplerBindingInfo> mBindingList;
    tsl::robin_map<BindingKey, SamplerBindingInfo> mBindingMap;
    uint8_t mSamplerBlockOffsets[filament::BindingPoints::COUNT] = { UNKNOWN_OFFSET };
};


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_SAMPLERBINDINGMAP_H
