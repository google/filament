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

#include "WebGPUVertexBuffer.h"

#include "DriverBase.h"

#include <backend/Handle.h>

#include <webgpu/webgpu_cpp.h>

#include <cstddef>
#include <cstdint>

namespace filament::backend {

WebGPUVertexBuffer::WebGPUVertexBuffer(uint32_t const vertexCount, size_t const slotCount,
        Handle<HwVertexBufferInfo> vertexBufferInfoHandle)
        : HwVertexBuffer{ vertexCount },
          mVertexBufferInfoHandle{ vertexBufferInfoHandle } {
    mSlotBindings.resize(slotCount);
}

} // namespace filament::backend
