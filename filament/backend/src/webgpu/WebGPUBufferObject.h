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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUBUFFEROBJECT_H
#define TNT_FILAMENT_BACKEND_WEBGPUBUFFEROBJECT_H

#include "DriverBase.h"
#include "WebGPUBufferBase.h"

#include <cstdint>
#include <memory>

namespace wgpu {
class Device;
}

namespace filament::backend {

class BufferDescriptor;
class WebGPUQueueManager;
class WebGPUStagePool;
class WebGPUVertexBuffer;
class WebGPUVertexBufferInfo;
enum class BufferObjectBinding : uint8_t;

/**
 * A WebGPU-specific implementation of BufferObject.
 * A BufferObject is a GPU buffer that can be used for a variety of purposes, such as storing
 * uniform data, vertex attributes, or pixel data. The specific usage is determined by the
 * BufferObjectBinding.
 *
 * UNIFORM and SHADER_STORAGE buffer objects create their GPU buffer immediately and are updated
 * directly.
 *
 * VERTEX buffer objects defer the creation of their GPU buffer until they are attached to a
 * vertex buffer, because only then is the vertex layout known. Some Filament vertex formats have
 * no WebGPU equivalent (see webgpuutils::VertexSlotRepack), in which case the buffer object takes
 * the "transform" path and its GPU buffer holds the data converted to the WebGPU layout:
 *  - Before attachment, updates are kept in a CPU-side copy of the buffer.
 *  - On attachment, the GPU buffer is created in either the source layout (direct path) or the
 *    converted layout (transform path), the CPU-side copy is uploaded or converted, and then it is
 *    released. No CPU-side copy is kept after attachment.
 *  - After attachment, updates on the transform path are converted on the fly and must therefore
 *    cover whole vertices of every slot they touch.
 *  - All attachments of a VERTEX buffer object must agree on the path, and on the transform path
 *    they must also have an identical layout, since the source data is no longer available to
 *    produce another conversion.
 */
class WebGPUBufferObject final : public HwBufferObject, private WebGPUBufferBase {
public:
    WebGPUBufferObject(wgpu::Device const&, BufferObjectBinding, uint32_t byteCount);
    ~WebGPUBufferObject();

    // The GPU buffer of UNIFORM and SHADER_STORAGE buffer objects. Vertex buffers obtain their
    // GPU buffers through attachToVertexBuffer() instead.
    using WebGPUBufferBase::getBuffer;

    // See the IMPORTANT NOTE of WebGPUBufferBase::updateGPUBuffer().
    void updateGPUBuffer(BufferDescriptor const&, uint32_t byteOffset, wgpu::Device const&,
            WebGPUQueueManager*, WebGPUStagePool*);

    // Attaches this VERTEX buffer object as source buffer `bufferIndex` of `vertexBuffer`, and
    // updates the vertex buffer's slot bindings accordingly.
    void attachToVertexBuffer(WebGPUVertexBuffer& vertexBuffer,
            WebGPUVertexBufferInfo const& vertexBufferInfo, uint32_t bufferIndex,
            wgpu::Device const&, WebGPUQueueManager*, WebGPUStagePool*);

private:
    struct VertexState;
    struct SlotTransform;

    void writeConvertedVertices(SlotTransform const&, uint8_t const* data, uint32_t byteOffset,
            uint32_t size, WebGPUQueueManager*, WebGPUStagePool*) const;

    // Only allocated for VERTEX buffer objects, which keeps the handle small for the others.
    std::unique_ptr<VertexState> mVertexState;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBUFFEROBJECT_H
