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

using namespace details;

struct RenderTarget::BuilderDetails {
    Texture* mColorTexture = nullptr;
    Texture* mDepthTexture = nullptr;
    CubemapFace mCubemapFace = CubemapFace::POSITIVE_X;
    uint8_t mMiplevel = 0;
};

using BuilderType = RenderTarget::Builder;
BuilderType::Builder() noexcept = default;
BuilderType::~Builder() noexcept = default;
BuilderType::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder(Builder&& rhs) noexcept = default;
BuilderType& BuilderType::operator=(BuilderType const& rhs) noexcept = default;
BuilderType& BuilderType::operator=(BuilderType&& rhs) noexcept = default;

RenderTarget::Builder& RenderTarget::Builder::color(Texture* texture) noexcept {
    mImpl->mColorTexture = texture;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::depth(Texture* texture) noexcept {
    mImpl->mDepthTexture = texture;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::miplevel(uint8_t level) noexcept {
    mImpl->mMiplevel = level;
    return *this;
}

RenderTarget::Builder& RenderTarget::Builder::face(CubemapFace face) noexcept {
    mImpl->mCubemapFace = face;
    return *this;
}

RenderTarget* RenderTarget::Builder::build(Engine& engine) {
    using backend::TextureUsage;
    FTexture* color = upcast(mImpl->mColorTexture);
    FTexture* depth = upcast(mImpl->mDepthTexture);

    if (!ASSERT_PRECONDITION_NON_FATAL(color, "color attachment not set")) {
        return nullptr;
    }
    if (!ASSERT_PRECONDITION_NON_FATAL(color->getUsage() & TextureUsage::COLOR_ATTACHMENT,
            "Texture usage must contain COLOR_ATTACHMENT")) {
        return nullptr;
    }
    if (depth && !ASSERT_PRECONDITION_NON_FATAL(depth->getUsage() & TextureUsage::DEPTH_ATTACHMENT,
            "Texture usage must contain DEPTH_ATTACHMENT")) {
        return nullptr;
    }

    return upcast(engine).createRenderTarget(*this);
}

// ------------------------------------------------------------------------------------------------

namespace details {

FRenderTarget::HwHandle FRenderTarget::createHandle(FEngine& engine, const Builder& builder) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    const Texture* depth = builder.mImpl->mDepthTexture;
    const Texture* color = builder.mImpl->mColorTexture;
    const uint8_t level = builder.mImpl->mMiplevel;
    const CubemapFace face = builder.mImpl->mCubemapFace;
    const backend::TargetBufferFlags flags = depth ? backend::COLOR_AND_DEPTH : backend::COLOR;
    const uint32_t width = FTexture::valueForLevel(level, color->getWidth());
    const uint32_t height = FTexture::valueForLevel(level, color->getHeight());

    // For now we do not support multisampled render targets in the public-facing API, but please
    // note that post-processing includes FXAA by default.
    const uint8_t samples = 1;

    const backend::TargetBufferInfo cinfo(upcast(color)->getHwHandle(), level, face);
    const backend::TargetBufferInfo dinfo(
            depth ? upcast(depth)->getHwHandle() : backend::TextureHandle(),
            level, face);
    return driver.createRenderTarget(flags, width, height, samples, cinfo, dinfo, {});
}

FRenderTarget::FRenderTarget(FEngine& engine, const RenderTarget::Builder& builder)
        : mColorTexture(upcast(builder->mColorTexture)),
          mDepthTexture(upcast(builder->mDepthTexture)),
          mMiplevel(builder->mMiplevel),
          mCubemapFace(builder->mCubemapFace),
          mHandle(createHandle(engine, builder)) {
}

void FRenderTarget::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyRenderTarget(mHandle);
}

} // namespace details

// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

Texture* RenderTarget::getColor() const noexcept {
    return upcast(this)->getColor();
}

Texture* RenderTarget::getDepth() const noexcept {
    return upcast(this)->getDepth();
}

uint8_t RenderTarget::getMiplevel() const noexcept {
    return upcast(this)->getMiplevel();
}

RenderTarget::CubemapFace RenderTarget::getFace() const noexcept {
    return upcast(this)->getFace();
}

} // namespace filament
