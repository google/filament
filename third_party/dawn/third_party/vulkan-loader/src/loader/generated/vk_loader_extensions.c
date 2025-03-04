// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See loader_extension_generator.py for modifications

/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 */

// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"
#include "vk_loader_extensions.h"
#include <vulkan/vk_icd.h>
#include "wsi.h"
#include "debug_utils.h"
#include "extension_manual.h"

// Device extension error function
VKAPI_ATTR VkResult VKAPI_CALL vkDevExtError(VkDevice dev) {
    struct loader_device *found_dev;
    // The device going in is a trampoline device
    struct loader_icd_term *icd_term = loader_get_icd_and_device(dev, &found_dev);

    if (icd_term)
        loader_log(icd_term->this_instance, VULKAN_LOADER_ERROR_BIT, 0,
                   "Bad destination in loader trampoline dispatch,"
                   "Are layers and extensions that you are calling enabled?");
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VKAPI_ATTR bool VKAPI_CALL loader_icd_init_entries(struct loader_instance* inst, struct loader_icd_term *icd_term) {
    const PFN_vkGetInstanceProcAddr fp_gipa = icd_term->scanned_icd->GetInstanceProcAddr;

#define LOOKUP_GIPA(func) icd_term->dispatch.func = (PFN_vk##func)fp_gipa(icd_term->instance, "vk" #func);

#define LOOKUP_REQUIRED_GIPA(func)                                                      \
    do {                                                                                \
        LOOKUP_GIPA(func);                                                              \
        if (!icd_term->dispatch.func) {                                                 \
            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0, "Unable to load %s from ICD %s",\
                       "vk"#func, icd_term->scanned_icd->lib_name);                     \
            return false;                                                               \
        }                                                                               \
    } while (0)


    // ---- Core Vulkan 1.0
    LOOKUP_REQUIRED_GIPA(DestroyInstance);
    LOOKUP_REQUIRED_GIPA(EnumeratePhysicalDevices);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceFeatures);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceFormatProperties);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceImageFormatProperties);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceProperties);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceQueueFamilyProperties);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceMemoryProperties);
    LOOKUP_REQUIRED_GIPA(GetDeviceProcAddr);
    LOOKUP_REQUIRED_GIPA(CreateDevice);
    LOOKUP_REQUIRED_GIPA(EnumerateDeviceExtensionProperties);
    LOOKUP_REQUIRED_GIPA(GetPhysicalDeviceSparseImageFormatProperties);

    // ---- Core Vulkan 1.1
    LOOKUP_GIPA(EnumeratePhysicalDeviceGroups);
    LOOKUP_GIPA(GetPhysicalDeviceFeatures2);
    LOOKUP_GIPA(GetPhysicalDeviceProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceFormatProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceImageFormatProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceQueueFamilyProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceMemoryProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceSparseImageFormatProperties2);
    LOOKUP_GIPA(GetPhysicalDeviceExternalBufferProperties);
    LOOKUP_GIPA(GetPhysicalDeviceExternalFenceProperties);
    LOOKUP_GIPA(GetPhysicalDeviceExternalSemaphoreProperties);

    // ---- Core Vulkan 1.3
    LOOKUP_GIPA(GetPhysicalDeviceToolProperties);

    // ---- VK_KHR_surface extension commands
    LOOKUP_GIPA(DestroySurfaceKHR);
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceSupportKHR);
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceFormatsKHR);
    LOOKUP_GIPA(GetPhysicalDeviceSurfacePresentModesKHR);

    // ---- VK_KHR_swapchain extension commands
    LOOKUP_GIPA(GetPhysicalDevicePresentRectanglesKHR);

    // ---- VK_KHR_display extension commands
    LOOKUP_GIPA(GetPhysicalDeviceDisplayPropertiesKHR);
    LOOKUP_GIPA(GetPhysicalDeviceDisplayPlanePropertiesKHR);
    LOOKUP_GIPA(GetDisplayPlaneSupportedDisplaysKHR);
    LOOKUP_GIPA(GetDisplayModePropertiesKHR);
    LOOKUP_GIPA(CreateDisplayModeKHR);
    LOOKUP_GIPA(GetDisplayPlaneCapabilitiesKHR);
    LOOKUP_GIPA(CreateDisplayPlaneSurfaceKHR);

    // ---- VK_KHR_xlib_surface extension commands
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    LOOKUP_GIPA(CreateXlibSurfaceKHR);
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    LOOKUP_GIPA(GetPhysicalDeviceXlibPresentationSupportKHR);
#endif // VK_USE_PLATFORM_XLIB_KHR

    // ---- VK_KHR_xcb_surface extension commands
#if defined(VK_USE_PLATFORM_XCB_KHR)
    LOOKUP_GIPA(CreateXcbSurfaceKHR);
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    LOOKUP_GIPA(GetPhysicalDeviceXcbPresentationSupportKHR);
#endif // VK_USE_PLATFORM_XCB_KHR

    // ---- VK_KHR_wayland_surface extension commands
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    LOOKUP_GIPA(CreateWaylandSurfaceKHR);
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    LOOKUP_GIPA(GetPhysicalDeviceWaylandPresentationSupportKHR);
#endif // VK_USE_PLATFORM_WAYLAND_KHR

    // ---- VK_KHR_android_surface extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    LOOKUP_GIPA(CreateAndroidSurfaceKHR);
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_KHR_win32_surface extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    LOOKUP_GIPA(CreateWin32SurfaceKHR);
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    LOOKUP_GIPA(GetPhysicalDeviceWin32PresentationSupportKHR);
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_video_queue extension commands
    LOOKUP_GIPA(GetPhysicalDeviceVideoCapabilitiesKHR);
    LOOKUP_GIPA(GetPhysicalDeviceVideoFormatPropertiesKHR);

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    LOOKUP_GIPA(GetPhysicalDeviceFeatures2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceFormatProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceImageFormatProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceQueueFamilyProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceMemoryProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceSparseImageFormatProperties2KHR);

    // ---- VK_KHR_device_group_creation extension commands
    LOOKUP_GIPA(EnumeratePhysicalDeviceGroupsKHR);

    // ---- VK_KHR_external_memory_capabilities extension commands
    LOOKUP_GIPA(GetPhysicalDeviceExternalBufferPropertiesKHR);

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    LOOKUP_GIPA(GetPhysicalDeviceExternalSemaphorePropertiesKHR);

    // ---- VK_KHR_external_fence_capabilities extension commands
    LOOKUP_GIPA(GetPhysicalDeviceExternalFencePropertiesKHR);

    // ---- VK_KHR_performance_query extension commands
    LOOKUP_GIPA(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
    LOOKUP_GIPA(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR);

    // ---- VK_KHR_get_surface_capabilities2 extension commands
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceCapabilities2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceFormats2KHR);

    // ---- VK_KHR_get_display_properties2 extension commands
    LOOKUP_GIPA(GetPhysicalDeviceDisplayProperties2KHR);
    LOOKUP_GIPA(GetPhysicalDeviceDisplayPlaneProperties2KHR);
    LOOKUP_GIPA(GetDisplayModeProperties2KHR);
    LOOKUP_GIPA(GetDisplayPlaneCapabilities2KHR);

    // ---- VK_KHR_fragment_shading_rate extension commands
    LOOKUP_GIPA(GetPhysicalDeviceFragmentShadingRatesKHR);

    // ---- VK_KHR_video_encode_queue extension commands
    LOOKUP_GIPA(GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR);

    // ---- VK_KHR_cooperative_matrix extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCooperativeMatrixPropertiesKHR);

    // ---- VK_KHR_calibrated_timestamps extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCalibrateableTimeDomainsKHR);

    // ---- VK_EXT_debug_report extension commands
    LOOKUP_GIPA(CreateDebugReportCallbackEXT);
    LOOKUP_GIPA(DestroyDebugReportCallbackEXT);
    LOOKUP_GIPA(DebugReportMessageEXT);

    // ---- VK_GGP_stream_descriptor_surface extension commands
#if defined(VK_USE_PLATFORM_GGP)
    LOOKUP_GIPA(CreateStreamDescriptorSurfaceGGP);
#endif // VK_USE_PLATFORM_GGP

    // ---- VK_NV_external_memory_capabilities extension commands
    LOOKUP_GIPA(GetPhysicalDeviceExternalImageFormatPropertiesNV);

    // ---- VK_NN_vi_surface extension commands
#if defined(VK_USE_PLATFORM_VI_NN)
    LOOKUP_GIPA(CreateViSurfaceNN);
#endif // VK_USE_PLATFORM_VI_NN

    // ---- VK_EXT_direct_mode_display extension commands
    LOOKUP_GIPA(ReleaseDisplayEXT);

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    LOOKUP_GIPA(AcquireXlibDisplayEXT);
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    LOOKUP_GIPA(GetRandROutputDisplayEXT);
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    LOOKUP_GIPA(GetPhysicalDeviceSurfaceCapabilities2EXT);

    // ---- VK_MVK_ios_surface extension commands
#if defined(VK_USE_PLATFORM_IOS_MVK)
    LOOKUP_GIPA(CreateIOSSurfaceMVK);
#endif // VK_USE_PLATFORM_IOS_MVK

    // ---- VK_MVK_macos_surface extension commands
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    LOOKUP_GIPA(CreateMacOSSurfaceMVK);
#endif // VK_USE_PLATFORM_MACOS_MVK

    // ---- VK_EXT_debug_utils extension commands
    LOOKUP_GIPA(CreateDebugUtilsMessengerEXT);
    LOOKUP_GIPA(DestroyDebugUtilsMessengerEXT);
    LOOKUP_GIPA(SubmitDebugUtilsMessageEXT);

    // ---- VK_EXT_sample_locations extension commands
    LOOKUP_GIPA(GetPhysicalDeviceMultisamplePropertiesEXT);

    // ---- VK_EXT_calibrated_timestamps extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCalibrateableTimeDomainsEXT);

    // ---- VK_FUCHSIA_imagepipe_surface extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    LOOKUP_GIPA(CreateImagePipeSurfaceFUCHSIA);
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_EXT_metal_surface extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    LOOKUP_GIPA(CreateMetalSurfaceEXT);
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_tooling_info extension commands
    LOOKUP_GIPA(GetPhysicalDeviceToolPropertiesEXT);

    // ---- VK_NV_cooperative_matrix extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCooperativeMatrixPropertiesNV);

    // ---- VK_NV_coverage_reduction_mode extension commands
    LOOKUP_GIPA(GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV);

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    LOOKUP_GIPA(GetPhysicalDeviceSurfacePresentModes2EXT);
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_headless_surface extension commands
    LOOKUP_GIPA(CreateHeadlessSurfaceEXT);

    // ---- VK_EXT_acquire_drm_display extension commands
    LOOKUP_GIPA(AcquireDrmDisplayEXT);
    LOOKUP_GIPA(GetDrmDisplayEXT);

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    LOOKUP_GIPA(AcquireWinrtDisplayNV);
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    LOOKUP_GIPA(GetWinrtDisplayNV);
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_directfb_surface extension commands
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    LOOKUP_GIPA(CreateDirectFBSurfaceEXT);
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    LOOKUP_GIPA(GetPhysicalDeviceDirectFBPresentationSupportEXT);
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

    // ---- VK_QNX_screen_surface extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    LOOKUP_GIPA(CreateScreenSurfaceQNX);
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    LOOKUP_GIPA(GetPhysicalDeviceScreenPresentationSupportQNX);
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_optical_flow extension commands
    LOOKUP_GIPA(GetPhysicalDeviceOpticalFlowImageFormatsNV);

    // ---- VK_NV_cooperative_vector extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCooperativeVectorPropertiesNV);

    // ---- VK_NV_cooperative_matrix2 extension commands
    LOOKUP_GIPA(GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV);

#undef LOOKUP_REQUIRED_GIPA
#undef LOOKUP_GIPA

    return true;
};

// Init Device function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_device_dispatch_table(struct loader_dev_dispatch_table *dev_table, PFN_vkGetDeviceProcAddr gpa,
                                                             VkDevice dev) {
    VkLayerDispatchTable *table = &dev_table->core_dispatch;
    if (table->magic != DEVICE_DISP_TABLE_MAGIC_NUMBER) { abort(); }
    for (uint32_t i = 0; i < MAX_NUM_UNKNOWN_EXTS; i++) dev_table->ext_dispatch[i] = (PFN_vkDevExt)vkDevExtError;

    // ---- Core Vulkan 1.0 commands
    table->GetDeviceProcAddr = gpa;
    table->DestroyDevice = (PFN_vkDestroyDevice)gpa(dev, "vkDestroyDevice");
    table->GetDeviceQueue = (PFN_vkGetDeviceQueue)gpa(dev, "vkGetDeviceQueue");
    table->QueueSubmit = (PFN_vkQueueSubmit)gpa(dev, "vkQueueSubmit");
    table->QueueWaitIdle = (PFN_vkQueueWaitIdle)gpa(dev, "vkQueueWaitIdle");
    table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle)gpa(dev, "vkDeviceWaitIdle");
    table->AllocateMemory = (PFN_vkAllocateMemory)gpa(dev, "vkAllocateMemory");
    table->FreeMemory = (PFN_vkFreeMemory)gpa(dev, "vkFreeMemory");
    table->MapMemory = (PFN_vkMapMemory)gpa(dev, "vkMapMemory");
    table->UnmapMemory = (PFN_vkUnmapMemory)gpa(dev, "vkUnmapMemory");
    table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpa(dev, "vkFlushMappedMemoryRanges");
    table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpa(dev, "vkInvalidateMappedMemoryRanges");
    table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)gpa(dev, "vkGetDeviceMemoryCommitment");
    table->BindBufferMemory = (PFN_vkBindBufferMemory)gpa(dev, "vkBindBufferMemory");
    table->BindImageMemory = (PFN_vkBindImageMemory)gpa(dev, "vkBindImageMemory");
    table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpa(dev, "vkGetBufferMemoryRequirements");
    table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpa(dev, "vkGetImageMemoryRequirements");
    table->GetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)gpa(dev, "vkGetImageSparseMemoryRequirements");
    table->QueueBindSparse = (PFN_vkQueueBindSparse)gpa(dev, "vkQueueBindSparse");
    table->CreateFence = (PFN_vkCreateFence)gpa(dev, "vkCreateFence");
    table->DestroyFence = (PFN_vkDestroyFence)gpa(dev, "vkDestroyFence");
    table->ResetFences = (PFN_vkResetFences)gpa(dev, "vkResetFences");
    table->GetFenceStatus = (PFN_vkGetFenceStatus)gpa(dev, "vkGetFenceStatus");
    table->WaitForFences = (PFN_vkWaitForFences)gpa(dev, "vkWaitForFences");
    table->CreateSemaphore = (PFN_vkCreateSemaphore)gpa(dev, "vkCreateSemaphore");
    table->DestroySemaphore = (PFN_vkDestroySemaphore)gpa(dev, "vkDestroySemaphore");
    table->CreateEvent = (PFN_vkCreateEvent)gpa(dev, "vkCreateEvent");
    table->DestroyEvent = (PFN_vkDestroyEvent)gpa(dev, "vkDestroyEvent");
    table->GetEventStatus = (PFN_vkGetEventStatus)gpa(dev, "vkGetEventStatus");
    table->SetEvent = (PFN_vkSetEvent)gpa(dev, "vkSetEvent");
    table->ResetEvent = (PFN_vkResetEvent)gpa(dev, "vkResetEvent");
    table->CreateQueryPool = (PFN_vkCreateQueryPool)gpa(dev, "vkCreateQueryPool");
    table->DestroyQueryPool = (PFN_vkDestroyQueryPool)gpa(dev, "vkDestroyQueryPool");
    table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults)gpa(dev, "vkGetQueryPoolResults");
    table->CreateBuffer = (PFN_vkCreateBuffer)gpa(dev, "vkCreateBuffer");
    table->DestroyBuffer = (PFN_vkDestroyBuffer)gpa(dev, "vkDestroyBuffer");
    table->CreateBufferView = (PFN_vkCreateBufferView)gpa(dev, "vkCreateBufferView");
    table->DestroyBufferView = (PFN_vkDestroyBufferView)gpa(dev, "vkDestroyBufferView");
    table->CreateImage = (PFN_vkCreateImage)gpa(dev, "vkCreateImage");
    table->DestroyImage = (PFN_vkDestroyImage)gpa(dev, "vkDestroyImage");
    table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)gpa(dev, "vkGetImageSubresourceLayout");
    table->CreateImageView = (PFN_vkCreateImageView)gpa(dev, "vkCreateImageView");
    table->DestroyImageView = (PFN_vkDestroyImageView)gpa(dev, "vkDestroyImageView");
    table->CreateShaderModule = (PFN_vkCreateShaderModule)gpa(dev, "vkCreateShaderModule");
    table->DestroyShaderModule = (PFN_vkDestroyShaderModule)gpa(dev, "vkDestroyShaderModule");
    table->CreatePipelineCache = (PFN_vkCreatePipelineCache)gpa(dev, "vkCreatePipelineCache");
    table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache)gpa(dev, "vkDestroyPipelineCache");
    table->GetPipelineCacheData = (PFN_vkGetPipelineCacheData)gpa(dev, "vkGetPipelineCacheData");
    table->MergePipelineCaches = (PFN_vkMergePipelineCaches)gpa(dev, "vkMergePipelineCaches");
    table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)gpa(dev, "vkCreateGraphicsPipelines");
    table->CreateComputePipelines = (PFN_vkCreateComputePipelines)gpa(dev, "vkCreateComputePipelines");
    table->DestroyPipeline = (PFN_vkDestroyPipeline)gpa(dev, "vkDestroyPipeline");
    table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout)gpa(dev, "vkCreatePipelineLayout");
    table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)gpa(dev, "vkDestroyPipelineLayout");
    table->CreateSampler = (PFN_vkCreateSampler)gpa(dev, "vkCreateSampler");
    table->DestroySampler = (PFN_vkDestroySampler)gpa(dev, "vkDestroySampler");
    table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)gpa(dev, "vkCreateDescriptorSetLayout");
    table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)gpa(dev, "vkDestroyDescriptorSetLayout");
    table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool)gpa(dev, "vkCreateDescriptorPool");
    table->DestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)gpa(dev, "vkDestroyDescriptorPool");
    table->ResetDescriptorPool = (PFN_vkResetDescriptorPool)gpa(dev, "vkResetDescriptorPool");
    table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)gpa(dev, "vkAllocateDescriptorSets");
    table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets)gpa(dev, "vkFreeDescriptorSets");
    table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)gpa(dev, "vkUpdateDescriptorSets");
    table->CreateFramebuffer = (PFN_vkCreateFramebuffer)gpa(dev, "vkCreateFramebuffer");
    table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer)gpa(dev, "vkDestroyFramebuffer");
    table->CreateRenderPass = (PFN_vkCreateRenderPass)gpa(dev, "vkCreateRenderPass");
    table->DestroyRenderPass = (PFN_vkDestroyRenderPass)gpa(dev, "vkDestroyRenderPass");
    table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)gpa(dev, "vkGetRenderAreaGranularity");
    table->CreateCommandPool = (PFN_vkCreateCommandPool)gpa(dev, "vkCreateCommandPool");
    table->DestroyCommandPool = (PFN_vkDestroyCommandPool)gpa(dev, "vkDestroyCommandPool");
    table->ResetCommandPool = (PFN_vkResetCommandPool)gpa(dev, "vkResetCommandPool");
    table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)gpa(dev, "vkAllocateCommandBuffers");
    table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers)gpa(dev, "vkFreeCommandBuffers");
    table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer)gpa(dev, "vkBeginCommandBuffer");
    table->EndCommandBuffer = (PFN_vkEndCommandBuffer)gpa(dev, "vkEndCommandBuffer");
    table->ResetCommandBuffer = (PFN_vkResetCommandBuffer)gpa(dev, "vkResetCommandBuffer");
    table->CmdBindPipeline = (PFN_vkCmdBindPipeline)gpa(dev, "vkCmdBindPipeline");
    table->CmdSetViewport = (PFN_vkCmdSetViewport)gpa(dev, "vkCmdSetViewport");
    table->CmdSetScissor = (PFN_vkCmdSetScissor)gpa(dev, "vkCmdSetScissor");
    table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth)gpa(dev, "vkCmdSetLineWidth");
    table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias)gpa(dev, "vkCmdSetDepthBias");
    table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)gpa(dev, "vkCmdSetBlendConstants");
    table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)gpa(dev, "vkCmdSetDepthBounds");
    table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)gpa(dev, "vkCmdSetStencilCompareMask");
    table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)gpa(dev, "vkCmdSetStencilWriteMask");
    table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference)gpa(dev, "vkCmdSetStencilReference");
    table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)gpa(dev, "vkCmdBindDescriptorSets");
    table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)gpa(dev, "vkCmdBindIndexBuffer");
    table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)gpa(dev, "vkCmdBindVertexBuffers");
    table->CmdDraw = (PFN_vkCmdDraw)gpa(dev, "vkCmdDraw");
    table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed)gpa(dev, "vkCmdDrawIndexed");
    table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect)gpa(dev, "vkCmdDrawIndirect");
    table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)gpa(dev, "vkCmdDrawIndexedIndirect");
    table->CmdDispatch = (PFN_vkCmdDispatch)gpa(dev, "vkCmdDispatch");
    table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)gpa(dev, "vkCmdDispatchIndirect");
    table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpa(dev, "vkCmdCopyBuffer");
    table->CmdCopyImage = (PFN_vkCmdCopyImage)gpa(dev, "vkCmdCopyImage");
    table->CmdBlitImage = (PFN_vkCmdBlitImage)gpa(dev, "vkCmdBlitImage");
    table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)gpa(dev, "vkCmdCopyBufferToImage");
    table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)gpa(dev, "vkCmdCopyImageToBuffer");
    table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)gpa(dev, "vkCmdUpdateBuffer");
    table->CmdFillBuffer = (PFN_vkCmdFillBuffer)gpa(dev, "vkCmdFillBuffer");
    table->CmdClearColorImage = (PFN_vkCmdClearColorImage)gpa(dev, "vkCmdClearColorImage");
    table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)gpa(dev, "vkCmdClearDepthStencilImage");
    table->CmdClearAttachments = (PFN_vkCmdClearAttachments)gpa(dev, "vkCmdClearAttachments");
    table->CmdResolveImage = (PFN_vkCmdResolveImage)gpa(dev, "vkCmdResolveImage");
    table->CmdSetEvent = (PFN_vkCmdSetEvent)gpa(dev, "vkCmdSetEvent");
    table->CmdResetEvent = (PFN_vkCmdResetEvent)gpa(dev, "vkCmdResetEvent");
    table->CmdWaitEvents = (PFN_vkCmdWaitEvents)gpa(dev, "vkCmdWaitEvents");
    table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)gpa(dev, "vkCmdPipelineBarrier");
    table->CmdBeginQuery = (PFN_vkCmdBeginQuery)gpa(dev, "vkCmdBeginQuery");
    table->CmdEndQuery = (PFN_vkCmdEndQuery)gpa(dev, "vkCmdEndQuery");
    table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool)gpa(dev, "vkCmdResetQueryPool");
    table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)gpa(dev, "vkCmdWriteTimestamp");
    table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)gpa(dev, "vkCmdCopyQueryPoolResults");
    table->CmdPushConstants = (PFN_vkCmdPushConstants)gpa(dev, "vkCmdPushConstants");
    table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)gpa(dev, "vkCmdBeginRenderPass");
    table->CmdNextSubpass = (PFN_vkCmdNextSubpass)gpa(dev, "vkCmdNextSubpass");
    table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass)gpa(dev, "vkCmdEndRenderPass");
    table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands)gpa(dev, "vkCmdExecuteCommands");

    // ---- Core Vulkan 1.1 commands
    table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)gpa(dev, "vkBindBufferMemory2");
    table->BindImageMemory2 = (PFN_vkBindImageMemory2)gpa(dev, "vkBindImageMemory2");
    table->GetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)gpa(dev, "vkGetDeviceGroupPeerMemoryFeatures");
    table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)gpa(dev, "vkCmdSetDeviceMask");
    table->CmdDispatchBase = (PFN_vkCmdDispatchBase)gpa(dev, "vkCmdDispatchBase");
    table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)gpa(dev, "vkGetImageMemoryRequirements2");
    table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)gpa(dev, "vkGetBufferMemoryRequirements2");
    table->GetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)gpa(dev, "vkGetImageSparseMemoryRequirements2");
    table->TrimCommandPool = (PFN_vkTrimCommandPool)gpa(dev, "vkTrimCommandPool");
    table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)gpa(dev, "vkGetDeviceQueue2");
    table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)gpa(dev, "vkCreateSamplerYcbcrConversion");
    table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)gpa(dev, "vkDestroySamplerYcbcrConversion");
    table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)gpa(dev, "vkCreateDescriptorUpdateTemplate");
    table->DestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)gpa(dev, "vkDestroyDescriptorUpdateTemplate");
    table->UpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)gpa(dev, "vkUpdateDescriptorSetWithTemplate");
    table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)gpa(dev, "vkGetDescriptorSetLayoutSupport");

    // ---- Core Vulkan 1.2 commands
    table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)gpa(dev, "vkCmdDrawIndirectCount");
    table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)gpa(dev, "vkCmdDrawIndexedIndirectCount");
    table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)gpa(dev, "vkCreateRenderPass2");
    table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)gpa(dev, "vkCmdBeginRenderPass2");
    table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)gpa(dev, "vkCmdNextSubpass2");
    table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)gpa(dev, "vkCmdEndRenderPass2");
    table->ResetQueryPool = (PFN_vkResetQueryPool)gpa(dev, "vkResetQueryPool");
    table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)gpa(dev, "vkGetSemaphoreCounterValue");
    table->WaitSemaphores = (PFN_vkWaitSemaphores)gpa(dev, "vkWaitSemaphores");
    table->SignalSemaphore = (PFN_vkSignalSemaphore)gpa(dev, "vkSignalSemaphore");
    table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)gpa(dev, "vkGetBufferDeviceAddress");
    table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)gpa(dev, "vkGetBufferOpaqueCaptureAddress");
    table->GetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)gpa(dev, "vkGetDeviceMemoryOpaqueCaptureAddress");

    // ---- Core Vulkan 1.3 commands
    table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)gpa(dev, "vkCreatePrivateDataSlot");
    table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)gpa(dev, "vkDestroyPrivateDataSlot");
    table->SetPrivateData = (PFN_vkSetPrivateData)gpa(dev, "vkSetPrivateData");
    table->GetPrivateData = (PFN_vkGetPrivateData)gpa(dev, "vkGetPrivateData");
    table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)gpa(dev, "vkCmdSetEvent2");
    table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)gpa(dev, "vkCmdResetEvent2");
    table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)gpa(dev, "vkCmdWaitEvents2");
    table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)gpa(dev, "vkCmdPipelineBarrier2");
    table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)gpa(dev, "vkCmdWriteTimestamp2");
    table->QueueSubmit2 = (PFN_vkQueueSubmit2)gpa(dev, "vkQueueSubmit2");
    table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)gpa(dev, "vkCmdCopyBuffer2");
    table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)gpa(dev, "vkCmdCopyImage2");
    table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)gpa(dev, "vkCmdCopyBufferToImage2");
    table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)gpa(dev, "vkCmdCopyImageToBuffer2");
    table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)gpa(dev, "vkCmdBlitImage2");
    table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)gpa(dev, "vkCmdResolveImage2");
    table->CmdBeginRendering = (PFN_vkCmdBeginRendering)gpa(dev, "vkCmdBeginRendering");
    table->CmdEndRendering = (PFN_vkCmdEndRendering)gpa(dev, "vkCmdEndRendering");
    table->CmdSetCullMode = (PFN_vkCmdSetCullMode)gpa(dev, "vkCmdSetCullMode");
    table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)gpa(dev, "vkCmdSetFrontFace");
    table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)gpa(dev, "vkCmdSetPrimitiveTopology");
    table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)gpa(dev, "vkCmdSetViewportWithCount");
    table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)gpa(dev, "vkCmdSetScissorWithCount");
    table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)gpa(dev, "vkCmdBindVertexBuffers2");
    table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)gpa(dev, "vkCmdSetDepthTestEnable");
    table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)gpa(dev, "vkCmdSetDepthWriteEnable");
    table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)gpa(dev, "vkCmdSetDepthCompareOp");
    table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)gpa(dev, "vkCmdSetDepthBoundsTestEnable");
    table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)gpa(dev, "vkCmdSetStencilTestEnable");
    table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)gpa(dev, "vkCmdSetStencilOp");
    table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)gpa(dev, "vkCmdSetRasterizerDiscardEnable");
    table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)gpa(dev, "vkCmdSetDepthBiasEnable");
    table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)gpa(dev, "vkCmdSetPrimitiveRestartEnable");
    table->GetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)gpa(dev, "vkGetDeviceBufferMemoryRequirements");
    table->GetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)gpa(dev, "vkGetDeviceImageMemoryRequirements");
    table->GetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements)gpa(dev, "vkGetDeviceImageSparseMemoryRequirements");

    // ---- Core Vulkan 1.4 commands
    table->CmdSetLineStipple = (PFN_vkCmdSetLineStipple)gpa(dev, "vkCmdSetLineStipple");
    table->MapMemory2 = (PFN_vkMapMemory2)gpa(dev, "vkMapMemory2");
    table->UnmapMemory2 = (PFN_vkUnmapMemory2)gpa(dev, "vkUnmapMemory2");
    table->CmdBindIndexBuffer2 = (PFN_vkCmdBindIndexBuffer2)gpa(dev, "vkCmdBindIndexBuffer2");
    table->GetRenderingAreaGranularity = (PFN_vkGetRenderingAreaGranularity)gpa(dev, "vkGetRenderingAreaGranularity");
    table->GetDeviceImageSubresourceLayout = (PFN_vkGetDeviceImageSubresourceLayout)gpa(dev, "vkGetDeviceImageSubresourceLayout");
    table->GetImageSubresourceLayout2 = (PFN_vkGetImageSubresourceLayout2)gpa(dev, "vkGetImageSubresourceLayout2");
    table->CmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)gpa(dev, "vkCmdPushDescriptorSet");
    table->CmdPushDescriptorSetWithTemplate = (PFN_vkCmdPushDescriptorSetWithTemplate)gpa(dev, "vkCmdPushDescriptorSetWithTemplate");
    table->CmdSetRenderingAttachmentLocations = (PFN_vkCmdSetRenderingAttachmentLocations)gpa(dev, "vkCmdSetRenderingAttachmentLocations");
    table->CmdSetRenderingInputAttachmentIndices = (PFN_vkCmdSetRenderingInputAttachmentIndices)gpa(dev, "vkCmdSetRenderingInputAttachmentIndices");
    table->CmdBindDescriptorSets2 = (PFN_vkCmdBindDescriptorSets2)gpa(dev, "vkCmdBindDescriptorSets2");
    table->CmdPushConstants2 = (PFN_vkCmdPushConstants2)gpa(dev, "vkCmdPushConstants2");
    table->CmdPushDescriptorSet2 = (PFN_vkCmdPushDescriptorSet2)gpa(dev, "vkCmdPushDescriptorSet2");
    table->CmdPushDescriptorSetWithTemplate2 = (PFN_vkCmdPushDescriptorSetWithTemplate2)gpa(dev, "vkCmdPushDescriptorSetWithTemplate2");
    table->CopyMemoryToImage = (PFN_vkCopyMemoryToImage)gpa(dev, "vkCopyMemoryToImage");
    table->CopyImageToMemory = (PFN_vkCopyImageToMemory)gpa(dev, "vkCopyImageToMemory");
    table->CopyImageToImage = (PFN_vkCopyImageToImage)gpa(dev, "vkCopyImageToImage");
    table->TransitionImageLayout = (PFN_vkTransitionImageLayout)gpa(dev, "vkTransitionImageLayout");
}

// Init Device function pointer dispatch table with extension commands
VKAPI_ATTR void VKAPI_CALL loader_init_device_extension_dispatch_table(struct loader_dev_dispatch_table *dev_table,
                                                                       PFN_vkGetInstanceProcAddr gipa,
                                                                       PFN_vkGetDeviceProcAddr gdpa,
                                                                       VkInstance inst,
                                                                       VkDevice dev) {
    VkLayerDispatchTable *table = &dev_table->core_dispatch;
    table->magic = DEVICE_DISP_TABLE_MAGIC_NUMBER;

    // ---- VK_KHR_swapchain extension commands
    table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gdpa(dev, "vkCreateSwapchainKHR");
    table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)gdpa(dev, "vkDestroySwapchainKHR");
    table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)gdpa(dev, "vkGetSwapchainImagesKHR");
    table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)gdpa(dev, "vkAcquireNextImageKHR");
    table->QueuePresentKHR = (PFN_vkQueuePresentKHR)gdpa(dev, "vkQueuePresentKHR");
    table->GetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)gdpa(dev, "vkGetDeviceGroupPresentCapabilitiesKHR");
    table->GetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)gdpa(dev, "vkGetDeviceGroupSurfacePresentModesKHR");
    table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)gdpa(dev, "vkAcquireNextImage2KHR");

    // ---- VK_KHR_display_swapchain extension commands
    table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)gdpa(dev, "vkCreateSharedSwapchainsKHR");

    // ---- VK_KHR_video_queue extension commands
    table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)gdpa(dev, "vkCreateVideoSessionKHR");
    table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)gdpa(dev, "vkDestroyVideoSessionKHR");
    table->GetVideoSessionMemoryRequirementsKHR = (PFN_vkGetVideoSessionMemoryRequirementsKHR)gdpa(dev, "vkGetVideoSessionMemoryRequirementsKHR");
    table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)gdpa(dev, "vkBindVideoSessionMemoryKHR");
    table->CreateVideoSessionParametersKHR = (PFN_vkCreateVideoSessionParametersKHR)gdpa(dev, "vkCreateVideoSessionParametersKHR");
    table->UpdateVideoSessionParametersKHR = (PFN_vkUpdateVideoSessionParametersKHR)gdpa(dev, "vkUpdateVideoSessionParametersKHR");
    table->DestroyVideoSessionParametersKHR = (PFN_vkDestroyVideoSessionParametersKHR)gdpa(dev, "vkDestroyVideoSessionParametersKHR");
    table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)gdpa(dev, "vkCmdBeginVideoCodingKHR");
    table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)gdpa(dev, "vkCmdEndVideoCodingKHR");
    table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)gdpa(dev, "vkCmdControlVideoCodingKHR");

    // ---- VK_KHR_video_decode_queue extension commands
    table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)gdpa(dev, "vkCmdDecodeVideoKHR");

    // ---- VK_KHR_dynamic_rendering extension commands
    table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)gdpa(dev, "vkCmdBeginRenderingKHR");
    table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)gdpa(dev, "vkCmdEndRenderingKHR");

    // ---- VK_KHR_device_group extension commands
    table->GetDeviceGroupPeerMemoryFeaturesKHR = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)gdpa(dev, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)gdpa(dev, "vkCmdSetDeviceMaskKHR");
    table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)gdpa(dev, "vkCmdDispatchBaseKHR");

    // ---- VK_KHR_maintenance1 extension commands
    table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)gdpa(dev, "vkTrimCommandPoolKHR");

    // ---- VK_KHR_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)gdpa(dev, "vkGetMemoryWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)gdpa(dev, "vkGetMemoryWin32HandlePropertiesKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_memory_fd extension commands
    table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)gdpa(dev, "vkGetMemoryFdKHR");
    table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)gdpa(dev, "vkGetMemoryFdPropertiesKHR");

    // ---- VK_KHR_external_semaphore_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)gdpa(dev, "vkImportSemaphoreWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)gdpa(dev, "vkGetSemaphoreWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_semaphore_fd extension commands
    table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)gdpa(dev, "vkImportSemaphoreFdKHR");
    table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)gdpa(dev, "vkGetSemaphoreFdKHR");

    // ---- VK_KHR_push_descriptor extension commands
    table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)gdpa(dev, "vkCmdPushDescriptorSetKHR");
    table->CmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)gdpa(dev, "vkCmdPushDescriptorSetWithTemplateKHR");

    // ---- VK_KHR_descriptor_update_template extension commands
    table->CreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)gdpa(dev, "vkCreateDescriptorUpdateTemplateKHR");
    table->DestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)gdpa(dev, "vkDestroyDescriptorUpdateTemplateKHR");
    table->UpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)gdpa(dev, "vkUpdateDescriptorSetWithTemplateKHR");

    // ---- VK_KHR_create_renderpass2 extension commands
    table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)gdpa(dev, "vkCreateRenderPass2KHR");
    table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)gdpa(dev, "vkCmdBeginRenderPass2KHR");
    table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)gdpa(dev, "vkCmdNextSubpass2KHR");
    table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)gdpa(dev, "vkCmdEndRenderPass2KHR");

    // ---- VK_KHR_shared_presentable_image extension commands
    table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)gdpa(dev, "vkGetSwapchainStatusKHR");

    // ---- VK_KHR_external_fence_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)gdpa(dev, "vkImportFenceWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)gdpa(dev, "vkGetFenceWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_fence_fd extension commands
    table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)gdpa(dev, "vkImportFenceFdKHR");
    table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)gdpa(dev, "vkGetFenceFdKHR");

    // ---- VK_KHR_performance_query extension commands
    table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)gdpa(dev, "vkAcquireProfilingLockKHR");
    table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)gdpa(dev, "vkReleaseProfilingLockKHR");

    // ---- VK_KHR_get_memory_requirements2 extension commands
    table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)gdpa(dev, "vkGetImageMemoryRequirements2KHR");
    table->GetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)gdpa(dev, "vkGetBufferMemoryRequirements2KHR");
    table->GetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)gdpa(dev, "vkGetImageSparseMemoryRequirements2KHR");

    // ---- VK_KHR_sampler_ycbcr_conversion extension commands
    table->CreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)gdpa(dev, "vkCreateSamplerYcbcrConversionKHR");
    table->DestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)gdpa(dev, "vkDestroySamplerYcbcrConversionKHR");

    // ---- VK_KHR_bind_memory2 extension commands
    table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)gdpa(dev, "vkBindBufferMemory2KHR");
    table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)gdpa(dev, "vkBindImageMemory2KHR");

    // ---- VK_KHR_maintenance3 extension commands
    table->GetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR)gdpa(dev, "vkGetDescriptorSetLayoutSupportKHR");

    // ---- VK_KHR_draw_indirect_count extension commands
    table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)gdpa(dev, "vkCmdDrawIndirectCountKHR");
    table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)gdpa(dev, "vkCmdDrawIndexedIndirectCountKHR");

    // ---- VK_KHR_timeline_semaphore extension commands
    table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)gdpa(dev, "vkGetSemaphoreCounterValueKHR");
    table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)gdpa(dev, "vkWaitSemaphoresKHR");
    table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)gdpa(dev, "vkSignalSemaphoreKHR");

    // ---- VK_KHR_fragment_shading_rate extension commands
    table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)gdpa(dev, "vkCmdSetFragmentShadingRateKHR");

    // ---- VK_KHR_dynamic_rendering_local_read extension commands
    table->CmdSetRenderingAttachmentLocationsKHR = (PFN_vkCmdSetRenderingAttachmentLocationsKHR)gdpa(dev, "vkCmdSetRenderingAttachmentLocationsKHR");
    table->CmdSetRenderingInputAttachmentIndicesKHR = (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)gdpa(dev, "vkCmdSetRenderingInputAttachmentIndicesKHR");

    // ---- VK_KHR_present_wait extension commands
    table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)gdpa(dev, "vkWaitForPresentKHR");

    // ---- VK_KHR_buffer_device_address extension commands
    table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)gdpa(dev, "vkGetBufferDeviceAddressKHR");
    table->GetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR)gdpa(dev, "vkGetBufferOpaqueCaptureAddressKHR");
    table->GetDeviceMemoryOpaqueCaptureAddressKHR = (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)gdpa(dev, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");

    // ---- VK_KHR_deferred_host_operations extension commands
    table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)gdpa(dev, "vkCreateDeferredOperationKHR");
    table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)gdpa(dev, "vkDestroyDeferredOperationKHR");
    table->GetDeferredOperationMaxConcurrencyKHR = (PFN_vkGetDeferredOperationMaxConcurrencyKHR)gdpa(dev, "vkGetDeferredOperationMaxConcurrencyKHR");
    table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)gdpa(dev, "vkGetDeferredOperationResultKHR");
    table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)gdpa(dev, "vkDeferredOperationJoinKHR");

    // ---- VK_KHR_pipeline_executable_properties extension commands
    table->GetPipelineExecutablePropertiesKHR = (PFN_vkGetPipelineExecutablePropertiesKHR)gdpa(dev, "vkGetPipelineExecutablePropertiesKHR");
    table->GetPipelineExecutableStatisticsKHR = (PFN_vkGetPipelineExecutableStatisticsKHR)gdpa(dev, "vkGetPipelineExecutableStatisticsKHR");
    table->GetPipelineExecutableInternalRepresentationsKHR = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)gdpa(dev, "vkGetPipelineExecutableInternalRepresentationsKHR");

    // ---- VK_KHR_map_memory2 extension commands
    table->MapMemory2KHR = (PFN_vkMapMemory2KHR)gdpa(dev, "vkMapMemory2KHR");
    table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)gdpa(dev, "vkUnmapMemory2KHR");

    // ---- VK_KHR_video_encode_queue extension commands
    table->GetEncodedVideoSessionParametersKHR = (PFN_vkGetEncodedVideoSessionParametersKHR)gdpa(dev, "vkGetEncodedVideoSessionParametersKHR");
    table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)gdpa(dev, "vkCmdEncodeVideoKHR");

    // ---- VK_KHR_synchronization2 extension commands
    table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)gdpa(dev, "vkCmdSetEvent2KHR");
    table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)gdpa(dev, "vkCmdResetEvent2KHR");
    table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)gdpa(dev, "vkCmdWaitEvents2KHR");
    table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)gdpa(dev, "vkCmdPipelineBarrier2KHR");
    table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)gdpa(dev, "vkCmdWriteTimestamp2KHR");
    table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)gdpa(dev, "vkQueueSubmit2KHR");

    // ---- VK_KHR_copy_commands2 extension commands
    table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)gdpa(dev, "vkCmdCopyBuffer2KHR");
    table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)gdpa(dev, "vkCmdCopyImage2KHR");
    table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)gdpa(dev, "vkCmdCopyBufferToImage2KHR");
    table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)gdpa(dev, "vkCmdCopyImageToBuffer2KHR");
    table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)gdpa(dev, "vkCmdBlitImage2KHR");
    table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)gdpa(dev, "vkCmdResolveImage2KHR");

    // ---- VK_KHR_ray_tracing_maintenance1 extension commands
    table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)gdpa(dev, "vkCmdTraceRaysIndirect2KHR");

    // ---- VK_KHR_maintenance4 extension commands
    table->GetDeviceBufferMemoryRequirementsKHR = (PFN_vkGetDeviceBufferMemoryRequirementsKHR)gdpa(dev, "vkGetDeviceBufferMemoryRequirementsKHR");
    table->GetDeviceImageMemoryRequirementsKHR = (PFN_vkGetDeviceImageMemoryRequirementsKHR)gdpa(dev, "vkGetDeviceImageMemoryRequirementsKHR");
    table->GetDeviceImageSparseMemoryRequirementsKHR = (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)gdpa(dev, "vkGetDeviceImageSparseMemoryRequirementsKHR");

    // ---- VK_KHR_maintenance5 extension commands
    table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)gdpa(dev, "vkCmdBindIndexBuffer2KHR");
    table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)gdpa(dev, "vkGetRenderingAreaGranularityKHR");
    table->GetDeviceImageSubresourceLayoutKHR = (PFN_vkGetDeviceImageSubresourceLayoutKHR)gdpa(dev, "vkGetDeviceImageSubresourceLayoutKHR");
    table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)gdpa(dev, "vkGetImageSubresourceLayout2KHR");

    // ---- VK_KHR_pipeline_binary extension commands
    table->CreatePipelineBinariesKHR = (PFN_vkCreatePipelineBinariesKHR)gdpa(dev, "vkCreatePipelineBinariesKHR");
    table->DestroyPipelineBinaryKHR = (PFN_vkDestroyPipelineBinaryKHR)gdpa(dev, "vkDestroyPipelineBinaryKHR");
    table->GetPipelineKeyKHR = (PFN_vkGetPipelineKeyKHR)gdpa(dev, "vkGetPipelineKeyKHR");
    table->GetPipelineBinaryDataKHR = (PFN_vkGetPipelineBinaryDataKHR)gdpa(dev, "vkGetPipelineBinaryDataKHR");
    table->ReleaseCapturedPipelineDataKHR = (PFN_vkReleaseCapturedPipelineDataKHR)gdpa(dev, "vkReleaseCapturedPipelineDataKHR");

    // ---- VK_KHR_line_rasterization extension commands
    table->CmdSetLineStippleKHR = (PFN_vkCmdSetLineStippleKHR)gdpa(dev, "vkCmdSetLineStippleKHR");

    // ---- VK_KHR_calibrated_timestamps extension commands
    table->GetCalibratedTimestampsKHR = (PFN_vkGetCalibratedTimestampsKHR)gdpa(dev, "vkGetCalibratedTimestampsKHR");

    // ---- VK_KHR_maintenance6 extension commands
    table->CmdBindDescriptorSets2KHR = (PFN_vkCmdBindDescriptorSets2KHR)gdpa(dev, "vkCmdBindDescriptorSets2KHR");
    table->CmdPushConstants2KHR = (PFN_vkCmdPushConstants2KHR)gdpa(dev, "vkCmdPushConstants2KHR");
    table->CmdPushDescriptorSet2KHR = (PFN_vkCmdPushDescriptorSet2KHR)gdpa(dev, "vkCmdPushDescriptorSet2KHR");
    table->CmdPushDescriptorSetWithTemplate2KHR = (PFN_vkCmdPushDescriptorSetWithTemplate2KHR)gdpa(dev, "vkCmdPushDescriptorSetWithTemplate2KHR");
    table->CmdSetDescriptorBufferOffsets2EXT = (PFN_vkCmdSetDescriptorBufferOffsets2EXT)gdpa(dev, "vkCmdSetDescriptorBufferOffsets2EXT");
    table->CmdBindDescriptorBufferEmbeddedSamplers2EXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)gdpa(dev, "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");

    // ---- VK_EXT_debug_marker extension commands
    table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)gdpa(dev, "vkDebugMarkerSetObjectTagEXT");
    table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)gdpa(dev, "vkDebugMarkerSetObjectNameEXT");
    table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)gdpa(dev, "vkCmdDebugMarkerBeginEXT");
    table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)gdpa(dev, "vkCmdDebugMarkerEndEXT");
    table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)gdpa(dev, "vkCmdDebugMarkerInsertEXT");

    // ---- VK_EXT_transform_feedback extension commands
    table->CmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT)gdpa(dev, "vkCmdBindTransformFeedbackBuffersEXT");
    table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)gdpa(dev, "vkCmdBeginTransformFeedbackEXT");
    table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)gdpa(dev, "vkCmdEndTransformFeedbackEXT");
    table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)gdpa(dev, "vkCmdBeginQueryIndexedEXT");
    table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)gdpa(dev, "vkCmdEndQueryIndexedEXT");
    table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)gdpa(dev, "vkCmdDrawIndirectByteCountEXT");

    // ---- VK_NVX_binary_import extension commands
    table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)gdpa(dev, "vkCreateCuModuleNVX");
    table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)gdpa(dev, "vkCreateCuFunctionNVX");
    table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)gdpa(dev, "vkDestroyCuModuleNVX");
    table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)gdpa(dev, "vkDestroyCuFunctionNVX");
    table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)gdpa(dev, "vkCmdCuLaunchKernelNVX");

    // ---- VK_NVX_image_view_handle extension commands
    table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)gdpa(dev, "vkGetImageViewHandleNVX");
    table->GetImageViewHandle64NVX = (PFN_vkGetImageViewHandle64NVX)gdpa(dev, "vkGetImageViewHandle64NVX");
    table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)gdpa(dev, "vkGetImageViewAddressNVX");

    // ---- VK_AMD_draw_indirect_count extension commands
    table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)gdpa(dev, "vkCmdDrawIndirectCountAMD");
    table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)gdpa(dev, "vkCmdDrawIndexedIndirectCountAMD");

    // ---- VK_AMD_shader_info extension commands
    table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)gdpa(dev, "vkGetShaderInfoAMD");

    // ---- VK_NV_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)gdpa(dev, "vkGetMemoryWin32HandleNV");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_conditional_rendering extension commands
    table->CmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)gdpa(dev, "vkCmdBeginConditionalRenderingEXT");
    table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)gdpa(dev, "vkCmdEndConditionalRenderingEXT");

    // ---- VK_NV_clip_space_w_scaling extension commands
    table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)gdpa(dev, "vkCmdSetViewportWScalingNV");

    // ---- VK_EXT_display_control extension commands
    table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)gdpa(dev, "vkDisplayPowerControlEXT");
    table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)gdpa(dev, "vkRegisterDeviceEventEXT");
    table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)gdpa(dev, "vkRegisterDisplayEventEXT");
    table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)gdpa(dev, "vkGetSwapchainCounterEXT");

    // ---- VK_GOOGLE_display_timing extension commands
    table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)gdpa(dev, "vkGetRefreshCycleDurationGOOGLE");
    table->GetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)gdpa(dev, "vkGetPastPresentationTimingGOOGLE");

    // ---- VK_EXT_discard_rectangles extension commands
    table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)gdpa(dev, "vkCmdSetDiscardRectangleEXT");
    table->CmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT)gdpa(dev, "vkCmdSetDiscardRectangleEnableEXT");
    table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)gdpa(dev, "vkCmdSetDiscardRectangleModeEXT");

    // ---- VK_EXT_hdr_metadata extension commands
    table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)gdpa(dev, "vkSetHdrMetadataEXT");

    // ---- VK_EXT_debug_utils extension commands
    table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)gipa(inst, "vkSetDebugUtilsObjectNameEXT");
    table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)gipa(inst, "vkSetDebugUtilsObjectTagEXT");
    table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)gipa(inst, "vkQueueBeginDebugUtilsLabelEXT");
    table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)gipa(inst, "vkQueueEndDebugUtilsLabelEXT");
    table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)gipa(inst, "vkQueueInsertDebugUtilsLabelEXT");
    table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)gipa(inst, "vkCmdBeginDebugUtilsLabelEXT");
    table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)gipa(inst, "vkCmdEndDebugUtilsLabelEXT");
    table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)gipa(inst, "vkCmdInsertDebugUtilsLabelEXT");

    // ---- VK_ANDROID_external_memory_android_hardware_buffer extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    table->GetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)gdpa(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    table->GetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID)gdpa(dev, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_AMDX_shader_enqueue extension commands
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->CreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX)gdpa(dev, "vkCreateExecutionGraphPipelinesAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->GetExecutionGraphPipelineScratchSizeAMDX = (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)gdpa(dev, "vkGetExecutionGraphPipelineScratchSizeAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->GetExecutionGraphPipelineNodeIndexAMDX = (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)gdpa(dev, "vkGetExecutionGraphPipelineNodeIndexAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->CmdInitializeGraphScratchMemoryAMDX = (PFN_vkCmdInitializeGraphScratchMemoryAMDX)gdpa(dev, "vkCmdInitializeGraphScratchMemoryAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)gdpa(dev, "vkCmdDispatchGraphAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)gdpa(dev, "vkCmdDispatchGraphIndirectAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    table->CmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX)gdpa(dev, "vkCmdDispatchGraphIndirectCountAMDX");
#endif // VK_ENABLE_BETA_EXTENSIONS

    // ---- VK_EXT_sample_locations extension commands
    table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)gdpa(dev, "vkCmdSetSampleLocationsEXT");

    // ---- VK_EXT_image_drm_format_modifier extension commands
    table->GetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)gdpa(dev, "vkGetImageDrmFormatModifierPropertiesEXT");

    // ---- VK_EXT_validation_cache extension commands
    table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)gdpa(dev, "vkCreateValidationCacheEXT");
    table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)gdpa(dev, "vkDestroyValidationCacheEXT");
    table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)gdpa(dev, "vkMergeValidationCachesEXT");
    table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)gdpa(dev, "vkGetValidationCacheDataEXT");

    // ---- VK_NV_shading_rate_image extension commands
    table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)gdpa(dev, "vkCmdBindShadingRateImageNV");
    table->CmdSetViewportShadingRatePaletteNV = (PFN_vkCmdSetViewportShadingRatePaletteNV)gdpa(dev, "vkCmdSetViewportShadingRatePaletteNV");
    table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)gdpa(dev, "vkCmdSetCoarseSampleOrderNV");

    // ---- VK_NV_ray_tracing extension commands
    table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)gdpa(dev, "vkCreateAccelerationStructureNV");
    table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)gdpa(dev, "vkDestroyAccelerationStructureNV");
    table->GetAccelerationStructureMemoryRequirementsNV = (PFN_vkGetAccelerationStructureMemoryRequirementsNV)gdpa(dev, "vkGetAccelerationStructureMemoryRequirementsNV");
    table->BindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)gdpa(dev, "vkBindAccelerationStructureMemoryNV");
    table->CmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)gdpa(dev, "vkCmdBuildAccelerationStructureNV");
    table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)gdpa(dev, "vkCmdCopyAccelerationStructureNV");
    table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)gdpa(dev, "vkCmdTraceRaysNV");
    table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)gdpa(dev, "vkCreateRayTracingPipelinesNV");
    table->GetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)gdpa(dev, "vkGetRayTracingShaderGroupHandlesKHR");
    table->GetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV)gdpa(dev, "vkGetRayTracingShaderGroupHandlesNV");
    table->GetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)gdpa(dev, "vkGetAccelerationStructureHandleNV");
    table->CmdWriteAccelerationStructuresPropertiesNV = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)gdpa(dev, "vkCmdWriteAccelerationStructuresPropertiesNV");
    table->CompileDeferredNV = (PFN_vkCompileDeferredNV)gdpa(dev, "vkCompileDeferredNV");

    // ---- VK_EXT_external_memory_host extension commands
    table->GetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)gdpa(dev, "vkGetMemoryHostPointerPropertiesEXT");

    // ---- VK_AMD_buffer_marker extension commands
    table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)gdpa(dev, "vkCmdWriteBufferMarkerAMD");
    table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)gdpa(dev, "vkCmdWriteBufferMarker2AMD");

    // ---- VK_EXT_calibrated_timestamps extension commands
    table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)gdpa(dev, "vkGetCalibratedTimestampsEXT");

    // ---- VK_NV_mesh_shader extension commands
    table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)gdpa(dev, "vkCmdDrawMeshTasksNV");
    table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)gdpa(dev, "vkCmdDrawMeshTasksIndirectNV");
    table->CmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)gdpa(dev, "vkCmdDrawMeshTasksIndirectCountNV");

    // ---- VK_NV_scissor_exclusive extension commands
    table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)gdpa(dev, "vkCmdSetExclusiveScissorEnableNV");
    table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)gdpa(dev, "vkCmdSetExclusiveScissorNV");

    // ---- VK_NV_device_diagnostic_checkpoints extension commands
    table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)gdpa(dev, "vkCmdSetCheckpointNV");
    table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)gdpa(dev, "vkGetQueueCheckpointDataNV");
    table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)gdpa(dev, "vkGetQueueCheckpointData2NV");

    // ---- VK_INTEL_performance_query extension commands
    table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)gdpa(dev, "vkInitializePerformanceApiINTEL");
    table->UninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)gdpa(dev, "vkUninitializePerformanceApiINTEL");
    table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)gdpa(dev, "vkCmdSetPerformanceMarkerINTEL");
    table->CmdSetPerformanceStreamMarkerINTEL = (PFN_vkCmdSetPerformanceStreamMarkerINTEL)gdpa(dev, "vkCmdSetPerformanceStreamMarkerINTEL");
    table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)gdpa(dev, "vkCmdSetPerformanceOverrideINTEL");
    table->AcquirePerformanceConfigurationINTEL = (PFN_vkAcquirePerformanceConfigurationINTEL)gdpa(dev, "vkAcquirePerformanceConfigurationINTEL");
    table->ReleasePerformanceConfigurationINTEL = (PFN_vkReleasePerformanceConfigurationINTEL)gdpa(dev, "vkReleasePerformanceConfigurationINTEL");
    table->QueueSetPerformanceConfigurationINTEL = (PFN_vkQueueSetPerformanceConfigurationINTEL)gdpa(dev, "vkQueueSetPerformanceConfigurationINTEL");
    table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)gdpa(dev, "vkGetPerformanceParameterINTEL");

    // ---- VK_AMD_display_native_hdr extension commands
    table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)gdpa(dev, "vkSetLocalDimmingAMD");

    // ---- VK_EXT_buffer_device_address extension commands
    table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)gdpa(dev, "vkGetBufferDeviceAddressEXT");

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->AcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)gdpa(dev, "vkAcquireFullScreenExclusiveModeEXT");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->ReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)gdpa(dev, "vkReleaseFullScreenExclusiveModeEXT");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)gdpa(dev, "vkGetDeviceGroupSurfacePresentModes2EXT");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_line_rasterization extension commands
    table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)gdpa(dev, "vkCmdSetLineStippleEXT");

    // ---- VK_EXT_host_query_reset extension commands
    table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)gdpa(dev, "vkResetQueryPoolEXT");

    // ---- VK_EXT_extended_dynamic_state extension commands
    table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)gdpa(dev, "vkCmdSetCullModeEXT");
    table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)gdpa(dev, "vkCmdSetFrontFaceEXT");
    table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)gdpa(dev, "vkCmdSetPrimitiveTopologyEXT");
    table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)gdpa(dev, "vkCmdSetViewportWithCountEXT");
    table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)gdpa(dev, "vkCmdSetScissorWithCountEXT");
    table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)gdpa(dev, "vkCmdBindVertexBuffers2EXT");
    table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)gdpa(dev, "vkCmdSetDepthTestEnableEXT");
    table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)gdpa(dev, "vkCmdSetDepthWriteEnableEXT");
    table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)gdpa(dev, "vkCmdSetDepthCompareOpEXT");
    table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)gdpa(dev, "vkCmdSetDepthBoundsTestEnableEXT");
    table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)gdpa(dev, "vkCmdSetStencilTestEnableEXT");
    table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)gdpa(dev, "vkCmdSetStencilOpEXT");

    // ---- VK_EXT_host_image_copy extension commands
    table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)gdpa(dev, "vkCopyMemoryToImageEXT");
    table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)gdpa(dev, "vkCopyImageToMemoryEXT");
    table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)gdpa(dev, "vkCopyImageToImageEXT");
    table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)gdpa(dev, "vkTransitionImageLayoutEXT");
    table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)gdpa(dev, "vkGetImageSubresourceLayout2EXT");

    // ---- VK_EXT_swapchain_maintenance1 extension commands
    table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)gdpa(dev, "vkReleaseSwapchainImagesEXT");

    // ---- VK_NV_device_generated_commands extension commands
    table->GetGeneratedCommandsMemoryRequirementsNV = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)gdpa(dev, "vkGetGeneratedCommandsMemoryRequirementsNV");
    table->CmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV)gdpa(dev, "vkCmdPreprocessGeneratedCommandsNV");
    table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)gdpa(dev, "vkCmdExecuteGeneratedCommandsNV");
    table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)gdpa(dev, "vkCmdBindPipelineShaderGroupNV");
    table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)gdpa(dev, "vkCreateIndirectCommandsLayoutNV");
    table->DestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV)gdpa(dev, "vkDestroyIndirectCommandsLayoutNV");

    // ---- VK_EXT_depth_bias_control extension commands
    table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)gdpa(dev, "vkCmdSetDepthBias2EXT");

    // ---- VK_EXT_private_data extension commands
    table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)gdpa(dev, "vkCreatePrivateDataSlotEXT");
    table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)gdpa(dev, "vkDestroyPrivateDataSlotEXT");
    table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)gdpa(dev, "vkSetPrivateDataEXT");
    table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)gdpa(dev, "vkGetPrivateDataEXT");

    // ---- VK_NV_cuda_kernel_launch extension commands
    table->CreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)gdpa(dev, "vkCreateCudaModuleNV");
    table->GetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)gdpa(dev, "vkGetCudaModuleCacheNV");
    table->CreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)gdpa(dev, "vkCreateCudaFunctionNV");
    table->DestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)gdpa(dev, "vkDestroyCudaModuleNV");
    table->DestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)gdpa(dev, "vkDestroyCudaFunctionNV");
    table->CmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)gdpa(dev, "vkCmdCudaLaunchKernelNV");

    // ---- VK_EXT_metal_objects extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)gdpa(dev, "vkExportMetalObjectsEXT");
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_descriptor_buffer extension commands
    table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)gdpa(dev, "vkGetDescriptorSetLayoutSizeEXT");
    table->GetDescriptorSetLayoutBindingOffsetEXT = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)gdpa(dev, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)gdpa(dev, "vkGetDescriptorEXT");
    table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)gdpa(dev, "vkCmdBindDescriptorBuffersEXT");
    table->CmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)gdpa(dev, "vkCmdSetDescriptorBufferOffsetsEXT");
    table->CmdBindDescriptorBufferEmbeddedSamplersEXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)gdpa(dev, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    table->GetBufferOpaqueCaptureDescriptorDataEXT = (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)gdpa(dev, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    table->GetImageOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)gdpa(dev, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    table->GetImageViewOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)gdpa(dev, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    table->GetSamplerOpaqueCaptureDescriptorDataEXT = (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)gdpa(dev, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT = (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)gdpa(dev, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");

    // ---- VK_NV_fragment_shading_rate_enums extension commands
    table->CmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV)gdpa(dev, "vkCmdSetFragmentShadingRateEnumNV");

    // ---- VK_EXT_device_fault extension commands
    table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)gdpa(dev, "vkGetDeviceFaultInfoEXT");

    // ---- VK_EXT_vertex_input_dynamic_state extension commands
    table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)gdpa(dev, "vkCmdSetVertexInputEXT");

    // ---- VK_FUCHSIA_external_memory extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)gdpa(dev, "vkGetMemoryZirconHandleFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->GetMemoryZirconHandlePropertiesFUCHSIA = (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)gdpa(dev, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_external_semaphore extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->ImportSemaphoreZirconHandleFUCHSIA = (PFN_vkImportSemaphoreZirconHandleFUCHSIA)gdpa(dev, "vkImportSemaphoreZirconHandleFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->GetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA)gdpa(dev, "vkGetSemaphoreZirconHandleFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_buffer_collection extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)gdpa(dev, "vkCreateBufferCollectionFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->SetBufferCollectionImageConstraintsFUCHSIA = (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)gdpa(dev, "vkSetBufferCollectionImageConstraintsFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->SetBufferCollectionBufferConstraintsFUCHSIA = (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)gdpa(dev, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)gdpa(dev, "vkDestroyBufferCollectionFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->GetBufferCollectionPropertiesFUCHSIA = (PFN_vkGetBufferCollectionPropertiesFUCHSIA)gdpa(dev, "vkGetBufferCollectionPropertiesFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_HUAWEI_subpass_shading extension commands
    table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI = (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)gdpa(dev, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)gdpa(dev, "vkCmdSubpassShadingHUAWEI");

    // ---- VK_HUAWEI_invocation_mask extension commands
    table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)gdpa(dev, "vkCmdBindInvocationMaskHUAWEI");

    // ---- VK_NV_external_memory_rdma extension commands
    table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)gdpa(dev, "vkGetMemoryRemoteAddressNV");

    // ---- VK_EXT_pipeline_properties extension commands
    table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)gdpa(dev, "vkGetPipelinePropertiesEXT");

    // ---- VK_EXT_extended_dynamic_state2 extension commands
    table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)gdpa(dev, "vkCmdSetPatchControlPointsEXT");
    table->CmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT)gdpa(dev, "vkCmdSetRasterizerDiscardEnableEXT");
    table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)gdpa(dev, "vkCmdSetDepthBiasEnableEXT");
    table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)gdpa(dev, "vkCmdSetLogicOpEXT");
    table->CmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT)gdpa(dev, "vkCmdSetPrimitiveRestartEnableEXT");

    // ---- VK_EXT_color_write_enable extension commands
    table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)gdpa(dev, "vkCmdSetColorWriteEnableEXT");

    // ---- VK_EXT_multi_draw extension commands
    table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)gdpa(dev, "vkCmdDrawMultiEXT");
    table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)gdpa(dev, "vkCmdDrawMultiIndexedEXT");

    // ---- VK_EXT_opacity_micromap extension commands
    table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)gdpa(dev, "vkCreateMicromapEXT");
    table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)gdpa(dev, "vkDestroyMicromapEXT");
    table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)gdpa(dev, "vkCmdBuildMicromapsEXT");
    table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)gdpa(dev, "vkBuildMicromapsEXT");
    table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)gdpa(dev, "vkCopyMicromapEXT");
    table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)gdpa(dev, "vkCopyMicromapToMemoryEXT");
    table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)gdpa(dev, "vkCopyMemoryToMicromapEXT");
    table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)gdpa(dev, "vkWriteMicromapsPropertiesEXT");
    table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)gdpa(dev, "vkCmdCopyMicromapEXT");
    table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)gdpa(dev, "vkCmdCopyMicromapToMemoryEXT");
    table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)gdpa(dev, "vkCmdCopyMemoryToMicromapEXT");
    table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)gdpa(dev, "vkCmdWriteMicromapsPropertiesEXT");
    table->GetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT)gdpa(dev, "vkGetDeviceMicromapCompatibilityEXT");
    table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)gdpa(dev, "vkGetMicromapBuildSizesEXT");

    // ---- VK_HUAWEI_cluster_culling_shader extension commands
    table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)gdpa(dev, "vkCmdDrawClusterHUAWEI");
    table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)gdpa(dev, "vkCmdDrawClusterIndirectHUAWEI");

    // ---- VK_EXT_pageable_device_local_memory extension commands
    table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)gdpa(dev, "vkSetDeviceMemoryPriorityEXT");

    // ---- VK_VALVE_descriptor_set_host_mapping extension commands
    table->GetDescriptorSetLayoutHostMappingInfoVALVE = (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)gdpa(dev, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    table->GetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE)gdpa(dev, "vkGetDescriptorSetHostMappingVALVE");

    // ---- VK_NV_copy_memory_indirect extension commands
    table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)gdpa(dev, "vkCmdCopyMemoryIndirectNV");
    table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)gdpa(dev, "vkCmdCopyMemoryToImageIndirectNV");

    // ---- VK_NV_memory_decompression extension commands
    table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)gdpa(dev, "vkCmdDecompressMemoryNV");
    table->CmdDecompressMemoryIndirectCountNV = (PFN_vkCmdDecompressMemoryIndirectCountNV)gdpa(dev, "vkCmdDecompressMemoryIndirectCountNV");

    // ---- VK_NV_device_generated_commands_compute extension commands
    table->GetPipelineIndirectMemoryRequirementsNV = (PFN_vkGetPipelineIndirectMemoryRequirementsNV)gdpa(dev, "vkGetPipelineIndirectMemoryRequirementsNV");
    table->CmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV)gdpa(dev, "vkCmdUpdatePipelineIndirectBufferNV");
    table->GetPipelineIndirectDeviceAddressNV = (PFN_vkGetPipelineIndirectDeviceAddressNV)gdpa(dev, "vkGetPipelineIndirectDeviceAddressNV");

    // ---- VK_EXT_extended_dynamic_state3 extension commands
    table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)gdpa(dev, "vkCmdSetDepthClampEnableEXT");
    table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)gdpa(dev, "vkCmdSetPolygonModeEXT");
    table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)gdpa(dev, "vkCmdSetRasterizationSamplesEXT");
    table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)gdpa(dev, "vkCmdSetSampleMaskEXT");
    table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)gdpa(dev, "vkCmdSetAlphaToCoverageEnableEXT");
    table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)gdpa(dev, "vkCmdSetAlphaToOneEnableEXT");
    table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)gdpa(dev, "vkCmdSetLogicOpEnableEXT");
    table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)gdpa(dev, "vkCmdSetColorBlendEnableEXT");
    table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)gdpa(dev, "vkCmdSetColorBlendEquationEXT");
    table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)gdpa(dev, "vkCmdSetColorWriteMaskEXT");
    table->CmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)gdpa(dev, "vkCmdSetTessellationDomainOriginEXT");
    table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)gdpa(dev, "vkCmdSetRasterizationStreamEXT");
    table->CmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT)gdpa(dev, "vkCmdSetConservativeRasterizationModeEXT");
    table->CmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)gdpa(dev, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)gdpa(dev, "vkCmdSetDepthClipEnableEXT");
    table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)gdpa(dev, "vkCmdSetSampleLocationsEnableEXT");
    table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)gdpa(dev, "vkCmdSetColorBlendAdvancedEXT");
    table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)gdpa(dev, "vkCmdSetProvokingVertexModeEXT");
    table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)gdpa(dev, "vkCmdSetLineRasterizationModeEXT");
    table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)gdpa(dev, "vkCmdSetLineStippleEnableEXT");
    table->CmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)gdpa(dev, "vkCmdSetDepthClipNegativeOneToOneEXT");
    table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)gdpa(dev, "vkCmdSetViewportWScalingEnableNV");
    table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)gdpa(dev, "vkCmdSetViewportSwizzleNV");
    table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)gdpa(dev, "vkCmdSetCoverageToColorEnableNV");
    table->CmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV)gdpa(dev, "vkCmdSetCoverageToColorLocationNV");
    table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)gdpa(dev, "vkCmdSetCoverageModulationModeNV");
    table->CmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV)gdpa(dev, "vkCmdSetCoverageModulationTableEnableNV");
    table->CmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV)gdpa(dev, "vkCmdSetCoverageModulationTableNV");
    table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)gdpa(dev, "vkCmdSetShadingRateImageEnableNV");
    table->CmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)gdpa(dev, "vkCmdSetRepresentativeFragmentTestEnableNV");
    table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)gdpa(dev, "vkCmdSetCoverageReductionModeNV");

    // ---- VK_EXT_shader_module_identifier extension commands
    table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)gdpa(dev, "vkGetShaderModuleIdentifierEXT");
    table->GetShaderModuleCreateInfoIdentifierEXT = (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)gdpa(dev, "vkGetShaderModuleCreateInfoIdentifierEXT");

    // ---- VK_NV_optical_flow extension commands
    table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)gdpa(dev, "vkCreateOpticalFlowSessionNV");
    table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)gdpa(dev, "vkDestroyOpticalFlowSessionNV");
    table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)gdpa(dev, "vkBindOpticalFlowSessionImageNV");
    table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)gdpa(dev, "vkCmdOpticalFlowExecuteNV");

    // ---- VK_AMD_anti_lag extension commands
    table->AntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD)gdpa(dev, "vkAntiLagUpdateAMD");

    // ---- VK_EXT_shader_object extension commands
    table->CreateShadersEXT = (PFN_vkCreateShadersEXT)gdpa(dev, "vkCreateShadersEXT");
    table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)gdpa(dev, "vkDestroyShaderEXT");
    table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)gdpa(dev, "vkGetShaderBinaryDataEXT");
    table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)gdpa(dev, "vkCmdBindShadersEXT");
    table->CmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT)gdpa(dev, "vkCmdSetDepthClampRangeEXT");

    // ---- VK_QCOM_tile_properties extension commands
    table->GetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM)gdpa(dev, "vkGetFramebufferTilePropertiesQCOM");
    table->GetDynamicRenderingTilePropertiesQCOM = (PFN_vkGetDynamicRenderingTilePropertiesQCOM)gdpa(dev, "vkGetDynamicRenderingTilePropertiesQCOM");

    // ---- VK_NV_cooperative_vector extension commands
    table->ConvertCooperativeVectorMatrixNV = (PFN_vkConvertCooperativeVectorMatrixNV)gdpa(dev, "vkConvertCooperativeVectorMatrixNV");
    table->CmdConvertCooperativeVectorMatrixNV = (PFN_vkCmdConvertCooperativeVectorMatrixNV)gdpa(dev, "vkCmdConvertCooperativeVectorMatrixNV");

    // ---- VK_NV_low_latency2 extension commands
    table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)gdpa(dev, "vkSetLatencySleepModeNV");
    table->LatencySleepNV = (PFN_vkLatencySleepNV)gdpa(dev, "vkLatencySleepNV");
    table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)gdpa(dev, "vkSetLatencyMarkerNV");
    table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)gdpa(dev, "vkGetLatencyTimingsNV");
    table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)gdpa(dev, "vkQueueNotifyOutOfBandNV");

    // ---- VK_EXT_attachment_feedback_loop_dynamic_state extension commands
    table->CmdSetAttachmentFeedbackLoopEnableEXT = (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)gdpa(dev, "vkCmdSetAttachmentFeedbackLoopEnableEXT");

    // ---- VK_QNX_external_memory_screen_buffer extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)gdpa(dev, "vkGetScreenBufferPropertiesQNX");
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_cluster_acceleration_structure extension commands
    table->GetClusterAccelerationStructureBuildSizesNV = (PFN_vkGetClusterAccelerationStructureBuildSizesNV)gdpa(dev, "vkGetClusterAccelerationStructureBuildSizesNV");
    table->CmdBuildClusterAccelerationStructureIndirectNV = (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV)gdpa(dev, "vkCmdBuildClusterAccelerationStructureIndirectNV");

    // ---- VK_NV_partitioned_acceleration_structure extension commands
    table->GetPartitionedAccelerationStructuresBuildSizesNV = (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV)gdpa(dev, "vkGetPartitionedAccelerationStructuresBuildSizesNV");
    table->CmdBuildPartitionedAccelerationStructuresNV = (PFN_vkCmdBuildPartitionedAccelerationStructuresNV)gdpa(dev, "vkCmdBuildPartitionedAccelerationStructuresNV");

    // ---- VK_EXT_device_generated_commands extension commands
    table->GetGeneratedCommandsMemoryRequirementsEXT = (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT)gdpa(dev, "vkGetGeneratedCommandsMemoryRequirementsEXT");
    table->CmdPreprocessGeneratedCommandsEXT = (PFN_vkCmdPreprocessGeneratedCommandsEXT)gdpa(dev, "vkCmdPreprocessGeneratedCommandsEXT");
    table->CmdExecuteGeneratedCommandsEXT = (PFN_vkCmdExecuteGeneratedCommandsEXT)gdpa(dev, "vkCmdExecuteGeneratedCommandsEXT");
    table->CreateIndirectCommandsLayoutEXT = (PFN_vkCreateIndirectCommandsLayoutEXT)gdpa(dev, "vkCreateIndirectCommandsLayoutEXT");
    table->DestroyIndirectCommandsLayoutEXT = (PFN_vkDestroyIndirectCommandsLayoutEXT)gdpa(dev, "vkDestroyIndirectCommandsLayoutEXT");
    table->CreateIndirectExecutionSetEXT = (PFN_vkCreateIndirectExecutionSetEXT)gdpa(dev, "vkCreateIndirectExecutionSetEXT");
    table->DestroyIndirectExecutionSetEXT = (PFN_vkDestroyIndirectExecutionSetEXT)gdpa(dev, "vkDestroyIndirectExecutionSetEXT");
    table->UpdateIndirectExecutionSetPipelineEXT = (PFN_vkUpdateIndirectExecutionSetPipelineEXT)gdpa(dev, "vkUpdateIndirectExecutionSetPipelineEXT");
    table->UpdateIndirectExecutionSetShaderEXT = (PFN_vkUpdateIndirectExecutionSetShaderEXT)gdpa(dev, "vkUpdateIndirectExecutionSetShaderEXT");

    // ---- VK_EXT_external_memory_metal extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    table->GetMemoryMetalHandleEXT = (PFN_vkGetMemoryMetalHandleEXT)gdpa(dev, "vkGetMemoryMetalHandleEXT");
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_METAL_EXT)
    table->GetMemoryMetalHandlePropertiesEXT = (PFN_vkGetMemoryMetalHandlePropertiesEXT)gdpa(dev, "vkGetMemoryMetalHandlePropertiesEXT");
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_KHR_acceleration_structure extension commands
    table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)gdpa(dev, "vkCreateAccelerationStructureKHR");
    table->DestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)gdpa(dev, "vkDestroyAccelerationStructureKHR");
    table->CmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)gdpa(dev, "vkCmdBuildAccelerationStructuresKHR");
    table->CmdBuildAccelerationStructuresIndirectKHR = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)gdpa(dev, "vkCmdBuildAccelerationStructuresIndirectKHR");
    table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)gdpa(dev, "vkBuildAccelerationStructuresKHR");
    table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)gdpa(dev, "vkCopyAccelerationStructureKHR");
    table->CopyAccelerationStructureToMemoryKHR = (PFN_vkCopyAccelerationStructureToMemoryKHR)gdpa(dev, "vkCopyAccelerationStructureToMemoryKHR");
    table->CopyMemoryToAccelerationStructureKHR = (PFN_vkCopyMemoryToAccelerationStructureKHR)gdpa(dev, "vkCopyMemoryToAccelerationStructureKHR");
    table->WriteAccelerationStructuresPropertiesKHR = (PFN_vkWriteAccelerationStructuresPropertiesKHR)gdpa(dev, "vkWriteAccelerationStructuresPropertiesKHR");
    table->CmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)gdpa(dev, "vkCmdCopyAccelerationStructureKHR");
    table->CmdCopyAccelerationStructureToMemoryKHR = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)gdpa(dev, "vkCmdCopyAccelerationStructureToMemoryKHR");
    table->CmdCopyMemoryToAccelerationStructureKHR = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)gdpa(dev, "vkCmdCopyMemoryToAccelerationStructureKHR");
    table->GetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)gdpa(dev, "vkGetAccelerationStructureDeviceAddressKHR");
    table->CmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)gdpa(dev, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    table->GetDeviceAccelerationStructureCompatibilityKHR = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)gdpa(dev, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    table->GetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)gdpa(dev, "vkGetAccelerationStructureBuildSizesKHR");

    // ---- VK_KHR_ray_tracing_pipeline extension commands
    table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)gdpa(dev, "vkCmdTraceRaysKHR");
    table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)gdpa(dev, "vkCreateRayTracingPipelinesKHR");
    table->GetRayTracingCaptureReplayShaderGroupHandlesKHR = (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)gdpa(dev, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)gdpa(dev, "vkCmdTraceRaysIndirectKHR");
    table->GetRayTracingShaderGroupStackSizeKHR = (PFN_vkGetRayTracingShaderGroupStackSizeKHR)gdpa(dev, "vkGetRayTracingShaderGroupStackSizeKHR");
    table->CmdSetRayTracingPipelineStackSizeKHR = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)gdpa(dev, "vkCmdSetRayTracingPipelineStackSizeKHR");

    // ---- VK_EXT_mesh_shader extension commands
    table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)gdpa(dev, "vkCmdDrawMeshTasksEXT");
    table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)gdpa(dev, "vkCmdDrawMeshTasksIndirectEXT");
    table->CmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)gdpa(dev, "vkCmdDrawMeshTasksIndirectCountEXT");
}

// Init Instance function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_instance_core_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,
                                                                    VkInstance inst) {

    // ---- Core Vulkan 1.0 commands
    table->DestroyInstance = (PFN_vkDestroyInstance)gpa(inst, "vkDestroyInstance");
    table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)gpa(inst, "vkEnumeratePhysicalDevices");
    table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)gpa(inst, "vkGetPhysicalDeviceFeatures");
    table->GetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)gpa(inst, "vkGetPhysicalDeviceFormatProperties");
    table->GetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)gpa(inst, "vkGetPhysicalDeviceImageFormatProperties");
    table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpa(inst, "vkGetPhysicalDeviceProperties");
    table->GetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)gpa(inst, "vkGetPhysicalDeviceQueueFamilyProperties");
    table->GetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)gpa(inst, "vkGetPhysicalDeviceMemoryProperties");
    table->GetInstanceProcAddr = gpa;
    table->EnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)gpa(inst, "vkEnumerateDeviceExtensionProperties");
    table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)gpa(inst, "vkEnumerateDeviceLayerProperties");
    table->GetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)gpa(inst, "vkGetPhysicalDeviceSparseImageFormatProperties");

    // ---- Core Vulkan 1.1 commands
    table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)gpa(inst, "vkEnumeratePhysicalDeviceGroups");
    table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)gpa(inst, "vkGetPhysicalDeviceFeatures2");
    table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)gpa(inst, "vkGetPhysicalDeviceProperties2");
    table->GetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)gpa(inst, "vkGetPhysicalDeviceFormatProperties2");
    table->GetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)gpa(inst, "vkGetPhysicalDeviceImageFormatProperties2");
    table->GetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)gpa(inst, "vkGetPhysicalDeviceQueueFamilyProperties2");
    table->GetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)gpa(inst, "vkGetPhysicalDeviceMemoryProperties2");
    table->GetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)gpa(inst, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    table->GetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)gpa(inst, "vkGetPhysicalDeviceExternalBufferProperties");
    table->GetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)gpa(inst, "vkGetPhysicalDeviceExternalFenceProperties");
    table->GetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)gpa(inst, "vkGetPhysicalDeviceExternalSemaphoreProperties");

    // ---- Core Vulkan 1.3 commands
    table->GetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)gpa(inst, "vkGetPhysicalDeviceToolProperties");
}

// Init Instance function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_instance_extension_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,
                                                                        VkInstance inst) {

    // ---- VK_KHR_surface extension commands
    table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)gpa(inst, "vkDestroySurfaceKHR");
    table->GetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)gpa(inst, "vkGetPhysicalDeviceSurfaceSupportKHR");
    table->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)gpa(inst, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    table->GetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)gpa(inst, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    table->GetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)gpa(inst, "vkGetPhysicalDeviceSurfacePresentModesKHR");

    // ---- VK_KHR_swapchain extension commands
    table->GetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)gpa(inst, "vkGetPhysicalDevicePresentRectanglesKHR");

    // ---- VK_KHR_display extension commands
    table->GetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)gpa(inst, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    table->GetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)gpa(inst, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    table->GetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)gpa(inst, "vkGetDisplayPlaneSupportedDisplaysKHR");
    table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)gpa(inst, "vkGetDisplayModePropertiesKHR");
    table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)gpa(inst, "vkCreateDisplayModeKHR");
    table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)gpa(inst, "vkGetDisplayPlaneCapabilitiesKHR");
    table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)gpa(inst, "vkCreateDisplayPlaneSurfaceKHR");

    // ---- VK_KHR_xlib_surface extension commands
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)gpa(inst, "vkCreateXlibSurfaceKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    table->GetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)gpa(inst, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR

    // ---- VK_KHR_xcb_surface extension commands
#if defined(VK_USE_PLATFORM_XCB_KHR)
    table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)gpa(inst, "vkCreateXcbSurfaceKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    table->GetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)gpa(inst, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR

    // ---- VK_KHR_wayland_surface extension commands
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)gpa(inst, "vkCreateWaylandSurfaceKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    table->GetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)gpa(inst, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR

    // ---- VK_KHR_android_surface extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)gpa(inst, "vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_KHR_win32_surface extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)gpa(inst, "vkCreateWin32SurfaceKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)gpa(inst, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_video_queue extension commands
    table->GetPhysicalDeviceVideoCapabilitiesKHR = (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)gpa(inst, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    table->GetPhysicalDeviceVideoFormatPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)gpa(inst, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)gpa(inst, "vkGetPhysicalDeviceFeatures2KHR");
    table->GetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)gpa(inst, "vkGetPhysicalDeviceProperties2KHR");
    table->GetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)gpa(inst, "vkGetPhysicalDeviceFormatProperties2KHR");
    table->GetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)gpa(inst, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    table->GetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)gpa(inst, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    table->GetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)gpa(inst, "vkGetPhysicalDeviceMemoryProperties2KHR");
    table->GetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)gpa(inst, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");

    // ---- VK_KHR_device_group_creation extension commands
    table->EnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)gpa(inst, "vkEnumeratePhysicalDeviceGroupsKHR");

    // ---- VK_KHR_external_memory_capabilities extension commands
    table->GetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)gpa(inst, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    table->GetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)gpa(inst, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");

    // ---- VK_KHR_external_fence_capabilities extension commands
    table->GetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)gpa(inst, "vkGetPhysicalDeviceExternalFencePropertiesKHR");

    // ---- VK_KHR_performance_query extension commands
    table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)gpa(inst, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)gpa(inst, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");

    // ---- VK_KHR_get_surface_capabilities2 extension commands
    table->GetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)gpa(inst, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    table->GetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)gpa(inst, "vkGetPhysicalDeviceSurfaceFormats2KHR");

    // ---- VK_KHR_get_display_properties2 extension commands
    table->GetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)gpa(inst, "vkGetPhysicalDeviceDisplayProperties2KHR");
    table->GetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)gpa(inst, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)gpa(inst, "vkGetDisplayModeProperties2KHR");
    table->GetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)gpa(inst, "vkGetDisplayPlaneCapabilities2KHR");

    // ---- VK_KHR_fragment_shading_rate extension commands
    table->GetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)gpa(inst, "vkGetPhysicalDeviceFragmentShadingRatesKHR");

    // ---- VK_KHR_video_encode_queue extension commands
    table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)gpa(inst, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");

    // ---- VK_KHR_cooperative_matrix extension commands
    table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)gpa(inst, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");

    // ---- VK_KHR_calibrated_timestamps extension commands
    table->GetPhysicalDeviceCalibrateableTimeDomainsKHR = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)gpa(inst, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR");

    // ---- VK_EXT_debug_report extension commands
    table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)gpa(inst, "vkCreateDebugReportCallbackEXT");
    table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)gpa(inst, "vkDestroyDebugReportCallbackEXT");
    table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)gpa(inst, "vkDebugReportMessageEXT");

    // ---- VK_GGP_stream_descriptor_surface extension commands
#if defined(VK_USE_PLATFORM_GGP)
    table->CreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)gpa(inst, "vkCreateStreamDescriptorSurfaceGGP");
#endif // VK_USE_PLATFORM_GGP

    // ---- VK_NV_external_memory_capabilities extension commands
    table->GetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)gpa(inst, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");

    // ---- VK_NN_vi_surface extension commands
#if defined(VK_USE_PLATFORM_VI_NN)
    table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)gpa(inst, "vkCreateViSurfaceNN");
#endif // VK_USE_PLATFORM_VI_NN

    // ---- VK_EXT_direct_mode_display extension commands
    table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)gpa(inst, "vkReleaseDisplayEXT");

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)gpa(inst, "vkAcquireXlibDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)gpa(inst, "vkGetRandROutputDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    table->GetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)gpa(inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");

    // ---- VK_MVK_ios_surface extension commands
#if defined(VK_USE_PLATFORM_IOS_MVK)
    table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)gpa(inst, "vkCreateIOSSurfaceMVK");
#endif // VK_USE_PLATFORM_IOS_MVK

    // ---- VK_MVK_macos_surface extension commands
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)gpa(inst, "vkCreateMacOSSurfaceMVK");
#endif // VK_USE_PLATFORM_MACOS_MVK

    // ---- VK_EXT_debug_utils extension commands
    table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)gpa(inst, "vkCreateDebugUtilsMessengerEXT");
    table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)gpa(inst, "vkDestroyDebugUtilsMessengerEXT");
    table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)gpa(inst, "vkSubmitDebugUtilsMessageEXT");

    // ---- VK_EXT_sample_locations extension commands
    table->GetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)gpa(inst, "vkGetPhysicalDeviceMultisamplePropertiesEXT");

    // ---- VK_EXT_calibrated_timestamps extension commands
    table->GetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)gpa(inst, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");

    // ---- VK_FUCHSIA_imagepipe_surface extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)gpa(inst, "vkCreateImagePipeSurfaceFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_EXT_metal_surface extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)gpa(inst, "vkCreateMetalSurfaceEXT");
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_tooling_info extension commands
    table->GetPhysicalDeviceToolPropertiesEXT = (PFN_vkGetPhysicalDeviceToolPropertiesEXT)gpa(inst, "vkGetPhysicalDeviceToolPropertiesEXT");

    // ---- VK_NV_cooperative_matrix extension commands
    table->GetPhysicalDeviceCooperativeMatrixPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)gpa(inst, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");

    // ---- VK_NV_coverage_reduction_mode extension commands
    table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)gpa(inst, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetPhysicalDeviceSurfacePresentModes2EXT = (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)gpa(inst, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_headless_surface extension commands
    table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)gpa(inst, "vkCreateHeadlessSurfaceEXT");

    // ---- VK_EXT_acquire_drm_display extension commands
    table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)gpa(inst, "vkAcquireDrmDisplayEXT");
    table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)gpa(inst, "vkGetDrmDisplayEXT");

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)gpa(inst, "vkAcquireWinrtDisplayNV");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)gpa(inst, "vkGetWinrtDisplayNV");
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_directfb_surface extension commands
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)gpa(inst, "vkCreateDirectFBSurfaceEXT");
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    table->GetPhysicalDeviceDirectFBPresentationSupportEXT = (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)gpa(inst, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

    // ---- VK_QNX_screen_surface extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)gpa(inst, "vkCreateScreenSurfaceQNX");
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    table->GetPhysicalDeviceScreenPresentationSupportQNX = (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)gpa(inst, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_optical_flow extension commands
    table->GetPhysicalDeviceOpticalFlowImageFormatsNV = (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)gpa(inst, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");

    // ---- VK_NV_cooperative_vector extension commands
    table->GetPhysicalDeviceCooperativeVectorPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)gpa(inst, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");

    // ---- VK_NV_cooperative_matrix2 extension commands
    table->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)gpa(inst, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
}

// Functions that required a terminator need to have a separate dispatch table which contains their corresponding
// device function. This is used in the terminators themselves.
void init_extension_device_proc_terminator_dispatch(struct loader_device *dev) {
    struct loader_device_terminator_dispatch* dispatch = &dev->loader_dispatch.extension_terminator_dispatch;
    PFN_vkGetDeviceProcAddr gpda = (PFN_vkGetDeviceProcAddr)dev->phys_dev_term->this_icd_term->dispatch.GetDeviceProcAddr;
    // ---- VK_KHR_swapchain extension commands
    if (dev->driver_extensions.khr_swapchain_enabled)
       dispatch->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gpda(dev->icd_device, "vkCreateSwapchainKHR");
    if (dev->driver_extensions.khr_swapchain_enabled)
       dispatch->GetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)gpda(dev->icd_device, "vkGetDeviceGroupSurfacePresentModesKHR");
    // ---- VK_KHR_display_swapchain extension commands
    if (dev->driver_extensions.khr_display_swapchain_enabled)
       dispatch->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)gpda(dev->icd_device, "vkCreateSharedSwapchainsKHR");
    // ---- VK_EXT_debug_marker extension commands
    if (dev->driver_extensions.ext_debug_marker_enabled)
       dispatch->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)gpda(dev->icd_device, "vkDebugMarkerSetObjectTagEXT");
    if (dev->driver_extensions.ext_debug_marker_enabled)
       dispatch->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)gpda(dev->icd_device, "vkDebugMarkerSetObjectNameEXT");
    // ---- VK_EXT_debug_utils extension commands
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)gpda(dev->icd_device, "vkSetDebugUtilsObjectNameEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)gpda(dev->icd_device, "vkSetDebugUtilsObjectTagEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)gpda(dev->icd_device, "vkQueueBeginDebugUtilsLabelEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)gpda(dev->icd_device, "vkQueueEndDebugUtilsLabelEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)gpda(dev->icd_device, "vkQueueInsertDebugUtilsLabelEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)gpda(dev->icd_device, "vkCmdBeginDebugUtilsLabelEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)gpda(dev->icd_device, "vkCmdEndDebugUtilsLabelEXT");
    if (dev->driver_extensions.ext_debug_utils_enabled)
       dispatch->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)gpda(dev->icd_device, "vkCmdInsertDebugUtilsLabelEXT");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    // ---- VK_EXT_full_screen_exclusive extension commands
    if (dev->driver_extensions.ext_full_screen_exclusive_enabled && (dev->driver_extensions.khr_device_group_enabled || dev->driver_extensions.version_1_1_enabled))
       dispatch->GetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)gpda(dev->icd_device, "vkGetDeviceGroupSurfacePresentModes2EXT");
#endif // VK_USE_PLATFORM_WIN32_KHR
}

// These are prototypes for functions that need their trampoline called in all circumstances.
// They are used in loader_lookup_device_dispatch_table but are defined afterwards.
    // ---- VK_EXT_debug_marker extension commands
VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo);
VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo);
    // ---- VK_EXT_debug_utils extension commands
VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo);
VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo);

// Device command lookup function
VKAPI_ATTR void* VKAPI_CALL loader_lookup_device_dispatch_table(const VkLayerDispatchTable *table, const char *name, bool* found_name) {
    if (!name || name[0] != 'v' || name[1] != 'k') {
        *found_name = false;
        return NULL;
    }

    name += 2;
    *found_name = true;
    struct loader_device* dev = (struct loader_device *)table;
    const struct loader_instance* inst = dev->phys_dev_term->this_icd_term->this_instance;
    uint32_t api_version = VK_MAKE_API_VERSION(0, inst->app_api_version.major, inst->app_api_version.minor, inst->app_api_version.patch);


    // ---- Core Vulkan 1.0 commands
    if (!strcmp(name, "GetDeviceProcAddr")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetDeviceProcAddr;
    }
    if (!strcmp(name, "DestroyDevice")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyDevice;
    }
    if (!strcmp(name, "GetDeviceQueue")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetDeviceQueue;
    }
    if (!strcmp(name, "QueueSubmit")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->QueueSubmit;
    }
    if (!strcmp(name, "QueueWaitIdle")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->QueueWaitIdle;
    }
    if (!strcmp(name, "DeviceWaitIdle")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DeviceWaitIdle;
    }
    if (!strcmp(name, "AllocateMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->AllocateMemory;
    }
    if (!strcmp(name, "FreeMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->FreeMemory;
    }
    if (!strcmp(name, "MapMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->MapMemory;
    }
    if (!strcmp(name, "UnmapMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->UnmapMemory;
    }
    if (!strcmp(name, "FlushMappedMemoryRanges")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->FlushMappedMemoryRanges;
    }
    if (!strcmp(name, "InvalidateMappedMemoryRanges")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->InvalidateMappedMemoryRanges;
    }
    if (!strcmp(name, "GetDeviceMemoryCommitment")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetDeviceMemoryCommitment;
    }
    if (!strcmp(name, "BindBufferMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->BindBufferMemory;
    }
    if (!strcmp(name, "BindImageMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->BindImageMemory;
    }
    if (!strcmp(name, "GetBufferMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetBufferMemoryRequirements;
    }
    if (!strcmp(name, "GetImageMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetImageMemoryRequirements;
    }
    if (!strcmp(name, "GetImageSparseMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetImageSparseMemoryRequirements;
    }
    if (!strcmp(name, "QueueBindSparse")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->QueueBindSparse;
    }
    if (!strcmp(name, "CreateFence")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateFence;
    }
    if (!strcmp(name, "DestroyFence")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyFence;
    }
    if (!strcmp(name, "ResetFences")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->ResetFences;
    }
    if (!strcmp(name, "GetFenceStatus")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetFenceStatus;
    }
    if (!strcmp(name, "WaitForFences")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->WaitForFences;
    }
    if (!strcmp(name, "CreateSemaphore")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateSemaphore;
    }
    if (!strcmp(name, "DestroySemaphore")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroySemaphore;
    }
    if (!strcmp(name, "CreateEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateEvent;
    }
    if (!strcmp(name, "DestroyEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyEvent;
    }
    if (!strcmp(name, "GetEventStatus")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetEventStatus;
    }
    if (!strcmp(name, "SetEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->SetEvent;
    }
    if (!strcmp(name, "ResetEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->ResetEvent;
    }
    if (!strcmp(name, "CreateQueryPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateQueryPool;
    }
    if (!strcmp(name, "DestroyQueryPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyQueryPool;
    }
    if (!strcmp(name, "GetQueryPoolResults")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetQueryPoolResults;
    }
    if (!strcmp(name, "CreateBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateBuffer;
    }
    if (!strcmp(name, "DestroyBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyBuffer;
    }
    if (!strcmp(name, "CreateBufferView")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateBufferView;
    }
    if (!strcmp(name, "DestroyBufferView")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyBufferView;
    }
    if (!strcmp(name, "CreateImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateImage;
    }
    if (!strcmp(name, "DestroyImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyImage;
    }
    if (!strcmp(name, "GetImageSubresourceLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetImageSubresourceLayout;
    }
    if (!strcmp(name, "CreateImageView")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateImageView;
    }
    if (!strcmp(name, "DestroyImageView")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyImageView;
    }
    if (!strcmp(name, "CreateShaderModule")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateShaderModule;
    }
    if (!strcmp(name, "DestroyShaderModule")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyShaderModule;
    }
    if (!strcmp(name, "CreatePipelineCache")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreatePipelineCache;
    }
    if (!strcmp(name, "DestroyPipelineCache")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyPipelineCache;
    }
    if (!strcmp(name, "GetPipelineCacheData")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetPipelineCacheData;
    }
    if (!strcmp(name, "MergePipelineCaches")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->MergePipelineCaches;
    }
    if (!strcmp(name, "CreateGraphicsPipelines")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateGraphicsPipelines;
    }
    if (!strcmp(name, "CreateComputePipelines")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateComputePipelines;
    }
    if (!strcmp(name, "DestroyPipeline")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyPipeline;
    }
    if (!strcmp(name, "CreatePipelineLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreatePipelineLayout;
    }
    if (!strcmp(name, "DestroyPipelineLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyPipelineLayout;
    }
    if (!strcmp(name, "CreateSampler")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateSampler;
    }
    if (!strcmp(name, "DestroySampler")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroySampler;
    }
    if (!strcmp(name, "CreateDescriptorSetLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateDescriptorSetLayout;
    }
    if (!strcmp(name, "DestroyDescriptorSetLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyDescriptorSetLayout;
    }
    if (!strcmp(name, "CreateDescriptorPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateDescriptorPool;
    }
    if (!strcmp(name, "DestroyDescriptorPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyDescriptorPool;
    }
    if (!strcmp(name, "ResetDescriptorPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->ResetDescriptorPool;
    }
    if (!strcmp(name, "AllocateDescriptorSets")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->AllocateDescriptorSets;
    }
    if (!strcmp(name, "FreeDescriptorSets")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->FreeDescriptorSets;
    }
    if (!strcmp(name, "UpdateDescriptorSets")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->UpdateDescriptorSets;
    }
    if (!strcmp(name, "CreateFramebuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateFramebuffer;
    }
    if (!strcmp(name, "DestroyFramebuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyFramebuffer;
    }
    if (!strcmp(name, "CreateRenderPass")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateRenderPass;
    }
    if (!strcmp(name, "DestroyRenderPass")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyRenderPass;
    }
    if (!strcmp(name, "GetRenderAreaGranularity")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->GetRenderAreaGranularity;
    }
    if (!strcmp(name, "CreateCommandPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CreateCommandPool;
    }
    if (!strcmp(name, "DestroyCommandPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->DestroyCommandPool;
    }
    if (!strcmp(name, "ResetCommandPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->ResetCommandPool;
    }
    if (!strcmp(name, "AllocateCommandBuffers")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->AllocateCommandBuffers;
    }
    if (!strcmp(name, "FreeCommandBuffers")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->FreeCommandBuffers;
    }
    if (!strcmp(name, "BeginCommandBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->BeginCommandBuffer;
    }
    if (!strcmp(name, "EndCommandBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->EndCommandBuffer;
    }
    if (!strcmp(name, "ResetCommandBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->ResetCommandBuffer;
    }
    if (!strcmp(name, "CmdBindPipeline")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBindPipeline;
    }
    if (!strcmp(name, "CmdSetViewport")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetViewport;
    }
    if (!strcmp(name, "CmdSetScissor")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetScissor;
    }
    if (!strcmp(name, "CmdSetLineWidth")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetLineWidth;
    }
    if (!strcmp(name, "CmdSetDepthBias")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetDepthBias;
    }
    if (!strcmp(name, "CmdSetBlendConstants")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetBlendConstants;
    }
    if (!strcmp(name, "CmdSetDepthBounds")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetDepthBounds;
    }
    if (!strcmp(name, "CmdSetStencilCompareMask")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetStencilCompareMask;
    }
    if (!strcmp(name, "CmdSetStencilWriteMask")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetStencilWriteMask;
    }
    if (!strcmp(name, "CmdSetStencilReference")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetStencilReference;
    }
    if (!strcmp(name, "CmdBindDescriptorSets")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBindDescriptorSets;
    }
    if (!strcmp(name, "CmdBindIndexBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBindIndexBuffer;
    }
    if (!strcmp(name, "CmdBindVertexBuffers")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBindVertexBuffers;
    }
    if (!strcmp(name, "CmdDraw")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDraw;
    }
    if (!strcmp(name, "CmdDrawIndexed")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDrawIndexed;
    }
    if (!strcmp(name, "CmdDrawIndirect")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDrawIndirect;
    }
    if (!strcmp(name, "CmdDrawIndexedIndirect")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDrawIndexedIndirect;
    }
    if (!strcmp(name, "CmdDispatch")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDispatch;
    }
    if (!strcmp(name, "CmdDispatchIndirect")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdDispatchIndirect;
    }
    if (!strcmp(name, "CmdCopyBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdCopyBuffer;
    }
    if (!strcmp(name, "CmdCopyImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdCopyImage;
    }
    if (!strcmp(name, "CmdBlitImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBlitImage;
    }
    if (!strcmp(name, "CmdCopyBufferToImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdCopyBufferToImage;
    }
    if (!strcmp(name, "CmdCopyImageToBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdCopyImageToBuffer;
    }
    if (!strcmp(name, "CmdUpdateBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdUpdateBuffer;
    }
    if (!strcmp(name, "CmdFillBuffer")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdFillBuffer;
    }
    if (!strcmp(name, "CmdClearColorImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdClearColorImage;
    }
    if (!strcmp(name, "CmdClearDepthStencilImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdClearDepthStencilImage;
    }
    if (!strcmp(name, "CmdClearAttachments")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdClearAttachments;
    }
    if (!strcmp(name, "CmdResolveImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdResolveImage;
    }
    if (!strcmp(name, "CmdSetEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdSetEvent;
    }
    if (!strcmp(name, "CmdResetEvent")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdResetEvent;
    }
    if (!strcmp(name, "CmdWaitEvents")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdWaitEvents;
    }
    if (!strcmp(name, "CmdPipelineBarrier")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdPipelineBarrier;
    }
    if (!strcmp(name, "CmdBeginQuery")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBeginQuery;
    }
    if (!strcmp(name, "CmdEndQuery")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdEndQuery;
    }
    if (!strcmp(name, "CmdResetQueryPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdResetQueryPool;
    }
    if (!strcmp(name, "CmdWriteTimestamp")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdWriteTimestamp;
    }
    if (!strcmp(name, "CmdCopyQueryPoolResults")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdCopyQueryPoolResults;
    }
    if (!strcmp(name, "CmdPushConstants")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdPushConstants;
    }
    if (!strcmp(name, "CmdBeginRenderPass")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdBeginRenderPass;
    }
    if (!strcmp(name, "CmdNextSubpass")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdNextSubpass;
    }
    if (!strcmp(name, "CmdEndRenderPass")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdEndRenderPass;
    }
    if (!strcmp(name, "CmdExecuteCommands")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_0) return NULL;
        return (void *)table->CmdExecuteCommands;
    }

    // ---- Core Vulkan 1.1 commands
    if (!strcmp(name, "BindBufferMemory2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->BindBufferMemory2;
    }
    if (!strcmp(name, "BindImageMemory2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->BindImageMemory2;
    }
    if (!strcmp(name, "GetDeviceGroupPeerMemoryFeatures")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetDeviceGroupPeerMemoryFeatures;
    }
    if (!strcmp(name, "CmdSetDeviceMask")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->CmdSetDeviceMask;
    }
    if (!strcmp(name, "CmdDispatchBase")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->CmdDispatchBase;
    }
    if (!strcmp(name, "GetImageMemoryRequirements2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetImageMemoryRequirements2;
    }
    if (!strcmp(name, "GetBufferMemoryRequirements2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetBufferMemoryRequirements2;
    }
    if (!strcmp(name, "GetImageSparseMemoryRequirements2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetImageSparseMemoryRequirements2;
    }
    if (!strcmp(name, "TrimCommandPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->TrimCommandPool;
    }
    if (!strcmp(name, "GetDeviceQueue2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetDeviceQueue2;
    }
    if (!strcmp(name, "CreateSamplerYcbcrConversion")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->CreateSamplerYcbcrConversion;
    }
    if (!strcmp(name, "DestroySamplerYcbcrConversion")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->DestroySamplerYcbcrConversion;
    }
    if (!strcmp(name, "CreateDescriptorUpdateTemplate")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->CreateDescriptorUpdateTemplate;
    }
    if (!strcmp(name, "DestroyDescriptorUpdateTemplate")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->DestroyDescriptorUpdateTemplate;
    }
    if (!strcmp(name, "UpdateDescriptorSetWithTemplate")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->UpdateDescriptorSetWithTemplate;
    }
    if (!strcmp(name, "GetDescriptorSetLayoutSupport")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_1) return NULL;
        return (void *)table->GetDescriptorSetLayoutSupport;
    }

    // ---- Core Vulkan 1.2 commands
    if (!strcmp(name, "CmdDrawIndirectCount")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CmdDrawIndirectCount;
    }
    if (!strcmp(name, "CmdDrawIndexedIndirectCount")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CmdDrawIndexedIndirectCount;
    }
    if (!strcmp(name, "CreateRenderPass2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CreateRenderPass2;
    }
    if (!strcmp(name, "CmdBeginRenderPass2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CmdBeginRenderPass2;
    }
    if (!strcmp(name, "CmdNextSubpass2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CmdNextSubpass2;
    }
    if (!strcmp(name, "CmdEndRenderPass2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->CmdEndRenderPass2;
    }
    if (!strcmp(name, "ResetQueryPool")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->ResetQueryPool;
    }
    if (!strcmp(name, "GetSemaphoreCounterValue")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->GetSemaphoreCounterValue;
    }
    if (!strcmp(name, "WaitSemaphores")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->WaitSemaphores;
    }
    if (!strcmp(name, "SignalSemaphore")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->SignalSemaphore;
    }
    if (!strcmp(name, "GetBufferDeviceAddress")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->GetBufferDeviceAddress;
    }
    if (!strcmp(name, "GetBufferOpaqueCaptureAddress")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->GetBufferOpaqueCaptureAddress;
    }
    if (!strcmp(name, "GetDeviceMemoryOpaqueCaptureAddress")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_2) return NULL;
        return (void *)table->GetDeviceMemoryOpaqueCaptureAddress;
    }

    // ---- Core Vulkan 1.3 commands
    if (!strcmp(name, "CreatePrivateDataSlot")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CreatePrivateDataSlot;
    }
    if (!strcmp(name, "DestroyPrivateDataSlot")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->DestroyPrivateDataSlot;
    }
    if (!strcmp(name, "SetPrivateData")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->SetPrivateData;
    }
    if (!strcmp(name, "GetPrivateData")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->GetPrivateData;
    }
    if (!strcmp(name, "CmdSetEvent2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetEvent2;
    }
    if (!strcmp(name, "CmdResetEvent2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdResetEvent2;
    }
    if (!strcmp(name, "CmdWaitEvents2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdWaitEvents2;
    }
    if (!strcmp(name, "CmdPipelineBarrier2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdPipelineBarrier2;
    }
    if (!strcmp(name, "CmdWriteTimestamp2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdWriteTimestamp2;
    }
    if (!strcmp(name, "QueueSubmit2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->QueueSubmit2;
    }
    if (!strcmp(name, "CmdCopyBuffer2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdCopyBuffer2;
    }
    if (!strcmp(name, "CmdCopyImage2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdCopyImage2;
    }
    if (!strcmp(name, "CmdCopyBufferToImage2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdCopyBufferToImage2;
    }
    if (!strcmp(name, "CmdCopyImageToBuffer2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdCopyImageToBuffer2;
    }
    if (!strcmp(name, "CmdBlitImage2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdBlitImage2;
    }
    if (!strcmp(name, "CmdResolveImage2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdResolveImage2;
    }
    if (!strcmp(name, "CmdBeginRendering")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdBeginRendering;
    }
    if (!strcmp(name, "CmdEndRendering")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdEndRendering;
    }
    if (!strcmp(name, "CmdSetCullMode")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetCullMode;
    }
    if (!strcmp(name, "CmdSetFrontFace")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetFrontFace;
    }
    if (!strcmp(name, "CmdSetPrimitiveTopology")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetPrimitiveTopology;
    }
    if (!strcmp(name, "CmdSetViewportWithCount")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetViewportWithCount;
    }
    if (!strcmp(name, "CmdSetScissorWithCount")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetScissorWithCount;
    }
    if (!strcmp(name, "CmdBindVertexBuffers2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdBindVertexBuffers2;
    }
    if (!strcmp(name, "CmdSetDepthTestEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetDepthTestEnable;
    }
    if (!strcmp(name, "CmdSetDepthWriteEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetDepthWriteEnable;
    }
    if (!strcmp(name, "CmdSetDepthCompareOp")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetDepthCompareOp;
    }
    if (!strcmp(name, "CmdSetDepthBoundsTestEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetDepthBoundsTestEnable;
    }
    if (!strcmp(name, "CmdSetStencilTestEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetStencilTestEnable;
    }
    if (!strcmp(name, "CmdSetStencilOp")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetStencilOp;
    }
    if (!strcmp(name, "CmdSetRasterizerDiscardEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetRasterizerDiscardEnable;
    }
    if (!strcmp(name, "CmdSetDepthBiasEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetDepthBiasEnable;
    }
    if (!strcmp(name, "CmdSetPrimitiveRestartEnable")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->CmdSetPrimitiveRestartEnable;
    }
    if (!strcmp(name, "GetDeviceBufferMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->GetDeviceBufferMemoryRequirements;
    }
    if (!strcmp(name, "GetDeviceImageMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->GetDeviceImageMemoryRequirements;
    }
    if (!strcmp(name, "GetDeviceImageSparseMemoryRequirements")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_3) return NULL;
        return (void *)table->GetDeviceImageSparseMemoryRequirements;
    }

    // ---- Core Vulkan 1.4 commands
    if (!strcmp(name, "CmdSetLineStipple")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdSetLineStipple;
    }
    if (!strcmp(name, "MapMemory2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->MapMemory2;
    }
    if (!strcmp(name, "UnmapMemory2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->UnmapMemory2;
    }
    if (!strcmp(name, "CmdBindIndexBuffer2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdBindIndexBuffer2;
    }
    if (!strcmp(name, "GetRenderingAreaGranularity")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->GetRenderingAreaGranularity;
    }
    if (!strcmp(name, "GetDeviceImageSubresourceLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->GetDeviceImageSubresourceLayout;
    }
    if (!strcmp(name, "GetImageSubresourceLayout2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->GetImageSubresourceLayout2;
    }
    if (!strcmp(name, "CmdPushDescriptorSet")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdPushDescriptorSet;
    }
    if (!strcmp(name, "CmdPushDescriptorSetWithTemplate")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdPushDescriptorSetWithTemplate;
    }
    if (!strcmp(name, "CmdSetRenderingAttachmentLocations")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdSetRenderingAttachmentLocations;
    }
    if (!strcmp(name, "CmdSetRenderingInputAttachmentIndices")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdSetRenderingInputAttachmentIndices;
    }
    if (!strcmp(name, "CmdBindDescriptorSets2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdBindDescriptorSets2;
    }
    if (!strcmp(name, "CmdPushConstants2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdPushConstants2;
    }
    if (!strcmp(name, "CmdPushDescriptorSet2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdPushDescriptorSet2;
    }
    if (!strcmp(name, "CmdPushDescriptorSetWithTemplate2")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CmdPushDescriptorSetWithTemplate2;
    }
    if (!strcmp(name, "CopyMemoryToImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CopyMemoryToImage;
    }
    if (!strcmp(name, "CopyImageToMemory")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CopyImageToMemory;
    }
    if (!strcmp(name, "CopyImageToImage")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->CopyImageToImage;
    }
    if (!strcmp(name, "TransitionImageLayout")) {
        if (dev->should_ignore_device_commands_from_newer_version && api_version < VK_API_VERSION_1_4) return NULL;
        return (void *)table->TransitionImageLayout;
    }

    // ---- VK_KHR_swapchain extension commands
    if (!strcmp(name, "CreateSwapchainKHR")) return (void *)table->CreateSwapchainKHR;
    if (!strcmp(name, "DestroySwapchainKHR")) return (void *)table->DestroySwapchainKHR;
    if (!strcmp(name, "GetSwapchainImagesKHR")) return (void *)table->GetSwapchainImagesKHR;
    if (!strcmp(name, "AcquireNextImageKHR")) return (void *)table->AcquireNextImageKHR;
    if (!strcmp(name, "QueuePresentKHR")) return (void *)table->QueuePresentKHR;
    if (!strcmp(name, "GetDeviceGroupPresentCapabilitiesKHR")) return (void *)table->GetDeviceGroupPresentCapabilitiesKHR;
    if (!strcmp(name, "GetDeviceGroupSurfacePresentModesKHR")) return (void *)table->GetDeviceGroupSurfacePresentModesKHR;
    if (!strcmp(name, "AcquireNextImage2KHR")) return (void *)table->AcquireNextImage2KHR;

    // ---- VK_KHR_display_swapchain extension commands
    if (!strcmp(name, "CreateSharedSwapchainsKHR")) return (void *)table->CreateSharedSwapchainsKHR;

    // ---- VK_KHR_video_queue extension commands
    if (!strcmp(name, "CreateVideoSessionKHR")) return (void *)table->CreateVideoSessionKHR;
    if (!strcmp(name, "DestroyVideoSessionKHR")) return (void *)table->DestroyVideoSessionKHR;
    if (!strcmp(name, "GetVideoSessionMemoryRequirementsKHR")) return (void *)table->GetVideoSessionMemoryRequirementsKHR;
    if (!strcmp(name, "BindVideoSessionMemoryKHR")) return (void *)table->BindVideoSessionMemoryKHR;
    if (!strcmp(name, "CreateVideoSessionParametersKHR")) return (void *)table->CreateVideoSessionParametersKHR;
    if (!strcmp(name, "UpdateVideoSessionParametersKHR")) return (void *)table->UpdateVideoSessionParametersKHR;
    if (!strcmp(name, "DestroyVideoSessionParametersKHR")) return (void *)table->DestroyVideoSessionParametersKHR;
    if (!strcmp(name, "CmdBeginVideoCodingKHR")) return (void *)table->CmdBeginVideoCodingKHR;
    if (!strcmp(name, "CmdEndVideoCodingKHR")) return (void *)table->CmdEndVideoCodingKHR;
    if (!strcmp(name, "CmdControlVideoCodingKHR")) return (void *)table->CmdControlVideoCodingKHR;

    // ---- VK_KHR_video_decode_queue extension commands
    if (!strcmp(name, "CmdDecodeVideoKHR")) return (void *)table->CmdDecodeVideoKHR;

    // ---- VK_KHR_dynamic_rendering extension commands
    if (!strcmp(name, "CmdBeginRenderingKHR")) return (void *)table->CmdBeginRenderingKHR;
    if (!strcmp(name, "CmdEndRenderingKHR")) return (void *)table->CmdEndRenderingKHR;

    // ---- VK_KHR_device_group extension commands
    if (!strcmp(name, "GetDeviceGroupPeerMemoryFeaturesKHR")) return (void *)table->GetDeviceGroupPeerMemoryFeaturesKHR;
    if (!strcmp(name, "CmdSetDeviceMaskKHR")) return (void *)table->CmdSetDeviceMaskKHR;
    if (!strcmp(name, "CmdDispatchBaseKHR")) return (void *)table->CmdDispatchBaseKHR;

    // ---- VK_KHR_maintenance1 extension commands
    if (!strcmp(name, "TrimCommandPoolKHR")) return (void *)table->TrimCommandPoolKHR;

    // ---- VK_KHR_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetMemoryWin32HandleKHR")) return (void *)table->GetMemoryWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetMemoryWin32HandlePropertiesKHR")) return (void *)table->GetMemoryWin32HandlePropertiesKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_memory_fd extension commands
    if (!strcmp(name, "GetMemoryFdKHR")) return (void *)table->GetMemoryFdKHR;
    if (!strcmp(name, "GetMemoryFdPropertiesKHR")) return (void *)table->GetMemoryFdPropertiesKHR;

    // ---- VK_KHR_external_semaphore_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "ImportSemaphoreWin32HandleKHR")) return (void *)table->ImportSemaphoreWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetSemaphoreWin32HandleKHR")) return (void *)table->GetSemaphoreWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_semaphore_fd extension commands
    if (!strcmp(name, "ImportSemaphoreFdKHR")) return (void *)table->ImportSemaphoreFdKHR;
    if (!strcmp(name, "GetSemaphoreFdKHR")) return (void *)table->GetSemaphoreFdKHR;

    // ---- VK_KHR_push_descriptor extension commands
    if (!strcmp(name, "CmdPushDescriptorSetKHR")) return (void *)table->CmdPushDescriptorSetKHR;
    if (!strcmp(name, "CmdPushDescriptorSetWithTemplateKHR")) return (void *)table->CmdPushDescriptorSetWithTemplateKHR;

    // ---- VK_KHR_descriptor_update_template extension commands
    if (!strcmp(name, "CreateDescriptorUpdateTemplateKHR")) return (void *)table->CreateDescriptorUpdateTemplateKHR;
    if (!strcmp(name, "DestroyDescriptorUpdateTemplateKHR")) return (void *)table->DestroyDescriptorUpdateTemplateKHR;
    if (!strcmp(name, "UpdateDescriptorSetWithTemplateKHR")) return (void *)table->UpdateDescriptorSetWithTemplateKHR;

    // ---- VK_KHR_create_renderpass2 extension commands
    if (!strcmp(name, "CreateRenderPass2KHR")) return (void *)table->CreateRenderPass2KHR;
    if (!strcmp(name, "CmdBeginRenderPass2KHR")) return (void *)table->CmdBeginRenderPass2KHR;
    if (!strcmp(name, "CmdNextSubpass2KHR")) return (void *)table->CmdNextSubpass2KHR;
    if (!strcmp(name, "CmdEndRenderPass2KHR")) return (void *)table->CmdEndRenderPass2KHR;

    // ---- VK_KHR_shared_presentable_image extension commands
    if (!strcmp(name, "GetSwapchainStatusKHR")) return (void *)table->GetSwapchainStatusKHR;

    // ---- VK_KHR_external_fence_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "ImportFenceWin32HandleKHR")) return (void *)table->ImportFenceWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetFenceWin32HandleKHR")) return (void *)table->GetFenceWin32HandleKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_fence_fd extension commands
    if (!strcmp(name, "ImportFenceFdKHR")) return (void *)table->ImportFenceFdKHR;
    if (!strcmp(name, "GetFenceFdKHR")) return (void *)table->GetFenceFdKHR;

    // ---- VK_KHR_performance_query extension commands
    if (!strcmp(name, "AcquireProfilingLockKHR")) return (void *)table->AcquireProfilingLockKHR;
    if (!strcmp(name, "ReleaseProfilingLockKHR")) return (void *)table->ReleaseProfilingLockKHR;

    // ---- VK_KHR_get_memory_requirements2 extension commands
    if (!strcmp(name, "GetImageMemoryRequirements2KHR")) return (void *)table->GetImageMemoryRequirements2KHR;
    if (!strcmp(name, "GetBufferMemoryRequirements2KHR")) return (void *)table->GetBufferMemoryRequirements2KHR;
    if (!strcmp(name, "GetImageSparseMemoryRequirements2KHR")) return (void *)table->GetImageSparseMemoryRequirements2KHR;

    // ---- VK_KHR_sampler_ycbcr_conversion extension commands
    if (!strcmp(name, "CreateSamplerYcbcrConversionKHR")) return (void *)table->CreateSamplerYcbcrConversionKHR;
    if (!strcmp(name, "DestroySamplerYcbcrConversionKHR")) return (void *)table->DestroySamplerYcbcrConversionKHR;

    // ---- VK_KHR_bind_memory2 extension commands
    if (!strcmp(name, "BindBufferMemory2KHR")) return (void *)table->BindBufferMemory2KHR;
    if (!strcmp(name, "BindImageMemory2KHR")) return (void *)table->BindImageMemory2KHR;

    // ---- VK_KHR_maintenance3 extension commands
    if (!strcmp(name, "GetDescriptorSetLayoutSupportKHR")) return (void *)table->GetDescriptorSetLayoutSupportKHR;

    // ---- VK_KHR_draw_indirect_count extension commands
    if (!strcmp(name, "CmdDrawIndirectCountKHR")) return (void *)table->CmdDrawIndirectCountKHR;
    if (!strcmp(name, "CmdDrawIndexedIndirectCountKHR")) return (void *)table->CmdDrawIndexedIndirectCountKHR;

    // ---- VK_KHR_timeline_semaphore extension commands
    if (!strcmp(name, "GetSemaphoreCounterValueKHR")) return (void *)table->GetSemaphoreCounterValueKHR;
    if (!strcmp(name, "WaitSemaphoresKHR")) return (void *)table->WaitSemaphoresKHR;
    if (!strcmp(name, "SignalSemaphoreKHR")) return (void *)table->SignalSemaphoreKHR;

    // ---- VK_KHR_fragment_shading_rate extension commands
    if (!strcmp(name, "CmdSetFragmentShadingRateKHR")) return (void *)table->CmdSetFragmentShadingRateKHR;

    // ---- VK_KHR_dynamic_rendering_local_read extension commands
    if (!strcmp(name, "CmdSetRenderingAttachmentLocationsKHR")) return (void *)table->CmdSetRenderingAttachmentLocationsKHR;
    if (!strcmp(name, "CmdSetRenderingInputAttachmentIndicesKHR")) return (void *)table->CmdSetRenderingInputAttachmentIndicesKHR;

    // ---- VK_KHR_present_wait extension commands
    if (!strcmp(name, "WaitForPresentKHR")) return (void *)table->WaitForPresentKHR;

    // ---- VK_KHR_buffer_device_address extension commands
    if (!strcmp(name, "GetBufferDeviceAddressKHR")) return (void *)table->GetBufferDeviceAddressKHR;
    if (!strcmp(name, "GetBufferOpaqueCaptureAddressKHR")) return (void *)table->GetBufferOpaqueCaptureAddressKHR;
    if (!strcmp(name, "GetDeviceMemoryOpaqueCaptureAddressKHR")) return (void *)table->GetDeviceMemoryOpaqueCaptureAddressKHR;

    // ---- VK_KHR_deferred_host_operations extension commands
    if (!strcmp(name, "CreateDeferredOperationKHR")) return (void *)table->CreateDeferredOperationKHR;
    if (!strcmp(name, "DestroyDeferredOperationKHR")) return (void *)table->DestroyDeferredOperationKHR;
    if (!strcmp(name, "GetDeferredOperationMaxConcurrencyKHR")) return (void *)table->GetDeferredOperationMaxConcurrencyKHR;
    if (!strcmp(name, "GetDeferredOperationResultKHR")) return (void *)table->GetDeferredOperationResultKHR;
    if (!strcmp(name, "DeferredOperationJoinKHR")) return (void *)table->DeferredOperationJoinKHR;

    // ---- VK_KHR_pipeline_executable_properties extension commands
    if (!strcmp(name, "GetPipelineExecutablePropertiesKHR")) return (void *)table->GetPipelineExecutablePropertiesKHR;
    if (!strcmp(name, "GetPipelineExecutableStatisticsKHR")) return (void *)table->GetPipelineExecutableStatisticsKHR;
    if (!strcmp(name, "GetPipelineExecutableInternalRepresentationsKHR")) return (void *)table->GetPipelineExecutableInternalRepresentationsKHR;

    // ---- VK_KHR_map_memory2 extension commands
    if (!strcmp(name, "MapMemory2KHR")) return (void *)table->MapMemory2KHR;
    if (!strcmp(name, "UnmapMemory2KHR")) return (void *)table->UnmapMemory2KHR;

    // ---- VK_KHR_video_encode_queue extension commands
    if (!strcmp(name, "GetEncodedVideoSessionParametersKHR")) return (void *)table->GetEncodedVideoSessionParametersKHR;
    if (!strcmp(name, "CmdEncodeVideoKHR")) return (void *)table->CmdEncodeVideoKHR;

    // ---- VK_KHR_synchronization2 extension commands
    if (!strcmp(name, "CmdSetEvent2KHR")) return (void *)table->CmdSetEvent2KHR;
    if (!strcmp(name, "CmdResetEvent2KHR")) return (void *)table->CmdResetEvent2KHR;
    if (!strcmp(name, "CmdWaitEvents2KHR")) return (void *)table->CmdWaitEvents2KHR;
    if (!strcmp(name, "CmdPipelineBarrier2KHR")) return (void *)table->CmdPipelineBarrier2KHR;
    if (!strcmp(name, "CmdWriteTimestamp2KHR")) return (void *)table->CmdWriteTimestamp2KHR;
    if (!strcmp(name, "QueueSubmit2KHR")) return (void *)table->QueueSubmit2KHR;

    // ---- VK_KHR_copy_commands2 extension commands
    if (!strcmp(name, "CmdCopyBuffer2KHR")) return (void *)table->CmdCopyBuffer2KHR;
    if (!strcmp(name, "CmdCopyImage2KHR")) return (void *)table->CmdCopyImage2KHR;
    if (!strcmp(name, "CmdCopyBufferToImage2KHR")) return (void *)table->CmdCopyBufferToImage2KHR;
    if (!strcmp(name, "CmdCopyImageToBuffer2KHR")) return (void *)table->CmdCopyImageToBuffer2KHR;
    if (!strcmp(name, "CmdBlitImage2KHR")) return (void *)table->CmdBlitImage2KHR;
    if (!strcmp(name, "CmdResolveImage2KHR")) return (void *)table->CmdResolveImage2KHR;

    // ---- VK_KHR_ray_tracing_maintenance1 extension commands
    if (!strcmp(name, "CmdTraceRaysIndirect2KHR")) return (void *)table->CmdTraceRaysIndirect2KHR;

    // ---- VK_KHR_maintenance4 extension commands
    if (!strcmp(name, "GetDeviceBufferMemoryRequirementsKHR")) return (void *)table->GetDeviceBufferMemoryRequirementsKHR;
    if (!strcmp(name, "GetDeviceImageMemoryRequirementsKHR")) return (void *)table->GetDeviceImageMemoryRequirementsKHR;
    if (!strcmp(name, "GetDeviceImageSparseMemoryRequirementsKHR")) return (void *)table->GetDeviceImageSparseMemoryRequirementsKHR;

    // ---- VK_KHR_maintenance5 extension commands
    if (!strcmp(name, "CmdBindIndexBuffer2KHR")) return (void *)table->CmdBindIndexBuffer2KHR;
    if (!strcmp(name, "GetRenderingAreaGranularityKHR")) return (void *)table->GetRenderingAreaGranularityKHR;
    if (!strcmp(name, "GetDeviceImageSubresourceLayoutKHR")) return (void *)table->GetDeviceImageSubresourceLayoutKHR;
    if (!strcmp(name, "GetImageSubresourceLayout2KHR")) return (void *)table->GetImageSubresourceLayout2KHR;

    // ---- VK_KHR_pipeline_binary extension commands
    if (!strcmp(name, "CreatePipelineBinariesKHR")) return (void *)table->CreatePipelineBinariesKHR;
    if (!strcmp(name, "DestroyPipelineBinaryKHR")) return (void *)table->DestroyPipelineBinaryKHR;
    if (!strcmp(name, "GetPipelineKeyKHR")) return (void *)table->GetPipelineKeyKHR;
    if (!strcmp(name, "GetPipelineBinaryDataKHR")) return (void *)table->GetPipelineBinaryDataKHR;
    if (!strcmp(name, "ReleaseCapturedPipelineDataKHR")) return (void *)table->ReleaseCapturedPipelineDataKHR;

    // ---- VK_KHR_line_rasterization extension commands
    if (!strcmp(name, "CmdSetLineStippleKHR")) return (void *)table->CmdSetLineStippleKHR;

    // ---- VK_KHR_calibrated_timestamps extension commands
    if (!strcmp(name, "GetCalibratedTimestampsKHR")) return (void *)table->GetCalibratedTimestampsKHR;

    // ---- VK_KHR_maintenance6 extension commands
    if (!strcmp(name, "CmdBindDescriptorSets2KHR")) return (void *)table->CmdBindDescriptorSets2KHR;
    if (!strcmp(name, "CmdPushConstants2KHR")) return (void *)table->CmdPushConstants2KHR;
    if (!strcmp(name, "CmdPushDescriptorSet2KHR")) return (void *)table->CmdPushDescriptorSet2KHR;
    if (!strcmp(name, "CmdPushDescriptorSetWithTemplate2KHR")) return (void *)table->CmdPushDescriptorSetWithTemplate2KHR;
    if (!strcmp(name, "CmdSetDescriptorBufferOffsets2EXT")) return (void *)table->CmdSetDescriptorBufferOffsets2EXT;
    if (!strcmp(name, "CmdBindDescriptorBufferEmbeddedSamplers2EXT")) return (void *)table->CmdBindDescriptorBufferEmbeddedSamplers2EXT;

    // ---- VK_EXT_debug_marker extension commands
    if (!strcmp(name, "DebugMarkerSetObjectTagEXT")) return dev->layer_extensions.ext_debug_marker_enabled ? (void *)DebugMarkerSetObjectTagEXT : NULL;
    if (!strcmp(name, "DebugMarkerSetObjectNameEXT")) return dev->layer_extensions.ext_debug_marker_enabled ? (void *)DebugMarkerSetObjectNameEXT : NULL;
    if (!strcmp(name, "CmdDebugMarkerBeginEXT")) return (void *)table->CmdDebugMarkerBeginEXT;
    if (!strcmp(name, "CmdDebugMarkerEndEXT")) return (void *)table->CmdDebugMarkerEndEXT;
    if (!strcmp(name, "CmdDebugMarkerInsertEXT")) return (void *)table->CmdDebugMarkerInsertEXT;

    // ---- VK_EXT_transform_feedback extension commands
    if (!strcmp(name, "CmdBindTransformFeedbackBuffersEXT")) return (void *)table->CmdBindTransformFeedbackBuffersEXT;
    if (!strcmp(name, "CmdBeginTransformFeedbackEXT")) return (void *)table->CmdBeginTransformFeedbackEXT;
    if (!strcmp(name, "CmdEndTransformFeedbackEXT")) return (void *)table->CmdEndTransformFeedbackEXT;
    if (!strcmp(name, "CmdBeginQueryIndexedEXT")) return (void *)table->CmdBeginQueryIndexedEXT;
    if (!strcmp(name, "CmdEndQueryIndexedEXT")) return (void *)table->CmdEndQueryIndexedEXT;
    if (!strcmp(name, "CmdDrawIndirectByteCountEXT")) return (void *)table->CmdDrawIndirectByteCountEXT;

    // ---- VK_NVX_binary_import extension commands
    if (!strcmp(name, "CreateCuModuleNVX")) return (void *)table->CreateCuModuleNVX;
    if (!strcmp(name, "CreateCuFunctionNVX")) return (void *)table->CreateCuFunctionNVX;
    if (!strcmp(name, "DestroyCuModuleNVX")) return (void *)table->DestroyCuModuleNVX;
    if (!strcmp(name, "DestroyCuFunctionNVX")) return (void *)table->DestroyCuFunctionNVX;
    if (!strcmp(name, "CmdCuLaunchKernelNVX")) return (void *)table->CmdCuLaunchKernelNVX;

    // ---- VK_NVX_image_view_handle extension commands
    if (!strcmp(name, "GetImageViewHandleNVX")) return (void *)table->GetImageViewHandleNVX;
    if (!strcmp(name, "GetImageViewHandle64NVX")) return (void *)table->GetImageViewHandle64NVX;
    if (!strcmp(name, "GetImageViewAddressNVX")) return (void *)table->GetImageViewAddressNVX;

    // ---- VK_AMD_draw_indirect_count extension commands
    if (!strcmp(name, "CmdDrawIndirectCountAMD")) return (void *)table->CmdDrawIndirectCountAMD;
    if (!strcmp(name, "CmdDrawIndexedIndirectCountAMD")) return (void *)table->CmdDrawIndexedIndirectCountAMD;

    // ---- VK_AMD_shader_info extension commands
    if (!strcmp(name, "GetShaderInfoAMD")) return (void *)table->GetShaderInfoAMD;

    // ---- VK_NV_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetMemoryWin32HandleNV")) return (void *)table->GetMemoryWin32HandleNV;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_conditional_rendering extension commands
    if (!strcmp(name, "CmdBeginConditionalRenderingEXT")) return (void *)table->CmdBeginConditionalRenderingEXT;
    if (!strcmp(name, "CmdEndConditionalRenderingEXT")) return (void *)table->CmdEndConditionalRenderingEXT;

    // ---- VK_NV_clip_space_w_scaling extension commands
    if (!strcmp(name, "CmdSetViewportWScalingNV")) return (void *)table->CmdSetViewportWScalingNV;

    // ---- VK_EXT_display_control extension commands
    if (!strcmp(name, "DisplayPowerControlEXT")) return (void *)table->DisplayPowerControlEXT;
    if (!strcmp(name, "RegisterDeviceEventEXT")) return (void *)table->RegisterDeviceEventEXT;
    if (!strcmp(name, "RegisterDisplayEventEXT")) return (void *)table->RegisterDisplayEventEXT;
    if (!strcmp(name, "GetSwapchainCounterEXT")) return (void *)table->GetSwapchainCounterEXT;

    // ---- VK_GOOGLE_display_timing extension commands
    if (!strcmp(name, "GetRefreshCycleDurationGOOGLE")) return (void *)table->GetRefreshCycleDurationGOOGLE;
    if (!strcmp(name, "GetPastPresentationTimingGOOGLE")) return (void *)table->GetPastPresentationTimingGOOGLE;

    // ---- VK_EXT_discard_rectangles extension commands
    if (!strcmp(name, "CmdSetDiscardRectangleEXT")) return (void *)table->CmdSetDiscardRectangleEXT;
    if (!strcmp(name, "CmdSetDiscardRectangleEnableEXT")) return (void *)table->CmdSetDiscardRectangleEnableEXT;
    if (!strcmp(name, "CmdSetDiscardRectangleModeEXT")) return (void *)table->CmdSetDiscardRectangleModeEXT;

    // ---- VK_EXT_hdr_metadata extension commands
    if (!strcmp(name, "SetHdrMetadataEXT")) return (void *)table->SetHdrMetadataEXT;

    // ---- VK_EXT_debug_utils extension commands
    if (!strcmp(name, "SetDebugUtilsObjectNameEXT")) return dev->layer_extensions.ext_debug_utils_enabled ? (void *)SetDebugUtilsObjectNameEXT : NULL;
    if (!strcmp(name, "SetDebugUtilsObjectTagEXT")) return dev->layer_extensions.ext_debug_utils_enabled ? (void *)SetDebugUtilsObjectTagEXT : NULL;
    if (!strcmp(name, "QueueBeginDebugUtilsLabelEXT")) return (void *)table->QueueBeginDebugUtilsLabelEXT;
    if (!strcmp(name, "QueueEndDebugUtilsLabelEXT")) return (void *)table->QueueEndDebugUtilsLabelEXT;
    if (!strcmp(name, "QueueInsertDebugUtilsLabelEXT")) return (void *)table->QueueInsertDebugUtilsLabelEXT;
    if (!strcmp(name, "CmdBeginDebugUtilsLabelEXT")) return (void *)table->CmdBeginDebugUtilsLabelEXT;
    if (!strcmp(name, "CmdEndDebugUtilsLabelEXT")) return (void *)table->CmdEndDebugUtilsLabelEXT;
    if (!strcmp(name, "CmdInsertDebugUtilsLabelEXT")) return (void *)table->CmdInsertDebugUtilsLabelEXT;

    // ---- VK_ANDROID_external_memory_android_hardware_buffer extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp(name, "GetAndroidHardwareBufferPropertiesANDROID")) return (void *)table->GetAndroidHardwareBufferPropertiesANDROID;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp(name, "GetMemoryAndroidHardwareBufferANDROID")) return (void *)table->GetMemoryAndroidHardwareBufferANDROID;
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_AMDX_shader_enqueue extension commands
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "CreateExecutionGraphPipelinesAMDX")) return (void *)table->CreateExecutionGraphPipelinesAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "GetExecutionGraphPipelineScratchSizeAMDX")) return (void *)table->GetExecutionGraphPipelineScratchSizeAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "GetExecutionGraphPipelineNodeIndexAMDX")) return (void *)table->GetExecutionGraphPipelineNodeIndexAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "CmdInitializeGraphScratchMemoryAMDX")) return (void *)table->CmdInitializeGraphScratchMemoryAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "CmdDispatchGraphAMDX")) return (void *)table->CmdDispatchGraphAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "CmdDispatchGraphIndirectAMDX")) return (void *)table->CmdDispatchGraphIndirectAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp(name, "CmdDispatchGraphIndirectCountAMDX")) return (void *)table->CmdDispatchGraphIndirectCountAMDX;
#endif // VK_ENABLE_BETA_EXTENSIONS

    // ---- VK_EXT_sample_locations extension commands
    if (!strcmp(name, "CmdSetSampleLocationsEXT")) return (void *)table->CmdSetSampleLocationsEXT;

    // ---- VK_EXT_image_drm_format_modifier extension commands
    if (!strcmp(name, "GetImageDrmFormatModifierPropertiesEXT")) return (void *)table->GetImageDrmFormatModifierPropertiesEXT;

    // ---- VK_EXT_validation_cache extension commands
    if (!strcmp(name, "CreateValidationCacheEXT")) return (void *)table->CreateValidationCacheEXT;
    if (!strcmp(name, "DestroyValidationCacheEXT")) return (void *)table->DestroyValidationCacheEXT;
    if (!strcmp(name, "MergeValidationCachesEXT")) return (void *)table->MergeValidationCachesEXT;
    if (!strcmp(name, "GetValidationCacheDataEXT")) return (void *)table->GetValidationCacheDataEXT;

    // ---- VK_NV_shading_rate_image extension commands
    if (!strcmp(name, "CmdBindShadingRateImageNV")) return (void *)table->CmdBindShadingRateImageNV;
    if (!strcmp(name, "CmdSetViewportShadingRatePaletteNV")) return (void *)table->CmdSetViewportShadingRatePaletteNV;
    if (!strcmp(name, "CmdSetCoarseSampleOrderNV")) return (void *)table->CmdSetCoarseSampleOrderNV;

    // ---- VK_NV_ray_tracing extension commands
    if (!strcmp(name, "CreateAccelerationStructureNV")) return (void *)table->CreateAccelerationStructureNV;
    if (!strcmp(name, "DestroyAccelerationStructureNV")) return (void *)table->DestroyAccelerationStructureNV;
    if (!strcmp(name, "GetAccelerationStructureMemoryRequirementsNV")) return (void *)table->GetAccelerationStructureMemoryRequirementsNV;
    if (!strcmp(name, "BindAccelerationStructureMemoryNV")) return (void *)table->BindAccelerationStructureMemoryNV;
    if (!strcmp(name, "CmdBuildAccelerationStructureNV")) return (void *)table->CmdBuildAccelerationStructureNV;
    if (!strcmp(name, "CmdCopyAccelerationStructureNV")) return (void *)table->CmdCopyAccelerationStructureNV;
    if (!strcmp(name, "CmdTraceRaysNV")) return (void *)table->CmdTraceRaysNV;
    if (!strcmp(name, "CreateRayTracingPipelinesNV")) return (void *)table->CreateRayTracingPipelinesNV;
    if (!strcmp(name, "GetRayTracingShaderGroupHandlesKHR")) return (void *)table->GetRayTracingShaderGroupHandlesKHR;
    if (!strcmp(name, "GetRayTracingShaderGroupHandlesNV")) return (void *)table->GetRayTracingShaderGroupHandlesNV;
    if (!strcmp(name, "GetAccelerationStructureHandleNV")) return (void *)table->GetAccelerationStructureHandleNV;
    if (!strcmp(name, "CmdWriteAccelerationStructuresPropertiesNV")) return (void *)table->CmdWriteAccelerationStructuresPropertiesNV;
    if (!strcmp(name, "CompileDeferredNV")) return (void *)table->CompileDeferredNV;

    // ---- VK_EXT_external_memory_host extension commands
    if (!strcmp(name, "GetMemoryHostPointerPropertiesEXT")) return (void *)table->GetMemoryHostPointerPropertiesEXT;

    // ---- VK_AMD_buffer_marker extension commands
    if (!strcmp(name, "CmdWriteBufferMarkerAMD")) return (void *)table->CmdWriteBufferMarkerAMD;
    if (!strcmp(name, "CmdWriteBufferMarker2AMD")) return (void *)table->CmdWriteBufferMarker2AMD;

    // ---- VK_EXT_calibrated_timestamps extension commands
    if (!strcmp(name, "GetCalibratedTimestampsEXT")) return (void *)table->GetCalibratedTimestampsEXT;

    // ---- VK_NV_mesh_shader extension commands
    if (!strcmp(name, "CmdDrawMeshTasksNV")) return (void *)table->CmdDrawMeshTasksNV;
    if (!strcmp(name, "CmdDrawMeshTasksIndirectNV")) return (void *)table->CmdDrawMeshTasksIndirectNV;
    if (!strcmp(name, "CmdDrawMeshTasksIndirectCountNV")) return (void *)table->CmdDrawMeshTasksIndirectCountNV;

    // ---- VK_NV_scissor_exclusive extension commands
    if (!strcmp(name, "CmdSetExclusiveScissorEnableNV")) return (void *)table->CmdSetExclusiveScissorEnableNV;
    if (!strcmp(name, "CmdSetExclusiveScissorNV")) return (void *)table->CmdSetExclusiveScissorNV;

    // ---- VK_NV_device_diagnostic_checkpoints extension commands
    if (!strcmp(name, "CmdSetCheckpointNV")) return (void *)table->CmdSetCheckpointNV;
    if (!strcmp(name, "GetQueueCheckpointDataNV")) return (void *)table->GetQueueCheckpointDataNV;
    if (!strcmp(name, "GetQueueCheckpointData2NV")) return (void *)table->GetQueueCheckpointData2NV;

    // ---- VK_INTEL_performance_query extension commands
    if (!strcmp(name, "InitializePerformanceApiINTEL")) return (void *)table->InitializePerformanceApiINTEL;
    if (!strcmp(name, "UninitializePerformanceApiINTEL")) return (void *)table->UninitializePerformanceApiINTEL;
    if (!strcmp(name, "CmdSetPerformanceMarkerINTEL")) return (void *)table->CmdSetPerformanceMarkerINTEL;
    if (!strcmp(name, "CmdSetPerformanceStreamMarkerINTEL")) return (void *)table->CmdSetPerformanceStreamMarkerINTEL;
    if (!strcmp(name, "CmdSetPerformanceOverrideINTEL")) return (void *)table->CmdSetPerformanceOverrideINTEL;
    if (!strcmp(name, "AcquirePerformanceConfigurationINTEL")) return (void *)table->AcquirePerformanceConfigurationINTEL;
    if (!strcmp(name, "ReleasePerformanceConfigurationINTEL")) return (void *)table->ReleasePerformanceConfigurationINTEL;
    if (!strcmp(name, "QueueSetPerformanceConfigurationINTEL")) return (void *)table->QueueSetPerformanceConfigurationINTEL;
    if (!strcmp(name, "GetPerformanceParameterINTEL")) return (void *)table->GetPerformanceParameterINTEL;

    // ---- VK_AMD_display_native_hdr extension commands
    if (!strcmp(name, "SetLocalDimmingAMD")) return (void *)table->SetLocalDimmingAMD;

    // ---- VK_EXT_buffer_device_address extension commands
    if (!strcmp(name, "GetBufferDeviceAddressEXT")) return (void *)table->GetBufferDeviceAddressEXT;

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "AcquireFullScreenExclusiveModeEXT")) return (void *)table->AcquireFullScreenExclusiveModeEXT;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "ReleaseFullScreenExclusiveModeEXT")) return (void *)table->ReleaseFullScreenExclusiveModeEXT;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetDeviceGroupSurfacePresentModes2EXT")) return (void *)table->GetDeviceGroupSurfacePresentModes2EXT;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_line_rasterization extension commands
    if (!strcmp(name, "CmdSetLineStippleEXT")) return (void *)table->CmdSetLineStippleEXT;

    // ---- VK_EXT_host_query_reset extension commands
    if (!strcmp(name, "ResetQueryPoolEXT")) return (void *)table->ResetQueryPoolEXT;

    // ---- VK_EXT_extended_dynamic_state extension commands
    if (!strcmp(name, "CmdSetCullModeEXT")) return (void *)table->CmdSetCullModeEXT;
    if (!strcmp(name, "CmdSetFrontFaceEXT")) return (void *)table->CmdSetFrontFaceEXT;
    if (!strcmp(name, "CmdSetPrimitiveTopologyEXT")) return (void *)table->CmdSetPrimitiveTopologyEXT;
    if (!strcmp(name, "CmdSetViewportWithCountEXT")) return (void *)table->CmdSetViewportWithCountEXT;
    if (!strcmp(name, "CmdSetScissorWithCountEXT")) return (void *)table->CmdSetScissorWithCountEXT;
    if (!strcmp(name, "CmdBindVertexBuffers2EXT")) return (void *)table->CmdBindVertexBuffers2EXT;
    if (!strcmp(name, "CmdSetDepthTestEnableEXT")) return (void *)table->CmdSetDepthTestEnableEXT;
    if (!strcmp(name, "CmdSetDepthWriteEnableEXT")) return (void *)table->CmdSetDepthWriteEnableEXT;
    if (!strcmp(name, "CmdSetDepthCompareOpEXT")) return (void *)table->CmdSetDepthCompareOpEXT;
    if (!strcmp(name, "CmdSetDepthBoundsTestEnableEXT")) return (void *)table->CmdSetDepthBoundsTestEnableEXT;
    if (!strcmp(name, "CmdSetStencilTestEnableEXT")) return (void *)table->CmdSetStencilTestEnableEXT;
    if (!strcmp(name, "CmdSetStencilOpEXT")) return (void *)table->CmdSetStencilOpEXT;

    // ---- VK_EXT_host_image_copy extension commands
    if (!strcmp(name, "CopyMemoryToImageEXT")) return (void *)table->CopyMemoryToImageEXT;
    if (!strcmp(name, "CopyImageToMemoryEXT")) return (void *)table->CopyImageToMemoryEXT;
    if (!strcmp(name, "CopyImageToImageEXT")) return (void *)table->CopyImageToImageEXT;
    if (!strcmp(name, "TransitionImageLayoutEXT")) return (void *)table->TransitionImageLayoutEXT;
    if (!strcmp(name, "GetImageSubresourceLayout2EXT")) return (void *)table->GetImageSubresourceLayout2EXT;

    // ---- VK_EXT_swapchain_maintenance1 extension commands
    if (!strcmp(name, "ReleaseSwapchainImagesEXT")) return (void *)table->ReleaseSwapchainImagesEXT;

    // ---- VK_NV_device_generated_commands extension commands
    if (!strcmp(name, "GetGeneratedCommandsMemoryRequirementsNV")) return (void *)table->GetGeneratedCommandsMemoryRequirementsNV;
    if (!strcmp(name, "CmdPreprocessGeneratedCommandsNV")) return (void *)table->CmdPreprocessGeneratedCommandsNV;
    if (!strcmp(name, "CmdExecuteGeneratedCommandsNV")) return (void *)table->CmdExecuteGeneratedCommandsNV;
    if (!strcmp(name, "CmdBindPipelineShaderGroupNV")) return (void *)table->CmdBindPipelineShaderGroupNV;
    if (!strcmp(name, "CreateIndirectCommandsLayoutNV")) return (void *)table->CreateIndirectCommandsLayoutNV;
    if (!strcmp(name, "DestroyIndirectCommandsLayoutNV")) return (void *)table->DestroyIndirectCommandsLayoutNV;

    // ---- VK_EXT_depth_bias_control extension commands
    if (!strcmp(name, "CmdSetDepthBias2EXT")) return (void *)table->CmdSetDepthBias2EXT;

    // ---- VK_EXT_private_data extension commands
    if (!strcmp(name, "CreatePrivateDataSlotEXT")) return (void *)table->CreatePrivateDataSlotEXT;
    if (!strcmp(name, "DestroyPrivateDataSlotEXT")) return (void *)table->DestroyPrivateDataSlotEXT;
    if (!strcmp(name, "SetPrivateDataEXT")) return (void *)table->SetPrivateDataEXT;
    if (!strcmp(name, "GetPrivateDataEXT")) return (void *)table->GetPrivateDataEXT;

    // ---- VK_NV_cuda_kernel_launch extension commands
    if (!strcmp(name, "CreateCudaModuleNV")) return (void *)table->CreateCudaModuleNV;
    if (!strcmp(name, "GetCudaModuleCacheNV")) return (void *)table->GetCudaModuleCacheNV;
    if (!strcmp(name, "CreateCudaFunctionNV")) return (void *)table->CreateCudaFunctionNV;
    if (!strcmp(name, "DestroyCudaModuleNV")) return (void *)table->DestroyCudaModuleNV;
    if (!strcmp(name, "DestroyCudaFunctionNV")) return (void *)table->DestroyCudaFunctionNV;
    if (!strcmp(name, "CmdCudaLaunchKernelNV")) return (void *)table->CmdCudaLaunchKernelNV;

    // ---- VK_EXT_metal_objects extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp(name, "ExportMetalObjectsEXT")) return (void *)table->ExportMetalObjectsEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_descriptor_buffer extension commands
    if (!strcmp(name, "GetDescriptorSetLayoutSizeEXT")) return (void *)table->GetDescriptorSetLayoutSizeEXT;
    if (!strcmp(name, "GetDescriptorSetLayoutBindingOffsetEXT")) return (void *)table->GetDescriptorSetLayoutBindingOffsetEXT;
    if (!strcmp(name, "GetDescriptorEXT")) return (void *)table->GetDescriptorEXT;
    if (!strcmp(name, "CmdBindDescriptorBuffersEXT")) return (void *)table->CmdBindDescriptorBuffersEXT;
    if (!strcmp(name, "CmdSetDescriptorBufferOffsetsEXT")) return (void *)table->CmdSetDescriptorBufferOffsetsEXT;
    if (!strcmp(name, "CmdBindDescriptorBufferEmbeddedSamplersEXT")) return (void *)table->CmdBindDescriptorBufferEmbeddedSamplersEXT;
    if (!strcmp(name, "GetBufferOpaqueCaptureDescriptorDataEXT")) return (void *)table->GetBufferOpaqueCaptureDescriptorDataEXT;
    if (!strcmp(name, "GetImageOpaqueCaptureDescriptorDataEXT")) return (void *)table->GetImageOpaqueCaptureDescriptorDataEXT;
    if (!strcmp(name, "GetImageViewOpaqueCaptureDescriptorDataEXT")) return (void *)table->GetImageViewOpaqueCaptureDescriptorDataEXT;
    if (!strcmp(name, "GetSamplerOpaqueCaptureDescriptorDataEXT")) return (void *)table->GetSamplerOpaqueCaptureDescriptorDataEXT;
    if (!strcmp(name, "GetAccelerationStructureOpaqueCaptureDescriptorDataEXT")) return (void *)table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT;

    // ---- VK_NV_fragment_shading_rate_enums extension commands
    if (!strcmp(name, "CmdSetFragmentShadingRateEnumNV")) return (void *)table->CmdSetFragmentShadingRateEnumNV;

    // ---- VK_EXT_device_fault extension commands
    if (!strcmp(name, "GetDeviceFaultInfoEXT")) return (void *)table->GetDeviceFaultInfoEXT;

    // ---- VK_EXT_vertex_input_dynamic_state extension commands
    if (!strcmp(name, "CmdSetVertexInputEXT")) return (void *)table->CmdSetVertexInputEXT;

    // ---- VK_FUCHSIA_external_memory extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "GetMemoryZirconHandleFUCHSIA")) return (void *)table->GetMemoryZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "GetMemoryZirconHandlePropertiesFUCHSIA")) return (void *)table->GetMemoryZirconHandlePropertiesFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_external_semaphore extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "ImportSemaphoreZirconHandleFUCHSIA")) return (void *)table->ImportSemaphoreZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "GetSemaphoreZirconHandleFUCHSIA")) return (void *)table->GetSemaphoreZirconHandleFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_buffer_collection extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "CreateBufferCollectionFUCHSIA")) return (void *)table->CreateBufferCollectionFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "SetBufferCollectionImageConstraintsFUCHSIA")) return (void *)table->SetBufferCollectionImageConstraintsFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "SetBufferCollectionBufferConstraintsFUCHSIA")) return (void *)table->SetBufferCollectionBufferConstraintsFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "DestroyBufferCollectionFUCHSIA")) return (void *)table->DestroyBufferCollectionFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "GetBufferCollectionPropertiesFUCHSIA")) return (void *)table->GetBufferCollectionPropertiesFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_HUAWEI_subpass_shading extension commands
    if (!strcmp(name, "GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI")) return (void *)table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;
    if (!strcmp(name, "CmdSubpassShadingHUAWEI")) return (void *)table->CmdSubpassShadingHUAWEI;

    // ---- VK_HUAWEI_invocation_mask extension commands
    if (!strcmp(name, "CmdBindInvocationMaskHUAWEI")) return (void *)table->CmdBindInvocationMaskHUAWEI;

    // ---- VK_NV_external_memory_rdma extension commands
    if (!strcmp(name, "GetMemoryRemoteAddressNV")) return (void *)table->GetMemoryRemoteAddressNV;

    // ---- VK_EXT_pipeline_properties extension commands
    if (!strcmp(name, "GetPipelinePropertiesEXT")) return (void *)table->GetPipelinePropertiesEXT;

    // ---- VK_EXT_extended_dynamic_state2 extension commands
    if (!strcmp(name, "CmdSetPatchControlPointsEXT")) return (void *)table->CmdSetPatchControlPointsEXT;
    if (!strcmp(name, "CmdSetRasterizerDiscardEnableEXT")) return (void *)table->CmdSetRasterizerDiscardEnableEXT;
    if (!strcmp(name, "CmdSetDepthBiasEnableEXT")) return (void *)table->CmdSetDepthBiasEnableEXT;
    if (!strcmp(name, "CmdSetLogicOpEXT")) return (void *)table->CmdSetLogicOpEXT;
    if (!strcmp(name, "CmdSetPrimitiveRestartEnableEXT")) return (void *)table->CmdSetPrimitiveRestartEnableEXT;

    // ---- VK_EXT_color_write_enable extension commands
    if (!strcmp(name, "CmdSetColorWriteEnableEXT")) return (void *)table->CmdSetColorWriteEnableEXT;

    // ---- VK_EXT_multi_draw extension commands
    if (!strcmp(name, "CmdDrawMultiEXT")) return (void *)table->CmdDrawMultiEXT;
    if (!strcmp(name, "CmdDrawMultiIndexedEXT")) return (void *)table->CmdDrawMultiIndexedEXT;

    // ---- VK_EXT_opacity_micromap extension commands
    if (!strcmp(name, "CreateMicromapEXT")) return (void *)table->CreateMicromapEXT;
    if (!strcmp(name, "DestroyMicromapEXT")) return (void *)table->DestroyMicromapEXT;
    if (!strcmp(name, "CmdBuildMicromapsEXT")) return (void *)table->CmdBuildMicromapsEXT;
    if (!strcmp(name, "BuildMicromapsEXT")) return (void *)table->BuildMicromapsEXT;
    if (!strcmp(name, "CopyMicromapEXT")) return (void *)table->CopyMicromapEXT;
    if (!strcmp(name, "CopyMicromapToMemoryEXT")) return (void *)table->CopyMicromapToMemoryEXT;
    if (!strcmp(name, "CopyMemoryToMicromapEXT")) return (void *)table->CopyMemoryToMicromapEXT;
    if (!strcmp(name, "WriteMicromapsPropertiesEXT")) return (void *)table->WriteMicromapsPropertiesEXT;
    if (!strcmp(name, "CmdCopyMicromapEXT")) return (void *)table->CmdCopyMicromapEXT;
    if (!strcmp(name, "CmdCopyMicromapToMemoryEXT")) return (void *)table->CmdCopyMicromapToMemoryEXT;
    if (!strcmp(name, "CmdCopyMemoryToMicromapEXT")) return (void *)table->CmdCopyMemoryToMicromapEXT;
    if (!strcmp(name, "CmdWriteMicromapsPropertiesEXT")) return (void *)table->CmdWriteMicromapsPropertiesEXT;
    if (!strcmp(name, "GetDeviceMicromapCompatibilityEXT")) return (void *)table->GetDeviceMicromapCompatibilityEXT;
    if (!strcmp(name, "GetMicromapBuildSizesEXT")) return (void *)table->GetMicromapBuildSizesEXT;

    // ---- VK_HUAWEI_cluster_culling_shader extension commands
    if (!strcmp(name, "CmdDrawClusterHUAWEI")) return (void *)table->CmdDrawClusterHUAWEI;
    if (!strcmp(name, "CmdDrawClusterIndirectHUAWEI")) return (void *)table->CmdDrawClusterIndirectHUAWEI;

    // ---- VK_EXT_pageable_device_local_memory extension commands
    if (!strcmp(name, "SetDeviceMemoryPriorityEXT")) return (void *)table->SetDeviceMemoryPriorityEXT;

    // ---- VK_VALVE_descriptor_set_host_mapping extension commands
    if (!strcmp(name, "GetDescriptorSetLayoutHostMappingInfoVALVE")) return (void *)table->GetDescriptorSetLayoutHostMappingInfoVALVE;
    if (!strcmp(name, "GetDescriptorSetHostMappingVALVE")) return (void *)table->GetDescriptorSetHostMappingVALVE;

    // ---- VK_NV_copy_memory_indirect extension commands
    if (!strcmp(name, "CmdCopyMemoryIndirectNV")) return (void *)table->CmdCopyMemoryIndirectNV;
    if (!strcmp(name, "CmdCopyMemoryToImageIndirectNV")) return (void *)table->CmdCopyMemoryToImageIndirectNV;

    // ---- VK_NV_memory_decompression extension commands
    if (!strcmp(name, "CmdDecompressMemoryNV")) return (void *)table->CmdDecompressMemoryNV;
    if (!strcmp(name, "CmdDecompressMemoryIndirectCountNV")) return (void *)table->CmdDecompressMemoryIndirectCountNV;

    // ---- VK_NV_device_generated_commands_compute extension commands
    if (!strcmp(name, "GetPipelineIndirectMemoryRequirementsNV")) return (void *)table->GetPipelineIndirectMemoryRequirementsNV;
    if (!strcmp(name, "CmdUpdatePipelineIndirectBufferNV")) return (void *)table->CmdUpdatePipelineIndirectBufferNV;
    if (!strcmp(name, "GetPipelineIndirectDeviceAddressNV")) return (void *)table->GetPipelineIndirectDeviceAddressNV;

    // ---- VK_EXT_extended_dynamic_state3 extension commands
    if (!strcmp(name, "CmdSetDepthClampEnableEXT")) return (void *)table->CmdSetDepthClampEnableEXT;
    if (!strcmp(name, "CmdSetPolygonModeEXT")) return (void *)table->CmdSetPolygonModeEXT;
    if (!strcmp(name, "CmdSetRasterizationSamplesEXT")) return (void *)table->CmdSetRasterizationSamplesEXT;
    if (!strcmp(name, "CmdSetSampleMaskEXT")) return (void *)table->CmdSetSampleMaskEXT;
    if (!strcmp(name, "CmdSetAlphaToCoverageEnableEXT")) return (void *)table->CmdSetAlphaToCoverageEnableEXT;
    if (!strcmp(name, "CmdSetAlphaToOneEnableEXT")) return (void *)table->CmdSetAlphaToOneEnableEXT;
    if (!strcmp(name, "CmdSetLogicOpEnableEXT")) return (void *)table->CmdSetLogicOpEnableEXT;
    if (!strcmp(name, "CmdSetColorBlendEnableEXT")) return (void *)table->CmdSetColorBlendEnableEXT;
    if (!strcmp(name, "CmdSetColorBlendEquationEXT")) return (void *)table->CmdSetColorBlendEquationEXT;
    if (!strcmp(name, "CmdSetColorWriteMaskEXT")) return (void *)table->CmdSetColorWriteMaskEXT;
    if (!strcmp(name, "CmdSetTessellationDomainOriginEXT")) return (void *)table->CmdSetTessellationDomainOriginEXT;
    if (!strcmp(name, "CmdSetRasterizationStreamEXT")) return (void *)table->CmdSetRasterizationStreamEXT;
    if (!strcmp(name, "CmdSetConservativeRasterizationModeEXT")) return (void *)table->CmdSetConservativeRasterizationModeEXT;
    if (!strcmp(name, "CmdSetExtraPrimitiveOverestimationSizeEXT")) return (void *)table->CmdSetExtraPrimitiveOverestimationSizeEXT;
    if (!strcmp(name, "CmdSetDepthClipEnableEXT")) return (void *)table->CmdSetDepthClipEnableEXT;
    if (!strcmp(name, "CmdSetSampleLocationsEnableEXT")) return (void *)table->CmdSetSampleLocationsEnableEXT;
    if (!strcmp(name, "CmdSetColorBlendAdvancedEXT")) return (void *)table->CmdSetColorBlendAdvancedEXT;
    if (!strcmp(name, "CmdSetProvokingVertexModeEXT")) return (void *)table->CmdSetProvokingVertexModeEXT;
    if (!strcmp(name, "CmdSetLineRasterizationModeEXT")) return (void *)table->CmdSetLineRasterizationModeEXT;
    if (!strcmp(name, "CmdSetLineStippleEnableEXT")) return (void *)table->CmdSetLineStippleEnableEXT;
    if (!strcmp(name, "CmdSetDepthClipNegativeOneToOneEXT")) return (void *)table->CmdSetDepthClipNegativeOneToOneEXT;
    if (!strcmp(name, "CmdSetViewportWScalingEnableNV")) return (void *)table->CmdSetViewportWScalingEnableNV;
    if (!strcmp(name, "CmdSetViewportSwizzleNV")) return (void *)table->CmdSetViewportSwizzleNV;
    if (!strcmp(name, "CmdSetCoverageToColorEnableNV")) return (void *)table->CmdSetCoverageToColorEnableNV;
    if (!strcmp(name, "CmdSetCoverageToColorLocationNV")) return (void *)table->CmdSetCoverageToColorLocationNV;
    if (!strcmp(name, "CmdSetCoverageModulationModeNV")) return (void *)table->CmdSetCoverageModulationModeNV;
    if (!strcmp(name, "CmdSetCoverageModulationTableEnableNV")) return (void *)table->CmdSetCoverageModulationTableEnableNV;
    if (!strcmp(name, "CmdSetCoverageModulationTableNV")) return (void *)table->CmdSetCoverageModulationTableNV;
    if (!strcmp(name, "CmdSetShadingRateImageEnableNV")) return (void *)table->CmdSetShadingRateImageEnableNV;
    if (!strcmp(name, "CmdSetRepresentativeFragmentTestEnableNV")) return (void *)table->CmdSetRepresentativeFragmentTestEnableNV;
    if (!strcmp(name, "CmdSetCoverageReductionModeNV")) return (void *)table->CmdSetCoverageReductionModeNV;

    // ---- VK_EXT_shader_module_identifier extension commands
    if (!strcmp(name, "GetShaderModuleIdentifierEXT")) return (void *)table->GetShaderModuleIdentifierEXT;
    if (!strcmp(name, "GetShaderModuleCreateInfoIdentifierEXT")) return (void *)table->GetShaderModuleCreateInfoIdentifierEXT;

    // ---- VK_NV_optical_flow extension commands
    if (!strcmp(name, "CreateOpticalFlowSessionNV")) return (void *)table->CreateOpticalFlowSessionNV;
    if (!strcmp(name, "DestroyOpticalFlowSessionNV")) return (void *)table->DestroyOpticalFlowSessionNV;
    if (!strcmp(name, "BindOpticalFlowSessionImageNV")) return (void *)table->BindOpticalFlowSessionImageNV;
    if (!strcmp(name, "CmdOpticalFlowExecuteNV")) return (void *)table->CmdOpticalFlowExecuteNV;

    // ---- VK_AMD_anti_lag extension commands
    if (!strcmp(name, "AntiLagUpdateAMD")) return (void *)table->AntiLagUpdateAMD;

    // ---- VK_EXT_shader_object extension commands
    if (!strcmp(name, "CreateShadersEXT")) return (void *)table->CreateShadersEXT;
    if (!strcmp(name, "DestroyShaderEXT")) return (void *)table->DestroyShaderEXT;
    if (!strcmp(name, "GetShaderBinaryDataEXT")) return (void *)table->GetShaderBinaryDataEXT;
    if (!strcmp(name, "CmdBindShadersEXT")) return (void *)table->CmdBindShadersEXT;
    if (!strcmp(name, "CmdSetDepthClampRangeEXT")) return (void *)table->CmdSetDepthClampRangeEXT;

    // ---- VK_QCOM_tile_properties extension commands
    if (!strcmp(name, "GetFramebufferTilePropertiesQCOM")) return (void *)table->GetFramebufferTilePropertiesQCOM;
    if (!strcmp(name, "GetDynamicRenderingTilePropertiesQCOM")) return (void *)table->GetDynamicRenderingTilePropertiesQCOM;

    // ---- VK_NV_cooperative_vector extension commands
    if (!strcmp(name, "ConvertCooperativeVectorMatrixNV")) return (void *)table->ConvertCooperativeVectorMatrixNV;
    if (!strcmp(name, "CmdConvertCooperativeVectorMatrixNV")) return (void *)table->CmdConvertCooperativeVectorMatrixNV;

    // ---- VK_NV_low_latency2 extension commands
    if (!strcmp(name, "SetLatencySleepModeNV")) return (void *)table->SetLatencySleepModeNV;
    if (!strcmp(name, "LatencySleepNV")) return (void *)table->LatencySleepNV;
    if (!strcmp(name, "SetLatencyMarkerNV")) return (void *)table->SetLatencyMarkerNV;
    if (!strcmp(name, "GetLatencyTimingsNV")) return (void *)table->GetLatencyTimingsNV;
    if (!strcmp(name, "QueueNotifyOutOfBandNV")) return (void *)table->QueueNotifyOutOfBandNV;

    // ---- VK_EXT_attachment_feedback_loop_dynamic_state extension commands
    if (!strcmp(name, "CmdSetAttachmentFeedbackLoopEnableEXT")) return (void *)table->CmdSetAttachmentFeedbackLoopEnableEXT;

    // ---- VK_QNX_external_memory_screen_buffer extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (!strcmp(name, "GetScreenBufferPropertiesQNX")) return (void *)table->GetScreenBufferPropertiesQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_cluster_acceleration_structure extension commands
    if (!strcmp(name, "GetClusterAccelerationStructureBuildSizesNV")) return (void *)table->GetClusterAccelerationStructureBuildSizesNV;
    if (!strcmp(name, "CmdBuildClusterAccelerationStructureIndirectNV")) return (void *)table->CmdBuildClusterAccelerationStructureIndirectNV;

    // ---- VK_NV_partitioned_acceleration_structure extension commands
    if (!strcmp(name, "GetPartitionedAccelerationStructuresBuildSizesNV")) return (void *)table->GetPartitionedAccelerationStructuresBuildSizesNV;
    if (!strcmp(name, "CmdBuildPartitionedAccelerationStructuresNV")) return (void *)table->CmdBuildPartitionedAccelerationStructuresNV;

    // ---- VK_EXT_device_generated_commands extension commands
    if (!strcmp(name, "GetGeneratedCommandsMemoryRequirementsEXT")) return (void *)table->GetGeneratedCommandsMemoryRequirementsEXT;
    if (!strcmp(name, "CmdPreprocessGeneratedCommandsEXT")) return (void *)table->CmdPreprocessGeneratedCommandsEXT;
    if (!strcmp(name, "CmdExecuteGeneratedCommandsEXT")) return (void *)table->CmdExecuteGeneratedCommandsEXT;
    if (!strcmp(name, "CreateIndirectCommandsLayoutEXT")) return (void *)table->CreateIndirectCommandsLayoutEXT;
    if (!strcmp(name, "DestroyIndirectCommandsLayoutEXT")) return (void *)table->DestroyIndirectCommandsLayoutEXT;
    if (!strcmp(name, "CreateIndirectExecutionSetEXT")) return (void *)table->CreateIndirectExecutionSetEXT;
    if (!strcmp(name, "DestroyIndirectExecutionSetEXT")) return (void *)table->DestroyIndirectExecutionSetEXT;
    if (!strcmp(name, "UpdateIndirectExecutionSetPipelineEXT")) return (void *)table->UpdateIndirectExecutionSetPipelineEXT;
    if (!strcmp(name, "UpdateIndirectExecutionSetShaderEXT")) return (void *)table->UpdateIndirectExecutionSetShaderEXT;

    // ---- VK_EXT_external_memory_metal extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp(name, "GetMemoryMetalHandleEXT")) return (void *)table->GetMemoryMetalHandleEXT;
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp(name, "GetMemoryMetalHandlePropertiesEXT")) return (void *)table->GetMemoryMetalHandlePropertiesEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_KHR_acceleration_structure extension commands
    if (!strcmp(name, "CreateAccelerationStructureKHR")) return (void *)table->CreateAccelerationStructureKHR;
    if (!strcmp(name, "DestroyAccelerationStructureKHR")) return (void *)table->DestroyAccelerationStructureKHR;
    if (!strcmp(name, "CmdBuildAccelerationStructuresKHR")) return (void *)table->CmdBuildAccelerationStructuresKHR;
    if (!strcmp(name, "CmdBuildAccelerationStructuresIndirectKHR")) return (void *)table->CmdBuildAccelerationStructuresIndirectKHR;
    if (!strcmp(name, "BuildAccelerationStructuresKHR")) return (void *)table->BuildAccelerationStructuresKHR;
    if (!strcmp(name, "CopyAccelerationStructureKHR")) return (void *)table->CopyAccelerationStructureKHR;
    if (!strcmp(name, "CopyAccelerationStructureToMemoryKHR")) return (void *)table->CopyAccelerationStructureToMemoryKHR;
    if (!strcmp(name, "CopyMemoryToAccelerationStructureKHR")) return (void *)table->CopyMemoryToAccelerationStructureKHR;
    if (!strcmp(name, "WriteAccelerationStructuresPropertiesKHR")) return (void *)table->WriteAccelerationStructuresPropertiesKHR;
    if (!strcmp(name, "CmdCopyAccelerationStructureKHR")) return (void *)table->CmdCopyAccelerationStructureKHR;
    if (!strcmp(name, "CmdCopyAccelerationStructureToMemoryKHR")) return (void *)table->CmdCopyAccelerationStructureToMemoryKHR;
    if (!strcmp(name, "CmdCopyMemoryToAccelerationStructureKHR")) return (void *)table->CmdCopyMemoryToAccelerationStructureKHR;
    if (!strcmp(name, "GetAccelerationStructureDeviceAddressKHR")) return (void *)table->GetAccelerationStructureDeviceAddressKHR;
    if (!strcmp(name, "CmdWriteAccelerationStructuresPropertiesKHR")) return (void *)table->CmdWriteAccelerationStructuresPropertiesKHR;
    if (!strcmp(name, "GetDeviceAccelerationStructureCompatibilityKHR")) return (void *)table->GetDeviceAccelerationStructureCompatibilityKHR;
    if (!strcmp(name, "GetAccelerationStructureBuildSizesKHR")) return (void *)table->GetAccelerationStructureBuildSizesKHR;

    // ---- VK_KHR_ray_tracing_pipeline extension commands
    if (!strcmp(name, "CmdTraceRaysKHR")) return (void *)table->CmdTraceRaysKHR;
    if (!strcmp(name, "CreateRayTracingPipelinesKHR")) return (void *)table->CreateRayTracingPipelinesKHR;
    if (!strcmp(name, "GetRayTracingCaptureReplayShaderGroupHandlesKHR")) return (void *)table->GetRayTracingCaptureReplayShaderGroupHandlesKHR;
    if (!strcmp(name, "CmdTraceRaysIndirectKHR")) return (void *)table->CmdTraceRaysIndirectKHR;
    if (!strcmp(name, "GetRayTracingShaderGroupStackSizeKHR")) return (void *)table->GetRayTracingShaderGroupStackSizeKHR;
    if (!strcmp(name, "CmdSetRayTracingPipelineStackSizeKHR")) return (void *)table->CmdSetRayTracingPipelineStackSizeKHR;

    // ---- VK_EXT_mesh_shader extension commands
    if (!strcmp(name, "CmdDrawMeshTasksEXT")) return (void *)table->CmdDrawMeshTasksEXT;
    if (!strcmp(name, "CmdDrawMeshTasksIndirectEXT")) return (void *)table->CmdDrawMeshTasksIndirectEXT;
    if (!strcmp(name, "CmdDrawMeshTasksIndirectCountEXT")) return (void *)table->CmdDrawMeshTasksIndirectCountEXT;

    *found_name = false;
    return NULL;
}

// Instance command lookup function
VKAPI_ATTR void* VKAPI_CALL loader_lookup_instance_dispatch_table(const VkLayerInstanceDispatchTable *table, const char *name,
                                                                 bool *found_name) {
    if (!name || name[0] != 'v' || name[1] != 'k') {
        *found_name = false;
        return NULL;
    }

    *found_name = true;
    name += 2;

    // ---- Core Vulkan 1.0 commands
    if (!strcmp(name, "DestroyInstance")) return (void *)table->DestroyInstance;
    if (!strcmp(name, "EnumeratePhysicalDevices")) return (void *)table->EnumeratePhysicalDevices;
    if (!strcmp(name, "GetPhysicalDeviceFeatures")) return (void *)table->GetPhysicalDeviceFeatures;
    if (!strcmp(name, "GetPhysicalDeviceFormatProperties")) return (void *)table->GetPhysicalDeviceFormatProperties;
    if (!strcmp(name, "GetPhysicalDeviceImageFormatProperties")) return (void *)table->GetPhysicalDeviceImageFormatProperties;
    if (!strcmp(name, "GetPhysicalDeviceProperties")) return (void *)table->GetPhysicalDeviceProperties;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyProperties")) return (void *)table->GetPhysicalDeviceQueueFamilyProperties;
    if (!strcmp(name, "GetPhysicalDeviceMemoryProperties")) return (void *)table->GetPhysicalDeviceMemoryProperties;
    if (!strcmp(name, "GetInstanceProcAddr")) return (void *)table->GetInstanceProcAddr;
    if (!strcmp(name, "EnumerateDeviceExtensionProperties")) return (void *)table->EnumerateDeviceExtensionProperties;
    if (!strcmp(name, "EnumerateDeviceLayerProperties")) return (void *)table->EnumerateDeviceLayerProperties;
    if (!strcmp(name, "GetPhysicalDeviceSparseImageFormatProperties")) return (void *)table->GetPhysicalDeviceSparseImageFormatProperties;

    // ---- Core Vulkan 1.1 commands
    if (!strcmp(name, "EnumeratePhysicalDeviceGroups")) return (void *)table->EnumeratePhysicalDeviceGroups;
    if (!strcmp(name, "GetPhysicalDeviceFeatures2")) return (void *)table->GetPhysicalDeviceFeatures2;
    if (!strcmp(name, "GetPhysicalDeviceProperties2")) return (void *)table->GetPhysicalDeviceProperties2;
    if (!strcmp(name, "GetPhysicalDeviceFormatProperties2")) return (void *)table->GetPhysicalDeviceFormatProperties2;
    if (!strcmp(name, "GetPhysicalDeviceImageFormatProperties2")) return (void *)table->GetPhysicalDeviceImageFormatProperties2;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyProperties2")) return (void *)table->GetPhysicalDeviceQueueFamilyProperties2;
    if (!strcmp(name, "GetPhysicalDeviceMemoryProperties2")) return (void *)table->GetPhysicalDeviceMemoryProperties2;
    if (!strcmp(name, "GetPhysicalDeviceSparseImageFormatProperties2")) return (void *)table->GetPhysicalDeviceSparseImageFormatProperties2;
    if (!strcmp(name, "GetPhysicalDeviceExternalBufferProperties")) return (void *)table->GetPhysicalDeviceExternalBufferProperties;
    if (!strcmp(name, "GetPhysicalDeviceExternalFenceProperties")) return (void *)table->GetPhysicalDeviceExternalFenceProperties;
    if (!strcmp(name, "GetPhysicalDeviceExternalSemaphoreProperties")) return (void *)table->GetPhysicalDeviceExternalSemaphoreProperties;

    // ---- Core Vulkan 1.3 commands
    if (!strcmp(name, "GetPhysicalDeviceToolProperties")) return (void *)table->GetPhysicalDeviceToolProperties;

    // ---- VK_KHR_surface extension commands
    if (!strcmp(name, "DestroySurfaceKHR")) return (void *)table->DestroySurfaceKHR;
    if (!strcmp(name, "GetPhysicalDeviceSurfaceSupportKHR")) return (void *)table->GetPhysicalDeviceSurfaceSupportKHR;
    if (!strcmp(name, "GetPhysicalDeviceSurfaceCapabilitiesKHR")) return (void *)table->GetPhysicalDeviceSurfaceCapabilitiesKHR;
    if (!strcmp(name, "GetPhysicalDeviceSurfaceFormatsKHR")) return (void *)table->GetPhysicalDeviceSurfaceFormatsKHR;
    if (!strcmp(name, "GetPhysicalDeviceSurfacePresentModesKHR")) return (void *)table->GetPhysicalDeviceSurfacePresentModesKHR;

    // ---- VK_KHR_swapchain extension commands
    if (!strcmp(name, "GetPhysicalDevicePresentRectanglesKHR")) return (void *)table->GetPhysicalDevicePresentRectanglesKHR;

    // ---- VK_KHR_display extension commands
    if (!strcmp(name, "GetPhysicalDeviceDisplayPropertiesKHR")) return (void *)table->GetPhysicalDeviceDisplayPropertiesKHR;
    if (!strcmp(name, "GetPhysicalDeviceDisplayPlanePropertiesKHR")) return (void *)table->GetPhysicalDeviceDisplayPlanePropertiesKHR;
    if (!strcmp(name, "GetDisplayPlaneSupportedDisplaysKHR")) return (void *)table->GetDisplayPlaneSupportedDisplaysKHR;
    if (!strcmp(name, "GetDisplayModePropertiesKHR")) return (void *)table->GetDisplayModePropertiesKHR;
    if (!strcmp(name, "CreateDisplayModeKHR")) return (void *)table->CreateDisplayModeKHR;
    if (!strcmp(name, "GetDisplayPlaneCapabilitiesKHR")) return (void *)table->GetDisplayPlaneCapabilitiesKHR;
    if (!strcmp(name, "CreateDisplayPlaneSurfaceKHR")) return (void *)table->CreateDisplayPlaneSurfaceKHR;

    // ---- VK_KHR_xlib_surface extension commands
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!strcmp(name, "CreateXlibSurfaceKHR")) return (void *)table->CreateXlibSurfaceKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!strcmp(name, "GetPhysicalDeviceXlibPresentationSupportKHR")) return (void *)table->GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR

    // ---- VK_KHR_xcb_surface extension commands
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!strcmp(name, "CreateXcbSurfaceKHR")) return (void *)table->CreateXcbSurfaceKHR;
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!strcmp(name, "GetPhysicalDeviceXcbPresentationSupportKHR")) return (void *)table->GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XCB_KHR

    // ---- VK_KHR_wayland_surface extension commands
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (!strcmp(name, "CreateWaylandSurfaceKHR")) return (void *)table->CreateWaylandSurfaceKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (!strcmp(name, "GetPhysicalDeviceWaylandPresentationSupportKHR")) return (void *)table->GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR

    // ---- VK_KHR_android_surface extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp(name, "CreateAndroidSurfaceKHR")) return (void *)table->CreateAndroidSurfaceKHR;
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_KHR_win32_surface extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "CreateWin32SurfaceKHR")) return (void *)table->CreateWin32SurfaceKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetPhysicalDeviceWin32PresentationSupportKHR")) return (void *)table->GetPhysicalDeviceWin32PresentationSupportKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_video_queue extension commands
    if (!strcmp(name, "GetPhysicalDeviceVideoCapabilitiesKHR")) return (void *)table->GetPhysicalDeviceVideoCapabilitiesKHR;
    if (!strcmp(name, "GetPhysicalDeviceVideoFormatPropertiesKHR")) return (void *)table->GetPhysicalDeviceVideoFormatPropertiesKHR;

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    if (!strcmp(name, "GetPhysicalDeviceFeatures2KHR")) return (void *)table->GetPhysicalDeviceFeatures2KHR;
    if (!strcmp(name, "GetPhysicalDeviceProperties2KHR")) return (void *)table->GetPhysicalDeviceProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceFormatProperties2KHR")) return (void *)table->GetPhysicalDeviceFormatProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceImageFormatProperties2KHR")) return (void *)table->GetPhysicalDeviceImageFormatProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyProperties2KHR")) return (void *)table->GetPhysicalDeviceQueueFamilyProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceMemoryProperties2KHR")) return (void *)table->GetPhysicalDeviceMemoryProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceSparseImageFormatProperties2KHR")) return (void *)table->GetPhysicalDeviceSparseImageFormatProperties2KHR;

    // ---- VK_KHR_device_group_creation extension commands
    if (!strcmp(name, "EnumeratePhysicalDeviceGroupsKHR")) return (void *)table->EnumeratePhysicalDeviceGroupsKHR;

    // ---- VK_KHR_external_memory_capabilities extension commands
    if (!strcmp(name, "GetPhysicalDeviceExternalBufferPropertiesKHR")) return (void *)table->GetPhysicalDeviceExternalBufferPropertiesKHR;

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    if (!strcmp(name, "GetPhysicalDeviceExternalSemaphorePropertiesKHR")) return (void *)table->GetPhysicalDeviceExternalSemaphorePropertiesKHR;

    // ---- VK_KHR_external_fence_capabilities extension commands
    if (!strcmp(name, "GetPhysicalDeviceExternalFencePropertiesKHR")) return (void *)table->GetPhysicalDeviceExternalFencePropertiesKHR;

    // ---- VK_KHR_performance_query extension commands
    if (!strcmp(name, "EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR")) return (void *)table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR")) return (void *)table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;

    // ---- VK_KHR_get_surface_capabilities2 extension commands
    if (!strcmp(name, "GetPhysicalDeviceSurfaceCapabilities2KHR")) return (void *)table->GetPhysicalDeviceSurfaceCapabilities2KHR;
    if (!strcmp(name, "GetPhysicalDeviceSurfaceFormats2KHR")) return (void *)table->GetPhysicalDeviceSurfaceFormats2KHR;

    // ---- VK_KHR_get_display_properties2 extension commands
    if (!strcmp(name, "GetPhysicalDeviceDisplayProperties2KHR")) return (void *)table->GetPhysicalDeviceDisplayProperties2KHR;
    if (!strcmp(name, "GetPhysicalDeviceDisplayPlaneProperties2KHR")) return (void *)table->GetPhysicalDeviceDisplayPlaneProperties2KHR;
    if (!strcmp(name, "GetDisplayModeProperties2KHR")) return (void *)table->GetDisplayModeProperties2KHR;
    if (!strcmp(name, "GetDisplayPlaneCapabilities2KHR")) return (void *)table->GetDisplayPlaneCapabilities2KHR;

    // ---- VK_KHR_fragment_shading_rate extension commands
    if (!strcmp(name, "GetPhysicalDeviceFragmentShadingRatesKHR")) return (void *)table->GetPhysicalDeviceFragmentShadingRatesKHR;

    // ---- VK_KHR_video_encode_queue extension commands
    if (!strcmp(name, "GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR")) return (void *)table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;

    // ---- VK_KHR_cooperative_matrix extension commands
    if (!strcmp(name, "GetPhysicalDeviceCooperativeMatrixPropertiesKHR")) return (void *)table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR;

    // ---- VK_KHR_calibrated_timestamps extension commands
    if (!strcmp(name, "GetPhysicalDeviceCalibrateableTimeDomainsKHR")) return (void *)table->GetPhysicalDeviceCalibrateableTimeDomainsKHR;

    // ---- VK_EXT_debug_report extension commands
    if (!strcmp(name, "CreateDebugReportCallbackEXT")) return (void *)table->CreateDebugReportCallbackEXT;
    if (!strcmp(name, "DestroyDebugReportCallbackEXT")) return (void *)table->DestroyDebugReportCallbackEXT;
    if (!strcmp(name, "DebugReportMessageEXT")) return (void *)table->DebugReportMessageEXT;

    // ---- VK_GGP_stream_descriptor_surface extension commands
#if defined(VK_USE_PLATFORM_GGP)
    if (!strcmp(name, "CreateStreamDescriptorSurfaceGGP")) return (void *)table->CreateStreamDescriptorSurfaceGGP;
#endif // VK_USE_PLATFORM_GGP

    // ---- VK_NV_external_memory_capabilities extension commands
    if (!strcmp(name, "GetPhysicalDeviceExternalImageFormatPropertiesNV")) return (void *)table->GetPhysicalDeviceExternalImageFormatPropertiesNV;

    // ---- VK_NN_vi_surface extension commands
#if defined(VK_USE_PLATFORM_VI_NN)
    if (!strcmp(name, "CreateViSurfaceNN")) return (void *)table->CreateViSurfaceNN;
#endif // VK_USE_PLATFORM_VI_NN

    // ---- VK_EXT_direct_mode_display extension commands
    if (!strcmp(name, "ReleaseDisplayEXT")) return (void *)table->ReleaseDisplayEXT;

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    if (!strcmp(name, "AcquireXlibDisplayEXT")) return (void *)table->AcquireXlibDisplayEXT;
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    if (!strcmp(name, "GetRandROutputDisplayEXT")) return (void *)table->GetRandROutputDisplayEXT;
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    if (!strcmp(name, "GetPhysicalDeviceSurfaceCapabilities2EXT")) return (void *)table->GetPhysicalDeviceSurfaceCapabilities2EXT;

    // ---- VK_MVK_ios_surface extension commands
#if defined(VK_USE_PLATFORM_IOS_MVK)
    if (!strcmp(name, "CreateIOSSurfaceMVK")) return (void *)table->CreateIOSSurfaceMVK;
#endif // VK_USE_PLATFORM_IOS_MVK

    // ---- VK_MVK_macos_surface extension commands
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (!strcmp(name, "CreateMacOSSurfaceMVK")) return (void *)table->CreateMacOSSurfaceMVK;
#endif // VK_USE_PLATFORM_MACOS_MVK

    // ---- VK_EXT_debug_utils extension commands
    if (!strcmp(name, "CreateDebugUtilsMessengerEXT")) return (void *)table->CreateDebugUtilsMessengerEXT;
    if (!strcmp(name, "DestroyDebugUtilsMessengerEXT")) return (void *)table->DestroyDebugUtilsMessengerEXT;
    if (!strcmp(name, "SubmitDebugUtilsMessageEXT")) return (void *)table->SubmitDebugUtilsMessageEXT;

    // ---- VK_EXT_sample_locations extension commands
    if (!strcmp(name, "GetPhysicalDeviceMultisamplePropertiesEXT")) return (void *)table->GetPhysicalDeviceMultisamplePropertiesEXT;

    // ---- VK_EXT_calibrated_timestamps extension commands
    if (!strcmp(name, "GetPhysicalDeviceCalibrateableTimeDomainsEXT")) return (void *)table->GetPhysicalDeviceCalibrateableTimeDomainsEXT;

    // ---- VK_FUCHSIA_imagepipe_surface extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp(name, "CreateImagePipeSurfaceFUCHSIA")) return (void *)table->CreateImagePipeSurfaceFUCHSIA;
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_EXT_metal_surface extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp(name, "CreateMetalSurfaceEXT")) return (void *)table->CreateMetalSurfaceEXT;
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_tooling_info extension commands
    if (!strcmp(name, "GetPhysicalDeviceToolPropertiesEXT")) return (void *)table->GetPhysicalDeviceToolPropertiesEXT;

    // ---- VK_NV_cooperative_matrix extension commands
    if (!strcmp(name, "GetPhysicalDeviceCooperativeMatrixPropertiesNV")) return (void *)table->GetPhysicalDeviceCooperativeMatrixPropertiesNV;

    // ---- VK_NV_coverage_reduction_mode extension commands
    if (!strcmp(name, "GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV")) return (void *)table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetPhysicalDeviceSurfacePresentModes2EXT")) return (void *)table->GetPhysicalDeviceSurfacePresentModes2EXT;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_headless_surface extension commands
    if (!strcmp(name, "CreateHeadlessSurfaceEXT")) return (void *)table->CreateHeadlessSurfaceEXT;

    // ---- VK_EXT_acquire_drm_display extension commands
    if (!strcmp(name, "AcquireDrmDisplayEXT")) return (void *)table->AcquireDrmDisplayEXT;
    if (!strcmp(name, "GetDrmDisplayEXT")) return (void *)table->GetDrmDisplayEXT;

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "AcquireWinrtDisplayNV")) return (void *)table->AcquireWinrtDisplayNV;
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "GetWinrtDisplayNV")) return (void *)table->GetWinrtDisplayNV;
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_directfb_surface extension commands
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (!strcmp(name, "CreateDirectFBSurfaceEXT")) return (void *)table->CreateDirectFBSurfaceEXT;
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (!strcmp(name, "GetPhysicalDeviceDirectFBPresentationSupportEXT")) return (void *)table->GetPhysicalDeviceDirectFBPresentationSupportEXT;
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

    // ---- VK_QNX_screen_surface extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (!strcmp(name, "CreateScreenSurfaceQNX")) return (void *)table->CreateScreenSurfaceQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (!strcmp(name, "GetPhysicalDeviceScreenPresentationSupportQNX")) return (void *)table->GetPhysicalDeviceScreenPresentationSupportQNX;
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_optical_flow extension commands
    if (!strcmp(name, "GetPhysicalDeviceOpticalFlowImageFormatsNV")) return (void *)table->GetPhysicalDeviceOpticalFlowImageFormatsNV;

    // ---- VK_NV_cooperative_vector extension commands
    if (!strcmp(name, "GetPhysicalDeviceCooperativeVectorPropertiesNV")) return (void *)table->GetPhysicalDeviceCooperativeVectorPropertiesNV;

    // ---- VK_NV_cooperative_matrix2 extension commands
    if (!strcmp(name, "GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV")) return (void *)table->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV;

    *found_name = false;
    return NULL;
}


// ---- VK_KHR_video_queue extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceVideoCapabilitiesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceVideoCapabilitiesKHR(unwrapped_phys_dev, pVideoProfile, pCapabilities);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceVideoCapabilitiesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceVideoCapabilitiesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceVideoCapabilitiesKHR(phys_dev_term->phys_dev, pVideoProfile, pCapabilities);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceVideoFormatPropertiesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceVideoFormatPropertiesKHR(unwrapped_phys_dev, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceVideoFormatPropertiesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceVideoFormatPropertiesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceVideoFormatPropertiesKHR(phys_dev_term->phys_dev, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateVideoSessionKHR: Invalid device "
                   "[VUID-vkCreateVideoSessionKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
}

VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyVideoSessionKHR: Invalid device "
                   "[VUID-vkDestroyVideoSessionKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyVideoSessionKHR(device, videoSession, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL GetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetVideoSessionMemoryRequirementsKHR: Invalid device "
                   "[VUID-vkGetVideoSessionMemoryRequirementsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL BindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindVideoSessionMemoryKHR: Invalid device "
                   "[VUID-vkBindVideoSessionMemoryKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateVideoSessionParametersKHR: Invalid device "
                   "[VUID-vkCreateVideoSessionParametersKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
}

VKAPI_ATTR VkResult VKAPI_CALL UpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateVideoSessionParametersKHR: Invalid device "
                   "[VUID-vkUpdateVideoSessionParametersKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->UpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
}

VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyVideoSessionParametersKHR: Invalid device "
                   "[VUID-vkDestroyVideoSessionParametersKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginVideoCodingKHR: Invalid commandBuffer "
                   "[VUID-vkCmdBeginVideoCodingKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndVideoCodingKHR: Invalid commandBuffer "
                   "[VUID-vkCmdEndVideoCodingKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdControlVideoCodingKHR: Invalid commandBuffer "
                   "[VUID-vkCmdControlVideoCodingKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}


// ---- VK_KHR_video_decode_queue extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDecodeVideoKHR: Invalid commandBuffer "
                   "[VUID-vkCmdDecodeVideoKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
}


// ---- VK_KHR_dynamic_rendering extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginRenderingKHR: Invalid commandBuffer "
                   "[VUID-vkCmdBeginRenderingKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndRenderingKHR: Invalid commandBuffer "
                   "[VUID-vkCmdEndRenderingKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndRenderingKHR(commandBuffer);
}


// ---- VK_KHR_device_group extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetDeviceGroupPeerMemoryFeaturesKHR(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlags*                   pPeerMemoryFeatures) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceGroupPeerMemoryFeaturesKHR: Invalid device "
                   "[VUID-vkGetDeviceGroupPeerMemoryFeaturesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDeviceMaskKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetDeviceMaskKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}

VKAPI_ATTR void VKAPI_CALL CmdDispatchBaseKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchBaseKHR: Invalid commandBuffer "
                   "[VUID-vkCmdDispatchBaseKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}


// ---- VK_KHR_maintenance1 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkTrimCommandPoolKHR: Invalid device "
                   "[VUID-vkTrimCommandPoolKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->TrimCommandPoolKHR(device, commandPool, flags);
}


// ---- VK_KHR_external_memory_win32 extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryWin32HandleKHR: Invalid device "
                   "[VUID-vkGetMemoryWin32HandleKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryWin32HandlePropertiesKHR: Invalid device "
                   "[VUID-vkGetMemoryWin32HandlePropertiesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_KHR_external_memory_fd extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryFdKHR: Invalid device "
                   "[VUID-vkGetMemoryFdKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryFdKHR(device, pGetFdInfo, pFd);
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryFdPropertiesKHR: Invalid device "
                   "[VUID-vkGetMemoryFdPropertiesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
}


// ---- VK_KHR_external_semaphore_win32 extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkImportSemaphoreWin32HandleKHR: Invalid device "
                   "[VUID-vkImportSemaphoreWin32HandleKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSemaphoreWin32HandleKHR: Invalid device "
                   "[VUID-vkGetSemaphoreWin32HandleKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_KHR_external_semaphore_fd extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkImportSemaphoreFdKHR: Invalid device "
                   "[VUID-vkImportSemaphoreFdKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSemaphoreFdKHR: Invalid device "
                   "[VUID-vkGetSemaphoreFdKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSemaphoreFdKHR(device, pGetFdInfo, pFd);
}


// ---- VK_KHR_push_descriptor extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSetKHR: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSetKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSetWithTemplateKHR: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}


// ---- VK_KHR_descriptor_update_template extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDescriptorUpdateTemplateKHR: Invalid device "
                   "[VUID-vkCreateDescriptorUpdateTemplateKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDescriptorUpdateTemplateKHR: Invalid device "
                   "[VUID-vkDestroyDescriptorUpdateTemplateKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateDescriptorSetWithTemplateKHR: Invalid device "
                   "[VUID-vkUpdateDescriptorSetWithTemplateKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->UpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
}


// ---- VK_KHR_create_renderpass2 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass2KHR(
    VkDevice                                    device,
    const VkRenderPassCreateInfo2*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateRenderPass2KHR: Invalid device "
                   "[VUID-vkCreateRenderPass2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderPassBeginInfo*                pRenderPassBegin,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginRenderPass2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdBeginRenderPass2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdNextSubpass2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdNextSubpass2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndRenderPass2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdEndRenderPass2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}


// ---- VK_KHR_shared_presentable_image extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSwapchainStatusKHR: Invalid device "
                   "[VUID-vkGetSwapchainStatusKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSwapchainStatusKHR(device, swapchain);
}


// ---- VK_KHR_external_fence_win32 extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkImportFenceWin32HandleKHR: Invalid device "
                   "[VUID-vkImportFenceWin32HandleKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetFenceWin32HandleKHR: Invalid device "
                   "[VUID-vkGetFenceWin32HandleKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_KHR_external_fence_fd extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkImportFenceFdKHR: Invalid device "
                   "[VUID-vkImportFenceFdKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ImportFenceFdKHR(device, pImportFenceFdInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL GetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetFenceFdKHR: Invalid device "
                   "[VUID-vkGetFenceFdKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetFenceFdKHR(device, pGetFdInfo, pFd);
}


// ---- VK_KHR_performance_query extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR: Invalid physicalDevice "
                   "[VUID-vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(unwrapped_phys_dev, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(phys_dev_term->phys_dev, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(unwrapped_phys_dev, pPerformanceQueryCreateInfo, pNumPasses);
}

VKAPI_ATTR void VKAPI_CALL terminator_GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkQueryPoolPerformanceCreateInfoKHR*  pPerformanceQueryCreateInfo,
    uint32_t*                                   pNumPasses) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    icd_term->dispatch.GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(phys_dev_term->phys_dev, pPerformanceQueryCreateInfo, pNumPasses);
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAcquireProfilingLockKHR: Invalid device "
                   "[VUID-vkAcquireProfilingLockKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->AcquireProfilingLockKHR(device, pInfo);
}

VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkReleaseProfilingLockKHR: Invalid device "
                   "[VUID-vkReleaseProfilingLockKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->ReleaseProfilingLockKHR(device);
}


// ---- VK_KHR_get_memory_requirements2 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2*       pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageMemoryRequirements2KHR: Invalid device "
                   "[VUID-vkGetImageMemoryRequirements2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferMemoryRequirements2KHR: Invalid device "
                   "[VUID-vkGetBufferMemoryRequirements2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSparseMemoryRequirements2KHR: Invalid device "
                   "[VUID-vkGetImageSparseMemoryRequirements2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}


// ---- VK_KHR_sampler_ycbcr_conversion extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfo*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversion*                   pYcbcrConversion) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateSamplerYcbcrConversionKHR: Invalid device "
                   "[VUID-vkCreateSamplerYcbcrConversionKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

VKAPI_ATTR void VKAPI_CALL DestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversion                    ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroySamplerYcbcrConversionKHR: Invalid device "
                   "[VUID-vkDestroySamplerYcbcrConversionKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
}


// ---- VK_KHR_bind_memory2 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindBufferMemory2KHR: Invalid device "
                   "[VUID-vkBindBufferMemory2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfo*                pBindInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindImageMemory2KHR: Invalid device "
                   "[VUID-vkBindImageMemory2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
}


// ---- VK_KHR_maintenance3 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    VkDescriptorSetLayoutSupport*               pSupport) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetLayoutSupportKHR: Invalid device "
                   "[VUID-vkGetDescriptorSetLayoutSupportKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}


// ---- VK_KHR_draw_indirect_count extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndirectCountKHR: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndirectCountKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountKHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndexedIndirectCountKHR: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndexedIndirectCountKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}


// ---- VK_KHR_timeline_semaphore extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSemaphoreCounterValueKHR: Invalid device "
                   "[VUID-vkGetSemaphoreCounterValueKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSemaphoreCounterValueKHR(device, semaphore, pValue);
}

VKAPI_ATTR VkResult VKAPI_CALL WaitSemaphoresKHR(
    VkDevice                                    device,
    const VkSemaphoreWaitInfo*                  pWaitInfo,
    uint64_t                                    timeout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWaitSemaphoresKHR: Invalid device "
                   "[VUID-vkWaitSemaphoresKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->WaitSemaphoresKHR(device, pWaitInfo, timeout);
}

VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSignalSemaphoreKHR: Invalid device "
                   "[VUID-vkSignalSemaphoreKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SignalSemaphoreKHR(device, pSignalInfo);
}


// ---- VK_KHR_fragment_shading_rate extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceFragmentShadingRatesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceFragmentShadingRatesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceFragmentShadingRatesKHR(unwrapped_phys_dev, pFragmentShadingRateCount, pFragmentShadingRates);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceFragmentShadingRatesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceFragmentShadingRatesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceFragmentShadingRatesKHR(phys_dev_term->phys_dev, pFragmentShadingRateCount, pFragmentShadingRates);
}

VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateKHR(
    VkCommandBuffer                             commandBuffer,
    const VkExtent2D*                           pFragmentSize,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetFragmentShadingRateKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetFragmentShadingRateKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}


// ---- VK_KHR_dynamic_rendering_local_read extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetRenderingAttachmentLocationsKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingAttachmentLocationInfo*    pLocationInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRenderingAttachmentLocationsKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetRenderingAttachmentLocationsKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRenderingAttachmentLocationsKHR(commandBuffer, pLocationInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRenderingInputAttachmentIndicesKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInputAttachmentIndexInfo*  pInputAttachmentIndexInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRenderingInputAttachmentIndicesKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetRenderingInputAttachmentIndicesKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRenderingInputAttachmentIndicesKHR(commandBuffer, pInputAttachmentIndexInfo);
}


// ---- VK_KHR_present_wait extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL WaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWaitForPresentKHR: Invalid device "
                   "[VUID-vkWaitForPresentKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->WaitForPresentKHR(device, swapchain, presentId, timeout);
}


// ---- VK_KHR_buffer_device_address extension trampoline/terminators

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferDeviceAddressKHR: Invalid device "
                   "[VUID-vkGetBufferDeviceAddressKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferDeviceAddressKHR(device, pInfo);
}

VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferOpaqueCaptureAddressKHR: Invalid device "
                   "[VUID-vkGetBufferOpaqueCaptureAddressKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferOpaqueCaptureAddressKHR(device, pInfo);
}

VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceMemoryOpaqueCaptureAddressKHR: Invalid device "
                   "[VUID-vkGetDeviceMemoryOpaqueCaptureAddressKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
}


// ---- VK_KHR_deferred_host_operations extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateDeferredOperationKHR(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator,
    VkDeferredOperationKHR*                     pDeferredOperation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateDeferredOperationKHR: Invalid device "
                   "[VUID-vkCreateDeferredOperationKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
}

VKAPI_ATTR void VKAPI_CALL DestroyDeferredOperationKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyDeferredOperationKHR: Invalid device "
                   "[VUID-vkDestroyDeferredOperationKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyDeferredOperationKHR(device, operation, pAllocator);
}

VKAPI_ATTR uint32_t VKAPI_CALL GetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeferredOperationMaxConcurrencyKHR: Invalid device "
                   "[VUID-vkGetDeferredOperationMaxConcurrencyKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeferredOperationMaxConcurrencyKHR(device, operation);
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeferredOperationResultKHR: Invalid device "
                   "[VUID-vkGetDeferredOperationResultKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeferredOperationResultKHR(device, operation);
}

VKAPI_ATTR VkResult VKAPI_CALL DeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDeferredOperationJoinKHR: Invalid device "
                   "[VUID-vkDeferredOperationJoinKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->DeferredOperationJoinKHR(device, operation);
}


// ---- VK_KHR_pipeline_executable_properties extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutablePropertiesKHR(
    VkDevice                                    device,
    const VkPipelineInfoKHR*                    pPipelineInfo,
    uint32_t*                                   pExecutableCount,
    VkPipelineExecutablePropertiesKHR*          pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineExecutablePropertiesKHR: Invalid device "
                   "[VUID-vkGetPipelineExecutablePropertiesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableStatisticsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pStatisticCount,
    VkPipelineExecutableStatisticKHR*           pStatistics) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineExecutableStatisticsKHR: Invalid device "
                   "[VUID-vkGetPipelineExecutableStatisticsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice                                    device,
    const VkPipelineExecutableInfoKHR*          pExecutableInfo,
    uint32_t*                                   pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineExecutableInternalRepresentationsKHR: Invalid device "
                   "[VUID-vkGetPipelineExecutableInternalRepresentationsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}


// ---- VK_KHR_map_memory2 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL MapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryMapInfo*                      pMemoryMapInfo,
    void**                                      ppData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkMapMemory2KHR: Invalid device "
                   "[VUID-vkMapMemory2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->MapMemory2KHR(device, pMemoryMapInfo, ppData);
}

VKAPI_ATTR VkResult VKAPI_CALL UnmapMemory2KHR(
    VkDevice                                    device,
    const VkMemoryUnmapInfo*                    pMemoryUnmapInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUnmapMemory2KHR: Invalid device "
                   "[VUID-vkUnmapMemory2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->UnmapMemory2KHR(device, pMemoryUnmapInfo);
}


// ---- VK_KHR_video_encode_queue extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR*     pQualityLevelProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(unwrapped_phys_dev, pQualityLevelInfo, pQualityLevelProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR*     pQualityLevelProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(phys_dev_term->phys_dev, pQualityLevelInfo, pQualityLevelProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetEncodedVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo,
    size_t*                                     pDataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetEncodedVideoSessionParametersKHR: Invalid device "
                   "[VUID-vkGetEncodedVideoSessionParametersKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetEncodedVideoSessionParametersKHR(device, pVideoSessionParametersInfo, pFeedbackInfo, pDataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL CmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEncodeVideoKHR: Invalid commandBuffer "
                   "[VUID-vkCmdEncodeVideoKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
}


// ---- VK_KHR_synchronization2 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetEvent2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetEvent2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResetEvent2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdResetEvent2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdResetEvent2KHR(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWaitEvents2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdWaitEvents2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPipelineBarrier2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdPipelineBarrier2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteTimestamp2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdWriteTimestamp2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueSubmit2KHR: Invalid queue "
                   "[VUID-vkQueueSubmit2KHR-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->QueueSubmit2KHR(queue, submitCount, pSubmits, fence);
}


// ---- VK_KHR_copy_commands2 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBuffer2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyBuffer2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImage2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyImage2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyBufferToImage2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyBufferToImage2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyImageToBuffer2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyImageToBuffer2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBlitImage2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdBlitImage2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdResolveImage2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdResolveImage2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
}


// ---- VK_KHR_ray_tracing_maintenance1 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdTraceRaysIndirect2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdTraceRaysIndirect2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}


// ---- VK_KHR_maintenance4 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceBufferMemoryRequirementsKHR: Invalid device "
                   "[VUID-vkGetDeviceBufferMemoryRequirementsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageMemoryRequirementsKHR: Invalid device "
                   "[VUID-vkGetDeviceImageMemoryRequirementsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageSparseMemoryRequirementsKHR: Invalid device "
                   "[VUID-vkGetDeviceImageSparseMemoryRequirementsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}


// ---- VK_KHR_maintenance5 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkIndexType                                 indexType) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindIndexBuffer2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdBindIndexBuffer2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindIndexBuffer2KHR(commandBuffer, buffer, offset, size, indexType);
}

VKAPI_ATTR void VKAPI_CALL GetRenderingAreaGranularityKHR(
    VkDevice                                    device,
    const VkRenderingAreaInfo*                  pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRenderingAreaGranularityKHR: Invalid device "
                   "[VUID-vkGetRenderingAreaGranularityKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetRenderingAreaGranularityKHR(device, pRenderingAreaInfo, pGranularity);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageSubresourceLayoutKHR(
    VkDevice                                    device,
    const VkDeviceImageSubresourceInfo*         pInfo,
    VkSubresourceLayout2*                       pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceImageSubresourceLayoutKHR: Invalid device "
                   "[VUID-vkGetDeviceImageSubresourceLayoutKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceImageSubresourceLayoutKHR(device, pInfo, pLayout);
}

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2KHR(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSubresourceLayout2KHR: Invalid device "
                   "[VUID-vkGetImageSubresourceLayout2KHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageSubresourceLayout2KHR(device, image, pSubresource, pLayout);
}


// ---- VK_KHR_pipeline_binary extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineBinariesKHR(
    VkDevice                                    device,
    const VkPipelineBinaryCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineBinaryHandlesInfoKHR*             pBinaries) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreatePipelineBinariesKHR: Invalid device "
                   "[VUID-vkCreatePipelineBinariesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreatePipelineBinariesKHR(device, pCreateInfo, pAllocator, pBinaries);
}

VKAPI_ATTR void VKAPI_CALL DestroyPipelineBinaryKHR(
    VkDevice                                    device,
    VkPipelineBinaryKHR                         pipelineBinary,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPipelineBinaryKHR: Invalid device "
                   "[VUID-vkDestroyPipelineBinaryKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyPipelineBinaryKHR(device, pipelineBinary, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineKeyKHR(
    VkDevice                                    device,
    const VkPipelineCreateInfoKHR*              pPipelineCreateInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineKey) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineKeyKHR: Invalid device "
                   "[VUID-vkGetPipelineKeyKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineKeyKHR(device, pPipelineCreateInfo, pPipelineKey);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPipelineBinaryDataKHR(
    VkDevice                                    device,
    const VkPipelineBinaryDataInfoKHR*          pInfo,
    VkPipelineBinaryKeyKHR*                     pPipelineBinaryKey,
    size_t*                                     pPipelineBinaryDataSize,
    void*                                       pPipelineBinaryData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineBinaryDataKHR: Invalid device "
                   "[VUID-vkGetPipelineBinaryDataKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineBinaryDataKHR(device, pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize, pPipelineBinaryData);
}

VKAPI_ATTR VkResult VKAPI_CALL ReleaseCapturedPipelineDataKHR(
    VkDevice                                    device,
    const VkReleaseCapturedPipelineDataInfoKHR* pInfo,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkReleaseCapturedPipelineDataKHR: Invalid device "
                   "[VUID-vkReleaseCapturedPipelineDataKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ReleaseCapturedPipelineDataKHR(device, pInfo, pAllocator);
}


// ---- VK_KHR_cooperative_matrix extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesKHR*           pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCooperativeMatrixPropertiesKHR(unwrapped_phys_dev, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesKHR*           pProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixPropertiesKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCooperativeMatrixPropertiesKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(phys_dev_term->phys_dev, pPropertyCount, pProperties);
}


// ---- VK_KHR_line_rasterization extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineStippleKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetLineStippleKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLineStippleKHR(commandBuffer, lineStippleFactor, lineStipplePattern);
}


// ---- VK_KHR_calibrated_timestamps extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCalibrateableTimeDomainsKHR-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCalibrateableTimeDomainsKHR(unwrapped_phys_dev, pTimeDomainCount, pTimeDomains);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCalibrateableTimeDomainsKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCalibrateableTimeDomainsKHR) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCalibrateableTimeDomainsKHR");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCalibrateableTimeDomainsKHR(phys_dev_term->phys_dev, pTimeDomainCount, pTimeDomains);
}

VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsKHR(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetCalibratedTimestampsKHR: Invalid device "
                   "[VUID-vkGetCalibratedTimestampsKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}


// ---- VK_KHR_maintenance6 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorSetsInfo*             pBindDescriptorSetsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorSets2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdBindDescriptorSets2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdPushConstants2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushConstantsInfo*                  pPushConstantsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushConstants2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdPushConstants2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushConstants2KHR(commandBuffer, pPushConstantsInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSet2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetInfo*              pPushDescriptorSetInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSet2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSet2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSet2KHR(commandBuffer, pPushDescriptorSetInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkPushDescriptorSetWithTemplateInfo*  pPushDescriptorSetWithTemplateInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPushDescriptorSetWithTemplate2KHR: Invalid commandBuffer "
                   "[VUID-vkCmdPushDescriptorSetWithTemplate2KHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPushDescriptorSetWithTemplate2KHR(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkSetDescriptorBufferOffsetsInfoEXT*  pSetDescriptorBufferOffsetsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDescriptorBufferOffsets2EXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDescriptorBufferOffsets2EXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDescriptorBufferOffsets2EXT(commandBuffer, pSetDescriptorBufferOffsetsInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindDescriptorBufferEmbeddedSamplers2EXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindDescriptorBufferEmbeddedSamplers2EXT(commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo);
}


// ---- VK_EXT_debug_marker extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDebugMarkerSetObjectTagEXT: Invalid device "
                   "[VUID-vkDebugMarkerSetObjectTagEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugMarkerObjectTagInfoEXT local_tag_info;
    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugMarkerObjectTagInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->object;
        local_tag_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;
    }
    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->object;
        local_tag_info.object = (uint64_t)(uintptr_t)instance->instance;
    }
    return disp->DebugMarkerSetObjectTagEXT(device, &local_tag_info);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    struct loader_device *dev;
    struct loader_icd_term *icd_term = loader_get_icd_and_device(device, &dev);
    if (NULL == icd_term || NULL == dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "DebugMarkerSetObjectTagEXT: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugMarkerObjectTagInfoEXT local_tag_info;
    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugMarkerObjectTagInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t)pTagInfo->object;
        local_tag_info.object = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;
    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.
    } else if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT) {
        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {
            VkIcdSurface *icd_surface = (VkIcdSurface *)(uintptr_t)pTagInfo->object;
            if (NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR)
                && icd_term->surface_list.list[icd_surface->surface_index]) {
                local_tag_info.object = (uint64_t)icd_term->surface_list.list[icd_surface->surface_index];
            }
        }
    // If this is an instance we have to replace it with the proper one for the next call.
    } else if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        local_tag_info.object = (uint64_t)(uintptr_t)icd_term->instance;
    }
    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports
    // debug utils but the driver does not.
    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.DebugMarkerSetObjectTagEXT)
        return VK_SUCCESS;
    return dev->loader_dispatch.extension_terminator_dispatch.DebugMarkerSetObjectTagEXT(device, &local_tag_info);
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDebugMarkerSetObjectNameEXT: Invalid device "
                   "[VUID-vkDebugMarkerSetObjectNameEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugMarkerObjectNameInfoEXT local_name_info;
    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugMarkerObjectNameInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->object;
        local_name_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;
    }
    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->object;
        local_name_info.object = (uint64_t)(uintptr_t)instance->instance;
    }
    return disp->DebugMarkerSetObjectNameEXT(device, &local_name_info);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    struct loader_device *dev;
    struct loader_icd_term *icd_term = loader_get_icd_and_device(device, &dev);
    if (NULL == icd_term || NULL == dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "DebugMarkerSetObjectNameEXT: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugMarkerObjectNameInfoEXT local_name_info;
    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugMarkerObjectNameInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t)pNameInfo->object;
        local_name_info.object = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;
    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.
    } else if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT) {
        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {
            VkIcdSurface *icd_surface = (VkIcdSurface *)(uintptr_t)pNameInfo->object;
            if (NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR)
                && icd_term->surface_list.list[icd_surface->surface_index]) {
                local_name_info.object = (uint64_t)icd_term->surface_list.list[icd_surface->surface_index];
            }
        }
    // If this is an instance we have to replace it with the proper one for the next call.
    } else if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        local_name_info.object = (uint64_t)(uintptr_t)icd_term->instance;
    }
    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports
    // debug utils but the driver does not.
    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.DebugMarkerSetObjectNameEXT)
        return VK_SUCCESS;
    return dev->loader_dispatch.extension_terminator_dispatch.DebugMarkerSetObjectNameEXT(device, &local_name_info);
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDebugMarkerBeginEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDebugMarkerEndEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDebugMarkerEndEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDebugMarkerEndEXT(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDebugMarkerInsertEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}


// ---- VK_EXT_transform_feedback extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBindTransformFeedbackBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindTransformFeedbackBuffersEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindTransformFeedbackBuffersEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginTransformFeedbackEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBeginTransformFeedbackEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

VKAPI_ATTR void VKAPI_CALL CmdEndTransformFeedbackEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstCounterBuffer,
    uint32_t                                    counterBufferCount,
    const VkBuffer*                             pCounterBuffers,
    const VkDeviceSize*                         pCounterBufferOffsets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndTransformFeedbackEXT: Invalid commandBuffer "
                   "[VUID-vkCmdEndTransformFeedbackEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    VkQueryControlFlags                         flags,
    uint32_t                                    index) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginQueryIndexedEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBeginQueryIndexedEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}

VKAPI_ATTR void VKAPI_CALL CmdEndQueryIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    VkQueryPool                                 queryPool,
    uint32_t                                    query,
    uint32_t                                    index) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndQueryIndexedEXT: Invalid commandBuffer "
                   "[VUID-vkCmdEndQueryIndexedEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectByteCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    VkBuffer                                    counterBuffer,
    VkDeviceSize                                counterBufferOffset,
    uint32_t                                    counterOffset,
    uint32_t                                    vertexStride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndirectByteCountEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndirectByteCountEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}


// ---- VK_NVX_binary_import extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateCuModuleNVX: Invalid device "
                   "[VUID-vkCreateCuModuleNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateCuFunctionNVX: Invalid device "
                   "[VUID-vkCreateCuFunctionNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
}

VKAPI_ATTR void VKAPI_CALL DestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyCuModuleNVX: Invalid device "
                   "[VUID-vkDestroyCuModuleNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyCuModuleNVX(device, module, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyCuFunctionNVX: Invalid device "
                   "[VUID-vkDestroyCuFunctionNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyCuFunctionNVX(device, function, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL CmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCuLaunchKernelNVX: Invalid commandBuffer "
                   "[VUID-vkCmdCuLaunchKernelNVX-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
}


// ---- VK_NVX_image_view_handle extension trampoline/terminators

VKAPI_ATTR uint32_t VKAPI_CALL GetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageViewHandleNVX: Invalid device "
                   "[VUID-vkGetImageViewHandleNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageViewHandleNVX(device, pInfo);
}

VKAPI_ATTR uint64_t VKAPI_CALL GetImageViewHandle64NVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageViewHandle64NVX: Invalid device "
                   "[VUID-vkGetImageViewHandle64NVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageViewHandle64NVX(device, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageViewAddressNVX(
    VkDevice                                    device,
    VkImageView                                 imageView,
    VkImageViewAddressPropertiesNVX*            pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageViewAddressNVX: Invalid device "
                   "[VUID-vkGetImageViewAddressNVX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageViewAddressNVX(device, imageView, pProperties);
}


// ---- VK_AMD_draw_indirect_count extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndirectCountAMD: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndirectCountAMD-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawIndexedIndirectCountAMD: Invalid commandBuffer "
                   "[VUID-vkCmdDrawIndexedIndirectCountAMD-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}


// ---- VK_AMD_shader_info extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetShaderInfoAMD: Invalid device "
                   "[VUID-vkGetShaderInfoAMD-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}


// ---- VK_NV_external_memory_win32 extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryWin32HandleNV: Invalid device "
                   "[VUID-vkGetMemoryWin32HandleNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryWin32HandleNV(device, memory, handleType, pHandle);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_EXT_conditional_rendering extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBeginConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer,
    const VkConditionalRenderingBeginInfoEXT*   pConditionalRenderingBegin) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginConditionalRenderingEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBeginConditionalRenderingEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
}

VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndConditionalRenderingEXT: Invalid commandBuffer "
                   "[VUID-vkCmdEndConditionalRenderingEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdEndConditionalRenderingEXT(commandBuffer);
}


// ---- VK_NV_clip_space_w_scaling extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportWScalingNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportWScalingNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}


// ---- VK_EXT_display_control extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL DisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDisplayPowerControlEXT: Invalid device "
                   "[VUID-vkDisplayPowerControlEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->DisplayPowerControlEXT(device, display, pDisplayPowerInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkRegisterDeviceEventEXT: Invalid device "
                   "[VUID-vkRegisterDeviceEventEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->RegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
}

VKAPI_ATTR VkResult VKAPI_CALL RegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkRegisterDisplayEventEXT: Invalid device "
                   "[VUID-vkRegisterDisplayEventEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->RegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSwapchainCounterEXT: Invalid device "
                   "[VUID-vkGetSwapchainCounterEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
}


// ---- VK_GOOGLE_display_timing extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRefreshCycleDurationGOOGLE: Invalid device "
                   "[VUID-vkGetRefreshCycleDurationGOOGLE-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPastPresentationTimingGOOGLE: Invalid device "
                   "[VUID-vkGetPastPresentationTimingGOOGLE-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
}


// ---- VK_EXT_discard_rectangles extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDiscardRectangleEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDiscardRectangleEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    discardRectangleEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDiscardRectangleEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDiscardRectangleEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDiscardRectangleEnableEXT(commandBuffer, discardRectangleEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDiscardRectangleModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDiscardRectangleModeEXT                   discardRectangleMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDiscardRectangleModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDiscardRectangleModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDiscardRectangleModeEXT(commandBuffer, discardRectangleMode);
}


// ---- VK_EXT_hdr_metadata extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL SetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetHdrMetadataEXT: Invalid device "
                   "[VUID-vkSetHdrMetadataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
}


// ---- VK_EXT_debug_utils extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetDebugUtilsObjectNameEXT: Invalid device "
                   "[VUID-vkSetDebugUtilsObjectNameEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugUtilsObjectNameInfoEXT local_name_info;
    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugUtilsObjectNameInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pNameInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->objectHandle;
        local_name_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;
    }
    if (pNameInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->objectHandle;
        local_name_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;
    }
    if (disp->SetDebugUtilsObjectNameEXT != NULL) {
        return disp->SetDebugUtilsObjectNameEXT(device, &local_name_info);
    } else {
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_SetDebugUtilsObjectNameEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectNameInfoEXT*        pNameInfo) {
    struct loader_device *dev;
    struct loader_icd_term *icd_term = loader_get_icd_and_device(device, &dev);
    if (NULL == icd_term || NULL == dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "SetDebugUtilsObjectNameEXT: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugUtilsObjectNameInfoEXT local_name_info;
    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugUtilsObjectNameInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pNameInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t)pNameInfo->objectHandle;
        local_name_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;
    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.
    } else if (pNameInfo->objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {
            VkIcdSurface *icd_surface = (VkIcdSurface *)(uintptr_t)pNameInfo->objectHandle;
            if (NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR)
                && icd_term->surface_list.list[icd_surface->surface_index]) {
                local_name_info.objectHandle = (uint64_t)icd_term->surface_list.list[icd_surface->surface_index];
            }
        }
    // If this is an instance we have to replace it with the proper one for the next call.
    } else if (pNameInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        local_name_info.objectHandle = (uint64_t)(uintptr_t)icd_term->instance;
    }
    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports
    // debug utils but the driver does not.
    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.SetDebugUtilsObjectNameEXT)
        return VK_SUCCESS;
    return dev->loader_dispatch.extension_terminator_dispatch.SetDebugUtilsObjectNameEXT(device, &local_name_info);
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetDebugUtilsObjectTagEXT: Invalid device "
                   "[VUID-vkSetDebugUtilsObjectTagEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugUtilsObjectTagInfoEXT local_tag_info;
    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugUtilsObjectTagInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pTagInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->objectHandle;
        local_tag_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;
    }
    if (pTagInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->objectHandle;
        local_tag_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;
    }
    if (disp->SetDebugUtilsObjectTagEXT != NULL) {
        return disp->SetDebugUtilsObjectTagEXT(device, &local_tag_info);
    } else {
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_SetDebugUtilsObjectTagEXT(
    VkDevice                                    device,
    const VkDebugUtilsObjectTagInfoEXT*         pTagInfo) {
    struct loader_device *dev;
    struct loader_icd_term *icd_term = loader_get_icd_and_device(device, &dev);
    if (NULL == icd_term || NULL == dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "SetDebugUtilsObjectTagEXT: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    VkDebugUtilsObjectTagInfoEXT local_tag_info;
    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugUtilsObjectTagInfoEXT));
    // If this is a physical device, we have to replace it with the proper one for the next call.
    if (pTagInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t)pTagInfo->objectHandle;
        local_tag_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;
    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.
    } else if (pTagInfo->objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {
            VkIcdSurface *icd_surface = (VkIcdSurface *)(uintptr_t)pTagInfo->objectHandle;
            if (NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR)
                && icd_term->surface_list.list[icd_surface->surface_index]) {
                local_tag_info.objectHandle = (uint64_t)icd_term->surface_list.list[icd_surface->surface_index];
            }
        }
    // If this is an instance we have to replace it with the proper one for the next call.
    } else if (pTagInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        local_tag_info.objectHandle = (uint64_t)(uintptr_t)icd_term->instance;
    }
    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports
    // debug utils but the driver does not.
    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.SetDebugUtilsObjectTagEXT)
        return VK_SUCCESS;
    return dev->loader_dispatch.extension_terminator_dispatch.SetDebugUtilsObjectTagEXT(device, &local_tag_info);
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueBeginDebugUtilsLabelEXT: Invalid queue "
                   "[VUID-vkQueueBeginDebugUtilsLabelEXT-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->QueueBeginDebugUtilsLabelEXT != NULL) {
        disp->QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_QueueBeginDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(queue);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.QueueBeginDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueEndDebugUtilsLabelEXT: Invalid queue "
                   "[VUID-vkQueueEndDebugUtilsLabelEXT-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->QueueEndDebugUtilsLabelEXT != NULL) {
        disp->QueueEndDebugUtilsLabelEXT(queue);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(queue);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.QueueEndDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.QueueEndDebugUtilsLabelEXT(queue);
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueInsertDebugUtilsLabelEXT: Invalid queue "
                   "[VUID-vkQueueInsertDebugUtilsLabelEXT-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->QueueInsertDebugUtilsLabelEXT != NULL) {
        disp->QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(queue);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.QueueInsertDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBeginDebugUtilsLabelEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->CmdBeginDebugUtilsLabelEXT != NULL) {
        disp->CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(commandBuffer);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.CmdBeginDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdEndDebugUtilsLabelEXT: Invalid commandBuffer "
                   "[VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->CmdEndDebugUtilsLabelEXT != NULL) {
        disp->CmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(commandBuffer);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.CmdEndDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.CmdEndDebugUtilsLabelEXT(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdInsertDebugUtilsLabelEXT: Invalid commandBuffer "
                   "[VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    if (disp->CmdInsertDebugUtilsLabelEXT != NULL) {
        disp->CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL terminator_CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch(commandBuffer);
    if (NULL == dispatch_table) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "VK_EXT_debug_utils: Invalid device handle");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    // Only call down if the device supports the function
    if (NULL != dispatch_table->extension_terminator_dispatch.CmdInsertDebugUtilsLabelEXT)
        dispatch_table->extension_terminator_dispatch.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}


// ---- VK_ANDROID_external_memory_android_hardware_buffer extension trampoline/terminators

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetAndroidHardwareBufferPropertiesANDROID(
    VkDevice                                    device,
    const struct AHardwareBuffer*               buffer,
    VkAndroidHardwareBufferPropertiesANDROID*   pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAndroidHardwareBufferPropertiesANDROID: Invalid device "
                   "[VUID-vkGetAndroidHardwareBufferPropertiesANDROID-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryAndroidHardwareBufferANDROID(
    VkDevice                                    device,
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo,
    struct AHardwareBuffer**                    pBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryAndroidHardwareBufferANDROID: Invalid device "
                   "[VUID-vkGetMemoryAndroidHardwareBufferANDROID-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

// ---- VK_AMDX_shader_enqueue extension trampoline/terminators

#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR VkResult VKAPI_CALL CreateExecutionGraphPipelinesAMDX(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkExecutionGraphPipelineCreateInfoAMDX* pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateExecutionGraphPipelinesAMDX: Invalid device "
                   "[VUID-vkCreateExecutionGraphPipelinesAMDX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateExecutionGraphPipelinesAMDX(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineScratchSizeAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    VkExecutionGraphPipelineScratchSizeAMDX*    pSizeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetExecutionGraphPipelineScratchSizeAMDX: Invalid device "
                   "[VUID-vkGetExecutionGraphPipelineScratchSizeAMDX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetExecutionGraphPipelineScratchSizeAMDX(device, executionGraph, pSizeInfo);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR VkResult VKAPI_CALL GetExecutionGraphPipelineNodeIndexAMDX(
    VkDevice                                    device,
    VkPipeline                                  executionGraph,
    const VkPipelineShaderStageNodeCreateInfoAMDX* pNodeInfo,
    uint32_t*                                   pNodeIndex) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetExecutionGraphPipelineNodeIndexAMDX: Invalid device "
                   "[VUID-vkGetExecutionGraphPipelineNodeIndexAMDX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetExecutionGraphPipelineNodeIndexAMDX(device, executionGraph, pNodeInfo, pNodeIndex);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR void VKAPI_CALL CmdInitializeGraphScratchMemoryAMDX(
    VkCommandBuffer                             commandBuffer,
    VkPipeline                                  executionGraph,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdInitializeGraphScratchMemoryAMDX: Invalid commandBuffer "
                   "[VUID-vkCmdInitializeGraphScratchMemoryAMDX-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdInitializeGraphScratchMemoryAMDX(commandBuffer, executionGraph, scratch, scratchSize);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchGraphAMDX: Invalid commandBuffer "
                   "[VUID-vkCmdDispatchGraphAMDX-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDispatchGraphAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    const VkDispatchGraphCountInfoAMDX*         pCountInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchGraphIndirectAMDX: Invalid commandBuffer "
                   "[VUID-vkCmdDispatchGraphIndirectAMDX-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDispatchGraphIndirectAMDX(commandBuffer, scratch, scratchSize, pCountInfo);
}

#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
VKAPI_ATTR void VKAPI_CALL CmdDispatchGraphIndirectCountAMDX(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             scratch,
    VkDeviceSize                                scratchSize,
    VkDeviceAddress                             countInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDispatchGraphIndirectCountAMDX: Invalid commandBuffer "
                   "[VUID-vkCmdDispatchGraphIndirectCountAMDX-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDispatchGraphIndirectCountAMDX(commandBuffer, scratch, scratchSize, countInfo);
}

#endif // VK_ENABLE_BETA_EXTENSIONS

// ---- VK_EXT_sample_locations extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetSampleLocationsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetSampleLocationsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceMultisamplePropertiesEXT: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceMultisamplePropertiesEXT-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    disp->GetPhysicalDeviceMultisamplePropertiesEXT(unwrapped_phys_dev, samples, pMultisampleProperties);
}

VKAPI_ATTR void VKAPI_CALL terminator_GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceMultisamplePropertiesEXT) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceMultisamplePropertiesEXT");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    icd_term->dispatch.GetPhysicalDeviceMultisamplePropertiesEXT(phys_dev_term->phys_dev, samples, pMultisampleProperties);
}


// ---- VK_EXT_image_drm_format_modifier extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetImageDrmFormatModifierPropertiesEXT(
    VkDevice                                    device,
    VkImage                                     image,
    VkImageDrmFormatModifierPropertiesEXT*      pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageDrmFormatModifierPropertiesEXT: Invalid device "
                   "[VUID-vkGetImageDrmFormatModifierPropertiesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
}


// ---- VK_EXT_validation_cache extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateValidationCacheEXT: Invalid device "
                   "[VUID-vkCreateValidationCacheEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
}

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyValidationCacheEXT: Invalid device "
                   "[VUID-vkDestroyValidationCacheEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyValidationCacheEXT(device, validationCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkMergeValidationCachesEXT: Invalid device "
                   "[VUID-vkMergeValidationCachesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->MergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
}

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetValidationCacheDataEXT: Invalid device "
                   "[VUID-vkGetValidationCacheDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
}


// ---- VK_NV_shading_rate_image extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBindShadingRateImageNV(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindShadingRateImageNV: Invalid commandBuffer "
                   "[VUID-vkCmdBindShadingRateImageNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportShadingRatePaletteNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkShadingRatePaletteNV*               pShadingRatePalettes) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportShadingRatePaletteNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportShadingRatePaletteNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoarseSampleOrderNV(
    VkCommandBuffer                             commandBuffer,
    VkCoarseSampleOrderTypeNV                   sampleOrderType,
    uint32_t                                    customSampleOrderCount,
    const VkCoarseSampleOrderCustomNV*          pCustomSampleOrders) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoarseSampleOrderNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoarseSampleOrderNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}


// ---- VK_NV_ray_tracing extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureNV(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoNV*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureNV*                  pAccelerationStructure) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateAccelerationStructureNV: Invalid device "
                   "[VUID-vkCreateAccelerationStructureNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyAccelerationStructureNV: Invalid device "
                   "[VUID-vkDestroyAccelerationStructureNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAccelerationStructureMemoryRequirementsNV: Invalid device "
                   "[VUID-vkGetAccelerationStructureMemoryRequirementsNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindAccelerationStructureMemoryNV: Invalid device "
                   "[VUID-vkBindAccelerationStructureMemoryNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    const VkAccelerationStructureInfoNV*        pInfo,
    VkBuffer                                    instanceData,
    VkDeviceSize                                instanceOffset,
    VkBool32                                    update,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkBuffer                                    scratch,
    VkDeviceSize                                scratchOffset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildAccelerationStructureNV: Invalid commandBuffer "
                   "[VUID-vkCmdBuildAccelerationStructureNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureNV(
    VkCommandBuffer                             commandBuffer,
    VkAccelerationStructureNV                   dst,
    VkAccelerationStructureNV                   src,
    VkCopyAccelerationStructureModeKHR          mode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyAccelerationStructureNV: Invalid commandBuffer "
                   "[VUID-vkCmdCopyAccelerationStructureNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    raygenShaderBindingTableBuffer,
    VkDeviceSize                                raygenShaderBindingOffset,
    VkBuffer                                    missShaderBindingTableBuffer,
    VkDeviceSize                                missShaderBindingOffset,
    VkDeviceSize                                missShaderBindingStride,
    VkBuffer                                    hitShaderBindingTableBuffer,
    VkDeviceSize                                hitShaderBindingOffset,
    VkDeviceSize                                hitShaderBindingStride,
    VkBuffer                                    callableShaderBindingTableBuffer,
    VkDeviceSize                                callableShaderBindingOffset,
    VkDeviceSize                                callableShaderBindingStride,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdTraceRaysNV: Invalid commandBuffer "
                   "[VUID-vkCmdTraceRaysNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoNV*     pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateRayTracingPipelinesNV: Invalid device "
                   "[VUID-vkCreateRayTracingPipelinesNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRayTracingShaderGroupHandlesKHR: Invalid device "
                   "[VUID-vkGetRayTracingShaderGroupHandlesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingShaderGroupHandlesNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRayTracingShaderGroupHandlesNV: Invalid device "
                   "[VUID-vkGetRayTracingShaderGroupHandlesNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureHandleNV(
    VkDevice                                    device,
    VkAccelerationStructureNV                   accelerationStructure,
    size_t                                      dataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAccelerationStructureHandleNV: Invalid device "
                   "[VUID-vkGetAccelerationStructureHandleNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureNV*            pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteAccelerationStructuresPropertiesNV: Invalid commandBuffer "
                   "[VUID-vkCmdWriteAccelerationStructuresPropertiesNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCompileDeferredNV: Invalid device "
                   "[VUID-vkCompileDeferredNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CompileDeferredNV(device, pipeline, shader);
}


// ---- VK_EXT_external_memory_host extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryHostPointerPropertiesEXT: Invalid device "
                   "[VUID-vkGetMemoryHostPointerPropertiesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
}


// ---- VK_AMD_buffer_marker extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteBufferMarkerAMD: Invalid commandBuffer "
                   "[VUID-vkCmdWriteBufferMarkerAMD-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteBufferMarker2AMD: Invalid commandBuffer "
                   "[VUID-vkCmdWriteBufferMarker2AMD-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}


// ---- VK_EXT_calibrated_timestamps extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCalibrateableTimeDomainsEXT-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCalibrateableTimeDomainsEXT(unwrapped_phys_dev, pTimeDomainCount, pTimeDomains);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainKHR*                            pTimeDomains) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCalibrateableTimeDomainsEXT) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCalibrateableTimeDomainsEXT");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCalibrateableTimeDomainsEXT(phys_dev_term->phys_dev, pTimeDomainCount, pTimeDomains);
}

VKAPI_ATTR VkResult VKAPI_CALL GetCalibratedTimestampsEXT(
    VkDevice                                    device,
    uint32_t                                    timestampCount,
    const VkCalibratedTimestampInfoKHR*         pTimestampInfos,
    uint64_t*                                   pTimestamps,
    uint64_t*                                   pMaxDeviation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetCalibratedTimestampsEXT: Invalid device "
                   "[VUID-vkGetCalibratedTimestampsEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}


// ---- VK_NV_mesh_shader extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    taskCount,
    uint32_t                                    firstTask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksNV: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksIndirectNV: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksIndirectNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksIndirectCountNV: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksIndirectCountNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}


// ---- VK_NV_scissor_exclusive extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorEnableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkBool32*                             pExclusiveScissorEnables) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetExclusiveScissorEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetExclusiveScissorEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetExclusiveScissorEnableNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissorEnables);
}

VKAPI_ATTR void VKAPI_CALL CmdSetExclusiveScissorNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstExclusiveScissor,
    uint32_t                                    exclusiveScissorCount,
    const VkRect2D*                             pExclusiveScissors) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetExclusiveScissorNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetExclusiveScissorNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}


// ---- VK_NV_device_diagnostic_checkpoints extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCheckpointNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCheckpointNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetQueueCheckpointDataNV: Invalid queue "
                   "[VUID-vkGetQueueCheckpointDataNV-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointData2NV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointData2NV*                        pCheckpointData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetQueueCheckpointData2NV: Invalid queue "
                   "[VUID-vkGetQueueCheckpointData2NV-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}


// ---- VK_INTEL_performance_query extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL InitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkInitializePerformanceApiINTEL: Invalid device "
                   "[VUID-vkInitializePerformanceApiINTEL-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->InitializePerformanceApiINTEL(device, pInitializeInfo);
}

VKAPI_ATTR void VKAPI_CALL UninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUninitializePerformanceApiINTEL: Invalid device "
                   "[VUID-vkUninitializePerformanceApiINTEL-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->UninitializePerformanceApiINTEL(device);
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPerformanceMarkerINTEL: Invalid commandBuffer "
                   "[VUID-vkCmdSetPerformanceMarkerINTEL-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPerformanceStreamMarkerINTEL: Invalid commandBuffer "
                   "[VUID-vkCmdSetPerformanceStreamMarkerINTEL-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPerformanceOverrideINTEL: Invalid commandBuffer "
                   "[VUID-vkCmdSetPerformanceOverrideINTEL-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL AcquirePerformanceConfigurationINTEL(
    VkDevice                                    device,
    const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo,
    VkPerformanceConfigurationINTEL*            pConfiguration) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAcquirePerformanceConfigurationINTEL: Invalid device "
                   "[VUID-vkAcquirePerformanceConfigurationINTEL-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->AcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
}

VKAPI_ATTR VkResult VKAPI_CALL ReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkReleasePerformanceConfigurationINTEL: Invalid device "
                   "[VUID-vkReleasePerformanceConfigurationINTEL-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ReleasePerformanceConfigurationINTEL(device, configuration);
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueSetPerformanceConfigurationINTEL: Invalid queue "
                   "[VUID-vkQueueSetPerformanceConfigurationINTEL-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->QueueSetPerformanceConfigurationINTEL(queue, configuration);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPerformanceParameterINTEL(
    VkDevice                                    device,
    VkPerformanceParameterTypeINTEL             parameter,
    VkPerformanceValueINTEL*                    pValue) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPerformanceParameterINTEL: Invalid device "
                   "[VUID-vkGetPerformanceParameterINTEL-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPerformanceParameterINTEL(device, parameter, pValue);
}


// ---- VK_AMD_display_native_hdr extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL SetLocalDimmingAMD(
    VkDevice                                    device,
    VkSwapchainKHR                              swapChain,
    VkBool32                                    localDimmingEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetLocalDimmingAMD: Invalid device "
                   "[VUID-vkSetLocalDimmingAMD-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->SetLocalDimmingAMD(device, swapChain, localDimmingEnable);
}


// ---- VK_EXT_buffer_device_address extension trampoline/terminators

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressEXT(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferDeviceAddressEXT: Invalid device "
                   "[VUID-vkGetBufferDeviceAddressEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferDeviceAddressEXT(device, pInfo);
}


// ---- VK_NV_cooperative_matrix extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCooperativeMatrixPropertiesNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCooperativeMatrixPropertiesNV(unwrapped_phys_dev, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCooperativeMatrixPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixPropertiesNV*            pProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixPropertiesNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCooperativeMatrixPropertiesNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixPropertiesNV(phys_dev_term->phys_dev, pPropertyCount, pProperties);
}


// ---- VK_NV_coverage_reduction_mode extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(unwrapped_phys_dev, pCombinationCount, pCombinations);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pCombinationCount,
    VkFramebufferMixedSamplesCombinationNV*     pCombinations) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(phys_dev_term->phys_dev, pCombinationCount, pCombinations);
}


// ---- VK_EXT_full_screen_exclusive extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL AcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAcquireFullScreenExclusiveModeEXT: Invalid device "
                   "[VUID-vkAcquireFullScreenExclusiveModeEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->AcquireFullScreenExclusiveModeEXT(device, swapchain);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL ReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkReleaseFullScreenExclusiveModeEXT: Invalid device "
                   "[VUID-vkReleaseFullScreenExclusiveModeEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ReleaseFullScreenExclusiveModeEXT(device, swapchain);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_EXT_line_rasterization extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    lineStippleFactor,
    uint16_t                                    lineStipplePattern) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineStippleEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetLineStippleEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}


// ---- VK_EXT_host_query_reset extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL ResetQueryPoolEXT(
    VkDevice                                    device,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery,
    uint32_t                                    queryCount) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkResetQueryPoolEXT: Invalid device "
                   "[VUID-vkResetQueryPoolEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->ResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
}


// ---- VK_EXT_extended_dynamic_state extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCullModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetCullModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCullModeEXT(commandBuffer, cullMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetFrontFaceEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetFrontFaceEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetFrontFaceEXT(commandBuffer, frontFace);
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPrimitiveTopologyEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetPrimitiveTopologyEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportWithCountEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportWithCountEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCountEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetScissorWithCountEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetScissorWithCountEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2EXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindVertexBuffers2EXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindVertexBuffers2EXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthTestEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthTestEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthWriteEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthWriteEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthCompareOpEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthCompareOpEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBoundsTestEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthBoundsTestEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilTestEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetStencilTestEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetStencilOpEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetStencilOpEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}


// ---- VK_EXT_host_image_copy extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToImageEXT(
    VkDevice                                    device,
    const VkCopyMemoryToImageInfo*              pCopyMemoryToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMemoryToImageEXT: Invalid device "
                   "[VUID-vkCopyMemoryToImageEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMemoryToImageEXT(device, pCopyMemoryToImageInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyImageToMemoryEXT(
    VkDevice                                    device,
    const VkCopyImageToMemoryInfo*              pCopyImageToMemoryInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyImageToMemoryEXT: Invalid device "
                   "[VUID-vkCopyImageToMemoryEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyImageToMemoryEXT(device, pCopyImageToMemoryInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyImageToImageEXT(
    VkDevice                                    device,
    const VkCopyImageToImageInfo*               pCopyImageToImageInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyImageToImageEXT: Invalid device "
                   "[VUID-vkCopyImageToImageEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyImageToImageEXT(device, pCopyImageToImageInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL TransitionImageLayoutEXT(
    VkDevice                                    device,
    uint32_t                                    transitionCount,
    const VkHostImageLayoutTransitionInfo*      pTransitions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkTransitionImageLayoutEXT: Invalid device "
                   "[VUID-vkTransitionImageLayoutEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->TransitionImageLayoutEXT(device, transitionCount, pTransitions);
}

VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2*                  pSubresource,
    VkSubresourceLayout2*                       pLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageSubresourceLayout2EXT: Invalid device "
                   "[VUID-vkGetImageSubresourceLayout2EXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
}


// ---- VK_EXT_swapchain_maintenance1 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL ReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkReleaseSwapchainImagesEXT: Invalid device "
                   "[VUID-vkReleaseSwapchainImagesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ReleaseSwapchainImagesEXT(device, pReleaseInfo);
}


// ---- VK_NV_device_generated_commands extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetGeneratedCommandsMemoryRequirementsNV: Invalid device "
                   "[VUID-vkGetGeneratedCommandsMemoryRequirementsNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPreprocessGeneratedCommandsNV: Invalid commandBuffer "
                   "[VUID-vkCmdPreprocessGeneratedCommandsNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdExecuteGeneratedCommandsNV: Invalid commandBuffer "
                   "[VUID-vkCmdExecuteGeneratedCommandsNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBindPipelineShaderGroupNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline,
    uint32_t                                    groupIndex) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindPipelineShaderGroupNV: Invalid commandBuffer "
                   "[VUID-vkCmdBindPipelineShaderGroupNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutNV(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateIndirectCommandsLayoutNV: Invalid device "
                   "[VUID-vkCreateIndirectCommandsLayoutNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutNV(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyIndirectCommandsLayoutNV: Invalid device "
                   "[VUID-vkDestroyIndirectCommandsLayoutNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
}


// ---- VK_EXT_depth_bias_control extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkDepthBiasInfoEXT*                   pDepthBiasInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBias2EXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthBias2EXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthBias2EXT(commandBuffer, pDepthBiasInfo);
}


// ---- VK_EXT_acquire_drm_display extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL AcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAcquireDrmDisplayEXT: Invalid physicalDevice "
                   "[VUID-vkAcquireDrmDisplayEXT-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->AcquireDrmDisplayEXT(unwrapped_phys_dev, drmFd, display);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_AcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.AcquireDrmDisplayEXT) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support AcquireDrmDisplayEXT");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return icd_term->dispatch.AcquireDrmDisplayEXT(phys_dev_term->phys_dev, drmFd, display);
}

VKAPI_ATTR VkResult VKAPI_CALL GetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDrmDisplayEXT: Invalid physicalDevice "
                   "[VUID-vkGetDrmDisplayEXT-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetDrmDisplayEXT(unwrapped_phys_dev, drmFd, connectorId, display);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetDrmDisplayEXT) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetDrmDisplayEXT");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return icd_term->dispatch.GetDrmDisplayEXT(phys_dev_term->phys_dev, drmFd, connectorId, display);
}


// ---- VK_EXT_private_data extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreatePrivateDataSlotEXT: Invalid device "
                   "[VUID-vkCreatePrivateDataSlotEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
}

VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyPrivateDataSlotEXT: Invalid device "
                   "[VUID-vkDestroyPrivateDataSlotEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL SetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetPrivateDataEXT: Invalid device "
                   "[VUID-vkSetPrivateDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
}

VKAPI_ATTR void VKAPI_CALL GetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPrivateDataEXT: Invalid device "
                   "[VUID-vkGetPrivateDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
}


// ---- VK_NV_cuda_kernel_launch extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateCudaModuleNV(
    VkDevice                                    device,
    const VkCudaModuleCreateInfoNV*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaModuleNV*                             pModule) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateCudaModuleNV: Invalid device "
                   "[VUID-vkCreateCudaModuleNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateCudaModuleNV(device, pCreateInfo, pAllocator, pModule);
}

VKAPI_ATTR VkResult VKAPI_CALL GetCudaModuleCacheNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    size_t*                                     pCacheSize,
    void*                                       pCacheData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetCudaModuleCacheNV: Invalid device "
                   "[VUID-vkGetCudaModuleCacheNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetCudaModuleCacheNV(device, module, pCacheSize, pCacheData);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCudaFunctionNV(
    VkDevice                                    device,
    const VkCudaFunctionCreateInfoNV*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCudaFunctionNV*                           pFunction) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateCudaFunctionNV: Invalid device "
                   "[VUID-vkCreateCudaFunctionNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateCudaFunctionNV(device, pCreateInfo, pAllocator, pFunction);
}

VKAPI_ATTR void VKAPI_CALL DestroyCudaModuleNV(
    VkDevice                                    device,
    VkCudaModuleNV                              module,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyCudaModuleNV: Invalid device "
                   "[VUID-vkDestroyCudaModuleNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyCudaModuleNV(device, module, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyCudaFunctionNV(
    VkDevice                                    device,
    VkCudaFunctionNV                            function,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyCudaFunctionNV: Invalid device "
                   "[VUID-vkDestroyCudaFunctionNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyCudaFunctionNV(device, function, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL CmdCudaLaunchKernelNV(
    VkCommandBuffer                             commandBuffer,
    const VkCudaLaunchInfoNV*                   pLaunchInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCudaLaunchKernelNV: Invalid commandBuffer "
                   "[VUID-vkCmdCudaLaunchKernelNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCudaLaunchKernelNV(commandBuffer, pLaunchInfo);
}


// ---- VK_EXT_metal_objects extension trampoline/terminators

#if defined(VK_USE_PLATFORM_METAL_EXT)
VKAPI_ATTR void VKAPI_CALL ExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkExportMetalObjectsEXT: Invalid device "
                   "[VUID-vkExportMetalObjectsEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->ExportMetalObjectsEXT(device, pMetalObjectsInfo);
}

#endif // VK_USE_PLATFORM_METAL_EXT

// ---- VK_EXT_descriptor_buffer extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetLayoutSizeEXT: Invalid device "
                   "[VUID-vkGetDescriptorSetLayoutSizeEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetLayoutBindingOffsetEXT: Invalid device "
                   "[VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorEXT: Invalid device "
                   "[VUID-vkGetDescriptorEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorBuffersEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindDescriptorBuffersEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDescriptorBufferOffsetsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDescriptorBufferOffsetsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindDescriptorBufferEmbeddedSamplersEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
}

VKAPI_ATTR VkResult VKAPI_CALL GetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferOpaqueCaptureDescriptorDataEXT: Invalid device "
                   "[VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageOpaqueCaptureDescriptorDataEXT: Invalid device "
                   "[VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetImageViewOpaqueCaptureDescriptorDataEXT: Invalid device "
                   "[VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSamplerOpaqueCaptureDescriptorDataEXT: Invalid device "
                   "[VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT: Invalid device "
                   "[VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
}


// ---- VK_NV_fragment_shading_rate_enums extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetFragmentShadingRateEnumNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetFragmentShadingRateEnumNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}


// ---- VK_EXT_device_fault extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceFaultInfoEXT: Invalid device "
                   "[VUID-vkGetDeviceFaultInfoEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);
}


// ---- VK_NV_acquire_winrt_display extension trampoline/terminators

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAcquireWinrtDisplayNV: Invalid physicalDevice "
                   "[VUID-vkAcquireWinrtDisplayNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->AcquireWinrtDisplayNV(unwrapped_phys_dev, display);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.AcquireWinrtDisplayNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support AcquireWinrtDisplayNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.AcquireWinrtDisplayNV(phys_dev_term->phys_dev, display);
}

#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL GetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetWinrtDisplayNV: Invalid physicalDevice "
                   "[VUID-vkGetWinrtDisplayNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetWinrtDisplayNV(unwrapped_phys_dev, deviceRelativeId, pDisplay);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    deviceRelativeId,
    VkDisplayKHR*                               pDisplay) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetWinrtDisplayNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetWinrtDisplayNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetWinrtDisplayNV(phys_dev_term->phys_dev, deviceRelativeId, pDisplay);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

// ---- VK_EXT_vertex_input_dynamic_state extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetVertexInputEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetVertexInputEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}


// ---- VK_FUCHSIA_external_memory extension trampoline/terminators

#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryZirconHandleFUCHSIA: Invalid device "
                   "[VUID-vkGetMemoryZirconHandleFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryZirconHandlePropertiesFUCHSIA: Invalid device "
                   "[VUID-vkGetMemoryZirconHandlePropertiesFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties);
}

#endif // VK_USE_PLATFORM_FUCHSIA

// ---- VK_FUCHSIA_external_semaphore extension trampoline/terminators

#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkImportSemaphoreZirconHandleFUCHSIA: Invalid device "
                   "[VUID-vkImportSemaphoreZirconHandleFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetSemaphoreZirconHandleFUCHSIA: Invalid device "
                   "[VUID-vkGetSemaphoreZirconHandleFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
}

#endif // VK_USE_PLATFORM_FUCHSIA

// ---- VK_FUCHSIA_buffer_collection extension trampoline/terminators

#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL CreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateBufferCollectionFUCHSIA: Invalid device "
                   "[VUID-vkCreateBufferCollectionFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetBufferCollectionImageConstraintsFUCHSIA: Invalid device "
                   "[VUID-vkSetBufferCollectionImageConstraintsFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetBufferCollectionBufferConstraintsFUCHSIA: Invalid device "
                   "[VUID-vkSetBufferCollectionBufferConstraintsFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR void VKAPI_CALL DestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyBufferCollectionFUCHSIA: Invalid device "
                   "[VUID-vkDestroyBufferCollectionFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
}

#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
VKAPI_ATTR VkResult VKAPI_CALL GetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetBufferCollectionPropertiesFUCHSIA: Invalid device "
                   "[VUID-vkGetBufferCollectionPropertiesFUCHSIA-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
}

#endif // VK_USE_PLATFORM_FUCHSIA

// ---- VK_HUAWEI_subpass_shading extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI: Invalid device "
                   "[VUID-vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
}

VKAPI_ATTR void VKAPI_CALL CmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSubpassShadingHUAWEI: Invalid commandBuffer "
                   "[VUID-vkCmdSubpassShadingHUAWEI-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSubpassShadingHUAWEI(commandBuffer);
}


// ---- VK_HUAWEI_invocation_mask extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindInvocationMaskHUAWEI: Invalid commandBuffer "
                   "[VUID-vkCmdBindInvocationMaskHUAWEI-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}


// ---- VK_NV_external_memory_rdma extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryRemoteAddressNV: Invalid device "
                   "[VUID-vkGetMemoryRemoteAddressNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
}


// ---- VK_EXT_pipeline_properties extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelinePropertiesEXT: Invalid device "
                   "[VUID-vkGetPipelinePropertiesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
}


// ---- VK_EXT_extended_dynamic_state2 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPatchControlPointsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetPatchControlPointsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRasterizerDiscardEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetRasterizerDiscardEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthBiasEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthBiasEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLogicOpEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetLogicOpEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLogicOpEXT(commandBuffer, logicOp);
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPrimitiveRestartEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetPrimitiveRestartEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}


// ---- VK_EXT_color_write_enable extension trampoline/terminators

VKAPI_ATTR void                                    VKAPI_CALL CmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetColorWriteEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetColorWriteEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}


// ---- VK_EXT_multi_draw extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMultiEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMultiEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMultiIndexedEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMultiIndexedEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}


// ---- VK_EXT_opacity_micromap extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateMicromapEXT: Invalid device "
                   "[VUID-vkCreateMicromapEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
}

VKAPI_ATTR void VKAPI_CALL DestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyMicromapEXT: Invalid device "
                   "[VUID-vkDestroyMicromapEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyMicromapEXT(device, micromap, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildMicromapsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBuildMicromapsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL BuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBuildMicromapsEXT: Invalid device "
                   "[VUID-vkBuildMicromapsEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMicromapEXT: Invalid device "
                   "[VUID-vkCopyMicromapEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMicromapEXT(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMicromapToMemoryEXT: Invalid device "
                   "[VUID-vkCopyMicromapToMemoryEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMemoryToMicromapEXT: Invalid device "
                   "[VUID-vkCopyMemoryToMicromapEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL WriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWriteMicromapsPropertiesEXT: Invalid device "
                   "[VUID-vkWriteMicromapsPropertiesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->WriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMicromapEXT: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMicromapEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMicromapEXT(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMicromapToMemoryEXT: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMicromapToMemoryEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMemoryToMicromapEXT: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMemoryToMicromapEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteMicromapsPropertiesEXT: Invalid commandBuffer "
                   "[VUID-vkCmdWriteMicromapsPropertiesEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceMicromapCompatibilityEXT: Invalid device "
                   "[VUID-vkGetDeviceMicromapCompatibilityEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
}

VKAPI_ATTR void VKAPI_CALL GetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMicromapBuildSizesEXT: Invalid device "
                   "[VUID-vkGetMicromapBuildSizesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
}


// ---- VK_HUAWEI_cluster_culling_shader extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawClusterHUAWEI: Invalid commandBuffer "
                   "[VUID-vkCmdDrawClusterHUAWEI-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawClusterIndirectHUAWEI: Invalid commandBuffer "
                   "[VUID-vkCmdDrawClusterIndirectHUAWEI-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
}


// ---- VK_EXT_pageable_device_local_memory extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL SetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetDeviceMemoryPriorityEXT: Invalid device "
                   "[VUID-vkSetDeviceMemoryPriorityEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->SetDeviceMemoryPriorityEXT(device, memory, priority);
}


// ---- VK_VALVE_descriptor_set_host_mapping extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetLayoutHostMappingInfoVALVE: Invalid device "
                   "[VUID-vkGetDescriptorSetLayoutHostMappingInfoVALVE-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDescriptorSetHostMappingVALVE: Invalid device "
                   "[VUID-vkGetDescriptorSetHostMappingVALVE-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
}


// ---- VK_NV_copy_memory_indirect extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMemoryIndirectNV: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMemoryIndirectNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMemoryToImageIndirectNV: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMemoryToImageIndirectNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
}


// ---- VK_NV_memory_decompression extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDecompressMemoryNV: Invalid commandBuffer "
                   "[VUID-vkCmdDecompressMemoryNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDecompressMemoryIndirectCountNV: Invalid commandBuffer "
                   "[VUID-vkCmdDecompressMemoryIndirectCountNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
}


// ---- VK_NV_device_generated_commands_compute extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetPipelineIndirectMemoryRequirementsNV(
    VkDevice                                    device,
    const VkComputePipelineCreateInfo*          pCreateInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineIndirectMemoryRequirementsNV: Invalid device "
                   "[VUID-vkGetPipelineIndirectMemoryRequirementsNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetPipelineIndirectMemoryRequirementsNV(device, pCreateInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL CmdUpdatePipelineIndirectBufferNV(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipeline                                  pipeline) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdUpdatePipelineIndirectBufferNV: Invalid commandBuffer "
                   "[VUID-vkCmdUpdatePipelineIndirectBufferNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdUpdatePipelineIndirectBufferNV(commandBuffer, pipelineBindPoint, pipeline);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetPipelineIndirectDeviceAddressNV(
    VkDevice                                    device,
    const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPipelineIndirectDeviceAddressNV: Invalid device "
                   "[VUID-vkGetPipelineIndirectDeviceAddressNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetPipelineIndirectDeviceAddressNV(device, pInfo);
}


// ---- VK_EXT_extended_dynamic_state3 extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthClampEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthClampEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetPolygonModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetPolygonModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetPolygonModeEXT(commandBuffer, polygonMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRasterizationSamplesEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetRasterizationSamplesEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
}

VKAPI_ATTR void VKAPI_CALL CmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetSampleMaskEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetSampleMaskEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
}

VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetAlphaToCoverageEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetAlphaToCoverageEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetAlphaToOneEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetAlphaToOneEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLogicOpEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetLogicOpEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetColorBlendEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetColorBlendEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetColorBlendEquationEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetColorBlendEquationEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetColorWriteMaskEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetColorWriteMaskEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

VKAPI_ATTR void VKAPI_CALL CmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetTessellationDomainOriginEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetTessellationDomainOriginEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRasterizationStreamEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetRasterizationStreamEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
}

VKAPI_ATTR void VKAPI_CALL CmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetConservativeRasterizationModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetConservativeRasterizationModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetExtraPrimitiveOverestimationSizeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthClipEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthClipEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetSampleLocationsEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetSampleLocationsEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetColorBlendAdvancedEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetColorBlendAdvancedEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}

VKAPI_ATTR void VKAPI_CALL CmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetProvokingVertexModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetProvokingVertexModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineRasterizationModeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetLineRasterizationModeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetLineStippleEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetLineStippleEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthClipNegativeOneToOneEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthClipNegativeOneToOneEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportWScalingEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportWScalingEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetViewportSwizzleNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetViewportSwizzleNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageToColorEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageToColorEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageToColorLocationNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageToColorLocationNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageModulationModeNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageModulationModeNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageModulationTableEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageModulationTableEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageModulationTableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageModulationTableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetShadingRateImageEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetShadingRateImageEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRepresentativeFragmentTestEnableNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetRepresentativeFragmentTestEnableNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetCoverageReductionModeNV: Invalid commandBuffer "
                   "[VUID-vkCmdSetCoverageReductionModeNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
}


// ---- VK_EXT_shader_module_identifier extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetShaderModuleIdentifierEXT: Invalid device "
                   "[VUID-vkGetShaderModuleIdentifierEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
}

VKAPI_ATTR void VKAPI_CALL GetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetShaderModuleCreateInfoIdentifierEXT: Invalid device "
                   "[VUID-vkGetShaderModuleCreateInfoIdentifierEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
}


// ---- VK_NV_optical_flow extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceOpticalFlowImageFormatsNV: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceOpticalFlowImageFormatsNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceOpticalFlowImageFormatsNV(unwrapped_phys_dev, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceOpticalFlowImageFormatsNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceOpticalFlowImageFormatsNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceOpticalFlowImageFormatsNV(phys_dev_term->phys_dev, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateOpticalFlowSessionNV: Invalid device "
                   "[VUID-vkCreateOpticalFlowSessionNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
}

VKAPI_ATTR void VKAPI_CALL DestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyOpticalFlowSessionNV: Invalid device "
                   "[VUID-vkDestroyOpticalFlowSessionNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyOpticalFlowSessionNV(device, session, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL BindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBindOpticalFlowSessionImageNV: Invalid device "
                   "[VUID-vkBindOpticalFlowSessionImageNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
}

VKAPI_ATTR void VKAPI_CALL CmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdOpticalFlowExecuteNV: Invalid commandBuffer "
                   "[VUID-vkCmdOpticalFlowExecuteNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
}


// ---- VK_AMD_anti_lag extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL AntiLagUpdateAMD(
    VkDevice                                    device,
    const VkAntiLagDataAMD*                     pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkAntiLagUpdateAMD: Invalid device "
                   "[VUID-vkAntiLagUpdateAMD-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->AntiLagUpdateAMD(device, pData);
}


// ---- VK_EXT_shader_object extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(
    VkDevice                                    device,
    uint32_t                                    createInfoCount,
    const VkShaderCreateInfoEXT*                pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderEXT*                                pShaders) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateShadersEXT: Invalid device "
                   "[VUID-vkCreateShadersEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyShaderEXT: Invalid device "
                   "[VUID-vkDestroyShaderEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyShaderEXT(device, shader, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL GetShaderBinaryDataEXT(
    VkDevice                                    device,
    VkShaderEXT                                 shader,
    size_t*                                     pDataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetShaderBinaryDataEXT: Invalid device "
                   "[VUID-vkGetShaderBinaryDataEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetShaderBinaryDataEXT(device, shader, pDataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL CmdBindShadersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    stageCount,
    const VkShaderStageFlagBits*                pStages,
    const VkShaderEXT*                          pShaders) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBindShadersEXT: Invalid commandBuffer "
                   "[VUID-vkCmdBindShadersEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBindShadersEXT(commandBuffer, stageCount, pStages, pShaders);
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampRangeEXT(
    VkCommandBuffer                             commandBuffer,
    VkDepthClampModeEXT                         depthClampMode,
    const VkDepthClampRangeEXT*                 pDepthClampRange) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetDepthClampRangeEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetDepthClampRangeEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetDepthClampRangeEXT(commandBuffer, depthClampMode, pDepthClampRange);
}


// ---- VK_QCOM_tile_properties extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetFramebufferTilePropertiesQCOM: Invalid device "
                   "[VUID-vkGetFramebufferTilePropertiesQCOM-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL GetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDynamicRenderingTilePropertiesQCOM: Invalid device "
                   "[VUID-vkGetDynamicRenderingTilePropertiesQCOM-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
}


// ---- VK_NV_cooperative_vector extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeVectorPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeVectorPropertiesNV*            pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCooperativeVectorPropertiesNV: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCooperativeVectorPropertiesNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCooperativeVectorPropertiesNV(unwrapped_phys_dev, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCooperativeVectorPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeVectorPropertiesNV*            pProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCooperativeVectorPropertiesNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCooperativeVectorPropertiesNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCooperativeVectorPropertiesNV(phys_dev_term->phys_dev, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL ConvertCooperativeVectorMatrixNV(
    VkDevice                                    device,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkConvertCooperativeVectorMatrixNV: Invalid device "
                   "[VUID-vkConvertCooperativeVectorMatrixNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->ConvertCooperativeVectorMatrixNV(device, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdConvertCooperativeVectorMatrixNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkConvertCooperativeVectorMatrixInfoNV* pInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdConvertCooperativeVectorMatrixNV: Invalid commandBuffer "
                   "[VUID-vkCmdConvertCooperativeVectorMatrixNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdConvertCooperativeVectorMatrixNV(commandBuffer, infoCount, pInfos);
}


// ---- VK_NV_low_latency2 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL SetLatencySleepModeNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepModeInfoNV*             pSleepModeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetLatencySleepModeNV: Invalid device "
                   "[VUID-vkSetLatencySleepModeNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL LatencySleepNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkLatencySleepInfoNV*                 pSleepInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkLatencySleepNV: Invalid device "
                   "[VUID-vkLatencySleepNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->LatencySleepNV(device, swapchain, pSleepInfo);
}

VKAPI_ATTR void VKAPI_CALL SetLatencyMarkerNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkSetLatencyMarkerInfoNV*             pLatencyMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkSetLatencyMarkerNV: Invalid device "
                   "[VUID-vkSetLatencyMarkerNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL GetLatencyTimingsNV(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkGetLatencyMarkerInfoNV*                   pLatencyMarkerInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetLatencyTimingsNV: Invalid device "
                   "[VUID-vkGetLatencyTimingsNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL QueueNotifyOutOfBandNV(
    VkQueue                                     queue,
    const VkOutOfBandQueueTypeInfoNV*           pQueueTypeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(queue);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkQueueNotifyOutOfBandNV: Invalid queue "
                   "[VUID-vkQueueNotifyOutOfBandNV-queue-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
}


// ---- VK_EXT_attachment_feedback_loop_dynamic_state extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdSetAttachmentFeedbackLoopEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkImageAspectFlags                          aspectMask) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetAttachmentFeedbackLoopEnableEXT: Invalid commandBuffer "
                   "[VUID-vkCmdSetAttachmentFeedbackLoopEnableEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetAttachmentFeedbackLoopEnableEXT(commandBuffer, aspectMask);
}


// ---- VK_QNX_external_memory_screen_buffer extension trampoline/terminators

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
VKAPI_ATTR VkResult VKAPI_CALL GetScreenBufferPropertiesQNX(
    VkDevice                                    device,
    const struct _screen_buffer*                buffer,
    VkScreenBufferPropertiesQNX*                pProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetScreenBufferPropertiesQNX: Invalid device "
                   "[VUID-vkGetScreenBufferPropertiesQNX-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetScreenBufferPropertiesQNX(device, buffer, pProperties);
}

#endif // VK_USE_PLATFORM_SCREEN_QNX

// ---- VK_NV_cluster_acceleration_structure extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetClusterAccelerationStructureBuildSizesNV(
    VkDevice                                    device,
    const VkClusterAccelerationStructureInputInfoNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetClusterAccelerationStructureBuildSizesNV: Invalid device "
                   "[VUID-vkGetClusterAccelerationStructureBuildSizesNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetClusterAccelerationStructureBuildSizesNV(device, pInfo, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildClusterAccelerationStructureIndirectNV(
    VkCommandBuffer                             commandBuffer,
    const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildClusterAccelerationStructureIndirectNV: Invalid commandBuffer "
                   "[VUID-vkCmdBuildClusterAccelerationStructureIndirectNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildClusterAccelerationStructureIndirectNV(commandBuffer, pCommandInfos);
}


// ---- VK_NV_partitioned_acceleration_structure extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice                                    device,
    const VkPartitionedAccelerationStructureInstancesInputNV* pInfo,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPartitionedAccelerationStructuresBuildSizesNV: Invalid device "
                   "[VUID-vkGetPartitionedAccelerationStructuresBuildSizesNV-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetPartitionedAccelerationStructuresBuildSizesNV(device, pInfo, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildPartitionedAccelerationStructuresNV(
    VkCommandBuffer                             commandBuffer,
    const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildPartitionedAccelerationStructuresNV: Invalid commandBuffer "
                   "[VUID-vkCmdBuildPartitionedAccelerationStructuresNV-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildPartitionedAccelerationStructuresNV(commandBuffer, pBuildInfo);
}


// ---- VK_EXT_device_generated_commands extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsEXT(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetGeneratedCommandsMemoryRequirementsEXT: Invalid device "
                   "[VUID-vkGetGeneratedCommandsMemoryRequirementsEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetGeneratedCommandsMemoryRequirementsEXT(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo,
    VkCommandBuffer                             stateCommandBuffer) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdPreprocessGeneratedCommandsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdPreprocessGeneratedCommandsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdPreprocessGeneratedCommandsEXT(commandBuffer, pGeneratedCommandsInfo, stateCommandBuffer);
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoEXT*           pGeneratedCommandsInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdExecuteGeneratedCommandsEXT: Invalid commandBuffer "
                   "[VUID-vkCmdExecuteGeneratedCommandsEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdExecuteGeneratedCommandsEXT(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutEXT*                pIndirectCommandsLayout) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateIndirectCommandsLayoutEXT: Invalid device "
                   "[VUID-vkCreateIndirectCommandsLayoutEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateIndirectCommandsLayoutEXT(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}

VKAPI_ATTR void VKAPI_CALL DestroyIndirectCommandsLayoutEXT(
    VkDevice                                    device,
    VkIndirectCommandsLayoutEXT                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyIndirectCommandsLayoutEXT: Invalid device "
                   "[VUID-vkDestroyIndirectCommandsLayoutEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyIndirectCommandsLayoutEXT(device, indirectCommandsLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateIndirectExecutionSetEXT(
    VkDevice                                    device,
    const VkIndirectExecutionSetCreateInfoEXT*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectExecutionSetEXT*                  pIndirectExecutionSet) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateIndirectExecutionSetEXT: Invalid device "
                   "[VUID-vkCreateIndirectExecutionSetEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateIndirectExecutionSetEXT(device, pCreateInfo, pAllocator, pIndirectExecutionSet);
}

VKAPI_ATTR void VKAPI_CALL DestroyIndirectExecutionSetEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyIndirectExecutionSetEXT: Invalid device "
                   "[VUID-vkDestroyIndirectExecutionSetEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyIndirectExecutionSetEXT(device, indirectExecutionSet, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetPipelineEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateIndirectExecutionSetPipelineEXT: Invalid device "
                   "[VUID-vkUpdateIndirectExecutionSetPipelineEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->UpdateIndirectExecutionSetPipelineEXT(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}

VKAPI_ATTR void VKAPI_CALL UpdateIndirectExecutionSetShaderEXT(
    VkDevice                                    device,
    VkIndirectExecutionSetEXT                   indirectExecutionSet,
    uint32_t                                    executionSetWriteCount,
    const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkUpdateIndirectExecutionSetShaderEXT: Invalid device "
                   "[VUID-vkUpdateIndirectExecutionSetShaderEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->UpdateIndirectExecutionSetShaderEXT(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}


// ---- VK_NV_cooperative_matrix2 extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    const VkLayerInstanceDispatchTable *disp;
    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device(physicalDevice);
    if (VK_NULL_HANDLE == unwrapped_phys_dev) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV: Invalid physicalDevice "
                   "[VUID-vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV-physicalDevice-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp = loader_get_instance_layer_dispatch(physicalDevice);
    return disp->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(unwrapped_phys_dev, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL terminator_GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)physicalDevice;
    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;
    if (NULL == icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV) {
        loader_log(icd_term->this_instance, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT, 0,
                   "ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return icd_term->dispatch.GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(phys_dev_term->phys_dev, pPropertyCount, pProperties);
}


// ---- VK_EXT_external_memory_metal extension trampoline/terminators

#if defined(VK_USE_PLATFORM_METAL_EXT)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandleEXT(
    VkDevice                                    device,
    const VkMemoryGetMetalHandleInfoEXT*        pGetMetalHandleInfo,
    void**                                      pHandle) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryMetalHandleEXT: Invalid device "
                   "[VUID-vkGetMemoryMetalHandleEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryMetalHandleEXT(device, pGetMetalHandleInfo, pHandle);
}

#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_METAL_EXT)
VKAPI_ATTR VkResult VKAPI_CALL GetMemoryMetalHandlePropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    const void*                                 pHandle,
    VkMemoryMetalHandlePropertiesEXT*           pMemoryMetalHandleProperties) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetMemoryMetalHandlePropertiesEXT: Invalid device "
                   "[VUID-vkGetMemoryMetalHandlePropertiesEXT-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetMemoryMetalHandlePropertiesEXT(device, handleType, pHandle, pMemoryMetalHandleProperties);
}

#endif // VK_USE_PLATFORM_METAL_EXT

// ---- VK_KHR_acceleration_structure extension trampoline/terminators

VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateAccelerationStructureKHR: Invalid device "
                   "[VUID-vkCreateAccelerationStructureKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

VKAPI_ATTR void VKAPI_CALL DestroyAccelerationStructureKHR(
    VkDevice                                    device,
    VkAccelerationStructureKHR                  accelerationStructure,
    const VkAllocationCallbacks*                pAllocator) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkDestroyAccelerationStructureKHR: Invalid device "
                   "[VUID-vkDestroyAccelerationStructureKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->DestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildAccelerationStructuresKHR: Invalid commandBuffer "
                   "[VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR void VKAPI_CALL CmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkDeviceAddress*                      pIndirectDeviceAddresses,
    const uint32_t*                             pIndirectStrides,
    const uint32_t* const*                      ppMaxPrimitiveCounts) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdBuildAccelerationStructuresIndirectKHR: Invalid commandBuffer "
                   "[VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

VKAPI_ATTR VkResult VKAPI_CALL BuildAccelerationStructuresKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkBuildAccelerationStructuresKHR: Invalid device "
                   "[VUID-vkBuildAccelerationStructuresKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->BuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyAccelerationStructureKHR: Invalid device "
                   "[VUID-vkCopyAccelerationStructureKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyAccelerationStructureKHR(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyAccelerationStructureToMemoryKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyAccelerationStructureToMemoryKHR: Invalid device "
                   "[VUID-vkCopyAccelerationStructureToMemoryKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToAccelerationStructureKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCopyMemoryToAccelerationStructureKHR: Invalid device "
                   "[VUID-vkCopyMemoryToAccelerationStructureKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL WriteAccelerationStructuresPropertiesKHR(
    VkDevice                                    device,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkWriteAccelerationStructuresPropertiesKHR: Invalid device "
                   "[VUID-vkWriteAccelerationStructuresPropertiesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->WriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyAccelerationStructureKHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyAccelerationStructureKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyAccelerationStructureToMemoryKHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyAccelerationStructureToMemoryKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdCopyMemoryToAccelerationStructureKHR: Invalid commandBuffer "
                   "[VUID-vkCmdCopyMemoryToAccelerationStructureKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAccelerationStructureDeviceAddressKHR: Invalid device "
                   "[VUID-vkGetAccelerationStructureDeviceAddressKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetAccelerationStructureDeviceAddressKHR(device, pInfo);
}

VKAPI_ATTR void VKAPI_CALL CmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    accelerationStructureCount,
    const VkAccelerationStructureKHR*           pAccelerationStructures,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdWriteAccelerationStructuresPropertiesKHR: Invalid commandBuffer "
                   "[VUID-vkCmdWriteAccelerationStructuresPropertiesKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetDeviceAccelerationStructureCompatibilityKHR: Invalid device "
                   "[VUID-vkGetDeviceAccelerationStructureCompatibilityKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureBuildSizesKHR(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t*                             pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR*   pSizeInfo) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetAccelerationStructureBuildSizesKHR: Invalid device "
                   "[VUID-vkGetAccelerationStructureBuildSizesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->GetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}


// ---- VK_KHR_ray_tracing_pipeline extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    uint32_t                                    width,
    uint32_t                                    height,
    uint32_t                                    depth) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdTraceRaysKHR: Invalid commandBuffer "
                   "[VUID-vkCmdTraceRaysKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkRayTracingPipelineCreateInfoKHR*    pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCreateRayTracingPipelinesKHR: Invalid device "
                   "[VUID-vkCreateRayTracingPipelinesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->CreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR VkResult VKAPI_CALL GetRayTracingCaptureReplayShaderGroupHandlesKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    firstGroup,
    uint32_t                                    groupCount,
    size_t                                      dataSize,
    void*                                       pData) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR: Invalid device "
                   "[VUID-vkGetRayTracingCaptureReplayShaderGroupHandlesKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirectKHR(
    VkCommandBuffer                             commandBuffer,
    const VkStridedDeviceAddressRegionKHR*      pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pMissShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR*      pCallableShaderBindingTable,
    VkDeviceAddress                             indirectDeviceAddress) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdTraceRaysIndirectKHR: Invalid commandBuffer "
                   "[VUID-vkCmdTraceRaysIndirectKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

VKAPI_ATTR VkDeviceSize VKAPI_CALL GetRayTracingShaderGroupStackSizeKHR(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    group,
    VkShaderGroupShaderKHR                      groupShader) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(device);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkGetRayTracingShaderGroupStackSizeKHR: Invalid device "
                   "[VUID-vkGetRayTracingShaderGroupStackSizeKHR-device-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    return disp->GetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
}

VKAPI_ATTR void VKAPI_CALL CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdSetRayTracingPipelineStackSizeKHR: Invalid commandBuffer "
                   "[VUID-vkCmdSetRayTracingPipelineStackSizeKHR-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}


// ---- VK_EXT_mesh_shader extension trampoline/terminators

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksIndirectEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksIndirectEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    const VkLayerDispatchTable *disp = loader_get_dispatch(commandBuffer);
    if (NULL == disp) {
        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,
                   "vkCmdDrawMeshTasksIndirectCountEXT: Invalid commandBuffer "
                   "[VUID-vkCmdDrawMeshTasksIndirectCountEXT-commandBuffer-parameter]");
        abort(); /* Intentionally fail so user can correct issue. */
    }
    disp->CmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

// GPA helpers for extensions
bool extension_instance_gpa(struct loader_instance *ptr_instance, const char *name, void **addr) {
    *addr = NULL;


    // ---- VK_KHR_video_queue extension commands
    if (!strcmp("vkGetPhysicalDeviceVideoCapabilitiesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceVideoCapabilitiesKHR;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceVideoFormatPropertiesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceVideoFormatPropertiesKHR;
        return true;
    }
    if (!strcmp("vkCreateVideoSessionKHR", name)) {
        *addr = (void *)CreateVideoSessionKHR;
        return true;
    }
    if (!strcmp("vkDestroyVideoSessionKHR", name)) {
        *addr = (void *)DestroyVideoSessionKHR;
        return true;
    }
    if (!strcmp("vkGetVideoSessionMemoryRequirementsKHR", name)) {
        *addr = (void *)GetVideoSessionMemoryRequirementsKHR;
        return true;
    }
    if (!strcmp("vkBindVideoSessionMemoryKHR", name)) {
        *addr = (void *)BindVideoSessionMemoryKHR;
        return true;
    }
    if (!strcmp("vkCreateVideoSessionParametersKHR", name)) {
        *addr = (void *)CreateVideoSessionParametersKHR;
        return true;
    }
    if (!strcmp("vkUpdateVideoSessionParametersKHR", name)) {
        *addr = (void *)UpdateVideoSessionParametersKHR;
        return true;
    }
    if (!strcmp("vkDestroyVideoSessionParametersKHR", name)) {
        *addr = (void *)DestroyVideoSessionParametersKHR;
        return true;
    }
    if (!strcmp("vkCmdBeginVideoCodingKHR", name)) {
        *addr = (void *)CmdBeginVideoCodingKHR;
        return true;
    }
    if (!strcmp("vkCmdEndVideoCodingKHR", name)) {
        *addr = (void *)CmdEndVideoCodingKHR;
        return true;
    }
    if (!strcmp("vkCmdControlVideoCodingKHR", name)) {
        *addr = (void *)CmdControlVideoCodingKHR;
        return true;
    }

    // ---- VK_KHR_video_decode_queue extension commands
    if (!strcmp("vkCmdDecodeVideoKHR", name)) {
        *addr = (void *)CmdDecodeVideoKHR;
        return true;
    }

    // ---- VK_KHR_dynamic_rendering extension commands
    if (!strcmp("vkCmdBeginRenderingKHR", name)) {
        *addr = (void *)CmdBeginRenderingKHR;
        return true;
    }
    if (!strcmp("vkCmdEndRenderingKHR", name)) {
        *addr = (void *)CmdEndRenderingKHR;
        return true;
    }

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    if (!strcmp("vkGetPhysicalDeviceFeatures2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceFeatures2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceProperties2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceFormatProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceFormatProperties2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceImageFormatProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceImageFormatProperties2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceQueueFamilyProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceQueueFamilyProperties2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceMemoryProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceMemoryProperties2
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceSparseImageFormatProperties2KHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 == 1)
                     ? (void *)vkGetPhysicalDeviceSparseImageFormatProperties2
                     : NULL;
        return true;
    }

    // ---- VK_KHR_device_group extension commands
    if (!strcmp("vkGetDeviceGroupPeerMemoryFeaturesKHR", name)) {
        *addr = (void *)GetDeviceGroupPeerMemoryFeaturesKHR;
        return true;
    }
    if (!strcmp("vkCmdSetDeviceMaskKHR", name)) {
        *addr = (void *)CmdSetDeviceMaskKHR;
        return true;
    }
    if (!strcmp("vkCmdDispatchBaseKHR", name)) {
        *addr = (void *)CmdDispatchBaseKHR;
        return true;
    }

    // ---- VK_KHR_maintenance1 extension commands
    if (!strcmp("vkTrimCommandPoolKHR", name)) {
        *addr = (void *)TrimCommandPoolKHR;
        return true;
    }

    // ---- VK_KHR_device_group_creation extension commands
    if (!strcmp("vkEnumeratePhysicalDeviceGroupsKHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_device_group_creation == 1)
                     ? (void *)vkEnumeratePhysicalDeviceGroups
                     : NULL;
        return true;
    }

    // ---- VK_KHR_external_memory_capabilities extension commands
    if (!strcmp("vkGetPhysicalDeviceExternalBufferPropertiesKHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_external_memory_capabilities == 1)
                     ? (void *)vkGetPhysicalDeviceExternalBufferProperties
                     : NULL;
        return true;
    }

    // ---- VK_KHR_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetMemoryWin32HandleKHR", name)) {
        *addr = (void *)GetMemoryWin32HandleKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetMemoryWin32HandlePropertiesKHR", name)) {
        *addr = (void *)GetMemoryWin32HandlePropertiesKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_memory_fd extension commands
    if (!strcmp("vkGetMemoryFdKHR", name)) {
        *addr = (void *)GetMemoryFdKHR;
        return true;
    }
    if (!strcmp("vkGetMemoryFdPropertiesKHR", name)) {
        *addr = (void *)GetMemoryFdPropertiesKHR;
        return true;
    }

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    if (!strcmp("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_external_semaphore_capabilities == 1)
                     ? (void *)vkGetPhysicalDeviceExternalSemaphoreProperties
                     : NULL;
        return true;
    }

    // ---- VK_KHR_external_semaphore_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkImportSemaphoreWin32HandleKHR", name)) {
        *addr = (void *)ImportSemaphoreWin32HandleKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetSemaphoreWin32HandleKHR", name)) {
        *addr = (void *)GetSemaphoreWin32HandleKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_semaphore_fd extension commands
    if (!strcmp("vkImportSemaphoreFdKHR", name)) {
        *addr = (void *)ImportSemaphoreFdKHR;
        return true;
    }
    if (!strcmp("vkGetSemaphoreFdKHR", name)) {
        *addr = (void *)GetSemaphoreFdKHR;
        return true;
    }

    // ---- VK_KHR_push_descriptor extension commands
    if (!strcmp("vkCmdPushDescriptorSetKHR", name)) {
        *addr = (void *)CmdPushDescriptorSetKHR;
        return true;
    }
    if (!strcmp("vkCmdPushDescriptorSetWithTemplateKHR", name)) {
        *addr = (void *)CmdPushDescriptorSetWithTemplateKHR;
        return true;
    }

    // ---- VK_KHR_descriptor_update_template extension commands
    if (!strcmp("vkCreateDescriptorUpdateTemplateKHR", name)) {
        *addr = (void *)CreateDescriptorUpdateTemplateKHR;
        return true;
    }
    if (!strcmp("vkDestroyDescriptorUpdateTemplateKHR", name)) {
        *addr = (void *)DestroyDescriptorUpdateTemplateKHR;
        return true;
    }
    if (!strcmp("vkUpdateDescriptorSetWithTemplateKHR", name)) {
        *addr = (void *)UpdateDescriptorSetWithTemplateKHR;
        return true;
    }

    // ---- VK_KHR_create_renderpass2 extension commands
    if (!strcmp("vkCreateRenderPass2KHR", name)) {
        *addr = (void *)CreateRenderPass2KHR;
        return true;
    }
    if (!strcmp("vkCmdBeginRenderPass2KHR", name)) {
        *addr = (void *)CmdBeginRenderPass2KHR;
        return true;
    }
    if (!strcmp("vkCmdNextSubpass2KHR", name)) {
        *addr = (void *)CmdNextSubpass2KHR;
        return true;
    }
    if (!strcmp("vkCmdEndRenderPass2KHR", name)) {
        *addr = (void *)CmdEndRenderPass2KHR;
        return true;
    }

    // ---- VK_KHR_shared_presentable_image extension commands
    if (!strcmp("vkGetSwapchainStatusKHR", name)) {
        *addr = (void *)GetSwapchainStatusKHR;
        return true;
    }

    // ---- VK_KHR_external_fence_capabilities extension commands
    if (!strcmp("vkGetPhysicalDeviceExternalFencePropertiesKHR", name)) {
        *addr = (ptr_instance->enabled_known_extensions.khr_external_fence_capabilities == 1)
                     ? (void *)vkGetPhysicalDeviceExternalFenceProperties
                     : NULL;
        return true;
    }

    // ---- VK_KHR_external_fence_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkImportFenceWin32HandleKHR", name)) {
        *addr = (void *)ImportFenceWin32HandleKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetFenceWin32HandleKHR", name)) {
        *addr = (void *)GetFenceWin32HandleKHR;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_external_fence_fd extension commands
    if (!strcmp("vkImportFenceFdKHR", name)) {
        *addr = (void *)ImportFenceFdKHR;
        return true;
    }
    if (!strcmp("vkGetFenceFdKHR", name)) {
        *addr = (void *)GetFenceFdKHR;
        return true;
    }

    // ---- VK_KHR_performance_query extension commands
    if (!strcmp("vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", name)) {
        *addr = (void *)EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
        return true;
    }
    if (!strcmp("vkAcquireProfilingLockKHR", name)) {
        *addr = (void *)AcquireProfilingLockKHR;
        return true;
    }
    if (!strcmp("vkReleaseProfilingLockKHR", name)) {
        *addr = (void *)ReleaseProfilingLockKHR;
        return true;
    }

    // ---- VK_KHR_get_memory_requirements2 extension commands
    if (!strcmp("vkGetImageMemoryRequirements2KHR", name)) {
        *addr = (void *)GetImageMemoryRequirements2KHR;
        return true;
    }
    if (!strcmp("vkGetBufferMemoryRequirements2KHR", name)) {
        *addr = (void *)GetBufferMemoryRequirements2KHR;
        return true;
    }
    if (!strcmp("vkGetImageSparseMemoryRequirements2KHR", name)) {
        *addr = (void *)GetImageSparseMemoryRequirements2KHR;
        return true;
    }

    // ---- VK_KHR_sampler_ycbcr_conversion extension commands
    if (!strcmp("vkCreateSamplerYcbcrConversionKHR", name)) {
        *addr = (void *)CreateSamplerYcbcrConversionKHR;
        return true;
    }
    if (!strcmp("vkDestroySamplerYcbcrConversionKHR", name)) {
        *addr = (void *)DestroySamplerYcbcrConversionKHR;
        return true;
    }

    // ---- VK_KHR_bind_memory2 extension commands
    if (!strcmp("vkBindBufferMemory2KHR", name)) {
        *addr = (void *)BindBufferMemory2KHR;
        return true;
    }
    if (!strcmp("vkBindImageMemory2KHR", name)) {
        *addr = (void *)BindImageMemory2KHR;
        return true;
    }

    // ---- VK_KHR_maintenance3 extension commands
    if (!strcmp("vkGetDescriptorSetLayoutSupportKHR", name)) {
        *addr = (void *)GetDescriptorSetLayoutSupportKHR;
        return true;
    }

    // ---- VK_KHR_draw_indirect_count extension commands
    if (!strcmp("vkCmdDrawIndirectCountKHR", name)) {
        *addr = (void *)CmdDrawIndirectCountKHR;
        return true;
    }
    if (!strcmp("vkCmdDrawIndexedIndirectCountKHR", name)) {
        *addr = (void *)CmdDrawIndexedIndirectCountKHR;
        return true;
    }

    // ---- VK_KHR_timeline_semaphore extension commands
    if (!strcmp("vkGetSemaphoreCounterValueKHR", name)) {
        *addr = (void *)GetSemaphoreCounterValueKHR;
        return true;
    }
    if (!strcmp("vkWaitSemaphoresKHR", name)) {
        *addr = (void *)WaitSemaphoresKHR;
        return true;
    }
    if (!strcmp("vkSignalSemaphoreKHR", name)) {
        *addr = (void *)SignalSemaphoreKHR;
        return true;
    }

    // ---- VK_KHR_fragment_shading_rate extension commands
    if (!strcmp("vkGetPhysicalDeviceFragmentShadingRatesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceFragmentShadingRatesKHR;
        return true;
    }
    if (!strcmp("vkCmdSetFragmentShadingRateKHR", name)) {
        *addr = (void *)CmdSetFragmentShadingRateKHR;
        return true;
    }

    // ---- VK_KHR_dynamic_rendering_local_read extension commands
    if (!strcmp("vkCmdSetRenderingAttachmentLocationsKHR", name)) {
        *addr = (void *)CmdSetRenderingAttachmentLocationsKHR;
        return true;
    }
    if (!strcmp("vkCmdSetRenderingInputAttachmentIndicesKHR", name)) {
        *addr = (void *)CmdSetRenderingInputAttachmentIndicesKHR;
        return true;
    }

    // ---- VK_KHR_present_wait extension commands
    if (!strcmp("vkWaitForPresentKHR", name)) {
        *addr = (void *)WaitForPresentKHR;
        return true;
    }

    // ---- VK_KHR_buffer_device_address extension commands
    if (!strcmp("vkGetBufferDeviceAddressKHR", name)) {
        *addr = (void *)GetBufferDeviceAddressKHR;
        return true;
    }
    if (!strcmp("vkGetBufferOpaqueCaptureAddressKHR", name)) {
        *addr = (void *)GetBufferOpaqueCaptureAddressKHR;
        return true;
    }
    if (!strcmp("vkGetDeviceMemoryOpaqueCaptureAddressKHR", name)) {
        *addr = (void *)GetDeviceMemoryOpaqueCaptureAddressKHR;
        return true;
    }

    // ---- VK_KHR_deferred_host_operations extension commands
    if (!strcmp("vkCreateDeferredOperationKHR", name)) {
        *addr = (void *)CreateDeferredOperationKHR;
        return true;
    }
    if (!strcmp("vkDestroyDeferredOperationKHR", name)) {
        *addr = (void *)DestroyDeferredOperationKHR;
        return true;
    }
    if (!strcmp("vkGetDeferredOperationMaxConcurrencyKHR", name)) {
        *addr = (void *)GetDeferredOperationMaxConcurrencyKHR;
        return true;
    }
    if (!strcmp("vkGetDeferredOperationResultKHR", name)) {
        *addr = (void *)GetDeferredOperationResultKHR;
        return true;
    }
    if (!strcmp("vkDeferredOperationJoinKHR", name)) {
        *addr = (void *)DeferredOperationJoinKHR;
        return true;
    }

    // ---- VK_KHR_pipeline_executable_properties extension commands
    if (!strcmp("vkGetPipelineExecutablePropertiesKHR", name)) {
        *addr = (void *)GetPipelineExecutablePropertiesKHR;
        return true;
    }
    if (!strcmp("vkGetPipelineExecutableStatisticsKHR", name)) {
        *addr = (void *)GetPipelineExecutableStatisticsKHR;
        return true;
    }
    if (!strcmp("vkGetPipelineExecutableInternalRepresentationsKHR", name)) {
        *addr = (void *)GetPipelineExecutableInternalRepresentationsKHR;
        return true;
    }

    // ---- VK_KHR_map_memory2 extension commands
    if (!strcmp("vkMapMemory2KHR", name)) {
        *addr = (void *)MapMemory2KHR;
        return true;
    }
    if (!strcmp("vkUnmapMemory2KHR", name)) {
        *addr = (void *)UnmapMemory2KHR;
        return true;
    }

    // ---- VK_KHR_video_encode_queue extension commands
    if (!strcmp("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;
        return true;
    }
    if (!strcmp("vkGetEncodedVideoSessionParametersKHR", name)) {
        *addr = (void *)GetEncodedVideoSessionParametersKHR;
        return true;
    }
    if (!strcmp("vkCmdEncodeVideoKHR", name)) {
        *addr = (void *)CmdEncodeVideoKHR;
        return true;
    }

    // ---- VK_KHR_synchronization2 extension commands
    if (!strcmp("vkCmdSetEvent2KHR", name)) {
        *addr = (void *)CmdSetEvent2KHR;
        return true;
    }
    if (!strcmp("vkCmdResetEvent2KHR", name)) {
        *addr = (void *)CmdResetEvent2KHR;
        return true;
    }
    if (!strcmp("vkCmdWaitEvents2KHR", name)) {
        *addr = (void *)CmdWaitEvents2KHR;
        return true;
    }
    if (!strcmp("vkCmdPipelineBarrier2KHR", name)) {
        *addr = (void *)CmdPipelineBarrier2KHR;
        return true;
    }
    if (!strcmp("vkCmdWriteTimestamp2KHR", name)) {
        *addr = (void *)CmdWriteTimestamp2KHR;
        return true;
    }
    if (!strcmp("vkQueueSubmit2KHR", name)) {
        *addr = (void *)QueueSubmit2KHR;
        return true;
    }

    // ---- VK_KHR_copy_commands2 extension commands
    if (!strcmp("vkCmdCopyBuffer2KHR", name)) {
        *addr = (void *)CmdCopyBuffer2KHR;
        return true;
    }
    if (!strcmp("vkCmdCopyImage2KHR", name)) {
        *addr = (void *)CmdCopyImage2KHR;
        return true;
    }
    if (!strcmp("vkCmdCopyBufferToImage2KHR", name)) {
        *addr = (void *)CmdCopyBufferToImage2KHR;
        return true;
    }
    if (!strcmp("vkCmdCopyImageToBuffer2KHR", name)) {
        *addr = (void *)CmdCopyImageToBuffer2KHR;
        return true;
    }
    if (!strcmp("vkCmdBlitImage2KHR", name)) {
        *addr = (void *)CmdBlitImage2KHR;
        return true;
    }
    if (!strcmp("vkCmdResolveImage2KHR", name)) {
        *addr = (void *)CmdResolveImage2KHR;
        return true;
    }

    // ---- VK_KHR_ray_tracing_maintenance1 extension commands
    if (!strcmp("vkCmdTraceRaysIndirect2KHR", name)) {
        *addr = (void *)CmdTraceRaysIndirect2KHR;
        return true;
    }

    // ---- VK_KHR_maintenance4 extension commands
    if (!strcmp("vkGetDeviceBufferMemoryRequirementsKHR", name)) {
        *addr = (void *)GetDeviceBufferMemoryRequirementsKHR;
        return true;
    }
    if (!strcmp("vkGetDeviceImageMemoryRequirementsKHR", name)) {
        *addr = (void *)GetDeviceImageMemoryRequirementsKHR;
        return true;
    }
    if (!strcmp("vkGetDeviceImageSparseMemoryRequirementsKHR", name)) {
        *addr = (void *)GetDeviceImageSparseMemoryRequirementsKHR;
        return true;
    }

    // ---- VK_KHR_maintenance5 extension commands
    if (!strcmp("vkCmdBindIndexBuffer2KHR", name)) {
        *addr = (void *)CmdBindIndexBuffer2KHR;
        return true;
    }
    if (!strcmp("vkGetRenderingAreaGranularityKHR", name)) {
        *addr = (void *)GetRenderingAreaGranularityKHR;
        return true;
    }
    if (!strcmp("vkGetDeviceImageSubresourceLayoutKHR", name)) {
        *addr = (void *)GetDeviceImageSubresourceLayoutKHR;
        return true;
    }
    if (!strcmp("vkGetImageSubresourceLayout2KHR", name)) {
        *addr = (void *)GetImageSubresourceLayout2KHR;
        return true;
    }

    // ---- VK_KHR_pipeline_binary extension commands
    if (!strcmp("vkCreatePipelineBinariesKHR", name)) {
        *addr = (void *)CreatePipelineBinariesKHR;
        return true;
    }
    if (!strcmp("vkDestroyPipelineBinaryKHR", name)) {
        *addr = (void *)DestroyPipelineBinaryKHR;
        return true;
    }
    if (!strcmp("vkGetPipelineKeyKHR", name)) {
        *addr = (void *)GetPipelineKeyKHR;
        return true;
    }
    if (!strcmp("vkGetPipelineBinaryDataKHR", name)) {
        *addr = (void *)GetPipelineBinaryDataKHR;
        return true;
    }
    if (!strcmp("vkReleaseCapturedPipelineDataKHR", name)) {
        *addr = (void *)ReleaseCapturedPipelineDataKHR;
        return true;
    }

    // ---- VK_KHR_cooperative_matrix extension commands
    if (!strcmp("vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR", name)) {
        *addr = (void *)GetPhysicalDeviceCooperativeMatrixPropertiesKHR;
        return true;
    }

    // ---- VK_KHR_line_rasterization extension commands
    if (!strcmp("vkCmdSetLineStippleKHR", name)) {
        *addr = (void *)CmdSetLineStippleKHR;
        return true;
    }

    // ---- VK_KHR_calibrated_timestamps extension commands
    if (!strcmp("vkGetPhysicalDeviceCalibrateableTimeDomainsKHR", name)) {
        *addr = (void *)GetPhysicalDeviceCalibrateableTimeDomainsKHR;
        return true;
    }
    if (!strcmp("vkGetCalibratedTimestampsKHR", name)) {
        *addr = (void *)GetCalibratedTimestampsKHR;
        return true;
    }

    // ---- VK_KHR_maintenance6 extension commands
    if (!strcmp("vkCmdBindDescriptorSets2KHR", name)) {
        *addr = (void *)CmdBindDescriptorSets2KHR;
        return true;
    }
    if (!strcmp("vkCmdPushConstants2KHR", name)) {
        *addr = (void *)CmdPushConstants2KHR;
        return true;
    }
    if (!strcmp("vkCmdPushDescriptorSet2KHR", name)) {
        *addr = (void *)CmdPushDescriptorSet2KHR;
        return true;
    }
    if (!strcmp("vkCmdPushDescriptorSetWithTemplate2KHR", name)) {
        *addr = (void *)CmdPushDescriptorSetWithTemplate2KHR;
        return true;
    }
    if (!strcmp("vkCmdSetDescriptorBufferOffsets2EXT", name)) {
        *addr = (void *)CmdSetDescriptorBufferOffsets2EXT;
        return true;
    }
    if (!strcmp("vkCmdBindDescriptorBufferEmbeddedSamplers2EXT", name)) {
        *addr = (void *)CmdBindDescriptorBufferEmbeddedSamplers2EXT;
        return true;
    }

    // ---- VK_EXT_debug_marker extension commands
    if (!strcmp("vkDebugMarkerSetObjectTagEXT", name)) {
        *addr = (void *)DebugMarkerSetObjectTagEXT;
        return true;
    }
    if (!strcmp("vkDebugMarkerSetObjectNameEXT", name)) {
        *addr = (void *)DebugMarkerSetObjectNameEXT;
        return true;
    }
    if (!strcmp("vkCmdDebugMarkerBeginEXT", name)) {
        *addr = (void *)CmdDebugMarkerBeginEXT;
        return true;
    }
    if (!strcmp("vkCmdDebugMarkerEndEXT", name)) {
        *addr = (void *)CmdDebugMarkerEndEXT;
        return true;
    }
    if (!strcmp("vkCmdDebugMarkerInsertEXT", name)) {
        *addr = (void *)CmdDebugMarkerInsertEXT;
        return true;
    }

    // ---- VK_EXT_transform_feedback extension commands
    if (!strcmp("vkCmdBindTransformFeedbackBuffersEXT", name)) {
        *addr = (void *)CmdBindTransformFeedbackBuffersEXT;
        return true;
    }
    if (!strcmp("vkCmdBeginTransformFeedbackEXT", name)) {
        *addr = (void *)CmdBeginTransformFeedbackEXT;
        return true;
    }
    if (!strcmp("vkCmdEndTransformFeedbackEXT", name)) {
        *addr = (void *)CmdEndTransformFeedbackEXT;
        return true;
    }
    if (!strcmp("vkCmdBeginQueryIndexedEXT", name)) {
        *addr = (void *)CmdBeginQueryIndexedEXT;
        return true;
    }
    if (!strcmp("vkCmdEndQueryIndexedEXT", name)) {
        *addr = (void *)CmdEndQueryIndexedEXT;
        return true;
    }
    if (!strcmp("vkCmdDrawIndirectByteCountEXT", name)) {
        *addr = (void *)CmdDrawIndirectByteCountEXT;
        return true;
    }

    // ---- VK_NVX_binary_import extension commands
    if (!strcmp("vkCreateCuModuleNVX", name)) {
        *addr = (void *)CreateCuModuleNVX;
        return true;
    }
    if (!strcmp("vkCreateCuFunctionNVX", name)) {
        *addr = (void *)CreateCuFunctionNVX;
        return true;
    }
    if (!strcmp("vkDestroyCuModuleNVX", name)) {
        *addr = (void *)DestroyCuModuleNVX;
        return true;
    }
    if (!strcmp("vkDestroyCuFunctionNVX", name)) {
        *addr = (void *)DestroyCuFunctionNVX;
        return true;
    }
    if (!strcmp("vkCmdCuLaunchKernelNVX", name)) {
        *addr = (void *)CmdCuLaunchKernelNVX;
        return true;
    }

    // ---- VK_NVX_image_view_handle extension commands
    if (!strcmp("vkGetImageViewHandleNVX", name)) {
        *addr = (void *)GetImageViewHandleNVX;
        return true;
    }
    if (!strcmp("vkGetImageViewHandle64NVX", name)) {
        *addr = (void *)GetImageViewHandle64NVX;
        return true;
    }
    if (!strcmp("vkGetImageViewAddressNVX", name)) {
        *addr = (void *)GetImageViewAddressNVX;
        return true;
    }

    // ---- VK_AMD_draw_indirect_count extension commands
    if (!strcmp("vkCmdDrawIndirectCountAMD", name)) {
        *addr = (void *)CmdDrawIndirectCountAMD;
        return true;
    }
    if (!strcmp("vkCmdDrawIndexedIndirectCountAMD", name)) {
        *addr = (void *)CmdDrawIndexedIndirectCountAMD;
        return true;
    }

    // ---- VK_AMD_shader_info extension commands
    if (!strcmp("vkGetShaderInfoAMD", name)) {
        *addr = (void *)GetShaderInfoAMD;
        return true;
    }

    // ---- VK_NV_external_memory_capabilities extension commands
    if (!strcmp("vkGetPhysicalDeviceExternalImageFormatPropertiesNV", name)) {
        *addr = (ptr_instance->enabled_known_extensions.nv_external_memory_capabilities == 1)
                     ? (void *)GetPhysicalDeviceExternalImageFormatPropertiesNV
                     : NULL;
        return true;
    }

    // ---- VK_NV_external_memory_win32 extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetMemoryWin32HandleNV", name)) {
        *addr = (void *)GetMemoryWin32HandleNV;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_conditional_rendering extension commands
    if (!strcmp("vkCmdBeginConditionalRenderingEXT", name)) {
        *addr = (void *)CmdBeginConditionalRenderingEXT;
        return true;
    }
    if (!strcmp("vkCmdEndConditionalRenderingEXT", name)) {
        *addr = (void *)CmdEndConditionalRenderingEXT;
        return true;
    }

    // ---- VK_NV_clip_space_w_scaling extension commands
    if (!strcmp("vkCmdSetViewportWScalingNV", name)) {
        *addr = (void *)CmdSetViewportWScalingNV;
        return true;
    }

    // ---- VK_EXT_direct_mode_display extension commands
    if (!strcmp("vkReleaseDisplayEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_direct_mode_display == 1)
                     ? (void *)ReleaseDisplayEXT
                     : NULL;
        return true;
    }

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    if (!strcmp("vkAcquireXlibDisplayEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_acquire_xlib_display == 1)
                     ? (void *)AcquireXlibDisplayEXT
                     : NULL;
        return true;
    }
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    if (!strcmp("vkGetRandROutputDisplayEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_acquire_xlib_display == 1)
                     ? (void *)GetRandROutputDisplayEXT
                     : NULL;
        return true;
    }
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    if (!strcmp("vkGetPhysicalDeviceSurfaceCapabilities2EXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_display_surface_counter == 1)
                     ? (void *)GetPhysicalDeviceSurfaceCapabilities2EXT
                     : NULL;
        return true;
    }

    // ---- VK_EXT_display_control extension commands
    if (!strcmp("vkDisplayPowerControlEXT", name)) {
        *addr = (void *)DisplayPowerControlEXT;
        return true;
    }
    if (!strcmp("vkRegisterDeviceEventEXT", name)) {
        *addr = (void *)RegisterDeviceEventEXT;
        return true;
    }
    if (!strcmp("vkRegisterDisplayEventEXT", name)) {
        *addr = (void *)RegisterDisplayEventEXT;
        return true;
    }
    if (!strcmp("vkGetSwapchainCounterEXT", name)) {
        *addr = (void *)GetSwapchainCounterEXT;
        return true;
    }

    // ---- VK_GOOGLE_display_timing extension commands
    if (!strcmp("vkGetRefreshCycleDurationGOOGLE", name)) {
        *addr = (void *)GetRefreshCycleDurationGOOGLE;
        return true;
    }
    if (!strcmp("vkGetPastPresentationTimingGOOGLE", name)) {
        *addr = (void *)GetPastPresentationTimingGOOGLE;
        return true;
    }

    // ---- VK_EXT_discard_rectangles extension commands
    if (!strcmp("vkCmdSetDiscardRectangleEXT", name)) {
        *addr = (void *)CmdSetDiscardRectangleEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDiscardRectangleEnableEXT", name)) {
        *addr = (void *)CmdSetDiscardRectangleEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDiscardRectangleModeEXT", name)) {
        *addr = (void *)CmdSetDiscardRectangleModeEXT;
        return true;
    }

    // ---- VK_EXT_hdr_metadata extension commands
    if (!strcmp("vkSetHdrMetadataEXT", name)) {
        *addr = (void *)SetHdrMetadataEXT;
        return true;
    }

    // ---- VK_EXT_debug_utils extension commands
    if (!strcmp("vkSetDebugUtilsObjectNameEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)SetDebugUtilsObjectNameEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkSetDebugUtilsObjectTagEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)SetDebugUtilsObjectTagEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkQueueBeginDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)QueueBeginDebugUtilsLabelEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkQueueEndDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)QueueEndDebugUtilsLabelEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkQueueInsertDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)QueueInsertDebugUtilsLabelEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkCmdBeginDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)CmdBeginDebugUtilsLabelEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkCmdEndDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)CmdEndDebugUtilsLabelEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkCmdInsertDebugUtilsLabelEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_debug_utils == 1)
                     ? (void *)CmdInsertDebugUtilsLabelEXT
                     : NULL;
        return true;
    }

    // ---- VK_ANDROID_external_memory_android_hardware_buffer extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp("vkGetAndroidHardwareBufferPropertiesANDROID", name)) {
        *addr = (void *)GetAndroidHardwareBufferPropertiesANDROID;
        return true;
    }
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp("vkGetMemoryAndroidHardwareBufferANDROID", name)) {
        *addr = (void *)GetMemoryAndroidHardwareBufferANDROID;
        return true;
    }
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_AMDX_shader_enqueue extension commands
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkCreateExecutionGraphPipelinesAMDX", name)) {
        *addr = (void *)CreateExecutionGraphPipelinesAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkGetExecutionGraphPipelineScratchSizeAMDX", name)) {
        *addr = (void *)GetExecutionGraphPipelineScratchSizeAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkGetExecutionGraphPipelineNodeIndexAMDX", name)) {
        *addr = (void *)GetExecutionGraphPipelineNodeIndexAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkCmdInitializeGraphScratchMemoryAMDX", name)) {
        *addr = (void *)CmdInitializeGraphScratchMemoryAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkCmdDispatchGraphAMDX", name)) {
        *addr = (void *)CmdDispatchGraphAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkCmdDispatchGraphIndirectAMDX", name)) {
        *addr = (void *)CmdDispatchGraphIndirectAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    if (!strcmp("vkCmdDispatchGraphIndirectCountAMDX", name)) {
        *addr = (void *)CmdDispatchGraphIndirectCountAMDX;
        return true;
    }
#endif // VK_ENABLE_BETA_EXTENSIONS

    // ---- VK_EXT_sample_locations extension commands
    if (!strcmp("vkCmdSetSampleLocationsEXT", name)) {
        *addr = (void *)CmdSetSampleLocationsEXT;
        return true;
    }
    if (!strcmp("vkGetPhysicalDeviceMultisamplePropertiesEXT", name)) {
        *addr = (void *)GetPhysicalDeviceMultisamplePropertiesEXT;
        return true;
    }

    // ---- VK_EXT_image_drm_format_modifier extension commands
    if (!strcmp("vkGetImageDrmFormatModifierPropertiesEXT", name)) {
        *addr = (void *)GetImageDrmFormatModifierPropertiesEXT;
        return true;
    }

    // ---- VK_EXT_validation_cache extension commands
    if (!strcmp("vkCreateValidationCacheEXT", name)) {
        *addr = (void *)CreateValidationCacheEXT;
        return true;
    }
    if (!strcmp("vkDestroyValidationCacheEXT", name)) {
        *addr = (void *)DestroyValidationCacheEXT;
        return true;
    }
    if (!strcmp("vkMergeValidationCachesEXT", name)) {
        *addr = (void *)MergeValidationCachesEXT;
        return true;
    }
    if (!strcmp("vkGetValidationCacheDataEXT", name)) {
        *addr = (void *)GetValidationCacheDataEXT;
        return true;
    }

    // ---- VK_NV_shading_rate_image extension commands
    if (!strcmp("vkCmdBindShadingRateImageNV", name)) {
        *addr = (void *)CmdBindShadingRateImageNV;
        return true;
    }
    if (!strcmp("vkCmdSetViewportShadingRatePaletteNV", name)) {
        *addr = (void *)CmdSetViewportShadingRatePaletteNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoarseSampleOrderNV", name)) {
        *addr = (void *)CmdSetCoarseSampleOrderNV;
        return true;
    }

    // ---- VK_NV_ray_tracing extension commands
    if (!strcmp("vkCreateAccelerationStructureNV", name)) {
        *addr = (void *)CreateAccelerationStructureNV;
        return true;
    }
    if (!strcmp("vkDestroyAccelerationStructureNV", name)) {
        *addr = (void *)DestroyAccelerationStructureNV;
        return true;
    }
    if (!strcmp("vkGetAccelerationStructureMemoryRequirementsNV", name)) {
        *addr = (void *)GetAccelerationStructureMemoryRequirementsNV;
        return true;
    }
    if (!strcmp("vkBindAccelerationStructureMemoryNV", name)) {
        *addr = (void *)BindAccelerationStructureMemoryNV;
        return true;
    }
    if (!strcmp("vkCmdBuildAccelerationStructureNV", name)) {
        *addr = (void *)CmdBuildAccelerationStructureNV;
        return true;
    }
    if (!strcmp("vkCmdCopyAccelerationStructureNV", name)) {
        *addr = (void *)CmdCopyAccelerationStructureNV;
        return true;
    }
    if (!strcmp("vkCmdTraceRaysNV", name)) {
        *addr = (void *)CmdTraceRaysNV;
        return true;
    }
    if (!strcmp("vkCreateRayTracingPipelinesNV", name)) {
        *addr = (void *)CreateRayTracingPipelinesNV;
        return true;
    }
    if (!strcmp("vkGetRayTracingShaderGroupHandlesKHR", name)) {
        *addr = (void *)GetRayTracingShaderGroupHandlesKHR;
        return true;
    }
    if (!strcmp("vkGetRayTracingShaderGroupHandlesNV", name)) {
        *addr = (void *)GetRayTracingShaderGroupHandlesNV;
        return true;
    }
    if (!strcmp("vkGetAccelerationStructureHandleNV", name)) {
        *addr = (void *)GetAccelerationStructureHandleNV;
        return true;
    }
    if (!strcmp("vkCmdWriteAccelerationStructuresPropertiesNV", name)) {
        *addr = (void *)CmdWriteAccelerationStructuresPropertiesNV;
        return true;
    }
    if (!strcmp("vkCompileDeferredNV", name)) {
        *addr = (void *)CompileDeferredNV;
        return true;
    }

    // ---- VK_EXT_external_memory_host extension commands
    if (!strcmp("vkGetMemoryHostPointerPropertiesEXT", name)) {
        *addr = (void *)GetMemoryHostPointerPropertiesEXT;
        return true;
    }

    // ---- VK_AMD_buffer_marker extension commands
    if (!strcmp("vkCmdWriteBufferMarkerAMD", name)) {
        *addr = (void *)CmdWriteBufferMarkerAMD;
        return true;
    }
    if (!strcmp("vkCmdWriteBufferMarker2AMD", name)) {
        *addr = (void *)CmdWriteBufferMarker2AMD;
        return true;
    }

    // ---- VK_EXT_calibrated_timestamps extension commands
    if (!strcmp("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", name)) {
        *addr = (void *)GetPhysicalDeviceCalibrateableTimeDomainsEXT;
        return true;
    }
    if (!strcmp("vkGetCalibratedTimestampsEXT", name)) {
        *addr = (void *)GetCalibratedTimestampsEXT;
        return true;
    }

    // ---- VK_NV_mesh_shader extension commands
    if (!strcmp("vkCmdDrawMeshTasksNV", name)) {
        *addr = (void *)CmdDrawMeshTasksNV;
        return true;
    }
    if (!strcmp("vkCmdDrawMeshTasksIndirectNV", name)) {
        *addr = (void *)CmdDrawMeshTasksIndirectNV;
        return true;
    }
    if (!strcmp("vkCmdDrawMeshTasksIndirectCountNV", name)) {
        *addr = (void *)CmdDrawMeshTasksIndirectCountNV;
        return true;
    }

    // ---- VK_NV_scissor_exclusive extension commands
    if (!strcmp("vkCmdSetExclusiveScissorEnableNV", name)) {
        *addr = (void *)CmdSetExclusiveScissorEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetExclusiveScissorNV", name)) {
        *addr = (void *)CmdSetExclusiveScissorNV;
        return true;
    }

    // ---- VK_NV_device_diagnostic_checkpoints extension commands
    if (!strcmp("vkCmdSetCheckpointNV", name)) {
        *addr = (void *)CmdSetCheckpointNV;
        return true;
    }
    if (!strcmp("vkGetQueueCheckpointDataNV", name)) {
        *addr = (void *)GetQueueCheckpointDataNV;
        return true;
    }
    if (!strcmp("vkGetQueueCheckpointData2NV", name)) {
        *addr = (void *)GetQueueCheckpointData2NV;
        return true;
    }

    // ---- VK_INTEL_performance_query extension commands
    if (!strcmp("vkInitializePerformanceApiINTEL", name)) {
        *addr = (void *)InitializePerformanceApiINTEL;
        return true;
    }
    if (!strcmp("vkUninitializePerformanceApiINTEL", name)) {
        *addr = (void *)UninitializePerformanceApiINTEL;
        return true;
    }
    if (!strcmp("vkCmdSetPerformanceMarkerINTEL", name)) {
        *addr = (void *)CmdSetPerformanceMarkerINTEL;
        return true;
    }
    if (!strcmp("vkCmdSetPerformanceStreamMarkerINTEL", name)) {
        *addr = (void *)CmdSetPerformanceStreamMarkerINTEL;
        return true;
    }
    if (!strcmp("vkCmdSetPerformanceOverrideINTEL", name)) {
        *addr = (void *)CmdSetPerformanceOverrideINTEL;
        return true;
    }
    if (!strcmp("vkAcquirePerformanceConfigurationINTEL", name)) {
        *addr = (void *)AcquirePerformanceConfigurationINTEL;
        return true;
    }
    if (!strcmp("vkReleasePerformanceConfigurationINTEL", name)) {
        *addr = (void *)ReleasePerformanceConfigurationINTEL;
        return true;
    }
    if (!strcmp("vkQueueSetPerformanceConfigurationINTEL", name)) {
        *addr = (void *)QueueSetPerformanceConfigurationINTEL;
        return true;
    }
    if (!strcmp("vkGetPerformanceParameterINTEL", name)) {
        *addr = (void *)GetPerformanceParameterINTEL;
        return true;
    }

    // ---- VK_AMD_display_native_hdr extension commands
    if (!strcmp("vkSetLocalDimmingAMD", name)) {
        *addr = (void *)SetLocalDimmingAMD;
        return true;
    }

    // ---- VK_EXT_buffer_device_address extension commands
    if (!strcmp("vkGetBufferDeviceAddressEXT", name)) {
        *addr = (void *)GetBufferDeviceAddressEXT;
        return true;
    }

    // ---- VK_EXT_tooling_info extension commands
    if (!strcmp("vkGetPhysicalDeviceToolPropertiesEXT", name)) {
        *addr = (void *)GetPhysicalDeviceToolPropertiesEXT;
        return true;
    }

    // ---- VK_NV_cooperative_matrix extension commands
    if (!strcmp("vkGetPhysicalDeviceCooperativeMatrixPropertiesNV", name)) {
        *addr = (void *)GetPhysicalDeviceCooperativeMatrixPropertiesNV;
        return true;
    }

    // ---- VK_NV_coverage_reduction_mode extension commands
    if (!strcmp("vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV", name)) {
        *addr = (void *)GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;
        return true;
    }

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetPhysicalDeviceSurfacePresentModes2EXT", name)) {
        *addr = (void *)GetPhysicalDeviceSurfacePresentModes2EXT;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkAcquireFullScreenExclusiveModeEXT", name)) {
        *addr = (void *)AcquireFullScreenExclusiveModeEXT;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkReleaseFullScreenExclusiveModeEXT", name)) {
        *addr = (void *)ReleaseFullScreenExclusiveModeEXT;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetDeviceGroupSurfacePresentModes2EXT", name)) {
        *addr = (void *)GetDeviceGroupSurfacePresentModes2EXT;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_line_rasterization extension commands
    if (!strcmp("vkCmdSetLineStippleEXT", name)) {
        *addr = (void *)CmdSetLineStippleEXT;
        return true;
    }

    // ---- VK_EXT_host_query_reset extension commands
    if (!strcmp("vkResetQueryPoolEXT", name)) {
        *addr = (void *)ResetQueryPoolEXT;
        return true;
    }

    // ---- VK_EXT_extended_dynamic_state extension commands
    if (!strcmp("vkCmdSetCullModeEXT", name)) {
        *addr = (void *)CmdSetCullModeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetFrontFaceEXT", name)) {
        *addr = (void *)CmdSetFrontFaceEXT;
        return true;
    }
    if (!strcmp("vkCmdSetPrimitiveTopologyEXT", name)) {
        *addr = (void *)CmdSetPrimitiveTopologyEXT;
        return true;
    }
    if (!strcmp("vkCmdSetViewportWithCountEXT", name)) {
        *addr = (void *)CmdSetViewportWithCountEXT;
        return true;
    }
    if (!strcmp("vkCmdSetScissorWithCountEXT", name)) {
        *addr = (void *)CmdSetScissorWithCountEXT;
        return true;
    }
    if (!strcmp("vkCmdBindVertexBuffers2EXT", name)) {
        *addr = (void *)CmdBindVertexBuffers2EXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthTestEnableEXT", name)) {
        *addr = (void *)CmdSetDepthTestEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthWriteEnableEXT", name)) {
        *addr = (void *)CmdSetDepthWriteEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthCompareOpEXT", name)) {
        *addr = (void *)CmdSetDepthCompareOpEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthBoundsTestEnableEXT", name)) {
        *addr = (void *)CmdSetDepthBoundsTestEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetStencilTestEnableEXT", name)) {
        *addr = (void *)CmdSetStencilTestEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetStencilOpEXT", name)) {
        *addr = (void *)CmdSetStencilOpEXT;
        return true;
    }

    // ---- VK_EXT_host_image_copy extension commands
    if (!strcmp("vkCopyMemoryToImageEXT", name)) {
        *addr = (void *)CopyMemoryToImageEXT;
        return true;
    }
    if (!strcmp("vkCopyImageToMemoryEXT", name)) {
        *addr = (void *)CopyImageToMemoryEXT;
        return true;
    }
    if (!strcmp("vkCopyImageToImageEXT", name)) {
        *addr = (void *)CopyImageToImageEXT;
        return true;
    }
    if (!strcmp("vkTransitionImageLayoutEXT", name)) {
        *addr = (void *)TransitionImageLayoutEXT;
        return true;
    }
    if (!strcmp("vkGetImageSubresourceLayout2EXT", name)) {
        *addr = (void *)GetImageSubresourceLayout2EXT;
        return true;
    }

    // ---- VK_EXT_swapchain_maintenance1 extension commands
    if (!strcmp("vkReleaseSwapchainImagesEXT", name)) {
        *addr = (void *)ReleaseSwapchainImagesEXT;
        return true;
    }

    // ---- VK_NV_device_generated_commands extension commands
    if (!strcmp("vkGetGeneratedCommandsMemoryRequirementsNV", name)) {
        *addr = (void *)GetGeneratedCommandsMemoryRequirementsNV;
        return true;
    }
    if (!strcmp("vkCmdPreprocessGeneratedCommandsNV", name)) {
        *addr = (void *)CmdPreprocessGeneratedCommandsNV;
        return true;
    }
    if (!strcmp("vkCmdExecuteGeneratedCommandsNV", name)) {
        *addr = (void *)CmdExecuteGeneratedCommandsNV;
        return true;
    }
    if (!strcmp("vkCmdBindPipelineShaderGroupNV", name)) {
        *addr = (void *)CmdBindPipelineShaderGroupNV;
        return true;
    }
    if (!strcmp("vkCreateIndirectCommandsLayoutNV", name)) {
        *addr = (void *)CreateIndirectCommandsLayoutNV;
        return true;
    }
    if (!strcmp("vkDestroyIndirectCommandsLayoutNV", name)) {
        *addr = (void *)DestroyIndirectCommandsLayoutNV;
        return true;
    }

    // ---- VK_EXT_depth_bias_control extension commands
    if (!strcmp("vkCmdSetDepthBias2EXT", name)) {
        *addr = (void *)CmdSetDepthBias2EXT;
        return true;
    }

    // ---- VK_EXT_acquire_drm_display extension commands
    if (!strcmp("vkAcquireDrmDisplayEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_acquire_drm_display == 1)
                     ? (void *)AcquireDrmDisplayEXT
                     : NULL;
        return true;
    }
    if (!strcmp("vkGetDrmDisplayEXT", name)) {
        *addr = (ptr_instance->enabled_known_extensions.ext_acquire_drm_display == 1)
                     ? (void *)GetDrmDisplayEXT
                     : NULL;
        return true;
    }

    // ---- VK_EXT_private_data extension commands
    if (!strcmp("vkCreatePrivateDataSlotEXT", name)) {
        *addr = (void *)CreatePrivateDataSlotEXT;
        return true;
    }
    if (!strcmp("vkDestroyPrivateDataSlotEXT", name)) {
        *addr = (void *)DestroyPrivateDataSlotEXT;
        return true;
    }
    if (!strcmp("vkSetPrivateDataEXT", name)) {
        *addr = (void *)SetPrivateDataEXT;
        return true;
    }
    if (!strcmp("vkGetPrivateDataEXT", name)) {
        *addr = (void *)GetPrivateDataEXT;
        return true;
    }

    // ---- VK_NV_cuda_kernel_launch extension commands
    if (!strcmp("vkCreateCudaModuleNV", name)) {
        *addr = (void *)CreateCudaModuleNV;
        return true;
    }
    if (!strcmp("vkGetCudaModuleCacheNV", name)) {
        *addr = (void *)GetCudaModuleCacheNV;
        return true;
    }
    if (!strcmp("vkCreateCudaFunctionNV", name)) {
        *addr = (void *)CreateCudaFunctionNV;
        return true;
    }
    if (!strcmp("vkDestroyCudaModuleNV", name)) {
        *addr = (void *)DestroyCudaModuleNV;
        return true;
    }
    if (!strcmp("vkDestroyCudaFunctionNV", name)) {
        *addr = (void *)DestroyCudaFunctionNV;
        return true;
    }
    if (!strcmp("vkCmdCudaLaunchKernelNV", name)) {
        *addr = (void *)CmdCudaLaunchKernelNV;
        return true;
    }

    // ---- VK_EXT_metal_objects extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp("vkExportMetalObjectsEXT", name)) {
        *addr = (void *)ExportMetalObjectsEXT;
        return true;
    }
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_descriptor_buffer extension commands
    if (!strcmp("vkGetDescriptorSetLayoutSizeEXT", name)) {
        *addr = (void *)GetDescriptorSetLayoutSizeEXT;
        return true;
    }
    if (!strcmp("vkGetDescriptorSetLayoutBindingOffsetEXT", name)) {
        *addr = (void *)GetDescriptorSetLayoutBindingOffsetEXT;
        return true;
    }
    if (!strcmp("vkGetDescriptorEXT", name)) {
        *addr = (void *)GetDescriptorEXT;
        return true;
    }
    if (!strcmp("vkCmdBindDescriptorBuffersEXT", name)) {
        *addr = (void *)CmdBindDescriptorBuffersEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDescriptorBufferOffsetsEXT", name)) {
        *addr = (void *)CmdSetDescriptorBufferOffsetsEXT;
        return true;
    }
    if (!strcmp("vkCmdBindDescriptorBufferEmbeddedSamplersEXT", name)) {
        *addr = (void *)CmdBindDescriptorBufferEmbeddedSamplersEXT;
        return true;
    }
    if (!strcmp("vkGetBufferOpaqueCaptureDescriptorDataEXT", name)) {
        *addr = (void *)GetBufferOpaqueCaptureDescriptorDataEXT;
        return true;
    }
    if (!strcmp("vkGetImageOpaqueCaptureDescriptorDataEXT", name)) {
        *addr = (void *)GetImageOpaqueCaptureDescriptorDataEXT;
        return true;
    }
    if (!strcmp("vkGetImageViewOpaqueCaptureDescriptorDataEXT", name)) {
        *addr = (void *)GetImageViewOpaqueCaptureDescriptorDataEXT;
        return true;
    }
    if (!strcmp("vkGetSamplerOpaqueCaptureDescriptorDataEXT", name)) {
        *addr = (void *)GetSamplerOpaqueCaptureDescriptorDataEXT;
        return true;
    }
    if (!strcmp("vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", name)) {
        *addr = (void *)GetAccelerationStructureOpaqueCaptureDescriptorDataEXT;
        return true;
    }

    // ---- VK_NV_fragment_shading_rate_enums extension commands
    if (!strcmp("vkCmdSetFragmentShadingRateEnumNV", name)) {
        *addr = (void *)CmdSetFragmentShadingRateEnumNV;
        return true;
    }

    // ---- VK_EXT_device_fault extension commands
    if (!strcmp("vkGetDeviceFaultInfoEXT", name)) {
        *addr = (void *)GetDeviceFaultInfoEXT;
        return true;
    }

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkAcquireWinrtDisplayNV", name)) {
        *addr = (void *)AcquireWinrtDisplayNV;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp("vkGetWinrtDisplayNV", name)) {
        *addr = (void *)GetWinrtDisplayNV;
        return true;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_vertex_input_dynamic_state extension commands
    if (!strcmp("vkCmdSetVertexInputEXT", name)) {
        *addr = (void *)CmdSetVertexInputEXT;
        return true;
    }

    // ---- VK_FUCHSIA_external_memory extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkGetMemoryZirconHandleFUCHSIA", name)) {
        *addr = (void *)GetMemoryZirconHandleFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkGetMemoryZirconHandlePropertiesFUCHSIA", name)) {
        *addr = (void *)GetMemoryZirconHandlePropertiesFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_external_semaphore extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkImportSemaphoreZirconHandleFUCHSIA", name)) {
        *addr = (void *)ImportSemaphoreZirconHandleFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkGetSemaphoreZirconHandleFUCHSIA", name)) {
        *addr = (void *)GetSemaphoreZirconHandleFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_FUCHSIA_buffer_collection extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkCreateBufferCollectionFUCHSIA", name)) {
        *addr = (void *)CreateBufferCollectionFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkSetBufferCollectionImageConstraintsFUCHSIA", name)) {
        *addr = (void *)SetBufferCollectionImageConstraintsFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkSetBufferCollectionBufferConstraintsFUCHSIA", name)) {
        *addr = (void *)SetBufferCollectionBufferConstraintsFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkDestroyBufferCollectionFUCHSIA", name)) {
        *addr = (void *)DestroyBufferCollectionFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (!strcmp("vkGetBufferCollectionPropertiesFUCHSIA", name)) {
        *addr = (void *)GetBufferCollectionPropertiesFUCHSIA;
        return true;
    }
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_HUAWEI_subpass_shading extension commands
    if (!strcmp("vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", name)) {
        *addr = (void *)GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;
        return true;
    }
    if (!strcmp("vkCmdSubpassShadingHUAWEI", name)) {
        *addr = (void *)CmdSubpassShadingHUAWEI;
        return true;
    }

    // ---- VK_HUAWEI_invocation_mask extension commands
    if (!strcmp("vkCmdBindInvocationMaskHUAWEI", name)) {
        *addr = (void *)CmdBindInvocationMaskHUAWEI;
        return true;
    }

    // ---- VK_NV_external_memory_rdma extension commands
    if (!strcmp("vkGetMemoryRemoteAddressNV", name)) {
        *addr = (void *)GetMemoryRemoteAddressNV;
        return true;
    }

    // ---- VK_EXT_pipeline_properties extension commands
    if (!strcmp("vkGetPipelinePropertiesEXT", name)) {
        *addr = (void *)GetPipelinePropertiesEXT;
        return true;
    }

    // ---- VK_EXT_extended_dynamic_state2 extension commands
    if (!strcmp("vkCmdSetPatchControlPointsEXT", name)) {
        *addr = (void *)CmdSetPatchControlPointsEXT;
        return true;
    }
    if (!strcmp("vkCmdSetRasterizerDiscardEnableEXT", name)) {
        *addr = (void *)CmdSetRasterizerDiscardEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthBiasEnableEXT", name)) {
        *addr = (void *)CmdSetDepthBiasEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetLogicOpEXT", name)) {
        *addr = (void *)CmdSetLogicOpEXT;
        return true;
    }
    if (!strcmp("vkCmdSetPrimitiveRestartEnableEXT", name)) {
        *addr = (void *)CmdSetPrimitiveRestartEnableEXT;
        return true;
    }

    // ---- VK_EXT_color_write_enable extension commands
    if (!strcmp("vkCmdSetColorWriteEnableEXT", name)) {
        *addr = (void *)CmdSetColorWriteEnableEXT;
        return true;
    }

    // ---- VK_EXT_multi_draw extension commands
    if (!strcmp("vkCmdDrawMultiEXT", name)) {
        *addr = (void *)CmdDrawMultiEXT;
        return true;
    }
    if (!strcmp("vkCmdDrawMultiIndexedEXT", name)) {
        *addr = (void *)CmdDrawMultiIndexedEXT;
        return true;
    }

    // ---- VK_EXT_opacity_micromap extension commands
    if (!strcmp("vkCreateMicromapEXT", name)) {
        *addr = (void *)CreateMicromapEXT;
        return true;
    }
    if (!strcmp("vkDestroyMicromapEXT", name)) {
        *addr = (void *)DestroyMicromapEXT;
        return true;
    }
    if (!strcmp("vkCmdBuildMicromapsEXT", name)) {
        *addr = (void *)CmdBuildMicromapsEXT;
        return true;
    }
    if (!strcmp("vkBuildMicromapsEXT", name)) {
        *addr = (void *)BuildMicromapsEXT;
        return true;
    }
    if (!strcmp("vkCopyMicromapEXT", name)) {
        *addr = (void *)CopyMicromapEXT;
        return true;
    }
    if (!strcmp("vkCopyMicromapToMemoryEXT", name)) {
        *addr = (void *)CopyMicromapToMemoryEXT;
        return true;
    }
    if (!strcmp("vkCopyMemoryToMicromapEXT", name)) {
        *addr = (void *)CopyMemoryToMicromapEXT;
        return true;
    }
    if (!strcmp("vkWriteMicromapsPropertiesEXT", name)) {
        *addr = (void *)WriteMicromapsPropertiesEXT;
        return true;
    }
    if (!strcmp("vkCmdCopyMicromapEXT", name)) {
        *addr = (void *)CmdCopyMicromapEXT;
        return true;
    }
    if (!strcmp("vkCmdCopyMicromapToMemoryEXT", name)) {
        *addr = (void *)CmdCopyMicromapToMemoryEXT;
        return true;
    }
    if (!strcmp("vkCmdCopyMemoryToMicromapEXT", name)) {
        *addr = (void *)CmdCopyMemoryToMicromapEXT;
        return true;
    }
    if (!strcmp("vkCmdWriteMicromapsPropertiesEXT", name)) {
        *addr = (void *)CmdWriteMicromapsPropertiesEXT;
        return true;
    }
    if (!strcmp("vkGetDeviceMicromapCompatibilityEXT", name)) {
        *addr = (void *)GetDeviceMicromapCompatibilityEXT;
        return true;
    }
    if (!strcmp("vkGetMicromapBuildSizesEXT", name)) {
        *addr = (void *)GetMicromapBuildSizesEXT;
        return true;
    }

    // ---- VK_HUAWEI_cluster_culling_shader extension commands
    if (!strcmp("vkCmdDrawClusterHUAWEI", name)) {
        *addr = (void *)CmdDrawClusterHUAWEI;
        return true;
    }
    if (!strcmp("vkCmdDrawClusterIndirectHUAWEI", name)) {
        *addr = (void *)CmdDrawClusterIndirectHUAWEI;
        return true;
    }

    // ---- VK_EXT_pageable_device_local_memory extension commands
    if (!strcmp("vkSetDeviceMemoryPriorityEXT", name)) {
        *addr = (void *)SetDeviceMemoryPriorityEXT;
        return true;
    }

    // ---- VK_VALVE_descriptor_set_host_mapping extension commands
    if (!strcmp("vkGetDescriptorSetLayoutHostMappingInfoVALVE", name)) {
        *addr = (void *)GetDescriptorSetLayoutHostMappingInfoVALVE;
        return true;
    }
    if (!strcmp("vkGetDescriptorSetHostMappingVALVE", name)) {
        *addr = (void *)GetDescriptorSetHostMappingVALVE;
        return true;
    }

    // ---- VK_NV_copy_memory_indirect extension commands
    if (!strcmp("vkCmdCopyMemoryIndirectNV", name)) {
        *addr = (void *)CmdCopyMemoryIndirectNV;
        return true;
    }
    if (!strcmp("vkCmdCopyMemoryToImageIndirectNV", name)) {
        *addr = (void *)CmdCopyMemoryToImageIndirectNV;
        return true;
    }

    // ---- VK_NV_memory_decompression extension commands
    if (!strcmp("vkCmdDecompressMemoryNV", name)) {
        *addr = (void *)CmdDecompressMemoryNV;
        return true;
    }
    if (!strcmp("vkCmdDecompressMemoryIndirectCountNV", name)) {
        *addr = (void *)CmdDecompressMemoryIndirectCountNV;
        return true;
    }

    // ---- VK_NV_device_generated_commands_compute extension commands
    if (!strcmp("vkGetPipelineIndirectMemoryRequirementsNV", name)) {
        *addr = (void *)GetPipelineIndirectMemoryRequirementsNV;
        return true;
    }
    if (!strcmp("vkCmdUpdatePipelineIndirectBufferNV", name)) {
        *addr = (void *)CmdUpdatePipelineIndirectBufferNV;
        return true;
    }
    if (!strcmp("vkGetPipelineIndirectDeviceAddressNV", name)) {
        *addr = (void *)GetPipelineIndirectDeviceAddressNV;
        return true;
    }

    // ---- VK_EXT_extended_dynamic_state3 extension commands
    if (!strcmp("vkCmdSetDepthClampEnableEXT", name)) {
        *addr = (void *)CmdSetDepthClampEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetPolygonModeEXT", name)) {
        *addr = (void *)CmdSetPolygonModeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetRasterizationSamplesEXT", name)) {
        *addr = (void *)CmdSetRasterizationSamplesEXT;
        return true;
    }
    if (!strcmp("vkCmdSetSampleMaskEXT", name)) {
        *addr = (void *)CmdSetSampleMaskEXT;
        return true;
    }
    if (!strcmp("vkCmdSetAlphaToCoverageEnableEXT", name)) {
        *addr = (void *)CmdSetAlphaToCoverageEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetAlphaToOneEnableEXT", name)) {
        *addr = (void *)CmdSetAlphaToOneEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetLogicOpEnableEXT", name)) {
        *addr = (void *)CmdSetLogicOpEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetColorBlendEnableEXT", name)) {
        *addr = (void *)CmdSetColorBlendEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetColorBlendEquationEXT", name)) {
        *addr = (void *)CmdSetColorBlendEquationEXT;
        return true;
    }
    if (!strcmp("vkCmdSetColorWriteMaskEXT", name)) {
        *addr = (void *)CmdSetColorWriteMaskEXT;
        return true;
    }
    if (!strcmp("vkCmdSetTessellationDomainOriginEXT", name)) {
        *addr = (void *)CmdSetTessellationDomainOriginEXT;
        return true;
    }
    if (!strcmp("vkCmdSetRasterizationStreamEXT", name)) {
        *addr = (void *)CmdSetRasterizationStreamEXT;
        return true;
    }
    if (!strcmp("vkCmdSetConservativeRasterizationModeEXT", name)) {
        *addr = (void *)CmdSetConservativeRasterizationModeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetExtraPrimitiveOverestimationSizeEXT", name)) {
        *addr = (void *)CmdSetExtraPrimitiveOverestimationSizeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthClipEnableEXT", name)) {
        *addr = (void *)CmdSetDepthClipEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetSampleLocationsEnableEXT", name)) {
        *addr = (void *)CmdSetSampleLocationsEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetColorBlendAdvancedEXT", name)) {
        *addr = (void *)CmdSetColorBlendAdvancedEXT;
        return true;
    }
    if (!strcmp("vkCmdSetProvokingVertexModeEXT", name)) {
        *addr = (void *)CmdSetProvokingVertexModeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetLineRasterizationModeEXT", name)) {
        *addr = (void *)CmdSetLineRasterizationModeEXT;
        return true;
    }
    if (!strcmp("vkCmdSetLineStippleEnableEXT", name)) {
        *addr = (void *)CmdSetLineStippleEnableEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthClipNegativeOneToOneEXT", name)) {
        *addr = (void *)CmdSetDepthClipNegativeOneToOneEXT;
        return true;
    }
    if (!strcmp("vkCmdSetViewportWScalingEnableNV", name)) {
        *addr = (void *)CmdSetViewportWScalingEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetViewportSwizzleNV", name)) {
        *addr = (void *)CmdSetViewportSwizzleNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageToColorEnableNV", name)) {
        *addr = (void *)CmdSetCoverageToColorEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageToColorLocationNV", name)) {
        *addr = (void *)CmdSetCoverageToColorLocationNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageModulationModeNV", name)) {
        *addr = (void *)CmdSetCoverageModulationModeNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageModulationTableEnableNV", name)) {
        *addr = (void *)CmdSetCoverageModulationTableEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageModulationTableNV", name)) {
        *addr = (void *)CmdSetCoverageModulationTableNV;
        return true;
    }
    if (!strcmp("vkCmdSetShadingRateImageEnableNV", name)) {
        *addr = (void *)CmdSetShadingRateImageEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetRepresentativeFragmentTestEnableNV", name)) {
        *addr = (void *)CmdSetRepresentativeFragmentTestEnableNV;
        return true;
    }
    if (!strcmp("vkCmdSetCoverageReductionModeNV", name)) {
        *addr = (void *)CmdSetCoverageReductionModeNV;
        return true;
    }

    // ---- VK_EXT_shader_module_identifier extension commands
    if (!strcmp("vkGetShaderModuleIdentifierEXT", name)) {
        *addr = (void *)GetShaderModuleIdentifierEXT;
        return true;
    }
    if (!strcmp("vkGetShaderModuleCreateInfoIdentifierEXT", name)) {
        *addr = (void *)GetShaderModuleCreateInfoIdentifierEXT;
        return true;
    }

    // ---- VK_NV_optical_flow extension commands
    if (!strcmp("vkGetPhysicalDeviceOpticalFlowImageFormatsNV", name)) {
        *addr = (void *)GetPhysicalDeviceOpticalFlowImageFormatsNV;
        return true;
    }
    if (!strcmp("vkCreateOpticalFlowSessionNV", name)) {
        *addr = (void *)CreateOpticalFlowSessionNV;
        return true;
    }
    if (!strcmp("vkDestroyOpticalFlowSessionNV", name)) {
        *addr = (void *)DestroyOpticalFlowSessionNV;
        return true;
    }
    if (!strcmp("vkBindOpticalFlowSessionImageNV", name)) {
        *addr = (void *)BindOpticalFlowSessionImageNV;
        return true;
    }
    if (!strcmp("vkCmdOpticalFlowExecuteNV", name)) {
        *addr = (void *)CmdOpticalFlowExecuteNV;
        return true;
    }

    // ---- VK_AMD_anti_lag extension commands
    if (!strcmp("vkAntiLagUpdateAMD", name)) {
        *addr = (void *)AntiLagUpdateAMD;
        return true;
    }

    // ---- VK_EXT_shader_object extension commands
    if (!strcmp("vkCreateShadersEXT", name)) {
        *addr = (void *)CreateShadersEXT;
        return true;
    }
    if (!strcmp("vkDestroyShaderEXT", name)) {
        *addr = (void *)DestroyShaderEXT;
        return true;
    }
    if (!strcmp("vkGetShaderBinaryDataEXT", name)) {
        *addr = (void *)GetShaderBinaryDataEXT;
        return true;
    }
    if (!strcmp("vkCmdBindShadersEXT", name)) {
        *addr = (void *)CmdBindShadersEXT;
        return true;
    }
    if (!strcmp("vkCmdSetDepthClampRangeEXT", name)) {
        *addr = (void *)CmdSetDepthClampRangeEXT;
        return true;
    }

    // ---- VK_QCOM_tile_properties extension commands
    if (!strcmp("vkGetFramebufferTilePropertiesQCOM", name)) {
        *addr = (void *)GetFramebufferTilePropertiesQCOM;
        return true;
    }
    if (!strcmp("vkGetDynamicRenderingTilePropertiesQCOM", name)) {
        *addr = (void *)GetDynamicRenderingTilePropertiesQCOM;
        return true;
    }

    // ---- VK_NV_cooperative_vector extension commands
    if (!strcmp("vkGetPhysicalDeviceCooperativeVectorPropertiesNV", name)) {
        *addr = (void *)GetPhysicalDeviceCooperativeVectorPropertiesNV;
        return true;
    }
    if (!strcmp("vkConvertCooperativeVectorMatrixNV", name)) {
        *addr = (void *)ConvertCooperativeVectorMatrixNV;
        return true;
    }
    if (!strcmp("vkCmdConvertCooperativeVectorMatrixNV", name)) {
        *addr = (void *)CmdConvertCooperativeVectorMatrixNV;
        return true;
    }

    // ---- VK_NV_low_latency2 extension commands
    if (!strcmp("vkSetLatencySleepModeNV", name)) {
        *addr = (void *)SetLatencySleepModeNV;
        return true;
    }
    if (!strcmp("vkLatencySleepNV", name)) {
        *addr = (void *)LatencySleepNV;
        return true;
    }
    if (!strcmp("vkSetLatencyMarkerNV", name)) {
        *addr = (void *)SetLatencyMarkerNV;
        return true;
    }
    if (!strcmp("vkGetLatencyTimingsNV", name)) {
        *addr = (void *)GetLatencyTimingsNV;
        return true;
    }
    if (!strcmp("vkQueueNotifyOutOfBandNV", name)) {
        *addr = (void *)QueueNotifyOutOfBandNV;
        return true;
    }

    // ---- VK_EXT_attachment_feedback_loop_dynamic_state extension commands
    if (!strcmp("vkCmdSetAttachmentFeedbackLoopEnableEXT", name)) {
        *addr = (void *)CmdSetAttachmentFeedbackLoopEnableEXT;
        return true;
    }

    // ---- VK_QNX_external_memory_screen_buffer extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (!strcmp("vkGetScreenBufferPropertiesQNX", name)) {
        *addr = (void *)GetScreenBufferPropertiesQNX;
        return true;
    }
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_cluster_acceleration_structure extension commands
    if (!strcmp("vkGetClusterAccelerationStructureBuildSizesNV", name)) {
        *addr = (void *)GetClusterAccelerationStructureBuildSizesNV;
        return true;
    }
    if (!strcmp("vkCmdBuildClusterAccelerationStructureIndirectNV", name)) {
        *addr = (void *)CmdBuildClusterAccelerationStructureIndirectNV;
        return true;
    }

    // ---- VK_NV_partitioned_acceleration_structure extension commands
    if (!strcmp("vkGetPartitionedAccelerationStructuresBuildSizesNV", name)) {
        *addr = (void *)GetPartitionedAccelerationStructuresBuildSizesNV;
        return true;
    }
    if (!strcmp("vkCmdBuildPartitionedAccelerationStructuresNV", name)) {
        *addr = (void *)CmdBuildPartitionedAccelerationStructuresNV;
        return true;
    }

    // ---- VK_EXT_device_generated_commands extension commands
    if (!strcmp("vkGetGeneratedCommandsMemoryRequirementsEXT", name)) {
        *addr = (void *)GetGeneratedCommandsMemoryRequirementsEXT;
        return true;
    }
    if (!strcmp("vkCmdPreprocessGeneratedCommandsEXT", name)) {
        *addr = (void *)CmdPreprocessGeneratedCommandsEXT;
        return true;
    }
    if (!strcmp("vkCmdExecuteGeneratedCommandsEXT", name)) {
        *addr = (void *)CmdExecuteGeneratedCommandsEXT;
        return true;
    }
    if (!strcmp("vkCreateIndirectCommandsLayoutEXT", name)) {
        *addr = (void *)CreateIndirectCommandsLayoutEXT;
        return true;
    }
    if (!strcmp("vkDestroyIndirectCommandsLayoutEXT", name)) {
        *addr = (void *)DestroyIndirectCommandsLayoutEXT;
        return true;
    }
    if (!strcmp("vkCreateIndirectExecutionSetEXT", name)) {
        *addr = (void *)CreateIndirectExecutionSetEXT;
        return true;
    }
    if (!strcmp("vkDestroyIndirectExecutionSetEXT", name)) {
        *addr = (void *)DestroyIndirectExecutionSetEXT;
        return true;
    }
    if (!strcmp("vkUpdateIndirectExecutionSetPipelineEXT", name)) {
        *addr = (void *)UpdateIndirectExecutionSetPipelineEXT;
        return true;
    }
    if (!strcmp("vkUpdateIndirectExecutionSetShaderEXT", name)) {
        *addr = (void *)UpdateIndirectExecutionSetShaderEXT;
        return true;
    }

    // ---- VK_NV_cooperative_matrix2 extension commands
    if (!strcmp("vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV", name)) {
        *addr = (void *)GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV;
        return true;
    }

    // ---- VK_EXT_external_memory_metal extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp("vkGetMemoryMetalHandleEXT", name)) {
        *addr = (void *)GetMemoryMetalHandleEXT;
        return true;
    }
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp("vkGetMemoryMetalHandlePropertiesEXT", name)) {
        *addr = (void *)GetMemoryMetalHandlePropertiesEXT;
        return true;
    }
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_KHR_acceleration_structure extension commands
    if (!strcmp("vkCreateAccelerationStructureKHR", name)) {
        *addr = (void *)CreateAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkDestroyAccelerationStructureKHR", name)) {
        *addr = (void *)DestroyAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkCmdBuildAccelerationStructuresKHR", name)) {
        *addr = (void *)CmdBuildAccelerationStructuresKHR;
        return true;
    }
    if (!strcmp("vkCmdBuildAccelerationStructuresIndirectKHR", name)) {
        *addr = (void *)CmdBuildAccelerationStructuresIndirectKHR;
        return true;
    }
    if (!strcmp("vkBuildAccelerationStructuresKHR", name)) {
        *addr = (void *)BuildAccelerationStructuresKHR;
        return true;
    }
    if (!strcmp("vkCopyAccelerationStructureKHR", name)) {
        *addr = (void *)CopyAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkCopyAccelerationStructureToMemoryKHR", name)) {
        *addr = (void *)CopyAccelerationStructureToMemoryKHR;
        return true;
    }
    if (!strcmp("vkCopyMemoryToAccelerationStructureKHR", name)) {
        *addr = (void *)CopyMemoryToAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkWriteAccelerationStructuresPropertiesKHR", name)) {
        *addr = (void *)WriteAccelerationStructuresPropertiesKHR;
        return true;
    }
    if (!strcmp("vkCmdCopyAccelerationStructureKHR", name)) {
        *addr = (void *)CmdCopyAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkCmdCopyAccelerationStructureToMemoryKHR", name)) {
        *addr = (void *)CmdCopyAccelerationStructureToMemoryKHR;
        return true;
    }
    if (!strcmp("vkCmdCopyMemoryToAccelerationStructureKHR", name)) {
        *addr = (void *)CmdCopyMemoryToAccelerationStructureKHR;
        return true;
    }
    if (!strcmp("vkGetAccelerationStructureDeviceAddressKHR", name)) {
        *addr = (void *)GetAccelerationStructureDeviceAddressKHR;
        return true;
    }
    if (!strcmp("vkCmdWriteAccelerationStructuresPropertiesKHR", name)) {
        *addr = (void *)CmdWriteAccelerationStructuresPropertiesKHR;
        return true;
    }
    if (!strcmp("vkGetDeviceAccelerationStructureCompatibilityKHR", name)) {
        *addr = (void *)GetDeviceAccelerationStructureCompatibilityKHR;
        return true;
    }
    if (!strcmp("vkGetAccelerationStructureBuildSizesKHR", name)) {
        *addr = (void *)GetAccelerationStructureBuildSizesKHR;
        return true;
    }

    // ---- VK_KHR_ray_tracing_pipeline extension commands
    if (!strcmp("vkCmdTraceRaysKHR", name)) {
        *addr = (void *)CmdTraceRaysKHR;
        return true;
    }
    if (!strcmp("vkCreateRayTracingPipelinesKHR", name)) {
        *addr = (void *)CreateRayTracingPipelinesKHR;
        return true;
    }
    if (!strcmp("vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", name)) {
        *addr = (void *)GetRayTracingCaptureReplayShaderGroupHandlesKHR;
        return true;
    }
    if (!strcmp("vkCmdTraceRaysIndirectKHR", name)) {
        *addr = (void *)CmdTraceRaysIndirectKHR;
        return true;
    }
    if (!strcmp("vkGetRayTracingShaderGroupStackSizeKHR", name)) {
        *addr = (void *)GetRayTracingShaderGroupStackSizeKHR;
        return true;
    }
    if (!strcmp("vkCmdSetRayTracingPipelineStackSizeKHR", name)) {
        *addr = (void *)CmdSetRayTracingPipelineStackSizeKHR;
        return true;
    }

    // ---- VK_EXT_mesh_shader extension commands
    if (!strcmp("vkCmdDrawMeshTasksEXT", name)) {
        *addr = (void *)CmdDrawMeshTasksEXT;
        return true;
    }
    if (!strcmp("vkCmdDrawMeshTasksIndirectEXT", name)) {
        *addr = (void *)CmdDrawMeshTasksIndirectEXT;
        return true;
    }
    if (!strcmp("vkCmdDrawMeshTasksIndirectCountEXT", name)) {
        *addr = (void *)CmdDrawMeshTasksIndirectCountEXT;
        return true;
    }
    return false;
}

// A function that can be used to query enabled extensions during a vkCreateInstance call
void extensions_create_instance(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {

    // ---- VK_KHR_get_physical_device_properties2 extension commands
        if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.khr_get_physical_device_properties2 = 1;

    // ---- VK_KHR_device_group_creation extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.khr_device_group_creation = 1;

    // ---- VK_KHR_external_memory_capabilities extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.khr_external_memory_capabilities = 1;

    // ---- VK_KHR_external_semaphore_capabilities extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.khr_external_semaphore_capabilities = 1;

    // ---- VK_KHR_external_fence_capabilities extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.khr_external_fence_capabilities = 1;

    // ---- VK_NV_external_memory_capabilities extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.nv_external_memory_capabilities = 1;

    // ---- VK_EXT_direct_mode_display extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.ext_direct_mode_display = 1;

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.ext_acquire_xlib_display = 1;
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.ext_display_surface_counter = 1;

    // ---- VK_EXT_debug_utils extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.ext_debug_utils = 1;

    // ---- VK_EXT_acquire_drm_display extension commands
        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME)) {
            ptr_instance->enabled_known_extensions.ext_acquire_drm_display = 1;
        }
    }
}

// Some device commands still need a terminator because the loader needs to unwrap something about them.
// In many cases, the item needing unwrapping is a VkPhysicalDevice or VkSurfaceKHR object.  But there may be other items
// in the future.
PFN_vkVoidFunction get_extension_device_proc_terminator(struct loader_device *dev, const char *name, bool* found_name) {
    *found_name = false;
    if (!name || name[0] != 'v' || name[1] != 'k') {
        return NULL;
    }
    name += 2;
    // ---- VK_KHR_swapchain extension commands
    if (!strcmp(name, "CreateSwapchainKHR")) {
        *found_name = true;
        return dev->driver_extensions.khr_swapchain_enabled ?
            (PFN_vkVoidFunction)terminator_CreateSwapchainKHR : NULL;
    }
    if (!strcmp(name, "GetDeviceGroupSurfacePresentModesKHR")) {
        *found_name = true;
        return dev->driver_extensions.khr_swapchain_enabled ?
            (PFN_vkVoidFunction)terminator_GetDeviceGroupSurfacePresentModesKHR : NULL;
    }
    // ---- VK_KHR_display_swapchain extension commands
    if (!strcmp(name, "CreateSharedSwapchainsKHR")) {
        *found_name = true;
        return dev->driver_extensions.khr_display_swapchain_enabled ?
            (PFN_vkVoidFunction)terminator_CreateSharedSwapchainsKHR : NULL;
    }
    // ---- VK_EXT_debug_marker extension commands
    if (!strcmp(name, "DebugMarkerSetObjectTagEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_marker_enabled ?
            (PFN_vkVoidFunction)terminator_DebugMarkerSetObjectTagEXT : NULL;
    }
    if (!strcmp(name, "DebugMarkerSetObjectNameEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_marker_enabled ?
            (PFN_vkVoidFunction)terminator_DebugMarkerSetObjectNameEXT : NULL;
    }
    // ---- VK_EXT_debug_utils extension commands
    if (!strcmp(name, "SetDebugUtilsObjectNameEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_SetDebugUtilsObjectNameEXT : NULL;
    }
    if (!strcmp(name, "SetDebugUtilsObjectTagEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_SetDebugUtilsObjectTagEXT : NULL;
    }
    if (!strcmp(name, "QueueBeginDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_QueueBeginDebugUtilsLabelEXT : NULL;
    }
    if (!strcmp(name, "QueueEndDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_QueueEndDebugUtilsLabelEXT : NULL;
    }
    if (!strcmp(name, "QueueInsertDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_QueueInsertDebugUtilsLabelEXT : NULL;
    }
    if (!strcmp(name, "CmdBeginDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_CmdBeginDebugUtilsLabelEXT : NULL;
    }
    if (!strcmp(name, "CmdEndDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_CmdEndDebugUtilsLabelEXT : NULL;
    }
    if (!strcmp(name, "CmdInsertDebugUtilsLabelEXT")) {
        *found_name = true;
        return dev->driver_extensions.ext_debug_utils_enabled ?
            (PFN_vkVoidFunction)terminator_CmdInsertDebugUtilsLabelEXT : NULL;
    }
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    // ---- VK_EXT_full_screen_exclusive extension commands
    if (!strcmp(name, "GetDeviceGroupSurfacePresentModes2EXT")) {
        *found_name = true;
        return (dev->driver_extensions.ext_full_screen_exclusive_enabled && (dev->driver_extensions.khr_device_group_enabled || dev->driver_extensions.version_1_1_enabled)) ?
            (PFN_vkVoidFunction)terminator_GetDeviceGroupSurfacePresentModes2EXT : NULL;
    }
#endif // VK_USE_PLATFORM_WIN32_KHR
    return NULL;
}

// This table contains the loader's instance dispatch table, which contains
// default functions if no instance layers are activated.  This contains
// pointers to "terminator functions".
const VkLayerInstanceDispatchTable instance_disp = {

    // ---- Core Vulkan 1.0 commands
    .DestroyInstance = terminator_DestroyInstance,
    .EnumeratePhysicalDevices = terminator_EnumeratePhysicalDevices,
    .GetPhysicalDeviceFeatures = terminator_GetPhysicalDeviceFeatures,
    .GetPhysicalDeviceFormatProperties = terminator_GetPhysicalDeviceFormatProperties,
    .GetPhysicalDeviceImageFormatProperties = terminator_GetPhysicalDeviceImageFormatProperties,
    .GetPhysicalDeviceProperties = terminator_GetPhysicalDeviceProperties,
    .GetPhysicalDeviceQueueFamilyProperties = terminator_GetPhysicalDeviceQueueFamilyProperties,
    .GetPhysicalDeviceMemoryProperties = terminator_GetPhysicalDeviceMemoryProperties,
    .GetInstanceProcAddr = vkGetInstanceProcAddr,
    .EnumerateDeviceExtensionProperties = terminator_EnumerateDeviceExtensionProperties,
    .EnumerateDeviceLayerProperties = terminator_EnumerateDeviceLayerProperties,
    .GetPhysicalDeviceSparseImageFormatProperties = terminator_GetPhysicalDeviceSparseImageFormatProperties,

    // ---- Core Vulkan 1.1 commands
    .EnumeratePhysicalDeviceGroups = terminator_EnumeratePhysicalDeviceGroups,
    .GetPhysicalDeviceFeatures2 = terminator_GetPhysicalDeviceFeatures2,
    .GetPhysicalDeviceProperties2 = terminator_GetPhysicalDeviceProperties2,
    .GetPhysicalDeviceFormatProperties2 = terminator_GetPhysicalDeviceFormatProperties2,
    .GetPhysicalDeviceImageFormatProperties2 = terminator_GetPhysicalDeviceImageFormatProperties2,
    .GetPhysicalDeviceQueueFamilyProperties2 = terminator_GetPhysicalDeviceQueueFamilyProperties2,
    .GetPhysicalDeviceMemoryProperties2 = terminator_GetPhysicalDeviceMemoryProperties2,
    .GetPhysicalDeviceSparseImageFormatProperties2 = terminator_GetPhysicalDeviceSparseImageFormatProperties2,
    .GetPhysicalDeviceExternalBufferProperties = terminator_GetPhysicalDeviceExternalBufferProperties,
    .GetPhysicalDeviceExternalFenceProperties = terminator_GetPhysicalDeviceExternalFenceProperties,
    .GetPhysicalDeviceExternalSemaphoreProperties = terminator_GetPhysicalDeviceExternalSemaphoreProperties,

    // ---- Core Vulkan 1.3 commands
    .GetPhysicalDeviceToolProperties = terminator_GetPhysicalDeviceToolProperties,

    // ---- VK_KHR_surface extension commands
    .DestroySurfaceKHR = terminator_DestroySurfaceKHR,
    .GetPhysicalDeviceSurfaceSupportKHR = terminator_GetPhysicalDeviceSurfaceSupportKHR,
    .GetPhysicalDeviceSurfaceCapabilitiesKHR = terminator_GetPhysicalDeviceSurfaceCapabilitiesKHR,
    .GetPhysicalDeviceSurfaceFormatsKHR = terminator_GetPhysicalDeviceSurfaceFormatsKHR,
    .GetPhysicalDeviceSurfacePresentModesKHR = terminator_GetPhysicalDeviceSurfacePresentModesKHR,

    // ---- VK_KHR_swapchain extension commands
    .GetPhysicalDevicePresentRectanglesKHR = terminator_GetPhysicalDevicePresentRectanglesKHR,

    // ---- VK_KHR_display extension commands
    .GetPhysicalDeviceDisplayPropertiesKHR = terminator_GetPhysicalDeviceDisplayPropertiesKHR,
    .GetPhysicalDeviceDisplayPlanePropertiesKHR = terminator_GetPhysicalDeviceDisplayPlanePropertiesKHR,
    .GetDisplayPlaneSupportedDisplaysKHR = terminator_GetDisplayPlaneSupportedDisplaysKHR,
    .GetDisplayModePropertiesKHR = terminator_GetDisplayModePropertiesKHR,
    .CreateDisplayModeKHR = terminator_CreateDisplayModeKHR,
    .GetDisplayPlaneCapabilitiesKHR = terminator_GetDisplayPlaneCapabilitiesKHR,
    .CreateDisplayPlaneSurfaceKHR = terminator_CreateDisplayPlaneSurfaceKHR,

    // ---- VK_KHR_xlib_surface extension commands
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    .CreateXlibSurfaceKHR = terminator_CreateXlibSurfaceKHR,
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    .GetPhysicalDeviceXlibPresentationSupportKHR = terminator_GetPhysicalDeviceXlibPresentationSupportKHR,
#endif // VK_USE_PLATFORM_XLIB_KHR

    // ---- VK_KHR_xcb_surface extension commands
#if defined(VK_USE_PLATFORM_XCB_KHR)
    .CreateXcbSurfaceKHR = terminator_CreateXcbSurfaceKHR,
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    .GetPhysicalDeviceXcbPresentationSupportKHR = terminator_GetPhysicalDeviceXcbPresentationSupportKHR,
#endif // VK_USE_PLATFORM_XCB_KHR

    // ---- VK_KHR_wayland_surface extension commands
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    .CreateWaylandSurfaceKHR = terminator_CreateWaylandSurfaceKHR,
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    .GetPhysicalDeviceWaylandPresentationSupportKHR = terminator_GetPhysicalDeviceWaylandPresentationSupportKHR,
#endif // VK_USE_PLATFORM_WAYLAND_KHR

    // ---- VK_KHR_android_surface extension commands
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    .CreateAndroidSurfaceKHR = terminator_CreateAndroidSurfaceKHR,
#endif // VK_USE_PLATFORM_ANDROID_KHR

    // ---- VK_KHR_win32_surface extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    .CreateWin32SurfaceKHR = terminator_CreateWin32SurfaceKHR,
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    .GetPhysicalDeviceWin32PresentationSupportKHR = terminator_GetPhysicalDeviceWin32PresentationSupportKHR,
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_KHR_video_queue extension commands
    .GetPhysicalDeviceVideoCapabilitiesKHR = terminator_GetPhysicalDeviceVideoCapabilitiesKHR,
    .GetPhysicalDeviceVideoFormatPropertiesKHR = terminator_GetPhysicalDeviceVideoFormatPropertiesKHR,

    // ---- VK_KHR_get_physical_device_properties2 extension commands
    .GetPhysicalDeviceFeatures2KHR = terminator_GetPhysicalDeviceFeatures2,
    .GetPhysicalDeviceProperties2KHR = terminator_GetPhysicalDeviceProperties2,
    .GetPhysicalDeviceFormatProperties2KHR = terminator_GetPhysicalDeviceFormatProperties2,
    .GetPhysicalDeviceImageFormatProperties2KHR = terminator_GetPhysicalDeviceImageFormatProperties2,
    .GetPhysicalDeviceQueueFamilyProperties2KHR = terminator_GetPhysicalDeviceQueueFamilyProperties2,
    .GetPhysicalDeviceMemoryProperties2KHR = terminator_GetPhysicalDeviceMemoryProperties2,
    .GetPhysicalDeviceSparseImageFormatProperties2KHR = terminator_GetPhysicalDeviceSparseImageFormatProperties2,

    // ---- VK_KHR_device_group_creation extension commands
    .EnumeratePhysicalDeviceGroupsKHR = terminator_EnumeratePhysicalDeviceGroups,

    // ---- VK_KHR_external_memory_capabilities extension commands
    .GetPhysicalDeviceExternalBufferPropertiesKHR = terminator_GetPhysicalDeviceExternalBufferProperties,

    // ---- VK_KHR_external_semaphore_capabilities extension commands
    .GetPhysicalDeviceExternalSemaphorePropertiesKHR = terminator_GetPhysicalDeviceExternalSemaphoreProperties,

    // ---- VK_KHR_external_fence_capabilities extension commands
    .GetPhysicalDeviceExternalFencePropertiesKHR = terminator_GetPhysicalDeviceExternalFenceProperties,

    // ---- VK_KHR_performance_query extension commands
    .EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = terminator_EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR,
    .GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = terminator_GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR,

    // ---- VK_KHR_get_surface_capabilities2 extension commands
    .GetPhysicalDeviceSurfaceCapabilities2KHR = terminator_GetPhysicalDeviceSurfaceCapabilities2KHR,
    .GetPhysicalDeviceSurfaceFormats2KHR = terminator_GetPhysicalDeviceSurfaceFormats2KHR,

    // ---- VK_KHR_get_display_properties2 extension commands
    .GetPhysicalDeviceDisplayProperties2KHR = terminator_GetPhysicalDeviceDisplayProperties2KHR,
    .GetPhysicalDeviceDisplayPlaneProperties2KHR = terminator_GetPhysicalDeviceDisplayPlaneProperties2KHR,
    .GetDisplayModeProperties2KHR = terminator_GetDisplayModeProperties2KHR,
    .GetDisplayPlaneCapabilities2KHR = terminator_GetDisplayPlaneCapabilities2KHR,

    // ---- VK_KHR_fragment_shading_rate extension commands
    .GetPhysicalDeviceFragmentShadingRatesKHR = terminator_GetPhysicalDeviceFragmentShadingRatesKHR,

    // ---- VK_KHR_video_encode_queue extension commands
    .GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = terminator_GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR,

    // ---- VK_KHR_cooperative_matrix extension commands
    .GetPhysicalDeviceCooperativeMatrixPropertiesKHR = terminator_GetPhysicalDeviceCooperativeMatrixPropertiesKHR,

    // ---- VK_KHR_calibrated_timestamps extension commands
    .GetPhysicalDeviceCalibrateableTimeDomainsKHR = terminator_GetPhysicalDeviceCalibrateableTimeDomainsKHR,

    // ---- VK_EXT_debug_report extension commands
    .CreateDebugReportCallbackEXT = terminator_CreateDebugReportCallbackEXT,
    .DestroyDebugReportCallbackEXT = terminator_DestroyDebugReportCallbackEXT,
    .DebugReportMessageEXT = terminator_DebugReportMessageEXT,

    // ---- VK_GGP_stream_descriptor_surface extension commands
#if defined(VK_USE_PLATFORM_GGP)
    .CreateStreamDescriptorSurfaceGGP = terminator_CreateStreamDescriptorSurfaceGGP,
#endif // VK_USE_PLATFORM_GGP

    // ---- VK_NV_external_memory_capabilities extension commands
    .GetPhysicalDeviceExternalImageFormatPropertiesNV = terminator_GetPhysicalDeviceExternalImageFormatPropertiesNV,

    // ---- VK_NN_vi_surface extension commands
#if defined(VK_USE_PLATFORM_VI_NN)
    .CreateViSurfaceNN = terminator_CreateViSurfaceNN,
#endif // VK_USE_PLATFORM_VI_NN

    // ---- VK_EXT_direct_mode_display extension commands
    .ReleaseDisplayEXT = terminator_ReleaseDisplayEXT,

    // ---- VK_EXT_acquire_xlib_display extension commands
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    .AcquireXlibDisplayEXT = terminator_AcquireXlibDisplayEXT,
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    .GetRandROutputDisplayEXT = terminator_GetRandROutputDisplayEXT,
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    // ---- VK_EXT_display_surface_counter extension commands
    .GetPhysicalDeviceSurfaceCapabilities2EXT = terminator_GetPhysicalDeviceSurfaceCapabilities2EXT,

    // ---- VK_MVK_ios_surface extension commands
#if defined(VK_USE_PLATFORM_IOS_MVK)
    .CreateIOSSurfaceMVK = terminator_CreateIOSSurfaceMVK,
#endif // VK_USE_PLATFORM_IOS_MVK

    // ---- VK_MVK_macos_surface extension commands
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    .CreateMacOSSurfaceMVK = terminator_CreateMacOSSurfaceMVK,
#endif // VK_USE_PLATFORM_MACOS_MVK

    // ---- VK_EXT_debug_utils extension commands
    .CreateDebugUtilsMessengerEXT = terminator_CreateDebugUtilsMessengerEXT,
    .DestroyDebugUtilsMessengerEXT = terminator_DestroyDebugUtilsMessengerEXT,
    .SubmitDebugUtilsMessageEXT = terminator_SubmitDebugUtilsMessageEXT,

    // ---- VK_EXT_sample_locations extension commands
    .GetPhysicalDeviceMultisamplePropertiesEXT = terminator_GetPhysicalDeviceMultisamplePropertiesEXT,

    // ---- VK_EXT_calibrated_timestamps extension commands
    .GetPhysicalDeviceCalibrateableTimeDomainsEXT = terminator_GetPhysicalDeviceCalibrateableTimeDomainsEXT,

    // ---- VK_FUCHSIA_imagepipe_surface extension commands
#if defined(VK_USE_PLATFORM_FUCHSIA)
    .CreateImagePipeSurfaceFUCHSIA = terminator_CreateImagePipeSurfaceFUCHSIA,
#endif // VK_USE_PLATFORM_FUCHSIA

    // ---- VK_EXT_metal_surface extension commands
#if defined(VK_USE_PLATFORM_METAL_EXT)
    .CreateMetalSurfaceEXT = terminator_CreateMetalSurfaceEXT,
#endif // VK_USE_PLATFORM_METAL_EXT

    // ---- VK_EXT_tooling_info extension commands
    .GetPhysicalDeviceToolPropertiesEXT = terminator_GetPhysicalDeviceToolPropertiesEXT,

    // ---- VK_NV_cooperative_matrix extension commands
    .GetPhysicalDeviceCooperativeMatrixPropertiesNV = terminator_GetPhysicalDeviceCooperativeMatrixPropertiesNV,

    // ---- VK_NV_coverage_reduction_mode extension commands
    .GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = terminator_GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV,

    // ---- VK_EXT_full_screen_exclusive extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    .GetPhysicalDeviceSurfacePresentModes2EXT = terminator_GetPhysicalDeviceSurfacePresentModes2EXT,
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_headless_surface extension commands
    .CreateHeadlessSurfaceEXT = terminator_CreateHeadlessSurfaceEXT,

    // ---- VK_EXT_acquire_drm_display extension commands
    .AcquireDrmDisplayEXT = terminator_AcquireDrmDisplayEXT,
    .GetDrmDisplayEXT = terminator_GetDrmDisplayEXT,

    // ---- VK_NV_acquire_winrt_display extension commands
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    .AcquireWinrtDisplayNV = terminator_AcquireWinrtDisplayNV,
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    .GetWinrtDisplayNV = terminator_GetWinrtDisplayNV,
#endif // VK_USE_PLATFORM_WIN32_KHR

    // ---- VK_EXT_directfb_surface extension commands
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    .CreateDirectFBSurfaceEXT = terminator_CreateDirectFBSurfaceEXT,
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    .GetPhysicalDeviceDirectFBPresentationSupportEXT = terminator_GetPhysicalDeviceDirectFBPresentationSupportEXT,
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

    // ---- VK_QNX_screen_surface extension commands
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    .CreateScreenSurfaceQNX = terminator_CreateScreenSurfaceQNX,
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    .GetPhysicalDeviceScreenPresentationSupportQNX = terminator_GetPhysicalDeviceScreenPresentationSupportQNX,
#endif // VK_USE_PLATFORM_SCREEN_QNX

    // ---- VK_NV_optical_flow extension commands
    .GetPhysicalDeviceOpticalFlowImageFormatsNV = terminator_GetPhysicalDeviceOpticalFlowImageFormatsNV,

    // ---- VK_NV_cooperative_vector extension commands
    .GetPhysicalDeviceCooperativeVectorPropertiesNV = terminator_GetPhysicalDeviceCooperativeVectorPropertiesNV,

    // ---- VK_NV_cooperative_matrix2 extension commands
    .GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV = terminator_GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV,
};

// A null-terminated list of all of the instance extensions supported by the loader.
// If an instance extension name is not in this list, but it is exported by one or more of the
// ICDs detected by the loader, then the extension name not in the list will be filtered out
// before passing the list of extensions to the application.
const char *const LOADER_INSTANCE_EXTENSIONS[] = {
                                                  VK_KHR_SURFACE_EXTENSION_NAME,
                                                  VK_KHR_DISPLAY_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_XLIB_KHR)
                                                  VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
                                                  VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
                                                  VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
                                                  VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_WIN32_KHR
                                                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                  VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
                                                  VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
                                                  VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
                                                  VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME,
                                                  VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
                                                  VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
                                                  VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME,
                                                  VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                                                  VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_GGP)
                                                  VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_GGP
                                                  VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
                                                  VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_VI_NN)
                                                  VK_NN_VI_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_VI_NN
                                                  VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
                                                  VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
                                                  VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
                                                  VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_IOS_MVK)
                                                  VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
                                                  VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_MACOS_MVK
                                                  VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_FUCHSIA)
                                                  VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_METAL_EXT)
                                                  VK_EXT_METAL_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_METAL_EXT
                                                  VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
                                                  VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
                                                  VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
                                                  VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
                                                  VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
                                                  VK_QNX_SCREEN_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_SCREEN_QNX
                                                  VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME,
                                                  VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME,
                                                  VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
                                                  VK_NV_DISPLAY_STEREO_EXTENSION_NAME,
                                                  NULL };
// clang-format on
