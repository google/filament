/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <backend/platforms/PlatformOSMesa.h>

#include <utils/Log.h>
#include <utils/Panic.h>

#include <memory>

namespace filament::backend {

using namespace backend;

namespace {

using BackingType = GLfloat;
#define BACKING_GL_TYPE GL_FLOAT

struct OSMesaSwapchain {
    OSMesaSwapchain(uint32_t width, uint32_t height)
        : width(width),
          height(height),
          buffer(new uint8_t[width * height * 4 * sizeof(BackingType)]) {}

    uint32_t width = 0;
    uint32_t height = 0;
    std::unique_ptr<uint8_t[]> buffer;
};

} // anonymous namespace

Driver* PlatformOSMesa::createDriver(void* const sharedGLContext,
        const DriverConfig& driverConfig) noexcept {
    FILAMENT_CHECK_PRECONDITION(sharedGLContext == nullptr)
            << "shared GL context is not supported with PlatformOSMesa";
    mContext = OSMesaCreateContext(GL_RGBA, NULL);

    // We need to do a no-op makecurrent here so that the context will be in a correct state before
    // any GL calls.
    auto chain = createSwapChain(1, 1, 0);
    makeCurrent(ContextType::UNPROTECTED, chain, nullptr);
    destroySwapChain(chain);

    int result = bluegl::bind();
    FILAMENT_CHECK_POSTCONDITION(!result) << "Unable to load OpenGL entry points.";

    return OpenGLPlatform::createDefaultDriver(this, sharedGLContext, driverConfig);
}

void PlatformOSMesa::terminate() noexcept {
    OSMesaDestroyContext(mContext);
    bluegl::unbind();
}

Platform::SwapChain* PlatformOSMesa::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    FILAMENT_CHECK_POSTCONDITION(false) << "Cannot create non-headless swapchain";
    return (SwapChain*) nativeWindow;
}

Platform::SwapChain* PlatformOSMesa::createSwapChain(uint32_t width, uint32_t height,
        uint64_t flags) noexcept {
    OSMesaSwapchain* swapchain = new OSMesaSwapchain(width, height);
    return (SwapChain*) swapchain;
}

void PlatformOSMesa::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    OSMesaSwapchain* impl = (OSMesaSwapchain*) swapChain;
    delete impl;
}

bool PlatformOSMesa::makeCurrent(ContextType type, SwapChain* drawSwapChain,
        SwapChain* readSwapChain) noexcept {
    OSMesaSwapchain* impl = (OSMesaSwapchain*) drawSwapChain;

    auto result = OSMesaMakeCurrent(mContext, (BackingType*) impl->buffer.get(), BACKING_GL_TYPE,
            impl->width, impl->height);
    FILAMENT_CHECK_POSTCONDITION(result) << "OSMesaMakeCurrent failed!";

    return true;
}

void PlatformOSMesa::commit(Platform::SwapChain* swapChain) noexcept {
    // No-op since we are not scanning out to a display.
}

} // namespace filament::backend
