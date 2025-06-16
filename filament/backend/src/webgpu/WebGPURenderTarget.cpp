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

#include "WebGPURenderTarget.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <private/backend/BackendUtils.h>
#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

WebGPURenderTarget::WebGPURenderTarget(const uint32_t width, const uint32_t height,
        const uint8_t samples, const uint8_t layerCount, MRT const& colorAttachmentsMRT,
        Attachment const& depthAttachmentInfo, Attachment const& stencilAttachmentInfo, TargetBufferFlags const& targetFlags)
    : HwRenderTarget{ width, height },
      mDefaultRenderTarget{ false },
      mTargetFlags{ targetFlags },
      mSamples{ samples },
      mLayerCount{ layerCount },
      mColorAttachments{ colorAttachmentsMRT },
      mDepthAttachment{ depthAttachmentInfo },
      mStencilAttachment{ stencilAttachmentInfo } {
    // TODO consider making this an array
    mColorAttachmentDescriptors.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

// Default constructor for the default render target
WebGPURenderTarget::WebGPURenderTarget()
    : HwRenderTarget{ 0, 0 },
      mDefaultRenderTarget{ true },
      mTargetFlags{ TargetBufferFlags::NONE },
      mSamples{ 1 },
      mLayerCount{ 1 }
     {}

wgpu::LoadOp WebGPURenderTarget::getLoadOperation(RenderPassParams const& params,
        const TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.clear & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear;
    }
    if (any(params.flags.discardStart & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear;
    }
    return wgpu::LoadOp::Load;
}

wgpu::StoreOp WebGPURenderTarget::getStoreOperation(RenderPassParams const& params,
        const TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.discardEnd & bufferToOperateOn)) {
        return wgpu::StoreOp::Discard;
    }
    return wgpu::StoreOp::Store;
}

void WebGPURenderTarget::setUpRenderPassAttachments(wgpu::RenderPassDescriptor& outDescriptor,
        RenderPassParams const& params, wgpu::TextureView const& defaultColorTextureView,
        wgpu::TextureView const& defaultDepthStencilTextureView,
        wgpu::TextureFormat const& defaultDepthStencilFormat,
        wgpu::TextureView const* customColorTextureViews, uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthTextureView,
        wgpu::TextureView const& customStencilTextureView,
        const wgpu::TextureFormat customDepthFormat,
        const wgpu::TextureFormat customStencilFormat) {
    mColorAttachmentDescriptors.clear();

    const bool hasDepth = any(mTargetFlags & TargetBufferFlags::DEPTH);
    const bool hasStencil = any(mTargetFlags & TargetBufferFlags::STENCIL);
    mHasDepthStencilAttachment = hasDepth || hasStencil;
    bool const depthReadOnly =
                (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0;


    // Color attachments
    if (mDefaultRenderTarget) {
        assert_invariant(defaultColorTextureView);
        mColorAttachmentDescriptors.push_back({
            .view = defaultColorTextureView,
            .resolveTarget = nullptr,
            .loadOp = WebGPURenderTarget::getLoadOperation(params, TargetBufferFlags::COLOR0),
            .storeOp = WebGPURenderTarget::getStoreOperation(params, TargetBufferFlags::COLOR0),
            .clearValue = { params.clearColor.r, params.clearColor.g, params.clearColor.b,
                params.clearColor.a }
        });
    } else {
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                mColorAttachmentDescriptors.push_back({ .view = customColorTextureViews[i],
                    // .resolveTarget = nullptr; // TODO: MSAA resolve for custom RT
                    .loadOp =
                            WebGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp = WebGPURenderTarget::getStoreOperation(params,
                            getTargetBufferFlagsAt(i)),
                    .clearValue = { .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a } });
            }
        }
    }
    outDescriptor.colorAttachmentCount = mColorAttachmentDescriptors.size();
    outDescriptor.colorAttachments = mColorAttachmentDescriptors.data();

    if(mHasDepthStencilAttachment){
        wgpu::TextureView depthStencilViewToUse;

        if (mDefaultRenderTarget) {
            depthStencilViewToUse = defaultDepthStencilTextureView;
            assert_invariant(depthStencilViewToUse);
        } else {
            if (customDepthTextureView || customStencilTextureView) {
                // WebGPU requires combined depth/stencil textures. Ensure consistency for custom RTs.
                FILAMENT_CHECK_POSTCONDITION(
                        customDepthTextureView.Get() == customStencilTextureView.Get() ||
                        !customDepthTextureView || !customStencilTextureView)
                        << "WebGPU backend requires depth and stencil to be in a single texture. "
                        << "If both are specified, they must refer to the same resource "
                           "(wgpu::TextureView).";

                depthStencilViewToUse = customDepthTextureView ? customDepthTextureView : customStencilTextureView;
            }
        }
    }

     // Depth/Stencil Attachments
    if (mHasDepthStencilAttachment) {
        wgpu::TextureView depthStencilViewToUse;
        if (mDefaultRenderTarget) {
            depthStencilViewToUse = defaultDepthStencilTextureView;

            assert_invariant(depthStencilViewToUse);
        } else {
            if (customDepthTextureView || customStencilTextureView) {

                FILAMENT_CHECK_POSTCONDITION(
                        customDepthTextureView.Get() == customStencilTextureView.Get() ||
                        !customDepthTextureView || !customStencilTextureView)
                        << "WebGPU backend requires depth and stencil to be in a single texture. "
                        << "If both are specified, they must refer to the same resource "
                           "(wgpu::TextureView).";

                depthStencilViewToUse = customDepthTextureView ? customDepthTextureView : customStencilTextureView;
            }
        }

        if (depthStencilViewToUse) {
            mDepthStencilAttachmentDescriptor = {};
            mDepthStencilAttachmentDescriptor.view = depthStencilViewToUse;

            if (hasDepth) {
                mDepthStencilAttachmentDescriptor.depthLoadOp = depthReadOnly ? wgpu::LoadOp::Undefined :getLoadOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthStoreOp = depthReadOnly ? wgpu::StoreOp::Undefined :getStoreOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthClearValue = params.clearDepth;
                mDepthStencilAttachmentDescriptor.depthReadOnly = depthReadOnly;
            } else {
                mDepthStencilAttachmentDescriptor.depthLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthReadOnly = true;
            }

            if (hasStencil) {
                mDepthStencilAttachmentDescriptor.stencilLoadOp = getLoadOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilStoreOp = getStoreOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilClearValue = params.clearStencil;
                mDepthStencilAttachmentDescriptor.stencilReadOnly = (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.stencilLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilReadOnly = true;
            }
            outDescriptor.depthStencilAttachment = &mDepthStencilAttachmentDescriptor;

        } else {

            outDescriptor.depthStencilAttachment = nullptr;
            mDepthStencilAttachmentDescriptor = {};
        }

    } else {
        outDescriptor.depthStencilAttachment = nullptr;
        mDepthStencilAttachmentDescriptor = {};
    }

    // descriptor.sampleCount was removed from the core spec. If your webgpu.h still has it,
    // and your Dawn version expects it, you might need to set it here based on this->samples.
    // e.g., descriptor.sampleCount = this->samples;
}

} // namespace filament::backend
