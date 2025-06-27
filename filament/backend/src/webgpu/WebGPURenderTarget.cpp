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
    mColorAttachmentDesc.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
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
        wgpu::TextureView const* customColorTextureViews, uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthStencilTextureView) {
    mColorAttachmentDesc.clear();

    const bool hasDepth = any(mTargetFlags & TargetBufferFlags::DEPTH);
    const bool hasStencil = any(mTargetFlags & TargetBufferFlags::STENCIL);
    mHasDepthStencilAttachment = hasDepth || hasStencil;
    bool const depthReadOnly = (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0;

    // Color attachments
    if (mDefaultRenderTarget) {
        assert_invariant(defaultColorTextureView);
        mColorAttachmentDesc.push_back({
            .view = defaultColorTextureView,
            .resolveTarget = nullptr,
            .loadOp = WebGPURenderTarget::getLoadOperation(params, TargetBufferFlags::COLOR0),
            .storeOp = WebGPURenderTarget::getStoreOperation(params, TargetBufferFlags::COLOR0),
            .clearValue = {
                .r = params.clearColor.r,
                .g = params.clearColor.g,
                .b = params.clearColor.b,
                .a = params.clearColor.a }});
    } else {
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                mColorAttachmentDesc.push_back({
                    .view = customColorTextureViews[i],
                    .resolveTarget = nullptr, // We handle MSAA on the WebGPU driver's resolve function
                    .loadOp =
                            WebGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp = WebGPURenderTarget::getStoreOperation(params,
                            getTargetBufferFlagsAt(i)),
                    .clearValue = {
                        .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a }});
            }
        }
    }
    outDescriptor.colorAttachmentCount = mColorAttachmentDesc.size();
    outDescriptor.colorAttachments = mColorAttachmentDesc.data();

    // Depth/Stencil Attachments
    if (mHasDepthStencilAttachment) {
        wgpu::TextureView depthStencilViewToUse{ nullptr };
        if (mDefaultRenderTarget) {
            depthStencilViewToUse = defaultDepthStencilTextureView;
            assert_invariant(depthStencilViewToUse);
        } else {
            if (customDepthStencilTextureView) {
                depthStencilViewToUse = customDepthStencilTextureView;
            }
        }

        if (depthStencilViewToUse) {
            mDepthStencilAttachmentDesc = {};
            mDepthStencilAttachmentDesc.view = depthStencilViewToUse;

            if (hasDepth) {
                mDepthStencilAttachmentDesc.depthLoadOp =
                        depthReadOnly ? wgpu::LoadOp::Undefined
                                      : getLoadOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDesc.depthStoreOp =
                        depthReadOnly ? wgpu::StoreOp::Undefined
                                      : getStoreOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDesc.depthClearValue = static_cast<float>(params.clearDepth);
                mDepthStencilAttachmentDesc.depthReadOnly = depthReadOnly;
            } else {
                mDepthStencilAttachmentDesc.depthLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDesc.depthStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDesc.depthReadOnly = true;
            }

            if (hasStencil) {
                mDepthStencilAttachmentDesc.stencilLoadOp =
                        getLoadOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDesc.stencilStoreOp =
                        getStoreOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDesc.stencilClearValue = params.clearStencil;
                mDepthStencilAttachmentDesc.stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0;
            } else {
                mDepthStencilAttachmentDesc.stencilLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDesc.stencilStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDesc.stencilReadOnly = true;
            }
            outDescriptor.depthStencilAttachment = &mDepthStencilAttachmentDesc;

        } else {
            outDescriptor.depthStencilAttachment = nullptr;
            mDepthStencilAttachmentDesc = {};
        }

    } else {
        outDescriptor.depthStencilAttachment = nullptr;
        mDepthStencilAttachmentDesc = {};
    }
}

} // namespace filament::backend
