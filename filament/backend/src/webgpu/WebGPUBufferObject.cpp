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

#include "WebGPUBufferObject.h"

#include "DriverBase.h"
#include "WebGPUBufferBase.h"
#include "WebGPUConstants.h"
#include "WebGPUVertexBuffer.h"
#include "WebGPUVertexBufferInfo.h"

#include "webgpu/utils/VertexHelper.h"

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>

#include <utils/debug.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

namespace filament::backend {

namespace {

[[nodiscard]] constexpr wgpu::BufferUsage getBufferObjectUsage(
        const BufferObjectBinding bindingType) noexcept {
    switch (bindingType) {
        case BufferObjectBinding::VERTEX:         return wgpu::BufferUsage::Vertex;
        case BufferObjectBinding::UNIFORM:        return wgpu::BufferUsage::Uniform;
        case BufferObjectBinding::SHADER_STORAGE: return wgpu::BufferUsage::Storage;
    }
}

} // namespace

// How one WebGPU vertex buffer slot fed by this buffer object is laid out in the converted GPU
// buffer.
struct WebGPUBufferObject::SlotTransform final {
    webgpuutils::VertexSlotRepack repack;
    uint32_t srcOffset = 0;   // offset of the slot's first vertex in the source data
    uint32_t vertexCount = 0; // number of vertices converted
    uint64_t dstOffset = 0;   // offset of the slot's first vertex in the converted GPU buffer

    bool operator==(SlotTransform const&) const = default;
};

struct WebGPUBufferObject::VertexState final {
    enum class Path : uint8_t {
        UNATTACHED, // no GPU buffer yet, updates go to pendingData
        DIRECT,     // the GPU buffer holds the source data as-is
        TRANSFORM,  // the GPU buffer holds the data converted as described by slots
    };
    Path path = Path::UNATTACHED;
    // CPU-side copy of the updates made before attachment. Released on attachment.
    std::vector<uint8_t> pendingData;
    // Only used on the TRANSFORM path.
    std::vector<SlotTransform> slots;
};

// The usage flags are determined by the binding type, and always include CopyDst to allow for
// updating the buffer.
WebGPUBufferObject::WebGPUBufferObject(wgpu::Device const& device,
        const BufferObjectBinding bindingType, const uint32_t byteCount)
        : HwBufferObject{ byteCount, false } {
    if (bindingType == BufferObjectBinding::VERTEX) {
        // The GPU buffer is created when attaching to a vertex buffer.
        mVertexState = std::make_unique<VertexState>();
        return;
    }
    createBuffer(device, wgpu::BufferUsage::CopyDst | getBufferObjectUsage(bindingType), byteCount,
            "buffer_object");
}

WebGPUBufferObject::~WebGPUBufferObject() = default;

void WebGPUBufferObject::updateGPUBuffer(BufferDescriptor const& bufferDescriptor,
        uint32_t const byteOffset, wgpu::Device const& device,
        WebGPUQueueManager* const webGPUQueueManager, WebGPUStagePool* const webGPUStagePool) {
    if (!mVertexState) {
        WebGPUBufferBase::updateGPUBuffer(bufferDescriptor, byteOffset, device, webGPUQueueManager,
                webGPUStagePool);
        return;
    }

    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.buffer)
            << "updateGPUBuffer called with a null buffer";
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.size + byteOffset <= byteCount)
            << "Attempting to copy " << bufferDescriptor.size << " bytes into a buffer of size "
            << byteCount << " at offset " << byteOffset;
    FILAMENT_CHECK_PRECONDITION(byteOffset % FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS == 0)
            << "Byte offset must be a multiple of " << FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS
            << " but is " << byteOffset;

    switch (mVertexState->path) {
        case VertexState::Path::UNATTACHED: {
            std::vector<uint8_t>& pendingData = mVertexState->pendingData;
            if (pendingData.empty()) {
                pendingData.resize(byteCount, 0);
            }
            memcpy(pendingData.data() + byteOffset, bufferDescriptor.buffer, bufferDescriptor.size);
            break;
        }
        case VertexState::Path::DIRECT:
            WebGPUBufferBase::updateGPUBuffer(bufferDescriptor, byteOffset, device,
                    webGPUQueueManager, webGPUStagePool);
            break;
        case VertexState::Path::TRANSFORM:
            for (SlotTransform const& slot: mVertexState->slots) {
                writeConvertedVertices(slot, static_cast<uint8_t const*>(bufferDescriptor.buffer),
                        byteOffset, bufferDescriptor.size, webGPUQueueManager, webGPUStagePool);
            }
            break;
    }
}

void WebGPUBufferObject::attachToVertexBuffer(WebGPUVertexBuffer& vertexBuffer,
        WebGPUVertexBufferInfo const& vertexBufferInfo, uint32_t const bufferIndex,
        wgpu::Device const& device, WebGPUQueueManager* const webGPUQueueManager,
        WebGPUStagePool* const webGPUStagePool) {
    FILAMENT_CHECK_PRECONDITION(mVertexState)
            << "Only buffer objects created with BufferObjectBinding::VERTEX can be attached to "
               "a vertex buffer";
    auto const& slotInfos = vertexBufferInfo.getWebGPUSlotBindingInfos();
    std::vector<WebGPUVertexBuffer::SlotBinding>& slotBindings = vertexBuffer.getSlotBindings();
    assert_invariant(slotBindings.size() == slotInfos.size());

    // Work out how the slots fed by this buffer would be laid out in a converted buffer.
    bool needsTransform = false;
    std::vector<SlotTransform> slots;
    std::vector<size_t> slotIndices;
    uint64_t convertedSize = 0;
    for (size_t slotIndex = 0; slotIndex < slotInfos.size(); ++slotIndex) {
        WebGPUVertexBufferInfo::WebGPUSlotBindingInfo const& slotInfo = slotInfos[slotIndex];
        if (slotInfo.sourceBufferIndex != bufferIndex) {
            continue;
        }
        needsTransform = needsTransform || slotInfo.repack.needsTransform;
        uint32_t const vertexCount = std::min(vertexBuffer.vertexCount,
                webgpuutils::getAvailableVertexCount(slotInfo.repack, slotInfo.bufferOffset,
                        byteCount));
        slots.push_back({
            .repack = slotInfo.repack,
            .srcOffset = slotInfo.bufferOffset,
            .vertexCount = vertexCount,
            .dstOffset = convertedSize,
        });
        slotIndices.push_back(slotIndex);
        // dstStride is a multiple of 4, so every slot starts 4-byte aligned.
        convertedSize += uint64_t(vertexCount) * slotInfo.repack.dstStride;
    }

    VertexState& state = *mVertexState;
    switch (state.path) {
        case VertexState::Path::UNATTACHED:
            if (needsTransform) {
                FILAMENT_CHECK_PRECONDITION(convertedSize <= std::numeric_limits<uint32_t>::max())
                        << "Converted vertex buffer is too large (" << convertedSize << " bytes)";
                createBuffer(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                        std::max<uint32_t>(uint32_t(convertedSize), 4), "buffer_object_converted");
                state.slots = slots;
                if (!state.pendingData.empty()) {
                    for (SlotTransform const& slot: state.slots) {
                        writeConvertedVertices(slot, state.pendingData.data(), 0, byteCount,
                                webGPUQueueManager, webGPUStagePool);
                    }
                }
                state.path = VertexState::Path::TRANSFORM;
            } else {
                createBuffer(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                        byteCount, "buffer_object");
                if (!state.pendingData.empty()) {
                    uint8_t const* const pendingData = state.pendingData.data();
                    writeToBuffer(getBuffer(), 0, byteCount, webGPUQueueManager, webGPUStagePool,
                            [this, pendingData](uint8_t* const destination) {
                                memcpy(destination, pendingData, byteCount);
                            });
                }
                state.path = VertexState::Path::DIRECT;
            }
            // The data now lives on the GPU; no CPU-side copy is kept after attachment.
            std::vector<uint8_t>().swap(state.pendingData);
            break;
        case VertexState::Path::DIRECT:
            FILAMENT_CHECK_PRECONDITION(!needsTransform)
                    << "This vertex buffer object was previously attached with a layout that "
                       "WebGPU supports directly, and cannot be attached with a layout that "
                       "needs a format conversion";
            break;
        case VertexState::Path::TRANSFORM:
            FILAMENT_CHECK_PRECONDITION(needsTransform && slots == state.slots)
                    << "This vertex buffer object was previously attached with a layout that "
                       "needs a format conversion, and can only be attached again with an "
                       "identical layout and vertex count";
            break;
    }

    // Bind the slots fed by this buffer object.
    bool const transformed = state.path == VertexState::Path::TRANSFORM;
    for (size_t i = 0; i < slotIndices.size(); ++i) {
        slotBindings[slotIndices[i]] = {
            .buffer = getBuffer(),
            .offset = transformed ? slots[i].dstOffset : slots[i].srcOffset,
        };
    }
}

void WebGPUBufferObject::writeConvertedVertices(SlotTransform const& slot,
        uint8_t const* const data, uint32_t const byteOffset, uint32_t const size,
        WebGPUQueueManager* const webGPUQueueManager,
        WebGPUStagePool* const webGPUStagePool) const {
    webgpuutils::VertexSlotRepack const& repack = slot.repack;
    int64_t const updateBegin = int64_t(byteOffset) - int64_t(slot.srcOffset);
    webgpuutils::VertexRange const range = webgpuutils::getUpdatedVertexRange(repack,
            slot.vertexCount, updateBegin, updateBegin + int64_t(size));
    if (range.count == 0) {
        return;
    }
    FILAMENT_CHECK_PRECONDITION(range.complete)
            << "Updates to a vertex buffer object that needs a format conversion in WebGPU must "
               "cover whole vertices. The update at offset "
            << byteOffset << " of size " << size
            << " partially covers vertices of a slot starting at offset " << slot.srcOffset
            << " with a stride of " << repack.srcStride;

    // The update covers every byte read by the vertices in the range, so this points inside
    // `data`.
    uint8_t const* const src =
            data + (int64_t(slot.srcOffset) + int64_t(range.first) * repack.srcStride +
                           repack.srcVertexBegin - int64_t(byteOffset));
    uint64_t const dstOffset = slot.dstOffset + uint64_t(range.first) * repack.dstStride;
    writeToBuffer(getBuffer(), uint32_t(dstOffset), size_t(range.count) * repack.dstStride,
            webGPUQueueManager, webGPUStagePool,
            [&repack, src, &range](uint8_t* const destination) {
                webgpuutils::repackVertices(repack, src, destination, range.count);
            });
}

} // namespace filament::backend
