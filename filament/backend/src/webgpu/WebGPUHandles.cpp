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

#include "WebGPUHandles.h"

namespace filament::backend {

    WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const &device, uint8_t elementSize, uint32_t indexCount) {
        wgpu::BufferDescriptor descriptor{
                .label = "WGPUIndexBuffer",
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                .size = elementSize * indexCount,
                .mappedAtCreation = false };
        buffer = device.CreateBuffer(&descriptor);
    }

    WGPUVertexBuffer::WGPUVertexBuffer(wgpu::Device const &device, uint32_t vextexCount, uint32_t bufferCount,
                                       Handle<WGPUVertexBufferInfo> vbih)
            : HwVertexBuffer(vextexCount),
              vbih(vbih),
              buffers(MAX_VERTEX_BUFFER_COUNT) {
        wgpu::BufferDescriptor descriptor {
                .label = "WGPUVertexBuffer",
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                .size = vextexCount * bufferCount,
                .mappedAtCreation = false };

        for (uint32_t i = 0; i < bufferCount; ++i) {
            buffers[i] = device.CreateBuffer(&descriptor);
        }
    }


// TODO: Empty function is a place holder for verxtex buffer updates and should be
// updated for that purpose.
void WGPUVertexBuffer::setBuffer(WGPUBufferObject* bufferObject, uint32_t index) {}

WGPUBufferObject::WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount)
    : HwBufferObject(byteCount),
      bufferObjectBinding(bindingType) {}
}// namespace filament::backend
