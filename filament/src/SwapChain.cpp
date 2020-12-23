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

namespace filament {

FSwapChain::FSwapChain(FEngine& engine, void* nativeWindow, uint64_t flags)
        : mEngine(engine), mNativeWindow(nativeWindow), mConfigFlags(flags) {
    mSwapChain = engine.getDriverApi().createSwapChain(nativeWindow, flags);
}

FSwapChain::FSwapChain(FEngine& engine, uint32_t width, uint32_t height, uint64_t flags)
        : mEngine(engine), mConfigFlags(flags) {
    mSwapChain = engine.getDriverApi().createSwapChainHeadless(width, height, flags);
}

void FSwapChain::terminate(FEngine& engine) noexcept {
    engine.getDriverApi().destroySwapChain(mSwapChain);
}

void FSwapChain::setFrameScheduledCallback(FrameScheduledCallback callback, void* user) {
    mEngine.getDriverApi().setFrameScheduledCallback(mSwapChain, callback, user);
}

void FSwapChain::setFrameCompletedCallback(FrameCompletedCallback callback, void* user) {
    mEngine.getDriverApi().setFrameCompletedCallback(mSwapChain, callback, user);
}

void* SwapChain::getNativeWindow() const noexcept {
    return upcast(this)->getNativeWindow();
}

void SwapChain::setFrameScheduledCallback(FrameScheduledCallback callback, void* user) {
    return upcast(this)->setFrameScheduledCallback(callback, user);
}

void SwapChain::setFrameCompletedCallback(FrameCompletedCallback callback, void* user) {
    return upcast(this)->setFrameCompletedCallback(callback, user);
}

} // namespace filament
