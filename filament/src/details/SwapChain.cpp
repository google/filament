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

#include <utils/CString.h>
#include <utils/Invocable.h>
#include <utils/Logger.h>

#include <new>
#include <utility>

#include <stdint.h>

namespace filament {

namespace {

utils::CString getRemovedFlags(uint64_t originalFlags, uint64_t modifiedFlags) {
    utils::CString removed;
    if ((originalFlags & backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) &&
        !(modifiedFlags & backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE)) {
        removed += "SRGB_COLORSPACE ";
    }
    if ((originalFlags & backend::SWAP_CHAIN_CONFIG_PROTECTED_CONTENT) &&
        !(modifiedFlags & backend::SWAP_CHAIN_CONFIG_PROTECTED_CONTENT)) {
        removed += "PROTECTED_CONTENT ";
    }
    return removed;
}

} // anonymous namespace

FSwapChain::FSwapChain(FEngine& engine, void* nativeWindow, uint64_t const flags)
        : mEngine(engine), mNativeWindow(nativeWindow), mConfigFlags(initFlags(engine, flags)) {
    mHwSwapChain = engine.getDriverApi().createSwapChain(nativeWindow, mConfigFlags);
}

FSwapChain::FSwapChain(FEngine& engine, uint32_t const width, uint32_t const height, uint64_t const flags)
        : mEngine(engine), mWidth(width), mHeight(height), mConfigFlags(initFlags(engine, flags)) {
    mHwSwapChain = engine.getDriverApi().createSwapChainHeadless(width, height, mConfigFlags);
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
    const uint64_t originalFlags = flags;
    if (!isSRGBSwapChainSupported(engine)) {
        flags &= ~CONFIG_SRGB_COLORSPACE;
    }
    if (!isProtectedContentSupported(engine)) {
        flags &= ~CONFIG_PROTECTED_CONTENT;
    }
    if (originalFlags != flags) {
        LOG(WARNING) << "SwapChain flags were modified to remove features that are not supported. "
                     << "Removed: " << getRemovedFlags(originalFlags, flags).c_str_safe();
    }
    return flags;
}

void FSwapChain::terminate(FEngine& engine) noexcept {
    engine.getDriverApi().destroySwapChain(mHwSwapChain);
}

void FSwapChain::setFrameScheduledCallback(
        backend::CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t const flags) {
    mFrameScheduledCallbackIsSet = bool(callback);
    mEngine.getDriverApi().setFrameScheduledCallback(
            mHwSwapChain, handler, std::move(callback), flags);
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
