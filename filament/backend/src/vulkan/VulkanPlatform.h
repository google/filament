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

// In debug builds, we enable validation layers and set up a debug callback if the extension is
// available. Caution: the debug callback causes a null pointer dereference with optimized builds.
//
// To enable validation layers in Android, also be sure to set the jniLibs property in the gradle
// file for your app by adding the following lines into the "android" section. This copies the
// appropriate libraries from the NDK to the device. This makes the aar much larger, so it should be
// avoided in release builds.
//
// sourceSets.main.jniLibs {
//   srcDirs = ["${android.ndkDirectory}/sources/third_party/vulkan/src/build-android/jniLibs"]
// }
//
#if defined(NDEBUG)
#define VK_ENABLE_VALIDATION 0
#else
#define VK_ENABLE_VALIDATION 1
#endif

namespace filament {
namespace backend {

class VulkanPlatform : public DefaultPlatform {
public:
    // Given a Vulkan instance and native window handle, creates the platform-specific surface.
    virtual void* createVkSurfaceKHR(void* nativeWindow, void* instance, uint64_t flags) noexcept = 0;

   ~VulkanPlatform() override;
};

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_VULKANPLATFORM_H
