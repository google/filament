// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_VULKAN_VULKANFUNCTIONS_H_
#define SRC_DAWN_NATIVE_VULKAN_VULKANFUNCTIONS_H_

#include "dawn/common/Compiler.h"
#include "dawn/common/vulkan_platform.h"

#include "dawn/native/Error.h"

namespace dawn {
class DynamicLib;
}  // namespace dawn

namespace dawn::native::vulkan {

struct VulkanGlobalInfo;
struct VulkanDeviceInfo;

#if defined(UNDEFINED_SANITIZER) && DAWN_COMPILER_IS(CLANG)
#define DAWN_NO_SANITIZE_VK_FN 1
#else
#define DAWN_NO_SANITIZE_VK_FN 0
#endif

template <typename F>
struct VkFnImpl;

// Override the type of Vulkan functions to be a bound std::function if
// DAWN_NO_SANITIZE_VK_FN is set. See comment at AsVkNoSanitizeFn in VulkanFunctions.cpp
// for more information.
template <typename R, typename... Args>
struct VkFnImpl<R(VKAPI_PTR*)(Args...)> {
#if DAWN_NO_SANITIZE_VK_FN
    using type = std::function<R(Args...)>;
#else
    using type = R(VKAPI_PTR*)(Args...);
#endif
};

template <typename F>
using VkFn = typename VkFnImpl<F>::type;

// Stores the Vulkan entry points. Also loads them from the dynamic library
// and the vkGet*ProcAddress entry points.
struct VulkanFunctions {
    MaybeError LoadGlobalProcs(const DynamicLib& vulkanLib);
    MaybeError LoadInstanceProcs(VkInstance instance, const VulkanGlobalInfo& globalInfo);
    MaybeError LoadDeviceProcs(VkInstance instance,
                               VkDevice device,
                               const VulkanDeviceInfo& deviceInfo);

    // ---------- Global procs

    // Initial proc from which we can get all the others
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;

    VkFn<PFN_vkCreateInstance> CreateInstance = nullptr;
    VkFn<PFN_vkEnumerateInstanceExtensionProperties> EnumerateInstanceExtensionProperties = nullptr;
    VkFn<PFN_vkEnumerateInstanceLayerProperties> EnumerateInstanceLayerProperties = nullptr;
    // DestroyInstance isn't technically a global proc but we want to be able to use it
    // before querying the instance procs in case we need to error out during initialization.
    VkFn<PFN_vkDestroyInstance> DestroyInstance = nullptr;

    // Core Vulkan 1.1
    VkFn<PFN_vkEnumerateInstanceVersion> EnumerateInstanceVersion = nullptr;

    // ---------- Instance procs

    // Core Vulkan 1.0
    VkFn<PFN_vkCreateDevice> CreateDevice = nullptr;
    VkFn<PFN_vkEnumerateDeviceExtensionProperties> EnumerateDeviceExtensionProperties = nullptr;
    VkFn<PFN_vkEnumerateDeviceLayerProperties> EnumerateDeviceLayerProperties = nullptr;
    VkFn<PFN_vkEnumeratePhysicalDevices> EnumeratePhysicalDevices = nullptr;
    VkFn<PFN_vkGetDeviceProcAddr> GetDeviceProcAddr = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceFeatures> GetPhysicalDeviceFeatures = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceFormatProperties> GetPhysicalDeviceFormatProperties = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceImageFormatProperties> GetPhysicalDeviceImageFormatProperties =
        nullptr;
    VkFn<PFN_vkGetPhysicalDeviceMemoryProperties> GetPhysicalDeviceMemoryProperties = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceProperties> GetPhysicalDeviceProperties = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties =
        nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>
        GetPhysicalDeviceSparseImageFormatProperties = nullptr;
    // Not technically an instance proc but we want to be able to use it as soon as the
    // device is created.
    VkFn<PFN_vkDestroyDevice> DestroyDevice = nullptr;

    // VK_EXT_debug_utils
    VkFn<PFN_vkCmdBeginDebugUtilsLabelEXT> CmdBeginDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkCmdEndDebugUtilsLabelEXT> CmdEndDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkCmdInsertDebugUtilsLabelEXT> CmdInsertDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkCreateDebugUtilsMessengerEXT> CreateDebugUtilsMessengerEXT = nullptr;
    VkFn<PFN_vkDestroyDebugUtilsMessengerEXT> DestroyDebugUtilsMessengerEXT = nullptr;
    VkFn<PFN_vkQueueBeginDebugUtilsLabelEXT> QueueBeginDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkQueueEndDebugUtilsLabelEXT> QueueEndDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkQueueInsertDebugUtilsLabelEXT> QueueInsertDebugUtilsLabelEXT = nullptr;
    VkFn<PFN_vkSetDebugUtilsObjectNameEXT> SetDebugUtilsObjectNameEXT = nullptr;
    VkFn<PFN_vkSetDebugUtilsObjectTagEXT> SetDebugUtilsObjectTagEXT = nullptr;
    VkFn<PFN_vkSubmitDebugUtilsMessageEXT> SubmitDebugUtilsMessageEXT = nullptr;

    // VK_KHR_surface
    VkFn<PFN_vkDestroySurfaceKHR> DestroySurfaceKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSurfaceSupportKHR> GetPhysicalDeviceSurfaceSupportKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR> GetPhysicalDeviceSurfaceCapabilitiesKHR =
        nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR> GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR> GetPhysicalDeviceSurfacePresentModesKHR =
        nullptr;

    // Core Vulkan 1.1 promoted extensions, set if either the core version or the extension is
    // present.

    // VK_KHR_external_memory_capabilities
    VkFn<PFN_vkGetPhysicalDeviceExternalBufferProperties>
        GetPhysicalDeviceExternalBufferProperties = nullptr;

    // VK_KHR_external_semaphore_capabilities
    VkFn<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>
        GetPhysicalDeviceExternalSemaphoreProperties = nullptr;

    // VK_KHR_get_physical_device_properties2
    VkFn<PFN_vkGetPhysicalDeviceFeatures2> GetPhysicalDeviceFeatures2 = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceProperties2> GetPhysicalDeviceProperties2 = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceFormatProperties2> GetPhysicalDeviceFormatProperties2 = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceImageFormatProperties2> GetPhysicalDeviceImageFormatProperties2 =
        nullptr;
    VkFn<PFN_vkGetPhysicalDeviceQueueFamilyProperties2> GetPhysicalDeviceQueueFamilyProperties2 =
        nullptr;
    VkFn<PFN_vkGetPhysicalDeviceMemoryProperties2> GetPhysicalDeviceMemoryProperties2 = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2>
        GetPhysicalDeviceSparseImageFormatProperties2 = nullptr;

#if defined(VK_USE_PLATFORM_FUCHSIA)
    // FUCHSIA_image_pipe_surface
    VkFn<PFN_vkCreateImagePipeSurfaceFUCHSIA> CreateImagePipeSurfaceFUCHSIA = nullptr;
#endif  // defined(VK_USE_PLATFORM_FUCHSIA)

#if defined(DAWN_ENABLE_BACKEND_METAL)
    // EXT_metal_surface
    VkFn<PFN_vkCreateMetalSurfaceEXT> CreateMetalSurfaceEXT = nullptr;
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_USE_WAYLAND)
    // KHR_wayland_surface
    PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
        GetPhysicalDeviceWaylandPresentationSupportKHR = nullptr;
#endif  // defined(DAWN_USE_WAYLAND)

#if DAWN_PLATFORM_IS(WINDOWS)
    // KHR_win32_surface
    VkFn<PFN_vkCreateWin32SurfaceKHR> CreateWin32SurfaceKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>
        GetPhysicalDeviceWin32PresentationSupportKHR = nullptr;
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if DAWN_PLATFORM_IS(ANDROID)
    // KHR_android_surface
    VkFn<PFN_vkCreateAndroidSurfaceKHR> CreateAndroidSurfaceKHR = nullptr;

    // VK_ANDROID_external_memory_android_hardware_buffer
    VkFn<PFN_vkGetAndroidHardwareBufferPropertiesANDROID>
        GetAndroidHardwareBufferPropertiesANDROID = nullptr;
    VkFn<PFN_vkGetMemoryAndroidHardwareBufferANDROID> GetMemoryAndroidHardwareBufferANDROID =
        nullptr;
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if defined(DAWN_USE_X11)
    // KHR_xlib_surface
    VkFn<PFN_vkCreateXlibSurfaceKHR> CreateXlibSurfaceKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>
        GetPhysicalDeviceXlibPresentationSupportKHR = nullptr;

    // KHR_xcb_surface
    VkFn<PFN_vkCreateXcbSurfaceKHR> CreateXcbSurfaceKHR = nullptr;
    VkFn<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>
        GetPhysicalDeviceXcbPresentationSupportKHR = nullptr;
#endif  // defined(DAWN_USE_X11)

    // ---------- Instance procs for device extensions

    // VK_KHR_cooperative_matrix
    VkFn<PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR>
        GetPhysicalDeviceCooperativeMatrixPropertiesKHR = nullptr;

    // ---------- Device procs

    // Core Vulkan 1.0
    VkFn<PFN_vkAllocateCommandBuffers> AllocateCommandBuffers = nullptr;
    VkFn<PFN_vkAllocateDescriptorSets> AllocateDescriptorSets = nullptr;
    VkFn<PFN_vkAllocateMemory> AllocateMemory = nullptr;
    VkFn<PFN_vkBeginCommandBuffer> BeginCommandBuffer = nullptr;
    VkFn<PFN_vkBindBufferMemory> BindBufferMemory = nullptr;
    VkFn<PFN_vkBindImageMemory> BindImageMemory = nullptr;
    VkFn<PFN_vkCmdBeginQuery> CmdBeginQuery = nullptr;
    VkFn<PFN_vkCmdBeginRenderPass> CmdBeginRenderPass = nullptr;
    VkFn<PFN_vkCmdBindDescriptorSets> CmdBindDescriptorSets = nullptr;
    VkFn<PFN_vkCmdBindIndexBuffer> CmdBindIndexBuffer = nullptr;
    VkFn<PFN_vkCmdBindPipeline> CmdBindPipeline = nullptr;
    VkFn<PFN_vkCmdBindVertexBuffers> CmdBindVertexBuffers = nullptr;
    VkFn<PFN_vkCmdBlitImage> CmdBlitImage = nullptr;
    VkFn<PFN_vkCmdClearAttachments> CmdClearAttachments = nullptr;
    VkFn<PFN_vkCmdClearColorImage> CmdClearColorImage = nullptr;
    VkFn<PFN_vkCmdClearDepthStencilImage> CmdClearDepthStencilImage = nullptr;
    VkFn<PFN_vkCmdCopyBuffer> CmdCopyBuffer = nullptr;
    VkFn<PFN_vkCmdCopyBufferToImage> CmdCopyBufferToImage = nullptr;
    VkFn<PFN_vkCmdCopyImage> CmdCopyImage = nullptr;
    VkFn<PFN_vkCmdCopyImageToBuffer> CmdCopyImageToBuffer = nullptr;
    VkFn<PFN_vkCmdCopyQueryPoolResults> CmdCopyQueryPoolResults = nullptr;
    VkFn<PFN_vkCmdDispatch> CmdDispatch = nullptr;
    VkFn<PFN_vkCmdDispatchIndirect> CmdDispatchIndirect = nullptr;
    VkFn<PFN_vkCmdDraw> CmdDraw = nullptr;
    VkFn<PFN_vkCmdDrawIndexed> CmdDrawIndexed = nullptr;
    VkFn<PFN_vkCmdDrawIndexedIndirect> CmdDrawIndexedIndirect = nullptr;
    VkFn<PFN_vkCmdDrawIndirect> CmdDrawIndirect = nullptr;
    VkFn<PFN_vkCmdEndQuery> CmdEndQuery = nullptr;
    VkFn<PFN_vkCmdEndRenderPass> CmdEndRenderPass = nullptr;
    VkFn<PFN_vkCmdExecuteCommands> CmdExecuteCommands = nullptr;
    VkFn<PFN_vkCmdFillBuffer> CmdFillBuffer = nullptr;
    VkFn<PFN_vkCmdNextSubpass> CmdNextSubpass = nullptr;
    VkFn<PFN_vkCmdPipelineBarrier> CmdPipelineBarrier = nullptr;
    VkFn<PFN_vkCmdPushConstants> CmdPushConstants = nullptr;
    VkFn<PFN_vkCmdResetEvent> CmdResetEvent = nullptr;
    VkFn<PFN_vkCmdResetQueryPool> CmdResetQueryPool = nullptr;
    VkFn<PFN_vkCmdResolveImage> CmdResolveImage = nullptr;
    VkFn<PFN_vkCmdSetBlendConstants> CmdSetBlendConstants = nullptr;
    VkFn<PFN_vkCmdSetDepthBias> CmdSetDepthBias = nullptr;
    VkFn<PFN_vkCmdSetDepthBounds> CmdSetDepthBounds = nullptr;
    VkFn<PFN_vkCmdSetEvent> CmdSetEvent = nullptr;
    VkFn<PFN_vkCmdSetLineWidth> CmdSetLineWidth = nullptr;
    VkFn<PFN_vkCmdSetScissor> CmdSetScissor = nullptr;
    VkFn<PFN_vkCmdSetStencilCompareMask> CmdSetStencilCompareMask = nullptr;
    VkFn<PFN_vkCmdSetStencilReference> CmdSetStencilReference = nullptr;
    VkFn<PFN_vkCmdSetStencilWriteMask> CmdSetStencilWriteMask = nullptr;
    VkFn<PFN_vkCmdSetViewport> CmdSetViewport = nullptr;
    VkFn<PFN_vkCmdUpdateBuffer> CmdUpdateBuffer = nullptr;
    VkFn<PFN_vkCmdWaitEvents> CmdWaitEvents = nullptr;
    VkFn<PFN_vkCmdWriteTimestamp> CmdWriteTimestamp = nullptr;
    VkFn<PFN_vkCreateBuffer> CreateBuffer = nullptr;
    VkFn<PFN_vkCreateBufferView> CreateBufferView = nullptr;
    VkFn<PFN_vkCreateCommandPool> CreateCommandPool = nullptr;
    VkFn<PFN_vkCreateComputePipelines> CreateComputePipelines = nullptr;
    VkFn<PFN_vkCreateDescriptorPool> CreateDescriptorPool = nullptr;
    VkFn<PFN_vkCreateDescriptorSetLayout> CreateDescriptorSetLayout = nullptr;
    VkFn<PFN_vkCreateEvent> CreateEvent = nullptr;
    VkFn<PFN_vkCreateFence> CreateFence = nullptr;
    VkFn<PFN_vkCreateFramebuffer> CreateFramebuffer = nullptr;
    VkFn<PFN_vkCreateGraphicsPipelines> CreateGraphicsPipelines = nullptr;
    VkFn<PFN_vkCreateImage> CreateImage = nullptr;
    VkFn<PFN_vkCreateImageView> CreateImageView = nullptr;
    VkFn<PFN_vkCreatePipelineCache> CreatePipelineCache = nullptr;
    VkFn<PFN_vkCreatePipelineLayout> CreatePipelineLayout = nullptr;
    VkFn<PFN_vkCreateQueryPool> CreateQueryPool = nullptr;
    VkFn<PFN_vkCreateRenderPass> CreateRenderPass = nullptr;
    VkFn<PFN_vkCreateSampler> CreateSampler = nullptr;
    VkFn<PFN_vkCreateSemaphore> CreateSemaphore = nullptr;
    VkFn<PFN_vkCreateShaderModule> CreateShaderModule = nullptr;
    VkFn<PFN_vkDestroyBuffer> DestroyBuffer = nullptr;
    VkFn<PFN_vkDestroyBufferView> DestroyBufferView = nullptr;
    VkFn<PFN_vkDestroyCommandPool> DestroyCommandPool = nullptr;
    VkFn<PFN_vkDestroyDescriptorPool> DestroyDescriptorPool = nullptr;
    VkFn<PFN_vkDestroyDescriptorSetLayout> DestroyDescriptorSetLayout = nullptr;
    VkFn<PFN_vkDestroyEvent> DestroyEvent = nullptr;
    VkFn<PFN_vkDestroyFence> DestroyFence = nullptr;
    VkFn<PFN_vkDestroyFramebuffer> DestroyFramebuffer = nullptr;
    VkFn<PFN_vkDestroyImage> DestroyImage = nullptr;
    VkFn<PFN_vkDestroyImageView> DestroyImageView = nullptr;
    VkFn<PFN_vkDestroyPipeline> DestroyPipeline = nullptr;
    VkFn<PFN_vkDestroyPipelineCache> DestroyPipelineCache = nullptr;
    VkFn<PFN_vkDestroyPipelineLayout> DestroyPipelineLayout = nullptr;
    VkFn<PFN_vkDestroyQueryPool> DestroyQueryPool = nullptr;
    VkFn<PFN_vkDestroyRenderPass> DestroyRenderPass = nullptr;
    VkFn<PFN_vkDestroySampler> DestroySampler = nullptr;
    VkFn<PFN_vkDestroySemaphore> DestroySemaphore = nullptr;
    VkFn<PFN_vkDestroyShaderModule> DestroyShaderModule = nullptr;
    VkFn<PFN_vkDeviceWaitIdle> DeviceWaitIdle = nullptr;
    VkFn<PFN_vkEndCommandBuffer> EndCommandBuffer = nullptr;
    VkFn<PFN_vkFlushMappedMemoryRanges> FlushMappedMemoryRanges = nullptr;
    VkFn<PFN_vkFreeCommandBuffers> FreeCommandBuffers = nullptr;
    VkFn<PFN_vkFreeDescriptorSets> FreeDescriptorSets = nullptr;
    VkFn<PFN_vkFreeMemory> FreeMemory = nullptr;
    VkFn<PFN_vkGetBufferMemoryRequirements> GetBufferMemoryRequirements = nullptr;
    VkFn<PFN_vkGetDeviceMemoryCommitment> GetDeviceMemoryCommitment = nullptr;
    VkFn<PFN_vkGetDeviceQueue> GetDeviceQueue = nullptr;
    VkFn<PFN_vkGetEventStatus> GetEventStatus = nullptr;
    VkFn<PFN_vkGetFenceStatus> GetFenceStatus = nullptr;
    VkFn<PFN_vkGetImageMemoryRequirements> GetImageMemoryRequirements = nullptr;
    VkFn<PFN_vkGetImageSparseMemoryRequirements> GetImageSparseMemoryRequirements = nullptr;
    VkFn<PFN_vkGetImageSubresourceLayout> GetImageSubresourceLayout = nullptr;
    VkFn<PFN_vkGetPipelineCacheData> GetPipelineCacheData = nullptr;
    VkFn<PFN_vkGetQueryPoolResults> GetQueryPoolResults = nullptr;
    VkFn<PFN_vkGetRenderAreaGranularity> GetRenderAreaGranularity = nullptr;
    VkFn<PFN_vkInvalidateMappedMemoryRanges> InvalidateMappedMemoryRanges = nullptr;
    VkFn<PFN_vkMapMemory> MapMemory = nullptr;
    VkFn<PFN_vkMergePipelineCaches> MergePipelineCaches = nullptr;
    VkFn<PFN_vkQueueBindSparse> QueueBindSparse = nullptr;
    VkFn<PFN_vkQueueSubmit> QueueSubmit = nullptr;
    VkFn<PFN_vkQueueWaitIdle> QueueWaitIdle = nullptr;
    VkFn<PFN_vkResetCommandBuffer> ResetCommandBuffer = nullptr;
    VkFn<PFN_vkResetCommandPool> ResetCommandPool = nullptr;
    VkFn<PFN_vkResetDescriptorPool> ResetDescriptorPool = nullptr;
    VkFn<PFN_vkResetEvent> ResetEvent = nullptr;
    VkFn<PFN_vkResetFences> ResetFences = nullptr;
    VkFn<PFN_vkSetEvent> SetEvent = nullptr;
    VkFn<PFN_vkUnmapMemory> UnmapMemory = nullptr;
    VkFn<PFN_vkUpdateDescriptorSets> UpdateDescriptorSets = nullptr;
    VkFn<PFN_vkWaitForFences> WaitForFences = nullptr;
    VkFn<PFN_vkCreateSamplerYcbcrConversion> CreateSamplerYcbcrConversion = nullptr;
    VkFn<PFN_vkDestroySamplerYcbcrConversion> DestroySamplerYcbcrConversion = nullptr;

    // VK_KHR_external_memory_fd
    VkFn<PFN_vkGetMemoryFdKHR> GetMemoryFdKHR = nullptr;
    VkFn<PFN_vkGetMemoryFdPropertiesKHR> GetMemoryFdPropertiesKHR = nullptr;

    // VK_EXT_external_memory_host
    VkFn<PFN_vkGetMemoryHostPointerPropertiesEXT> GetMemoryHostPointerPropertiesEXT = nullptr;

    // VK_KHR_external_semaphore_fd
    VkFn<PFN_vkImportSemaphoreFdKHR> ImportSemaphoreFdKHR = nullptr;
    VkFn<PFN_vkGetSemaphoreFdKHR> GetSemaphoreFdKHR = nullptr;

    // VK_KHR_get_memory_requirements2
    VkFn<PFN_vkGetBufferMemoryRequirements2KHR> GetBufferMemoryRequirements2 = nullptr;
    VkFn<PFN_vkGetImageMemoryRequirements2KHR> GetImageMemoryRequirements2 = nullptr;
    VkFn<PFN_vkGetImageSparseMemoryRequirements2KHR> GetImageSparseMemoryRequirements2 = nullptr;

    // VK_KHR_swapchain
    VkFn<PFN_vkCreateSwapchainKHR> CreateSwapchainKHR = nullptr;
    VkFn<PFN_vkDestroySwapchainKHR> DestroySwapchainKHR = nullptr;
    VkFn<PFN_vkGetSwapchainImagesKHR> GetSwapchainImagesKHR = nullptr;
    VkFn<PFN_vkAcquireNextImageKHR> AcquireNextImageKHR = nullptr;
    VkFn<PFN_vkQueuePresentKHR> QueuePresentKHR = nullptr;

    // VK_KHR_draw_indirect_count
    VkFn<PFN_vkCmdDrawIndirectCount> CmdDrawIndirectCountKHR = nullptr;
    VkFn<PFN_vkCmdDrawIndexedIndirectCount> CmdDrawIndexedIndirectCountKHR = nullptr;

#if VK_USE_PLATFORM_FUCHSIA
    // VK_FUCHSIA_external_memory
    VkFn<PFN_vkGetMemoryZirconHandleFUCHSIA> GetMemoryZirconHandleFUCHSIA = nullptr;
    VkFn<PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA> GetMemoryZirconHandlePropertiesFUCHSIA =
        nullptr;

    // VK_FUCHSIA_external_semaphore
    VkFn<PFN_vkImportSemaphoreZirconHandleFUCHSIA> ImportSemaphoreZirconHandleFUCHSIA = nullptr;
    VkFn<PFN_vkGetSemaphoreZirconHandleFUCHSIA> GetSemaphoreZirconHandleFUCHSIA = nullptr;
#endif
};

// Create a wrapper around VkResult in the dawn::native::vulkan namespace. This shadows the
// default VkResult (::VkResult). This ensures that assigning or creating a VkResult from a raw
// ::VkResult uses WrapUnsafe. This makes it clear that users of VkResult must be intentional
// about handling error cases.
class VkResult {
  public:
    constexpr static VkResult WrapUnsafe(::VkResult value) { return VkResult(value); }

    constexpr operator ::VkResult() const { return mValue; }

  private:
    // Private. Use VkResult::WrapUnsafe instead.
    explicit constexpr VkResult(::VkResult value) : mValue(value) {}

    ::VkResult mValue;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_VULKANFUNCTIONS_H_
