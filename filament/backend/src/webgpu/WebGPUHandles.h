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
