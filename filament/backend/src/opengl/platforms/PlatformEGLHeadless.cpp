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

#include "opengl/GLUtils.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>

using namespace utils;

namespace filament {
using namespace backend;

PlatformEGLHeadless::PlatformEGLHeadless() noexcept
        : PlatformEGL() {
}

bool PlatformEGLHeadless::isOpenGL() const noexcept {
    return  true;
}

backend::Driver* PlatformEGLHeadless::createDriver(const void* sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    EGLBoolean bindAPI = eglBindAPI(EGL_OPENGL_API);
    if (UTILS_UNLIKELY(!bindAPI)) {
        slog.e << "eglBindAPI EGL_OPENGL_API failed" << io::endl;
        return nullptr;
    }
    int bindBlueGL = bluegl::bind();
    if (UTILS_UNLIKELY(bindBlueGL != 0)) {
        slog.e << "bluegl bind failed" << io::endl;
        return nullptr;
    }

    return PlatformEGL::createDriver(sharedContext, driverConfig);
}

} // namespace filament
