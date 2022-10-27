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

#include "vulkan/PlatformVkLinuxGGP.h"

#include <bluevk/BlueVK.h>
#if defined(FILAMENT_SUPPORTS_GGP)
#include <ggp_c/ggp.h>
#endif
#include <utils/Panic.h>

#include "VulkanDriverFactory.h"

using namespace bluevk;

namespace filament::backend {

Driver* PlatformVkLinuxGGP::createDriver(
    void* const sharedContext,
    const Platform::DriverConfig& driverConfig) noexcept {
#if defined(FILAMENT_SUPPORTS_GGP)
  ASSERT_PRECONDITION(sharedContext == nullptr,
                      "Vulkan does not support shared contexts.");
  const char* requiredInstanceExtensions[] = {
      VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME};
  return VulkanDriverFactory::create(this, requiredInstanceExtensions,
                                     sizeof(requiredInstanceExtensions) /
                                         sizeof(requiredInstanceExtensions[0]),
                                     driverConfig);
#else
  PANIC_PRECONDITION("Filament does not support GGP.");
  return nullptr;
#endif
}

void* PlatformVkLinuxGGP::createVkSurfaceKHR(void* nativeWindow, void* instance,
                                             uint64_t flags) noexcept {
#if defined(FILAMENT_SUPPORTS_GGP)
  VkStreamDescriptorSurfaceCreateInfoGGP surface_create_info = {
      VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP};
  surface_create_info.streamDescriptor = kGgpPrimaryStreamDescriptor;
  PFN_vkCreateStreamDescriptorSurfaceGGP fpCreateStreamDescriptorSurfaceGGP =
      reinterpret_cast<PFN_vkCreateStreamDescriptorSurfaceGGP>(
          vkGetInstanceProcAddr(static_cast<VkInstance>(instance),
                                "vkCreateStreamDescriptorSurfaceGGP"));
  ASSERT_PRECONDITION(fpCreateStreamDescriptorSurfaceGGP != nullptr,
                      "Error getting VkInstance "
                      "function vkCreateStreamDescriptorSurfaceGGP");
  VkSurfaceKHR surface = nullptr;
  VkResult res = fpCreateStreamDescriptorSurfaceGGP(
      static_cast<VkInstance>(instance), &surface_create_info, nullptr,
      &surface);
  ASSERT_PRECONDITION(res == VK_SUCCESS, "Error in vulkan: %d", res);
  return surface;
#else
  PANIC_PRECONDITION("Filament does not support GGP.");
  return nullptr;
#endif
}

}  // namespace filament::backend
