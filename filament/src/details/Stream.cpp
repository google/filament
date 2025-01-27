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

#include <utils/CString.h>
#include <utils/Panic.h>
#include <filament/Stream.h>

namespace filament {

using namespace backend;

struct Stream::BuilderDetails {
    void* mStream = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
};

using BuilderType = Stream;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;


Stream::Builder& Stream::Builder::stream(void* stream) noexcept {
    mImpl->mStream = stream;
    return *this;
}

Stream::Builder& Stream::Builder::width(uint32_t const width) noexcept {
    mImpl->mWidth = width;
    return *this;
}

Stream::Builder& Stream::Builder::height(uint32_t const height) noexcept {
    mImpl->mHeight = height;
    return *this;
}

Stream::Builder& Stream::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

Stream* Stream::Builder::build(Engine& engine) {
    return downcast(engine).createStream(*this);
}

// ------------------------------------------------------------------------------------------------

FStream::FStream(FEngine& engine, const Builder& builder) noexcept
        : mEngine(engine),
          mStreamType(builder->mStream ? StreamType::NATIVE : StreamType::ACQUIRED),
          mNativeStream(builder->mStream),
          mWidth(builder->mWidth),
          mHeight(builder->mHeight) {

    if (mNativeStream) {
        // Note: this is a synchronous call. On Android, this calls back into Java.
        mStreamHandle = engine.getDriverApi().createStreamNative(mNativeStream);
    } else {
        mStreamHandle = engine.getDriverApi().createStreamAcquired();
    }

    if (auto name = builder.getName(); !name.empty()) {
        engine.getDriverApi().setDebugTag(mStreamHandle.getId(), std::move(name));
    }
}

void FStream::terminate(FEngine& engine) noexcept {
    engine.getDriverApi().destroyStream(mStreamHandle);
}

void FStream::setAcquiredImage(void* image,
        Callback const callback, void* userdata) noexcept {
    mEngine.getDriverApi().setAcquiredImage(mStreamHandle, image, nullptr, callback, userdata);
}

void FStream::setAcquiredImage(void* image,
        CallbackHandler* handler, Callback const callback, void* userdata) noexcept {
    mEngine.getDriverApi().setAcquiredImage(mStreamHandle, image, handler, callback, userdata);
}

void FStream::setDimensions(uint32_t const width, uint32_t const height) noexcept {
    mWidth = width;
    mHeight = height;

    // unfortunately, because this call is synchronous, we must make sure the handle has been
    // created first
    if (UTILS_UNLIKELY(!mStreamHandle)) {
        FFence::waitAndDestroy(mEngine.createFence(), Fence::Mode::FLUSH);
    }
    mEngine.getDriverApi().setStreamDimensions(mStreamHandle, mWidth, mHeight);
}

int64_t FStream::getTimestamp() const noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    return driver.getStreamTimestamp(mStreamHandle);
}

} // namespace filament
