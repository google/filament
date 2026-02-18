/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <backend/platforms/PlatformEGLHeadless.h>

#include <bluegl/BlueGL.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/compiler.h>

using namespace utils;

namespace filament {
using namespace backend;

PlatformEGLHeadless::PlatformEGLHeadless() noexcept
        : PlatformEGL() {
}

bool PlatformEGLHeadless::isOpenGL() const noexcept {
#if defined(BACKEND_OPENGL_VERSION_GL)
    return true;
#else
    return false;
#endif  // defined(BACKEND_OPENGL_VERSION_GL)
}

backend::Driver* PlatformEGLHeadless::createDriver(void* sharedContext,
        const Platform::DriverConfig& driverConfig) {
    auto bindApiHelper = [](EGLenum api, const char* errorString) -> bool {
        EGLBoolean bindAPI = eglBindAPI(api);
        if (UTILS_UNLIKELY(bindAPI == EGL_FALSE || bindAPI == EGL_BAD_PARAMETER)) {
            logEglError(errorString);
            eglReleaseThread();
            return false;
        };
        return true;
    };

    EGLenum api = isOpenGL() ? EGL_OPENGL_API : EGL_OPENGL_ES_API;
    const char* apiString = isOpenGL() ? "eglBindAPI EGL_OPENGL_API" : "eglBindAPI EGL_OPENGL_ES_API";
    if (!bindApiHelper(api, apiString)) {
        return nullptr;
    }

    int bindBlueGL = bluegl::bind();
    if (UTILS_UNLIKELY(bindBlueGL != 0)) {
        LOG(ERROR) << "bluegl bind failed";
        return nullptr;
    }

    return PlatformEGL::createDriverBase(sharedContext, driverConfig, true /* initFirstByQuery */);
}

} // namespace filament
