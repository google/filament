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

#ifndef TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_TEXTURE_MANAGER_ANDROID_H
#define TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_TEXTURE_MANAGER_ANDROID_H

#include "private/backend/VirtualMachineEnv.h"

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <android/api-level.h>
#include <android/hardware_buffer.h>

namespace filament {

/*
 * ExternalTextureManagerAndroid::ExternalTexture is basically a wrapper for AHardwareBuffer.
 *
 * This class doesn't rely on GL or EGL, and could be used for other Android platform if needed
 * (e.g. Vulkan).
 *
 * ExternalTextureManagerAndroid handle allocation/destruction using either Java or the NDK,
 * whichever is available.
 */
class ExternalTextureManagerAndroid {
public:

    struct ExternalTexture : public backend::Platform::ExternalTexture {
        void* clientBuffer = nullptr;
        AHardwareBuffer* hardwareBuffer = nullptr;
    };

    // must be called on backend thread
    static ExternalTextureManagerAndroid& create() noexcept;

    // must be called on backend thread
    static void destroy(ExternalTextureManagerAndroid* pExternalTextureManager) noexcept;

    // must be called on backend thread (only because we don't synchronize
    backend::Platform::ExternalTexture* createExternalTexture() noexcept;

    // called on app thread
    void reallocate(
        backend::Platform::ExternalTexture* ets, uint32_t w, uint32_t h,
        backend::TextureFormat format, uint64_t usage) noexcept;

    // must be called on backend thread
    void destroy(backend::Platform::ExternalTexture* ets) noexcept;

private:
    ExternalTextureManagerAndroid() noexcept;
    ~ExternalTextureManagerAndroid() noexcept;

    // called on app thread
    void alloc(backend::Platform::ExternalTexture* ets,
               uint32_t w, uint32_t h, backend::TextureFormat format, uint64_t usage) noexcept;

    // called on gl thread
    void destroyStorage(backend::Platform::ExternalTexture* ets) noexcept;

    VirtualMachineEnv& mVm;

    jclass mGraphicBufferClass = nullptr;
    jmethodID mGraphicBuffer_nCreateGraphicBuffer = nullptr;
    jmethodID mGraphicBuffer_nDestroyGraphicBuffer = nullptr;
};



} // namespace filament

#endif // TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_TEXTURE_MANAGER_ANDROID_H
