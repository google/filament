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

class WebGPUDescriptorSet final : public HwDescriptorSet {
public:
    WebGPUDescriptorSet(wgpu::BindGroupLayout const&,
            std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const&);
    ~WebGPUDescriptorSet();

    void addEntry(unsigned int index, wgpu::BindGroupEntry&& entry);

    [[nodiscard]] wgpu::BindGroup lockAndReturn(wgpu::Device const&);

    [[nodiscard]] bool getIsLocked() const { return mBindGroup != nullptr; }

    [[nodiscard]] size_t getEntitiesWithDynamicOffsetsCount() const {
        return mEntriesWithDynamicOffsetsCount;
    }

    // May be nullptr. Use lockAndReturn to create the bind group when appropriate
    [[nodiscard]] wgpu::BindGroup const& getBindGroup() const { return mBindGroup; }

#if FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
    [[nodiscard]] wgpu::BindGroupLayout const& getLayout() const { return mLayout; }
#endif

private:
    wgpu::BindGroupLayout mLayout = nullptr;
    std::array<uint8_t, MAX_DESCRIPTOR_COUNT> mEntryIndexByBinding{};
    std::vector<wgpu::BindGroupEntry> mEntries;
    const size_t mEntriesWithDynamicOffsetsCount;
    wgpu::BindGroup mBindGroup = nullptr;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSET_H
