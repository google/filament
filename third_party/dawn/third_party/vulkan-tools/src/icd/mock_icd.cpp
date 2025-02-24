/*
** Copyright (c) 2015-2018, 2023 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "mock_icd.h"
#include "function_definitions.h"

namespace vkmock {

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName) {
    // TODO: This function should only care about physical device functions and return nullptr for other functions
    const auto& item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    // Mock should intercept all functions so if we get here just return null
    return nullptr;
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

}  // namespace vkmock

extern "C" {

EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
    if (!vkmock::negotiate_loader_icd_interface_called) {
        vkmock::loader_interface_version = 1;
    }
    return vkmock::GetInstanceProcAddr(instance, pName);
}

EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetPhysicalDeviceProcAddr(VkInstance instance, const char* pName) {
    return vkmock::GetPhysicalDeviceProcAddr(instance, pName);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
    vkmock::negotiate_loader_icd_interface_called = true;
    vkmock::loader_interface_version = *pSupportedVersion;
    if (*pSupportedVersion > vkmock::SUPPORTED_LOADER_ICD_INTERFACE_VERSION) {
        *pSupportedVersion = vkmock::SUPPORTED_LOADER_ICD_INTERFACE_VERSION;
    }
    return VK_SUCCESS;
}

EXPORT VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                      const VkAllocationCallbacks* pAllocator) {
    vkmock::DestroySurfaceKHR(instance, surface, pAllocator);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                           VkBool32* pSupported) {
    return vkmock::GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface,
                                                                                VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    return vkmock::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                           uint32_t* pSurfaceFormatCount,
                                                                           VkSurfaceFormatKHR* pSurfaceFormats) {
    return vkmock::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                                                VkPresentModeKHR* pPresentModes) {
    return vkmock::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                     const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                                     const VkAllocationCallbacks* pAllocator,
                                                                     VkSurfaceKHR* pSurface) {
    return vkmock::CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_XLIB_KHR */

#ifdef VK_USE_PLATFORM_XCB_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_XCB_KHR */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(VkInstance instance,
                                                                const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */

#ifdef VK_USE_PLATFORM_ANDROID_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateAndroidSurfaceKHR(VkInstance instance,
                                                                const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_ANDROID_KHR */

#ifdef VK_USE_PLATFORM_WIN32_KHR

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_WIN32_KHR */

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface,
                                                                             VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return vkmock::GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              uint32_t* pRectCount, VkRect2D* pRects) {
    return vkmock::GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

#ifdef VK_USE_PLATFORM_VI_NN

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_VI_NN */

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                 VkSurfaceKHR surface,
                                                                                 VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    return vkmock::GetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
}

#ifdef VK_USE_PLATFORM_IOS_MVK

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_IOS_MVK */

#ifdef VK_USE_PLATFORM_MACOS_MVK

EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vkmock::CreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* VK_USE_PLATFORM_MACOS_MVK */

}  // end extern "C"
