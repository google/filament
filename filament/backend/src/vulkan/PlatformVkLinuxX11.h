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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_VK_LINUX_X11_H
#define TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_VK_LINUX_X11_H

#include <stdint.h>

#include <backend/DriverEnums.h>
#include "VulkanPlatform.h"

#ifdef FILAMENT_SUPPORTS_XCB
#include <xcb/xcb.h>
#endif

#ifdef FILAMENT_SUPPORTS_XLIB
#include <X11/Xlib.h>
#endif

namespace filament::backend {

class PlatformVkLinuxX11 final : public VulkanPlatform {
public:

    Driver* createDriver(void* const sharedContext, const Platform::DriverConfig& driverConfig) noexcept override;

    void* createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept override;

    int getOSVersion() const noexcept override { return 0; }

private:
#ifdef FILAMENT_SUPPORTS_XCB
    xcb_connection_t* mConnection;
#endif
#ifdef FILAMENT_SUPPORTS_XLIB
    Display* mDisplay;
#endif
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_VK_LINUX_X11_H
