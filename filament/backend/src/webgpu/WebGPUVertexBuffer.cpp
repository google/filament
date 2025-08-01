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

#include <cstdint>

namespace filament::backend {

WebGPUVertexBuffer::WebGPUVertexBuffer(const uint32_t vertexCount, const uint32_t bufferCount,
        Handle<HwVertexBufferInfo> vertexBufferInfoHandle)
    : HwVertexBuffer{ vertexCount },
      mVertexBufferInfoHandle{ vertexBufferInfoHandle } {
    mBuffers.resize(bufferCount);
}

} // namespace filament::backend
