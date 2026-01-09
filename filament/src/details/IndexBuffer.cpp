/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/IndexBuffer.h"
#include "details/AsyncHelpers.h"
#include "details/Engine.h"

#include "FilamentAPI-impl.h"
#include "filament/FilamentAPI.h"

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>

#include <utility>

#include <stdint.h>
#include <stddef.h>

namespace filament {

struct IndexBuffer::BuilderDetails {
    uint32_t mIndexCount = 0;
    IndexType mIndexType = IndexType::UINT;
    bool mAsynchronous = false;
    backend::CallbackHandler* mAsyncCreationHandler = nullptr;
    AsyncCompletionCallback mAsyncCreationCallback;
    void* mAsyncCreationUserData = nullptr;
};

using BuilderType = IndexBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

IndexBuffer::Builder& IndexBuffer::Builder::indexCount(uint32_t const indexCount) noexcept {
    mImpl->mIndexCount = indexCount;
    return *this;
}

IndexBuffer::Builder& IndexBuffer::Builder::bufferType(IndexType const indexType) noexcept {
    mImpl->mIndexType = indexType;
    return *this;
}

IndexBuffer::Builder& IndexBuffer::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

IndexBuffer::Builder& IndexBuffer::Builder::name(utils::StaticString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

IndexBuffer::Builder& IndexBuffer::Builder::async(backend::CallbackHandler* handler,
        AsyncCompletionCallback callback, void* user) noexcept {
    mImpl->mAsynchronous = true;
    mImpl->mAsyncCreationHandler = handler;
    mImpl->mAsyncCreationCallback = std::move(callback);
    mImpl->mAsyncCreationUserData = user;
    return *this;
}

IndexBuffer* IndexBuffer::Builder::build(Engine& engine) {
    FILAMENT_CHECK_PRECONDITION(!mImpl->mAsynchronous || engine.isAsynchronousModeEnabled())
            << "Engine not configured for async operations";

    return downcast(engine).createIndexBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FIndexBuffer::FIndexBuffer(FEngine& engine, const Builder& builder)
        : mIndexCount(builder->mIndexCount) {
    auto name = builder.getName();
    const char* const tag = name.empty() ? "(no tag)" : name.c_str_safe();

    FILAMENT_CHECK_PRECONDITION(
            builder->mIndexType == IndexType::UINT || builder->mIndexType == IndexType::USHORT)
            << "Invalid index type " << static_cast<int>(builder->mIndexType) << ", tag=" << tag;

    FEngine::DriverApi& driver = engine.getDriverApi();

    if (builder->mAsynchronous) {
        // Copy the callback instead of moving it because moving would leave the builder with an
        // empty completion callback. If the builder were emptied, users would need to call `async`
        // again to reuse the builder for an additional new object.
        auto copiedCompletionCallback = builder->mAsyncCreationCallback;

        using IndexBufferCountdownCallbackHandler = CountdownCallbackHandler<IndexBuffer>;
        auto* cdHandler = IndexBufferCountdownCallbackHandler::make(
                /* handler */ builder->mAsyncCreationHandler,
                /* userCallback */ std::move(copiedCompletionCallback),
                /* userParam1 */ this,
                /* userParam2 */ builder->mAsyncCreationUserData,
                /* onCountdownComplete */ [this]() {
                    // `std::memory_order_relaxed` should be sufficient because no other variables
                    // need to be visible to other threads in a strict sequence.
                    mCreationComplete.store(true, std::memory_order_relaxed);
                },
                /* driver */ &engine.getDriver());

        mHandle = driver.createIndexBufferAsync(
                backend::ElementType(builder->mIndexType),
                uint32_t(builder->mIndexCount),
                backend::BufferUsage::STATIC,
                cdHandler, &IndexBufferCountdownCallbackHandler::countdownCallback, cdHandler,
                std::move(name));
        cdHandler->increaseCountdown();
    } else {
        mHandle = driver.createIndexBuffer(
                backend::ElementType(builder->mIndexType),
                uint32_t(builder->mIndexCount),
                backend::BufferUsage::STATIC,
                std::move(name));
        // In regular (non-asynchronous) mode, we know creation is complete as soon as all
        // creation-relevant API calls are recorded into the command stream, because subsequent API
        // calls will always be invoked after that (even including asynchronous version of APIs).
        mCreationComplete.store(true, std::memory_order_relaxed);
    }
}

void FIndexBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyIndexBuffer(mHandle);
}

void FIndexBuffer::setBuffer(FEngine& engine, BufferDescriptor&& buffer, uint32_t const byteOffset) {

    FILAMENT_CHECK_PRECONDITION((byteOffset & 0x3) == 0)
            << "byteOffset must be a multiple of 4";

    engine.getDriverApi().updateIndexBuffer(mHandle, std::move(buffer), byteOffset);
}

backend::AsyncCallId FIndexBuffer::setBufferAsync(FEngine& engine, BufferDescriptor&& buffer,
            uint32_t byteOffset, backend::CallbackHandler* handler,
            AsyncCompletionCallback callback, void* user) {

    FILAMENT_CHECK_PRECONDITION((byteOffset & 0x3) == 0)
            << "byteOffset must be a multiple of 4";

    using IndexBufferCallbackAdapter = CallbackAdapter<IndexBuffer>;
    auto* const cbWrapper = IndexBufferCallbackAdapter::make(std::move(callback), this, user);
    return engine.getDriverApi().updateIndexBufferAsync(mHandle, std::move(buffer), byteOffset,
            handler, &IndexBufferCallbackAdapter::func, cbWrapper);
}

} // namespace filament
