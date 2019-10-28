/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "details/Stream.h"

#include "details/Engine.h"
#include "details/Fence.h"

#include "FilamentAPI-impl.h"

#include <backend/PixelBufferDescriptor.h>

#include <utils/Panic.h>
#include <filament/Stream.h>


namespace filament {

using namespace details;
using namespace backend;

struct Stream::BuilderDetails {
    void* mStream = nullptr;
    intptr_t mExternalTextureId = 0;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
};

using BuilderType = Stream;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


Stream::Builder& Stream::Builder::stream(void* stream) noexcept {
    mImpl->mStream = stream;
    return *this;
}

Stream::Builder& Stream::Builder::stream(intptr_t externalTextureId) noexcept {
    mImpl->mExternalTextureId = externalTextureId;
    return *this;
}

Stream::Builder& Stream::Builder::width(uint32_t width) noexcept {
    mImpl->mWidth = width;
    return *this;
}

Stream::Builder& Stream::Builder::height(uint32_t height) noexcept {
    mImpl->mHeight = height;
    return *this;
}

Stream* Stream::Builder::build(Engine& engine) {
    FEngine::assertValid(engine, __PRETTY_FUNCTION__);
    if (!ASSERT_PRECONDITION_NON_FATAL(!mImpl->mStream || !mImpl->mExternalTextureId,
            "One and only one of the stream or external texture can be specified")) {
        return nullptr;
    }

    return upcast(engine).createStream(*this);
}

// ------------------------------------------------------------------------------------------------

namespace details {

FStream::FStream(FEngine& engine, const Builder& builder) noexcept
        : mEngine(engine),
          mNativeStream(builder->mStream),
          mExternalTextureId(builder->mExternalTextureId),
          mWidth(builder->mWidth),
          mHeight(builder->mHeight) {

    if (mNativeStream) {
        // Note: this is a synchronous call. On Android, this calls back into Java.
        mStreamHandle = engine.getDriverApi().createStream(mNativeStream);
    } else if (mExternalTextureId) {
        mStreamHandle = engine.getDriverApi().createStreamFromTextureId(
                mExternalTextureId, mWidth, mHeight);
    }
}

void FStream::terminate(FEngine& engine) noexcept {
    engine.getDriverApi().destroyStream(mStreamHandle);
}

void FStream::setDimensions(uint32_t width, uint32_t height) noexcept {
    mWidth = width;
    mHeight = height;

    // unfortunately, because this call is synchronous, we must make sure the handle has been
    // created first
    if (UTILS_UNLIKELY(!mStreamHandle)) {
        FFence::waitAndDestroy(mEngine.createFence(FFence::Type::SOFT), Fence::Mode::FLUSH);
    }
    mEngine.getDriverApi().setStreamDimensions(mStreamHandle, mWidth, mHeight);
}

void FStream::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) noexcept {
    if (isExternalTextureId()) {
        // this works only on external texture id streams

        const size_t sizeNeeded = PixelBufferDescriptor::computeDataSize(
                buffer.format, buffer.type,
                buffer.stride ? buffer.stride : width,
                buffer.top + height,
                buffer.alignment);

        if (!ASSERT_POSTCONDITION_NON_FATAL(buffer.size >= sizeNeeded,
                "buffer.size too small %u bytes, needed %u bytes", buffer.size, sizeNeeded)) {
            return;
        }

        FEngine::DriverApi& driver = mEngine.getDriverApi();
        driver.readStreamPixels(mStreamHandle,
                xoffset, yoffset, width, height, std::move(buffer));
    }
}

int64_t FStream::getTimestamp() const noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    return driver.getStreamTimestamp(mStreamHandle);
}


} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

bool Stream::isNativeStream() const noexcept {
    return upcast(this)->isNativeStream();
}

void Stream::setDimensions(uint32_t width, uint32_t height) noexcept {
    upcast(this)->setDimensions(width, height);
}

void Stream::readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        backend::PixelBufferDescriptor&& buffer) noexcept {
    upcast(this)->readPixels(xoffset, yoffset, width, height, std::move(buffer));
}

int64_t Stream::getTimestamp() const noexcept {
    return upcast(this)->getTimestamp();
}

} // namespace filament
