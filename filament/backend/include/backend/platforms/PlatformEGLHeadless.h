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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_HEADLESS_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_HEADLESS_H

#include "PlatformEGL.h"

namespace filament::backend {

/**
 * A concrete implementation of OpenGLPlatform that supports EGL with only headless swapchains.
 */
class PlatformEGLHeadless : public PlatformEGL {
public:
    PlatformEGLHeadless() noexcept;

    Driver* createDriver(void* sharedContext,
            const Platform::DriverConfig& driverConfig) noexcept override;

protected:
    bool isOpenGL() const noexcept override;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_HEADLESS_H
