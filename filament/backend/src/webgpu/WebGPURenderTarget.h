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

#include "WebGPUTexture.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace filament::backend {

/**
 * A WebGPU implementation of the HwRenderTarget.
 * This class represents a collection of attachments (textures) that can be rendered to.
 */
class WebGPURenderTarget : public HwRenderTarget {
public:
    using Attachment = TargetBufferInfo; // Using TargetBufferInfo directly for attachments

    WebGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount,
            MRT const& colorAttachments, Attachment const& depthAttachment,
            Attachment const& stencilAttachment, TargetBufferFlags const& targetFlags,
            std::function<WebGPUTexture*(const Handle<HwTexture>)> const&,
            wgpu::Device const&);

    // Default constructor for the default render target
    WebGPURenderTarget();

    void setUpRenderPassAttachments(wgpu::RenderPassDescriptor& outDescriptor,
            RenderPassParams const& params,
            // For default render target:
            wgpu::TextureView const& defaultColorTextureView,
            wgpu::TextureView const& defaultDepthStencilTextureView,
            // For custom render targets:
            wgpu::TextureView const* customColorTextureViews,            // Array of views
            wgpu::TextureView const* customColorMsaaSidecarTextureViews, // nullptrs if N/A
            uint32_t customColorTextureViewCount,
            wgpu::TextureView const& customDepthStencilTextureView,
            wgpu::TextureView const& customDepthStencilMsaaSidecarTextureView /* nullptr if N/A */);

    [[nodiscard]] bool isDefaultRenderTarget() const { return mDefaultRenderTarget; }
    [[nodiscard]] uint8_t getSamples() const { return mSamples; }
    [[nodiscard]] uint8_t getSampleCountPerAttachment() const { return mSampleCountPerAttachment; }
    [[nodiscard]] uint8_t getLayerCount() const { return mLayerCount; }

    [[nodiscard]] MRT const& getColorAttachmentInfos() const { return mColorAttachments; }
    [[nodiscard]] Attachment const& getDepthAttachmentInfo() const { return mDepthAttachment; }
    [[nodiscard]] Attachment const& getStencilAttachmentInfo() const { return mStencilAttachment; }

    [[nodiscard]] static wgpu::LoadOp getLoadOperation(RenderPassParams const& params,
            TargetBufferFlags buffer);
    [[nodiscard]] static wgpu::StoreOp getStoreOperation(RenderPassParams const& params,
            TargetBufferFlags buffer);

    [[nodiscard]] TargetBufferFlags getTargetFlags() const { return mTargetFlags; }
    void setTargetFlags( TargetBufferFlags value) { mTargetFlags = value; }

private:
    bool mDefaultRenderTarget = false;
    TargetBufferFlags mTargetFlags = TargetBufferFlags::NONE;
    uint8_t mSamples = 1;
    uint8_t mLayerCount = 1;

    MRT mColorAttachments{};
    // TODO WebGPU only supports a DepthStencil attachment, should this be just
    // mDepthStencilAttachment?
    Attachment mDepthAttachment{};
    Attachment mStencilAttachment{};

    uint8_t mSampleCountPerAttachment = 0;

    // Cached descriptors for the render pass
    std::vector<wgpu::RenderPassColorAttachment> mColorAttachmentDesc;
    wgpu::RenderPassDepthStencilAttachment mDepthStencilAttachmentDesc{};
    bool mHasDepthStencilAttachment = false;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUHANDLES_H
