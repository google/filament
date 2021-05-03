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
};

using BuilderType = RenderTarget;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

RenderTarget::Builder& RenderTarget::Builder::texture(AttachmentPoint pt, Texture* texture) noexcept {
    mImpl->mAttachments[pt].texture = upcast(texture);
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::mipLevel(AttachmentPoint pt, uint8_t level) noexcept {
    mImpl->mAttachments[pt].mipLevel = level;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::face(AttachmentPoint pt, CubemapFace face) noexcept {
    mImpl->mAttachments[pt].face = face;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::layer(AttachmentPoint pt, uint32_t layer) noexcept {
    mImpl->mAttachments[pt].layer = layer;
    return *this;
}

RenderTarget* RenderTarget::Builder::build(Engine& engine) {
    using backend::TextureUsage;
    const FRenderTarget::Attachment& color = mImpl->mAttachments[COLOR0];
    const FRenderTarget::Attachment& depth = mImpl->mAttachments[DEPTH];
    if (!ASSERT_PRECONDITION_NON_FATAL(color.texture, "COLOR0 attachment not set")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(color.texture->getUsage() & TextureUsage::COLOR_ATTACHMENT,
            "Texture usage must contain COLOR_ATTACHMENT")) {
        return nullptr;
    }
    if (depth.texture) {
        if (!ASSERT_PRECONDITION_NON_FATAL(depth.texture->getUsage() & TextureUsage::DEPTH_ATTACHMENT,
                "Texture usage must contain DEPTH_ATTACHMENT")) {
            return nullptr;
        }
    }

    const size_t maxDrawBuffers = upcast(engine).getDriverApi().getMaxDrawBuffers();
    for (size_t i = maxDrawBuffers; i < MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT; i++) {
        if (!ASSERT_PRECONDITION_NON_FATAL(!mImpl->mAttachments[i].texture,
                "Only %u color attachments are supported, but COLOR%u attachment is set",
                maxDrawBuffers, i)) {
            return nullptr;
        }
    }
    
    uint32_t minWidth = std::numeric_limits<uint32_t>::max();
    uint32_t maxWidth = 0;
    uint32_t minHeight = std::numeric_limits<uint32_t>::max();
    uint32_t maxHeight = 0;
    for (auto const& attachment : mImpl->mAttachments) {
        if (attachment.texture) {
            const uint32_t w = attachment.texture->getWidth(color.mipLevel);
            const uint32_t h = attachment.texture->getHeight(color.mipLevel);
            minWidth  = std::min(minWidth, w);
            minHeight = std::min(minHeight, h);
            maxWidth  = std::max(maxWidth, w);
            maxHeight = std::max(maxHeight, h);
        }
    }

    if (!ASSERT_PRECONDITION_NON_FATAL(minWidth == maxWidth && minHeight == maxHeight,
            "All attachments dimensions must match")) {
        return nullptr;
    }

    mImpl->mWidth  = minWidth;
    mImpl->mHeight = minHeight;
    return upcast(engine).createRenderTarget(*this);
}

// ------------------------------------------------------------------------------------------------

FRenderTarget::FRenderTarget(FEngine& engine, const RenderTarget::Builder& builder)
    : mSupportedColorAttachmentsCount(engine.getDriverApi().getMaxDrawBuffers()) {

    std::copy(std::begin(builder.mImpl->mAttachments), std::end(builder.mImpl->mAttachments),
            std::begin(mAttachments));

    backend::MRT mrt{};
    TargetBufferInfo dinfo{};

    auto setAttachment = [this](TargetBufferInfo& info, AttachmentPoint attachmentPoint) {
        Attachment const& attachment = mAttachments[attachmentPoint];
        auto t = upcast(attachment.texture);
        info.handle = t->getHwHandle();
        info.level  = attachment.mipLevel;
        if (t->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP) {
            info.face = attachment.face;
        } else {
            info.layer = attachment.layer;
        }
    };

    for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; i++) {
        if (mAttachments[i].texture) {
            mAttachmentMask |= getTargetBufferFlagsAt(i);
            setAttachment(mrt[i], (AttachmentPoint)i);
        }
    }
    if (mAttachments[DEPTH].texture) {
        mAttachmentMask |= TargetBufferFlags::DEPTH;
        setAttachment(dinfo, DEPTH);
    }

    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createRenderTarget(mAttachmentMask,
            builder.mImpl->mWidth, builder.mImpl->mHeight, builder.mImpl->mSamples, mrt, dinfo, {});
}

void FRenderTarget::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyRenderTarget(mHandle);
}

// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

Texture* RenderTarget::getTexture(AttachmentPoint attachment) const noexcept {
    return upcast(this)->getAttachment(attachment).texture;
}

uint8_t RenderTarget::getMipLevel(AttachmentPoint attachment) const noexcept {
    return upcast(this)->getAttachment(attachment).mipLevel;
}

RenderTarget::CubemapFace RenderTarget::getFace(AttachmentPoint attachment) const noexcept {
    return upcast(this)->getAttachment(attachment).face;
}

uint32_t RenderTarget::getLayer(AttachmentPoint attachment) const noexcept {
    return upcast(this)->getAttachment(attachment).layer;
}

uint8_t RenderTarget::getSupportedColorAttachmentsCount() const noexcept {
    return upcast(this)->getSupportedColorAttachmentsCount();
}

} // namespace filament
