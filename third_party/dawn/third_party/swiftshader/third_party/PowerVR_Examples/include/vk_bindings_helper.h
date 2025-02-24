// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_bindings_helper_generator.py for modifications

/*
\brief Helper functions for filling vk bindings function pointer tables.
\file vk_bindings_helper.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/* Corresponding to Vulkan registry file version #132# */

#pragma once
#include <string>
#include <cstring>
#include "pvr_openlib.h"
#include "vk_bindings.h"

namespace vk {
namespace internal {
/** DEFINE THE PLATFORM SPECIFIC LIBRARY NAME **/
#ifdef _WIN32
static const char* libName = "vulkan-1.dll";
#elif defined(TARGET_OS_MAC)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
static const char* libName = "libMoltenVK.dylib";
#else
static const char* libName = "libvulkan.dylib";
#endif
#else
static const char* libName = "libvulkan.so.1;libvulkan.so";
#endif
}
}

static inline bool initVkBindings(VkBindings *bindings)
{
	pvr::lib::LIBTYPE lib = pvr::lib::openlib(vk::internal::libName);

	memset(bindings, 0, sizeof(*bindings));

	// Load the function pointer dynamically for vkGetInstanceProcAddr
	bindings->vkGetInstanceProcAddr = pvr::lib::getLibFunctionChecked<PFN_vkGetInstanceProcAddr>(lib, "vkGetInstanceProcAddr");

	// Use vkGetInstanceProcAddr with a NULL instance to retrieve the function pointers
	bindings->vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
	bindings->vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
	bindings->vkCreateInstance = (PFN_vkCreateInstance)bindings->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
	bindings->vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)bindings->vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");

	// Get bindings for MVKConfiguration functions when using MoltenVK.
#ifdef VK_USE_PLATFORM_MACOS_MVK
	bindings->vkGetMoltenVKConfigurationMVK = pvr::lib::getLibFunctionChecked<PFN_vkGetMoltenVKConfigurationMVK>(lib, "vkGetMoltenVKConfigurationMVK");
	bindings->vkSetMoltenVKConfigurationMVK = pvr::lib::getLibFunctionChecked<PFN_vkSetMoltenVKConfigurationMVK>(lib, "vkSetMoltenVKConfigurationMVK");
#endif

	// validate that we have all necessary function pointers to continue
	if (!bindings->vkEnumerateInstanceExtensionProperties || !bindings->vkEnumerateInstanceLayerProperties || !bindings->vkCreateInstance)
	{
		return false;
	}
	return true;
}


static inline void initVkInstanceBindings(VkInstance instance, VkInstanceBindings *bindings, PFN_vkGetInstanceProcAddr getInstanceProcAddr) {
	memset(bindings, 0, sizeof(*bindings));
	// Instance function pointers

#if (defined(VK_VERSION_1_0))
	bindings->vkCreateDevice = (PFN_vkCreateDevice)getInstanceProcAddr(instance, "vkCreateDevice");
	bindings->vkDestroyInstance = (PFN_vkDestroyInstance)getInstanceProcAddr(instance, "vkDestroyInstance");
	bindings->vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)getInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
	bindings->vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)getInstanceProcAddr(instance, "vkEnumerateDeviceLayerProperties");
	bindings->vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)getInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
	bindings->vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)getInstanceProcAddr(instance, "vkGetDeviceProcAddr");
	bindings->vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
	bindings->vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties");
	bindings->vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties");
	bindings->vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
	bindings->vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
	bindings->vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
	bindings->vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
#endif // VK_VERSION_1_0

#if (defined(VK_VERSION_1_1))
	bindings->vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)getInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups");
	bindings->vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferProperties");
	bindings->vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalFenceProperties");
	bindings->vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
	bindings->vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2");
	bindings->vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties2");
	bindings->vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2");
	bindings->vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2");
	bindings->vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2");
	bindings->vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
	bindings->vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
#endif // VK_VERSION_1_1

#if (defined(VK_EXT_acquire_xlib_display))
#if (defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT))
	bindings->vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)getInstanceProcAddr(instance, "vkAcquireXlibDisplayEXT");
	bindings->vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)getInstanceProcAddr(instance, "vkGetRandROutputDisplayEXT");
#endif // (VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // VK_EXT_acquire_xlib_display

#if (defined(VK_EXT_calibrated_timestamps))
	bindings->vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)getInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
#endif // VK_EXT_calibrated_timestamps

#if (defined(VK_EXT_debug_report))
	bindings->vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)getInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	bindings->vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)getInstanceProcAddr(instance, "vkDebugReportMessageEXT");
	bindings->vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)getInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
#endif // VK_EXT_debug_report

#if (defined(VK_EXT_debug_utils))
	bindings->vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
	bindings->vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
	bindings->vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
	bindings->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)getInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	bindings->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)getInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	bindings->vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
	bindings->vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");
	bindings->vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)getInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
	bindings->vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)getInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	bindings->vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)getInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
	bindings->vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)getInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
#endif // VK_EXT_debug_utils

#if (defined(VK_EXT_direct_mode_display))
	bindings->vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)getInstanceProcAddr(instance, "vkReleaseDisplayEXT");
#endif // VK_EXT_direct_mode_display

#if (defined(VK_EXT_display_surface_counter))
	bindings->vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#endif // VK_EXT_display_surface_counter

#if (defined(VK_EXT_full_screen_exclusive))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkGetPhysicalDeviceSurfacePresentModes2EXT = (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_EXT_full_screen_exclusive

#if (defined(VK_EXT_headless_surface))
	bindings->vkCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)getInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
#endif // VK_EXT_headless_surface

#if (defined(VK_EXT_metal_surface))
#if (defined(VK_USE_PLATFORM_METAL_EXT))
	bindings->vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)getInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
#endif // (VK_USE_PLATFORM_METAL_EXT)
#endif // VK_EXT_metal_surface

#if (defined(VK_EXT_sample_locations))
	bindings->vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)getInstanceProcAddr(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
#endif // VK_EXT_sample_locations

#if (defined(VK_EXT_tooling_info))
	bindings->vkGetPhysicalDeviceToolPropertiesEXT = (PFN_vkGetPhysicalDeviceToolPropertiesEXT)getInstanceProcAddr(instance, "vkGetPhysicalDeviceToolPropertiesEXT");
#endif // VK_EXT_tooling_info

#if (defined(VK_FUCHSIA_imagepipe_surface))
#if (defined(VK_USE_PLATFORM_FUCHSIA))
	bindings->vkCreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)getInstanceProcAddr(instance, "vkCreateImagePipeSurfaceFUCHSIA");
#endif // (VK_USE_PLATFORM_FUCHSIA)
#endif // VK_FUCHSIA_imagepipe_surface

#if (defined(VK_GGP_stream_descriptor_surface))
#if (defined(VK_USE_PLATFORM_GGP))
	bindings->vkCreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)getInstanceProcAddr(instance, "vkCreateStreamDescriptorSurfaceGGP");
#endif // (VK_USE_PLATFORM_GGP)
#endif // VK_GGP_stream_descriptor_surface

#if (defined(VK_KHR_android_surface))
#if (defined(VK_USE_PLATFORM_ANDROID_KHR))
	bindings->vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)getInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
#endif // (VK_USE_PLATFORM_ANDROID_KHR)
#endif // VK_KHR_android_surface

#if (defined(VK_KHR_device_group_creation))
	bindings->vkEnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)getInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
#endif // VK_KHR_device_group_creation

#if (defined(VK_KHR_display))
	bindings->vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)getInstanceProcAddr(instance, "vkCreateDisplayModeKHR");
	bindings->vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)getInstanceProcAddr(instance, "vkCreateDisplayPlaneSurfaceKHR");
	bindings->vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)getInstanceProcAddr(instance, "vkGetDisplayModePropertiesKHR");
	bindings->vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)getInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilitiesKHR");
	bindings->vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)getInstanceProcAddr(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
	bindings->vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
	bindings->vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
#endif // VK_KHR_display

#if (defined(VK_KHR_external_fence_capabilities))
	bindings->vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
#endif // VK_KHR_external_fence_capabilities

#if (defined(VK_KHR_external_memory_capabilities))
	bindings->vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
#endif // VK_KHR_external_memory_capabilities

#if (defined(VK_KHR_external_semaphore_capabilities))
	bindings->vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
#endif // VK_KHR_external_semaphore_capabilities

#if (defined(VK_KHR_get_display_properties2))
	bindings->vkGetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)getInstanceProcAddr(instance, "vkGetDisplayModeProperties2KHR");
	bindings->vkGetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)getInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilities2KHR");
	bindings->vkGetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
	bindings->vkGetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
#endif // VK_KHR_get_display_properties2

#if (defined(VK_KHR_get_physical_device_properties2))
	bindings->vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2KHR");
	bindings->vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
	bindings->vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
	bindings->vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
	bindings->vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
	bindings->vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
	bindings->vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
#endif // VK_KHR_get_physical_device_properties2

#if (defined(VK_KHR_get_surface_capabilities2))
	bindings->vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
	bindings->vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
#endif // VK_KHR_get_surface_capabilities2

#if (defined(VK_KHR_performance_query))
	bindings->vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)getInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
	bindings->vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
#endif // VK_KHR_performance_query

#if (defined(VK_KHR_surface))
	bindings->vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)getInstanceProcAddr(instance, "vkDestroySurfaceKHR");
	bindings->vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	bindings->vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	bindings->vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	bindings->vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
#endif // VK_KHR_surface

#if (defined(VK_KHR_swapchain) || defined(VK_KHR_device_group))
	bindings->vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)getInstanceProcAddr(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
#endif // VK_KHR_swapchain,VK_KHR_device_group

#if (defined(VK_KHR_wayland_surface))
#if (defined(VK_USE_PLATFORM_WAYLAND_KHR))
	bindings->vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)getInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
	bindings->vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // (VK_USE_PLATFORM_WAYLAND_KHR)
#endif // VK_KHR_wayland_surface

#if (defined(VK_KHR_win32_surface))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)getInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	bindings->vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_win32_surface

#if (defined(VK_KHR_xcb_surface))
#if (defined(VK_USE_PLATFORM_XCB_KHR))
	bindings->vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)getInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
	bindings->vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // (VK_USE_PLATFORM_XCB_KHR)
#endif // VK_KHR_xcb_surface

#if (defined(VK_KHR_xlib_surface))
#if (defined(VK_USE_PLATFORM_XLIB_KHR))
	bindings->vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)getInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	bindings->vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)getInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // (VK_USE_PLATFORM_XLIB_KHR)
#endif // VK_KHR_xlib_surface

#if (defined(VK_MVK_ios_surface))
#if (defined(VK_USE_PLATFORM_IOS_MVK))
	bindings->vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)getInstanceProcAddr(instance, "vkCreateIOSSurfaceMVK");
#endif // (VK_USE_PLATFORM_IOS_MVK)
#endif // VK_MVK_ios_surface

#if (defined(VK_MVK_macos_surface))
#if (defined(VK_USE_PLATFORM_MACOS_MVK))
	bindings->vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)getInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
#endif // (VK_USE_PLATFORM_MACOS_MVK)
#endif // VK_MVK_macos_surface

#if (defined(VK_NN_vi_surface))
#if (defined(VK_USE_PLATFORM_VI_NN))
	bindings->vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)getInstanceProcAddr(instance, "vkCreateViSurfaceNN");
#endif // (VK_USE_PLATFORM_VI_NN)
#endif // VK_NN_vi_surface

#if (defined(VK_NVX_device_generated_commands))
	bindings->vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)getInstanceProcAddr(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
#endif // VK_NVX_device_generated_commands

#if (defined(VK_NV_cooperative_matrix))
	bindings->vkGetPhysicalDeviceCooperativeMatrixPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)getInstanceProcAddr(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
#endif // VK_NV_cooperative_matrix

#if (defined(VK_NV_coverage_reduction_mode))
	bindings->vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)getInstanceProcAddr(instance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
#endif // VK_NV_coverage_reduction_mode

#if (defined(VK_NV_external_memory_capabilities))
	bindings->vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#endif // VK_NV_external_memory_capabilities

}

static inline void initVkDeviceBindings(VkDevice device, VkDeviceBindings *bindings, PFN_vkGetDeviceProcAddr getDeviceProcAddr) {
	memset(bindings, 0, sizeof(*bindings));
	// Device function pointers

#if (defined(VK_VERSION_1_0))
	bindings->vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)getDeviceProcAddr(device, "vkAllocateCommandBuffers");
	bindings->vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)getDeviceProcAddr(device, "vkAllocateDescriptorSets");
	bindings->vkAllocateMemory = (PFN_vkAllocateMemory)getDeviceProcAddr(device, "vkAllocateMemory");
	bindings->vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)getDeviceProcAddr(device, "vkBeginCommandBuffer");
	bindings->vkBindBufferMemory = (PFN_vkBindBufferMemory)getDeviceProcAddr(device, "vkBindBufferMemory");
	bindings->vkBindImageMemory = (PFN_vkBindImageMemory)getDeviceProcAddr(device, "vkBindImageMemory");
	bindings->vkCmdBeginQuery = (PFN_vkCmdBeginQuery)getDeviceProcAddr(device, "vkCmdBeginQuery");
	bindings->vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)getDeviceProcAddr(device, "vkCmdBeginRenderPass");
	bindings->vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)getDeviceProcAddr(device, "vkCmdBindDescriptorSets");
	bindings->vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)getDeviceProcAddr(device, "vkCmdBindIndexBuffer");
	bindings->vkCmdBindPipeline = (PFN_vkCmdBindPipeline)getDeviceProcAddr(device, "vkCmdBindPipeline");
	bindings->vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)getDeviceProcAddr(device, "vkCmdBindVertexBuffers");
	bindings->vkCmdBlitImage = (PFN_vkCmdBlitImage)getDeviceProcAddr(device, "vkCmdBlitImage");
	bindings->vkCmdClearAttachments = (PFN_vkCmdClearAttachments)getDeviceProcAddr(device, "vkCmdClearAttachments");
	bindings->vkCmdClearColorImage = (PFN_vkCmdClearColorImage)getDeviceProcAddr(device, "vkCmdClearColorImage");
	bindings->vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)getDeviceProcAddr(device, "vkCmdClearDepthStencilImage");
	bindings->vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)getDeviceProcAddr(device, "vkCmdCopyBuffer");
	bindings->vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)getDeviceProcAddr(device, "vkCmdCopyBufferToImage");
	bindings->vkCmdCopyImage = (PFN_vkCmdCopyImage)getDeviceProcAddr(device, "vkCmdCopyImage");
	bindings->vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)getDeviceProcAddr(device, "vkCmdCopyImageToBuffer");
	bindings->vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)getDeviceProcAddr(device, "vkCmdCopyQueryPoolResults");
	bindings->vkCmdDispatch = (PFN_vkCmdDispatch)getDeviceProcAddr(device, "vkCmdDispatch");
	bindings->vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)getDeviceProcAddr(device, "vkCmdDispatchIndirect");
	bindings->vkCmdDraw = (PFN_vkCmdDraw)getDeviceProcAddr(device, "vkCmdDraw");
	bindings->vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)getDeviceProcAddr(device, "vkCmdDrawIndexed");
	bindings->vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)getDeviceProcAddr(device, "vkCmdDrawIndexedIndirect");
	bindings->vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)getDeviceProcAddr(device, "vkCmdDrawIndirect");
	bindings->vkCmdEndQuery = (PFN_vkCmdEndQuery)getDeviceProcAddr(device, "vkCmdEndQuery");
	bindings->vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)getDeviceProcAddr(device, "vkCmdEndRenderPass");
	bindings->vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)getDeviceProcAddr(device, "vkCmdExecuteCommands");
	bindings->vkCmdFillBuffer = (PFN_vkCmdFillBuffer)getDeviceProcAddr(device, "vkCmdFillBuffer");
	bindings->vkCmdNextSubpass = (PFN_vkCmdNextSubpass)getDeviceProcAddr(device, "vkCmdNextSubpass");
	bindings->vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)getDeviceProcAddr(device, "vkCmdPipelineBarrier");
	bindings->vkCmdPushConstants = (PFN_vkCmdPushConstants)getDeviceProcAddr(device, "vkCmdPushConstants");
	bindings->vkCmdResetEvent = (PFN_vkCmdResetEvent)getDeviceProcAddr(device, "vkCmdResetEvent");
	bindings->vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)getDeviceProcAddr(device, "vkCmdResetQueryPool");
	bindings->vkCmdResolveImage = (PFN_vkCmdResolveImage)getDeviceProcAddr(device, "vkCmdResolveImage");
	bindings->vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)getDeviceProcAddr(device, "vkCmdSetBlendConstants");
	bindings->vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)getDeviceProcAddr(device, "vkCmdSetDepthBias");
	bindings->vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)getDeviceProcAddr(device, "vkCmdSetDepthBounds");
	bindings->vkCmdSetEvent = (PFN_vkCmdSetEvent)getDeviceProcAddr(device, "vkCmdSetEvent");
	bindings->vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)getDeviceProcAddr(device, "vkCmdSetLineWidth");
	bindings->vkCmdSetScissor = (PFN_vkCmdSetScissor)getDeviceProcAddr(device, "vkCmdSetScissor");
	bindings->vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)getDeviceProcAddr(device, "vkCmdSetStencilCompareMask");
	bindings->vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)getDeviceProcAddr(device, "vkCmdSetStencilReference");
	bindings->vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)getDeviceProcAddr(device, "vkCmdSetStencilWriteMask");
	bindings->vkCmdSetViewport = (PFN_vkCmdSetViewport)getDeviceProcAddr(device, "vkCmdSetViewport");
	bindings->vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)getDeviceProcAddr(device, "vkCmdUpdateBuffer");
	bindings->vkCmdWaitEvents = (PFN_vkCmdWaitEvents)getDeviceProcAddr(device, "vkCmdWaitEvents");
	bindings->vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)getDeviceProcAddr(device, "vkCmdWriteTimestamp");
	bindings->vkCreateBuffer = (PFN_vkCreateBuffer)getDeviceProcAddr(device, "vkCreateBuffer");
	bindings->vkCreateBufferView = (PFN_vkCreateBufferView)getDeviceProcAddr(device, "vkCreateBufferView");
	bindings->vkCreateCommandPool = (PFN_vkCreateCommandPool)getDeviceProcAddr(device, "vkCreateCommandPool");
	bindings->vkCreateComputePipelines = (PFN_vkCreateComputePipelines)getDeviceProcAddr(device, "vkCreateComputePipelines");
	bindings->vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)getDeviceProcAddr(device, "vkCreateDescriptorPool");
	bindings->vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)getDeviceProcAddr(device, "vkCreateDescriptorSetLayout");
	bindings->vkCreateEvent = (PFN_vkCreateEvent)getDeviceProcAddr(device, "vkCreateEvent");
	bindings->vkCreateFence = (PFN_vkCreateFence)getDeviceProcAddr(device, "vkCreateFence");
	bindings->vkCreateFramebuffer = (PFN_vkCreateFramebuffer)getDeviceProcAddr(device, "vkCreateFramebuffer");
	bindings->vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)getDeviceProcAddr(device, "vkCreateGraphicsPipelines");
	bindings->vkCreateImage = (PFN_vkCreateImage)getDeviceProcAddr(device, "vkCreateImage");
	bindings->vkCreateImageView = (PFN_vkCreateImageView)getDeviceProcAddr(device, "vkCreateImageView");
	bindings->vkCreatePipelineCache = (PFN_vkCreatePipelineCache)getDeviceProcAddr(device, "vkCreatePipelineCache");
	bindings->vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)getDeviceProcAddr(device, "vkCreatePipelineLayout");
	bindings->vkCreateQueryPool = (PFN_vkCreateQueryPool)getDeviceProcAddr(device, "vkCreateQueryPool");
	bindings->vkCreateRenderPass = (PFN_vkCreateRenderPass)getDeviceProcAddr(device, "vkCreateRenderPass");
	bindings->vkCreateSampler = (PFN_vkCreateSampler)getDeviceProcAddr(device, "vkCreateSampler");
	bindings->vkCreateSemaphore = (PFN_vkCreateSemaphore)getDeviceProcAddr(device, "vkCreateSemaphore");
	bindings->vkCreateShaderModule = (PFN_vkCreateShaderModule)getDeviceProcAddr(device, "vkCreateShaderModule");
	bindings->vkDestroyBuffer = (PFN_vkDestroyBuffer)getDeviceProcAddr(device, "vkDestroyBuffer");
	bindings->vkDestroyBufferView = (PFN_vkDestroyBufferView)getDeviceProcAddr(device, "vkDestroyBufferView");
	bindings->vkDestroyCommandPool = (PFN_vkDestroyCommandPool)getDeviceProcAddr(device, "vkDestroyCommandPool");
	bindings->vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)getDeviceProcAddr(device, "vkDestroyDescriptorPool");
	bindings->vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)getDeviceProcAddr(device, "vkDestroyDescriptorSetLayout");
	bindings->vkDestroyDevice = (PFN_vkDestroyDevice)getDeviceProcAddr(device, "vkDestroyDevice");
	bindings->vkDestroyEvent = (PFN_vkDestroyEvent)getDeviceProcAddr(device, "vkDestroyEvent");
	bindings->vkDestroyFence = (PFN_vkDestroyFence)getDeviceProcAddr(device, "vkDestroyFence");
	bindings->vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)getDeviceProcAddr(device, "vkDestroyFramebuffer");
	bindings->vkDestroyImage = (PFN_vkDestroyImage)getDeviceProcAddr(device, "vkDestroyImage");
	bindings->vkDestroyImageView = (PFN_vkDestroyImageView)getDeviceProcAddr(device, "vkDestroyImageView");
	bindings->vkDestroyPipeline = (PFN_vkDestroyPipeline)getDeviceProcAddr(device, "vkDestroyPipeline");
	bindings->vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)getDeviceProcAddr(device, "vkDestroyPipelineCache");
	bindings->vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)getDeviceProcAddr(device, "vkDestroyPipelineLayout");
	bindings->vkDestroyQueryPool = (PFN_vkDestroyQueryPool)getDeviceProcAddr(device, "vkDestroyQueryPool");
	bindings->vkDestroyRenderPass = (PFN_vkDestroyRenderPass)getDeviceProcAddr(device, "vkDestroyRenderPass");
	bindings->vkDestroySampler = (PFN_vkDestroySampler)getDeviceProcAddr(device, "vkDestroySampler");
	bindings->vkDestroySemaphore = (PFN_vkDestroySemaphore)getDeviceProcAddr(device, "vkDestroySemaphore");
	bindings->vkDestroyShaderModule = (PFN_vkDestroyShaderModule)getDeviceProcAddr(device, "vkDestroyShaderModule");
	bindings->vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)getDeviceProcAddr(device, "vkDeviceWaitIdle");
	bindings->vkEndCommandBuffer = (PFN_vkEndCommandBuffer)getDeviceProcAddr(device, "vkEndCommandBuffer");
	bindings->vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)getDeviceProcAddr(device, "vkFlushMappedMemoryRanges");
	bindings->vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)getDeviceProcAddr(device, "vkFreeCommandBuffers");
	bindings->vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)getDeviceProcAddr(device, "vkFreeDescriptorSets");
	bindings->vkFreeMemory = (PFN_vkFreeMemory)getDeviceProcAddr(device, "vkFreeMemory");
	bindings->vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)getDeviceProcAddr(device, "vkGetBufferMemoryRequirements");
	bindings->vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)getDeviceProcAddr(device, "vkGetDeviceMemoryCommitment");
	bindings->vkGetDeviceQueue = (PFN_vkGetDeviceQueue)getDeviceProcAddr(device, "vkGetDeviceQueue");
	bindings->vkGetEventStatus = (PFN_vkGetEventStatus)getDeviceProcAddr(device, "vkGetEventStatus");
	bindings->vkGetFenceStatus = (PFN_vkGetFenceStatus)getDeviceProcAddr(device, "vkGetFenceStatus");
	bindings->vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)getDeviceProcAddr(device, "vkGetImageMemoryRequirements");
	bindings->vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)getDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements");
	bindings->vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)getDeviceProcAddr(device, "vkGetImageSubresourceLayout");
	bindings->vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData)getDeviceProcAddr(device, "vkGetPipelineCacheData");
	bindings->vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)getDeviceProcAddr(device, "vkGetQueryPoolResults");
	bindings->vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)getDeviceProcAddr(device, "vkGetRenderAreaGranularity");
	bindings->vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)getDeviceProcAddr(device, "vkInvalidateMappedMemoryRanges");
	bindings->vkMapMemory = (PFN_vkMapMemory)getDeviceProcAddr(device, "vkMapMemory");
	bindings->vkMergePipelineCaches = (PFN_vkMergePipelineCaches)getDeviceProcAddr(device, "vkMergePipelineCaches");
	bindings->vkQueueBindSparse = (PFN_vkQueueBindSparse)getDeviceProcAddr(device, "vkQueueBindSparse");
	bindings->vkQueueSubmit = (PFN_vkQueueSubmit)getDeviceProcAddr(device, "vkQueueSubmit");
	bindings->vkQueueWaitIdle = (PFN_vkQueueWaitIdle)getDeviceProcAddr(device, "vkQueueWaitIdle");
	bindings->vkResetCommandBuffer = (PFN_vkResetCommandBuffer)getDeviceProcAddr(device, "vkResetCommandBuffer");
	bindings->vkResetCommandPool = (PFN_vkResetCommandPool)getDeviceProcAddr(device, "vkResetCommandPool");
	bindings->vkResetDescriptorPool = (PFN_vkResetDescriptorPool)getDeviceProcAddr(device, "vkResetDescriptorPool");
	bindings->vkResetEvent = (PFN_vkResetEvent)getDeviceProcAddr(device, "vkResetEvent");
	bindings->vkResetFences = (PFN_vkResetFences)getDeviceProcAddr(device, "vkResetFences");
	bindings->vkSetEvent = (PFN_vkSetEvent)getDeviceProcAddr(device, "vkSetEvent");
	bindings->vkUnmapMemory = (PFN_vkUnmapMemory)getDeviceProcAddr(device, "vkUnmapMemory");
	bindings->vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)getDeviceProcAddr(device, "vkUpdateDescriptorSets");
	bindings->vkWaitForFences = (PFN_vkWaitForFences)getDeviceProcAddr(device, "vkWaitForFences");
#endif // VK_VERSION_1_0

#if (defined(VK_VERSION_1_1))
	bindings->vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)getDeviceProcAddr(device, "vkBindBufferMemory2");
	bindings->vkBindImageMemory2 = (PFN_vkBindImageMemory2)getDeviceProcAddr(device, "vkBindImageMemory2");
	bindings->vkCmdDispatchBase = (PFN_vkCmdDispatchBase)getDeviceProcAddr(device, "vkCmdDispatchBase");
	bindings->vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)getDeviceProcAddr(device, "vkCmdSetDeviceMask");
	bindings->vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)getDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplate");
	bindings->vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)getDeviceProcAddr(device, "vkCreateSamplerYcbcrConversion");
	bindings->vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)getDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplate");
	bindings->vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)getDeviceProcAddr(device, "vkDestroySamplerYcbcrConversion");
	bindings->vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)getDeviceProcAddr(device, "vkGetBufferMemoryRequirements2");
	bindings->vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)getDeviceProcAddr(device, "vkGetDescriptorSetLayoutSupport");
	bindings->vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)getDeviceProcAddr(device, "vkGetDeviceGroupPeerMemoryFeatures");
	bindings->vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)getDeviceProcAddr(device, "vkGetDeviceQueue2");
	bindings->vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)getDeviceProcAddr(device, "vkGetImageMemoryRequirements2");
	bindings->vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)getDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2");
	bindings->vkTrimCommandPool = (PFN_vkTrimCommandPool)getDeviceProcAddr(device, "vkTrimCommandPool");
	bindings->vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)getDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplate");
#endif // VK_VERSION_1_1

#if (defined(VK_VERSION_1_2))
	bindings->vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)getDeviceProcAddr(device, "vkCmdBeginRenderPass2");
	bindings->vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)getDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCount");
	bindings->vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)getDeviceProcAddr(device, "vkCmdDrawIndirectCount");
	bindings->vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)getDeviceProcAddr(device, "vkCmdEndRenderPass2");
	bindings->vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2)getDeviceProcAddr(device, "vkCmdNextSubpass2");
	bindings->vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)getDeviceProcAddr(device, "vkCreateRenderPass2");
	bindings->vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)getDeviceProcAddr(device, "vkGetBufferDeviceAddress");
	bindings->vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)getDeviceProcAddr(device, "vkGetBufferOpaqueCaptureAddress");
	bindings->vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)getDeviceProcAddr(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
	bindings->vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)getDeviceProcAddr(device, "vkGetSemaphoreCounterValue");
	bindings->vkResetQueryPool = (PFN_vkResetQueryPool)getDeviceProcAddr(device, "vkResetQueryPool");
	bindings->vkSignalSemaphore = (PFN_vkSignalSemaphore)getDeviceProcAddr(device, "vkSignalSemaphore");
	bindings->vkWaitSemaphores = (PFN_vkWaitSemaphores)getDeviceProcAddr(device, "vkWaitSemaphores");
#endif // VK_VERSION_1_2

#if (defined(VK_AMD_buffer_marker))
	bindings->vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)getDeviceProcAddr(device, "vkCmdWriteBufferMarkerAMD");
#endif // VK_AMD_buffer_marker

#if (defined(VK_AMD_display_native_hdr))
	bindings->vkSetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)getDeviceProcAddr(device, "vkSetLocalDimmingAMD");
#endif // VK_AMD_display_native_hdr

#if (defined(VK_AMD_draw_indirect_count))
	bindings->vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)getDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCountAMD");
	bindings->vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)getDeviceProcAddr(device, "vkCmdDrawIndirectCountAMD");
#endif // VK_AMD_draw_indirect_count

#if (defined(VK_AMD_shader_info))
	bindings->vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)getDeviceProcAddr(device, "vkGetShaderInfoAMD");
#endif // VK_AMD_shader_info

#if (defined(VK_ANDROID_external_memory_android_hardware_buffer))
#if (defined(VK_USE_PLATFORM_ANDROID_KHR))
	bindings->vkGetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)getDeviceProcAddr(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
	bindings->vkGetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID)getDeviceProcAddr(device, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif // (VK_USE_PLATFORM_ANDROID_KHR)
#endif // VK_ANDROID_external_memory_android_hardware_buffer

#if (defined(VK_EXT_buffer_device_address))
	bindings->vkGetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)getDeviceProcAddr(device, "vkGetBufferDeviceAddressEXT");
#endif // VK_EXT_buffer_device_address

#if (defined(VK_EXT_calibrated_timestamps))
	bindings->vkGetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)getDeviceProcAddr(device, "vkGetCalibratedTimestampsEXT");
#endif // VK_EXT_calibrated_timestamps

#if (defined(VK_EXT_conditional_rendering))
	bindings->vkCmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)getDeviceProcAddr(device, "vkCmdBeginConditionalRenderingEXT");
	bindings->vkCmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)getDeviceProcAddr(device, "vkCmdEndConditionalRenderingEXT");
#endif // VK_EXT_conditional_rendering

#if (defined(VK_EXT_debug_marker))
	bindings->vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)getDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
	bindings->vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)getDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
	bindings->vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)getDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	bindings->vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)getDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
	bindings->vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)getDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
#endif // VK_EXT_debug_marker

#if (defined(VK_EXT_discard_rectangles))
	bindings->vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)getDeviceProcAddr(device, "vkCmdSetDiscardRectangleEXT");
#endif // VK_EXT_discard_rectangles

#if (defined(VK_EXT_display_control))
	bindings->vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)getDeviceProcAddr(device, "vkDisplayPowerControlEXT");
	bindings->vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)getDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
	bindings->vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)getDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
	bindings->vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)getDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
#endif // VK_EXT_display_control

#if (defined(VK_EXT_external_memory_host))
	bindings->vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)getDeviceProcAddr(device, "vkGetMemoryHostPointerPropertiesEXT");
#endif // VK_EXT_external_memory_host

#if (defined(VK_EXT_full_screen_exclusive))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkAcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)getDeviceProcAddr(device, "vkAcquireFullScreenExclusiveModeEXT");
	bindings->vkGetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)getDeviceProcAddr(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
	bindings->vkReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)getDeviceProcAddr(device, "vkReleaseFullScreenExclusiveModeEXT");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_EXT_full_screen_exclusive

#if (defined(VK_EXT_hdr_metadata))
	bindings->vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)getDeviceProcAddr(device, "vkSetHdrMetadataEXT");
#endif // VK_EXT_hdr_metadata

#if (defined(VK_EXT_host_query_reset))
	bindings->vkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)getDeviceProcAddr(device, "vkResetQueryPoolEXT");
#endif // VK_EXT_host_query_reset

#if (defined(VK_EXT_image_drm_format_modifier))
	bindings->vkGetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)getDeviceProcAddr(device, "vkGetImageDrmFormatModifierPropertiesEXT");
#endif // VK_EXT_image_drm_format_modifier

#if (defined(VK_EXT_line_rasterization))
	bindings->vkCmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)getDeviceProcAddr(device, "vkCmdSetLineStippleEXT");
#endif // VK_EXT_line_rasterization

#if (defined(VK_EXT_sample_locations))
	bindings->vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)getDeviceProcAddr(device, "vkCmdSetSampleLocationsEXT");
#endif // VK_EXT_sample_locations

#if (defined(VK_EXT_transform_feedback))
	bindings->vkCmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)getDeviceProcAddr(device, "vkCmdBeginQueryIndexedEXT");
	bindings->vkCmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)getDeviceProcAddr(device, "vkCmdBeginTransformFeedbackEXT");
	bindings->vkCmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT)getDeviceProcAddr(device, "vkCmdBindTransformFeedbackBuffersEXT");
	bindings->vkCmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)getDeviceProcAddr(device, "vkCmdDrawIndirectByteCountEXT");
	bindings->vkCmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)getDeviceProcAddr(device, "vkCmdEndQueryIndexedEXT");
	bindings->vkCmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)getDeviceProcAddr(device, "vkCmdEndTransformFeedbackEXT");
#endif // VK_EXT_transform_feedback

#if (defined(VK_EXT_validation_cache))
	bindings->vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)getDeviceProcAddr(device, "vkCreateValidationCacheEXT");
	bindings->vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)getDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
	bindings->vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)getDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
	bindings->vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)getDeviceProcAddr(device, "vkMergeValidationCachesEXT");
#endif // VK_EXT_validation_cache

#if (defined(VK_GOOGLE_display_timing))
	bindings->vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)getDeviceProcAddr(device, "vkGetPastPresentationTimingGOOGLE");
	bindings->vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)getDeviceProcAddr(device, "vkGetRefreshCycleDurationGOOGLE");
#endif // VK_GOOGLE_display_timing

#if (defined(VK_INTEL_performance_query))
	bindings->vkAcquirePerformanceConfigurationINTEL = (PFN_vkAcquirePerformanceConfigurationINTEL)getDeviceProcAddr(device, "vkAcquirePerformanceConfigurationINTEL");
	bindings->vkCmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)getDeviceProcAddr(device, "vkCmdSetPerformanceMarkerINTEL");
	bindings->vkCmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)getDeviceProcAddr(device, "vkCmdSetPerformanceOverrideINTEL");
	bindings->vkCmdSetPerformanceStreamMarkerINTEL = (PFN_vkCmdSetPerformanceStreamMarkerINTEL)getDeviceProcAddr(device, "vkCmdSetPerformanceStreamMarkerINTEL");
	bindings->vkGetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)getDeviceProcAddr(device, "vkGetPerformanceParameterINTEL");
	bindings->vkInitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)getDeviceProcAddr(device, "vkInitializePerformanceApiINTEL");
	bindings->vkQueueSetPerformanceConfigurationINTEL = (PFN_vkQueueSetPerformanceConfigurationINTEL)getDeviceProcAddr(device, "vkQueueSetPerformanceConfigurationINTEL");
	bindings->vkReleasePerformanceConfigurationINTEL = (PFN_vkReleasePerformanceConfigurationINTEL)getDeviceProcAddr(device, "vkReleasePerformanceConfigurationINTEL");
	bindings->vkUninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)getDeviceProcAddr(device, "vkUninitializePerformanceApiINTEL");
#endif // VK_INTEL_performance_query

#if (defined(VK_KHR_bind_memory2))
	bindings->vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)getDeviceProcAddr(device, "vkBindBufferMemory2KHR");
	bindings->vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)getDeviceProcAddr(device, "vkBindImageMemory2KHR");
#endif // VK_KHR_bind_memory2

#if (defined(VK_KHR_buffer_device_address))
	bindings->vkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)getDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
	bindings->vkGetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR)getDeviceProcAddr(device, "vkGetBufferOpaqueCaptureAddressKHR");
	bindings->vkGetDeviceMemoryOpaqueCaptureAddressKHR = (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)getDeviceProcAddr(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
#endif // VK_KHR_buffer_device_address

#if (defined(VK_KHR_create_renderpass2))
	bindings->vkCmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)getDeviceProcAddr(device, "vkCmdBeginRenderPass2KHR");
	bindings->vkCmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)getDeviceProcAddr(device, "vkCmdEndRenderPass2KHR");
	bindings->vkCmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)getDeviceProcAddr(device, "vkCmdNextSubpass2KHR");
	bindings->vkCreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)getDeviceProcAddr(device, "vkCreateRenderPass2KHR");
#endif // VK_KHR_create_renderpass2

#if (defined(VK_KHR_descriptor_update_template) || defined(VK_KHR_push_descriptor))
	bindings->vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)getDeviceProcAddr(device, "vkCmdPushDescriptorSetWithTemplateKHR");
#endif // VK_KHR_descriptor_update_template,VK_KHR_push_descriptor

#if (defined(VK_KHR_descriptor_update_template))
	bindings->vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)getDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplateKHR");
	bindings->vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)getDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplateKHR");
	bindings->vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)getDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplateKHR");
#endif // VK_KHR_descriptor_update_template

#if (defined(VK_KHR_device_group))
	bindings->vkCmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)getDeviceProcAddr(device, "vkCmdDispatchBaseKHR");
	bindings->vkCmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)getDeviceProcAddr(device, "vkCmdSetDeviceMaskKHR");
	bindings->vkGetDeviceGroupPeerMemoryFeaturesKHR = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)getDeviceProcAddr(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
#endif // VK_KHR_device_group

#if (defined(VK_KHR_display_swapchain))
	bindings->vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)getDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
#endif // VK_KHR_display_swapchain

#if (defined(VK_KHR_draw_indirect_count))
	bindings->vkCmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)getDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCountKHR");
	bindings->vkCmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)getDeviceProcAddr(device, "vkCmdDrawIndirectCountKHR");
#endif // VK_KHR_draw_indirect_count

#if (defined(VK_KHR_external_fence_fd))
	bindings->vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)getDeviceProcAddr(device, "vkGetFenceFdKHR");
	bindings->vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)getDeviceProcAddr(device, "vkImportFenceFdKHR");
#endif // VK_KHR_external_fence_fd

#if (defined(VK_KHR_external_fence_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)getDeviceProcAddr(device, "vkGetFenceWin32HandleKHR");
	bindings->vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)getDeviceProcAddr(device, "vkImportFenceWin32HandleKHR");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_fence_win32

#if (defined(VK_KHR_external_memory_fd))
	bindings->vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)getDeviceProcAddr(device, "vkGetMemoryFdKHR");
	bindings->vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)getDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
#endif // VK_KHR_external_memory_fd

#if (defined(VK_KHR_external_memory_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)getDeviceProcAddr(device, "vkGetMemoryWin32HandleKHR");
	bindings->vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)getDeviceProcAddr(device, "vkGetMemoryWin32HandlePropertiesKHR");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_memory_win32

#if (defined(VK_KHR_external_semaphore_fd))
	bindings->vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)getDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
	bindings->vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)getDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
#endif // VK_KHR_external_semaphore_fd

#if (defined(VK_KHR_external_semaphore_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)getDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");
	bindings->vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)getDeviceProcAddr(device, "vkImportSemaphoreWin32HandleKHR");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_semaphore_win32

#if (defined(VK_KHR_get_memory_requirements2))
	bindings->vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)getDeviceProcAddr(device, "vkGetBufferMemoryRequirements2KHR");
	bindings->vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)getDeviceProcAddr(device, "vkGetImageMemoryRequirements2KHR");
	bindings->vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)getDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2KHR");
#endif // VK_KHR_get_memory_requirements2

#if (defined(VK_KHR_maintenance1))
	bindings->vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)getDeviceProcAddr(device, "vkTrimCommandPoolKHR");
#endif // VK_KHR_maintenance1

#if (defined(VK_KHR_maintenance3))
	bindings->vkGetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR)getDeviceProcAddr(device, "vkGetDescriptorSetLayoutSupportKHR");
#endif // VK_KHR_maintenance3

#if (defined(VK_KHR_performance_query))
	bindings->vkAcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)getDeviceProcAddr(device, "vkAcquireProfilingLockKHR");
	bindings->vkReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)getDeviceProcAddr(device, "vkReleaseProfilingLockKHR");
#endif // VK_KHR_performance_query

#if (defined(VK_KHR_pipeline_executable_properties))
	bindings->vkGetPipelineExecutableInternalRepresentationsKHR = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)getDeviceProcAddr(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
	bindings->vkGetPipelineExecutablePropertiesKHR = (PFN_vkGetPipelineExecutablePropertiesKHR)getDeviceProcAddr(device, "vkGetPipelineExecutablePropertiesKHR");
	bindings->vkGetPipelineExecutableStatisticsKHR = (PFN_vkGetPipelineExecutableStatisticsKHR)getDeviceProcAddr(device, "vkGetPipelineExecutableStatisticsKHR");
#endif // VK_KHR_pipeline_executable_properties

#if (defined(VK_KHR_push_descriptor))
	bindings->vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)getDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
#endif // VK_KHR_push_descriptor

#if (defined(VK_KHR_sampler_ycbcr_conversion))
	bindings->vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)getDeviceProcAddr(device, "vkCreateSamplerYcbcrConversionKHR");
	bindings->vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)getDeviceProcAddr(device, "vkDestroySamplerYcbcrConversionKHR");
#endif // VK_KHR_sampler_ycbcr_conversion

#if (defined(VK_KHR_shared_presentable_image))
	bindings->vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)getDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
#endif // VK_KHR_shared_presentable_image

#if (defined(VK_KHR_swapchain) || defined(VK_KHR_device_group))
	bindings->vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)getDeviceProcAddr(device, "vkAcquireNextImage2KHR");
	bindings->vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)getDeviceProcAddr(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
	bindings->vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)getDeviceProcAddr(device, "vkGetDeviceGroupSurfacePresentModesKHR");
#endif // VK_KHR_swapchain,VK_KHR_device_group

#if (defined(VK_KHR_swapchain))
	bindings->vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)getDeviceProcAddr(device, "vkAcquireNextImageKHR");
	bindings->vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)getDeviceProcAddr(device, "vkCreateSwapchainKHR");
	bindings->vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)getDeviceProcAddr(device, "vkDestroySwapchainKHR");
	bindings->vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)getDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
	bindings->vkQueuePresentKHR = (PFN_vkQueuePresentKHR)getDeviceProcAddr(device, "vkQueuePresentKHR");
#endif // VK_KHR_swapchain

#if (defined(VK_KHR_timeline_semaphore))
	bindings->vkGetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)getDeviceProcAddr(device, "vkGetSemaphoreCounterValueKHR");
	bindings->vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)getDeviceProcAddr(device, "vkSignalSemaphoreKHR");
	bindings->vkWaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)getDeviceProcAddr(device, "vkWaitSemaphoresKHR");
#endif // VK_KHR_timeline_semaphore

#if (defined(VK_NVX_device_generated_commands))
	bindings->vkCmdProcessCommandsNVX = (PFN_vkCmdProcessCommandsNVX)getDeviceProcAddr(device, "vkCmdProcessCommandsNVX");
	bindings->vkCmdReserveSpaceForCommandsNVX = (PFN_vkCmdReserveSpaceForCommandsNVX)getDeviceProcAddr(device, "vkCmdReserveSpaceForCommandsNVX");
	bindings->vkCreateIndirectCommandsLayoutNVX = (PFN_vkCreateIndirectCommandsLayoutNVX)getDeviceProcAddr(device, "vkCreateIndirectCommandsLayoutNVX");
	bindings->vkCreateObjectTableNVX = (PFN_vkCreateObjectTableNVX)getDeviceProcAddr(device, "vkCreateObjectTableNVX");
	bindings->vkDestroyIndirectCommandsLayoutNVX = (PFN_vkDestroyIndirectCommandsLayoutNVX)getDeviceProcAddr(device, "vkDestroyIndirectCommandsLayoutNVX");
	bindings->vkDestroyObjectTableNVX = (PFN_vkDestroyObjectTableNVX)getDeviceProcAddr(device, "vkDestroyObjectTableNVX");
	bindings->vkRegisterObjectsNVX = (PFN_vkRegisterObjectsNVX)getDeviceProcAddr(device, "vkRegisterObjectsNVX");
	bindings->vkUnregisterObjectsNVX = (PFN_vkUnregisterObjectsNVX)getDeviceProcAddr(device, "vkUnregisterObjectsNVX");
#endif // VK_NVX_device_generated_commands

#if (defined(VK_NVX_image_view_handle))
	bindings->vkGetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)getDeviceProcAddr(device, "vkGetImageViewHandleNVX");
#endif // VK_NVX_image_view_handle

#if (defined(VK_NV_clip_space_w_scaling))
	bindings->vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)getDeviceProcAddr(device, "vkCmdSetViewportWScalingNV");
#endif // VK_NV_clip_space_w_scaling

#if (defined(VK_NV_device_diagnostic_checkpoints))
	bindings->vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)getDeviceProcAddr(device, "vkCmdSetCheckpointNV");
	bindings->vkGetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)getDeviceProcAddr(device, "vkGetQueueCheckpointDataNV");
#endif // VK_NV_device_diagnostic_checkpoints

#if (defined(VK_NV_external_memory_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	bindings->vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)getDeviceProcAddr(device, "vkGetMemoryWin32HandleNV");
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_NV_external_memory_win32

#if (defined(VK_NV_mesh_shader))
	bindings->vkCmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)getDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountNV");
	bindings->vkCmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)getDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectNV");
	bindings->vkCmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)getDeviceProcAddr(device, "vkCmdDrawMeshTasksNV");
#endif // VK_NV_mesh_shader

#if (defined(VK_NV_ray_tracing))
	bindings->vkBindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)getDeviceProcAddr(device, "vkBindAccelerationStructureMemoryNV");
	bindings->vkCmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)getDeviceProcAddr(device, "vkCmdBuildAccelerationStructureNV");
	bindings->vkCmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)getDeviceProcAddr(device, "vkCmdCopyAccelerationStructureNV");
	bindings->vkCmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)getDeviceProcAddr(device, "vkCmdTraceRaysNV");
	bindings->vkCmdWriteAccelerationStructuresPropertiesNV = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)getDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesNV");
	bindings->vkCompileDeferredNV = (PFN_vkCompileDeferredNV)getDeviceProcAddr(device, "vkCompileDeferredNV");
	bindings->vkCreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)getDeviceProcAddr(device, "vkCreateAccelerationStructureNV");
	bindings->vkCreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)getDeviceProcAddr(device, "vkCreateRayTracingPipelinesNV");
	bindings->vkDestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)getDeviceProcAddr(device, "vkDestroyAccelerationStructureNV");
	bindings->vkGetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)getDeviceProcAddr(device, "vkGetAccelerationStructureHandleNV");
	bindings->vkGetAccelerationStructureMemoryRequirementsNV = (PFN_vkGetAccelerationStructureMemoryRequirementsNV)getDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsNV");
	bindings->vkGetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV)getDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesNV");
#endif // VK_NV_ray_tracing

#if (defined(VK_NV_scissor_exclusive))
	bindings->vkCmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)getDeviceProcAddr(device, "vkCmdSetExclusiveScissorNV");
#endif // VK_NV_scissor_exclusive

#if (defined(VK_NV_shading_rate_image))
	bindings->vkCmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)getDeviceProcAddr(device, "vkCmdBindShadingRateImageNV");
	bindings->vkCmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)getDeviceProcAddr(device, "vkCmdSetCoarseSampleOrderNV");
	bindings->vkCmdSetViewportShadingRatePaletteNV = (PFN_vkCmdSetViewportShadingRatePaletteNV)getDeviceProcAddr(device, "vkCmdSetViewportShadingRatePaletteNV");
#endif // VK_NV_shading_rate_image

}
