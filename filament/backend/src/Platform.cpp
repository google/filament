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

#include <backend/Platform.h>

#if defined(ANDROID)
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformEGLAndroid.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkAndroid.h"
    #endif
#elif defined(SWIFTSHADER)
    #include "opengl/PlatformEGL.h"
#elif defined(IOS)
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformCocoaTouchGL.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkCocoaTouch.h"
    #endif
#elif defined(__APPLE__)
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformCocoaGL.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkCocoa.h"
    #endif
#elif defined(__linux__)
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformGLX.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkLinux.h"
    #endif
#elif defined(WIN32)
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformWGL.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkWindows.h"
    #endif
#elif defined(__EMSCRIPTEN__)
    #include "opengl/PlatformWebGL.h"
#else
    #ifndef FILAMENT_USE_EXTERNAL_GLES3
        #include "opengl/PlatformDummyGL.h"
    #endif
#endif

#if defined (FILAMENT_SUPPORTS_METAL)
    #include "metal/PlatformMetal.h"
#endif

#include "noop/PlatformNoop.h"

namespace filament {
namespace backend {

// this generates the vtable in this translation unit
Platform::~Platform() noexcept = default;

// Creates the platform-specific Platform object. The caller takes ownership and is
// responsible for destroying it. Initialization of the backend API is deferred until
// createDriver(). The passed-in backend hint is replaced with the resolved backend.
DefaultPlatform* DefaultPlatform::create(Backend* backend) noexcept {
    assert(backend);
    if (*backend == Backend::DEFAULT) {
        *backend = Backend::OPENGL;
    }
    if (*backend == Backend::NOOP) {
        return new PlatformNoop();
    }
    if (*backend == Backend::VULKAN) {
        #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
            #if defined(ANDROID)
                return new PlatformVkAndroid();
            #elif defined(IOS)
                return new PlatformVkCocoaTouch();
            #elif defined(__linux__)
                return new PlatformVkLinux();
            #elif defined(__APPLE__)
                return new PlatformVkCocoa();
            #elif defined(WIN32)
                return new PlatformVkWindows();
            #else
                return nullptr;
            #endif
        #else
            return nullptr;
        #endif
    }
    if (*backend == Backend::METAL) {
#if defined(FILAMENT_SUPPORTS_METAL)
        return new PlatformMetal();
#else
        return nullptr;
#endif
    }
    #if defined(FILAMENT_USE_EXTERNAL_GLES3)
        return nullptr;
    #elif defined(SWIFTSHADER)
        return new PlatformEGL();
    #elif defined(ANDROID)
        return new PlatformEGLAndroid();
    #elif defined(IOS)
        return new PlatformCocoaTouchGL();
    #elif defined(__APPLE__)
        return new PlatformCocoaGL();
    #elif defined(__linux__)
        return new PlatformGLX();
    #elif defined(WIN32)
        return new PlatformWGL();
    #elif defined(__EMSCRIPTEN__)
        return new PlatformWebGL();
    #else
        return new PlatformDummyGL();
    #endif
}

// destroys an Platform create by create()
void DefaultPlatform::destroy(DefaultPlatform** platform) noexcept {
    delete *platform;
    *platform = nullptr;
}

DefaultPlatform::~DefaultPlatform() noexcept = default;

} // namespace backend
} // namespace filament
