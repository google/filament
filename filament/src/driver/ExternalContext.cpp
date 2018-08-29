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

#include <filament/driver/ExternalContext.h>

#if defined(ANDROID)
    #ifndef USE_EXTERNAL_GLES3
        #include "driver/opengl/ContextManagerEGL.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "driver/vulkan/ContextManagerVkAndroid.h"
    #endif
#elif defined(__APPLE__)
    #ifndef USE_EXTERNAL_GLES3
        #include "driver/opengl/ContextManagerCocoa.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "driver/vulkan/ContextManagerVkCocoa.h"
    #endif
#elif defined(__linux__)
    #ifndef USE_EXTERNAL_GLES3
        #include "driver/opengl/ContextManagerGLX.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "driver/vulkan/ContextManagerVkLinux.h"
    #endif
#elif defined(WIN32)
    #ifndef USE_EXTERNAL_GLES3
        #include "driver/opengl/ContextManagerWGL.h"
    #endif
    #if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
        #include "driver/vulkan/ContextManagerVkWindows.h"
    #endif
#elif defined(__EMSCRIPTEN__)
    #include "driver/opengl/ContextManagerWebGL.h"
#else
    #ifndef USE_EXTERNAL_GLES3
        #include "driver/opengl/ContextManagerDummy.h"
    #endif
#endif

namespace filament {
namespace driver {

// this generates the vtable in this translation unit
ExternalContext::~ExternalContext() noexcept = default;

ContextManagerGL::~ContextManagerGL() noexcept = default;

ContextManagerVk::~ContextManagerVk() noexcept = default;

ExternalContext* ExternalContext::create(Backend* backend) noexcept {
    assert(backend);
    if (*backend == Backend::DEFAULT) {
        *backend = Backend::OPENGL;
    }
    if (*backend == Backend::VULKAN) {
        #if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
            #if defined(ANDROID)
                return new ContextManagerVkAndroid();
            #elif defined(__linux__)
                return new ContextManagerVkLinux();
            #elif defined(__APPLE__)
                return new ContextManagerVkCocoa();
            #elif defined(WIN32)
                return new ContextManagerVkWindows();
            #else
                return nullptr;
            #endif
        #else
            return nullptr;
        #endif
    }
    #if defined(USE_EXTERNAL_GLES3)
        return nullptr;
    #elif defined(ANDROID)
        return new ContextManagerEGL();
    #elif defined(__APPLE__)
        return new ContextManagerCocoa();
    #elif defined(__linux__)
        return new ContextManagerGLX();
    #elif defined(WIN32)
        return new ContextManagerWGL();
    #elif defined(__EMSCRIPTEN__)
        return new ContextManagerWebGL();
    #else
        return new ContextManagerDummy();
    #endif
    return nullptr;
}

} // namespace driver
} // namespace filament
