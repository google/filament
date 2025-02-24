/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
 */

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "generated/dispatch_functions.h"

namespace stateless {
bool Device::manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                       VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                       const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (semaphore == VK_NULL_HANDLE && fence == VK_NULL_HANDLE) {
        skip |= LogError("VUID-vkAcquireNextImageKHR-semaphore-01780", swapchain, error_obj.location,
                         "semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool Device::manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                        uint32_t *pImageIndex, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pAcquireInfo->semaphore == VK_NULL_HANDLE && pAcquireInfo->fence == VK_NULL_HANDLE) {
        skip |= LogError("VUID-VkAcquireNextImageInfoKHR-semaphore-01782", pAcquireInfo->swapchain,
                         error_obj.location.dot(Field::pAcquireInfo), "semaphore and fence are both VK_NULL_HANDLE.");
    }

    return skip;
}

bool Device::ValidateSwapchainCreateInfoMaintenance1(const VkSwapchainCreateInfoKHR &create_info, const Location &loc) const {
    bool skip = false;

    if (enabled_features.swapchainMaintenance1) {
        return skip;
    }

    if (vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(create_info.pNext)) {
        skip |= LogError("VUID-VkSwapchainCreateInfoKHR-swapchainMaintenance1-10155", device, loc.dot(Field::pNext),
                         "contains VkSwapchainPresentModesCreateInfoEXT, but swapchainMaintenance1 is not enabled");
    }

    if (const auto *present_scaling_create_info =
            vku::FindStructInPNextChain<VkSwapchainPresentScalingCreateInfoEXT>(create_info.pNext)) {
        if (present_scaling_create_info->scalingBehavior != 0) {
            skip |= LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-swapchainMaintenance1-10154", device,
                             loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::scalingBehavior),
                             " is %s, but swapchainMaintenance1 is not enabled",
                             string_VkPresentScalingFlagsEXT(present_scaling_create_info->scalingBehavior).c_str());
        } else if (present_scaling_create_info->presentGravityX != 0) {
            skip |= LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-swapchainMaintenance1-10154", device,
                             loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityX),
                             " is %s, but swapchainMaintenance1 is not enabled",
                             string_VkPresentGravityFlagsEXT(present_scaling_create_info->presentGravityX).c_str());
        } else if (present_scaling_create_info->presentGravityY != 0) {
            skip |= LogError("VUID-VkSwapchainPresentScalingCreateInfoEXT-swapchainMaintenance1-10154", device,
                             loc.pNext(Struct::VkSwapchainPresentScalingCreateInfoEXT, Field::presentGravityY),
                             " is %s, but swapchainMaintenance1 is not enabled",
                             string_VkPresentGravityFlagsEXT(present_scaling_create_info->presentGravityY).c_str());
        }
    }

    if (create_info.flags & VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT) {
        skip |= LogError("VUID-VkSwapchainCreateInfoKHR-swapchainMaintenance1-10157", device, loc.dot(Field::flags),
                         "is %s, but swapchainMaintenance1 is not enabled",
                         string_VkSwapchainCreateFlagsKHR(create_info.flags).c_str());
    }

    return skip;
}

bool Device::ValidateSwapchainCreateInfo(const Context &context, const VkSwapchainCreateInfoKHR &create_info,
                                         const Location &loc) const {
    bool skip = false;

    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (create_info.imageSharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        if (create_info.queueFamilyIndexCount <= 1) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01278", device, loc.dot(Field::imageSharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                             create_info.queueFamilyIndexCount);
        }

        // If imageSharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (create_info.pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-imageSharingMode-01277", device, loc.dot(Field::imageSharingMode),
                             "is VK_SHARING_MODE_CONCURRENT, but pQueueFamilyIndices is NULL.");
        }
    }

    skip |= context.ValidateNotZero(create_info.imageArrayLayers == 0, "VUID-VkSwapchainCreateInfoKHR-imageArrayLayers-01275",
                                    loc.dot(Field::imageArrayLayers));

    // Validate VK_KHR_image_format_list VkImageFormatListCreateInfo
    const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(create_info.pNext);
    if (format_list_info) {
        const uint32_t view_format_count = format_list_info->viewFormatCount;
        if (((create_info.flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) == 0) && (view_format_count > 1)) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-04100", device,
                             loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount),
                             "is %" PRIu32 " but flag (%s) does not includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR.",
                             view_format_count, string_VkImageCreateFlags(create_info.flags).c_str());
        }

        // Using the first format, compare the rest of the formats against it that they are compatible
        for (uint32_t i = 1; i < view_format_count; i++) {
            if (vkuFormatCompatibilityClass(format_list_info->pViewFormats[0]) !=
                vkuFormatCompatibilityClass(format_list_info->pViewFormats[i])) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-pNext-04099", device,
                                 loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats, i),
                                 "(%s) and pViewFormats[0] (%s) are not compatible in the pNext chain.",
                                 string_VkFormat(format_list_info->pViewFormats[i]),
                                 string_VkFormat(format_list_info->pViewFormats[0]));
            }
        }
    }

    // Validate VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR
    if ((create_info.flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) != 0) {
        if (format_list_info == nullptr) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                             "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but the pNext chain does not contain an instance "
                             "of VkImageFormatListCreateInfo.");
        } else if (format_list_info->viewFormatCount == 0) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                             "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but %s is zero.",
                             loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount).Fields().c_str());
        } else {
            bool found_base_format = false;
            for (uint32_t i = 0; i < format_list_info->viewFormatCount; ++i) {
                if (format_list_info->pViewFormats[i] == create_info.imageFormat) {
                    found_base_format = true;
                    break;
                }
            }
            if (!found_base_format) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03168", device, loc.dot(Field::flags),
                                 "includes VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR but none of the "
                                 "elements of the pViewFormats member of VkImageFormatListCreateInfo match "
                                 "imageFormat (%s).",
                                 string_VkFormat(create_info.imageFormat));
            }
        }
    }

    if (create_info.presentMode == VK_PRESENT_MODE_FIFO_LATEST_READY_EXT && !enabled_features.presentModeFifoLatestReady) {
        skip |=
            LogError("VUID-VkSwapchainCreateInfoKHR-presentModeFifoLatestReady-10161", device, loc.dot(Field::presentMode),
                     "is %s, but feature presentModeFifoLatestReady is not enabled", string_VkPresentModeKHR(create_info.presentMode));
    }

    if (create_info.flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) {
        auto stateless_instance = static_cast<Instance *>(dispatch_instance_->GetValidationObject(container_type));
        const auto &physdev_extensions = stateless_instance->physical_device_extensions.at(physical_device);
        const bool is_required_ext_supported = IsExtEnabled(physdev_extensions.vk_khr_surface_protected_capabilities);
        const bool is_required_ext_enabled = IsExtEnabled(extensions.vk_khr_surface_protected_capabilities);

        if (is_required_ext_supported && !is_required_ext_enabled) {
            skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03187", device, loc.dot(Field::flags),
                             "contains VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR, but extension VK_KHR_surface_protected_capabilities "
                             "was not enabled and surface capabilities VkSurfaceProtectedCapabilitiesKHR were not queried.");
        } else if (is_required_ext_enabled) {
            VkPhysicalDeviceSurfaceInfo2KHR surface_info = vku::InitStructHelper();
            surface_info.surface = create_info.surface;
            VkSurfaceProtectedCapabilitiesKHR surface_protected_capabilities = vku::InitStructHelper();
            VkSurfaceCapabilities2KHR surface_capabilities = vku::InitStructHelper(&surface_protected_capabilities);
            const VkResult result =
                DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info, &surface_capabilities);

            if (result == VK_SUCCESS && !surface_protected_capabilities.supportsProtected) {
                skip |= LogError("VUID-VkSwapchainCreateInfoKHR-flags-03187", device, loc.dot(Field::flags),
                                 "contains VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR but the surface "
                                 "capabilities does not have VkSurfaceProtectedCapabilitiesKHR.supportsProtected set to VK_TRUE.");
            }
        }
    }

    skip |= ValidateSwapchainCreateInfoMaintenance1(create_info, loc);

    return skip;
}

bool Device::manual_PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT *pReleaseInfo,
                                                             const Context &context) const {
    bool skip = false;

    if (!enabled_features.swapchainMaintenance1) {
        skip |= LogError("VUID-vkReleaseSwapchainImagesEXT-swapchainMaintenance1-10159", device, context.error_obj.location,
                         "swapchainMaintenance1 is not enabled");
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                      const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |= ValidateSwapchainCreateInfo(context, *pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool Device::manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            skip |= ValidateSwapchainCreateInfo(context, pCreateInfos[i], error_obj.location.dot(Field::pCreateInfos, i));
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!pPresentInfo) return skip;

    if (const auto *present_regions = vku::FindStructInPNextChain<VkPresentRegionsKHR>(pPresentInfo->pNext)) {
        if (present_regions->swapchainCount != pPresentInfo->swapchainCount) {
            skip |= LogError("VUID-VkPresentRegionsKHR-swapchainCount-01260", device,
                             error_obj.location.pNext(Struct::VkPresentRegionsKHR, Field::swapchainCount),
                             "(%" PRIu32 ") is not equal to %s (%" PRIu32 ").", present_regions->swapchainCount,
                             error_obj.location.dot(Field::pPresentInfo).dot(Field::swapchainCount).Fields().c_str(),
                             pPresentInfo->swapchainCount);
        }
    }

    if (vku::FindStructInPNextChain<VkSwapchainPresentFenceInfoEXT>(pPresentInfo->pNext) &&
        !enabled_features.swapchainMaintenance1) {
        skip |= LogError("VUID-VkPresentInfoKHR-swapchainMaintenance1-10158", device, error_obj.location.dot(Field::pNext),
                         "contains VkSwapchainPresentFenceInfoEXT, but swapchainMaintenance1 is not enabled");
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        for (uint32_t j = i + 1; j < pPresentInfo->swapchainCount; ++j) {
            if (pPresentInfo->pSwapchains[i] == pPresentInfo->pSwapchains[j]) {
                skip |= LogError("VUID-VkPresentInfoKHR-pSwapchain-09231", device, error_obj.location.dot(Field::pSwapchain),
                                 "[%" PRIu32 "] and pSwapchain[%" PRIu32 "] are both %s.", i, j,
                                 FormatHandle(pPresentInfo->pSwapchains[i]).c_str());
            }
        }
    }

    return skip;
}

bool Instance::manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                          const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const VkDisplayModeParametersKHR display_mode_parameters = pCreateInfo->parameters;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location param_loc = create_info_loc.dot(Field::parameters);

    skip |= context.ValidateNotZero(display_mode_parameters.visibleRegion.width == 0, "VUID-VkDisplayModeParametersKHR-width-01990",
                                    param_loc.dot(Field::visibleRegion).dot(Field::width));
    skip |=
        context.ValidateNotZero(display_mode_parameters.visibleRegion.height == 0, "VUID-VkDisplayModeParametersKHR-height-01991",
                                param_loc.dot(Field::visibleRegion).dot(Field::width));
    skip |= context.ValidateNotZero(display_mode_parameters.refreshRate == 0, "VUID-VkDisplayModeParametersKHR-refreshRate-01992",
                                    param_loc.dot(Field::refreshRate));

    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                        uint32_t *pSurfaceFormatCount,
                                                                        VkSurfaceFormatKHR *pSurfaceFormats,
                                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
        skip |=
            LogError("VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-surface-06524", physicalDevice,
                     error_obj.location.dot(Field::surface), "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                             uint32_t *pPresentModeCount,
                                                                             VkPresentModeKHR *pPresentModes,
                                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
        skip |=
            LogError("VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-surface-06524", physicalDevice,
                     error_obj.location.dot(Field::surface), "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                              VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    const auto *capabilities_full_screen_exclusive =
        vku::FindStructInPNextChain<VkSurfaceCapabilitiesFullScreenExclusiveEXT>(pSurfaceCapabilities->pNext);
    if (capabilities_full_screen_exclusive) {
        const auto *full_screen_exclusive_win32_info =
            vku::FindStructInPNextChain<VkSurfaceFullScreenExclusiveWin32InfoEXT>(pSurfaceInfo->pNext);
        if (!full_screen_exclusive_win32_info) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-02671", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains "
                             "VkSurfaceCapabilitiesFullScreenExclusiveEXT, but pSurfaceInfo->pNext does not contain "
                             "VkSurfaceFullScreenExclusiveWin32InfoEXT");
        }
    }
#endif

    const auto *surface_present_mode_compatibility =
        vku::FindStructInPNextChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);
    const auto *surface_present_scaling_compatibilities =
        vku::FindStructInPNextChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);

    if (!(vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext))) {
        if (surface_present_mode_compatibility) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07776", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains VkSurfacePresentModeCompatibilityEXT, but "
                             "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
        }

        if (surface_present_scaling_compatibilities) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07777", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains VkSurfacePresentScalingCapabilitiesEXT, but "
                             "pSurfaceInfo->pNext does not contain a VkSurfacePresentModeEXT structure.");
        }
    }

    if (pSurfaceInfo->surface == VK_NULL_HANDLE) {
        if (surface_present_mode_compatibility) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07778", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains a "
                             "VkSurfacePresentModeCompatibilityEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
        }

        if (surface_present_scaling_compatibilities) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-pNext-07779", physicalDevice, error_obj.location,
                             "pSurfaceCapabilities->pNext contains a "
                             "VkSurfacePresentScalingCapabilitiesEXT structure, but pSurfaceInfo->surface is VK_NULL_HANDLE.");
        }
    }
    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                         uint32_t *pSurfaceFormatCount,
                                                                         VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                         const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    if (pSurfaceFormats) {
        if (vku::FindStructInPNextChain<VkImageCompressionPropertiesEXT>(pSurfaceFormats->pNext)) {
            const auto &physdev_extensions = physical_device_extensions.at(physicalDevice);
            if (!IsExtEnabled(physdev_extensions.vk_ext_image_compression_control_swapchain)) {
                skip |= LogError("VUID-VkSurfaceFormat2KHR-pNext-06750", physicalDevice,
                                 error_obj.location.dot(Field::pSurfaceFormats).dot(Field::pNext),
                                 "contains a VkImageCompressionPropertiesEXT struct but VK_EXT_image_compression_control_swapchain "
                                 "is not supported.");
            }
        }
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool Instance::manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                              uint32_t *pPresentModeCount,
                                                                              VkPresentModeKHR *pPresentModes,
                                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pSurfaceInfo && pSurfaceInfo->surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521", physicalDevice,
                         error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                         "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
    }
    return skip;
}

bool Instance::manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                           const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pCreateInfo->hwnd == nullptr) {
        skip |= LogError("VUID-VkWin32SurfaceCreateInfoKHR-hwnd-01308", instance, error_obj.location, "pCreateInfo->hwnd is NULL.");
    }

    return skip;
}

bool Device::PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device,
                                                                  const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                  VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    Context context(*this, error_obj, extensions);
    if (!IsExtEnabled(extensions.vk_khr_swapchain))
        skip |= OutputExtensionError(error_obj.location, {vvl::Extension::_VK_KHR_swapchain});
    if (!IsExtEnabled(extensions.vk_khr_get_surface_capabilities2))
        skip |= OutputExtensionError(error_obj.location, {vvl::Extension::_VK_KHR_get_surface_capabilities2});
    if (!IsExtEnabled(extensions.vk_khr_surface))
        skip |= OutputExtensionError(error_obj.location, {vvl::Extension::_VK_KHR_surface});
    if (!IsExtEnabled(extensions.vk_khr_get_physical_device_properties2))
        skip |= OutputExtensionError(error_obj.location, {vvl::Extension::_VK_KHR_get_physical_device_properties2});
    if (!IsExtEnabled(extensions.vk_ext_full_screen_exclusive))
        skip |= OutputExtensionError(error_obj.location, {vvl::Extension::_VK_EXT_full_screen_exclusive});
    if (!pModes) {
        skip |= LogError("VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pModes-parameter", device,
                         error_obj.location.dot(Field::pModes), "is NULL.");
    }

    skip |= context.ValidateStructType(
        error_obj.location.dot(Field::pSurfaceInfo), pSurfaceInfo, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, true,
        "VUID-vkGetDeviceGroupSurfacePresentModes2EXT-pSurfaceInfo-parameter", "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-sType");
    if (pSurfaceInfo != NULL) {
        constexpr std::array allowed_structs = {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
                                                VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT,
                                                VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT};

        skip |= context.ValidateStructPnext(error_obj.location.dot(Field::pSurfaceInfo), pSurfaceInfo->pNext,
                                            allowed_structs.size(), allowed_structs.data(), GeneratedVulkanHeaderVersion,
                                            "VUID-VkPhysicalDeviceSurfaceInfo2KHR-pNext-pNext",
                                            "VUID-VkPhysicalDeviceSurfaceInfo2KHR-sType-unique");

        if (pSurfaceInfo->surface == VK_NULL_HANDLE && !IsExtEnabled(extensions.vk_google_surfaceless_query)) {
            skip |= LogError("VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-pSurfaceInfo-06521", device,
                             error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface),
                             "is VK_NULL_HANDLE and VK_GOOGLE_surfaceless_query is not enabled.");
        }

        skip |=
            context.ValidateRequiredHandle(error_obj.location.dot(Field::pSurfaceInfo).dot(Field::surface), pSurfaceInfo->surface);
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool Instance::manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const auto display = pCreateInfo->display;
    const auto surface = pCreateInfo->surface;

    if (display == nullptr) {
        skip |= LogError("VUID-VkWaylandSurfaceCreateInfoKHR-display-01304", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::display), "is NULL!");
    }

    if (surface == nullptr) {
        skip |= LogError("VUID-VkWaylandSurfaceCreateInfoKHR-surface-01305", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::surface), "is NULL!");
    }

    return skip;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
bool Instance::manual_PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                         const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const auto connection = pCreateInfo->connection;
    const auto window = pCreateInfo->window;

    if (connection == nullptr) {
        skip |= LogError("VUID-VkXcbSurfaceCreateInfoKHR-connection-01310", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::connection), "is NULL!");
    }

    skip |= context.ValidateNotZero(window == 0, "VUID-VkXcbSurfaceCreateInfoKHR-window-01311",
                                    error_obj.location.dot(Field::pCreateInfo).dot(Field::window));

    return skip;
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool Instance::manual_PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const auto display = pCreateInfo->dpy;
    const auto window = pCreateInfo->window;

    if (display == nullptr) {
        skip |= LogError("VUID-VkXlibSurfaceCreateInfoKHR-dpy-01313", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::dpy), "is NULL!");
    }

    skip |= context.ValidateNotZero(window == 0, "VUID-VkXlibSurfaceCreateInfoKHR-window-01314",
                                    error_obj.location.dot(Field::pCreateInfo).dot(Field::window));

    return skip;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool Instance::manual_PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pCreateInfo->window == nullptr) {
        skip |= LogError("VUID-VkAndroidSurfaceCreateInfoKHR-window-01248", instance,
                         error_obj.location.dot(Field::pCreateInfo).dot(Field::window), "is NULL.");
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
}  // namespace stateless
