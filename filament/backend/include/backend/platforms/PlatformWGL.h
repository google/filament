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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WGL_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WGL_H

#include <stdint.h>

#include <windows.h>
#include "utils/unwindows.h"

#include <backend/platforms/OpenGLPlatform.h>
#include <backend/DriverEnums.h>

#include <vector>

namespace filament::backend {

/**
 * A concrete implementation of OpenGLPlatform that supports WGL.
 */
class PlatformWGL : public OpenGLPlatform {
protected:
    // --------------------------------------------------------------------------------------------
    // Platform Interface

    Driver* createDriver(void* sharedGLContext,
            const Platform::DriverConfig& driverConfig) override;

    int getOSVersion() const noexcept final override { return 0; }

    // --------------------------------------------------------------------------------------------
    // OpenGLPlatform Interface

    void terminate() noexcept override;

    bool isExtraContextSupported() const noexcept override;
    void createContext(bool shared) override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    bool makeCurrent(ContextType type, SwapChain* drawSwapChain, SwapChain* readSwapChain) override;
    void commit(SwapChain* swapChain) noexcept override;

protected:
    HGLRC mContext = NULL;
    HWND mHWnd = NULL;
    HDC mWhdc = NULL;
    PIXELFORMATDESCRIPTOR mPfd = {};
    std::vector<int> mAttribs;

    // For shared contexts
    static constexpr int SHARED_CONTEXT_NUM = 2;
    std::vector<HGLRC> mAdditionalContexts;
    std::atomic<int> mNextFreeSharedContextIndex{0};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_GLX_H
