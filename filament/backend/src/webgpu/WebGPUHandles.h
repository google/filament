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


#ifndef TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
#define TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H

#include "DriverBase.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/FixedCapacityVector.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <vector>

namespace filament::backend {

class WGPUProgram final : public HwProgram {
public:
    WGPUProgram(wgpu::Device&, Program&);

    wgpu::ShaderModule vertexShaderModule = nullptr;
    wgpu::ShaderModule fragmentShaderModule = nullptr;
    wgpu::ShaderModule computeShaderModule = nullptr;
    std::vector<wgpu::ConstantEntry> constants;
};

struct WGPUBufferObject;

// VertexBufferInfo contains layout info for Vertex Buffer based on WebGPU structs. In WebGPU each
// VertexBufferLayout is associated with a single vertex buffer. So number of mVertexBufferLayout
// is equal to bufferCount. Each VertexBufferLayout can contain multiple VertexAttribute. Bind index
// of vertex buffer is implicitly calculated by the position of VertexBufferLayout in an array.
class WGPUVertexBufferInfo : public HwVertexBufferInfo {
public:
    WGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
            AttributeArray const& attributes);
    inline  wgpu::VertexBufferLayout const* getVertexBufferLayout() const {
        return mVertexBufferLayout.data();
    }

    inline uint32_t getVertexBufferLayoutSize() const {
        return mVertexBufferLayout.size();
    }

    inline wgpu::VertexAttribute const* getVertexAttributeForIndex(uint32_t index) const {
        assert_invariant(index < mAttributes.size());
        return mAttributes[index].data();
    }

    inline uint32_t getVertexAttributeSize(uint32_t index) const {
        assert_invariant(index < mAttributes.size());
        return mAttributes[index].size();
    }

private:
    // TODO: can we do better in terms on heap management.
    std::vector<wgpu::VertexBufferLayout> mVertexBufferLayout {};
    std::vector<std::vector<wgpu::VertexAttribute>> mAttributes {};
};

struct WGPUVertexBuffer : public HwVertexBuffer {
    WGPUVertexBuffer(wgpu::Device const &device, uint32_t vertexCount, uint32_t bufferCount,
                     Handle<HwVertexBufferInfo> vbih);

    Handle<HwVertexBufferInfo> vbih;
    utils::FixedCapacityVector<wgpu::Buffer> buffers;
};

struct WGPUIndexBuffer : public HwIndexBuffer {
    WGPUIndexBuffer(wgpu::Device const &device, uint8_t elementSize,
                    uint32_t indexCount);

    wgpu::Buffer buffer;
    wgpu::IndexFormat indexFormat;
};

struct WGPUBufferObject : HwBufferObject {
    WGPUBufferObject(wgpu::Device const &device, BufferObjectBinding bindingType, uint32_t byteCount);

    wgpu::Buffer buffer = nullptr;
    const BufferObjectBinding bufferObjectBinding;
};
class WebGPUDescriptorSetLayout final : public HwDescriptorSetLayout {
public:
    WebGPUDescriptorSetLayout(DescriptorSetLayout const& layout, wgpu::Device const& device);
    ~WebGPUDescriptorSetLayout();
    [[nodiscard]] const wgpu::BindGroupLayout& getLayout() const { return mLayout; }
    [[nodiscard]] uint getLayoutSize() const { return mLayoutSize; }

private:
    // TODO: If this is useful elsewhere, remove it from this class
    // Convert Filament Shader Stage Flags bitmask to webgpu equivilant
    static wgpu::ShaderStage filamentStageToWGPUStage(ShaderStageFlags fFlags);
    uint mLayoutSize;
    wgpu::BindGroupLayout mLayout;
};

class WebGPUDescriptorSet final : public HwDescriptorSet {
public:
    WebGPUDescriptorSet(const wgpu::BindGroupLayout& layout, uint layoutSize);
    ~WebGPUDescriptorSet();

    wgpu::BindGroup lockAndReturn(wgpu::Device const& device);
    void addEntry(uint index, wgpu::BindGroupEntry&& entry);
    [[nodiscard]] bool getIsLocked() const { return mBindGroup != nullptr; }

private:
    // TODO: Consider storing what we used to make the layout. However we need to essentially
    // Recreate some of the info (Sampler in slot X with the actual sampler) so letting Dawn confirm
    // there isn't a mismatch may be easiest.
    // Also storing the wgpu ObjectBase takes care of ownership challenges in theory
    wgpu::BindGroupLayout mLayout;
    std::vector<wgpu::BindGroupEntry> entries;
    wgpu::BindGroup mBindGroup;
};

// TODO: Currently WGPUTexture is not used by WebGPU for useful task.
// Update the struct when used by WebGPU driver.
struct WGPUTexture : public HwTexture {
    WGPUTexture(SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage) noexcept;

    // constructors for creating texture views
    WGPUTexture(WGPUTexture const* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

    wgpu::Texture texture = nullptr;
    // TODO: Adding this but not yet setting it up. Filament "Textures" are combined image samplers,
    // rep both.
    wgpu::Sampler sampler = nullptr;
    //TODO: Not sure all the ways HwTexture is used. Overloading like this might be entirely wrong.
    wgpu::TextureView texView = nullptr;
};

struct WGPURenderPrimitive : public HwRenderPrimitive {
    WGPURenderPrimitive();

    void setBuffers(WGPUVertexBufferInfo const* const vbi,
            WGPUVertexBuffer* vertexBuffer, WGPUIndexBuffer* indexBuffer);

    WGPUVertexBuffer* vertexBuffer = nullptr;
    WGPUIndexBuffer* indexBuffer = nullptr;
};

// TODO: Currently WGPURenderTarget is not used by WebGPU for useful task.
// Update the struct when used by WebGPU driver.
struct WGPURenderTarget : public HwRenderTarget {
    class Attachment {
    public:
        friend struct WGPURenderTarget;

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
