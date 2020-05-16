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

namespace filament {

using namespace backend;

struct RenderTarget::BuilderDetails {
    FRenderTarget::Attachment mAttachments[RenderTarget::ATTACHMENT_COUNT];
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
    const FRenderTarget::Attachment& color = mImpl->mAttachments[COLOR];
    const FRenderTarget::Attachment& depth = mImpl->mAttachments[DEPTH];
    const FTexture* cTexture = color.texture;
    const FTexture* dTexture = depth.texture;
    if (!ASSERT_PRECONDITION_NON_FATAL(cTexture, "color attachment not set")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(cTexture->getUsage() & TextureUsage::COLOR_ATTACHMENT,
            "Texture usage must contain COLOR_ATTACHMENT")) {
        return nullptr;
    }
    if (depth.texture) {
        if (!ASSERT_PRECONDITION_NON_FATAL(dTexture->getUsage() & TextureUsage::DEPTH_ATTACHMENT,
                "Texture usage must contain DEPTH_ATTACHMENT")) {
            return nullptr;
        }
        const uint32_t cWidth = FTexture::valueForLevel(color.mipLevel, cTexture->getWidth());
        const uint32_t cHeight = FTexture::valueForLevel(color.mipLevel, cTexture->getHeight());
        const uint32_t dWidth = FTexture::valueForLevel(depth.mipLevel, dTexture->getWidth());
        const uint32_t dHeight = FTexture::valueForLevel(depth.mipLevel, dTexture->getHeight());
        if (!ASSERT_PRECONDITION_NON_FATAL(cWidth == dWidth && cHeight == dHeight,
                "Attachment dimensions must match")) {
            return nullptr;
        }
    }
    return upcast(engine).createRenderTarget(*this);
}

// ------------------------------------------------------------------------------------------------

FRenderTarget::HwHandle FRenderTarget::createHandle(FEngine& engine, const Builder& builder) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    const Attachment& color = builder.mImpl->mAttachments[COLOR];
    const Attachment& depth = builder.mImpl->mAttachments[DEPTH];
    const TargetBufferFlags flags = depth.texture ?
            (TargetBufferFlags::COLOR0 | TargetBufferFlags::DEPTH) : TargetBufferFlags::COLOR0;

    // For now we do not support multisampled render targets in the public-facing API, but please
    // note that post-processing includes FXAA by default.
    const uint8_t samples = 1;

    const TargetBufferInfo cinfo(upcast(color.texture)->getHwHandle(),
            color.mipLevel, color.face);

    const TargetBufferInfo dinfo(
            depth.texture ? upcast(depth.texture)->getHwHandle() : TextureHandle(),
            color.mipLevel, color.face);

    const uint32_t width = FTexture::valueForLevel(color.mipLevel, color.texture->getWidth());
    const uint32_t height = FTexture::valueForLevel(color.mipLevel, color.texture->getHeight());

    return driver.createRenderTarget(flags, width, height, samples, cinfo, dinfo, {});
}

FRenderTarget::FRenderTarget(FEngine& engine, const RenderTarget::Builder& builder) :
    mHandle(createHandle(engine, builder)) {
    mAttachments[COLOR] = builder.mImpl->mAttachments[COLOR];
    mAttachments[DEPTH] = builder.mImpl->mAttachments[DEPTH];
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

} // namespace filament
