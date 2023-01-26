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
    struct SurfaceBundle {
        void* surface;
        // On certain platforms, the extent of the surface cannot be queried from Vulkan. In those
        // situations, we allow the frontend to pass in the extent to use in creating the swap
        // chains.
        uint32_t width;
        uint32_t height;
    };

    // Given a Vulkan instance and native window handle, creates the platform-specific surface.
    virtual SurfaceBundle createVkSurfaceKHR(void* nativeWindow, void* instance,
        uint64_t flags) noexcept = 0;

   ~VulkanPlatform() override;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_PLATFORMS_VULKANPLATFORM_H
