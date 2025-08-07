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

#include "WebGPUTexture.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include <private/backend/BackendUtils.h>
#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string_view>

namespace filament::backend {

namespace {

/**
 * Panics if the number of samples in the original attachment textures do not match.
 * @return The number of samples in the original attachment textures (the number/count in each one).
 *         Returns 0 if there are no attachments.
 */
[[nodiscard]] uint8_t createMsaaSidecarTextures(const uint8_t renderTargetSampleCount,
        const TargetBufferFlags targetFlags, MRT const& colorAttachments,
        TargetBufferInfo const& depthAttachment, TargetBufferInfo const& stencilAttachment,
        std::function<WebGPUTexture*(const Handle<HwTexture>)> const& getWebGPUTexture,
        wgpu::Device const& device) {
    struct Target final {
        std::string_view name;
        TargetBufferFlags flag{ TargetBufferFlags::NONE };
        size_t colorIndex{ 0 };
    };
    // assuming 8 colors. hopefully the following static_assert will get tripped if another
    // color ever gets added
    static_assert(
            (TargetBufferFlags::COLOR0 | TargetBufferFlags::COLOR1 | TargetBufferFlags::COLOR2 |
                    TargetBufferFlags::COLOR3 | TargetBufferFlags::COLOR4 |
                    TargetBufferFlags::COLOR5 | TargetBufferFlags::COLOR6 |
                    TargetBufferFlags::COLOR7) == TargetBufferFlags::COLOR_ALL);
    const static std::array TARGETS{
        Target{ .name = "COLOR0", .flag = TargetBufferFlags::COLOR0, .colorIndex = 0 },
        Target{ .name = "COLOR1", .flag = TargetBufferFlags::COLOR1, .colorIndex = 1 },
        Target{ .name = "COLOR2", .flag = TargetBufferFlags::COLOR2, .colorIndex = 2 },
        Target{ .name = "COLOR3", .flag = TargetBufferFlags::COLOR3, .colorIndex = 3 },
        Target{ .name = "COLOR4", .flag = TargetBufferFlags::COLOR4, .colorIndex = 4 },
        Target{ .name = "COLOR5", .flag = TargetBufferFlags::COLOR5, .colorIndex = 5 },
        Target{ .name = "COLOR6", .flag = TargetBufferFlags::COLOR6, .colorIndex = 6 },
        Target{ .name = "COLOR7", .flag = TargetBufferFlags::COLOR7, .colorIndex = 7 },
        Target{ .name = "DEPTH", .flag = TargetBufferFlags::DEPTH },
        Target{ .name = "STENCIL", .flag = TargetBufferFlags::STENCIL },
    };
#ifndef NDEBUG
    for (auto const& target: TARGETS) {
        assert_invariant(target.colorIndex <= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT &&
                         "color index is >= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT, which should "
                         "not be possible and could lead to us indexing into the colorAttachments "
                         "array beyond its bounds.");
    }
#endif
    bool firstAttachment{ true };
    uint8_t sampleCountPerAttachment{ 0 };
    for (auto const& target: TARGETS) {
        if (any(targetFlags & target.flag)) {
            const Handle<HwTexture> textureHandle{
                target.flag == TargetBufferFlags::DEPTH
                        ? depthAttachment.handle
                        : (target.flag == TargetBufferFlags::STENCIL
                                          ? stencilAttachment.handle
                                          : colorAttachments[target.colorIndex].handle)
            };
            WebGPUTexture* const texture{ getWebGPUTexture(textureHandle) };
            assert_invariant(texture != nullptr && "target flag indicate the use of an attachment "
                                                   "for which we do not have a texture?");
            if (firstAttachment) {
                sampleCountPerAttachment = texture->samples;
                firstAttachment = false;
            }
            FILAMENT_CHECK_PRECONDITION(texture->samples == sampleCountPerAttachment)
                    << target.name << " attachment texture has " << +texture->samples
                    << " but the other attachment(s) have " << +sampleCountPerAttachment;
            if (renderTargetSampleCount > 1 && sampleCountPerAttachment == 1) {
                texture->createMsaaSidecarTextureIfNotAlreadyCreated(renderTargetSampleCount,
                        device);
            }
        }
    }
    assert_invariant(sampleCountPerAttachment);
    return sampleCountPerAttachment;
}

}  // namespace

WebGPURenderTarget::WebGPURenderTarget(const uint32_t width, const uint32_t height,
        const uint8_t samples, const uint8_t layerCount, MRT const& colorAttachmentsMRT,
        Attachment const& depthAttachmentInfo, Attachment const& stencilAttachmentInfo,
        TargetBufferFlags const& targetFlags,
        std::function<WebGPUTexture*(const Handle<HwTexture>)> const& getWebGPUTexture,
        wgpu::Device const& device)
    : HwRenderTarget{ width, height },
      mDefaultRenderTarget{ false },
      mTargetFlags{ targetFlags },
      mSamples{ samples },
      mLayerCount{ layerCount <= 0 ? static_cast<uint8_t>(1) : layerCount },
      mColorAttachments{ colorAttachmentsMRT },
      mDepthAttachment{ depthAttachmentInfo },
      mStencilAttachment{ stencilAttachmentInfo },
      mSampleCountPerAttachment{ createMsaaSidecarTextures(mSamples, mTargetFlags,
              mColorAttachments, mDepthAttachment, mStencilAttachment, getWebGPUTexture, device) } {
    // TODO consider possibly making this an array (that would avoid a heap allocation, but the
    //      concern is the size limitation on the handle itself)
    mColorAttachmentDesc.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

// Default constructor for the default render target
WebGPURenderTarget::WebGPURenderTarget()
    : HwRenderTarget{ 0, 0 },
      mDefaultRenderTarget{ true },
      // starting with the initial assumption that the default render target has a single color
      // attachment and a depth attachment, not necessarily a depth attachment
      mTargetFlags{ TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH },
      mSamples{ 1 },
      mLayerCount{ 1 },
      mSampleCountPerAttachment{ 1 } {}

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
        wgpu::TextureView const* customColorTextureViews,
        wgpu::TextureView const* customColorMsaaSidecarTextureViews,
        uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthStencilTextureView,
        wgpu::TextureView const& customDepthStencilMsaaSidecarTextureView) {
    const bool hasMsaaSidecars{ (customColorTextureViewCount > 0 &&
                                        customColorMsaaSidecarTextureViews[0]) ||
                                customDepthStencilMsaaSidecarTextureView };
    // either all the textures have MSAA sidecars or none of them do
    if (hasMsaaSidecars) {
        FILAMENT_CHECK_PRECONDITION(std::all_of(customColorMsaaSidecarTextureViews,
                customColorMsaaSidecarTextureViews + customColorTextureViewCount,
                [](wgpu::TextureView const& msaaView) { return msaaView != nullptr; }))
                << "A color or depth/stencil attachment texture has a MSAA sidecar but at least "
                   "one other color attachment texture does not.";
        FILAMENT_CHECK_PRECONDITION(customDepthStencilMsaaSidecarTextureView != nullptr)
                << "The color attachment texture(s) have MSAA sidecar(s) but the depth/stencil "
                   "texture does not.";
    } else {
        FILAMENT_CHECK_PRECONDITION(std::all_of(customColorMsaaSidecarTextureViews,
                customColorMsaaSidecarTextureViews + customColorTextureViewCount,
                [](wgpu::TextureView const& msaaView) { return msaaView == nullptr; }))
                << "A color or depth/stencil attachment texture does not have a MSAA sidecar but "
                   "at least one color attachment texture does.";
        FILAMENT_CHECK_PRECONDITION(customDepthStencilMsaaSidecarTextureView == nullptr)
                << "Custom color textures for the render target do not have MSAA sidecar(s) but "
                   "the depth/stencil texture does.";
    }

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
                .a = params.clearColor.a,
            },
        });
    } else {
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                const wgpu::TextureView msaaSidecar{ customColorMsaaSidecarTextureViews[i] };
                mColorAttachmentDesc.push_back({
                    .view = hasMsaaSidecars ? msaaSidecar : customColorTextureViews[i],
                    .resolveTarget = hasMsaaSidecars ? customColorTextureViews[i] : nullptr,
                    .loadOp =
                            WebGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp = WebGPURenderTarget::getStoreOperation(params,
                            getTargetBufferFlagsAt(i)),
                    .clearValue = {
                        .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a,
                    },
                });
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
                depthStencilViewToUse = hasMsaaSidecars ? customDepthStencilMsaaSidecarTextureView
                                                        : customDepthStencilTextureView;
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
