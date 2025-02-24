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

#include "VkGetProcAddress.hpp"
#include "VkDevice.hpp"

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#ifdef __ANDROID__
#	include <hardware/hwvulkan.h>
#	include <vulkan/vk_android_native_buffer.h>
#	include <cerrno>
#endif

namespace vk {

// Functions that can be obtained through GetInstanceProcAddr without an instance
#define MAKE_VULKAN_GLOBAL_ENTRY(aFunction)                           \
	{                                                                 \
#		aFunction, reinterpret_cast < PFN_vkVoidFunction>(aFunction) \
	}

// TODO(b/208256248): Avoid exit-time destructor.
static const std::unordered_map<std::string, PFN_vkVoidFunction> globalFunctionPointers = {
	MAKE_VULKAN_GLOBAL_ENTRY(vkGetInstanceProcAddr),
	MAKE_VULKAN_GLOBAL_ENTRY(vkCreateInstance),
	MAKE_VULKAN_GLOBAL_ENTRY(vkEnumerateInstanceExtensionProperties),
	MAKE_VULKAN_GLOBAL_ENTRY(vkEnumerateInstanceLayerProperties),
	MAKE_VULKAN_GLOBAL_ENTRY(vkEnumerateInstanceVersion),

	MAKE_VULKAN_GLOBAL_ENTRY(vk_icdGetInstanceProcAddr),
	MAKE_VULKAN_GLOBAL_ENTRY(vk_icdNegotiateLoaderICDInterfaceVersion),
#if VK_USE_PLATFORM_WIN32_KHR
	MAKE_VULKAN_GLOBAL_ENTRY(vk_icdEnumerateAdapterPhysicalDevices),
#endif  // VK_USE_PLATFORM_WIN32_KHR
};
#undef MAKE_VULKAN_GLOBAL_ENTRY

// Functions that can be obtained through GetInstanceProcAddr with an instance object
struct InstanceFunctionEntry
{
	PFN_vkVoidFunction pfn;
	// True if the first argument is a VkPhysicalDevice. See
	// https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderDriverInterface.md#reason-for-adding-vk_icdgetphysicaldeviceprocaddr
	bool isPhysicalDeviceFn;
};

// Template magic to detect if the first argument of a C function is a VkPhysicalDevice.
template<typename T>
struct FunctionArgs
{};

template<typename R, typename... Args>
struct FunctionArgs<VKAPI_ATTR R VKAPI_CALL(Args...)>
{
	using Tuple = std::tuple<Args...>;
	using FirstType = typename std::tuple_element<0, Tuple>::type;
};

template<typename T>
constexpr bool HasPhysicalDeviceFirstArgument =
    std::is_same<typename FunctionArgs<T>::FirstType, VkPhysicalDevice>::value;

#define MAKE_VULKAN_INSTANCE_ENTRY(aFunction)                       \
	{                                                               \
		#aFunction,                                                 \
		{                                                           \
			reinterpret_cast<PFN_vkVoidFunction>(aFunction),        \
			    HasPhysicalDeviceFirstArgument<decltype(aFunction)> \
		}                                                           \
	}

// TODO(b/208256248): Avoid exit-time destructor.
static const std::unordered_map<std::string, InstanceFunctionEntry> instanceFunctionPointers = {

	MAKE_VULKAN_INSTANCE_ENTRY(vkDestroyInstance),
	MAKE_VULKAN_INSTANCE_ENTRY(vkEnumeratePhysicalDevices),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFeatures),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFormatProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceImageFormatProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceMemoryProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateDevice),
	MAKE_VULKAN_INSTANCE_ENTRY(vkEnumerateDeviceExtensionProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkEnumerateDeviceLayerProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkEnumeratePhysicalDeviceGroups),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFeatures2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFormatProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceImageFormatProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceMemoryProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties2),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalBufferProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalFenceProperties),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalSemaphoreProperties),
	// VK_KHR_device_group_creation
	MAKE_VULKAN_INSTANCE_ENTRY(vkEnumeratePhysicalDeviceGroupsKHR),
	// VK_KHR_external_fence_capabilities
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalFencePropertiesKHR),
	// VK_KHR_external_memory_capabilities
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalBufferPropertiesKHR),
	// VK_KHR_external_semaphore_capabilities
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR),
	// VK_KHR_get_physical_device_properties2
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFeatures2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceProperties2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceFormatProperties2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceImageFormatProperties2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceMemoryProperties2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties2KHR),
	// VK_EXT_debug_utils
	MAKE_VULKAN_INSTANCE_ENTRY(vkCmdBeginDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkCmdEndDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkCmdInsertDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateDebugUtilsMessengerEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkDestroyDebugUtilsMessengerEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkQueueBeginDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkQueueEndDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkQueueInsertDebugUtilsLabelEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkSetDebugUtilsObjectNameEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkSetDebugUtilsObjectTagEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkSubmitDebugUtilsMessageEXT),
	// VK_EXT_tooling_info
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceToolProperties),
#ifndef __ANDROID__
	// VK_KHR_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkDestroySurfaceKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfaceSupportKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfaceFormatsKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfacePresentModesKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDevicePresentRectanglesKHR),
	// VK_KHR_get_surface_capabilities2
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfaceCapabilities2KHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceSurfaceFormats2KHR),
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
	// VK_KHR_Xcb_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateXcbSurfaceKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceXcbPresentationSupportKHR),
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	// VK_KHR_wayland_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateWaylandSurfaceKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceWaylandPresentationSupportKHR),
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
	// VK_EXT_directfb_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateDirectFBSurfaceEXT),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceDirectFBPresentationSupportEXT),
#endif
#ifdef VK_USE_PLATFORM_DISPLAY_KHR
	// VK_KHR_display
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateDisplayModeKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateDisplayPlaneSurfaceKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetDisplayModePropertiesKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetDisplayPlaneCapabilitiesKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetDisplayPlaneSupportedDisplaysKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceDisplayPlanePropertiesKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceDisplayPropertiesKHR),
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
	// VK_MVK_macos_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateMacOSSurfaceMVK),
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
	// VK_EXT_metal_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateMetalSurfaceEXT),
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	// VK_KHR_win32_surface
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateWin32SurfaceKHR),
	MAKE_VULKAN_INSTANCE_ENTRY(vkGetPhysicalDeviceWin32PresentationSupportKHR),
#endif
	MAKE_VULKAN_INSTANCE_ENTRY(vkCreateHeadlessSurfaceEXT),
};
#undef MAKE_VULKAN_INSTANCE_ENTRY

// Functions that can be obtained through GetDeviceProcAddr with a device object
#define MAKE_VULKAN_DEVICE_ENTRY(aFunction)                           \
	{                                                                 \
#		aFunction, reinterpret_cast < PFN_vkVoidFunction>(aFunction) \
	}

// TODO(b/208256248): Avoid exit-time destructor.
static const std::unordered_map<std::string, PFN_vkVoidFunction> deviceFunctionPointers = {
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceProcAddr),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyDevice),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceQueue),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueSubmit),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueWaitIdle),
	MAKE_VULKAN_DEVICE_ENTRY(vkDeviceWaitIdle),
	MAKE_VULKAN_DEVICE_ENTRY(vkAllocateMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkFreeMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkMapMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkUnmapMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkFlushMappedMemoryRanges),
	MAKE_VULKAN_DEVICE_ENTRY(vkInvalidateMappedMemoryRanges),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceMemoryCommitment),
	MAKE_VULKAN_DEVICE_ENTRY(vkBindBufferMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkBindImageMemory),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferMemoryRequirements),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetImageMemoryRequirements),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetImageSparseMemoryRequirements),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueBindSparse),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateFence),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyFence),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetFences),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetFenceStatus),
	MAKE_VULKAN_DEVICE_ENTRY(vkWaitForFences),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateSemaphore),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroySemaphore),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetEventStatus),
	MAKE_VULKAN_DEVICE_ENTRY(vkSetEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateQueryPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyQueryPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetQueryPoolResults),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateBufferView),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyBufferView),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetImageSubresourceLayout),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateImageView),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyImageView),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateShaderModule),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyShaderModule),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreatePipelineCache),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyPipelineCache),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetPipelineCacheData),
	MAKE_VULKAN_DEVICE_ENTRY(vkMergePipelineCaches),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateGraphicsPipelines),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateComputePipelines),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyPipeline),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreatePipelineLayout),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyPipelineLayout),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateSampler),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroySampler),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateDescriptorSetLayout),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyDescriptorSetLayout),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateDescriptorPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyDescriptorPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetDescriptorPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkAllocateDescriptorSets),
	MAKE_VULKAN_DEVICE_ENTRY(vkFreeDescriptorSets),
	MAKE_VULKAN_DEVICE_ENTRY(vkUpdateDescriptorSets),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateFramebuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyFramebuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateRenderPass),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyRenderPass),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetRenderAreaGranularity),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateCommandPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyCommandPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetCommandPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkAllocateCommandBuffers),
	MAKE_VULKAN_DEVICE_ENTRY(vkFreeCommandBuffers),
	MAKE_VULKAN_DEVICE_ENTRY(vkBeginCommandBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkEndCommandBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetCommandBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindPipeline),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetViewport),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetScissor),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetLineWidth),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthBias),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetBlendConstants),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthBounds),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilCompareMask),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilWriteMask),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilReference),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindDescriptorSets),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindIndexBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindVertexBuffers),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDraw),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndexed),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndirect),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndexedIndirect),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDispatch),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDispatchIndirect),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBlitImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBufferToImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImageToBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdUpdateBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdFillBuffer),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdClearColorImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdClearDepthStencilImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdClearAttachments),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdResolveImage),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdResetEvent),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdWaitEvents),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdPipelineBarrier),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginQuery),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndQuery),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdResetQueryPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdWriteTimestamp),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyQueryPoolResults),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdPushConstants),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginRenderPass),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdNextSubpass),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndRenderPass),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdExecuteCommands),
	MAKE_VULKAN_DEVICE_ENTRY(vkBindBufferMemory2),
	MAKE_VULKAN_DEVICE_ENTRY(vkBindImageMemory2),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceGroupPeerMemoryFeatures),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDeviceMask),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDispatchBase),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetImageMemoryRequirements2),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferMemoryRequirements2),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetImageSparseMemoryRequirements2),
	MAKE_VULKAN_DEVICE_ENTRY(vkTrimCommandPool),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceQueue2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateSamplerYcbcrConversion),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroySamplerYcbcrConversion),
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateDescriptorUpdateTemplate),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyDescriptorUpdateTemplate),
	MAKE_VULKAN_DEVICE_ENTRY(vkUpdateDescriptorSetWithTemplate),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDescriptorSetLayoutSupport),
	// Device level VK_EXT_debug_utils functions
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdInsertDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueBeginDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueEndDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueInsertDebugUtilsLabelEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkSetDebugUtilsObjectNameEXT),
	MAKE_VULKAN_DEVICE_ENTRY(vkSetDebugUtilsObjectTagEXT),
#ifdef __ANDROID__
	MAKE_VULKAN_DEVICE_ENTRY(vkGetSwapchainGrallocUsageANDROID),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetSwapchainGrallocUsage2ANDROID),
	MAKE_VULKAN_DEVICE_ENTRY(vkAcquireImageANDROID),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueSignalReleaseImageANDROID),
#endif
	// Vulkan 1.2 Entry point functions
	MAKE_VULKAN_DEVICE_ENTRY(vkCreateRenderPass2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginRenderPass2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndRenderPass2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdNextSubpass2),
	MAKE_VULKAN_DEVICE_ENTRY(vkResetQueryPool),
	// VK_KHR_timeline_semaphore
	MAKE_VULKAN_DEVICE_ENTRY(vkGetSemaphoreCounterValue),
	MAKE_VULKAN_DEVICE_ENTRY(vkSignalSemaphore),
	MAKE_VULKAN_DEVICE_ENTRY(vkWaitSemaphores),
	// VK_KHR_buffer_device_address
	MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferDeviceAddress),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferOpaqueCaptureAddress),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceMemoryOpaqueCaptureAddress),
	// VK_KHR_draw_indirect_count
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndirectCount),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndexedIndirectCount),
	// Vulkan 1.3 Entry point functions
	// VK_KHR_copy_commands2
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBuffer2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImage2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBufferToImage2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImageToBuffer2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBlitImage2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdResolveImage2),
	// VK_KHR_dynamic_rendering
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginRendering),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndRendering),
	// VK_KHR_maintenance4
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceBufferMemoryRequirements),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceImageMemoryRequirements),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceImageSparseMemoryRequirements),
	// VK_KHR_synchronization2
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetEvent2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdResetEvent2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdWaitEvents2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdPipelineBarrier2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdWriteTimestamp2),
	MAKE_VULKAN_DEVICE_ENTRY(vkQueueSubmit2),
	// VK_EXT_extended_dynamic_state
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetCullMode),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetFrontFace),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetPrimitiveTopology),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetViewportWithCount),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetScissorWithCount),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindVertexBuffers2),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthTestEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthWriteEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthCompareOp),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthBoundsTestEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilTestEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilOp),
	// VK_EXT_extended_dynamic_state2 (partial promotion, VKCmdSetLogicOpEXT and VKCmdSetPatchControlPointsEXT are not promoted)
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetRasterizerDiscardEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthBiasEnable),
	MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetPrimitiveRestartEnable),
	// VK_EXT_private_data
	MAKE_VULKAN_DEVICE_ENTRY(vkCreatePrivateDataSlot),
	MAKE_VULKAN_DEVICE_ENTRY(vkDestroyPrivateDataSlot),
	MAKE_VULKAN_DEVICE_ENTRY(vkSetPrivateData),
	MAKE_VULKAN_DEVICE_ENTRY(vkGetPrivateData),
};

// TODO(b/208256248): Avoid exit-time destructor.
static const std::vector<std::pair<const char *, std::unordered_map<std::string, PFN_vkVoidFunction>>> deviceExtensionFunctionPointers = {
	// VK_KHR_descriptor_update_template
	{
	    VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCreateDescriptorUpdateTemplateKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkDestroyDescriptorUpdateTemplateKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkUpdateDescriptorSetWithTemplateKHR),
	    } },
	// VK_KHR_device_group
	{
	    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceGroupPeerMemoryFeaturesKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDeviceMaskKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdDispatchBaseKHR),
	    } },
	// VK_KHR_maintenance1
	{
	    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkTrimCommandPoolKHR),
	    } },
	// VK_KHR_sampler_ycbcr_conversion
	{
	    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCreateSamplerYcbcrConversionKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkDestroySamplerYcbcrConversionKHR),
	    } },
	// VK_KHR_bind_memory2
	{
	    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkBindBufferMemory2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkBindImageMemory2KHR),
	    } },
	// VK_KHR_copy_commands2
	{
	    VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdBlitImage2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBuffer2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyBufferToImage2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImage2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdCopyImageToBuffer2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdResolveImage2KHR),
	    } },
	// VK_KHR_get_memory_requirements2
	{
	    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetImageMemoryRequirements2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferMemoryRequirements2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetImageSparseMemoryRequirements2KHR),
	    } },
	// VK_KHR_maintenance3
	{
	    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDescriptorSetLayoutSupportKHR),
	    } },
	// VK_KHR_maintenance4
	{
	    VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceBufferMemoryRequirementsKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceImageMemoryRequirementsKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceImageSparseMemoryRequirementsKHR),
	    } },
	// VK_KHR_create_renderpass2
	{
	    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCreateRenderPass2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginRenderPass2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdNextSubpass2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndRenderPass2KHR),
	    } },
	// VK_KHR_timeline_semaphore
	{
	    VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetSemaphoreCounterValueKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkSignalSemaphoreKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkWaitSemaphoresKHR),
	    } },
	// VK_KHR_buffer_device_address
	{
	    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferDeviceAddressKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetBufferOpaqueCaptureAddressKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceMemoryOpaqueCaptureAddressKHR),
	    } },
	// VK_KHR_draw_indirect_count
	{
	    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndirectCountKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdDrawIndexedIndirectCountKHR),
	    } },
	// VK_KHR_dynamic_rendering
	{
	    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdBeginRenderingKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdEndRenderingKHR),
	    } },
	// VK_KHR_dynamic_rendering_local_read
	{
	    VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetRenderingAttachmentLocationsKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetRenderingInputAttachmentIndicesKHR),
	    } },
	// VK_EXT_extended_dynamic_state
	{
	    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdBindVertexBuffers2EXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetCullModeEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthBoundsTestEnableEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthCompareOpEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthTestEnableEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetDepthWriteEnableEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetFrontFaceEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetPrimitiveTopologyEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetScissorWithCountEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilOpEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetStencilTestEnableEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetViewportWithCountEXT),
	    } },
	// VK_EXT_vertex_input_dynamic_state
	{
	    VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetVertexInputEXT),
	    } },
	// VK_EXT_line_rasterization
	{
	    VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetLineStippleEXT),
	    } },
	// VK_EXT_host_query_reset
	{
	    VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkResetQueryPoolEXT),
	    } },
	// VK_EXT_tooling_info
	{
	    VK_EXT_TOOLING_INFO_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetPhysicalDeviceToolPropertiesEXT),
	    } },
	// VK_KHR_synchronization2
	{
	    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdPipelineBarrier2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdResetEvent2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdSetEvent2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdWaitEvents2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCmdWriteTimestamp2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkQueueSubmit2KHR),
	    } },
#ifndef __ANDROID__
	// VK_KHR_swapchain
	{
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCreateSwapchainKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkDestroySwapchainKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetSwapchainImagesKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkAcquireNextImageKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkAcquireNextImage2KHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkQueuePresentKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceGroupPresentCapabilitiesKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetDeviceGroupSurfacePresentModesKHR),
	    } },
	// VK_EXT_swapchain_maintenance1
	{
	    VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkReleaseSwapchainImagesEXT),
	    } },
#endif

#if SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD
	// VK_KHR_external_semaphore_fd
	{
	    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetSemaphoreFdKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkImportSemaphoreFdKHR),
	    } },
#endif

#if VK_USE_PLATFORM_FUCHSIA
	// VK_FUCHSIA_external_semaphore
	{
	    VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetSemaphoreZirconHandleFUCHSIA),
	        MAKE_VULKAN_DEVICE_ENTRY(vkImportSemaphoreZirconHandleFUCHSIA),
	    } },
#endif

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	// VK_KHR_external_memory_fd
	{
	    VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryFdKHR),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryFdPropertiesKHR),
	    } },
#endif

	// VK_EXT_external_memory_host
	{
	    VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryHostPointerPropertiesEXT),
	    } },

	// VK_EXT_private_data
	{
	    VK_EXT_PRIVATE_DATA_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCreatePrivateDataSlotEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkDestroyPrivateDataSlotEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetPrivateDataEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkSetPrivateDataEXT),
	    } },
	// VK_EXT_host_image_copy
	{
	    VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkCopyImageToImageEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCopyImageToMemoryEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkCopyMemoryToImageEXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetImageSubresourceLayout2EXT),
	        MAKE_VULKAN_DEVICE_ENTRY(vkTransitionImageLayoutEXT),
	    } },
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	// VK_ANDROID_external_memory_android_hardware_buffer
	{
	    VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetAndroidHardwareBufferPropertiesANDROID),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryAndroidHardwareBufferANDROID),
	    } },
#endif

#if VK_USE_PLATFORM_FUCHSIA
	// VK_FUCHSIA_external_memory
	{
	    VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME,
	    {
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryZirconHandleFUCHSIA),
	        MAKE_VULKAN_DEVICE_ENTRY(vkGetMemoryZirconHandlePropertiesFUCHSIA),
	    } },
#endif
};

#undef MAKE_VULKAN_DEVICE_ENTRY

PFN_vkVoidFunction GetInstanceProcAddr(Instance *instance, const char *pName)
{
	auto globalFunction = globalFunctionPointers.find(std::string(pName));
	if(globalFunction != globalFunctionPointers.end())
	{
		return globalFunction->second;
	}

	if(instance)
	{
		auto instanceFunction = instanceFunctionPointers.find(std::string(pName));
		if(instanceFunction != instanceFunctionPointers.end())
		{
			return instanceFunction->second.pfn;
		}

		auto deviceFunction = deviceFunctionPointers.find(std::string(pName));
		if(deviceFunction != deviceFunctionPointers.end())
		{
			return deviceFunction->second;
		}

		for(const auto &deviceExtensionFunctions : deviceExtensionFunctionPointers)
		{
			deviceFunction = deviceExtensionFunctions.second.find(std::string(pName));
			if(deviceFunction != deviceExtensionFunctions.second.end())
			{
				return deviceFunction->second;
			}
		}
	}

	return nullptr;
}

PFN_vkVoidFunction GetPhysicalDeviceProcAddr(Instance *instance, const char *pName)
{
	// This function must return nullptr if the name is not known, or the function doesn't take a
	// VkPhysicalDevice as the first argument. All functions that have a VkPhysicalDevice as first
	// argument are instance function, except for vkGetPhysicalDeviceToolPropertiesEXT which seems
	// to have been miscategorized as a device extension when it was made. So we special case that
	// funcion.
	std::string name = pName;
	if(name == "vkGetPhysicalDeviceToolPropertiesEXT")
	{
		return reinterpret_cast<PFN_vkVoidFunction>(vkGetPhysicalDeviceToolPropertiesEXT);
	}

	auto instanceFunction = instanceFunctionPointers.find(name);
	if(instanceFunction == instanceFunctionPointers.end())
	{
		return nullptr;
	}

	if(!instanceFunction->second.isPhysicalDeviceFn)
	{
		return nullptr;
	}

	return instanceFunction->second.pfn;
}

PFN_vkVoidFunction GetDeviceProcAddr(Device *device, const char *pName)
{
	auto deviceFunction = deviceFunctionPointers.find(std::string(pName));
	if(deviceFunction != deviceFunctionPointers.end())
	{
		return deviceFunction->second;
	}

	for(const auto &deviceExtensionFunctions : deviceExtensionFunctionPointers)
	{
		if(device->hasExtension(deviceExtensionFunctions.first))
		{
			deviceFunction = deviceExtensionFunctions.second.find(std::string(pName));
			if(deviceFunction != deviceExtensionFunctions.second.end())
			{
				return deviceFunction->second;
			}
		}
	}

	return nullptr;
}

}  // namespace vk

#ifdef __ANDROID__

extern "C" hwvulkan_module_t HAL_MODULE_INFO_SYM;

namespace {

int CloseDevice(struct hw_device_t *)
{
	return 0;
}

hwvulkan_device_t hal_device = {
	.common = {
	    .tag = HARDWARE_DEVICE_TAG,
	    .version = HWVULKAN_DEVICE_API_VERSION_0_1,
	    .module = &HAL_MODULE_INFO_SYM.common,
	    .close = CloseDevice,
	},
	.EnumerateInstanceExtensionProperties = vkEnumerateInstanceExtensionProperties,
	.CreateInstance = vkCreateInstance,
	.GetInstanceProcAddr = vkGetInstanceProcAddr,
};

int OpenDevice(const hw_module_t *module, const char *id, hw_device_t **device)
{
	if(strcmp(id, HWVULKAN_DEVICE_0) != 0) return -ENOENT;
	*device = &hal_device.common;
	return 0;
}

hw_module_methods_t module_methods = { .open = OpenDevice };

}  // namespace

extern "C" hwvulkan_module_t HAL_MODULE_INFO_SYM = {
	.common = {
	    .tag = HARDWARE_MODULE_TAG,
	    .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
	    .hal_api_version = HARDWARE_HAL_API_VERSION,
	    .id = HWVULKAN_HARDWARE_MODULE_ID,
	    .name = "Swiftshader Pastel",
	    .author = "Google",
	    .methods = &module_methods,
	}
};

#endif  // __ANDROID__

#if VK_USE_PLATFORM_FUCHSIA
PFN_vkConnectToService vk::icdFuchsiaServiceConnectCallback = nullptr;
#endif
