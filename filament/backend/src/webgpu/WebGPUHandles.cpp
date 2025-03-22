//
// Created by Idris Idris Shah on 3/21/25.
//

#include "WebGPUHandles.h"

namespace filament::backend {

WGPUVertexBuffer::WGPUVertexBuffer(uint32_t vextexCount, uint32_t bufferCount,
        Handle<WGPUVertexBufferInfo> vbih)
    : HwVertexBuffer(vextexCount),
      vbih(vbih),
      mBuffers(MAX_VERTEX_BUFFER_COUNT) {}
void WGPUVertexBuffer::setBuffer(WGPUBufferObject* bufferObject, uint32_t index) {}

WGPUBufferObject::WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount)
    : HwBufferObject(byteCount),
      mBindingType(bindingType) {}
}// namespace filament::backend
