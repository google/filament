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

#include "android/VirtualMachineEnv.h"

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <android/api-level.h>
#include <android/hardware_buffer.h>

#if __ANDROID_API__ >= 26
#   define PLATFORM_HAS_HARDWAREBUFFER
#endif

namespace filament {

class ExternalTextureManagerAndroid {
public:

    struct ExternalTexture : public backend::Platform::ExternalTexture {
        void* clientBuffer = nullptr;
        AHardwareBuffer* hardwareBuffer = nullptr;
    };

    static ExternalTextureManagerAndroid& get() noexcept;

    // called on gl thread
    ExternalTextureManagerAndroid() noexcept;

    // not quite sure on which thread this is going to be called
    ~ExternalTextureManagerAndroid() noexcept;

    // called on gl thread
    backend::Platform::ExternalTexture* create() noexcept;

    // called on app thread
    void reallocate(
        backend::Platform::ExternalTexture* ets, uint32_t w, uint32_t h,
        backend::TextureFormat format, uint64_t usage) noexcept;

    // called on gl thread
    void destroy(backend::Platform::ExternalTexture* ets) noexcept;

private:
    // called on app thread
    void alloc(backend::Platform::ExternalTexture* ets,
               uint32_t w, uint32_t h, backend::TextureFormat format, uint64_t usage) noexcept;

    // called on gl thread
    void destroyStorage(backend::Platform::ExternalTexture* ets) noexcept;

    VirtualMachineEnv& mVm;

#ifndef PLATFORM_HAS_HARDWAREBUFFER
    // if we compile for API 26 (Oreo) and above, we're guaranteed to have AHardwareBuffer
    // in all other cases, we need to get them at runtime.
    int (*AHardwareBuffer_allocate)(const AHardwareBuffer_Desc*, AHardwareBuffer**) = nullptr;
    void (*AHardwareBuffer_release)(AHardwareBuffer*) = nullptr;

    jclass mGraphicBufferClass = nullptr;
    jmethodID mGraphicBuffer_nCreateGraphicBuffer = nullptr;
    jmethodID mGraphicBuffer_nDestroyGraphicBuffer = nullptr;
#endif
};



} // namespace filament

#endif // TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_TEXTURE_MANAGER_ANDROID_H
