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

#ifndef TNT_FILAMENT_DRIVER_VULKANPLATFORM_H
#define TNT_FILAMENT_DRIVER_VULKANPLATFORM_H

#include <backend/Platform.h>

namespace filament {

namespace backend {
class Driver;
} // namespace backend

namespace backend {

class VulkanPlatform : public DefaultPlatform {
public:
    // Given a Vulkan instance and native window handle, creates the platform-specific surface.
    virtual void* createVkSurfaceKHR(void* nativeWindow, void* instance) noexcept = 0;
    virtual void getClientExtent(void* window,  uint32_t* width, uint32_t* height) noexcept = 0;

   ~VulkanPlatform() override;
};

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_VULKANPLATFORM_H
