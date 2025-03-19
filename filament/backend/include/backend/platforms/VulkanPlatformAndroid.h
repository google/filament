/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H
#define TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H

#include <backend/Platform.h>

#include <android/hardware_buffer.h>

namespace filament::backend::fvkandroid {

struct ExternalImageVulkanAndroid : public Platform::ExternalImage {
    AHardwareBuffer* aHardwareBuffer = nullptr;
    bool sRGB = false;
    unsigned int width;   // Texture width
    unsigned int height;  // Texture height
    TextureFormat format; // Texture format
    TextureUsage usage;   // Texture usage flags

protected:
    ~ExternalImageVulkanAndroid() override;
};

Platform::ExternalImageHandle UTILS_PUBLIC createExternalImage(AHardwareBuffer const* buffer,
        bool sRGB) noexcept;

} // namespace filament::backend::fvkandroid

#endif // TNT_FILAMENT_BACKEND_PLATFORMS_VULKAN_PLATFORM_ANDROID_H
