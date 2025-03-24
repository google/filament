//
// Created by Idris Idris Shah on 3/21/25.
//

#ifndef TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
#define TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H

#include "DriverBase.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <utils/FixedCapacityVector.h>

#include <webgpu/webgpu_cpp.h>

namespace filament::backend {

struct WGPUBufferObject;
struct WGPUVertexBufferInfo : public HwVertexBufferInfo {
    WGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
            AttributeArray const& attributes)
            : HwVertexBufferInfo(bufferCount, attributeCount),
              attributes(attributes) {
    }
    AttributeArray attributes;
};

struct WGPUVertexBuffer : public HwVertexBuffer {
    WGPUVertexBuffer(uint32_t vextexCount, uint32_t bufferCount, Handle<WGPUVertexBufferInfo> vbih);
    void setBuffer(WGPUBufferObject* bufferObject, uint32_t index);

    Handle<WGPUVertexBufferInfo> vbih;
    utils::FixedCapacityVector<WGPUBuffer> mBuffers;
};

struct WGPUBufferObject: HwBufferObject {
    WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount);

    WGPUBuffer mBuffer;
    const BufferObjectBinding mBindingType;
};
}
#endif // TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
