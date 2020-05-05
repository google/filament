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

#ifndef TNT_FILAMENT_DRIVER_VULKAN_PLATFORM_VK_LINUX_H
#define TNT_FILAMENT_DRIVER_VULKAN_PLATFORM_VK_LINUX_H

#include <stdint.h>

#include <backend/DriverEnums.h>
#include "VulkanPlatform.h"

#include <X11/Xlib.h>

namespace filament {

class PlatformVkLinux final : public backend::VulkanPlatform {
public:

    backend::Driver* createDriver(void* const sharedContext) noexcept override;

    void* createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept override;
    void getClientExtent(void* nativeWindow,  uint32_t* width, uint32_t* height) noexcept override;

    int getOSVersion() const noexcept override { return 0; }

private:
    Display* mDisplay;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_VULKAN_PLATFORM_VK_LINUX_H
