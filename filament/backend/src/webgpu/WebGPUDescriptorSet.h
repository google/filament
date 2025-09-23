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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSET_H
#define TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSET_H

#include "WebGPUDescriptorSetLayout.h"

#include "WebGPUConstants.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <vector>

namespace filament::backend {

/**
  * A WebGPU-specific implementation of DescriptorSet.
  * This class manages a collection of resource bindings (e.g., textures, buffers) that are used by
  * a shader. It corresponds to a `wgpu::BindGroup` in WebGPU. The actual `wgpu::BindGroup` is
  * created lazily when `lockAndReturn` is called, at which point the descriptor set becomes
  * immutable.
  */
class WebGPUDescriptorSet final : public HwDescriptorSet {
public:
    WebGPUDescriptorSet(wgpu::BindGroupLayout const&,
            std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const&);
    ~WebGPUDescriptorSet();

    /**
     * Adds or updates a resource binding in the descriptor set.
     * This method is called to populate the descriptor set with buffers, textures, and samplers.
     * It is only valid to call this before the descriptor set is locked (i.e., before
     * `lockAndReturn` is called).
     */
    void addEntry(unsigned int index, wgpu::BindGroupEntry&& entry);

    /**
     * Finalizes the descriptor set by creating the wgpu::BindGroup. After this call, the
     * descriptor set is considered "locked" and cannot be modified further.
     */
    [[nodiscard]] wgpu::BindGroup lockAndReturn(wgpu::Device const&);

    /**
     * @return true if the descriptor set has been locked (i.e., the wgpu::BindGroup has been
     * created).
     */
    [[nodiscard]] bool getIsLocked() const { return mBindGroup != nullptr; }

    [[nodiscard]] size_t getEntitiesWithDynamicOffsetsCount() const {
        return mEntriesWithDynamicOffsetsCount;
    }

    /**
     * May be nullptr. Use lockAndReturn to create the bind group when appropriate
     */
    [[nodiscard]] wgpu::BindGroup const& getBindGroup() const { return mBindGroup; }

#if FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
    [[nodiscard]] wgpu::BindGroupLayout const& getLayout() const { return mLayout; }
#endif

private:
    wgpu::BindGroupLayout mLayout = nullptr;
    std::array<uint8_t, MAX_DESCRIPTOR_COUNT> mEntryIndexByBinding{};
    std::vector<wgpu::BindGroupEntry> mEntries;
    const size_t mEntriesWithDynamicOffsetsCount;
    // This is created lazily when lockAndReturn is called.
    wgpu::BindGroup mBindGroup = nullptr;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSET_H
