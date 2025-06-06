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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFERINFO_H
#define TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFERINFO_H

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <cstdint>
#include <vector>

namespace wgpu {
struct Limits;
struct VertexAttribute;
struct VertexBufferLayout;
} // namespace wgpu

namespace filament::backend {

// Maps Filament vertex attributes to WebGPU vertex buffer bindings.
class WebGPUVertexBufferInfo final : public HwVertexBufferInfo {
public:
    WebGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
            AttributeArray const& attributes, wgpu::Limits const& deviceLimits);

    [[nodiscard]] wgpu::VertexBufferLayout const* getVertexBufferLayouts() const {
        return mVertexBufferLayouts.data();
    }

    [[nodiscard]] size_t getVertexBufferLayoutCount() const;

    struct WebGPUSlotBindingInfo final {
        uint8_t sourceBufferIndex = 0; // limited by filament::backend::Attribute::buffer
        uint32_t bufferOffset = 0;     // limited by filament::backend::Attribute::offset
        uint8_t stride = 0;            // limited by filament::backend::Attribute::stride
    };

    [[nodiscard]] std::vector<WebGPUSlotBindingInfo> const& getWebGPUSlotBindingInfos() const {
        return mWebGPUSlotBindingInfos;
    }

private:
    // This stores the final wgpu::VertexBufferLayout objects, one per WebGPU slot.
    // (this is a vector and not an array due to size limitations of the handle in the handle
    // allocator)
    std::vector<wgpu::VertexBufferLayout> mVertexBufferLayouts;

    // This stores all the vertex attributes across all the vertex buffer layouts, where
    // the attributes are contiguous for each buffer layout.
    // Each buffer layout references the first in a contiguous group of attributes in this array,
    // as well as the number of such attributes in this vector.
    // (this is a vector and not an array due to size limitations of the handle in the handle
    // allocator)
    std::vector<wgpu::VertexAttribute> mVertexAttributes;

    // Stores information for the driver to perform setVertexBuffer calls
    // (this is a vector and not an array due to size limitations of the handle in the handle
    // allocator)
    std::vector<WebGPUSlotBindingInfo> mWebGPUSlotBindingInfos;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFERINFO_H
