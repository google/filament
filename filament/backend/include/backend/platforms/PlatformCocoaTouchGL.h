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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_COCOA_TOUCH_GL_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_COCOA_TOUCH_GL_H

#include <stdint.h>

#include <backend/platforms/OpenGLPlatform.h>

#include <backend/DriverEnums.h>

namespace filament::backend {

struct PlatformCocoaTouchGLImpl;

class PlatformCocoaTouchGL : public OpenGLPlatform {
public:
    PlatformCocoaTouchGL();
    ~PlatformCocoaTouchGL() noexcept;

    // --------------------------------------------------------------------------------------------
    // Platform Interface

    Driver* createDriver(void* sharedGLContext,
            const Platform::DriverConfig& driverConfig) noexcept override;

    int getOSVersion() const noexcept final { return 0; }

    // --------------------------------------------------------------------------------------------
    // OpenGLPlatform Interface

    void terminate() noexcept override;

    uint32_t createDefaultRenderTarget() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept override;
    void commit(SwapChain* swapChain) noexcept override;

    OpenGLPlatform::ExternalTexture* createExternalImageTexture() noexcept override;
    void destroyExternalImage(ExternalTexture* texture) noexcept override;
    void retainExternalImage(void* externalImage) noexcept override;
    bool setExternalImage(void* externalImage, ExternalTexture* texture) noexcept override;

private:
    PlatformCocoaTouchGLImpl* pImpl = nullptr;
};

using ContextManager = PlatformCocoaTouchGL;

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_COCOA_TOUCH_GL_H
