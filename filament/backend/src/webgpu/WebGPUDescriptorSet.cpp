/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "WebGPUDescriptorSet.h"

#include "WebGPUDescriptorSetLayout.h"

#include <backend/DriverEnums.h>

#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace filament::backend {

namespace {

constexpr uint8_t INVALID_INDEX = MAX_DESCRIPTOR_COUNT + 1;

} // namespace

WebGPUDescriptorSet::WebGPUDescriptorSet(wgpu::BindGroupLayout const& layout,
        std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const& bindGroupEntries)
    : mLayout{ layout },
      mEntriesWithDynamicOffsetsCount{ static_cast<size_t>(std::count_if(bindGroupEntries.begin(),
              bindGroupEntries.end(), [](auto const& entry) { return entry.hasDynamicOffset; })) } {

    mEntries.resize(bindGroupEntries.size());
    for (size_t i = 0; i < bindGroupEntries.size(); ++i) {
        mEntries[i].binding = bindGroupEntries[i].binding;
    }
    // Establish the size of entries based on the layout. This should be reliable and efficient.
    assert_invariant(INVALID_INDEX > mEntryIndexByBinding.size());
    for (size_t i = 0; i < mEntryIndexByBinding.size(); i++) {
        mEntryIndexByBinding[i] = INVALID_INDEX;
    }
    for (size_t index = 0; index < mEntries.size(); index++) {
        wgpu::BindGroupEntry const& entry = mEntries[index];
        assert_invariant(entry.binding < mEntryIndexByBinding.size());
        mEntryIndexByBinding[entry.binding] = static_cast<uint8_t>(index);
    }
}

void WebGPUDescriptorSet::addEntry(const unsigned int index, wgpu::BindGroupEntry&& entry) {
    if (mBindGroup) {
        // We will keep getting hits from future updates, but shouldn't do anything
        // Filament guarantees this won't change after things have locked.
        return;
    }
    // TODO: Putting some level of trust that Filament is not going to reuse indexes or go past the
    // layout index for efficiency. Add guards if wrong.
    FILAMENT_CHECK_POSTCONDITION(index < mEntryIndexByBinding.size())
            << "impossible/invalid index for a descriptor/binding (our of range or >= "
               "MAX_DESCRIPTOR_COUNT) "
            << index;
    uint8_t entryIndex = mEntryIndexByBinding[index];
    FILAMENT_CHECK_POSTCONDITION(entryIndex != INVALID_INDEX && entryIndex < mEntries.size())
            << "Invalid binding " << index;
    entry.binding = index;
    mEntries[entryIndex] = std::move(entry);
}

wgpu::BindGroup WebGPUDescriptorSet::lockAndReturn(wgpu::Device const& device) {
    if (mBindGroup) {
        return mBindGroup;
    }
    // TODO label? Should we just copy layout label?
    const wgpu::BindGroupDescriptor descriptor{
        .layout = mLayout,
        .entryCount = mEntries.size(),
        .entries = mEntries.data()
    };
    mBindGroup = device.CreateBindGroup(&descriptor);
    FILAMENT_CHECK_POSTCONDITION(mBindGroup) << "Failed to create bind group?";
    // once we have created the bind group itself we should no longer need any other state
    mLayout = nullptr;
    mEntries.clear();
    mEntries.shrink_to_fit();
    return mBindGroup;
}

WebGPUDescriptorSet::~WebGPUDescriptorSet() {
    mBindGroup = nullptr;
    mLayout = nullptr;
}

} // namespace filament::backend
