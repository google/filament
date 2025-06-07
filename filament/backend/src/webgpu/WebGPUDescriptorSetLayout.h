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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSETLAYOUT_H
#define TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSETLAYOUT_H

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <vector>

namespace filament::backend {

class WebGPUDescriptorSetLayout final : public HwDescriptorSetLayout {
public:
    struct BindGroupEntryInfo final {
        uint8_t binding = 0;
        bool hasDynamicOffset = false;
    };

    WebGPUDescriptorSetLayout(DescriptorSetLayout const&, wgpu::Device const&);
    ~WebGPUDescriptorSetLayout() = default;

    [[nodiscard]] wgpu::BindGroupLayout const& getLayout() const { return mLayout; }

    [[nodiscard]] std::vector<BindGroupEntryInfo> const& getBindGroupEntries() const {
        return mBindGroupEntries;
    }

private:
    std::vector<BindGroupEntryInfo> mBindGroupEntries;
    wgpu::BindGroupLayout mLayout = nullptr;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUDESCRIPTORSETLAYOUT_H
