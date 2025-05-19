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
#include <bitset>
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

    enum class BindGroupEntryType : uint8_t {
        UNIFORM_BUFFER,
        TEXTURE_VIEW,
        SAMPLER
    };

    struct BindGroupEntryInfo final {
        uint8_t binding = 0;
        BindGroupEntryType type = BindGroupEntryType::UNIFORM_BUFFER;
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
    // Convert Filament Shader Stage Flags bitmask to webgpu equivilant
    static wgpu::ShaderStage filamentStageToWGPUStage(ShaderStageFlags fFlags);
    std::vector<BindGroupEntryInfo> mBindGroupEntries;
    wgpu::BindGroupLayout mLayout;
};

class WebGPUDescriptorSet final : public HwDescriptorSet {
public:
    static void initializeDummyResourcesIfNotAlready(wgpu::Device const&,
            wgpu::TextureFormat aColorFormat);

    WebGPUDescriptorSet(wgpu::BindGroupLayout const& layout,
            std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const& bindGroupEntries);
    ~WebGPUDescriptorSet();

    wgpu::BindGroup lockAndReturn(wgpu::Device const&);
    void addEntry(unsigned int index, wgpu::BindGroupEntry&& entry);
    [[nodiscard]] bool getIsLocked() const { return mBindGroup != nullptr; }
    [[nodiscard]] size_t countEntitiesWithDynamicOffsets() const;

private:
    static wgpu::Buffer sDummyUniformBuffer;
    static wgpu::Texture sDummyTexture;
    static wgpu::TextureView sDummyTextureView;
    static wgpu::Sampler sDummySampler;

    static std::vector<wgpu::BindGroupEntry> createDummyEntriesSortedByBinding(
            std::vector<filament::backend::WebGPUDescriptorSetLayout::BindGroupEntryInfo> const&);

    // TODO: Consider storing what we used to make the layout. However we need to essentially
    // Recreate some of the info (Sampler in slot X with the actual sampler) so letting Dawn confirm
    // there isn't a mismatch may be easiest.
    // Also storing the wgpu ObjectBase takes care of ownership challenges in theory
    wgpu::BindGroupLayout mLayout = nullptr;
    static constexpr uint8_t INVALID_INDEX = MAX_DESCRIPTOR_COUNT + 1;
    std::array<uint8_t, MAX_DESCRIPTOR_COUNT> mEntryIndexByBinding{};
    std::vector<wgpu::BindGroupEntry> mEntriesSortedByBinding;
    std::bitset<MAX_DESCRIPTOR_COUNT> mEntriesByBindingWithDynamicOffsets{};
    std::bitset<MAX_DESCRIPTOR_COUNT> mEntriesByBindingAdded{};
    std::vector<uint32_t> mDynamicOffsets;
    wgpu::BindGroup mBindGroup = nullptr;
};

class WGPUTexture : public HwTexture {
public:
    WGPUTexture(SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
            uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            wgpu::Device const& device) noexcept;

    WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept;

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
};

struct WGPURenderPrimitive : public HwRenderPrimitive {

    WGPUVertexBuffer* vertexBuffer = nullptr;
    WGPUIndexBuffer* indexBuffer = nullptr;
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
              texture(gpuTexture->getTexture()),
              mWGPUTexture(gpuTexture) {}
        operator bool() const {
            return mWGPUTexture != nullptr;
        }

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

    void setUpRenderPassAttachments(wgpu::RenderPassDescriptor& descriptor,
            wgpu::TextureView const& textureView, RenderPassParams const& params);

    math::uint2 getAttachmentSize() noexcept;

    bool isDefaultRenderTarget() const { return defaultRenderTarget; }
    uint8_t getSamples() const { return samples; }

    Attachment getDrawColorAttachment(size_t index);
    Attachment getReadColorAttachment(size_t index);

    static wgpu::LoadOp getLoadOperation(const RenderPassParams& params, TargetBufferFlags buffer);
    static wgpu::StoreOp getStoreOperation(const RenderPassParams& params, TargetBufferFlags buffer);
private:
    bool defaultRenderTarget = false;
    uint8_t samples = 1;

    Attachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    math::uint2 attachmentSize = {};
    std::vector<wgpu::RenderPassColorAttachment> colorAttachments{};
};

}// namespace filament::backend
#endif// TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
