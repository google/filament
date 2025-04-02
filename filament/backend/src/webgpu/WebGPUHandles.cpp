//
// Created by Idris Idris Shah on 3/21/25.
//

#include "WebGPUHandles.h"

namespace filament::backend {

    // TODO: Should we pass &device or include the .h and use mDevice
    WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const &device, uint8_t elementSize, uint32_t indexCount) {
        wgpu::BufferDescriptor descriptor{
                .label = "WGPUIndexBuffer",
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                .size = elementSize * indexCount,
                .mappedAtCreation = false};
        buffer = device.CreateBuffer(&descriptor);
    }


    WGPUVertexBuffer::WGPUVertexBuffer(wgpu::Device const &device, uint32_t vextexCount, uint32_t bufferCount,
                                       Handle<WGPUVertexBufferInfo> vbih)
            : HwVertexBuffer(vextexCount),
              vbih(vbih),
              mBuffers(MAX_VERTEX_BUFFER_COUNT) {
        wgpu::BufferDescriptor descriptor{
                .label = "WGPUIndexBuffer",
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                .size = vextexCount * bufferCount,
                .mappedAtCreation = false};

        for (uint32_t i = 0; i < bufferCount; ++i) {
            mBuffers[i] = device.CreateBuffer(&descriptor);
        }
    }

    void WGPUVertexBuffer::setBuffer(WGPUBufferObject *bufferObject, uint32_t index) {}

    WGPUBufferObject::WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount)
            : HwBufferObject(byteCount),
              mBindingType(bindingType) {

    }
}
