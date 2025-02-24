// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See vk_bindings_generator.py for modifications

/*
\brief Vk Vulkan function pointers.
\file vk_bindings.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/* Corresponding to Vulkan registry file version #132# */

#pragma once
#ifndef VK_PROTOTYPES
#define VK_NO_PROTOTYPES 1
#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
#include "MoltenVK/vk_mvk_moltenvk.h"
#else
#include "vulkan/vulkan.h"
#endif

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);

// Vulkan function pointer table
typedef struct VkBindings_
{
	// ---- Before using Vulkan, an application must initialize it by loading the Vulkan commands, and creating a VkInstance object.

	// ---- This function table provides the functions necessary for achieving this.
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;

// ---- Functions required for MoltenVK MVKConfiguration functionality.
#ifdef VK_USE_PLATFORM_MACOS_MVK
	PFN_vkSetMoltenVKConfigurationMVK vkSetMoltenVKConfigurationMVK;
	PFN_vkGetMoltenVKConfigurationMVK vkGetMoltenVKConfigurationMVK;
#endif

} VkBindings;


// Instance function pointers
typedef struct VkInstanceBindings_ {
	// Manually add in vkGetPhysicalDeviceProcAddr entry
	PFN_GetPhysicalDeviceProcAddr vkGetPhysicalDeviceProcAddr;

#if (defined(VK_VERSION_1_0))
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
#endif // VK_VERSION_1_0

#if (defined(VK_VERSION_1_1))
	PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
	PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
	PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
	PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;
	PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
	PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
	PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
	PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
	PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
#endif // VK_VERSION_1_1

#if (defined(VK_EXT_acquire_xlib_display))
#if (defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT))
	PFN_vkAcquireXlibDisplayEXT vkAcquireXlibDisplayEXT;
	PFN_vkGetRandROutputDisplayEXT vkGetRandROutputDisplayEXT;
#endif // (VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // VK_EXT_acquire_xlib_display

#if (defined(VK_EXT_calibrated_timestamps))
	PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT;
#endif // VK_EXT_calibrated_timestamps

#if (defined(VK_EXT_debug_report))
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
#endif // VK_EXT_debug_report

#if (defined(VK_EXT_debug_utils))
	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
	PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
	PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT;
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT;
	PFN_vkSubmitDebugUtilsMessageEXT vkSubmitDebugUtilsMessageEXT;
#endif // VK_EXT_debug_utils

#if (defined(VK_EXT_direct_mode_display))
	PFN_vkReleaseDisplayEXT vkReleaseDisplayEXT;
#endif // VK_EXT_direct_mode_display

#if (defined(VK_EXT_display_surface_counter))
	PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT vkGetPhysicalDeviceSurfaceCapabilities2EXT;
#endif // VK_EXT_display_surface_counter

#if (defined(VK_EXT_full_screen_exclusive))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT vkGetPhysicalDeviceSurfacePresentModes2EXT;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_EXT_full_screen_exclusive

#if (defined(VK_EXT_headless_surface))
	PFN_vkCreateHeadlessSurfaceEXT vkCreateHeadlessSurfaceEXT;
#endif // VK_EXT_headless_surface

#if (defined(VK_EXT_metal_surface))
#if (defined(VK_USE_PLATFORM_METAL_EXT))
	PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
#endif // (VK_USE_PLATFORM_METAL_EXT)
#endif // VK_EXT_metal_surface

#if (defined(VK_EXT_sample_locations))
	PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT vkGetPhysicalDeviceMultisamplePropertiesEXT;
#endif // VK_EXT_sample_locations

#if (defined(VK_EXT_tooling_info))
	PFN_vkGetPhysicalDeviceToolPropertiesEXT vkGetPhysicalDeviceToolPropertiesEXT;
#endif // VK_EXT_tooling_info

#if (defined(VK_FUCHSIA_imagepipe_surface))
#if (defined(VK_USE_PLATFORM_FUCHSIA))
	PFN_vkCreateImagePipeSurfaceFUCHSIA vkCreateImagePipeSurfaceFUCHSIA;
#endif // (VK_USE_PLATFORM_FUCHSIA)
#endif // VK_FUCHSIA_imagepipe_surface

#if (defined(VK_GGP_stream_descriptor_surface))
#if (defined(VK_USE_PLATFORM_GGP))
	PFN_vkCreateStreamDescriptorSurfaceGGP vkCreateStreamDescriptorSurfaceGGP;
#endif // (VK_USE_PLATFORM_GGP)
#endif // VK_GGP_stream_descriptor_surface

#if (defined(VK_KHR_android_surface))
#if (defined(VK_USE_PLATFORM_ANDROID_KHR))
	PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#endif // (VK_USE_PLATFORM_ANDROID_KHR)
#endif // VK_KHR_android_surface

#if (defined(VK_KHR_device_group_creation))
	PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR;
#endif // VK_KHR_device_group_creation

#if (defined(VK_KHR_display))
	PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
	PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;
	PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
	PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
	PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
	PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
	PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
#endif // VK_KHR_display

#if (defined(VK_KHR_external_fence_capabilities))
	PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR vkGetPhysicalDeviceExternalFencePropertiesKHR;
#endif // VK_KHR_external_fence_capabilities

#if (defined(VK_KHR_external_memory_capabilities))
	PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR vkGetPhysicalDeviceExternalBufferPropertiesKHR;
#endif // VK_KHR_external_memory_capabilities

#if (defined(VK_KHR_external_semaphore_capabilities))
	PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR vkGetPhysicalDeviceExternalSemaphorePropertiesKHR;
#endif // VK_KHR_external_semaphore_capabilities

#if (defined(VK_KHR_get_display_properties2))
	PFN_vkGetDisplayModeProperties2KHR vkGetDisplayModeProperties2KHR;
	PFN_vkGetDisplayPlaneCapabilities2KHR vkGetDisplayPlaneCapabilities2KHR;
	PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR vkGetPhysicalDeviceDisplayPlaneProperties2KHR;
	PFN_vkGetPhysicalDeviceDisplayProperties2KHR vkGetPhysicalDeviceDisplayProperties2KHR;
#endif // VK_KHR_get_display_properties2

#if (defined(VK_KHR_get_physical_device_properties2))
	PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR;
	PFN_vkGetPhysicalDeviceFormatProperties2KHR vkGetPhysicalDeviceFormatProperties2KHR;
	PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR;
	PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR vkGetPhysicalDeviceQueueFamilyProperties2KHR;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR vkGetPhysicalDeviceSparseImageFormatProperties2KHR;
#endif // VK_KHR_get_physical_device_properties2

#if (defined(VK_KHR_get_surface_capabilities2))
	PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR;
	PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR;
#endif // VK_KHR_get_surface_capabilities2

#if (defined(VK_KHR_performance_query))
	PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
	PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
#endif // VK_KHR_performance_query

#if (defined(VK_KHR_surface))
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
#endif // VK_KHR_surface

#if (defined(VK_KHR_swapchain) || defined(VK_KHR_device_group))
	PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR;
#endif // VK_KHR_swapchain,VK_KHR_device_group

#if (defined(VK_KHR_wayland_surface))
#if (defined(VK_USE_PLATFORM_WAYLAND_KHR))
	PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
	PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // (VK_USE_PLATFORM_WAYLAND_KHR)
#endif // VK_KHR_wayland_surface

#if (defined(VK_KHR_win32_surface))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_win32_surface

#if (defined(VK_KHR_xcb_surface))
#if (defined(VK_USE_PLATFORM_XCB_KHR))
	PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
	PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif // (VK_USE_PLATFORM_XCB_KHR)
#endif // VK_KHR_xcb_surface

#if (defined(VK_KHR_xlib_surface))
#if (defined(VK_USE_PLATFORM_XLIB_KHR))
	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
	PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif // (VK_USE_PLATFORM_XLIB_KHR)
#endif // VK_KHR_xlib_surface

#if (defined(VK_MVK_ios_surface))
#if (defined(VK_USE_PLATFORM_IOS_MVK))
	PFN_vkCreateIOSSurfaceMVK vkCreateIOSSurfaceMVK;
#endif // (VK_USE_PLATFORM_IOS_MVK)
#endif // VK_MVK_ios_surface

#if (defined(VK_MVK_macos_surface))
#if (defined(VK_USE_PLATFORM_MACOS_MVK))
	PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;
#endif // (VK_USE_PLATFORM_MACOS_MVK)
#endif // VK_MVK_macos_surface

#if (defined(VK_NN_vi_surface))
#if (defined(VK_USE_PLATFORM_VI_NN))
	PFN_vkCreateViSurfaceNN vkCreateViSurfaceNN;
#endif // (VK_USE_PLATFORM_VI_NN)
#endif // VK_NN_vi_surface

#if (defined(VK_NVX_device_generated_commands))
	PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX;
#endif // VK_NVX_device_generated_commands

#if (defined(VK_NV_cooperative_matrix))
	PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV vkGetPhysicalDeviceCooperativeMatrixPropertiesNV;
#endif // VK_NV_cooperative_matrix

#if (defined(VK_NV_coverage_reduction_mode))
	PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;
#endif // VK_NV_coverage_reduction_mode

#if (defined(VK_NV_external_memory_capabilities))
	PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV vkGetPhysicalDeviceExternalImageFormatPropertiesNV;
#endif // VK_NV_external_memory_capabilities



} VkInstanceBindings;
// Device function pointers
typedef struct VkDeviceBindings_ {

#if (defined(VK_VERSION_1_0))
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkCmdBeginQuery vkCmdBeginQuery;
	PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
	PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
	PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
	PFN_vkCmdBindPipeline vkCmdBindPipeline;
	PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
	PFN_vkCmdBlitImage vkCmdBlitImage;
	PFN_vkCmdClearAttachments vkCmdClearAttachments;
	PFN_vkCmdClearColorImage vkCmdClearColorImage;
	PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
	PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
	PFN_vkCmdCopyImage vkCmdCopyImage;
	PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
	PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
	PFN_vkCmdDispatch vkCmdDispatch;
	PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
	PFN_vkCmdDraw vkCmdDraw;
	PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
	PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
	PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
	PFN_vkCmdEndQuery vkCmdEndQuery;
	PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
	PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
	PFN_vkCmdFillBuffer vkCmdFillBuffer;
	PFN_vkCmdNextSubpass vkCmdNextSubpass;
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkCmdPushConstants vkCmdPushConstants;
	PFN_vkCmdResetEvent vkCmdResetEvent;
	PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
	PFN_vkCmdResolveImage vkCmdResolveImage;
	PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
	PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
	PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
	PFN_vkCmdSetEvent vkCmdSetEvent;
	PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
	PFN_vkCmdSetScissor vkCmdSetScissor;
	PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
	PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
	PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
	PFN_vkCmdSetViewport vkCmdSetViewport;
	PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
	PFN_vkCmdWaitEvents vkCmdWaitEvents;
	PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkCreateBufferView vkCreateBufferView;
	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkCreateComputePipelines vkCreateComputePipelines;
	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkCreateEvent vkCreateEvent;
	PFN_vkCreateFence vkCreateFence;
	PFN_vkCreateFramebuffer vkCreateFramebuffer;
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
	PFN_vkCreateImage vkCreateImage;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkCreatePipelineCache vkCreatePipelineCache;
	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkCreateQueryPool vkCreateQueryPool;
	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkCreateSampler vkCreateSampler;
	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkCreateShaderModule vkCreateShaderModule;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkDestroyBufferView vkDestroyBufferView;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkDestroyEvent vkDestroyEvent;
	PFN_vkDestroyFence vkDestroyFence;
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
	PFN_vkDestroyImage vkDestroyImage;
	PFN_vkDestroyImageView vkDestroyImageView;
	PFN_vkDestroyPipeline vkDestroyPipeline;
	PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
	PFN_vkDestroyQueryPool vkDestroyQueryPool;
	PFN_vkDestroyRenderPass vkDestroyRenderPass;
	PFN_vkDestroySampler vkDestroySampler;
	PFN_vkDestroySemaphore vkDestroySemaphore;
	PFN_vkDestroyShaderModule vkDestroyShaderModule;
	PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;
	PFN_vkGetEventStatus vkGetEventStatus;
	PFN_vkGetFenceStatus vkGetFenceStatus;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
	PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
	PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
	PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
	PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkMergePipelineCaches vkMergePipelineCaches;
	PFN_vkQueueBindSparse vkQueueBindSparse;
	PFN_vkQueueSubmit vkQueueSubmit;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;
	PFN_vkResetCommandBuffer vkResetCommandBuffer;
	PFN_vkResetCommandPool vkResetCommandPool;
	PFN_vkResetDescriptorPool vkResetDescriptorPool;
	PFN_vkResetEvent vkResetEvent;
	PFN_vkResetFences vkResetFences;
	PFN_vkSetEvent vkSetEvent;
	PFN_vkUnmapMemory vkUnmapMemory;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
	PFN_vkWaitForFences vkWaitForFences;
#endif // VK_VERSION_1_0

#if (defined(VK_VERSION_1_1))
	PFN_vkBindBufferMemory2 vkBindBufferMemory2;
	PFN_vkBindImageMemory2 vkBindImageMemory2;
	PFN_vkCmdDispatchBase vkCmdDispatchBase;
	PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
	PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
	PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
	PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
	PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
	PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
	PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;
	PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
	PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
	PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
	PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
	PFN_vkTrimCommandPool vkTrimCommandPool;
	PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
#endif // VK_VERSION_1_1

#if (defined(VK_VERSION_1_2))
	PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
	PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
	PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
	PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
	PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
	PFN_vkCreateRenderPass2 vkCreateRenderPass2;
	PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
	PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
	PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;
	PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
	PFN_vkResetQueryPool vkResetQueryPool;
	PFN_vkSignalSemaphore vkSignalSemaphore;
	PFN_vkWaitSemaphores vkWaitSemaphores;
#endif // VK_VERSION_1_2

#if (defined(VK_AMD_buffer_marker))
	PFN_vkCmdWriteBufferMarkerAMD vkCmdWriteBufferMarkerAMD;
#endif // VK_AMD_buffer_marker

#if (defined(VK_AMD_display_native_hdr))
	PFN_vkSetLocalDimmingAMD vkSetLocalDimmingAMD;
#endif // VK_AMD_display_native_hdr

#if (defined(VK_AMD_draw_indirect_count))
	PFN_vkCmdDrawIndexedIndirectCountAMD vkCmdDrawIndexedIndirectCountAMD;
	PFN_vkCmdDrawIndirectCountAMD vkCmdDrawIndirectCountAMD;
#endif // VK_AMD_draw_indirect_count

#if (defined(VK_AMD_shader_info))
	PFN_vkGetShaderInfoAMD vkGetShaderInfoAMD;
#endif // VK_AMD_shader_info

#if (defined(VK_ANDROID_external_memory_android_hardware_buffer))
#if (defined(VK_USE_PLATFORM_ANDROID_KHR))
	PFN_vkGetAndroidHardwareBufferPropertiesANDROID vkGetAndroidHardwareBufferPropertiesANDROID;
	PFN_vkGetMemoryAndroidHardwareBufferANDROID vkGetMemoryAndroidHardwareBufferANDROID;
#endif // (VK_USE_PLATFORM_ANDROID_KHR)
#endif // VK_ANDROID_external_memory_android_hardware_buffer

#if (defined(VK_EXT_buffer_device_address))
	PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT;
#endif // VK_EXT_buffer_device_address

#if (defined(VK_EXT_calibrated_timestamps))
	PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT;
#endif // VK_EXT_calibrated_timestamps

#if (defined(VK_EXT_conditional_rendering))
	PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT;
	PFN_vkCmdEndConditionalRenderingEXT vkCmdEndConditionalRenderingEXT;
#endif // VK_EXT_conditional_rendering

#if (defined(VK_EXT_debug_marker))
	PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT;
	PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT;
	PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT;
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
	PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT;
#endif // VK_EXT_debug_marker

#if (defined(VK_EXT_discard_rectangles))
	PFN_vkCmdSetDiscardRectangleEXT vkCmdSetDiscardRectangleEXT;
#endif // VK_EXT_discard_rectangles

#if (defined(VK_EXT_display_control))
	PFN_vkDisplayPowerControlEXT vkDisplayPowerControlEXT;
	PFN_vkGetSwapchainCounterEXT vkGetSwapchainCounterEXT;
	PFN_vkRegisterDeviceEventEXT vkRegisterDeviceEventEXT;
	PFN_vkRegisterDisplayEventEXT vkRegisterDisplayEventEXT;
#endif // VK_EXT_display_control

#if (defined(VK_EXT_external_memory_host))
	PFN_vkGetMemoryHostPointerPropertiesEXT vkGetMemoryHostPointerPropertiesEXT;
#endif // VK_EXT_external_memory_host

#if (defined(VK_EXT_full_screen_exclusive))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkAcquireFullScreenExclusiveModeEXT vkAcquireFullScreenExclusiveModeEXT;
	PFN_vkGetDeviceGroupSurfacePresentModes2EXT vkGetDeviceGroupSurfacePresentModes2EXT;
	PFN_vkReleaseFullScreenExclusiveModeEXT vkReleaseFullScreenExclusiveModeEXT;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_EXT_full_screen_exclusive

#if (defined(VK_EXT_hdr_metadata))
	PFN_vkSetHdrMetadataEXT vkSetHdrMetadataEXT;
#endif // VK_EXT_hdr_metadata

#if (defined(VK_EXT_host_query_reset))
	PFN_vkResetQueryPoolEXT vkResetQueryPoolEXT;
#endif // VK_EXT_host_query_reset

#if (defined(VK_EXT_image_drm_format_modifier))
	PFN_vkGetImageDrmFormatModifierPropertiesEXT vkGetImageDrmFormatModifierPropertiesEXT;
#endif // VK_EXT_image_drm_format_modifier

#if (defined(VK_EXT_line_rasterization))
	PFN_vkCmdSetLineStippleEXT vkCmdSetLineStippleEXT;
#endif // VK_EXT_line_rasterization

#if (defined(VK_EXT_sample_locations))
	PFN_vkCmdSetSampleLocationsEXT vkCmdSetSampleLocationsEXT;
#endif // VK_EXT_sample_locations

#if (defined(VK_EXT_transform_feedback))
	PFN_vkCmdBeginQueryIndexedEXT vkCmdBeginQueryIndexedEXT;
	PFN_vkCmdBeginTransformFeedbackEXT vkCmdBeginTransformFeedbackEXT;
	PFN_vkCmdBindTransformFeedbackBuffersEXT vkCmdBindTransformFeedbackBuffersEXT;
	PFN_vkCmdDrawIndirectByteCountEXT vkCmdDrawIndirectByteCountEXT;
	PFN_vkCmdEndQueryIndexedEXT vkCmdEndQueryIndexedEXT;
	PFN_vkCmdEndTransformFeedbackEXT vkCmdEndTransformFeedbackEXT;
#endif // VK_EXT_transform_feedback

#if (defined(VK_EXT_validation_cache))
	PFN_vkCreateValidationCacheEXT vkCreateValidationCacheEXT;
	PFN_vkDestroyValidationCacheEXT vkDestroyValidationCacheEXT;
	PFN_vkGetValidationCacheDataEXT vkGetValidationCacheDataEXT;
	PFN_vkMergeValidationCachesEXT vkMergeValidationCachesEXT;
#endif // VK_EXT_validation_cache

#if (defined(VK_GOOGLE_display_timing))
	PFN_vkGetPastPresentationTimingGOOGLE vkGetPastPresentationTimingGOOGLE;
	PFN_vkGetRefreshCycleDurationGOOGLE vkGetRefreshCycleDurationGOOGLE;
#endif // VK_GOOGLE_display_timing

#if (defined(VK_INTEL_performance_query))
	PFN_vkAcquirePerformanceConfigurationINTEL vkAcquirePerformanceConfigurationINTEL;
	PFN_vkCmdSetPerformanceMarkerINTEL vkCmdSetPerformanceMarkerINTEL;
	PFN_vkCmdSetPerformanceOverrideINTEL vkCmdSetPerformanceOverrideINTEL;
	PFN_vkCmdSetPerformanceStreamMarkerINTEL vkCmdSetPerformanceStreamMarkerINTEL;
	PFN_vkGetPerformanceParameterINTEL vkGetPerformanceParameterINTEL;
	PFN_vkInitializePerformanceApiINTEL vkInitializePerformanceApiINTEL;
	PFN_vkQueueSetPerformanceConfigurationINTEL vkQueueSetPerformanceConfigurationINTEL;
	PFN_vkReleasePerformanceConfigurationINTEL vkReleasePerformanceConfigurationINTEL;
	PFN_vkUninitializePerformanceApiINTEL vkUninitializePerformanceApiINTEL;
#endif // VK_INTEL_performance_query

#if (defined(VK_KHR_bind_memory2))
	PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR;
	PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR;
#endif // VK_KHR_bind_memory2

#if (defined(VK_KHR_buffer_device_address))
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkGetBufferOpaqueCaptureAddressKHR vkGetBufferOpaqueCaptureAddressKHR;
	PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR;
#endif // VK_KHR_buffer_device_address

#if (defined(VK_KHR_create_renderpass2))
	PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR;
	PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR;
	PFN_vkCmdNextSubpass2KHR vkCmdNextSubpass2KHR;
	PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR;
#endif // VK_KHR_create_renderpass2

#if (defined(VK_KHR_descriptor_update_template) || defined(VK_KHR_push_descriptor))
	PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR;
#endif // VK_KHR_descriptor_update_template,VK_KHR_push_descriptor

#if (defined(VK_KHR_descriptor_update_template))
	PFN_vkCreateDescriptorUpdateTemplateKHR vkCreateDescriptorUpdateTemplateKHR;
	PFN_vkDestroyDescriptorUpdateTemplateKHR vkDestroyDescriptorUpdateTemplateKHR;
	PFN_vkUpdateDescriptorSetWithTemplateKHR vkUpdateDescriptorSetWithTemplateKHR;
#endif // VK_KHR_descriptor_update_template

#if (defined(VK_KHR_device_group))
	PFN_vkCmdDispatchBaseKHR vkCmdDispatchBaseKHR;
	PFN_vkCmdSetDeviceMaskKHR vkCmdSetDeviceMaskKHR;
	PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR vkGetDeviceGroupPeerMemoryFeaturesKHR;
#endif // VK_KHR_device_group

#if (defined(VK_KHR_display_swapchain))
	PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;
#endif // VK_KHR_display_swapchain

#if (defined(VK_KHR_draw_indirect_count))
	PFN_vkCmdDrawIndexedIndirectCountKHR vkCmdDrawIndexedIndirectCountKHR;
	PFN_vkCmdDrawIndirectCountKHR vkCmdDrawIndirectCountKHR;
#endif // VK_KHR_draw_indirect_count

#if (defined(VK_KHR_external_fence_fd))
	PFN_vkGetFenceFdKHR vkGetFenceFdKHR;
	PFN_vkImportFenceFdKHR vkImportFenceFdKHR;
#endif // VK_KHR_external_fence_fd

#if (defined(VK_KHR_external_fence_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkGetFenceWin32HandleKHR vkGetFenceWin32HandleKHR;
	PFN_vkImportFenceWin32HandleKHR vkImportFenceWin32HandleKHR;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_fence_win32

#if (defined(VK_KHR_external_memory_fd))
	PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR;
	PFN_vkGetMemoryFdPropertiesKHR vkGetMemoryFdPropertiesKHR;
#endif // VK_KHR_external_memory_fd

#if (defined(VK_KHR_external_memory_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR;
	PFN_vkGetMemoryWin32HandlePropertiesKHR vkGetMemoryWin32HandlePropertiesKHR;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_memory_win32

#if (defined(VK_KHR_external_semaphore_fd))
	PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR;
	PFN_vkImportSemaphoreFdKHR vkImportSemaphoreFdKHR;
#endif // VK_KHR_external_semaphore_fd

#if (defined(VK_KHR_external_semaphore_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkGetSemaphoreWin32HandleKHR vkGetSemaphoreWin32HandleKHR;
	PFN_vkImportSemaphoreWin32HandleKHR vkImportSemaphoreWin32HandleKHR;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_KHR_external_semaphore_win32

#if (defined(VK_KHR_get_memory_requirements2))
	PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR;
	PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR;
	PFN_vkGetImageSparseMemoryRequirements2KHR vkGetImageSparseMemoryRequirements2KHR;
#endif // VK_KHR_get_memory_requirements2

#if (defined(VK_KHR_maintenance1))
	PFN_vkTrimCommandPoolKHR vkTrimCommandPoolKHR;
#endif // VK_KHR_maintenance1

#if (defined(VK_KHR_maintenance3))
	PFN_vkGetDescriptorSetLayoutSupportKHR vkGetDescriptorSetLayoutSupportKHR;
#endif // VK_KHR_maintenance3

#if (defined(VK_KHR_performance_query))
	PFN_vkAcquireProfilingLockKHR vkAcquireProfilingLockKHR;
	PFN_vkReleaseProfilingLockKHR vkReleaseProfilingLockKHR;
#endif // VK_KHR_performance_query

#if (defined(VK_KHR_pipeline_executable_properties))
	PFN_vkGetPipelineExecutableInternalRepresentationsKHR vkGetPipelineExecutableInternalRepresentationsKHR;
	PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR;
	PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR;
#endif // VK_KHR_pipeline_executable_properties

#if (defined(VK_KHR_push_descriptor))
	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR;
#endif // VK_KHR_push_descriptor

#if (defined(VK_KHR_sampler_ycbcr_conversion))
	PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionKHR;
	PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionKHR;
#endif // VK_KHR_sampler_ycbcr_conversion

#if (defined(VK_KHR_shared_presentable_image))
	PFN_vkGetSwapchainStatusKHR vkGetSwapchainStatusKHR;
#endif // VK_KHR_shared_presentable_image

#if (defined(VK_KHR_swapchain) || defined(VK_KHR_device_group))
	PFN_vkAcquireNextImage2KHR vkAcquireNextImage2KHR;
	PFN_vkGetDeviceGroupPresentCapabilitiesKHR vkGetDeviceGroupPresentCapabilitiesKHR;
	PFN_vkGetDeviceGroupSurfacePresentModesKHR vkGetDeviceGroupSurfacePresentModesKHR;
#endif // VK_KHR_swapchain,VK_KHR_device_group

#if (defined(VK_KHR_swapchain))
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;
#endif // VK_KHR_swapchain

#if (defined(VK_KHR_timeline_semaphore))
	PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR;
	PFN_vkSignalSemaphoreKHR vkSignalSemaphoreKHR;
	PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;
#endif // VK_KHR_timeline_semaphore

#if (defined(VK_NVX_device_generated_commands))
	PFN_vkCmdProcessCommandsNVX vkCmdProcessCommandsNVX;
	PFN_vkCmdReserveSpaceForCommandsNVX vkCmdReserveSpaceForCommandsNVX;
	PFN_vkCreateIndirectCommandsLayoutNVX vkCreateIndirectCommandsLayoutNVX;
	PFN_vkCreateObjectTableNVX vkCreateObjectTableNVX;
	PFN_vkDestroyIndirectCommandsLayoutNVX vkDestroyIndirectCommandsLayoutNVX;
	PFN_vkDestroyObjectTableNVX vkDestroyObjectTableNVX;
	PFN_vkRegisterObjectsNVX vkRegisterObjectsNVX;
	PFN_vkUnregisterObjectsNVX vkUnregisterObjectsNVX;
#endif // VK_NVX_device_generated_commands

#if (defined(VK_NVX_image_view_handle))
	PFN_vkGetImageViewHandleNVX vkGetImageViewHandleNVX;
#endif // VK_NVX_image_view_handle

#if (defined(VK_NV_clip_space_w_scaling))
	PFN_vkCmdSetViewportWScalingNV vkCmdSetViewportWScalingNV;
#endif // VK_NV_clip_space_w_scaling

#if (defined(VK_NV_device_diagnostic_checkpoints))
	PFN_vkCmdSetCheckpointNV vkCmdSetCheckpointNV;
	PFN_vkGetQueueCheckpointDataNV vkGetQueueCheckpointDataNV;
#endif // VK_NV_device_diagnostic_checkpoints

#if (defined(VK_NV_external_memory_win32))
#if (defined(VK_USE_PLATFORM_WIN32_KHR))
	PFN_vkGetMemoryWin32HandleNV vkGetMemoryWin32HandleNV;
#endif // (VK_USE_PLATFORM_WIN32_KHR)
#endif // VK_NV_external_memory_win32

#if (defined(VK_NV_mesh_shader))
	PFN_vkCmdDrawMeshTasksIndirectCountNV vkCmdDrawMeshTasksIndirectCountNV;
	PFN_vkCmdDrawMeshTasksIndirectNV vkCmdDrawMeshTasksIndirectNV;
	PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV;
#endif // VK_NV_mesh_shader

#if (defined(VK_NV_ray_tracing))
	PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV;
	PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV;
	PFN_vkCmdCopyAccelerationStructureNV vkCmdCopyAccelerationStructureNV;
	PFN_vkCmdTraceRaysNV vkCmdTraceRaysNV;
	PFN_vkCmdWriteAccelerationStructuresPropertiesNV vkCmdWriteAccelerationStructuresPropertiesNV;
	PFN_vkCompileDeferredNV vkCompileDeferredNV;
	PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV;
	PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV;
	PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV;
	PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV;
	PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV;
	PFN_vkGetRayTracingShaderGroupHandlesNV vkGetRayTracingShaderGroupHandlesNV;
#endif // VK_NV_ray_tracing

#if (defined(VK_NV_scissor_exclusive))
	PFN_vkCmdSetExclusiveScissorNV vkCmdSetExclusiveScissorNV;
#endif // VK_NV_scissor_exclusive

#if (defined(VK_NV_shading_rate_image))
	PFN_vkCmdBindShadingRateImageNV vkCmdBindShadingRateImageNV;
	PFN_vkCmdSetCoarseSampleOrderNV vkCmdSetCoarseSampleOrderNV;
	PFN_vkCmdSetViewportShadingRatePaletteNV vkCmdSetViewportShadingRatePaletteNV;
#endif // VK_NV_shading_rate_image

} VkDeviceBindings;

