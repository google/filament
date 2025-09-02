// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See loader_extension_generator.py for modifications

/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * Copyright (c) 2023-2023 RasterGrid Kft.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

// clang-format off
#pragma once

#include <vulkan/vulkan.h>

#if !defined(PFN_GetPhysicalDeviceProcAddr)
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);
#endif

// Instance function pointer dispatch table
typedef struct VkLayerInstanceDispatchTable_ {
    // Manually add in GetPhysicalDeviceProcAddr entry
    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;

    // ---- Core Vulkan 1.0 commands
    PFN_vkCreateInstance CreateInstance;
    PFN_vkDestroyInstance DestroyInstance;
    PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties;
    PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties;
    PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_vkCreateDevice CreateDevice;
    PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
    PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
    PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties;
    PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties;
    PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties;

    // ---- Core Vulkan 1.1 commands
    PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion;
    PFN_vkEnumeratePhysicalDeviceGroups EnumeratePhysicalDeviceGroups;
    PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2;
    PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2;
    PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysicalDeviceFormatProperties2;
    PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysicalDeviceImageFormatProperties2;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysicalDeviceQueueFamilyProperties2;
    PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysicalDeviceMemoryProperties2;
    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysicalDeviceSparseImageFormatProperties2;
    PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties;
    PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties;
    PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties;

    // ---- Core Vulkan 1.3 commands
    PFN_vkGetPhysicalDeviceToolProperties GetPhysicalDeviceToolProperties;

    // ---- VK_KHR_surface extension commands
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;

    // ---- VK_KHR_swapchain extension commands
    PFN_vkGetPhysicalDevicePresentRectanglesKHR GetPhysicalDevicePresentRectanglesKHR;

    // ---- VK_KHR_display extension commands
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR;
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR;
    PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR;
    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR;
    PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR;
    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR;
    PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR;

    // ---- VK_KHR_xlib_surface extension commands
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR

    // ---- VK_KHR_xcb_surface extension commands
#if defined(VK_USE_PLATFORM_XCB_KHR)
    PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR;
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XCB_KHR

    // ---- VK_KHR_wayland_surface extension commands
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR

    // ---- VK_KHR_android_surface extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR;
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_KHR_win32_surface extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR GetPhysicalDeviceWin32PresentationSupportKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_video_queue extension commands
    PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR GetPhysicalDeviceVideoCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR GetPhysicalDeviceVideoFormatPropertiesKHR;

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysicalDeviceFeatures2KHR;
    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysicalDeviceProperties2KHR;
    PFN_vkGetPhysicalDeviceFormatProperties2KHR GetPhysicalDeviceFormatProperties2KHR;
    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR GetPhysicalDeviceImageFormatProperties2KHR;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR GetPhysicalDeviceQueueFamilyProperties2KHR;
    PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysicalDeviceMemoryProperties2KHR;
    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysicalDeviceSparseImageFormatProperties2KHR;

    // ---- VK_KHR_device_group_creation extension commands
    PFN_vkEnumeratePhysicalDeviceGroupsKHR EnumeratePhysicalDeviceGroupsKHR;

    // ---- VK_KHR_external_memory_capabilities extension commands
    PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR GetPhysicalDeviceExternalBufferPropertiesKHR;

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR GetPhysicalDeviceExternalSemaphorePropertiesKHR;

    // ---- VK_KHR_external_fence_capabilities extension commands
    PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR GetPhysicalDeviceExternalFencePropertiesKHR;

    // ---- VK_KHR_performance_query extension commands
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;

    // ---- VK_KHR_get_surface_capabilities2 extension commands
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR GetPhysicalDeviceSurfaceCapabilities2KHR;
    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR GetPhysicalDeviceSurfaceFormats2KHR;

    // ---- VK_KHR_get_display_properties2 extension commands
    PFN_vkGetPhysicalDeviceDisplayProperties2KHR GetPhysicalDeviceDisplayProperties2KHR;
    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR GetPhysicalDeviceDisplayPlaneProperties2KHR;
    PFN_vkGetDisplayModeProperties2KHR GetDisplayModeProperties2KHR;
    PFN_vkGetDisplayPlaneCapabilities2KHR GetDisplayPlaneCapabilities2KHR;

    // ---- VK_KHR_fragment_shading_rate extension commands
    PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR GetPhysicalDeviceFragmentShadingRatesKHR;

    // ---- VK_KHR_video_encode_queue extension commands
    PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;

    // ---- VK_KHR_cooperative_matrix extension commands
    PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR GetPhysicalDeviceCooperativeMatrixPropertiesKHR;

    // ---- VK_KHR_calibrated_timestamps extension commands
    PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR GetPhysicalDeviceCalibrateableTimeDomainsKHR;

    // ---- VK_EXT_debug_report extension commands
    PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT DebugReportMessageEXT;

    // ---- VK_GGP_stream_descriptor_surface extension commands
#if defined(VK_USE_PLATFORM_GGP)
    PFN_vkCreateStreamDescriptorSurfaceGGP CreateStreamDescriptorSurfaceGGP;
#endif // VK_USE_PLATFORM_GGP

    // ---- VK_NV_external_memory_capabilities extension commands
    PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV GetPhysicalDeviceExternalImageFormatPropertiesNV;

    // ---- VK_NN_vi_surface extension commands
#if defined(VK_USE_PLATFORM_VI_NN)
    PFN_vkCreateViSurfaceNN CreateViSurfaceNN;
#endif // VK_USE_PLATFORM_VI_NN

    // ---- VK_EXT_direct_mode_display extension commands
    PFN_vkReleaseDisplayEXT ReleaseDisplayEXT;

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    PFN_vkAcquireXlibDisplayEXT AcquireXlibDisplayEXT;
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    PFN_vkGetRandROutputDisplayEXT GetRandROutputDisplayEXT;
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT GetPhysicalDeviceSurfaceCapabilities2EXT;

    // ---- VK_MVK_ios_surface extension commands
#if defined(VK_USE_PLATFORM_IOS_MVK)
    PFN_vkCreateIOSSurfaceMVK CreateIOSSurfaceMVK;
#endif // VK_USE_PLATFORM_IOS_MVK

    // ---- VK_MVK_macos_surface extension commands
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    PFN_vkCreateMacOSSurfaceMVK CreateMacOSSurfaceMVK;
#endif // VK_USE_PLATFORM_MACOS_MVK

    // ---- VK_EXT_debug_utils extension commands
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
    PFN_vkSubmitDebugUtilsMessageEXT SubmitDebugUtilsMessageEXT;

    // ---- VK_EXT_sample_locations extension commands
    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT GetPhysicalDeviceMultisamplePropertiesEXT;

    // ---- VK_EXT_calibrated_timestamps extension commands
    PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT GetPhysicalDeviceCalibrateableTimeDomainsEXT;

    // ---- VK_FUCHSIA_imagepipe_surface extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkCreateImagePipeSurfaceFUCHSIA CreateImagePipeSurfaceFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_EXT_metal_surface extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    PFN_vkCreateMetalSurfaceEXT CreateMetalSurfaceEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_tooling_info extension commands
    PFN_vkGetPhysicalDeviceToolPropertiesEXT GetPhysicalDeviceToolPropertiesEXT;

    // ---- VK_NV_cooperative_matrix extension commands
    PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV GetPhysicalDeviceCooperativeMatrixPropertiesNV;

    // ---- VK_NV_coverage_reduction_mode extension commands
    PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT GetPhysicalDeviceSurfacePresentModes2EXT;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_headless_surface extension commands
    PFN_vkCreateHeadlessSurfaceEXT CreateHeadlessSurfaceEXT;

    // ---- VK_EXT_acquire_drm_display extension commands
    PFN_vkAcquireDrmDisplayEXT AcquireDrmDisplayEXT;
    PFN_vkGetDrmDisplayEXT GetDrmDisplayEXT;

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkAcquireWinrtDisplayNV AcquireWinrtDisplayNV;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetWinrtDisplayNV GetWinrtDisplayNV;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_directfb_surface extension commands
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    PFN_vkCreateDirectFBSurfaceEXT CreateDirectFBSurfaceEXT;
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT GetPhysicalDeviceDirectFBPresentationSupportEXT;
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

    // ---- VK_QNX_screen_surface extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    PFN_vkCreateScreenSurfaceQNX CreateScreenSurfaceQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX GetPhysicalDeviceScreenPresentationSupportQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_ARM_tensors extension commands
    PFN_vkGetPhysicalDeviceExternalTensorPropertiesARM GetPhysicalDeviceExternalTensorPropertiesARM;

    // ---- VK_NV_optical_flow extension commands
    PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV GetPhysicalDeviceOpticalFlowImageFormatsNV;

    // ---- VK_NV_cooperative_vector extension commands
    PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV GetPhysicalDeviceCooperativeVectorPropertiesNV;

    // ---- VK_ARM_data_graph extension commands
    PFN_vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM;
    PFN_vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM GetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM;

    // ---- VK_OHOS_surface extension commands
#if defined(VK_USE_PLATFORM_OHOS)
    PFN_vkCreateSurfaceOHOS CreateSurfaceOHOS;
#endif // VK_USE_PLATFORM_OHOS

    // ---- VK_NV_cooperative_matrix2 extension commands
    PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV;
} VkLayerInstanceDispatchTable;

// Device function pointer dispatch table
#define DEVICE_DISP_TABLE_MAGIC_NUMBER 0x10ADED040410ADEDUL
typedef struct VkLayerDispatchTable_ {
    uint64_t magic; // Should be DEVICE_DISP_TABLE_MAGIC_NUMBER

    // ---- Core Vulkan 1.0 commands
    PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
    PFN_vkDestroyDevice DestroyDevice;
    PFN_vkGetDeviceQueue GetDeviceQueue;
    PFN_vkQueueSubmit QueueSubmit;
    PFN_vkQueueWaitIdle QueueWaitIdle;
    PFN_vkDeviceWaitIdle DeviceWaitIdle;
    PFN_vkAllocateMemory AllocateMemory;
    PFN_vkFreeMemory FreeMemory;
    PFN_vkMapMemory MapMemory;
    PFN_vkUnmapMemory UnmapMemory;
    PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges;
    PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges;
    PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment;
    PFN_vkBindBufferMemory BindBufferMemory;
    PFN_vkBindImageMemory BindImageMemory;
    PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
    PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements;
    PFN_vkQueueBindSparse QueueBindSparse;
    PFN_vkCreateFence CreateFence;
    PFN_vkDestroyFence DestroyFence;
    PFN_vkResetFences ResetFences;
    PFN_vkGetFenceStatus GetFenceStatus;
    PFN_vkWaitForFences WaitForFences;
    PFN_vkCreateSemaphore CreateSemaphore;
    PFN_vkDestroySemaphore DestroySemaphore;
    PFN_vkCreateEvent CreateEvent;
    PFN_vkDestroyEvent DestroyEvent;
    PFN_vkGetEventStatus GetEventStatus;
    PFN_vkSetEvent SetEvent;
    PFN_vkResetEvent ResetEvent;
    PFN_vkCreateQueryPool CreateQueryPool;
    PFN_vkDestroyQueryPool DestroyQueryPool;
    PFN_vkGetQueryPoolResults GetQueryPoolResults;
    PFN_vkCreateBuffer CreateBuffer;
    PFN_vkDestroyBuffer DestroyBuffer;
    PFN_vkCreateBufferView CreateBufferView;
    PFN_vkDestroyBufferView DestroyBufferView;
    PFN_vkCreateImage CreateImage;
    PFN_vkDestroyImage DestroyImage;
    PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
    PFN_vkCreateImageView CreateImageView;
    PFN_vkDestroyImageView DestroyImageView;
    PFN_vkCreateShaderModule CreateShaderModule;
    PFN_vkDestroyShaderModule DestroyShaderModule;
    PFN_vkCreatePipelineCache CreatePipelineCache;
    PFN_vkDestroyPipelineCache DestroyPipelineCache;
    PFN_vkGetPipelineCacheData GetPipelineCacheData;
    PFN_vkMergePipelineCaches MergePipelineCaches;
    PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
    PFN_vkCreateComputePipelines CreateComputePipelines;
    PFN_vkDestroyPipeline DestroyPipeline;
    PFN_vkCreatePipelineLayout CreatePipelineLayout;
    PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
    PFN_vkCreateSampler CreateSampler;
    PFN_vkDestroySampler DestroySampler;
    PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
    PFN_vkCreateDescriptorPool CreateDescriptorPool;
    PFN_vkDestroyDescriptorPool DestroyDescriptorPool;
    PFN_vkResetDescriptorPool ResetDescriptorPool;
    PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
    PFN_vkFreeDescriptorSets FreeDescriptorSets;
    PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
    PFN_vkCreateFramebuffer CreateFramebuffer;
    PFN_vkDestroyFramebuffer DestroyFramebuffer;
    PFN_vkCreateRenderPass CreateRenderPass;
    PFN_vkDestroyRenderPass DestroyRenderPass;
    PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity;
    PFN_vkCreateCommandPool CreateCommandPool;
    PFN_vkDestroyCommandPool DestroyCommandPool;
    PFN_vkResetCommandPool ResetCommandPool;
    PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
    PFN_vkFreeCommandBuffers FreeCommandBuffers;
    PFN_vkBeginCommandBuffer BeginCommandBuffer;
    PFN_vkEndCommandBuffer EndCommandBuffer;
    PFN_vkResetCommandBuffer ResetCommandBuffer;
    PFN_vkCmdBindPipeline CmdBindPipeline;
    PFN_vkCmdSetViewport CmdSetViewport;
    PFN_vkCmdSetScissor CmdSetScissor;
    PFN_vkCmdSetLineWidth CmdSetLineWidth;
    PFN_vkCmdSetDepthBias CmdSetDepthBias;
    PFN_vkCmdSetBlendConstants CmdSetBlendConstants;
    PFN_vkCmdSetDepthBounds CmdSetDepthBounds;
    PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
    PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask;
    PFN_vkCmdSetStencilReference CmdSetStencilReference;
    PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
    PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer;
    PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
    PFN_vkCmdDraw CmdDraw;
    PFN_vkCmdDrawIndexed CmdDrawIndexed;
    PFN_vkCmdDrawIndirect CmdDrawIndirect;
    PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect;
    PFN_vkCmdDispatch CmdDispatch;
    PFN_vkCmdDispatchIndirect CmdDispatchIndirect;
    PFN_vkCmdCopyBuffer CmdCopyBuffer;
    PFN_vkCmdCopyImage CmdCopyImage;
    PFN_vkCmdBlitImage CmdBlitImage;
    PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
    PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
    PFN_vkCmdUpdateBuffer CmdUpdateBuffer;
    PFN_vkCmdFillBuffer CmdFillBuffer;
    PFN_vkCmdClearColorImage CmdClearColorImage;
    PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
    PFN_vkCmdClearAttachments CmdClearAttachments;
    PFN_vkCmdResolveImage CmdResolveImage;
    PFN_vkCmdSetEvent CmdSetEvent;
    PFN_vkCmdResetEvent CmdResetEvent;
    PFN_vkCmdWaitEvents CmdWaitEvents;
    PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
    PFN_vkCmdBeginQuery CmdBeginQuery;
    PFN_vkCmdEndQuery CmdEndQuery;
    PFN_vkCmdResetQueryPool CmdResetQueryPool;
    PFN_vkCmdWriteTimestamp CmdWriteTimestamp;
    PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults;
    PFN_vkCmdPushConstants CmdPushConstants;
    PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
    PFN_vkCmdNextSubpass CmdNextSubpass;
    PFN_vkCmdEndRenderPass CmdEndRenderPass;
    PFN_vkCmdExecuteCommands CmdExecuteCommands;

    // ---- Core Vulkan 1.1 commands
    PFN_vkBindBufferMemory2 BindBufferMemory2;
    PFN_vkBindImageMemory2 BindImageMemory2;
    PFN_vkGetDeviceGroupPeerMemoryFeatures GetDeviceGroupPeerMemoryFeatures;
    PFN_vkCmdSetDeviceMask CmdSetDeviceMask;
    PFN_vkCmdDispatchBase CmdDispatchBase;
    PFN_vkGetImageMemoryRequirements2 GetImageMemoryRequirements2;
    PFN_vkGetBufferMemoryRequirements2 GetBufferMemoryRequirements2;
    PFN_vkGetImageSparseMemoryRequirements2 GetImageSparseMemoryRequirements2;
    PFN_vkTrimCommandPool TrimCommandPool;
    PFN_vkGetDeviceQueue2 GetDeviceQueue2;
    PFN_vkCreateSamplerYcbcrConversion CreateSamplerYcbcrConversion;
    PFN_vkDestroySamplerYcbcrConversion DestroySamplerYcbcrConversion;
    PFN_vkCreateDescriptorUpdateTemplate CreateDescriptorUpdateTemplate;
    PFN_vkDestroyDescriptorUpdateTemplate DestroyDescriptorUpdateTemplate;
    PFN_vkUpdateDescriptorSetWithTemplate UpdateDescriptorSetWithTemplate;
    PFN_vkGetDescriptorSetLayoutSupport GetDescriptorSetLayoutSupport;

    // ---- Core Vulkan 1.2 commands
    PFN_vkCmdDrawIndirectCount CmdDrawIndirectCount;
    PFN_vkCmdDrawIndexedIndirectCount CmdDrawIndexedIndirectCount;
    PFN_vkCreateRenderPass2 CreateRenderPass2;
    PFN_vkCmdBeginRenderPass2 CmdBeginRenderPass2;
    PFN_vkCmdNextSubpass2 CmdNextSubpass2;
    PFN_vkCmdEndRenderPass2 CmdEndRenderPass2;
    PFN_vkResetQueryPool ResetQueryPool;
    PFN_vkGetSemaphoreCounterValue GetSemaphoreCounterValue;
    PFN_vkWaitSemaphores WaitSemaphores;
    PFN_vkSignalSemaphore SignalSemaphore;
    PFN_vkGetBufferDeviceAddress GetBufferDeviceAddress;
    PFN_vkGetBufferOpaqueCaptureAddress GetBufferOpaqueCaptureAddress;
    PFN_vkGetDeviceMemoryOpaqueCaptureAddress GetDeviceMemoryOpaqueCaptureAddress;

    // ---- Core Vulkan 1.3 commands
    PFN_vkCreatePrivateDataSlot CreatePrivateDataSlot;
    PFN_vkDestroyPrivateDataSlot DestroyPrivateDataSlot;
    PFN_vkSetPrivateData SetPrivateData;
    PFN_vkGetPrivateData GetPrivateData;
    PFN_vkCmdSetEvent2 CmdSetEvent2;
    PFN_vkCmdResetEvent2 CmdResetEvent2;
    PFN_vkCmdWaitEvents2 CmdWaitEvents2;
    PFN_vkCmdPipelineBarrier2 CmdPipelineBarrier2;
    PFN_vkCmdWriteTimestamp2 CmdWriteTimestamp2;
    PFN_vkQueueSubmit2 QueueSubmit2;
    PFN_vkCmdCopyBuffer2 CmdCopyBuffer2;
    PFN_vkCmdCopyImage2 CmdCopyImage2;
    PFN_vkCmdCopyBufferToImage2 CmdCopyBufferToImage2;
    PFN_vkCmdCopyImageToBuffer2 CmdCopyImageToBuffer2;
    PFN_vkCmdBlitImage2 CmdBlitImage2;
    PFN_vkCmdResolveImage2 CmdResolveImage2;
    PFN_vkCmdBeginRendering CmdBeginRendering;
    PFN_vkCmdEndRendering CmdEndRendering;
    PFN_vkCmdSetCullMode CmdSetCullMode;
    PFN_vkCmdSetFrontFace CmdSetFrontFace;
    PFN_vkCmdSetPrimitiveTopology CmdSetPrimitiveTopology;
    PFN_vkCmdSetViewportWithCount CmdSetViewportWithCount;
    PFN_vkCmdSetScissorWithCount CmdSetScissorWithCount;
    PFN_vkCmdBindVertexBuffers2 CmdBindVertexBuffers2;
    PFN_vkCmdSetDepthTestEnable CmdSetDepthTestEnable;
    PFN_vkCmdSetDepthWriteEnable CmdSetDepthWriteEnable;
    PFN_vkCmdSetDepthCompareOp CmdSetDepthCompareOp;
    PFN_vkCmdSetDepthBoundsTestEnable CmdSetDepthBoundsTestEnable;
    PFN_vkCmdSetStencilTestEnable CmdSetStencilTestEnable;
    PFN_vkCmdSetStencilOp CmdSetStencilOp;
    PFN_vkCmdSetRasterizerDiscardEnable CmdSetRasterizerDiscardEnable;
    PFN_vkCmdSetDepthBiasEnable CmdSetDepthBiasEnable;
    PFN_vkCmdSetPrimitiveRestartEnable CmdSetPrimitiveRestartEnable;
    PFN_vkGetDeviceBufferMemoryRequirements GetDeviceBufferMemoryRequirements;
    PFN_vkGetDeviceImageMemoryRequirements GetDeviceImageMemoryRequirements;
    PFN_vkGetDeviceImageSparseMemoryRequirements GetDeviceImageSparseMemoryRequirements;

    // ---- Core Vulkan 1.4 commands
    PFN_vkCmdSetLineStipple CmdSetLineStipple;
    PFN_vkMapMemory2 MapMemory2;
    PFN_vkUnmapMemory2 UnmapMemory2;
    PFN_vkCmdBindIndexBuffer2 CmdBindIndexBuffer2;
    PFN_vkGetRenderingAreaGranularity GetRenderingAreaGranularity;
    PFN_vkGetDeviceImageSubresourceLayout GetDeviceImageSubresourceLayout;
    PFN_vkGetImageSubresourceLayout2 GetImageSubresourceLayout2;
    PFN_vkCmdPushDescriptorSet CmdPushDescriptorSet;
    PFN_vkCmdPushDescriptorSetWithTemplate CmdPushDescriptorSetWithTemplate;
    PFN_vkCmdSetRenderingAttachmentLocations CmdSetRenderingAttachmentLocations;
    PFN_vkCmdSetRenderingInputAttachmentIndices CmdSetRenderingInputAttachmentIndices;
    PFN_vkCmdBindDescriptorSets2 CmdBindDescriptorSets2;
    PFN_vkCmdPushConstants2 CmdPushConstants2;
    PFN_vkCmdPushDescriptorSet2 CmdPushDescriptorSet2;
    PFN_vkCmdPushDescriptorSetWithTemplate2 CmdPushDescriptorSetWithTemplate2;
    PFN_vkCopyMemoryToImage CopyMemoryToImage;
    PFN_vkCopyImageToMemory CopyImageToMemory;
    PFN_vkCopyImageToImage CopyImageToImage;
    PFN_vkTransitionImageLayout TransitionImageLayout;

    // ---- VK_KHR_swapchain extension commands
    PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
    PFN_vkQueuePresentKHR QueuePresentKHR;
    PFN_vkGetDeviceGroupPresentCapabilitiesKHR GetDeviceGroupPresentCapabilitiesKHR;
    PFN_vkGetDeviceGroupSurfacePresentModesKHR GetDeviceGroupSurfacePresentModesKHR;
    PFN_vkAcquireNextImage2KHR AcquireNextImage2KHR;

    // ---- VK_KHR_display_swapchain extension commands
    PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR;

    // ---- VK_KHR_video_queue extension commands
    PFN_vkCreateVideoSessionKHR CreateVideoSessionKHR;
    PFN_vkDestroyVideoSessionKHR DestroyVideoSessionKHR;
    PFN_vkGetVideoSessionMemoryRequirementsKHR GetVideoSessionMemoryRequirementsKHR;
    PFN_vkBindVideoSessionMemoryKHR BindVideoSessionMemoryKHR;
    PFN_vkCreateVideoSessionParametersKHR CreateVideoSessionParametersKHR;
    PFN_vkUpdateVideoSessionParametersKHR UpdateVideoSessionParametersKHR;
    PFN_vkDestroyVideoSessionParametersKHR DestroyVideoSessionParametersKHR;
    PFN_vkCmdBeginVideoCodingKHR CmdBeginVideoCodingKHR;
    PFN_vkCmdEndVideoCodingKHR CmdEndVideoCodingKHR;
    PFN_vkCmdControlVideoCodingKHR CmdControlVideoCodingKHR;

    // ---- VK_KHR_video_decode_queue extension commands
    PFN_vkCmdDecodeVideoKHR CmdDecodeVideoKHR;

    // ---- VK_KHR_dynamic_rendering extension commands
    PFN_vkCmdBeginRenderingKHR CmdBeginRenderingKHR;
    PFN_vkCmdEndRenderingKHR CmdEndRenderingKHR;

    // ---- VK_KHR_device_group extension commands
    PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR GetDeviceGroupPeerMemoryFeaturesKHR;
    PFN_vkCmdSetDeviceMaskKHR CmdSetDeviceMaskKHR;
    PFN_vkCmdDispatchBaseKHR CmdDispatchBaseKHR;

    // ---- VK_KHR_maintenance1 extension commands
    PFN_vkTrimCommandPoolKHR TrimCommandPoolKHR;

    // ---- VK_KHR_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetMemoryWin32HandleKHR GetMemoryWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetMemoryWin32HandlePropertiesKHR GetMemoryWin32HandlePropertiesKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_memory_fd extension commands
    PFN_vkGetMemoryFdKHR GetMemoryFdKHR;
    PFN_vkGetMemoryFdPropertiesKHR GetMemoryFdPropertiesKHR;

    // ---- VK_KHR_external_semaphore_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkImportSemaphoreWin32HandleKHR ImportSemaphoreWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetSemaphoreWin32HandleKHR GetSemaphoreWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_semaphore_fd extension commands
    PFN_vkImportSemaphoreFdKHR ImportSemaphoreFdKHR;
    PFN_vkGetSemaphoreFdKHR GetSemaphoreFdKHR;

    // ---- VK_KHR_push_descriptor extension commands
    PFN_vkCmdPushDescriptorSetKHR CmdPushDescriptorSetKHR;
    PFN_vkCmdPushDescriptorSetWithTemplateKHR CmdPushDescriptorSetWithTemplateKHR;

    // ---- VK_KHR_descriptor_update_template extension commands
    PFN_vkCreateDescriptorUpdateTemplateKHR CreateDescriptorUpdateTemplateKHR;
    PFN_vkDestroyDescriptorUpdateTemplateKHR DestroyDescriptorUpdateTemplateKHR;
    PFN_vkUpdateDescriptorSetWithTemplateKHR UpdateDescriptorSetWithTemplateKHR;

    // ---- VK_KHR_create_renderpass2 extension commands
    PFN_vkCreateRenderPass2KHR CreateRenderPass2KHR;
    PFN_vkCmdBeginRenderPass2KHR CmdBeginRenderPass2KHR;
    PFN_vkCmdNextSubpass2KHR CmdNextSubpass2KHR;
    PFN_vkCmdEndRenderPass2KHR CmdEndRenderPass2KHR;

    // ---- VK_KHR_shared_presentable_image extension commands
    PFN_vkGetSwapchainStatusKHR GetSwapchainStatusKHR;

    // ---- VK_KHR_external_fence_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkImportFenceWin32HandleKHR ImportFenceWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetFenceWin32HandleKHR GetFenceWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_fence_fd extension commands
    PFN_vkImportFenceFdKHR ImportFenceFdKHR;
    PFN_vkGetFenceFdKHR GetFenceFdKHR;

    // ---- VK_KHR_performance_query extension commands
    PFN_vkAcquireProfilingLockKHR AcquireProfilingLockKHR;
    PFN_vkReleaseProfilingLockKHR ReleaseProfilingLockKHR;

    // ---- VK_KHR_get_memory_requirements2 extension commands
    PFN_vkGetImageMemoryRequirements2KHR GetImageMemoryRequirements2KHR;
    PFN_vkGetBufferMemoryRequirements2KHR GetBufferMemoryRequirements2KHR;
    PFN_vkGetImageSparseMemoryRequirements2KHR GetImageSparseMemoryRequirements2KHR;

    // ---- VK_KHR_sampler_ycbcr_conversion extension commands
    PFN_vkCreateSamplerYcbcrConversionKHR CreateSamplerYcbcrConversionKHR;
    PFN_vkDestroySamplerYcbcrConversionKHR DestroySamplerYcbcrConversionKHR;

    // ---- VK_KHR_bind_memory2 extension commands
    PFN_vkBindBufferMemory2KHR BindBufferMemory2KHR;
    PFN_vkBindImageMemory2KHR BindImageMemory2KHR;

    // ---- VK_KHR_maintenance3 extension commands
    PFN_vkGetDescriptorSetLayoutSupportKHR GetDescriptorSetLayoutSupportKHR;

    // ---- VK_KHR_draw_indirect_count extension commands
    PFN_vkCmdDrawIndirectCountKHR CmdDrawIndirectCountKHR;
    PFN_vkCmdDrawIndexedIndirectCountKHR CmdDrawIndexedIndirectCountKHR;

    // ---- VK_KHR_timeline_semaphore extension commands
    PFN_vkGetSemaphoreCounterValueKHR GetSemaphoreCounterValueKHR;
    PFN_vkWaitSemaphoresKHR WaitSemaphoresKHR;
    PFN_vkSignalSemaphoreKHR SignalSemaphoreKHR;

    // ---- VK_KHR_fragment_shading_rate extension commands
    PFN_vkCmdSetFragmentShadingRateKHR CmdSetFragmentShadingRateKHR;

    // ---- VK_KHR_dynamic_rendering_local_read extension commands
    PFN_vkCmdSetRenderingAttachmentLocationsKHR CmdSetRenderingAttachmentLocationsKHR;
    PFN_vkCmdSetRenderingInputAttachmentIndicesKHR CmdSetRenderingInputAttachmentIndicesKHR;

    // ---- VK_KHR_present_wait extension commands
    PFN_vkWaitForPresentKHR WaitForPresentKHR;

    // ---- VK_KHR_buffer_device_address extension commands
    PFN_vkGetBufferDeviceAddressKHR GetBufferDeviceAddressKHR;
    PFN_vkGetBufferOpaqueCaptureAddressKHR GetBufferOpaqueCaptureAddressKHR;
    PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR GetDeviceMemoryOpaqueCaptureAddressKHR;

    // ---- VK_KHR_deferred_host_operations extension commands
    PFN_vkCreateDeferredOperationKHR CreateDeferredOperationKHR;
    PFN_vkDestroyDeferredOperationKHR DestroyDeferredOperationKHR;
    PFN_vkGetDeferredOperationMaxConcurrencyKHR GetDeferredOperationMaxConcurrencyKHR;
    PFN_vkGetDeferredOperationResultKHR GetDeferredOperationResultKHR;
    PFN_vkDeferredOperationJoinKHR DeferredOperationJoinKHR;

    // ---- VK_KHR_pipeline_executable_properties extension commands
    PFN_vkGetPipelineExecutablePropertiesKHR GetPipelineExecutablePropertiesKHR;
    PFN_vkGetPipelineExecutableStatisticsKHR GetPipelineExecutableStatisticsKHR;
    PFN_vkGetPipelineExecutableInternalRepresentationsKHR GetPipelineExecutableInternalRepresentationsKHR;

    // ---- VK_KHR_map_memory2 extension commands
    PFN_vkMapMemory2KHR MapMemory2KHR;
    PFN_vkUnmapMemory2KHR UnmapMemory2KHR;

    // ---- VK_KHR_video_encode_queue extension commands
    PFN_vkGetEncodedVideoSessionParametersKHR GetEncodedVideoSessionParametersKHR;
    PFN_vkCmdEncodeVideoKHR CmdEncodeVideoKHR;

    // ---- VK_KHR_synchronization2 extension commands
    PFN_vkCmdSetEvent2KHR CmdSetEvent2KHR;
    PFN_vkCmdResetEvent2KHR CmdResetEvent2KHR;
    PFN_vkCmdWaitEvents2KHR CmdWaitEvents2KHR;
    PFN_vkCmdPipelineBarrier2KHR CmdPipelineBarrier2KHR;
    PFN_vkCmdWriteTimestamp2KHR CmdWriteTimestamp2KHR;
    PFN_vkQueueSubmit2KHR QueueSubmit2KHR;

    // ---- VK_KHR_copy_commands2 extension commands
    PFN_vkCmdCopyBuffer2KHR CmdCopyBuffer2KHR;
    PFN_vkCmdCopyImage2KHR CmdCopyImage2KHR;
    PFN_vkCmdCopyBufferToImage2KHR CmdCopyBufferToImage2KHR;
    PFN_vkCmdCopyImageToBuffer2KHR CmdCopyImageToBuffer2KHR;
    PFN_vkCmdBlitImage2KHR CmdBlitImage2KHR;
    PFN_vkCmdResolveImage2KHR CmdResolveImage2KHR;

    // ---- VK_KHR_ray_tracing_maintenance1 extension commands
    PFN_vkCmdTraceRaysIndirect2KHR CmdTraceRaysIndirect2KHR;

    // ---- VK_KHR_maintenance4 extension commands
    PFN_vkGetDeviceBufferMemoryRequirementsKHR GetDeviceBufferMemoryRequirementsKHR;
    PFN_vkGetDeviceImageMemoryRequirementsKHR GetDeviceImageMemoryRequirementsKHR;
    PFN_vkGetDeviceImageSparseMemoryRequirementsKHR GetDeviceImageSparseMemoryRequirementsKHR;

    // ---- VK_KHR_maintenance5 extension commands
    PFN_vkCmdBindIndexBuffer2KHR CmdBindIndexBuffer2KHR;
    PFN_vkGetRenderingAreaGranularityKHR GetRenderingAreaGranularityKHR;
    PFN_vkGetDeviceImageSubresourceLayoutKHR GetDeviceImageSubresourceLayoutKHR;
    PFN_vkGetImageSubresourceLayout2KHR GetImageSubresourceLayout2KHR;

    // ---- VK_KHR_present_wait2 extension commands
    PFN_vkWaitForPresent2KHR WaitForPresent2KHR;

    // ---- VK_KHR_pipeline_binary extension commands
    PFN_vkCreatePipelineBinariesKHR CreatePipelineBinariesKHR;
    PFN_vkDestroyPipelineBinaryKHR DestroyPipelineBinaryKHR;
    PFN_vkGetPipelineKeyKHR GetPipelineKeyKHR;
    PFN_vkGetPipelineBinaryDataKHR GetPipelineBinaryDataKHR;
    PFN_vkReleaseCapturedPipelineDataKHR ReleaseCapturedPipelineDataKHR;

    // ---- VK_KHR_swapchain_maintenance1 extension commands
    PFN_vkReleaseSwapchainImagesKHR ReleaseSwapchainImagesKHR;

    // ---- VK_KHR_line_rasterization extension commands
    PFN_vkCmdSetLineStippleKHR CmdSetLineStippleKHR;

    // ---- VK_KHR_calibrated_timestamps extension commands
    PFN_vkGetCalibratedTimestampsKHR GetCalibratedTimestampsKHR;

    // ---- VK_KHR_maintenance6 extension commands
    PFN_vkCmdBindDescriptorSets2KHR CmdBindDescriptorSets2KHR;
    PFN_vkCmdPushConstants2KHR CmdPushConstants2KHR;
    PFN_vkCmdPushDescriptorSet2KHR CmdPushDescriptorSet2KHR;
    PFN_vkCmdPushDescriptorSetWithTemplate2KHR CmdPushDescriptorSetWithTemplate2KHR;
    PFN_vkCmdSetDescriptorBufferOffsets2EXT CmdSetDescriptorBufferOffsets2EXT;
    PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT CmdBindDescriptorBufferEmbeddedSamplers2EXT;

    // ---- VK_EXT_debug_marker extension commands
    PFN_vkDebugMarkerSetObjectTagEXT DebugMarkerSetObjectTagEXT;
    PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT;
    PFN_vkCmdDebugMarkerBeginEXT CmdDebugMarkerBeginEXT;
    PFN_vkCmdDebugMarkerEndEXT CmdDebugMarkerEndEXT;
    PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT;

    // ---- VK_EXT_transform_feedback extension commands
    PFN_vkCmdBindTransformFeedbackBuffersEXT CmdBindTransformFeedbackBuffersEXT;
    PFN_vkCmdBeginTransformFeedbackEXT CmdBeginTransformFeedbackEXT;
    PFN_vkCmdEndTransformFeedbackEXT CmdEndTransformFeedbackEXT;
    PFN_vkCmdBeginQueryIndexedEXT CmdBeginQueryIndexedEXT;
    PFN_vkCmdEndQueryIndexedEXT CmdEndQueryIndexedEXT;
    PFN_vkCmdDrawIndirectByteCountEXT CmdDrawIndirectByteCountEXT;

    // ---- VK_NVX_binary_import extension commands
    PFN_vkCreateCuModuleNVX CreateCuModuleNVX;
    PFN_vkCreateCuFunctionNVX CreateCuFunctionNVX;
    PFN_vkDestroyCuModuleNVX DestroyCuModuleNVX;
    PFN_vkDestroyCuFunctionNVX DestroyCuFunctionNVX;
    PFN_vkCmdCuLaunchKernelNVX CmdCuLaunchKernelNVX;

    // ---- VK_NVX_image_view_handle extension commands
    PFN_vkGetImageViewHandleNVX GetImageViewHandleNVX;
    PFN_vkGetImageViewHandle64NVX GetImageViewHandle64NVX;
    PFN_vkGetImageViewAddressNVX GetImageViewAddressNVX;

    // ---- VK_AMD_draw_indirect_count extension commands
    PFN_vkCmdDrawIndirectCountAMD CmdDrawIndirectCountAMD;
    PFN_vkCmdDrawIndexedIndirectCountAMD CmdDrawIndexedIndirectCountAMD;

    // ---- VK_AMD_shader_info extension commands
    PFN_vkGetShaderInfoAMD GetShaderInfoAMD;

    // ---- VK_NV_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetMemoryWin32HandleNV GetMemoryWin32HandleNV;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_conditional_rendering extension commands
    PFN_vkCmdBeginConditionalRenderingEXT CmdBeginConditionalRenderingEXT;
    PFN_vkCmdEndConditionalRenderingEXT CmdEndConditionalRenderingEXT;

    // ---- VK_NV_clip_space_w_scaling extension commands
    PFN_vkCmdSetViewportWScalingNV CmdSetViewportWScalingNV;

    // ---- VK_EXT_display_control extension commands
    PFN_vkDisplayPowerControlEXT DisplayPowerControlEXT;
    PFN_vkRegisterDeviceEventEXT RegisterDeviceEventEXT;
    PFN_vkRegisterDisplayEventEXT RegisterDisplayEventEXT;
    PFN_vkGetSwapchainCounterEXT GetSwapchainCounterEXT;

    // ---- VK_GOOGLE_display_timing extension commands
    PFN_vkGetRefreshCycleDurationGOOGLE GetRefreshCycleDurationGOOGLE;
    PFN_vkGetPastPresentationTimingGOOGLE GetPastPresentationTimingGOOGLE;

    // ---- VK_EXT_discard_rectangles extension commands
    PFN_vkCmdSetDiscardRectangleEXT CmdSetDiscardRectangleEXT;
    PFN_vkCmdSetDiscardRectangleEnableEXT CmdSetDiscardRectangleEnableEXT;
    PFN_vkCmdSetDiscardRectangleModeEXT CmdSetDiscardRectangleModeEXT;

    // ---- VK_EXT_hdr_metadata extension commands
    PFN_vkSetHdrMetadataEXT SetHdrMetadataEXT;

    // ---- VK_EXT_debug_utils extension commands
    PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
    PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT;
    PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT;
    PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT;
    PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT;
    PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
    PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;

    // ---- VK_ANDROID_external_memory_android_hardware_buffer extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID GetAndroidHardwareBufferPropertiesANDROID;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    PFN_vkGetMemoryAndroidHardwareBufferANDROID GetMemoryAndroidHardwareBufferANDROID;
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_AMDX_shader_enqueue extension commands
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCreateExecutionGraphPipelinesAMDX CreateExecutionGraphPipelinesAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkGetExecutionGraphPipelineScratchSizeAMDX GetExecutionGraphPipelineScratchSizeAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkGetExecutionGraphPipelineNodeIndexAMDX GetExecutionGraphPipelineNodeIndexAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCmdInitializeGraphScratchMemoryAMDX CmdInitializeGraphScratchMemoryAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCmdDispatchGraphAMDX CmdDispatchGraphAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCmdDispatchGraphIndirectAMDX CmdDispatchGraphIndirectAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCmdDispatchGraphIndirectCountAMDX CmdDispatchGraphIndirectCountAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS

    // ---- VK_EXT_sample_locations extension commands
    PFN_vkCmdSetSampleLocationsEXT CmdSetSampleLocationsEXT;

    // ---- VK_EXT_image_drm_format_modifier extension commands
    PFN_vkGetImageDrmFormatModifierPropertiesEXT GetImageDrmFormatModifierPropertiesEXT;

    // ---- VK_EXT_validation_cache extension commands
    PFN_vkCreateValidationCacheEXT CreateValidationCacheEXT;
    PFN_vkDestroyValidationCacheEXT DestroyValidationCacheEXT;
    PFN_vkMergeValidationCachesEXT MergeValidationCachesEXT;
    PFN_vkGetValidationCacheDataEXT GetValidationCacheDataEXT;

    // ---- VK_NV_shading_rate_image extension commands
    PFN_vkCmdBindShadingRateImageNV CmdBindShadingRateImageNV;
    PFN_vkCmdSetViewportShadingRatePaletteNV CmdSetViewportShadingRatePaletteNV;
    PFN_vkCmdSetCoarseSampleOrderNV CmdSetCoarseSampleOrderNV;

    // ---- VK_NV_ray_tracing extension commands
    PFN_vkCreateAccelerationStructureNV CreateAccelerationStructureNV;
    PFN_vkDestroyAccelerationStructureNV DestroyAccelerationStructureNV;
    PFN_vkGetAccelerationStructureMemoryRequirementsNV GetAccelerationStructureMemoryRequirementsNV;
    PFN_vkBindAccelerationStructureMemoryNV BindAccelerationStructureMemoryNV;
    PFN_vkCmdBuildAccelerationStructureNV CmdBuildAccelerationStructureNV;
    PFN_vkCmdCopyAccelerationStructureNV CmdCopyAccelerationStructureNV;
    PFN_vkCmdTraceRaysNV CmdTraceRaysNV;
    PFN_vkCreateRayTracingPipelinesNV CreateRayTracingPipelinesNV;

    // ---- VK_KHR_ray_tracing_pipeline extension commands
    PFN_vkGetRayTracingShaderGroupHandlesKHR GetRayTracingShaderGroupHandlesKHR;

    // ---- VK_NV_ray_tracing extension commands
    PFN_vkGetRayTracingShaderGroupHandlesNV GetRayTracingShaderGroupHandlesNV;
    PFN_vkGetAccelerationStructureHandleNV GetAccelerationStructureHandleNV;
    PFN_vkCmdWriteAccelerationStructuresPropertiesNV CmdWriteAccelerationStructuresPropertiesNV;
    PFN_vkCompileDeferredNV CompileDeferredNV;

    // ---- VK_EXT_external_memory_host extension commands
    PFN_vkGetMemoryHostPointerPropertiesEXT GetMemoryHostPointerPropertiesEXT;

    // ---- VK_AMD_buffer_marker extension commands
    PFN_vkCmdWriteBufferMarkerAMD CmdWriteBufferMarkerAMD;
    PFN_vkCmdWriteBufferMarker2AMD CmdWriteBufferMarker2AMD;

    // ---- VK_EXT_calibrated_timestamps extension commands
    PFN_vkGetCalibratedTimestampsEXT GetCalibratedTimestampsEXT;

    // ---- VK_NV_mesh_shader extension commands
    PFN_vkCmdDrawMeshTasksNV CmdDrawMeshTasksNV;
    PFN_vkCmdDrawMeshTasksIndirectNV CmdDrawMeshTasksIndirectNV;
    PFN_vkCmdDrawMeshTasksIndirectCountNV CmdDrawMeshTasksIndirectCountNV;

    // ---- VK_NV_scissor_exclusive extension commands
    PFN_vkCmdSetExclusiveScissorEnableNV CmdSetExclusiveScissorEnableNV;
    PFN_vkCmdSetExclusiveScissorNV CmdSetExclusiveScissorNV;

    // ---- VK_NV_device_diagnostic_checkpoints extension commands
    PFN_vkCmdSetCheckpointNV CmdSetCheckpointNV;
    PFN_vkGetQueueCheckpointDataNV GetQueueCheckpointDataNV;
    PFN_vkGetQueueCheckpointData2NV GetQueueCheckpointData2NV;

    // ---- VK_INTEL_performance_query extension commands
    PFN_vkInitializePerformanceApiINTEL InitializePerformanceApiINTEL;
    PFN_vkUninitializePerformanceApiINTEL UninitializePerformanceApiINTEL;
    PFN_vkCmdSetPerformanceMarkerINTEL CmdSetPerformanceMarkerINTEL;
    PFN_vkCmdSetPerformanceStreamMarkerINTEL CmdSetPerformanceStreamMarkerINTEL;
    PFN_vkCmdSetPerformanceOverrideINTEL CmdSetPerformanceOverrideINTEL;
    PFN_vkAcquirePerformanceConfigurationINTEL AcquirePerformanceConfigurationINTEL;
    PFN_vkReleasePerformanceConfigurationINTEL ReleasePerformanceConfigurationINTEL;
    PFN_vkQueueSetPerformanceConfigurationINTEL QueueSetPerformanceConfigurationINTEL;
    PFN_vkGetPerformanceParameterINTEL GetPerformanceParameterINTEL;

    // ---- VK_AMD_display_native_hdr extension commands
    PFN_vkSetLocalDimmingAMD SetLocalDimmingAMD;

    // ---- VK_EXT_buffer_device_address extension commands
    PFN_vkGetBufferDeviceAddressEXT GetBufferDeviceAddressEXT;

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkAcquireFullScreenExclusiveModeEXT AcquireFullScreenExclusiveModeEXT;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkReleaseFullScreenExclusiveModeEXT ReleaseFullScreenExclusiveModeEXT;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkGetDeviceGroupSurfacePresentModes2EXT GetDeviceGroupSurfacePresentModes2EXT;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_line_rasterization extension commands
    PFN_vkCmdSetLineStippleEXT CmdSetLineStippleEXT;

    // ---- VK_EXT_host_query_reset extension commands
    PFN_vkResetQueryPoolEXT ResetQueryPoolEXT;

    // ---- VK_EXT_extended_dynamic_state extension commands
    PFN_vkCmdSetCullModeEXT CmdSetCullModeEXT;
    PFN_vkCmdSetFrontFaceEXT CmdSetFrontFaceEXT;
    PFN_vkCmdSetPrimitiveTopologyEXT CmdSetPrimitiveTopologyEXT;
    PFN_vkCmdSetViewportWithCountEXT CmdSetViewportWithCountEXT;
    PFN_vkCmdSetScissorWithCountEXT CmdSetScissorWithCountEXT;
    PFN_vkCmdBindVertexBuffers2EXT CmdBindVertexBuffers2EXT;
    PFN_vkCmdSetDepthTestEnableEXT CmdSetDepthTestEnableEXT;
    PFN_vkCmdSetDepthWriteEnableEXT CmdSetDepthWriteEnableEXT;
    PFN_vkCmdSetDepthCompareOpEXT CmdSetDepthCompareOpEXT;
    PFN_vkCmdSetDepthBoundsTestEnableEXT CmdSetDepthBoundsTestEnableEXT;
    PFN_vkCmdSetStencilTestEnableEXT CmdSetStencilTestEnableEXT;
    PFN_vkCmdSetStencilOpEXT CmdSetStencilOpEXT;

    // ---- VK_EXT_host_image_copy extension commands
    PFN_vkCopyMemoryToImageEXT CopyMemoryToImageEXT;
    PFN_vkCopyImageToMemoryEXT CopyImageToMemoryEXT;
    PFN_vkCopyImageToImageEXT CopyImageToImageEXT;
    PFN_vkTransitionImageLayoutEXT TransitionImageLayoutEXT;
    PFN_vkGetImageSubresourceLayout2EXT GetImageSubresourceLayout2EXT;

    // ---- VK_EXT_swapchain_maintenance1 extension commands
    PFN_vkReleaseSwapchainImagesEXT ReleaseSwapchainImagesEXT;

    // ---- VK_NV_device_generated_commands extension commands
    PFN_vkGetGeneratedCommandsMemoryRequirementsNV GetGeneratedCommandsMemoryRequirementsNV;
    PFN_vkCmdPreprocessGeneratedCommandsNV CmdPreprocessGeneratedCommandsNV;
    PFN_vkCmdExecuteGeneratedCommandsNV CmdExecuteGeneratedCommandsNV;
    PFN_vkCmdBindPipelineShaderGroupNV CmdBindPipelineShaderGroupNV;
    PFN_vkCreateIndirectCommandsLayoutNV CreateIndirectCommandsLayoutNV;
    PFN_vkDestroyIndirectCommandsLayoutNV DestroyIndirectCommandsLayoutNV;

    // ---- VK_EXT_depth_bias_control extension commands
    PFN_vkCmdSetDepthBias2EXT CmdSetDepthBias2EXT;

    // ---- VK_EXT_private_data extension commands
    PFN_vkCreatePrivateDataSlotEXT CreatePrivateDataSlotEXT;
    PFN_vkDestroyPrivateDataSlotEXT DestroyPrivateDataSlotEXT;
    PFN_vkSetPrivateDataEXT SetPrivateDataEXT;
    PFN_vkGetPrivateDataEXT GetPrivateDataEXT;

    // ---- VK_NV_cuda_kernel_launch extension commands
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCreateCudaModuleNV CreateCudaModuleNV;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkGetCudaModuleCacheNV GetCudaModuleCacheNV;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCreateCudaFunctionNV CreateCudaFunctionNV;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkDestroyCudaModuleNV DestroyCudaModuleNV;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkDestroyCudaFunctionNV DestroyCudaFunctionNV;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    PFN_vkCmdCudaLaunchKernelNV CmdCudaLaunchKernelNV;
#endif // VK_ENABLE_BETA_EXTENSIONS

    // ---- VK_QCOM_tile_shading extension commands
    PFN_vkCmdDispatchTileQCOM CmdDispatchTileQCOM;
    PFN_vkCmdBeginPerTileExecutionQCOM CmdBeginPerTileExecutionQCOM;
    PFN_vkCmdEndPerTileExecutionQCOM CmdEndPerTileExecutionQCOM;

    // ---- VK_EXT_metal_objects extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    PFN_vkExportMetalObjectsEXT ExportMetalObjectsEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_descriptor_buffer extension commands
    PFN_vkGetDescriptorSetLayoutSizeEXT GetDescriptorSetLayoutSizeEXT;
    PFN_vkGetDescriptorSetLayoutBindingOffsetEXT GetDescriptorSetLayoutBindingOffsetEXT;
    PFN_vkGetDescriptorEXT GetDescriptorEXT;
    PFN_vkCmdBindDescriptorBuffersEXT CmdBindDescriptorBuffersEXT;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT CmdSetDescriptorBufferOffsetsEXT;
    PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT CmdBindDescriptorBufferEmbeddedSamplersEXT;
    PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT GetBufferOpaqueCaptureDescriptorDataEXT;
    PFN_vkGetImageOpaqueCaptureDescriptorDataEXT GetImageOpaqueCaptureDescriptorDataEXT;
    PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT GetImageViewOpaqueCaptureDescriptorDataEXT;
    PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT GetSamplerOpaqueCaptureDescriptorDataEXT;
    PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT GetAccelerationStructureOpaqueCaptureDescriptorDataEXT;

    // ---- VK_NV_fragment_shading_rate_enums extension commands
    PFN_vkCmdSetFragmentShadingRateEnumNV CmdSetFragmentShadingRateEnumNV;

    // ---- VK_EXT_device_fault extension commands
    PFN_vkGetDeviceFaultInfoEXT GetDeviceFaultInfoEXT;

    // ---- VK_EXT_vertex_input_dynamic_state extension commands
    PFN_vkCmdSetVertexInputEXT CmdSetVertexInputEXT;

    // ---- VK_FUCHSIA_external_memory extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkGetMemoryZirconHandleFUCHSIA GetMemoryZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA GetMemoryZirconHandlePropertiesFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_external_semaphore extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkImportSemaphoreZirconHandleFUCHSIA ImportSemaphoreZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkGetSemaphoreZirconHandleFUCHSIA GetSemaphoreZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_buffer_collection extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkCreateBufferCollectionFUCHSIA CreateBufferCollectionFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkSetBufferCollectionImageConstraintsFUCHSIA SetBufferCollectionImageConstraintsFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA SetBufferCollectionBufferConstraintsFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkDestroyBufferCollectionFUCHSIA DestroyBufferCollectionFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkGetBufferCollectionPropertiesFUCHSIA GetBufferCollectionPropertiesFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_HUAWEI_subpass_shading extension commands
    PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;
    PFN_vkCmdSubpassShadingHUAWEI CmdSubpassShadingHUAWEI;

    // ---- VK_HUAWEI_invocation_mask extension commands
    PFN_vkCmdBindInvocationMaskHUAWEI CmdBindInvocationMaskHUAWEI;

    // ---- VK_NV_external_memory_rdma extension commands
    PFN_vkGetMemoryRemoteAddressNV GetMemoryRemoteAddressNV;

    // ---- VK_EXT_pipeline_properties extension commands
    PFN_vkGetPipelinePropertiesEXT GetPipelinePropertiesEXT;

    // ---- VK_EXT_extended_dynamic_state2 extension commands
    PFN_vkCmdSetPatchControlPointsEXT CmdSetPatchControlPointsEXT;
    PFN_vkCmdSetRasterizerDiscardEnableEXT CmdSetRasterizerDiscardEnableEXT;
    PFN_vkCmdSetDepthBiasEnableEXT CmdSetDepthBiasEnableEXT;
    PFN_vkCmdSetLogicOpEXT CmdSetLogicOpEXT;
    PFN_vkCmdSetPrimitiveRestartEnableEXT CmdSetPrimitiveRestartEnableEXT;

    // ---- VK_EXT_color_write_enable extension commands
    PFN_vkCmdSetColorWriteEnableEXT CmdSetColorWriteEnableEXT;

    // ---- VK_EXT_multi_draw extension commands
    PFN_vkCmdDrawMultiEXT CmdDrawMultiEXT;
    PFN_vkCmdDrawMultiIndexedEXT CmdDrawMultiIndexedEXT;

    // ---- VK_EXT_opacity_micromap extension commands
    PFN_vkCreateMicromapEXT CreateMicromapEXT;
    PFN_vkDestroyMicromapEXT DestroyMicromapEXT;
    PFN_vkCmdBuildMicromapsEXT CmdBuildMicromapsEXT;
    PFN_vkBuildMicromapsEXT BuildMicromapsEXT;
    PFN_vkCopyMicromapEXT CopyMicromapEXT;
    PFN_vkCopyMicromapToMemoryEXT CopyMicromapToMemoryEXT;
    PFN_vkCopyMemoryToMicromapEXT CopyMemoryToMicromapEXT;
    PFN_vkWriteMicromapsPropertiesEXT WriteMicromapsPropertiesEXT;
    PFN_vkCmdCopyMicromapEXT CmdCopyMicromapEXT;
    PFN_vkCmdCopyMicromapToMemoryEXT CmdCopyMicromapToMemoryEXT;
    PFN_vkCmdCopyMemoryToMicromapEXT CmdCopyMemoryToMicromapEXT;
    PFN_vkCmdWriteMicromapsPropertiesEXT CmdWriteMicromapsPropertiesEXT;
    PFN_vkGetDeviceMicromapCompatibilityEXT GetDeviceMicromapCompatibilityEXT;
    PFN_vkGetMicromapBuildSizesEXT GetMicromapBuildSizesEXT;

    // ---- VK_HUAWEI_cluster_culling_shader extension commands
    PFN_vkCmdDrawClusterHUAWEI CmdDrawClusterHUAWEI;
    PFN_vkCmdDrawClusterIndirectHUAWEI CmdDrawClusterIndirectHUAWEI;

    // ---- VK_EXT_pageable_device_local_memory extension commands
    PFN_vkSetDeviceMemoryPriorityEXT SetDeviceMemoryPriorityEXT;

    // ---- VK_VALVE_descriptor_set_host_mapping extension commands
    PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE GetDescriptorSetLayoutHostMappingInfoVALVE;
    PFN_vkGetDescriptorSetHostMappingVALVE GetDescriptorSetHostMappingVALVE;

    // ---- VK_NV_copy_memory_indirect extension commands
    PFN_vkCmdCopyMemoryIndirectNV CmdCopyMemoryIndirectNV;
    PFN_vkCmdCopyMemoryToImageIndirectNV CmdCopyMemoryToImageIndirectNV;

    // ---- VK_NV_memory_decompression extension commands
    PFN_vkCmdDecompressMemoryNV CmdDecompressMemoryNV;
    PFN_vkCmdDecompressMemoryIndirectCountNV CmdDecompressMemoryIndirectCountNV;

    // ---- VK_NV_device_generated_commands_compute extension commands
    PFN_vkGetPipelineIndirectMemoryRequirementsNV GetPipelineIndirectMemoryRequirementsNV;
    PFN_vkCmdUpdatePipelineIndirectBufferNV CmdUpdatePipelineIndirectBufferNV;
    PFN_vkGetPipelineIndirectDeviceAddressNV GetPipelineIndirectDeviceAddressNV;

    // ---- VK_EXT_extended_dynamic_state3 extension commands
    PFN_vkCmdSetDepthClampEnableEXT CmdSetDepthClampEnableEXT;
    PFN_vkCmdSetPolygonModeEXT CmdSetPolygonModeEXT;
    PFN_vkCmdSetRasterizationSamplesEXT CmdSetRasterizationSamplesEXT;
    PFN_vkCmdSetSampleMaskEXT CmdSetSampleMaskEXT;
    PFN_vkCmdSetAlphaToCoverageEnableEXT CmdSetAlphaToCoverageEnableEXT;
    PFN_vkCmdSetAlphaToOneEnableEXT CmdSetAlphaToOneEnableEXT;
    PFN_vkCmdSetLogicOpEnableEXT CmdSetLogicOpEnableEXT;
    PFN_vkCmdSetColorBlendEnableEXT CmdSetColorBlendEnableEXT;
    PFN_vkCmdSetColorBlendEquationEXT CmdSetColorBlendEquationEXT;
    PFN_vkCmdSetColorWriteMaskEXT CmdSetColorWriteMaskEXT;
    PFN_vkCmdSetTessellationDomainOriginEXT CmdSetTessellationDomainOriginEXT;
    PFN_vkCmdSetRasterizationStreamEXT CmdSetRasterizationStreamEXT;
    PFN_vkCmdSetConservativeRasterizationModeEXT CmdSetConservativeRasterizationModeEXT;
    PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT CmdSetExtraPrimitiveOverestimationSizeEXT;
    PFN_vkCmdSetDepthClipEnableEXT CmdSetDepthClipEnableEXT;
    PFN_vkCmdSetSampleLocationsEnableEXT CmdSetSampleLocationsEnableEXT;
    PFN_vkCmdSetColorBlendAdvancedEXT CmdSetColorBlendAdvancedEXT;
    PFN_vkCmdSetProvokingVertexModeEXT CmdSetProvokingVertexModeEXT;
    PFN_vkCmdSetLineRasterizationModeEXT CmdSetLineRasterizationModeEXT;
    PFN_vkCmdSetLineStippleEnableEXT CmdSetLineStippleEnableEXT;
    PFN_vkCmdSetDepthClipNegativeOneToOneEXT CmdSetDepthClipNegativeOneToOneEXT;
    PFN_vkCmdSetViewportWScalingEnableNV CmdSetViewportWScalingEnableNV;
    PFN_vkCmdSetViewportSwizzleNV CmdSetViewportSwizzleNV;
    PFN_vkCmdSetCoverageToColorEnableNV CmdSetCoverageToColorEnableNV;
    PFN_vkCmdSetCoverageToColorLocationNV CmdSetCoverageToColorLocationNV;
    PFN_vkCmdSetCoverageModulationModeNV CmdSetCoverageModulationModeNV;
    PFN_vkCmdSetCoverageModulationTableEnableNV CmdSetCoverageModulationTableEnableNV;
    PFN_vkCmdSetCoverageModulationTableNV CmdSetCoverageModulationTableNV;
    PFN_vkCmdSetShadingRateImageEnableNV CmdSetShadingRateImageEnableNV;
    PFN_vkCmdSetRepresentativeFragmentTestEnableNV CmdSetRepresentativeFragmentTestEnableNV;
    PFN_vkCmdSetCoverageReductionModeNV CmdSetCoverageReductionModeNV;

    // ---- VK_ARM_tensors extension commands
    PFN_vkCreateTensorARM CreateTensorARM;
    PFN_vkDestroyTensorARM DestroyTensorARM;
    PFN_vkCreateTensorViewARM CreateTensorViewARM;
    PFN_vkDestroyTensorViewARM DestroyTensorViewARM;
    PFN_vkGetTensorMemoryRequirementsARM GetTensorMemoryRequirementsARM;
    PFN_vkBindTensorMemoryARM BindTensorMemoryARM;
    PFN_vkGetDeviceTensorMemoryRequirementsARM GetDeviceTensorMemoryRequirementsARM;
    PFN_vkCmdCopyTensorARM CmdCopyTensorARM;
    PFN_vkGetTensorOpaqueCaptureDescriptorDataARM GetTensorOpaqueCaptureDescriptorDataARM;
    PFN_vkGetTensorViewOpaqueCaptureDescriptorDataARM GetTensorViewOpaqueCaptureDescriptorDataARM;

    // ---- VK_EXT_shader_module_identifier extension commands
    PFN_vkGetShaderModuleIdentifierEXT GetShaderModuleIdentifierEXT;
    PFN_vkGetShaderModuleCreateInfoIdentifierEXT GetShaderModuleCreateInfoIdentifierEXT;

    // ---- VK_NV_optical_flow extension commands
    PFN_vkCreateOpticalFlowSessionNV CreateOpticalFlowSessionNV;
    PFN_vkDestroyOpticalFlowSessionNV DestroyOpticalFlowSessionNV;
    PFN_vkBindOpticalFlowSessionImageNV BindOpticalFlowSessionImageNV;
    PFN_vkCmdOpticalFlowExecuteNV CmdOpticalFlowExecuteNV;

    // ---- VK_AMD_anti_lag extension commands
    PFN_vkAntiLagUpdateAMD AntiLagUpdateAMD;

    // ---- VK_EXT_shader_object extension commands
    PFN_vkCreateShadersEXT CreateShadersEXT;
    PFN_vkDestroyShaderEXT DestroyShaderEXT;
    PFN_vkGetShaderBinaryDataEXT GetShaderBinaryDataEXT;
    PFN_vkCmdBindShadersEXT CmdBindShadersEXT;
    PFN_vkCmdSetDepthClampRangeEXT CmdSetDepthClampRangeEXT;

    // ---- VK_QCOM_tile_properties extension commands
    PFN_vkGetFramebufferTilePropertiesQCOM GetFramebufferTilePropertiesQCOM;
    PFN_vkGetDynamicRenderingTilePropertiesQCOM GetDynamicRenderingTilePropertiesQCOM;

    // ---- VK_NV_cooperative_vector extension commands
    PFN_vkConvertCooperativeVectorMatrixNV ConvertCooperativeVectorMatrixNV;
    PFN_vkCmdConvertCooperativeVectorMatrixNV CmdConvertCooperativeVectorMatrixNV;

    // ---- VK_NV_low_latency2 extension commands
    PFN_vkSetLatencySleepModeNV SetLatencySleepModeNV;
    PFN_vkLatencySleepNV LatencySleepNV;
    PFN_vkSetLatencyMarkerNV SetLatencyMarkerNV;
    PFN_vkGetLatencyTimingsNV GetLatencyTimingsNV;
    PFN_vkQueueNotifyOutOfBandNV QueueNotifyOutOfBandNV;

    // ---- VK_ARM_data_graph extension commands
    PFN_vkCreateDataGraphPipelinesARM CreateDataGraphPipelinesARM;
    PFN_vkCreateDataGraphPipelineSessionARM CreateDataGraphPipelineSessionARM;
    PFN_vkGetDataGraphPipelineSessionBindPointRequirementsARM GetDataGraphPipelineSessionBindPointRequirementsARM;
    PFN_vkGetDataGraphPipelineSessionMemoryRequirementsARM GetDataGraphPipelineSessionMemoryRequirementsARM;
    PFN_vkBindDataGraphPipelineSessionMemoryARM BindDataGraphPipelineSessionMemoryARM;
    PFN_vkDestroyDataGraphPipelineSessionARM DestroyDataGraphPipelineSessionARM;
    PFN_vkCmdDispatchDataGraphARM CmdDispatchDataGraphARM;
    PFN_vkGetDataGraphPipelineAvailablePropertiesARM GetDataGraphPipelineAvailablePropertiesARM;
    PFN_vkGetDataGraphPipelinePropertiesARM GetDataGraphPipelinePropertiesARM;

    // ---- VK_EXT_attachment_feedback_loop_dynamic_state extension commands
    PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT CmdSetAttachmentFeedbackLoopEnableEXT;

    // ---- VK_QNX_external_memory_screen_buffer extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    PFN_vkGetScreenBufferPropertiesQNX GetScreenBufferPropertiesQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_QCOM_tile_memory_heap extension commands
    PFN_vkCmdBindTileMemoryQCOM CmdBindTileMemoryQCOM;

    // ---- VK_NV_external_compute_queue extension commands
    PFN_vkCreateExternalComputeQueueNV CreateExternalComputeQueueNV;
    PFN_vkDestroyExternalComputeQueueNV DestroyExternalComputeQueueNV;
    PFN_vkGetExternalComputeQueueDataNV GetExternalComputeQueueDataNV;

    // ---- VK_NV_cluster_acceleration_structure extension commands
    PFN_vkGetClusterAccelerationStructureBuildSizesNV GetClusterAccelerationStructureBuildSizesNV;
    PFN_vkCmdBuildClusterAccelerationStructureIndirectNV CmdBuildClusterAccelerationStructureIndirectNV;

    // ---- VK_NV_partitioned_acceleration_structure extension commands
    PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV GetPartitionedAccelerationStructuresBuildSizesNV;
    PFN_vkCmdBuildPartitionedAccelerationStructuresNV CmdBuildPartitionedAccelerationStructuresNV;

    // ---- VK_EXT_device_generated_commands extension commands
    PFN_vkGetGeneratedCommandsMemoryRequirementsEXT GetGeneratedCommandsMemoryRequirementsEXT;
    PFN_vkCmdPreprocessGeneratedCommandsEXT CmdPreprocessGeneratedCommandsEXT;
    PFN_vkCmdExecuteGeneratedCommandsEXT CmdExecuteGeneratedCommandsEXT;
    PFN_vkCreateIndirectCommandsLayoutEXT CreateIndirectCommandsLayoutEXT;
    PFN_vkDestroyIndirectCommandsLayoutEXT DestroyIndirectCommandsLayoutEXT;
    PFN_vkCreateIndirectExecutionSetEXT CreateIndirectExecutionSetEXT;
    PFN_vkDestroyIndirectExecutionSetEXT DestroyIndirectExecutionSetEXT;
    PFN_vkUpdateIndirectExecutionSetPipelineEXT UpdateIndirectExecutionSetPipelineEXT;
    PFN_vkUpdateIndirectExecutionSetShaderEXT UpdateIndirectExecutionSetShaderEXT;

    // ---- VK_EXT_external_memory_metal extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    PFN_vkGetMemoryMetalHandleEXT GetMemoryMetalHandleEXT;
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_METAL_EXT)
    PFN_vkGetMemoryMetalHandlePropertiesEXT GetMemoryMetalHandlePropertiesEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_fragment_density_map_offset extension commands
    PFN_vkCmdEndRendering2EXT CmdEndRendering2EXT;

    // ---- VK_KHR_acceleration_structure extension commands
    PFN_vkCreateAccelerationStructureKHR CreateAccelerationStructureKHR;
    PFN_vkDestroyAccelerationStructureKHR DestroyAccelerationStructureKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR CmdBuildAccelerationStructuresKHR;
    PFN_vkCmdBuildAccelerationStructuresIndirectKHR CmdBuildAccelerationStructuresIndirectKHR;
    PFN_vkBuildAccelerationStructuresKHR BuildAccelerationStructuresKHR;
    PFN_vkCopyAccelerationStructureKHR CopyAccelerationStructureKHR;
    PFN_vkCopyAccelerationStructureToMemoryKHR CopyAccelerationStructureToMemoryKHR;
    PFN_vkCopyMemoryToAccelerationStructureKHR CopyMemoryToAccelerationStructureKHR;
    PFN_vkWriteAccelerationStructuresPropertiesKHR WriteAccelerationStructuresPropertiesKHR;
    PFN_vkCmdCopyAccelerationStructureKHR CmdCopyAccelerationStructureKHR;
    PFN_vkCmdCopyAccelerationStructureToMemoryKHR CmdCopyAccelerationStructureToMemoryKHR;
    PFN_vkCmdCopyMemoryToAccelerationStructureKHR CmdCopyMemoryToAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureDeviceAddressKHR GetAccelerationStructureDeviceAddressKHR;
    PFN_vkCmdWriteAccelerationStructuresPropertiesKHR CmdWriteAccelerationStructuresPropertiesKHR;
    PFN_vkGetDeviceAccelerationStructureCompatibilityKHR GetDeviceAccelerationStructureCompatibilityKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR GetAccelerationStructureBuildSizesKHR;

    // ---- VK_KHR_ray_tracing_pipeline extension commands
    PFN_vkCmdTraceRaysKHR CmdTraceRaysKHR;
    PFN_vkCreateRayTracingPipelinesKHR CreateRayTracingPipelinesKHR;
    PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR GetRayTracingCaptureReplayShaderGroupHandlesKHR;
    PFN_vkCmdTraceRaysIndirectKHR CmdTraceRaysIndirectKHR;
    PFN_vkGetRayTracingShaderGroupStackSizeKHR GetRayTracingShaderGroupStackSizeKHR;
    PFN_vkCmdSetRayTracingPipelineStackSizeKHR CmdSetRayTracingPipelineStackSizeKHR;

    // ---- VK_EXT_mesh_shader extension commands
    PFN_vkCmdDrawMeshTasksEXT CmdDrawMeshTasksEXT;
    PFN_vkCmdDrawMeshTasksIndirectEXT CmdDrawMeshTasksIndirectEXT;
    PFN_vkCmdDrawMeshTasksIndirectCountEXT CmdDrawMeshTasksIndirectCountEXT;
} VkLayerDispatchTable;

// clang-format on
