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

#include "details/SwapChain.h"

#include "details/Engine.h"

#include <filament/SwapChain.h>

#include <backend/CallbackHandler.h>

#include <utils/Invocable.h>

#include <new>
#include <utility>

#include <stdint.h>

namespace filament {

FSwapChain::FSwapChain(FEngine& engine, void* nativeWindow, uint64_t flags)
        : mEngine(engine), mNativeWindow(nativeWindow), mConfigFlags(initFlags(engine, flags)) {
    mHwSwapChain = engine.getDriverApi().createSwapChain(nativeWindow, flags);
}

FSwapChain::FSwapChain(FEngine& engine, uint32_t width, uint32_t height, uint64_t flags)
        : mEngine(engine), mWidth(width), mHeight(height), mConfigFlags(initFlags(engine, flags)) {
    mHwSwapChain = engine.getDriverApi().createSwapChainHeadless(width, height, flags);
}

void FSwapChain::recreateWithNewFlags(FEngine& engine, uint64_t flags) noexcept {
    flags = initFlags(engine, flags);
    if (flags != mConfigFlags) {
        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.destroySwapChain(mHwSwapChain);
        mConfigFlags = flags;
        if (mNativeWindow) {
            mHwSwapChain = driver.createSwapChain(mNativeWindow, flags);
        } else {
            mHwSwapChain = driver.createSwapChainHeadless(mWidth, mHeight, flags);
        }
    }
}

uint64_t FSwapChain::initFlags(FEngine& engine, uint64_t flags) noexcept {
    if (!isSRGBSwapChainSupported(engine)) {
        flags &= ~CONFIG_SRGB_COLORSPACE;
    }
    if (!isProtectedContentSupported(engine)) {
        flags &= ~CONFIG_PROTECTED_CONTENT;
    }
    return flags;
}

void FSwapChain::terminate(FEngine& engine) noexcept {
    engine.getDriverApi().destroySwapChain(mHwSwapChain);
}

void FSwapChain::setFrameScheduledCallback(FrameScheduledCallback&& callback) {
    mFrameScheduledCallbackIsSet = bool(callback);
    mEngine.getDriverApi().setFrameScheduledCallback(mHwSwapChain, std::move(callback));
}

bool FSwapChain::isFrameScheduledCallbackSet() const noexcept {
    return mFrameScheduledCallbackIsSet;
}

void FSwapChain::setFrameCompletedCallback(
        backend::CallbackHandler* handler, FrameCompletedCallback&& callback) noexcept {
    using namespace std::placeholders;
    auto boundCallback = std::bind(std::move(callback), this);
    mEngine.getDriverApi().setFrameCompletedCallback(mHwSwapChain, handler, std::move(boundCallback));
}

bool FSwapChain::isSRGBSwapChainSupported(FEngine& engine) noexcept {
    return engine.getDriverApi().isSRGBSwapChainSupported();
}

bool FSwapChain::isProtectedContentSupported(FEngine& engine) noexcept {
    return engine.getDriverApi().isProtectedContentSupported();
}

} // namespace filament
