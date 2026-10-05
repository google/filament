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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFER_H
#define TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFER_H

#include "DriverBase.h"

#include <backend/Handle.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <vector>

namespace filament::backend {

class WebGPUVertexBuffer final : public HwVertexBuffer {
public:
    // The GPU buffer and offset bound to one WebGPU vertex buffer slot (see
    // WebGPUVertexBufferInfo). Depending on whether the attached buffer object needed a format
    // conversion, this is either the buffer object's own buffer at the slot's source offset, or
    // a converted buffer at the slot's offset within it.
    struct SlotBinding final {
        wgpu::Buffer buffer;
        uint64_t offset = 0;
    };

    WebGPUVertexBuffer(uint32_t vertexCount, size_t slotCount, Handle<HwVertexBufferInfo>);

    [[nodiscard]] Handle<HwVertexBufferInfo>& getVertexBufferInfoHandle() {
        return mVertexBufferInfoHandle;
    }

    [[nodiscard]] std::vector<SlotBinding>& getSlotBindings() { return mSlotBindings; }

private:
    Handle<HwVertexBufferInfo> mVertexBufferInfoHandle;
    std::vector<SlotBinding> mSlotBindings;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUVERTEXBUFFER_H
