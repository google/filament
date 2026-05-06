// Copyright 2015-2026 The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
//

// This header is generated from the Khronos Vulkan XML API Registry.

module;

#define VULKAN_HPP_CXX_MODULE 1

#include <cassert>
#include <cstring>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_hpp_macros.hpp>

#if !defined( VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING )
#  define VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING \
    "\n\tThe Vulkan-Hpp C++ named module is experimental. It is subject to change without prior notice.\n" \
  "\tTo silence this warning, define the VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING macro.\n" \
  "\tFor feedback, go to: https://github.com/KhronosGroup/Vulkan-Hpp/issues"

VULKAN_HPP_COMPILE_WARNING( VULKAN_HPP_CXX_MODULE_EXPERIMENTAL_WARNING )
#endif

export module vulkan;

export import std;

VULKAN_HPP_STATIC_ASSERT( VK_HEADER_VERSION == 349, "Wrong VK_HEADER_VERSION!" );

#if defined( _MSC_VER )
#  pragma warning( push )
#  pragma warning( disable : 5244 )
#elif defined( __clang__ )
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#elif defined( __GNUC__ )
#endif

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_extension_inspection.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_shared.hpp>

#if defined( _MSC_VER )
#  pragma warning( pop )
#elif defined( __clang__ )
#  pragma clang diagnostic pop
#elif defined( __GNUC__ )
#endif

export
{
  // This VkFlags type is used as part of a bitfield in some structures.
  // As it can't be mimicked by vk-data types, we need to export just that.
  using ::VkGeometryInstanceFlagsKHR;

  //==================
  //=== PFN TYPEs ===
  //==================

  //=== VK_VERSION_1_0 ===
  using ::PFN_vkAllocateCommandBuffers;
  using ::PFN_vkAllocateDescriptorSets;
  using ::PFN_vkAllocateMemory;
  using ::PFN_vkBeginCommandBuffer;
  using ::PFN_vkBindBufferMemory;
  using ::PFN_vkBindImageMemory;
  using ::PFN_vkCmdBeginQuery;
  using ::PFN_vkCmdBeginRenderPass;
  using ::PFN_vkCmdBindDescriptorSets;
  using ::PFN_vkCmdBindIndexBuffer;
  using ::PFN_vkCmdBindPipeline;
  using ::PFN_vkCmdBindVertexBuffers;
  using ::PFN_vkCmdBlitImage;
  using ::PFN_vkCmdClearAttachments;
  using ::PFN_vkCmdClearColorImage;
  using ::PFN_vkCmdClearDepthStencilImage;
  using ::PFN_vkCmdCopyBuffer;
  using ::PFN_vkCmdCopyBufferToImage;
  using ::PFN_vkCmdCopyImage;
  using ::PFN_vkCmdCopyImageToBuffer;
  using ::PFN_vkCmdCopyQueryPoolResults;
  using ::PFN_vkCmdDispatch;
  using ::PFN_vkCmdDispatchIndirect;
  using ::PFN_vkCmdDraw;
  using ::PFN_vkCmdDrawIndexed;
  using ::PFN_vkCmdDrawIndexedIndirect;
  using ::PFN_vkCmdDrawIndirect;
  using ::PFN_vkCmdEndQuery;
  using ::PFN_vkCmdEndRenderPass;
  using ::PFN_vkCmdExecuteCommands;
  using ::PFN_vkCmdFillBuffer;
  using ::PFN_vkCmdNextSubpass;
  using ::PFN_vkCmdPipelineBarrier;
  using ::PFN_vkCmdPushConstants;
  using ::PFN_vkCmdResetEvent;
  using ::PFN_vkCmdResetQueryPool;
  using ::PFN_vkCmdResolveImage;
  using ::PFN_vkCmdSetBlendConstants;
  using ::PFN_vkCmdSetDepthBias;
  using ::PFN_vkCmdSetDepthBounds;
  using ::PFN_vkCmdSetEvent;
  using ::PFN_vkCmdSetLineWidth;
  using ::PFN_vkCmdSetScissor;
  using ::PFN_vkCmdSetStencilCompareMask;
  using ::PFN_vkCmdSetStencilReference;
  using ::PFN_vkCmdSetStencilWriteMask;
  using ::PFN_vkCmdSetViewport;
  using ::PFN_vkCmdUpdateBuffer;
  using ::PFN_vkCmdWaitEvents;
  using ::PFN_vkCmdWriteTimestamp;
  using ::PFN_vkCreateBuffer;
  using ::PFN_vkCreateBufferView;
  using ::PFN_vkCreateCommandPool;
  using ::PFN_vkCreateComputePipelines;
  using ::PFN_vkCreateDescriptorPool;
  using ::PFN_vkCreateDescriptorSetLayout;
  using ::PFN_vkCreateDevice;
  using ::PFN_vkCreateEvent;
  using ::PFN_vkCreateFence;
  using ::PFN_vkCreateFramebuffer;
  using ::PFN_vkCreateGraphicsPipelines;
  using ::PFN_vkCreateImage;
  using ::PFN_vkCreateImageView;
  using ::PFN_vkCreateInstance;
  using ::PFN_vkCreatePipelineCache;
  using ::PFN_vkCreatePipelineLayout;
  using ::PFN_vkCreateQueryPool;
  using ::PFN_vkCreateRenderPass;
  using ::PFN_vkCreateSampler;
  using ::PFN_vkCreateSemaphore;
  using ::PFN_vkCreateShaderModule;
  using ::PFN_vkDestroyBuffer;
  using ::PFN_vkDestroyBufferView;
  using ::PFN_vkDestroyCommandPool;
  using ::PFN_vkDestroyDescriptorPool;
  using ::PFN_vkDestroyDescriptorSetLayout;
  using ::PFN_vkDestroyDevice;
  using ::PFN_vkDestroyEvent;
  using ::PFN_vkDestroyFence;
  using ::PFN_vkDestroyFramebuffer;
  using ::PFN_vkDestroyImage;
  using ::PFN_vkDestroyImageView;
  using ::PFN_vkDestroyInstance;
  using ::PFN_vkDestroyPipeline;
  using ::PFN_vkDestroyPipelineCache;
  using ::PFN_vkDestroyPipelineLayout;
  using ::PFN_vkDestroyQueryPool;
  using ::PFN_vkDestroyRenderPass;
  using ::PFN_vkDestroySampler;
  using ::PFN_vkDestroySemaphore;
  using ::PFN_vkDestroyShaderModule;
  using ::PFN_vkDeviceWaitIdle;
  using ::PFN_vkEndCommandBuffer;
  using ::PFN_vkEnumerateDeviceExtensionProperties;
  using ::PFN_vkEnumerateDeviceLayerProperties;
  using ::PFN_vkEnumerateInstanceExtensionProperties;
  using ::PFN_vkEnumerateInstanceLayerProperties;
  using ::PFN_vkEnumeratePhysicalDevices;
  using ::PFN_vkFlushMappedMemoryRanges;
  using ::PFN_vkFreeCommandBuffers;
  using ::PFN_vkFreeDescriptorSets;
  using ::PFN_vkFreeMemory;
  using ::PFN_vkGetBufferMemoryRequirements;
  using ::PFN_vkGetDeviceMemoryCommitment;
  using ::PFN_vkGetDeviceProcAddr;
  using ::PFN_vkGetDeviceQueue;
  using ::PFN_vkGetEventStatus;
  using ::PFN_vkGetFenceStatus;
  using ::PFN_vkGetImageMemoryRequirements;
  using ::PFN_vkGetImageSparseMemoryRequirements;
  using ::PFN_vkGetImageSubresourceLayout;
  using ::PFN_vkGetInstanceProcAddr;
  using ::PFN_vkGetPhysicalDeviceFeatures;
  using ::PFN_vkGetPhysicalDeviceFormatProperties;
  using ::PFN_vkGetPhysicalDeviceImageFormatProperties;
  using ::PFN_vkGetPhysicalDeviceMemoryProperties;
  using ::PFN_vkGetPhysicalDeviceProperties;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyProperties;
  using ::PFN_vkGetPhysicalDeviceSparseImageFormatProperties;
  using ::PFN_vkGetPipelineCacheData;
  using ::PFN_vkGetQueryPoolResults;
  using ::PFN_vkGetRenderAreaGranularity;
  using ::PFN_vkInvalidateMappedMemoryRanges;
  using ::PFN_vkMapMemory;
  using ::PFN_vkMergePipelineCaches;
  using ::PFN_vkQueueBindSparse;
  using ::PFN_vkQueueSubmit;
  using ::PFN_vkQueueWaitIdle;
  using ::PFN_vkResetCommandBuffer;
  using ::PFN_vkResetCommandPool;
  using ::PFN_vkResetDescriptorPool;
  using ::PFN_vkResetEvent;
  using ::PFN_vkResetFences;
  using ::PFN_vkSetEvent;
  using ::PFN_vkUnmapMemory;
  using ::PFN_vkUpdateDescriptorSets;
  using ::PFN_vkWaitForFences;

  //=== VK_VERSION_1_1 ===
  using ::PFN_vkBindBufferMemory2;
  using ::PFN_vkBindImageMemory2;
  using ::PFN_vkCmdDispatchBase;
  using ::PFN_vkCmdSetDeviceMask;
  using ::PFN_vkCreateDescriptorUpdateTemplate;
  using ::PFN_vkCreateSamplerYcbcrConversion;
  using ::PFN_vkDestroyDescriptorUpdateTemplate;
  using ::PFN_vkDestroySamplerYcbcrConversion;
  using ::PFN_vkEnumerateInstanceVersion;
  using ::PFN_vkEnumeratePhysicalDeviceGroups;
  using ::PFN_vkGetBufferMemoryRequirements2;
  using ::PFN_vkGetDescriptorSetLayoutSupport;
  using ::PFN_vkGetDeviceGroupPeerMemoryFeatures;
  using ::PFN_vkGetDeviceQueue2;
  using ::PFN_vkGetImageMemoryRequirements2;
  using ::PFN_vkGetImageSparseMemoryRequirements2;
  using ::PFN_vkGetPhysicalDeviceExternalBufferProperties;
  using ::PFN_vkGetPhysicalDeviceExternalFenceProperties;
  using ::PFN_vkGetPhysicalDeviceExternalSemaphoreProperties;
  using ::PFN_vkGetPhysicalDeviceFeatures2;
  using ::PFN_vkGetPhysicalDeviceFormatProperties2;
  using ::PFN_vkGetPhysicalDeviceImageFormatProperties2;
  using ::PFN_vkGetPhysicalDeviceMemoryProperties2;
  using ::PFN_vkGetPhysicalDeviceProperties2;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyProperties2;
  using ::PFN_vkGetPhysicalDeviceSparseImageFormatProperties2;
  using ::PFN_vkTrimCommandPool;
  using ::PFN_vkUpdateDescriptorSetWithTemplate;

  //=== VK_VERSION_1_2 ===
  using ::PFN_vkCmdBeginRenderPass2;
  using ::PFN_vkCmdDrawIndexedIndirectCount;
  using ::PFN_vkCmdDrawIndirectCount;
  using ::PFN_vkCmdEndRenderPass2;
  using ::PFN_vkCmdNextSubpass2;
  using ::PFN_vkCreateRenderPass2;
  using ::PFN_vkGetBufferDeviceAddress;
  using ::PFN_vkGetBufferOpaqueCaptureAddress;
  using ::PFN_vkGetDeviceMemoryOpaqueCaptureAddress;
  using ::PFN_vkGetSemaphoreCounterValue;
  using ::PFN_vkResetQueryPool;
  using ::PFN_vkSignalSemaphore;
  using ::PFN_vkWaitSemaphores;

  //=== VK_VERSION_1_3 ===
  using ::PFN_vkCmdBeginRendering;
  using ::PFN_vkCmdBindVertexBuffers2;
  using ::PFN_vkCmdBlitImage2;
  using ::PFN_vkCmdCopyBuffer2;
  using ::PFN_vkCmdCopyBufferToImage2;
  using ::PFN_vkCmdCopyImage2;
  using ::PFN_vkCmdCopyImageToBuffer2;
  using ::PFN_vkCmdEndRendering;
  using ::PFN_vkCmdPipelineBarrier2;
  using ::PFN_vkCmdResetEvent2;
  using ::PFN_vkCmdResolveImage2;
  using ::PFN_vkCmdSetCullMode;
  using ::PFN_vkCmdSetDepthBiasEnable;
  using ::PFN_vkCmdSetDepthBoundsTestEnable;
  using ::PFN_vkCmdSetDepthCompareOp;
  using ::PFN_vkCmdSetDepthTestEnable;
  using ::PFN_vkCmdSetDepthWriteEnable;
  using ::PFN_vkCmdSetEvent2;
  using ::PFN_vkCmdSetFrontFace;
  using ::PFN_vkCmdSetPrimitiveRestartEnable;
  using ::PFN_vkCmdSetPrimitiveTopology;
  using ::PFN_vkCmdSetRasterizerDiscardEnable;
  using ::PFN_vkCmdSetScissorWithCount;
  using ::PFN_vkCmdSetStencilOp;
  using ::PFN_vkCmdSetStencilTestEnable;
  using ::PFN_vkCmdSetViewportWithCount;
  using ::PFN_vkCmdWaitEvents2;
  using ::PFN_vkCmdWriteTimestamp2;
  using ::PFN_vkCreatePrivateDataSlot;
  using ::PFN_vkDestroyPrivateDataSlot;
  using ::PFN_vkGetDeviceBufferMemoryRequirements;
  using ::PFN_vkGetDeviceImageMemoryRequirements;
  using ::PFN_vkGetDeviceImageSparseMemoryRequirements;
  using ::PFN_vkGetPhysicalDeviceToolProperties;
  using ::PFN_vkGetPrivateData;
  using ::PFN_vkQueueSubmit2;
  using ::PFN_vkSetPrivateData;

  //=== VK_VERSION_1_4 ===
  using ::PFN_vkCmdBindDescriptorSets2;
  using ::PFN_vkCmdBindIndexBuffer2;
  using ::PFN_vkCmdPushConstants2;
  using ::PFN_vkCmdPushDescriptorSet;
  using ::PFN_vkCmdPushDescriptorSet2;
  using ::PFN_vkCmdPushDescriptorSetWithTemplate;
  using ::PFN_vkCmdPushDescriptorSetWithTemplate2;
  using ::PFN_vkCmdSetLineStipple;
  using ::PFN_vkCmdSetRenderingAttachmentLocations;
  using ::PFN_vkCmdSetRenderingInputAttachmentIndices;
  using ::PFN_vkCopyImageToImage;
  using ::PFN_vkCopyImageToMemory;
  using ::PFN_vkCopyMemoryToImage;
  using ::PFN_vkGetDeviceImageSubresourceLayout;
  using ::PFN_vkGetImageSubresourceLayout2;
  using ::PFN_vkGetRenderingAreaGranularity;
  using ::PFN_vkMapMemory2;
  using ::PFN_vkTransitionImageLayout;
  using ::PFN_vkUnmapMemory2;

  //=== VK_KHR_surface ===
  using ::PFN_vkDestroySurfaceKHR;
  using ::PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
  using ::PFN_vkGetPhysicalDeviceSurfaceFormatsKHR;
  using ::PFN_vkGetPhysicalDeviceSurfacePresentModesKHR;
  using ::PFN_vkGetPhysicalDeviceSurfaceSupportKHR;

  //=== VK_KHR_swapchain ===
  using ::PFN_vkAcquireNextImage2KHR;
  using ::PFN_vkAcquireNextImageKHR;
  using ::PFN_vkCreateSwapchainKHR;
  using ::PFN_vkDestroySwapchainKHR;
  using ::PFN_vkGetDeviceGroupPresentCapabilitiesKHR;
  using ::PFN_vkGetDeviceGroupSurfacePresentModesKHR;
  using ::PFN_vkGetPhysicalDevicePresentRectanglesKHR;
  using ::PFN_vkGetSwapchainImagesKHR;
  using ::PFN_vkQueuePresentKHR;

  //=== VK_KHR_display ===
  using ::PFN_vkCreateDisplayModeKHR;
  using ::PFN_vkCreateDisplayPlaneSurfaceKHR;
  using ::PFN_vkGetDisplayModePropertiesKHR;
  using ::PFN_vkGetDisplayPlaneCapabilitiesKHR;
  using ::PFN_vkGetDisplayPlaneSupportedDisplaysKHR;
  using ::PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
  using ::PFN_vkGetPhysicalDeviceDisplayPropertiesKHR;

  //=== VK_KHR_display_swapchain ===
  using ::PFN_vkCreateSharedSwapchainsKHR;

#if defined( VK_USE_PLATFORM_XLIB_KHR )
  //=== VK_KHR_xlib_surface ===
  using ::PFN_vkCreateXlibSurfaceKHR;
  using ::PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif /*VK_USE_PLATFORM_XLIB_KHR*/

#if defined( VK_USE_PLATFORM_XCB_KHR )
  //=== VK_KHR_xcb_surface ===
  using ::PFN_vkCreateXcbSurfaceKHR;
  using ::PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif /*VK_USE_PLATFORM_XCB_KHR*/

#if defined( VK_USE_PLATFORM_WAYLAND_KHR )
  //=== VK_KHR_wayland_surface ===
  using ::PFN_vkCreateWaylandSurfaceKHR;
  using ::PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif /*VK_USE_PLATFORM_WAYLAND_KHR*/

#if defined( VK_USE_PLATFORM_ANDROID_KHR )
  //=== VK_KHR_android_surface ===
  using ::PFN_vkCreateAndroidSurfaceKHR;
#endif /*VK_USE_PLATFORM_ANDROID_KHR*/

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_KHR_win32_surface ===
  using ::PFN_vkCreateWin32SurfaceKHR;
  using ::PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_EXT_debug_report ===
  using ::PFN_vkCreateDebugReportCallbackEXT;
  using ::PFN_vkDebugReportMessageEXT;
  using ::PFN_vkDestroyDebugReportCallbackEXT;

  //=== VK_EXT_debug_marker ===
  using ::PFN_vkCmdDebugMarkerBeginEXT;
  using ::PFN_vkCmdDebugMarkerEndEXT;
  using ::PFN_vkCmdDebugMarkerInsertEXT;
  using ::PFN_vkDebugMarkerSetObjectNameEXT;
  using ::PFN_vkDebugMarkerSetObjectTagEXT;

  //=== VK_KHR_video_queue ===
  using ::PFN_vkBindVideoSessionMemoryKHR;
  using ::PFN_vkCmdBeginVideoCodingKHR;
  using ::PFN_vkCmdControlVideoCodingKHR;
  using ::PFN_vkCmdEndVideoCodingKHR;
  using ::PFN_vkCreateVideoSessionKHR;
  using ::PFN_vkCreateVideoSessionParametersKHR;
  using ::PFN_vkDestroyVideoSessionKHR;
  using ::PFN_vkDestroyVideoSessionParametersKHR;
  using ::PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR;
  using ::PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR;
  using ::PFN_vkGetVideoSessionMemoryRequirementsKHR;
  using ::PFN_vkUpdateVideoSessionParametersKHR;

  //=== VK_KHR_video_decode_queue ===
  using ::PFN_vkCmdDecodeVideoKHR;

  //=== VK_EXT_transform_feedback ===
  using ::PFN_vkCmdBeginQueryIndexedEXT;
  using ::PFN_vkCmdBeginTransformFeedbackEXT;
  using ::PFN_vkCmdBindTransformFeedbackBuffersEXT;
  using ::PFN_vkCmdDrawIndirectByteCountEXT;
  using ::PFN_vkCmdEndQueryIndexedEXT;
  using ::PFN_vkCmdEndTransformFeedbackEXT;

  //=== VK_NVX_binary_import ===
  using ::PFN_vkCmdCuLaunchKernelNVX;
  using ::PFN_vkCreateCuFunctionNVX;
  using ::PFN_vkCreateCuModuleNVX;
  using ::PFN_vkDestroyCuFunctionNVX;
  using ::PFN_vkDestroyCuModuleNVX;

  //=== VK_NVX_image_view_handle ===
  using ::PFN_vkGetDeviceCombinedImageSamplerIndexNVX;
  using ::PFN_vkGetImageViewAddressNVX;
  using ::PFN_vkGetImageViewHandle64NVX;
  using ::PFN_vkGetImageViewHandleNVX;

  //=== VK_AMD_draw_indirect_count ===
  using ::PFN_vkCmdDrawIndexedIndirectCountAMD;
  using ::PFN_vkCmdDrawIndirectCountAMD;

  //=== VK_AMD_shader_info ===
  using ::PFN_vkGetShaderInfoAMD;

  //=== VK_KHR_dynamic_rendering ===
  using ::PFN_vkCmdBeginRenderingKHR;
  using ::PFN_vkCmdEndRenderingKHR;

#if defined( VK_USE_PLATFORM_GGP )
  //=== VK_GGP_stream_descriptor_surface ===
  using ::PFN_vkCreateStreamDescriptorSurfaceGGP;
#endif /*VK_USE_PLATFORM_GGP*/

  //=== VK_NV_external_memory_capabilities ===
  using ::PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_NV_external_memory_win32 ===
  using ::PFN_vkGetMemoryWin32HandleNV;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_KHR_get_physical_device_properties2 ===
  using ::PFN_vkGetPhysicalDeviceFeatures2KHR;
  using ::PFN_vkGetPhysicalDeviceFormatProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceImageFormatProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceMemoryProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR;

  //=== VK_KHR_device_group ===
  using ::PFN_vkCmdDispatchBaseKHR;
  using ::PFN_vkCmdSetDeviceMaskKHR;
  using ::PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR;

#if defined( VK_USE_PLATFORM_VI_NN )
  //=== VK_NN_vi_surface ===
  using ::PFN_vkCreateViSurfaceNN;
#endif /*VK_USE_PLATFORM_VI_NN*/

  //=== VK_KHR_maintenance1 ===
  using ::PFN_vkTrimCommandPoolKHR;

  //=== VK_KHR_device_group_creation ===
  using ::PFN_vkEnumeratePhysicalDeviceGroupsKHR;

  //=== VK_KHR_external_memory_capabilities ===
  using ::PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_KHR_external_memory_win32 ===
  using ::PFN_vkGetMemoryWin32HandleKHR;
  using ::PFN_vkGetMemoryWin32HandlePropertiesKHR;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_KHR_external_memory_fd ===
  using ::PFN_vkGetMemoryFdKHR;
  using ::PFN_vkGetMemoryFdPropertiesKHR;

  //=== VK_KHR_external_semaphore_capabilities ===
  using ::PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_KHR_external_semaphore_win32 ===
  using ::PFN_vkGetSemaphoreWin32HandleKHR;
  using ::PFN_vkImportSemaphoreWin32HandleKHR;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_KHR_external_semaphore_fd ===
  using ::PFN_vkGetSemaphoreFdKHR;
  using ::PFN_vkImportSemaphoreFdKHR;

  //=== VK_KHR_push_descriptor ===
  using ::PFN_vkCmdPushDescriptorSetKHR;
  using ::PFN_vkCmdPushDescriptorSetWithTemplateKHR;

  //=== VK_EXT_conditional_rendering ===
  using ::PFN_vkCmdBeginConditionalRenderingEXT;
  using ::PFN_vkCmdEndConditionalRenderingEXT;

  //=== VK_KHR_descriptor_update_template ===
  using ::PFN_vkCreateDescriptorUpdateTemplateKHR;
  using ::PFN_vkDestroyDescriptorUpdateTemplateKHR;
  using ::PFN_vkUpdateDescriptorSetWithTemplateKHR;

  //=== VK_NV_clip_space_w_scaling ===
  using ::PFN_vkCmdSetViewportWScalingNV;

  //=== VK_EXT_direct_mode_display ===
  using ::PFN_vkReleaseDisplayEXT;

#if defined( VK_USE_PLATFORM_XLIB_XRANDR_EXT )
  //=== VK_EXT_acquire_xlib_display ===
  using ::PFN_vkAcquireXlibDisplayEXT;
  using ::PFN_vkGetRandROutputDisplayEXT;
#endif /*VK_USE_PLATFORM_XLIB_XRANDR_EXT*/

  //=== VK_EXT_display_surface_counter ===
  using ::PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT;

  //=== VK_EXT_display_control ===
  using ::PFN_vkDisplayPowerControlEXT;
  using ::PFN_vkGetSwapchainCounterEXT;
  using ::PFN_vkRegisterDeviceEventEXT;
  using ::PFN_vkRegisterDisplayEventEXT;

  //=== VK_GOOGLE_display_timing ===
  using ::PFN_vkGetPastPresentationTimingGOOGLE;
  using ::PFN_vkGetRefreshCycleDurationGOOGLE;

  //=== VK_EXT_discard_rectangles ===
  using ::PFN_vkCmdSetDiscardRectangleEnableEXT;
  using ::PFN_vkCmdSetDiscardRectangleEXT;
  using ::PFN_vkCmdSetDiscardRectangleModeEXT;

  //=== VK_EXT_hdr_metadata ===
  using ::PFN_vkSetHdrMetadataEXT;

  //=== VK_KHR_create_renderpass2 ===
  using ::PFN_vkCmdBeginRenderPass2KHR;
  using ::PFN_vkCmdEndRenderPass2KHR;
  using ::PFN_vkCmdNextSubpass2KHR;
  using ::PFN_vkCreateRenderPass2KHR;

  //=== VK_KHR_shared_presentable_image ===
  using ::PFN_vkGetSwapchainStatusKHR;

  //=== VK_KHR_external_fence_capabilities ===
  using ::PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_KHR_external_fence_win32 ===
  using ::PFN_vkGetFenceWin32HandleKHR;
  using ::PFN_vkImportFenceWin32HandleKHR;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_KHR_external_fence_fd ===
  using ::PFN_vkGetFenceFdKHR;
  using ::PFN_vkImportFenceFdKHR;

  //=== VK_KHR_performance_query ===
  using ::PFN_vkAcquireProfilingLockKHR;
  using ::PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
  using ::PFN_vkReleaseProfilingLockKHR;

  //=== VK_KHR_get_surface_capabilities2 ===
  using ::PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR;
  using ::PFN_vkGetPhysicalDeviceSurfaceFormats2KHR;

  //=== VK_KHR_get_display_properties2 ===
  using ::PFN_vkGetDisplayModeProperties2KHR;
  using ::PFN_vkGetDisplayPlaneCapabilities2KHR;
  using ::PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR;
  using ::PFN_vkGetPhysicalDeviceDisplayProperties2KHR;

#if defined( VK_USE_PLATFORM_IOS_MVK )
  //=== VK_MVK_ios_surface ===
  using ::PFN_vkCreateIOSSurfaceMVK;
#endif /*VK_USE_PLATFORM_IOS_MVK*/

#if defined( VK_USE_PLATFORM_MACOS_MVK )
  //=== VK_MVK_macos_surface ===
  using ::PFN_vkCreateMacOSSurfaceMVK;
#endif /*VK_USE_PLATFORM_MACOS_MVK*/

  //=== VK_EXT_debug_utils ===
  using ::PFN_vkCmdBeginDebugUtilsLabelEXT;
  using ::PFN_vkCmdEndDebugUtilsLabelEXT;
  using ::PFN_vkCmdInsertDebugUtilsLabelEXT;
  using ::PFN_vkCreateDebugUtilsMessengerEXT;
  using ::PFN_vkDestroyDebugUtilsMessengerEXT;
  using ::PFN_vkQueueBeginDebugUtilsLabelEXT;
  using ::PFN_vkQueueEndDebugUtilsLabelEXT;
  using ::PFN_vkQueueInsertDebugUtilsLabelEXT;
  using ::PFN_vkSetDebugUtilsObjectNameEXT;
  using ::PFN_vkSetDebugUtilsObjectTagEXT;
  using ::PFN_vkSubmitDebugUtilsMessageEXT;

#if defined( VK_USE_PLATFORM_ANDROID_KHR )
  //=== VK_ANDROID_external_memory_android_hardware_buffer ===
  using ::PFN_vkGetAndroidHardwareBufferPropertiesANDROID;
  using ::PFN_vkGetMemoryAndroidHardwareBufferANDROID;
#endif /*VK_USE_PLATFORM_ANDROID_KHR*/

#if defined( VK_ENABLE_BETA_EXTENSIONS )
  //=== VK_AMDX_shader_enqueue ===
  using ::PFN_vkCmdDispatchGraphAMDX;
  using ::PFN_vkCmdDispatchGraphIndirectAMDX;
  using ::PFN_vkCmdDispatchGraphIndirectCountAMDX;
  using ::PFN_vkCmdInitializeGraphScratchMemoryAMDX;
  using ::PFN_vkCreateExecutionGraphPipelinesAMDX;
  using ::PFN_vkGetExecutionGraphPipelineNodeIndexAMDX;
  using ::PFN_vkGetExecutionGraphPipelineScratchSizeAMDX;
#endif /*VK_ENABLE_BETA_EXTENSIONS*/

  //=== VK_EXT_descriptor_heap ===
  using ::PFN_vkCmdBindResourceHeapEXT;
  using ::PFN_vkCmdBindSamplerHeapEXT;
  using ::PFN_vkCmdPushDataEXT;
  using ::PFN_vkGetImageOpaqueCaptureDataEXT;
  using ::PFN_vkGetPhysicalDeviceDescriptorSizeEXT;
  using ::PFN_vkGetTensorOpaqueCaptureDataARM;
  using ::PFN_vkRegisterCustomBorderColorEXT;
  using ::PFN_vkUnregisterCustomBorderColorEXT;
  using ::PFN_vkWriteResourceDescriptorsEXT;
  using ::PFN_vkWriteSamplerDescriptorsEXT;

  //=== VK_EXT_sample_locations ===
  using ::PFN_vkCmdSetSampleLocationsEXT;
  using ::PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT;

  //=== VK_KHR_get_memory_requirements2 ===
  using ::PFN_vkGetBufferMemoryRequirements2KHR;
  using ::PFN_vkGetImageMemoryRequirements2KHR;
  using ::PFN_vkGetImageSparseMemoryRequirements2KHR;

  //=== VK_KHR_acceleration_structure ===
  using ::PFN_vkBuildAccelerationStructuresKHR;
  using ::PFN_vkCmdBuildAccelerationStructuresIndirectKHR;
  using ::PFN_vkCmdBuildAccelerationStructuresKHR;
  using ::PFN_vkCmdCopyAccelerationStructureKHR;
  using ::PFN_vkCmdCopyAccelerationStructureToMemoryKHR;
  using ::PFN_vkCmdCopyMemoryToAccelerationStructureKHR;
  using ::PFN_vkCmdWriteAccelerationStructuresPropertiesKHR;
  using ::PFN_vkCopyAccelerationStructureKHR;
  using ::PFN_vkCopyAccelerationStructureToMemoryKHR;
  using ::PFN_vkCopyMemoryToAccelerationStructureKHR;
  using ::PFN_vkCreateAccelerationStructureKHR;
  using ::PFN_vkDestroyAccelerationStructureKHR;
  using ::PFN_vkGetAccelerationStructureBuildSizesKHR;
  using ::PFN_vkGetAccelerationStructureDeviceAddressKHR;
  using ::PFN_vkGetDeviceAccelerationStructureCompatibilityKHR;
  using ::PFN_vkWriteAccelerationStructuresPropertiesKHR;

  //=== VK_KHR_ray_tracing_pipeline ===
  using ::PFN_vkCmdSetRayTracingPipelineStackSizeKHR;
  using ::PFN_vkCmdTraceRaysIndirectKHR;
  using ::PFN_vkCmdTraceRaysKHR;
  using ::PFN_vkCreateRayTracingPipelinesKHR;
  using ::PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR;
  using ::PFN_vkGetRayTracingShaderGroupHandlesKHR;
  using ::PFN_vkGetRayTracingShaderGroupStackSizeKHR;

  //=== VK_KHR_sampler_ycbcr_conversion ===
  using ::PFN_vkCreateSamplerYcbcrConversionKHR;
  using ::PFN_vkDestroySamplerYcbcrConversionKHR;

  //=== VK_KHR_bind_memory2 ===
  using ::PFN_vkBindBufferMemory2KHR;
  using ::PFN_vkBindImageMemory2KHR;

  //=== VK_EXT_image_drm_format_modifier ===
  using ::PFN_vkGetImageDrmFormatModifierPropertiesEXT;

  //=== VK_EXT_validation_cache ===
  using ::PFN_vkCreateValidationCacheEXT;
  using ::PFN_vkDestroyValidationCacheEXT;
  using ::PFN_vkGetValidationCacheDataEXT;
  using ::PFN_vkMergeValidationCachesEXT;

  //=== VK_NV_shading_rate_image ===
  using ::PFN_vkCmdBindShadingRateImageNV;
  using ::PFN_vkCmdSetCoarseSampleOrderNV;
  using ::PFN_vkCmdSetViewportShadingRatePaletteNV;

  //=== VK_NV_ray_tracing ===
  using ::PFN_vkBindAccelerationStructureMemoryNV;
  using ::PFN_vkCmdBuildAccelerationStructureNV;
  using ::PFN_vkCmdCopyAccelerationStructureNV;
  using ::PFN_vkCmdTraceRaysNV;
  using ::PFN_vkCmdWriteAccelerationStructuresPropertiesNV;
  using ::PFN_vkCompileDeferredNV;
  using ::PFN_vkCreateAccelerationStructureNV;
  using ::PFN_vkCreateRayTracingPipelinesNV;
  using ::PFN_vkDestroyAccelerationStructureNV;
  using ::PFN_vkGetAccelerationStructureHandleNV;
  using ::PFN_vkGetAccelerationStructureMemoryRequirementsNV;
  using ::PFN_vkGetRayTracingShaderGroupHandlesNV;

  //=== VK_KHR_maintenance3 ===
  using ::PFN_vkGetDescriptorSetLayoutSupportKHR;

  //=== VK_KHR_draw_indirect_count ===
  using ::PFN_vkCmdDrawIndexedIndirectCountKHR;
  using ::PFN_vkCmdDrawIndirectCountKHR;

  //=== VK_EXT_external_memory_host ===
  using ::PFN_vkGetMemoryHostPointerPropertiesEXT;

  //=== VK_AMD_buffer_marker ===
  using ::PFN_vkCmdWriteBufferMarker2AMD;
  using ::PFN_vkCmdWriteBufferMarkerAMD;

  //=== VK_EXT_calibrated_timestamps ===
  using ::PFN_vkGetCalibratedTimestampsEXT;
  using ::PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT;

  //=== VK_NV_mesh_shader ===
  using ::PFN_vkCmdDrawMeshTasksIndirectCountNV;
  using ::PFN_vkCmdDrawMeshTasksIndirectNV;
  using ::PFN_vkCmdDrawMeshTasksNV;

  //=== VK_NV_scissor_exclusive ===
  using ::PFN_vkCmdSetExclusiveScissorEnableNV;
  using ::PFN_vkCmdSetExclusiveScissorNV;

  //=== VK_NV_device_diagnostic_checkpoints ===
  using ::PFN_vkCmdSetCheckpointNV;
  using ::PFN_vkGetQueueCheckpointData2NV;
  using ::PFN_vkGetQueueCheckpointDataNV;

  //=== VK_KHR_timeline_semaphore ===
  using ::PFN_vkGetSemaphoreCounterValueKHR;
  using ::PFN_vkSignalSemaphoreKHR;
  using ::PFN_vkWaitSemaphoresKHR;

  //=== VK_EXT_present_timing ===
  using ::PFN_vkGetPastPresentationTimingEXT;
  using ::PFN_vkGetSwapchainTimeDomainPropertiesEXT;
  using ::PFN_vkGetSwapchainTimingPropertiesEXT;
  using ::PFN_vkSetSwapchainPresentTimingQueueSizeEXT;

  //=== VK_INTEL_performance_query ===
  using ::PFN_vkAcquirePerformanceConfigurationINTEL;
  using ::PFN_vkCmdSetPerformanceMarkerINTEL;
  using ::PFN_vkCmdSetPerformanceOverrideINTEL;
  using ::PFN_vkCmdSetPerformanceStreamMarkerINTEL;
  using ::PFN_vkGetPerformanceParameterINTEL;
  using ::PFN_vkInitializePerformanceApiINTEL;
  using ::PFN_vkQueueSetPerformanceConfigurationINTEL;
  using ::PFN_vkReleasePerformanceConfigurationINTEL;
  using ::PFN_vkUninitializePerformanceApiINTEL;

  //=== VK_AMD_display_native_hdr ===
  using ::PFN_vkSetLocalDimmingAMD;

#if defined( VK_USE_PLATFORM_FUCHSIA )
  //=== VK_FUCHSIA_imagepipe_surface ===
  using ::PFN_vkCreateImagePipeSurfaceFUCHSIA;
#endif /*VK_USE_PLATFORM_FUCHSIA*/

#if defined( VK_USE_PLATFORM_METAL_EXT )
  //=== VK_EXT_metal_surface ===
  using ::PFN_vkCreateMetalSurfaceEXT;
#endif /*VK_USE_PLATFORM_METAL_EXT*/

  //=== VK_KHR_fragment_shading_rate ===
  using ::PFN_vkCmdSetFragmentShadingRateKHR;
  using ::PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR;

  //=== VK_KHR_dynamic_rendering_local_read ===
  using ::PFN_vkCmdSetRenderingAttachmentLocationsKHR;
  using ::PFN_vkCmdSetRenderingInputAttachmentIndicesKHR;

  //=== VK_EXT_buffer_device_address ===
  using ::PFN_vkGetBufferDeviceAddressEXT;

  //=== VK_EXT_tooling_info ===
  using ::PFN_vkGetPhysicalDeviceToolPropertiesEXT;

  //=== VK_KHR_present_wait ===
  using ::PFN_vkWaitForPresentKHR;

  //=== VK_NV_cooperative_matrix ===
  using ::PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV;

  //=== VK_NV_coverage_reduction_mode ===
  using ::PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_EXT_full_screen_exclusive ===
  using ::PFN_vkAcquireFullScreenExclusiveModeEXT;
  using ::PFN_vkGetDeviceGroupSurfacePresentModes2EXT;
  using ::PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT;
  using ::PFN_vkReleaseFullScreenExclusiveModeEXT;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

  //=== VK_EXT_headless_surface ===
  using ::PFN_vkCreateHeadlessSurfaceEXT;

  //=== VK_KHR_buffer_device_address ===
  using ::PFN_vkGetBufferDeviceAddressKHR;
  using ::PFN_vkGetBufferOpaqueCaptureAddressKHR;
  using ::PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR;

  //=== VK_EXT_line_rasterization ===
  using ::PFN_vkCmdSetLineStippleEXT;

  //=== VK_EXT_host_query_reset ===
  using ::PFN_vkResetQueryPoolEXT;

  //=== VK_EXT_extended_dynamic_state ===
  using ::PFN_vkCmdBindVertexBuffers2EXT;
  using ::PFN_vkCmdSetCullModeEXT;
  using ::PFN_vkCmdSetDepthBoundsTestEnableEXT;
  using ::PFN_vkCmdSetDepthCompareOpEXT;
  using ::PFN_vkCmdSetDepthTestEnableEXT;
  using ::PFN_vkCmdSetDepthWriteEnableEXT;
  using ::PFN_vkCmdSetFrontFaceEXT;
  using ::PFN_vkCmdSetPrimitiveTopologyEXT;
  using ::PFN_vkCmdSetScissorWithCountEXT;
  using ::PFN_vkCmdSetStencilOpEXT;
  using ::PFN_vkCmdSetStencilTestEnableEXT;
  using ::PFN_vkCmdSetViewportWithCountEXT;

  //=== VK_KHR_deferred_host_operations ===
  using ::PFN_vkCreateDeferredOperationKHR;
  using ::PFN_vkDeferredOperationJoinKHR;
  using ::PFN_vkDestroyDeferredOperationKHR;
  using ::PFN_vkGetDeferredOperationMaxConcurrencyKHR;
  using ::PFN_vkGetDeferredOperationResultKHR;

  //=== VK_KHR_pipeline_executable_properties ===
  using ::PFN_vkGetPipelineExecutableInternalRepresentationsKHR;
  using ::PFN_vkGetPipelineExecutablePropertiesKHR;
  using ::PFN_vkGetPipelineExecutableStatisticsKHR;

  //=== VK_EXT_host_image_copy ===
  using ::PFN_vkCopyImageToImageEXT;
  using ::PFN_vkCopyImageToMemoryEXT;
  using ::PFN_vkCopyMemoryToImageEXT;
  using ::PFN_vkGetImageSubresourceLayout2EXT;
  using ::PFN_vkTransitionImageLayoutEXT;

  //=== VK_KHR_map_memory2 ===
  using ::PFN_vkMapMemory2KHR;
  using ::PFN_vkUnmapMemory2KHR;

  //=== VK_EXT_swapchain_maintenance1 ===
  using ::PFN_vkReleaseSwapchainImagesEXT;

  //=== VK_NV_device_generated_commands ===
  using ::PFN_vkCmdBindPipelineShaderGroupNV;
  using ::PFN_vkCmdExecuteGeneratedCommandsNV;
  using ::PFN_vkCmdPreprocessGeneratedCommandsNV;
  using ::PFN_vkCreateIndirectCommandsLayoutNV;
  using ::PFN_vkDestroyIndirectCommandsLayoutNV;
  using ::PFN_vkGetGeneratedCommandsMemoryRequirementsNV;

  //=== VK_EXT_depth_bias_control ===
  using ::PFN_vkCmdSetDepthBias2EXT;

  //=== VK_EXT_acquire_drm_display ===
  using ::PFN_vkAcquireDrmDisplayEXT;
  using ::PFN_vkGetDrmDisplayEXT;

  //=== VK_EXT_private_data ===
  using ::PFN_vkCreatePrivateDataSlotEXT;
  using ::PFN_vkDestroyPrivateDataSlotEXT;
  using ::PFN_vkGetPrivateDataEXT;
  using ::PFN_vkSetPrivateDataEXT;

  //=== VK_KHR_video_encode_queue ===
  using ::PFN_vkCmdEncodeVideoKHR;
  using ::PFN_vkGetEncodedVideoSessionParametersKHR;
  using ::PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;

  //=== VK_QCOM_queue_perf_hint ===
  using ::PFN_vkQueueSetPerfHintQCOM;

#if defined( VK_ENABLE_BETA_EXTENSIONS )
  //=== VK_NV_cuda_kernel_launch ===
  using ::PFN_vkCmdCudaLaunchKernelNV;
  using ::PFN_vkCreateCudaFunctionNV;
  using ::PFN_vkCreateCudaModuleNV;
  using ::PFN_vkDestroyCudaFunctionNV;
  using ::PFN_vkDestroyCudaModuleNV;
  using ::PFN_vkGetCudaModuleCacheNV;
#endif /*VK_ENABLE_BETA_EXTENSIONS*/

  //=== VK_QCOM_tile_shading ===
  using ::PFN_vkCmdBeginPerTileExecutionQCOM;
  using ::PFN_vkCmdDispatchTileQCOM;
  using ::PFN_vkCmdEndPerTileExecutionQCOM;

#if defined( VK_USE_PLATFORM_METAL_EXT )
  //=== VK_EXT_metal_objects ===
  using ::PFN_vkExportMetalObjectsEXT;
#endif /*VK_USE_PLATFORM_METAL_EXT*/

  //=== VK_KHR_synchronization2 ===
  using ::PFN_vkCmdPipelineBarrier2KHR;
  using ::PFN_vkCmdResetEvent2KHR;
  using ::PFN_vkCmdSetEvent2KHR;
  using ::PFN_vkCmdWaitEvents2KHR;
  using ::PFN_vkCmdWriteTimestamp2KHR;
  using ::PFN_vkQueueSubmit2KHR;

  //=== VK_EXT_descriptor_buffer ===
  using ::PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT;
  using ::PFN_vkCmdBindDescriptorBuffersEXT;
  using ::PFN_vkCmdSetDescriptorBufferOffsetsEXT;
  using ::PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT;
  using ::PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT;
  using ::PFN_vkGetDescriptorEXT;
  using ::PFN_vkGetDescriptorSetLayoutBindingOffsetEXT;
  using ::PFN_vkGetDescriptorSetLayoutSizeEXT;
  using ::PFN_vkGetImageOpaqueCaptureDescriptorDataEXT;
  using ::PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT;
  using ::PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT;

  //=== VK_KHR_device_address_commands ===
  using ::PFN_vkCmdBeginConditionalRendering2EXT;
  using ::PFN_vkCmdBeginTransformFeedback2EXT;
  using ::PFN_vkCmdBindIndexBuffer3KHR;
  using ::PFN_vkCmdBindTransformFeedbackBuffers2EXT;
  using ::PFN_vkCmdBindVertexBuffers3KHR;
  using ::PFN_vkCmdCopyImageToMemoryKHR;
  using ::PFN_vkCmdCopyMemoryKHR;
  using ::PFN_vkCmdCopyMemoryToImageKHR;
  using ::PFN_vkCmdCopyQueryPoolResultsToMemoryKHR;
  using ::PFN_vkCmdDispatchIndirect2KHR;
  using ::PFN_vkCmdDrawIndexedIndirect2KHR;
  using ::PFN_vkCmdDrawIndexedIndirectCount2KHR;
  using ::PFN_vkCmdDrawIndirect2KHR;
  using ::PFN_vkCmdDrawIndirectByteCount2EXT;
  using ::PFN_vkCmdDrawIndirectCount2KHR;
  using ::PFN_vkCmdDrawMeshTasksIndirect2EXT;
  using ::PFN_vkCmdDrawMeshTasksIndirectCount2EXT;
  using ::PFN_vkCmdEndTransformFeedback2EXT;
  using ::PFN_vkCmdFillMemoryKHR;
  using ::PFN_vkCmdUpdateMemoryKHR;
  using ::PFN_vkCmdWriteMarkerToMemoryAMD;
  using ::PFN_vkCreateAccelerationStructure2KHR;

  //=== VK_NV_fragment_shading_rate_enums ===
  using ::PFN_vkCmdSetFragmentShadingRateEnumNV;

  //=== VK_EXT_mesh_shader ===
  using ::PFN_vkCmdDrawMeshTasksEXT;
  using ::PFN_vkCmdDrawMeshTasksIndirectCountEXT;
  using ::PFN_vkCmdDrawMeshTasksIndirectEXT;

  //=== VK_KHR_copy_commands2 ===
  using ::PFN_vkCmdBlitImage2KHR;
  using ::PFN_vkCmdCopyBuffer2KHR;
  using ::PFN_vkCmdCopyBufferToImage2KHR;
  using ::PFN_vkCmdCopyImage2KHR;
  using ::PFN_vkCmdCopyImageToBuffer2KHR;
  using ::PFN_vkCmdResolveImage2KHR;

  //=== VK_EXT_device_fault ===
  using ::PFN_vkGetDeviceFaultInfoEXT;

#if defined( VK_USE_PLATFORM_WIN32_KHR )
  //=== VK_NV_acquire_winrt_display ===
  using ::PFN_vkAcquireWinrtDisplayNV;
  using ::PFN_vkGetWinrtDisplayNV;
#endif /*VK_USE_PLATFORM_WIN32_KHR*/

#if defined( VK_USE_PLATFORM_DIRECTFB_EXT )
  //=== VK_EXT_directfb_surface ===
  using ::PFN_vkCreateDirectFBSurfaceEXT;
  using ::PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT;
#endif /*VK_USE_PLATFORM_DIRECTFB_EXT*/

  //=== VK_EXT_vertex_input_dynamic_state ===
  using ::PFN_vkCmdSetVertexInputEXT;

#if defined( VK_USE_PLATFORM_FUCHSIA )
  //=== VK_FUCHSIA_external_memory ===
  using ::PFN_vkGetMemoryZirconHandleFUCHSIA;
  using ::PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA;
#endif /*VK_USE_PLATFORM_FUCHSIA*/

#if defined( VK_USE_PLATFORM_FUCHSIA )
  //=== VK_FUCHSIA_external_semaphore ===
  using ::PFN_vkGetSemaphoreZirconHandleFUCHSIA;
  using ::PFN_vkImportSemaphoreZirconHandleFUCHSIA;
#endif /*VK_USE_PLATFORM_FUCHSIA*/

#if defined( VK_USE_PLATFORM_FUCHSIA )
  //=== VK_FUCHSIA_buffer_collection ===
  using ::PFN_vkCreateBufferCollectionFUCHSIA;
  using ::PFN_vkDestroyBufferCollectionFUCHSIA;
  using ::PFN_vkGetBufferCollectionPropertiesFUCHSIA;
  using ::PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA;
  using ::PFN_vkSetBufferCollectionImageConstraintsFUCHSIA;
#endif /*VK_USE_PLATFORM_FUCHSIA*/

  //=== VK_HUAWEI_subpass_shading ===
  using ::PFN_vkCmdSubpassShadingHUAWEI;
  using ::PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;

  //=== VK_HUAWEI_invocation_mask ===
  using ::PFN_vkCmdBindInvocationMaskHUAWEI;

  //=== VK_NV_external_memory_rdma ===
  using ::PFN_vkGetMemoryRemoteAddressNV;

  //=== VK_EXT_pipeline_properties ===
  using ::PFN_vkGetPipelinePropertiesEXT;

  //=== VK_EXT_extended_dynamic_state2 ===
  using ::PFN_vkCmdSetDepthBiasEnableEXT;
  using ::PFN_vkCmdSetLogicOpEXT;
  using ::PFN_vkCmdSetPatchControlPointsEXT;
  using ::PFN_vkCmdSetPrimitiveRestartEnableEXT;
  using ::PFN_vkCmdSetRasterizerDiscardEnableEXT;

#if defined( VK_USE_PLATFORM_SCREEN_QNX )
  //=== VK_QNX_screen_surface ===
  using ::PFN_vkCreateScreenSurfaceQNX;
  using ::PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX;
#endif /*VK_USE_PLATFORM_SCREEN_QNX*/

  //=== VK_EXT_color_write_enable ===
  using ::PFN_vkCmdSetColorWriteEnableEXT;

  //=== VK_KHR_ray_tracing_maintenance1 ===
  using ::PFN_vkCmdTraceRaysIndirect2KHR;

  //=== VK_EXT_multi_draw ===
  using ::PFN_vkCmdDrawMultiEXT;
  using ::PFN_vkCmdDrawMultiIndexedEXT;

  //=== VK_EXT_opacity_micromap ===
  using ::PFN_vkBuildMicromapsEXT;
  using ::PFN_vkCmdBuildMicromapsEXT;
  using ::PFN_vkCmdCopyMemoryToMicromapEXT;
  using ::PFN_vkCmdCopyMicromapEXT;
  using ::PFN_vkCmdCopyMicromapToMemoryEXT;
  using ::PFN_vkCmdWriteMicromapsPropertiesEXT;
  using ::PFN_vkCopyMemoryToMicromapEXT;
  using ::PFN_vkCopyMicromapEXT;
  using ::PFN_vkCopyMicromapToMemoryEXT;
  using ::PFN_vkCreateMicromapEXT;
  using ::PFN_vkDestroyMicromapEXT;
  using ::PFN_vkGetDeviceMicromapCompatibilityEXT;
  using ::PFN_vkGetMicromapBuildSizesEXT;
  using ::PFN_vkWriteMicromapsPropertiesEXT;

  //=== VK_HUAWEI_cluster_culling_shader ===
  using ::PFN_vkCmdDrawClusterHUAWEI;
  using ::PFN_vkCmdDrawClusterIndirectHUAWEI;

  //=== VK_EXT_pageable_device_local_memory ===
  using ::PFN_vkSetDeviceMemoryPriorityEXT;

  //=== VK_KHR_maintenance4 ===
  using ::PFN_vkGetDeviceBufferMemoryRequirementsKHR;
  using ::PFN_vkGetDeviceImageMemoryRequirementsKHR;
  using ::PFN_vkGetDeviceImageSparseMemoryRequirementsKHR;

  //=== VK_ARM_scheduling_controls ===
  using ::PFN_vkCmdSetDispatchParametersARM;

  //=== VK_VALVE_descriptor_set_host_mapping ===
  using ::PFN_vkGetDescriptorSetHostMappingVALVE;
  using ::PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE;

  //=== VK_NV_copy_memory_indirect ===
  using ::PFN_vkCmdCopyMemoryIndirectNV;
  using ::PFN_vkCmdCopyMemoryToImageIndirectNV;

  //=== VK_NV_memory_decompression ===
  using ::PFN_vkCmdDecompressMemoryIndirectCountNV;
  using ::PFN_vkCmdDecompressMemoryNV;

  //=== VK_NV_device_generated_commands_compute ===
  using ::PFN_vkCmdUpdatePipelineIndirectBufferNV;
  using ::PFN_vkGetPipelineIndirectDeviceAddressNV;
  using ::PFN_vkGetPipelineIndirectMemoryRequirementsNV;

#if defined( VK_USE_PLATFORM_OHOS )
  //=== VK_OHOS_external_memory ===
  using ::PFN_vkGetMemoryNativeBufferOHOS;
  using ::PFN_vkGetNativeBufferPropertiesOHOS;
#endif /*VK_USE_PLATFORM_OHOS*/

  //=== VK_EXT_extended_dynamic_state3 ===
  using ::PFN_vkCmdSetAlphaToCoverageEnableEXT;
  using ::PFN_vkCmdSetAlphaToOneEnableEXT;
  using ::PFN_vkCmdSetColorBlendAdvancedEXT;
  using ::PFN_vkCmdSetColorBlendEnableEXT;
  using ::PFN_vkCmdSetColorBlendEquationEXT;
  using ::PFN_vkCmdSetColorWriteMaskEXT;
  using ::PFN_vkCmdSetConservativeRasterizationModeEXT;
  using ::PFN_vkCmdSetCoverageModulationModeNV;
  using ::PFN_vkCmdSetCoverageModulationTableEnableNV;
  using ::PFN_vkCmdSetCoverageModulationTableNV;
  using ::PFN_vkCmdSetCoverageReductionModeNV;
  using ::PFN_vkCmdSetCoverageToColorEnableNV;
  using ::PFN_vkCmdSetCoverageToColorLocationNV;
  using ::PFN_vkCmdSetDepthClampEnableEXT;
  using ::PFN_vkCmdSetDepthClipEnableEXT;
  using ::PFN_vkCmdSetDepthClipNegativeOneToOneEXT;
  using ::PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT;
  using ::PFN_vkCmdSetLineRasterizationModeEXT;
  using ::PFN_vkCmdSetLineStippleEnableEXT;
  using ::PFN_vkCmdSetLogicOpEnableEXT;
  using ::PFN_vkCmdSetPolygonModeEXT;
  using ::PFN_vkCmdSetProvokingVertexModeEXT;
  using ::PFN_vkCmdSetRasterizationSamplesEXT;
  using ::PFN_vkCmdSetRasterizationStreamEXT;
  using ::PFN_vkCmdSetRepresentativeFragmentTestEnableNV;
  using ::PFN_vkCmdSetSampleLocationsEnableEXT;
  using ::PFN_vkCmdSetSampleMaskEXT;
  using ::PFN_vkCmdSetShadingRateImageEnableNV;
  using ::PFN_vkCmdSetTessellationDomainOriginEXT;
  using ::PFN_vkCmdSetViewportSwizzleNV;
  using ::PFN_vkCmdSetViewportWScalingEnableNV;

  //=== VK_ARM_tensors ===
  using ::PFN_vkBindTensorMemoryARM;
  using ::PFN_vkCmdCopyTensorARM;
  using ::PFN_vkCreateTensorARM;
  using ::PFN_vkCreateTensorViewARM;
  using ::PFN_vkDestroyTensorARM;
  using ::PFN_vkDestroyTensorViewARM;
  using ::PFN_vkGetDeviceTensorMemoryRequirementsARM;
  using ::PFN_vkGetPhysicalDeviceExternalTensorPropertiesARM;
  using ::PFN_vkGetTensorMemoryRequirementsARM;
  using ::PFN_vkGetTensorOpaqueCaptureDescriptorDataARM;
  using ::PFN_vkGetTensorViewOpaqueCaptureDescriptorDataARM;

  //=== VK_EXT_shader_module_identifier ===
  using ::PFN_vkGetShaderModuleCreateInfoIdentifierEXT;
  using ::PFN_vkGetShaderModuleIdentifierEXT;

  //=== VK_NV_optical_flow ===
  using ::PFN_vkBindOpticalFlowSessionImageNV;
  using ::PFN_vkCmdOpticalFlowExecuteNV;
  using ::PFN_vkCreateOpticalFlowSessionNV;
  using ::PFN_vkDestroyOpticalFlowSessionNV;
  using ::PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV;

  //=== VK_KHR_maintenance5 ===
  using ::PFN_vkCmdBindIndexBuffer2KHR;
  using ::PFN_vkGetDeviceImageSubresourceLayoutKHR;
  using ::PFN_vkGetImageSubresourceLayout2KHR;
  using ::PFN_vkGetRenderingAreaGranularityKHR;

  //=== VK_AMD_anti_lag ===
  using ::PFN_vkAntiLagUpdateAMD;

  //=== VK_KHR_present_wait2 ===
  using ::PFN_vkWaitForPresent2KHR;

  //=== VK_EXT_shader_object ===
  using ::PFN_vkCmdBindShadersEXT;
  using ::PFN_vkCmdSetDepthClampRangeEXT;
  using ::PFN_vkCreateShadersEXT;
  using ::PFN_vkDestroyShaderEXT;
  using ::PFN_vkGetShaderBinaryDataEXT;

  //=== VK_KHR_pipeline_binary ===
  using ::PFN_vkCreatePipelineBinariesKHR;
  using ::PFN_vkDestroyPipelineBinaryKHR;
  using ::PFN_vkGetPipelineBinaryDataKHR;
  using ::PFN_vkGetPipelineKeyKHR;
  using ::PFN_vkReleaseCapturedPipelineDataKHR;

  //=== VK_QCOM_tile_properties ===
  using ::PFN_vkGetDynamicRenderingTilePropertiesQCOM;
  using ::PFN_vkGetFramebufferTilePropertiesQCOM;

  //=== VK_KHR_swapchain_maintenance1 ===
  using ::PFN_vkReleaseSwapchainImagesKHR;

  //=== VK_NV_cooperative_vector ===
  using ::PFN_vkCmdConvertCooperativeVectorMatrixNV;
  using ::PFN_vkConvertCooperativeVectorMatrixNV;
  using ::PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV;

  //=== VK_NV_low_latency2 ===
  using ::PFN_vkGetLatencyTimingsNV;
  using ::PFN_vkLatencySleepNV;
  using ::PFN_vkQueueNotifyOutOfBandNV;
  using ::PFN_vkSetLatencyMarkerNV;
  using ::PFN_vkSetLatencySleepModeNV;

  //=== VK_KHR_cooperative_matrix ===
  using ::PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR;

  //=== VK_ARM_data_graph ===
  using ::PFN_vkBindDataGraphPipelineSessionMemoryARM;
  using ::PFN_vkCmdDispatchDataGraphARM;
  using ::PFN_vkCreateDataGraphPipelinesARM;
  using ::PFN_vkCreateDataGraphPipelineSessionARM;
  using ::PFN_vkDestroyDataGraphPipelineSessionARM;
  using ::PFN_vkGetDataGraphPipelineAvailablePropertiesARM;
  using ::PFN_vkGetDataGraphPipelinePropertiesARM;
  using ::PFN_vkGetDataGraphPipelineSessionBindPointRequirementsARM;
  using ::PFN_vkGetDataGraphPipelineSessionMemoryRequirementsARM;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM;
  using ::PFN_vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM;

  //=== VK_ARM_data_graph_instruction_set_tosa ===
  using ::PFN_vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM;

  //=== VK_EXT_attachment_feedback_loop_dynamic_state ===
  using ::PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT;

#if defined( VK_USE_PLATFORM_SCREEN_QNX )
  //=== VK_QNX_external_memory_screen_buffer ===
  using ::PFN_vkGetScreenBufferPropertiesQNX;
#endif /*VK_USE_PLATFORM_SCREEN_QNX*/

  //=== VK_KHR_line_rasterization ===
  using ::PFN_vkCmdSetLineStippleKHR;

  //=== VK_KHR_calibrated_timestamps ===
  using ::PFN_vkGetCalibratedTimestampsKHR;
  using ::PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR;

  //=== VK_KHR_maintenance6 ===
  using ::PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT;
  using ::PFN_vkCmdBindDescriptorSets2KHR;
  using ::PFN_vkCmdPushConstants2KHR;
  using ::PFN_vkCmdPushDescriptorSet2KHR;
  using ::PFN_vkCmdPushDescriptorSetWithTemplate2KHR;
  using ::PFN_vkCmdSetDescriptorBufferOffsets2EXT;

  //=== VK_QCOM_tile_memory_heap ===
  using ::PFN_vkCmdBindTileMemoryQCOM;

  //=== VK_KHR_copy_memory_indirect ===
  using ::PFN_vkCmdCopyMemoryIndirectKHR;
  using ::PFN_vkCmdCopyMemoryToImageIndirectKHR;

  //=== VK_EXT_memory_decompression ===
  using ::PFN_vkCmdDecompressMemoryEXT;
  using ::PFN_vkCmdDecompressMemoryIndirectCountEXT;

  //=== VK_NV_external_compute_queue ===
  using ::PFN_vkCreateExternalComputeQueueNV;
  using ::PFN_vkDestroyExternalComputeQueueNV;
  using ::PFN_vkGetExternalComputeQueueDataNV;

  //=== VK_NV_cluster_acceleration_structure ===
  using ::PFN_vkCmdBuildClusterAccelerationStructureIndirectNV;
  using ::PFN_vkGetClusterAccelerationStructureBuildSizesNV;

  //=== VK_NV_partitioned_acceleration_structure ===
  using ::PFN_vkCmdBuildPartitionedAccelerationStructuresNV;
  using ::PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV;

  //=== VK_EXT_device_generated_commands ===
  using ::PFN_vkCmdExecuteGeneratedCommandsEXT;
  using ::PFN_vkCmdPreprocessGeneratedCommandsEXT;
  using ::PFN_vkCreateIndirectCommandsLayoutEXT;
  using ::PFN_vkCreateIndirectExecutionSetEXT;
  using ::PFN_vkDestroyIndirectCommandsLayoutEXT;
  using ::PFN_vkDestroyIndirectExecutionSetEXT;
  using ::PFN_vkGetGeneratedCommandsMemoryRequirementsEXT;
  using ::PFN_vkUpdateIndirectExecutionSetPipelineEXT;
  using ::PFN_vkUpdateIndirectExecutionSetShaderEXT;

  //=== VK_KHR_device_fault ===
  using ::PFN_vkGetDeviceFaultDebugInfoKHR;
  using ::PFN_vkGetDeviceFaultReportsKHR;

#if defined( VK_USE_PLATFORM_OHOS )
  //=== VK_OHOS_surface ===
  using ::PFN_vkCreateSurfaceOHOS;
#endif /*VK_USE_PLATFORM_OHOS*/

  //=== VK_NV_cooperative_matrix2 ===
  using ::PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV;

#if defined( VK_USE_PLATFORM_METAL_EXT )
  //=== VK_EXT_external_memory_metal ===
  using ::PFN_vkGetMemoryMetalHandleEXT;
  using ::PFN_vkGetMemoryMetalHandlePropertiesEXT;
#endif /*VK_USE_PLATFORM_METAL_EXT*/

  //=== VK_ARM_performance_counters_by_region ===
  using ::PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM;

  //=== VK_ARM_shader_instrumentation ===
  using ::PFN_vkClearShaderInstrumentationMetricsARM;
  using ::PFN_vkCmdBeginShaderInstrumentationARM;
  using ::PFN_vkCmdEndShaderInstrumentationARM;
  using ::PFN_vkCreateShaderInstrumentationARM;
  using ::PFN_vkDestroyShaderInstrumentationARM;
  using ::PFN_vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM;
  using ::PFN_vkGetShaderInstrumentationValuesARM;

  //=== VK_EXT_fragment_density_map_offset ===
  using ::PFN_vkCmdEndRendering2EXT;

  //=== VK_EXT_custom_resolve ===
  using ::PFN_vkCmdBeginCustomResolveEXT;

  //=== VK_KHR_maintenance10 ===
  using ::PFN_vkCmdEndRendering2KHR;

  //=== VK_ARM_data_graph_optical_flow ===
  using ::PFN_vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM;

  //=== VK_NV_compute_occupancy_priority ===
  using ::PFN_vkCmdSetComputeOccupancyPriorityNV;

#if defined( VK_USE_PLATFORM_UBM_SEC )
  //=== VK_SEC_ubm_surface ===
  using ::PFN_vkCreateUbmSurfaceSEC;
  using ::PFN_vkGetPhysicalDeviceUbmPresentationSupportSEC;
#endif /*VK_USE_PLATFORM_UBM_SEC*/

  //=== VK_EXT_primitive_restart_index ===
  using ::PFN_vkCmdSetPrimitiveRestartIndexEXT;
}
