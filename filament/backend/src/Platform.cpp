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

#include <utils/Systrace.h>
#include <utils/debug.h>

#if defined(__ANDROID__)
    #include <sys/system_properties.h>
    #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3)
        #include "opengl/platforms/PlatformEGLAndroid.h"
    #endif
    #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkAndroid.h"
    #endif
#elif defined(IOS)
    #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3)
        #include "opengl/platforms/PlatformCocoaTouchGL.h"
    #endif
    #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkCocoaTouch.h"
    #endif
#elif defined(__APPLE__)
    #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3) && !defined(FILAMENT_USE_SWIFTSHADER)
        #include "opengl/platforms/PlatformCocoaGL.h"
    #endif
    #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkCocoa.h"
    #endif
#elif defined(__linux__)
    #if defined(FILAMENT_SUPPORTS_GGP)
        #include "vulkan/PlatformVkLinuxGGP.h"
    #elif defined(FILAMENT_SUPPORTS_WAYLAND)
        #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
            #include "vulkan/PlatformVkLinuxWayland.h"
        #endif
    #elif defined(FILAMENT_SUPPORTS_X11)
        #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3) && !defined(FILAMENT_USE_SWIFTSHADER)
            #include "opengl/platforms/PlatformGLX.h"
        #endif
        #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
            #include "vulkan/PlatformVkLinuxX11.h"
        #endif
    #elif defined(FILAMENT_SUPPORTS_EGL_ON_LINUX)
        #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3) && !defined(FILAMENT_USE_SWIFTSHADER)
            #include "opengl/platforms/PlatformEGLHeadless.h"
        #endif
    #endif
#elif defined(WIN32)
    #if defined(FILAMENT_SUPPORTS_OPENGL) && !defined(FILAMENT_USE_EXTERNAL_GLES3) && !defined(FILAMENT_USE_SWIFTSHADER)
        #include "opengl/platforms/PlatformWGL.h"
    #endif
    #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "vulkan/PlatformVkWindows.h"
    #endif
#elif defined(__EMSCRIPTEN__)
    #include "opengl/platforms/PlatformWebGL.h"
#endif

#if defined (FILAMENT_SUPPORTS_METAL)
namespace filament::backend {
filament::backend::DefaultPlatform* createDefaultMetalPlatform();
}
#endif

#include "noop/PlatformNoop.h"

namespace filament::backend {

// this generates the vtable in this translation unit
Platform::~Platform() noexcept = default;

// Creates the platform-specific Platform object. The caller takes ownership and is
// responsible for destroying it. Initialization of the backend API is deferred until
// createDriver(). The passed-in backend hint is replaced with the resolved backend.
DefaultPlatform* DefaultPlatform::create(Backend* backend) noexcept {
    SYSTRACE_CALL();
    assert_invariant(backend);

#if defined(__ANDROID__)
    char scratch[PROP_VALUE_MAX + 1];
    int length = __system_property_get("debug.filament.backend", scratch);
    if (length > 0) {
        *backend = Backend(atoi(scratch));
    }
#endif

    if (*backend == Backend::DEFAULT) {
#if defined(__EMSCRIPTEN__)
        *backend = Backend::OPENGL;
#elif defined(__ANDROID__)
        *backend = Backend::OPENGL;
#elif defined(IOS) || defined(__APPLE__)
        *backend = Backend::METAL;
#elif defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        *backend = Backend::VULKAN;
#else
        * backend = Backend::OPENGL;
#endif
    }
    if (*backend == Backend::NOOP) {
        return new PlatformNoop();
    }
    if (*backend == Backend::VULKAN) {
        #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
            #if defined(__ANDROID__)
                return new PlatformVkAndroid();
            #elif defined(IOS)
                return new PlatformVkCocoaTouch();
            #elif defined(__linux__)
                #if defined(FILAMENT_SUPPORTS_GGP)
                    return new PlatformVkLinuxGGP();
                #elif defined(FILAMENT_SUPPORTS_WAYLAND)
                    return new PlatformVkLinuxWayland();
                #elif defined(FILAMENT_SUPPORTS_X11)
                    return new PlatformVkLinuxX11();
                #endif
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
        return createDefaultMetalPlatform();
#else
        return nullptr;
#endif
    }
    assert_invariant(*backend == Backend::OPENGL);
    #if defined(FILAMENT_SUPPORTS_OPENGL)
        #if defined(FILAMENT_USE_EXTERNAL_GLES3) || defined(FILAMENT_USE_SWIFTSHADER)
            // Swiftshader OpenGLES support is deprecated and incomplete
            return nullptr;
        #elif defined(__ANDROID__)
            return new PlatformEGLAndroid();
        #elif defined(IOS)
            return new PlatformCocoaTouchGL();
        #elif defined(__APPLE__)
            return new PlatformCocoaGL();
        #elif defined(__linux__)
            #if defined(FILAMENT_SUPPORTS_X11)
                return new PlatformGLX();
            #elif defined(FILAMENT_SUPPORTS_EGL_ON_LINUX)
                return new PlatformEGLHeadless();
            #endif
        #elif defined(WIN32)
            return new PlatformWGL();
        #elif defined(__EMSCRIPTEN__)
            return new PlatformWebGL();
        #else
            return nullptr;
        #endif
    #else
        return nullptr;
    #endif
}

// destroys a Platform created by create()
void DefaultPlatform::destroy(DefaultPlatform** platform) noexcept {
    delete *platform;
    *platform = nullptr;
}

DefaultPlatform::~DefaultPlatform() noexcept = default;

} // namespace filament::backend
