/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "PlatformWebGL.h"
#include "OpenGLDriverFactory.h"

namespace filament {

using namespace backend;

Driver* PlatformWebGL::createDriver(void* const sharedGLContext) noexcept {
    return OpenGLDriverFactory::create(this, sharedGLContext);
}

void PlatformWebGL::terminate() noexcept {
}

Platform::SwapChain* PlatformWebGL::createSwapChain(
        void* nativeWindow, uint64_t& flags) noexcept {
    return (SwapChain*)nativeWindow;
}

Platform::SwapChain* PlatformWebGL::createSwapChain(
        uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    // TODO: implement headless SwapChain
    return nullptr;
}

void PlatformWebGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
}

void PlatformWebGL::makeCurrent(Platform::SwapChain* drawSwapChain,
        Platform::SwapChain* readSwapChain) noexcept {
}

void PlatformWebGL::commit(Platform::SwapChain* swapChain) noexcept {
}

Platform::Fence* PlatformWebGL::createFence() noexcept {
    Fence* f = new Fence();
    return f;
}

void PlatformWebGL::destroyFence(Fence* fence) noexcept {
    delete fence;
}

backend::FenceStatus PlatformWebGL::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return backend::FenceStatus::CONDITION_SATISFIED;
}

} // namespace filament
