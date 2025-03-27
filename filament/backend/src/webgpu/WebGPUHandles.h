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
          attributes(attributes) {}
    AttributeArray attributes;
};

struct WGPUVertexBuffer : public HwVertexBuffer {
    WGPUVertexBuffer(uint32_t vextexCount, uint32_t bufferCount, Handle<WGPUVertexBufferInfo> vbih);
    void setBuffer(WGPUBufferObject* bufferObject, uint32_t index);

    Handle<WGPUVertexBufferInfo> vbih;
    utils::FixedCapacityVector<wgpu::Buffer> mBuffers;
};

struct WGPUIndexBuffer : public HwIndexBuffer {
    WGPUIndexBuffer(BufferUsage usage, uint8_t elementSize, uint32_t indexCount);

    wgpu::Buffer buffer;
};

struct WGPUBufferObject : HwBufferObject {
    WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount);

    wgpu::Buffer mBuffer;
    const BufferObjectBinding mBindingType;
};

class WGPUTexture : public HwTexture {
public:
    WGPUTexture(SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage) noexcept;

    // constructors for creating texture views
    WGPUTexture(WGPUTexture const* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

    wgpu::Texture texture = nullptr;
};

class WGPURenderTarget : public HwRenderTarget {
public:
    class Attachment {
    public:
        friend class WGPURenderTarget;

        Attachment() = default;
        Attachment(WGPUTexture* gpuTexture, uint8_t level = 0, uint16_t layer = 0)
            : level(level),
              layer(layer),
              texture(gpuTexture->texture),
              mWGPUTexture(gpuTexture) {}

        uint8_t level = 0;
        uint16_t layer = 0;

    private:
        wgpu::Texture texture = nullptr;
        WGPUTexture* mWGPUTexture = nullptr;
    };

    WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples,
            Attachment colorAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]);
    WGPURenderTarget()
        : HwRenderTarget(0, 0),
          defaultRenderTarget(true) {}

    void setUpRenderPassAttachments(wgpu::RenderPassDescriptor* descriptor,
            const RenderPassParams& params);

    math::uint2 getAttachmentSize() noexcept;

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    Attachment getDrawColorAttachment(size_t index);
    Attachment getReadColorAttachment(size_t index);

private:
    static wgpu::LoadOp getLoadAction(const RenderPassParams& params, TargetBufferFlags buffer);
    static wgpu::LoadOp getStoreAction(const RenderPassParams& params, TargetBufferFlags buffer);

    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    Attachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    math::uint2 attachmentSize = {};
};

}// namespace filament::backend
#endif// TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
