/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
#define TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H

#include <backend/Platform.h>

namespace filament::backend {

/**
 * A Platform interface that creates a Vulkan backend.
 */

class VulkanPlatform : public Platform {
public:
    // Given a Vulkan instance and native window handle, creates the platform-specific surface.
    virtual void* createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept = 0;

    // We use the extent of the surface to determine the size of the swap chain. For platforms
    // where vkGetPhysicalDeviceSurfaceCapabilitiesKHR returns undefined surface extent, we use
    // this fallback as the request size for the swap chain.
    // This can be a no-op for most platforms.
    virtual void getSwapChainFallbackExtent(void* nativeWindow, uint32_t* width, uint32_t* height)
            noexcept {}

   ~VulkanPlatform() override;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
