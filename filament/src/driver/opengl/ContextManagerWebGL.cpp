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

#include "driver/opengl/ContextManagerWebGL.h"
#include "driver/opengl/OpenGLDriver.h"

namespace filament {

using namespace driver;

std::unique_ptr<Driver> ContextManagerWebGL::createDriver(void* const sharedGLContext) noexcept {
    return OpenGLDriver::create(this, sharedGLContext);
}

void ContextManagerWebGL::terminate() noexcept {
}

ExternalContext::SwapChain* ContextManagerWebGL::createSwapChain(
        void* nativeWindow, uint64_t& flags) noexcept {
    return (SwapChain*) nativeWindow;
}

void ContextManagerWebGL::destroySwapChain(ExternalContext::SwapChain* swapChain) noexcept {
}

void ContextManagerWebGL::makeCurrent(ExternalContext::SwapChain* swapChain) noexcept {
}

void ContextManagerWebGL::commit(ExternalContext::SwapChain* swapChain) noexcept {
}

ExternalContext::Fence* ContextManagerWebGL::createFence() noexcept {
    Fence* f = new Fence();
    return f;
}

void ContextManagerWebGL::destroyFence(Fence* fence) noexcept {
    delete fence;
}

driver::FenceStatus ContextManagerWebGL::waitFence(Fence* fence, uint64_t timeout) noexcept {
    return driver::FenceStatus::CONDITION_SATISFIED;
}

} // namespace filament
