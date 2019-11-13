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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H

#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <backend/DriverEnums.h>

#include "private/backend/OpenGLPlatform.h"

struct AHardwareBuffer;

namespace filament {

class VirtualMachineEnv;
class ExternalStreamManagerAndroid;
class ExternalTextureManagerAndroid;

class PlatformEGL final : public backend::OpenGLPlatform {
public:

    PlatformEGL() noexcept;

    backend::Driver* createDriver(void* sharedContext) noexcept override;
    void terminate() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept final;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept final;
    void destroySwapChain(SwapChain* swapChain) noexcept final;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept final;
    void commit(SwapChain* swapChain) noexcept final;

    void setPresentationTime(int64_t presentationTimeInNanosecond) noexcept final;

    bool canCreateFence() noexcept final { return true; }
    Fence* createFence() noexcept final;
    void destroyFence(Fence* fence) noexcept final;
    backend::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept final;

    Stream* createStream(void* nativeStream) noexcept final;
    void destroyStream(Stream* stream) noexcept final;
    void attach(Stream* stream, intptr_t tname) noexcept final;
    void detach(Stream* stream) noexcept final;
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept final;

    ExternalTexture* createExternalTextureStorage() noexcept final;
    void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, backend::TextureFormat format) noexcept final;
    void destroyExternalTextureStorage(ExternalTexture* ets) noexcept final;

    void createExternalImageTexture(void* texture) noexcept final;
    void destroyExternalImage(void* texture) noexcept final;

    backend::AcquiredImage transformAcquiredImage(backend::AcquiredImage source) noexcept final;

    int getOSVersion() const noexcept final;

private:
    EGLBoolean makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) noexcept;
    void initializeGlExtensions() noexcept;

    EGLDisplay mEGLDisplay = EGL_NO_DISPLAY;
    EGLContext mEGLContext = EGL_NO_CONTEXT;
    EGLSurface mCurrentDrawSurface = EGL_NO_SURFACE;
    EGLSurface mCurrentReadSurface = EGL_NO_SURFACE;
    EGLSurface mEGLDummySurface = EGL_NO_SURFACE;
    EGLConfig mEGLConfig;
    EGLConfig mEGLTransparentConfig;
    int mOSVersion;

    // supported extensions detected at runtime
    struct {
        bool OES_EGL_image_external_essl3 = false;
    } ext;

    ExternalStreamManagerAndroid& mExternalStreamManager;
    ExternalTextureManagerAndroid& mExternalTextureManager;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H
