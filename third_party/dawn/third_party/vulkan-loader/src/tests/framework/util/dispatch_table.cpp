// VulkanFunctions - loads vulkan functions for tests to use
/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 */

#include "dispatch_table.h"

#include "env_var_wrapper.h"

std::filesystem::path get_loader_path() {
    auto loader_path = std::filesystem::path(FRAMEWORK_VULKAN_LIBRARY_PATH);
    auto env_var_res = get_env_var("VK_LOADER_TEST_LOADER_PATH", false);
    if (!env_var_res.empty()) {
        loader_path = std::filesystem::path(env_var_res);
    }
    return loader_path;
}

void init_vulkan_functions(VulkanFunctions& funcs) {
#if defined(APPLE_STATIC_LOADER)
#define GPA(name) name
#else
#define GPA(name) funcs.loader.get_symbol(#name)
#endif

    // clang-format off
    funcs.vkGetInstanceProcAddr = GPA(vkGetInstanceProcAddr);
    funcs.vkEnumerateInstanceExtensionProperties = GPA(vkEnumerateInstanceExtensionProperties);
    funcs.vkEnumerateInstanceLayerProperties = GPA(vkEnumerateInstanceLayerProperties);
    funcs.vkEnumerateInstanceVersion = GPA(vkEnumerateInstanceVersion);
    funcs.vkCreateInstance = GPA(vkCreateInstance);
    funcs.vkDestroyInstance = GPA(vkDestroyInstance);
    funcs.vkEnumeratePhysicalDevices = GPA(vkEnumeratePhysicalDevices);
    funcs.vkEnumeratePhysicalDeviceGroups = GPA(vkEnumeratePhysicalDeviceGroups);
    funcs.vkGetPhysicalDeviceFeatures = GPA(vkGetPhysicalDeviceFeatures);
    funcs.vkGetPhysicalDeviceFeatures2 = GPA(vkGetPhysicalDeviceFeatures2);
    funcs.vkGetPhysicalDeviceFormatProperties = GPA(vkGetPhysicalDeviceFormatProperties);
    funcs.vkGetPhysicalDeviceFormatProperties2 = GPA(vkGetPhysicalDeviceFormatProperties2);
    funcs.vkGetPhysicalDeviceImageFormatProperties = GPA(vkGetPhysicalDeviceImageFormatProperties);
    funcs.vkGetPhysicalDeviceImageFormatProperties2 = GPA(vkGetPhysicalDeviceImageFormatProperties2);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties = GPA(vkGetPhysicalDeviceSparseImageFormatProperties);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2 = GPA(vkGetPhysicalDeviceSparseImageFormatProperties2);
    funcs.vkGetPhysicalDeviceProperties = GPA(vkGetPhysicalDeviceProperties);
    funcs.vkGetPhysicalDeviceProperties2 = GPA(vkGetPhysicalDeviceProperties2);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties = GPA(vkGetPhysicalDeviceQueueFamilyProperties);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2 = GPA(vkGetPhysicalDeviceQueueFamilyProperties2);
    funcs.vkGetPhysicalDeviceMemoryProperties = GPA(vkGetPhysicalDeviceMemoryProperties);
    funcs.vkGetPhysicalDeviceMemoryProperties2 = GPA(vkGetPhysicalDeviceMemoryProperties2);
    funcs.vkGetPhysicalDeviceSurfaceSupportKHR = GPA(vkGetPhysicalDeviceSurfaceSupportKHR);
    funcs.vkGetPhysicalDeviceSurfaceFormatsKHR = GPA(vkGetPhysicalDeviceSurfaceFormatsKHR);
    funcs.vkGetPhysicalDeviceSurfacePresentModesKHR = GPA(vkGetPhysicalDeviceSurfacePresentModesKHR);
    funcs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = GPA(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    funcs.vkEnumerateDeviceExtensionProperties = GPA(vkEnumerateDeviceExtensionProperties);
    funcs.vkEnumerateDeviceLayerProperties = GPA(vkEnumerateDeviceLayerProperties);
    funcs.vkGetPhysicalDeviceExternalBufferProperties = GPA(vkGetPhysicalDeviceExternalBufferProperties);
    funcs.vkGetPhysicalDeviceExternalFenceProperties = GPA(vkGetPhysicalDeviceExternalFenceProperties);
    funcs.vkGetPhysicalDeviceExternalSemaphoreProperties = GPA(vkGetPhysicalDeviceExternalSemaphoreProperties);

    funcs.vkDestroySurfaceKHR = GPA(vkDestroySurfaceKHR);
    funcs.vkGetDeviceProcAddr = GPA(vkGetDeviceProcAddr);
    funcs.vkCreateDevice = GPA(vkCreateDevice);

    funcs.vkCreateHeadlessSurfaceEXT = GPA(vkCreateHeadlessSurfaceEXT);
    funcs.vkCreateDisplayPlaneSurfaceKHR = GPA(vkCreateDisplayPlaneSurfaceKHR);
    funcs.vkGetPhysicalDeviceDisplayPropertiesKHR = GPA(vkGetPhysicalDeviceDisplayPropertiesKHR);
    funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR = GPA(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
    funcs.vkGetDisplayPlaneSupportedDisplaysKHR = GPA(vkGetDisplayPlaneSupportedDisplaysKHR);
    funcs.vkGetDisplayModePropertiesKHR = GPA(vkGetDisplayModePropertiesKHR);
    funcs.vkCreateDisplayModeKHR = GPA(vkCreateDisplayModeKHR);
    funcs.vkGetDisplayPlaneCapabilitiesKHR = GPA(vkGetDisplayPlaneCapabilitiesKHR);
    funcs.vkGetPhysicalDevicePresentRectanglesKHR = GPA(vkGetPhysicalDevicePresentRectanglesKHR);
    funcs.vkGetPhysicalDeviceDisplayProperties2KHR = GPA(vkGetPhysicalDeviceDisplayProperties2KHR);
    funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR = GPA(vkGetPhysicalDeviceDisplayPlaneProperties2KHR);
    funcs.vkGetDisplayModeProperties2KHR = GPA(vkGetDisplayModeProperties2KHR);
    funcs.vkGetDisplayPlaneCapabilities2KHR = GPA(vkGetDisplayPlaneCapabilities2KHR);
    funcs.vkGetPhysicalDeviceSurfaceCapabilities2KHR = GPA(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
    funcs.vkGetPhysicalDeviceSurfaceFormats2KHR = GPA(vkGetPhysicalDeviceSurfaceFormats2KHR);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    funcs.vkCreateAndroidSurfaceKHR = GPA(vkCreateAndroidSurfaceKHR);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    funcs.vkCreateDirectFBSurfaceEXT = GPA(vkCreateDirectFBSurfaceEXT);
    funcs.vkGetPhysicalDeviceDirectFBPresentationSupportEXT = GPA(vkGetPhysicalDeviceDirectFBPresentationSupportEXT);
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_FUCHSIA)
    funcs.vkCreateImagePipeSurfaceFUCHSIA = GPA(vkCreateImagePipeSurfaceFUCHSIA);
#endif  // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_GGP)
    funcs.vkCreateStreamDescriptorSurfaceGGP = GPA(vkCreateStreamDescriptorSurfaceGGP);
#endif  // VK_USE_PLATFORM_GGP
#if defined(VK_USE_PLATFORM_IOS_MVK)
    funcs.vkCreateIOSSurfaceMVK = GPA(vkCreateIOSSurfaceMVK);
#endif  // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    funcs.vkCreateMacOSSurfaceMVK = GPA(vkCreateMacOSSurfaceMVK);
#endif  // VK_USE_PLATFORM_MACOS_MVK
#if defined(VK_USE_PLATFORM_METAL_EXT)
    funcs.vkCreateMetalSurfaceEXT = GPA(vkCreateMetalSurfaceEXT);
#endif  // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    funcs.vkCreateScreenSurfaceQNX = GPA(vkCreateScreenSurfaceQNX);
    funcs.vkGetPhysicalDeviceScreenPresentationSupportQNX = GPA(vkGetPhysicalDeviceScreenPresentationSupportQNX);
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    funcs.vkCreateWaylandSurfaceKHR = GPA(vkCreateWaylandSurfaceKHR);
    funcs.vkGetPhysicalDeviceWaylandPresentationSupportKHR = GPA(vkGetPhysicalDeviceWaylandPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    funcs.vkCreateXcbSurfaceKHR = GPA(vkCreateXcbSurfaceKHR);
    funcs.vkGetPhysicalDeviceXcbPresentationSupportKHR = GPA(vkGetPhysicalDeviceXcbPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    funcs.vkCreateXlibSurfaceKHR = GPA(vkCreateXlibSurfaceKHR);
    funcs.vkGetPhysicalDeviceXlibPresentationSupportKHR = GPA(vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkCreateWin32SurfaceKHR = GPA(vkCreateWin32SurfaceKHR);
    funcs.vkGetPhysicalDeviceWin32PresentationSupportKHR = GPA(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkDestroyDevice = GPA(vkDestroyDevice);
    funcs.vkGetDeviceQueue = GPA(vkGetDeviceQueue);
#undef GPA
    // clang-format on
}

#if defined(APPLE_STATIC_LOADER)
VulkanFunctions::VulkanFunctions() {
#else
VulkanFunctions::VulkanFunctions() : loader(get_loader_path()) {
#endif
    init_vulkan_functions(*this);
}

InstanceFunctions::InstanceFunctions(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, VkInstance instance) {
    this->vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vkCreateDebugReportCallbackEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    vkDestroyDebugReportCallbackEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    vkCreateDebugUtilsMessengerEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
}

DeviceFunctions::DeviceFunctions(const VulkanFunctions& vulkan_functions, VkDevice device) {
    vkGetDeviceProcAddr = vulkan_functions.vkGetDeviceProcAddr;
    vkDestroyDevice = load(device, "vkDestroyDevice");
    vkGetDeviceQueue = load(device, "vkGetDeviceQueue");
    vkCreateCommandPool = load(device, "vkCreateCommandPool");
    vkAllocateCommandBuffers = load(device, "vkAllocateCommandBuffers");
    vkDestroyCommandPool = load(device, "vkDestroyCommandPool");
    vkCreateSwapchainKHR = load(device, "vkCreateSwapchainKHR");
    vkGetSwapchainImagesKHR = load(device, "vkGetSwapchainImagesKHR");
    vkDestroySwapchainKHR = load(device, "vkDestroySwapchainKHR");
}
