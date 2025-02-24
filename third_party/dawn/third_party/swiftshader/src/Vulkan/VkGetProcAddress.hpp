// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VK_UTILS_HPP_
#define VK_UTILS_HPP_

#include "Vulkan/VulkanPlatform.hpp"

namespace vk {

class Device;
class Instance;

PFN_vkVoidFunction GetInstanceProcAddr(Instance *instance, const char *pName);
PFN_vkVoidFunction GetPhysicalDeviceProcAddr(Instance *instance, const char *pName);
PFN_vkVoidFunction GetDeviceProcAddr(Device *device, const char *pName);

}  // namespace vk

#if VK_USE_PLATFORM_FUCHSIA
// See vk_icdInitializeServiceConnectCallback() in libVulkan.cpp for details
// about this global pointer. Since this is a private implementation detail
// between the Vulkan loader and the ICDs, this type will never be part of
// the <vulkan/vulkan_fuchsia.h> headers, so define the type here.
typedef VkResult(VKAPI_PTR *PFN_vkConnectToService)(const char *pName, uint32_t handle);

namespace vk {
extern PFN_vkConnectToService icdFuchsiaServiceConnectCallback;
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#endif  // VK_UTILS_HPP_
