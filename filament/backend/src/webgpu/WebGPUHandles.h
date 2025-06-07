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
#include <backend/TargetBufferInfo.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <vector>

namespace filament::backend {

class WGPUTexture : public HwTexture {
public:
    WGPUTexture(SamplerType samplerType, uint8_t levels, TextureFormat format,
            uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
            wgpu::Device const& device) noexcept;

    WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept;
    wgpu::TextureAspect getAspect() const { return mAspect; }
    size_t getBlockWidth() const { return mBlockWidth; }
    size_t getBlockHeight() const { return mBlockHeight; }

    [[nodiscard]] const wgpu::Texture& getTexture() const { return mTexture; }

    [[nodiscard]] wgpu::TextureView getDefaultTextureView() const {return mDefaultTextureView;}
    [[nodiscard]] wgpu::TextureView getOrMakeTextureView(uint8_t mipLevel, uint32_t arrayLayer) const;

    [[nodiscard]] wgpu::TextureFormat getFormat() const { return mFormat; }
    [[nodiscard]] uint32_t getArrayLayerCount() const { return mArrayLayerCount; }

    static wgpu::TextureFormat fToWGPUTextureFormat(
            filament::backend::TextureFormat const& fFormat);
    static wgpu::TextureAspect fToWGPUTextureViewAspect(
            filament::backend::TextureUsage const& fUsage,
            filament::backend::TextureFormat const& fFormat);

private:
    // CreateTextureR has info for a texture and sampler. Texture Views are needed for binding,
    // along with a sampler Current plan: Inherit the sampler and Texture to always exist (It is a
    // ref counted pointer) when making views. View is optional
    wgpu::Texture mTexture = nullptr;
    wgpu::TextureFormat mFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureAspect mAspect = wgpu::TextureAspect::Undefined;
    wgpu::TextureUsage mUsage = wgpu::TextureUsage::None;
    uint32_t mArrayLayerCount = 1;
    size_t mBlockWidth;
    size_t mBlockHeight;
    SamplerType mSamplerType;
    wgpu::TextureView mDefaultTextureView = nullptr;
    uint32_t mDefaultMipLevel = 0;
    uint32_t mDefaultBaseArrayLayer = 0;

    [[nodiscard]] wgpu::TextureUsage fToWGPUTextureUsage(
            filament::backend::TextureUsage const& fUsage);

    [[nodiscard]] wgpu::TextureView makeTextureView(const uint8_t& baseLevel,
            const uint8_t& levelCount, const uint32_t& baseArrayLayer,
            const uint32_t& arrayLayerCount, SamplerType samplerType) const noexcept;
};

class WGPURenderTarget : public HwRenderTarget {
public:
    using Attachment = TargetBufferInfo; // Using TargetBufferInfo directly for attachments

    WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount,
            const MRT& colorAttachments,
            const Attachment& depthAttachment,
            const Attachment& stencilAttachment);

    // Default constructor for the default render target
    WGPURenderTarget()
        : HwRenderTarget(0, 0),
          mDefaultRenderTarget(true),
          mSamples(1) {}

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
            wgpu::TextureView const& customStencilTextureView,
            wgpu::TextureFormat customDepthFormat,
            wgpu::TextureFormat customStencilFormat);

    bool isDefaultRenderTarget() const { return mDefaultRenderTarget; }
    [[nodiscard]] uint8_t getSamples() const { return mSamples; }
    [[nodiscard]] uint8_t getLayerCount() const { return mLayerCount; }

    // Accessors for the driver to get stored attachment info
    const MRT& getColorAttachmentInfos() const { return mColorAttachments; }
    const Attachment& getDepthAttachmentInfo() const { return mDepthAttachment; }
    const Attachment& getStencilAttachmentInfo() const { return mStencilAttachment; }

    // Static helpers for load/store operations
    static wgpu::LoadOp getLoadOperation(const RenderPassParams& params, TargetBufferFlags buffer);
    static wgpu::StoreOp getStoreOperation(const RenderPassParams& params, TargetBufferFlags buffer);

private:
    bool mDefaultRenderTarget = false;
    uint8_t mSamples = 1;
    uint8_t mLayerCount = 1;

    MRT mColorAttachments{};
    // TODO WebGPU only supports a DepthStencil attachment, should this be just mDepthStencilAttachment?
    Attachment mDepthAttachment{};
    Attachment mStencilAttachment{};

    // Cached descriptors for the render pass
    std::vector<wgpu::RenderPassColorAttachment> mColorAttachmentDescriptors;
    wgpu::RenderPassDepthStencilAttachment mDepthStencilAttachmentDescriptor{};
    bool mHasDepthStencilAttachment = false;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
