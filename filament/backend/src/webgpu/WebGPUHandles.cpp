//
// Created by Idris Idris Shah on 3/21/25.
//

#include "WebGPUHandles.h"

namespace filament::backend {

    WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const &device, BufferUsage usage, uint8_t elementSize,
                                     uint32_t indexCount) {
        wgpu::BufferDescriptor descriptor{
                .label = "my_vertex_buffer",
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                .size = elementSize * indexCount,
                .mappedAtCreation = false};
        buffer = device.CreateBuffer(&descriptor);
    }


    WGPUVertexBuffer::WGPUVertexBuffer(uint32_t vextexCount, uint32_t bufferCount,
                                       Handle<WGPUVertexBufferInfo> vbih)
            : HwVertexBuffer(vextexCount),
              vbih(vbih),
              mBuffers(MAX_VERTEX_BUFFER_COUNT) {
    }

    void WGPUVertexBuffer::setBuffer(WGPUBufferObject *bufferObject, uint32_t index) {}

    WGPUBufferObject::WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount)
            : HwBufferObject(byteCount),
              mBindingType(bindingType) {

    }
}
