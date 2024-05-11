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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WEBGL_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WEBGL_H

#include <stdint.h>

#include <backend/platforms/OpenGLPlatform.h>

#include <backend/DriverEnums.h>

namespace filament::backend {

/**
 * A concrete implementation of OpenGLPlatform that supports WebGL.
 */
class PlatformWebGL : public OpenGLPlatform {
protected:
    // --------------------------------------------------------------------------------------------
    // Platform Interface

    Driver* createDriver(void* sharedGLContext,
            const Platform::DriverConfig& driverConfig) noexcept override;

    int getOSVersion() const noexcept override;

    // --------------------------------------------------------------------------------------------
    // OpenGLPlatform Interface

    void terminate() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    bool makeCurrent(ContextType type, SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept override;
    void commit(SwapChain* swapChain) noexcept override;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WEBGL_H
