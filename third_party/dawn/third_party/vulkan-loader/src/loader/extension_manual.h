/*
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Mark Young <marky@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include <vulkan/vulkan.h>

// ---- Manually added trampoline/terminator functions

// These functions, for whatever reason, require more complex changes than
// can easily be automatically generated.

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        VkSurfaceCapabilities2EXT* pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   VkSurfaceCapabilities2EXT* pSurfaceCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

VKAPI_ATTR VkResult VKAPI_CALL terminator_ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
VKAPI_ATTR VkResult VKAPI_CALL AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display);

VKAPI_ATTR VkResult VKAPI_CALL terminator_AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy,
                                                                VkDisplayKHR display);

VKAPI_ATTR VkResult VKAPI_CALL GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                        VkDisplayKHR* pDisplay);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                                   VkDisplayKHR* pDisplay);
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                        uint32_t* pPresentModeCount,
                                                                        VkPresentModeKHR* pPresentModes);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceSurfacePresentModes2EXT(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes);
#endif  // VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                     const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                     VkDeviceGroupPresentModeFlagsKHR* pModes);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                                VkDeviceGroupPresentModeFlagsKHR* pModes);

// ---- VK_EXT_tooling_info extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                  VkPhysicalDeviceToolPropertiesEXT* pToolProperties);

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                             VkPhysicalDeviceToolPropertiesEXT* pToolProperties);
