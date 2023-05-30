/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_PLATFORM_VULKAN_H
#define TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_PLATFORM_VULKAN_H

#include <stdint.h>

#include <backend/DriverEnums.h>
#include <backend/platforms/VulkanPlatform.h>

namespace filament::backend {

// A concrete implementation of the VulkanPlatform
class PlatformVulkan final : public VulkanPlatform {
public:
    struct SurfaceBundle {
        void* surface;
        // On certain platforms, the extent of the surface cannot be queried from Vulkan. In those
        // situations, we allow the frontend to pass in the extent to use in creating the swap
        // chains. Platform implementation should set extent to 0 if they do not expect to set the
        // swap chain extent.
        uint32_t width;
        uint32_t height;
    };

    Driver* createDriver(void* const sharedContext,
            Platform::DriverConfig const& driverConfig) noexcept override;

    SurfaceBundle createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKAN_PLATFORM_PLATFORM_VULKAN_H
