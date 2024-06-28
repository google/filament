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

#include <utils/Panic.h>
#include <filament/RenderTarget.h>


namespace filament {

using namespace backend;

struct RenderTarget::BuilderDetails {
    FRenderTarget::Attachment mAttachments[FRenderTarget::ATTACHMENT_COUNT] = {};
    uint32_t mWidth{};
    uint32_t mHeight{};
    uint8_t mSamples = 1;   // currently not settable in the public facing API
    uint8_t mLayerCount = 0;// currently not settable in the public facing API
};

using BuilderType = RenderTarget;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

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

RenderTarget* RenderTarget::Builder::build(Engine& engine) {
    using backend::TextureUsage;
    const FRenderTarget::Attachment& color = mImpl->mAttachments[(size_t)AttachmentPoint::COLOR0];
    const FRenderTarget::Attachment& depth = mImpl->mAttachments[(size_t)AttachmentPoint::DEPTH];

    if (color.texture) {
        FILAMENT_CHECK_PRECONDITION(color.texture->getUsage() & TextureUsage::COLOR_ATTACHMENT)
                << "Texture usage must contain COLOR_ATTACHMENT";
    }

    if (depth.texture) {
        FILAMENT_CHECK_PRECONDITION(depth.texture->getUsage() & TextureUsage::DEPTH_ATTACHMENT)
                << "Texture usage must contain DEPTH_ATTACHMENT";
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
    for (auto const& attachment : mImpl->mAttachments) {
        if (attachment.texture) {
            const uint32_t w = attachment.texture->getWidth(attachment.mipLevel);
            const uint32_t h = attachment.texture->getHeight(attachment.mipLevel);
            minWidth  = std::min(minWidth, w);
            minHeight = std::min(minHeight, h);
            maxWidth  = std::max(maxWidth, w);
            maxHeight = std::max(maxHeight, h);
        }
    }

    FILAMENT_CHECK_PRECONDITION(minWidth == maxWidth && minHeight == maxHeight)
            << "All attachments dimensions must match";

    mImpl->mWidth  = minWidth;
    mImpl->mHeight = minHeight;
    return downcast(engine).createRenderTarget(*this);
}

// ------------------------------------------------------------------------------------------------

FRenderTarget::FRenderTarget(FEngine& engine, const RenderTarget::Builder& builder)
    : mSupportedColorAttachmentsCount(engine.getDriverApi().getMaxDrawBuffers()) {

    std::copy(std::begin(builder.mImpl->mAttachments), std::end(builder.mImpl->mAttachments),
            std::begin(mAttachments));

    backend::MRT mrt{};
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
