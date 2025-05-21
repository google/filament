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

#include <array>
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
    std::vector<wgpu::VertexBufferLayout> mVertexBufferLayout{};
    std::vector<std::vector<wgpu::VertexAttribute>> mAttributes{};
};

struct WGPUVertexBuffer : public HwVertexBuffer {
    WGPUVertexBuffer(wgpu::Device const &device, uint32_t vertexCount, uint32_t bufferCount,
                     Handle<HwVertexBufferInfo> vbih);

    Handle<HwVertexBufferInfo> vbih;
    utils::FixedCapacityVector<wgpu::Buffer> buffers;
};

class WGPUBufferBase {
public:
    void createBuffer(wgpu::Device const& device, wgpu::BufferUsage usage, uint32_t size,
            char const* label);
    void updateGPUBuffer(BufferDescriptor& bufferDescriptor, uint32_t byteOffset,
            wgpu::Queue queue);
    const wgpu::Buffer& getBuffer() const { return buffer; }
protected:
    wgpu::Buffer buffer;
private:
    // 4 bytes to hold any extra chunk we need.
    std::array<uint8_t,4> mRemainderChunk;
};

class WGPUIndexBuffer : public HwIndexBuffer, public WGPUBufferBase {
public:
    WGPUIndexBuffer(wgpu::Device const &device, uint8_t elementSize,
                    uint32_t indexCount);
    wgpu::IndexFormat indexFormat;
};

class WGPUBufferObject : public HwBufferObject, public WGPUBufferBase {
public:
    WGPUBufferObject(wgpu::Device const &device, BufferObjectBinding bindingType, uint32_t byteCount);
};

class WebGPUDescriptorSetLayout final : public HwDescriptorSetLayout {
public:

    struct BindGroupEntryInfo final {
        uint8_t binding = 0;
        bool hasDynamicOffset = false;
    };

    WebGPUDescriptorSetLayout(DescriptorSetLayout const& layout, wgpu::Device const& device);
    ~WebGPUDescriptorSetLayout();
    [[nodiscard]] const wgpu::BindGroupLayout& getLayout() const { return mLayout; }
    [[nodiscard]] std::vector<BindGroupEntryInfo> const& getBindGroupEntries() const {
        return mBindGroupEntries;
    }

private:
    // TODO: If this is useful elsewhere, remove it from this class
    // Convert Filament Shader Stage Flags bitmask to webgpu equivalent
    static wgpu::ShaderStage filamentStageToWGPUStage(ShaderStageFlags fFlags);
    std::vector<BindGroupEntryInfo> mBindGroupEntries;
    wgpu::BindGroupLayout mLayout;
};

class WebGPUDescriptorSet final : public HwDescriptorSet {
public:

    WebGPUDescriptorSet(wgpu::BindGroupLayout const& layout,
            std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const& bindGroupEntries);
    ~WebGPUDescriptorSet();

    wgpu::BindGroup lockAndReturn(wgpu::Device const&);
    void addEntry(unsigned int index, wgpu::BindGroupEntry&& entry);
    [[nodiscard]] bool getIsLocked() const { return mBindGroup != nullptr; }
    [[nodiscard]] size_t countEntitiesWithDynamicOffsets() const;

private:
    wgpu::BindGroupLayout mLayout = nullptr;
    static constexpr uint8_t INVALID_INDEX = MAX_DESCRIPTOR_COUNT + 1;
    std::array<uint8_t, MAX_DESCRIPTOR_COUNT> mEntryIndexByBinding{};
    std::vector<wgpu::BindGroupEntry> mEntries;
    const size_t mEntriesWithDynamicOffsetsCount;
    wgpu::BindGroup mBindGroup = nullptr;
};

class WGPUTexture : public HwTexture {
public:
    WGPUTexture(SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            wgpu::Device const& device) noexcept;

    WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept;
    wgpu::TextureAspect getAspect() const { return mAspect; }
    size_t getBlockWidth() const { return mBlockWidth; }
    size_t getBlockHeight() const { return mBlockHeight; }

    [[nodiscard]] const wgpu::Texture& getTexture() const { return mTexture; }
    [[nodiscard]] const wgpu::TextureView& getTexView() const { return mTexView; }

    static wgpu::TextureFormat fToWGPUTextureFormat(
            filament::backend::TextureFormat const& fFormat);
    static wgpu::TextureAspect fToWGPUTextureViewAspect(
            filament::backend::TextureUsage const& fUsage,
            filament::backend::TextureFormat const& fFormat);

private:
    wgpu::TextureView makeTextureView(const uint8_t& baseLevel, const uint8_t& levelCount,
            SamplerType target);
    // CreateTextureR has info for a texture and sampler. Texture Views are needed for binding,
    // along with a sampler Current plan: Inherit the sampler and Texture to always exist (It is a
    // ref counted pointer) when making views. View is optional
    wgpu::Texture mTexture = nullptr;
    wgpu::TextureUsage mUsage = wgpu::TextureUsage::None;
    wgpu::TextureFormat mFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureAspect mAspect = wgpu::TextureAspect::Undefined;
    uint32_t mArrayLayerCount = 1;
    wgpu::TextureView mTexView = nullptr;
    wgpu::TextureUsage fToWGPUTextureUsage(filament::backend::TextureUsage const& fUsage);
    size_t mBlockWidth;
    size_t mBlockHeight;
};

struct WGPURenderPrimitive : public HwRenderPrimitive {

    WGPUVertexBuffer* vertexBuffer = nullptr;
    WGPUIndexBuffer* indexBuffer = nullptr;
};



class WGPURenderTarget : public HwRenderTarget {
public:
    using Attachment = TargetBufferInfo; // Using TargetBufferInfo directly for attachments

    WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples,
            const MRT& colorAttachments,
            const Attachment& depthAttachment,
            const Attachment& stencilAttachment);

    // Default constructor for the default render target
    WGPURenderTarget()
        : HwRenderTarget(0, 0),
          defaultRenderTarget(true),
          samples(1) {}

    // Updated signature: takes resolved views for custom RTs, and default views for default RT
    void setUpRenderPassAttachments(
            wgpu::RenderPassDescriptor& descriptor,
            RenderPassParams const& params,
            // For default render target:
            wgpu::TextureView const& defaultColorTextureView,
            wgpu::TextureView const& defaultDepthStencilTextureView,
            // For custom render targets:
            wgpu::TextureView const* customColorTextureViews, // Array of views
            uint32_t customColorTextureViewCount,
            wgpu::TextureView const& customDepthTextureView,
            wgpu::TextureView const& customStencilTextureView);


    math::uint2 getAttachmentSize() const noexcept;

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    // Accessors for the driver to get stored attachment info
    const MRT& getColorAttachmentInfos() const { return mColorAttachments; }
    const Attachment& getDepthAttachmentInfo() const { return mDepthAttachment; }
    const Attachment& getStencilAttachmentInfo() const { return mStencilAttachment; }

    // Static helpers for load/store operations
    static wgpu::LoadOp getLoadOperation(const RenderPassParams& params, TargetBufferFlags buffer);
    static wgpu::StoreOp getStoreOperation(const RenderPassParams& params, TargetBufferFlags buffer);

private:
    // Helper to map MRT index to TargetBufferFlags
    static TargetBufferFlags getTargetBufferFlagsAt(int mrtIndex);

    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    MRT mColorAttachments{};
    Attachment mDepthAttachment{};
    Attachment mStencilAttachment{};

    // Cached descriptors for the render pass
    std::vector<wgpu::RenderPassColorAttachment> mColorAttachmentDescriptors;
    wgpu::RenderPassDepthStencilAttachment mDepthStencilAttachmentDescriptor{};
    bool mHasDepthStencilAttachment = false;
};
}// namespace filament::backend
#endif// TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
