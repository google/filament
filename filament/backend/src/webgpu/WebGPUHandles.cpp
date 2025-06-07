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

#include "WebGPUHandles.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <private/backend/BackendUtils.h>
#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

WGPURenderTarget::WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount,
        const MRT& colorAttachmentsMRT,
        const Attachment& depthAttachmentInfo,
        const Attachment& stencilAttachmentInfo)
    : HwRenderTarget(width, height),
      mDefaultRenderTarget(false),
      mSamples(samples),
      mLayerCount(layerCount),
      mColorAttachments(colorAttachmentsMRT),
      mDepthAttachment(depthAttachmentInfo),
      mStencilAttachment(stencilAttachmentInfo) {
    // TODO Make this an array
    mColorAttachmentDescriptors.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

wgpu::LoadOp WGPURenderTarget::getLoadOperation(RenderPassParams const& params,
                                                TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.clear & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear;
    }
    if (any(params.flags.discardStart & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear; // Or wgpu::LoadOp::Undefined if clear is not desired on discard
    }
    return wgpu::LoadOp::Load;
}

wgpu::StoreOp WGPURenderTarget::getStoreOperation(RenderPassParams const& params,
                                                  TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.discardEnd & bufferToOperateOn)) {
        return wgpu::StoreOp::Discard;
    }
    return wgpu::StoreOp::Store;
}

void WGPURenderTarget::setUpRenderPassAttachments(wgpu::RenderPassDescriptor& descriptor,
        RenderPassParams const& params, wgpu::TextureView const& defaultColorTextureView,
        wgpu::TextureView const& defaultDepthStencilTextureView,
        wgpu::TextureView const* customColorTextureViews, uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthTextureView,
        wgpu::TextureView const& customStencilTextureView, wgpu::TextureFormat customDepthFormat,
        wgpu::TextureFormat customStencilFormat) {
    mColorAttachmentDescriptors.clear();
    mHasDepthStencilAttachment = false;

    if (mDefaultRenderTarget) {
        assert_invariant(defaultColorTextureView);
        mColorAttachmentDescriptors.push_back({ .view = defaultColorTextureView,
            .resolveTarget = nullptr,
            .loadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::COLOR0),
            .storeOp = WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::COLOR0),
            .clearValue = { params.clearColor.r, params.clearColor.g, params.clearColor.b,
                params.clearColor.a } });

        if (defaultDepthStencilTextureView) {
            mDepthStencilAttachmentDescriptor = {
                .view = defaultDepthStencilTextureView,
                .depthLoadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH),
                .depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH),
                .depthClearValue = static_cast<float>(params.clearDepth),
                .depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0,
                .stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL),
                .stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL),
                .stencilClearValue = params.clearStencil,
                .stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0,
            };
            mHasDepthStencilAttachment = true;
        }
    } else {// Custom Render Target
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                mColorAttachmentDescriptors.push_back({ .view = customColorTextureViews[i],
                    // .resolveTarget = nullptr; // TODO: MSAA resolve for custom RT
                    .loadOp = WGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp =
                            WGPURenderTarget::getStoreOperation(params, getTargetBufferFlagsAt(i)),
                    .clearValue = { .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a } });
            }
        }

        FILAMENT_CHECK_POSTCONDITION(!(customDepthTextureView && customStencilTextureView))
                << "WebGPU CANNOT support separate texture views for depth + stencil. depth + "
                   "stencil needs to be in one texture view";

        const bool hasStencil =
                customStencilTextureView ||
                (customDepthFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        const bool hasDepth =
                customDepthTextureView ||
                (customStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        if (customDepthTextureView || customStencilTextureView) {
            assert_invariant((hasDepth || hasStencil) &&
                             "Depth or Texture view without a valid texture format");
            mDepthStencilAttachmentDescriptor = {};
            mDepthStencilAttachmentDescriptor.view =
                    customDepthTextureView ? customDepthTextureView : customStencilTextureView;

            if (hasDepth) {
                mDepthStencilAttachmentDescriptor.depthLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthClearValue =
                        static_cast<float>(params.clearDepth);
                mDepthStencilAttachmentDescriptor.depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.depthLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthReadOnly = true;
            }

            if (hasStencil) {
                mDepthStencilAttachmentDescriptor.stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilClearValue = params.clearStencil;
                mDepthStencilAttachmentDescriptor.stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.stencilLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilReadOnly = true;
            }
            mHasDepthStencilAttachment = true;
        }
    }

    descriptor.colorAttachmentCount = mColorAttachmentDescriptors.size();
    descriptor.colorAttachments = mColorAttachmentDescriptors.data();
    descriptor.depthStencilAttachment =
            mHasDepthStencilAttachment ? &mDepthStencilAttachmentDescriptor : nullptr;

    // descriptor.sampleCount was removed from the core spec. If your webgpu.h still has it,
    // and your Dawn version expects it, you might need to set it here based on this->samples.
    // e.g., descriptor.sampleCount = this->samples;
}

}// namespace filament::backend
