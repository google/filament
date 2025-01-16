/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "details/RenderTarget.h"

#include "details/Engine.h"
#include "details/Texture.h"

#include "FilamentAPI-impl.h"

#include <filament/RenderTarget.h>

#include <utils/compiler.h>
#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>

#include <algorithm>
#include <iterator>
#include <limits>

#include <stdint.h>
#include <stddef.h>


namespace filament {

using namespace backend;

struct RenderTarget::BuilderDetails {
    FRenderTarget::Attachment mAttachments[FRenderTarget::ATTACHMENT_COUNT] = {};
    uint32_t mWidth{};
    uint32_t mHeight{};
    uint8_t mSamples = 1;   // currently not settable in the public facing API
    // The number of layers for the render target. The value should be 1 except for multiview.
    // If multiview is enabled, this value is appropriately updated based on the layerCount value
    // from each attachment. Hence, #>1 means using multiview
    uint8_t mLayerCount = 1;
};

using BuilderType = RenderTarget;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

RenderTarget::Builder& RenderTarget::Builder::texture(AttachmentPoint pt, Texture* texture) noexcept {
    mImpl->mAttachments[(size_t)pt].texture = downcast(texture);
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::mipLevel(AttachmentPoint pt, uint8_t level) noexcept {
    mImpl->mAttachments[(size_t)pt].mipLevel = level;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::face(AttachmentPoint pt, CubemapFace face) noexcept {
    mImpl->mAttachments[(size_t)pt].face = face;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::layer(AttachmentPoint pt, uint32_t layer) noexcept {
    mImpl->mAttachments[(size_t)pt].layer = layer;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::multiview(AttachmentPoint pt, uint8_t layerCount,
        uint8_t baseLayer/*= 0*/) noexcept {
    mImpl->mAttachments[(size_t)pt].layer = baseLayer;
    mImpl->mAttachments[(size_t)pt].layerCount = layerCount;
    return *this;
}


RenderTarget* RenderTarget::Builder::build(Engine& engine) {
    using backend::TextureUsage;
    const FRenderTarget::Attachment& color = mImpl->mAttachments[(size_t)AttachmentPoint::COLOR0];
    const FRenderTarget::Attachment& depth = mImpl->mAttachments[(size_t)AttachmentPoint::DEPTH];

    if (color.texture) {
        FILAMENT_CHECK_PRECONDITION(color.texture->getUsage() & TextureUsage::COLOR_ATTACHMENT)
                << "Texture usage must contain COLOR_ATTACHMENT";
        FILAMENT_CHECK_PRECONDITION(color.texture->getTarget() != Texture::Sampler::SAMPLER_EXTERNAL)
                << "Color attachment can't be an external texture";
    }

    if (depth.texture) {
        FILAMENT_CHECK_PRECONDITION(depth.texture->getUsage() & TextureUsage::DEPTH_ATTACHMENT)
                << "Texture usage must contain DEPTH_ATTACHMENT";
        FILAMENT_CHECK_PRECONDITION(depth.texture->getTarget() != Texture::Sampler::SAMPLER_EXTERNAL)
                        << "Depth attachment can't be an external texture";
    }

    const size_t maxDrawBuffers = downcast(engine).getDriverApi().getMaxDrawBuffers();
    for (size_t i = maxDrawBuffers; i < MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT; i++) {
        FILAMENT_CHECK_PRECONDITION(!mImpl->mAttachments[i].texture)
                << "Only " << maxDrawBuffers << " color attachments are supported, but COLOR" << i
                << " attachment is set";
    }
    
    uint32_t minWidth = std::numeric_limits<uint32_t>::max();
    uint32_t maxWidth = 0;
    uint32_t minHeight = std::numeric_limits<uint32_t>::max();
    uint32_t maxHeight = 0;
    uint32_t minLayerCount = std::numeric_limits<uint32_t>::max();
    uint32_t maxLayerCount = 0;
    for (auto const& attachment : mImpl->mAttachments) {
        if (attachment.texture) {
            const uint32_t w = attachment.texture->getWidth(attachment.mipLevel);
            const uint32_t h = attachment.texture->getHeight(attachment.mipLevel);
            const uint32_t d = attachment.texture->getDepth(attachment.mipLevel);
            const uint32_t l = attachment.layerCount;
            if (l > 0) {
                FILAMENT_CHECK_PRECONDITION(
                        attachment.texture->getTarget() == Texture::Sampler::SAMPLER_2D_ARRAY)
                        << "Texture sampler must be of 2d array for multiview";
            }
            FILAMENT_CHECK_PRECONDITION(attachment.layer + l <= d)
                    << "layer + layerCount cannot exceed the number of depth";
            minWidth  = std::min(minWidth, w);
            minHeight = std::min(minHeight, h);
            minLayerCount = std::min(minLayerCount, l);
            maxWidth  = std::max(maxWidth, w);
            maxHeight = std::max(maxHeight, h);
            maxLayerCount = std::max(maxLayerCount, l);
        }
    }

    FILAMENT_CHECK_PRECONDITION(minWidth == maxWidth && minHeight == maxHeight
            && minLayerCount == maxLayerCount) << "All attachments dimensions must match";

    mImpl->mWidth  = minWidth;
    mImpl->mHeight = minHeight;
    if (minLayerCount > 0) {
        // mLayerCount should be 1 except for multiview use where we update this variable
        // to the number of layerCount for multiview.
        mImpl->mLayerCount = minLayerCount;
    }
    return downcast(engine).createRenderTarget(*this);
}

// ------------------------------------------------------------------------------------------------

FRenderTarget::FRenderTarget(FEngine& engine, const Builder& builder)
    : mSupportedColorAttachmentsCount(engine.getDriverApi().getMaxDrawBuffers()),
      mSupportsReadPixels(false) {
    std::copy(std::begin(builder.mImpl->mAttachments), std::end(builder.mImpl->mAttachments),
            std::begin(mAttachments));

    MRT mrt{};
    TargetBufferInfo dinfo{};

    auto setAttachment = [this, &driver = engine.getDriverApi()]
            (TargetBufferInfo& info, AttachmentPoint attachmentPoint) {
        Attachment const& attachment = mAttachments[(size_t)attachmentPoint];
        auto t = downcast(attachment.texture);
        info.handle = t->getHwHandle();
        info.level  = attachment.mipLevel;
        if (t->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP) {
            info.layer = +attachment.face;
        } else {
            info.layer = attachment.layer;
        }
        t->updateLodRange(info.level);
    };

    UTILS_NOUNROLL
    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        Attachment const& attachment = mAttachments[i];
        if (attachment.texture) {
            TargetBufferFlags const targetBufferBit = getTargetBufferFlagsAt(i);
            mAttachmentMask |= targetBufferBit;
            setAttachment(mrt[i], (AttachmentPoint)i);
            if (any(attachment.texture->getUsage() &
                    (TextureUsage::SAMPLEABLE | Texture::Usage::SUBPASS_INPUT))) {
                mSampleableAttachmentsMask |= targetBufferBit;
            }

            // readPixels() only applies to the color attachment that binds at index 0.
            if (i == 0 && any(attachment.texture->getUsage() & TextureUsage::COLOR_ATTACHMENT)) {

                // TODO: the following will be changed to
                //    mSupportsReadPixels =
                //            any(attachment.texture->getUsage() & TextureUsage::BLIT_SRC);
                //    in a later filament version when clients have properly added the right usage.
                mSupportsReadPixels = attachment.texture->hasBlitSrcUsage();
            }
        }
    }

    Attachment const& depthAttachment = mAttachments[(size_t)AttachmentPoint::DEPTH];
    if (depthAttachment.texture) {
        mAttachmentMask |= TargetBufferFlags::DEPTH;
        setAttachment(dinfo, AttachmentPoint::DEPTH);
        if (any(depthAttachment.texture->getUsage() &
                (TextureUsage::SAMPLEABLE | Texture::Usage::SUBPASS_INPUT))) {
            mSampleableAttachmentsMask |= TargetBufferFlags::DEPTH;
        }
    }

    // TODO: add stencil here when we support it

    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createRenderTarget(mAttachmentMask,
            builder.mImpl->mWidth, builder.mImpl->mHeight, builder.mImpl->mSamples,
            builder.mImpl->mLayerCount, mrt, dinfo, {});
}

void FRenderTarget::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyRenderTarget(mHandle);
}

bool FRenderTarget::hasSampleableDepth() const noexcept {
    const FTexture* depth = mAttachments[(size_t)AttachmentPoint::DEPTH].texture;
    return depth && (depth->getUsage() & TextureUsage::SAMPLEABLE) == TextureUsage::SAMPLEABLE;
}

} // namespace filament
