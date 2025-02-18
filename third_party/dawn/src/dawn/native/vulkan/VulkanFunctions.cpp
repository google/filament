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

#include "dawn/native/vulkan/VulkanFunctions.h"

#include <string>
#include <utility>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native::vulkan {

namespace {

#if DAWN_NO_SANITIZE_VK_FN

template <typename F>
struct AsVkNoSanitizeFn;

// SwiftShader does not export function pointer type information.
// So, when fuzzing with UBSAN, fuzzers break whenever calling
// a vk* function since it thinks the type of the function pointer
// does not match. Context: crbug.com/1296934.

// Workaround this problem by proxying through a std::function
// in UBSAN builds. The std::function delegates to a Call method
// which does the same cast of the function pointer type, however
// the Call method is tagged with
// `__attribute__((no_sanitize("function")))` to silence the error.
template <typename R, typename... Args>
struct AsVkNoSanitizeFn<R(VKAPI_PTR*)(Args...)> {
    auto operator()(void(VKAPI_PTR* addr)()) {
        return [addr](Args&&... args) -> R { return Call(addr, std::forward<Args>(args)...); };
    }

  private:
    __attribute__((no_sanitize("function"))) static R Call(void(VKAPI_PTR* addr)(),
                                                           Args&&... args) {
        return reinterpret_cast<R(VKAPI_PTR*)(Args...)>(addr)(std::forward<Args>(args)...);
    }
};
template <typename F>
auto AsVkFn(void(VKAPI_PTR* addr)()) {
    return AsVkNoSanitizeFn<F>{}(addr);
}

#else

template <typename F>
F AsVkFn(void(VKAPI_PTR* addr)()) {
    return reinterpret_cast<F>(addr);
}

#endif

}  // anonymous namespace

#define GET_GLOBAL_PROC(name)                                                        \
    do {                                                                             \
        name = AsVkFn<PFN_vk##name>(GetInstanceProcAddr(nullptr, "vk" #name));       \
        if (name == nullptr) {                                                       \
            return DAWN_INTERNAL_ERROR(std::string("Couldn't get proc vk") + #name); \
        }                                                                            \
    } while (0)

MaybeError VulkanFunctions::LoadGlobalProcs(const DynamicLib& vulkanLib) {
    if (!vulkanLib.GetProc(&GetInstanceProcAddr, "vkGetInstanceProcAddr")) {
        return DAWN_INTERNAL_ERROR("Couldn't get vkGetInstanceProcAddr");
    }

    GET_GLOBAL_PROC(CreateInstance);
    GET_GLOBAL_PROC(EnumerateInstanceExtensionProperties);
    GET_GLOBAL_PROC(EnumerateInstanceLayerProperties);

    // Is not available in Vulkan 1.0, so allow nullptr
    EnumerateInstanceVersion = AsVkFn<PFN_vkEnumerateInstanceVersion>(
        GetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

    return {};
}

#define GET_INSTANCE_PROC_BASE(name, procName)                                           \
    do {                                                                                 \
        name = AsVkFn<PFN_vk##name>(GetInstanceProcAddr(instance, "vk" #procName));      \
        if (name == nullptr) {                                                           \
            return DAWN_INTERNAL_ERROR(std::string("Couldn't get proc vk") + #procName); \
        }                                                                                \
    } while (0)

#define GET_INSTANCE_PROC(name) GET_INSTANCE_PROC_BASE(name, name)
#define GET_INSTANCE_PROC_VENDOR(name, vendor) GET_INSTANCE_PROC_BASE(name, name##vendor)

MaybeError VulkanFunctions::LoadInstanceProcs(VkInstance instance,
                                              const VulkanGlobalInfo& globalInfo) {
    // Load this proc first so that we can destroy the instance even if some other
    // GET_INSTANCE_PROC fails
    GET_INSTANCE_PROC(DestroyInstance);

    GET_INSTANCE_PROC(CreateDevice);
    GET_INSTANCE_PROC(DestroyDevice);
    GET_INSTANCE_PROC(EnumerateDeviceExtensionProperties);
    GET_INSTANCE_PROC(EnumerateDeviceLayerProperties);
    GET_INSTANCE_PROC(EnumeratePhysicalDevices);
    GET_INSTANCE_PROC(GetDeviceProcAddr);
    GET_INSTANCE_PROC(GetPhysicalDeviceFeatures);
    GET_INSTANCE_PROC(GetPhysicalDeviceFormatProperties);
    GET_INSTANCE_PROC(GetPhysicalDeviceImageFormatProperties);
    GET_INSTANCE_PROC(GetPhysicalDeviceMemoryProperties);
    GET_INSTANCE_PROC(GetPhysicalDeviceProperties);
    GET_INSTANCE_PROC(GetPhysicalDeviceQueueFamilyProperties);
    GET_INSTANCE_PROC(GetPhysicalDeviceSparseImageFormatProperties);

    if (globalInfo.HasExt(InstanceExt::DebugUtils)) {
        GET_INSTANCE_PROC(CmdBeginDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(CmdEndDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(CmdInsertDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(CreateDebugUtilsMessengerEXT);
        GET_INSTANCE_PROC(DestroyDebugUtilsMessengerEXT);
        GET_INSTANCE_PROC(QueueBeginDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(QueueEndDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(QueueInsertDebugUtilsLabelEXT);
        GET_INSTANCE_PROC(SetDebugUtilsObjectNameEXT);
        GET_INSTANCE_PROC(SetDebugUtilsObjectTagEXT);
        GET_INSTANCE_PROC(SubmitDebugUtilsMessageEXT);
    }

    // Vulkan 1.1 is not required to report promoted extensions from 1.0 and is not required to
    // support the vendor entrypoint in GetProcAddress.
    if (globalInfo.apiVersion >= VK_API_VERSION_1_1) {
        GET_INSTANCE_PROC(GetPhysicalDeviceExternalBufferProperties);
    } else if (globalInfo.HasExt(InstanceExt::ExternalMemoryCapabilities)) {
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceExternalBufferProperties, KHR);
    }

    if (globalInfo.apiVersion >= VK_API_VERSION_1_1) {
        GET_INSTANCE_PROC(GetPhysicalDeviceExternalSemaphoreProperties);
    } else if (globalInfo.HasExt(InstanceExt::ExternalSemaphoreCapabilities)) {
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceExternalSemaphoreProperties, KHR);
    }

    if (globalInfo.apiVersion >= VK_API_VERSION_1_1) {
        GET_INSTANCE_PROC(GetPhysicalDeviceFeatures2);
        GET_INSTANCE_PROC(GetPhysicalDeviceProperties2);
        GET_INSTANCE_PROC(GetPhysicalDeviceFormatProperties2);
        GET_INSTANCE_PROC(GetPhysicalDeviceImageFormatProperties2);
        GET_INSTANCE_PROC(GetPhysicalDeviceQueueFamilyProperties2);
        GET_INSTANCE_PROC(GetPhysicalDeviceMemoryProperties2);
        GET_INSTANCE_PROC(GetPhysicalDeviceSparseImageFormatProperties2);
    } else if (globalInfo.HasExt(InstanceExt::GetPhysicalDeviceProperties2)) {
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceFeatures2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceProperties2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceFormatProperties2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceImageFormatProperties2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceQueueFamilyProperties2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceMemoryProperties2, KHR);
        GET_INSTANCE_PROC_VENDOR(GetPhysicalDeviceSparseImageFormatProperties2, KHR);
    }

    if (globalInfo.HasExt(InstanceExt::Surface)) {
        GET_INSTANCE_PROC(DestroySurfaceKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceSurfaceSupportKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
    }

#if defined(VK_USE_PLATFORM_FUCHSIA)
    if (globalInfo.HasExt(InstanceExt::FuchsiaImagePipeSurface)) {
        GET_INSTANCE_PROC(CreateImagePipeSurfaceFUCHSIA);
    }
#endif  // defined(VK_USE_PLATFORM_FUCHSIA)

#if defined(DAWN_ENABLE_BACKEND_METAL)
    if (globalInfo.HasExt(InstanceExt::MetalSurface)) {
        GET_INSTANCE_PROC(CreateMetalSurfaceEXT);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_USE_WAYLAND)
    if (globalInfo.HasExt(InstanceExt::WaylandSurface)) {
        GET_INSTANCE_PROC(CreateWaylandSurfaceKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceWaylandPresentationSupportKHR);
    }
#endif  // defined(DAWN_USE_WAYLAND)

#if DAWN_PLATFORM_IS(WINDOWS)
    if (globalInfo.HasExt(InstanceExt::Win32Surface)) {
        GET_INSTANCE_PROC(CreateWin32SurfaceKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceWin32PresentationSupportKHR);
    }
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if DAWN_PLATFORM_IS(ANDROID)
    if (globalInfo.HasExt(InstanceExt::AndroidSurface)) {
        GET_INSTANCE_PROC(CreateAndroidSurfaceKHR);
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if defined(DAWN_USE_X11)
    if (globalInfo.HasExt(InstanceExt::XlibSurface)) {
        GET_INSTANCE_PROC(CreateXlibSurfaceKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceXlibPresentationSupportKHR);
    }
    if (globalInfo.HasExt(InstanceExt::XcbSurface)) {
        GET_INSTANCE_PROC(CreateXcbSurfaceKHR);
        GET_INSTANCE_PROC(GetPhysicalDeviceXcbPresentationSupportKHR);
    }
#endif  // defined(DAWN_USE_X11)
    return {};
}

#define GET_DEVICE_PROC(name)                                                        \
    do {                                                                             \
        name = AsVkFn<PFN_vk##name>(GetDeviceProcAddr(device, "vk" #name));          \
        if (name == nullptr) {                                                       \
            return DAWN_INTERNAL_ERROR(std::string("Couldn't get proc vk") + #name); \
        }                                                                            \
    } while (0)

MaybeError VulkanFunctions::LoadDeviceProcs(VkDevice device, const VulkanDeviceInfo& deviceInfo) {
    GET_DEVICE_PROC(AllocateCommandBuffers);
    GET_DEVICE_PROC(AllocateDescriptorSets);
    GET_DEVICE_PROC(AllocateMemory);
    GET_DEVICE_PROC(BeginCommandBuffer);
    GET_DEVICE_PROC(BindBufferMemory);
    GET_DEVICE_PROC(BindImageMemory);
    GET_DEVICE_PROC(CmdBeginQuery);
    GET_DEVICE_PROC(CmdBeginRenderPass);
    GET_DEVICE_PROC(CmdBindDescriptorSets);
    GET_DEVICE_PROC(CmdBindIndexBuffer);
    GET_DEVICE_PROC(CmdBindPipeline);
    GET_DEVICE_PROC(CmdBindVertexBuffers);
    GET_DEVICE_PROC(CmdBlitImage);
    GET_DEVICE_PROC(CmdClearAttachments);
    GET_DEVICE_PROC(CmdClearColorImage);
    GET_DEVICE_PROC(CmdClearDepthStencilImage);
    GET_DEVICE_PROC(CmdCopyBuffer);
    GET_DEVICE_PROC(CmdCopyBufferToImage);
    GET_DEVICE_PROC(CmdCopyImage);
    GET_DEVICE_PROC(CmdCopyImageToBuffer);
    GET_DEVICE_PROC(CmdCopyQueryPoolResults);
    GET_DEVICE_PROC(CmdDispatch);
    GET_DEVICE_PROC(CmdDispatchIndirect);
    GET_DEVICE_PROC(CmdDraw);
    GET_DEVICE_PROC(CmdDrawIndexed);
    GET_DEVICE_PROC(CmdDrawIndexedIndirect);
    GET_DEVICE_PROC(CmdDrawIndirect);
    GET_DEVICE_PROC(CmdEndQuery);
    GET_DEVICE_PROC(CmdEndRenderPass);
    GET_DEVICE_PROC(CmdExecuteCommands);
    GET_DEVICE_PROC(CmdFillBuffer);
    GET_DEVICE_PROC(CmdNextSubpass);
    GET_DEVICE_PROC(CmdPipelineBarrier);
    GET_DEVICE_PROC(CmdPushConstants);
    GET_DEVICE_PROC(CmdResetEvent);
    GET_DEVICE_PROC(CmdResetQueryPool);
    GET_DEVICE_PROC(CmdResolveImage);
    GET_DEVICE_PROC(CmdSetBlendConstants);
    GET_DEVICE_PROC(CmdSetDepthBias);
    GET_DEVICE_PROC(CmdSetDepthBounds);
    GET_DEVICE_PROC(CmdSetEvent);
    GET_DEVICE_PROC(CmdSetLineWidth);
    GET_DEVICE_PROC(CmdSetScissor);
    GET_DEVICE_PROC(CmdSetStencilCompareMask);
    GET_DEVICE_PROC(CmdSetStencilReference);
    GET_DEVICE_PROC(CmdSetStencilWriteMask);
    GET_DEVICE_PROC(CmdSetViewport);
    GET_DEVICE_PROC(CmdUpdateBuffer);
    GET_DEVICE_PROC(CmdWaitEvents);
    GET_DEVICE_PROC(CmdWriteTimestamp);
    GET_DEVICE_PROC(CreateBuffer);
    GET_DEVICE_PROC(CreateBufferView);
    GET_DEVICE_PROC(CreateCommandPool);
    GET_DEVICE_PROC(CreateComputePipelines);
    GET_DEVICE_PROC(CreateDescriptorPool);
    GET_DEVICE_PROC(CreateDescriptorSetLayout);
    GET_DEVICE_PROC(CreateEvent);
    GET_DEVICE_PROC(CreateFence);
    GET_DEVICE_PROC(CreateFramebuffer);
    GET_DEVICE_PROC(CreateGraphicsPipelines);
    GET_DEVICE_PROC(CreateImage);
    GET_DEVICE_PROC(CreateImageView);
    GET_DEVICE_PROC(CreatePipelineCache);
    GET_DEVICE_PROC(CreatePipelineLayout);
    GET_DEVICE_PROC(CreateQueryPool);
    GET_DEVICE_PROC(CreateRenderPass);
    GET_DEVICE_PROC(CreateSampler);
    GET_DEVICE_PROC(CreateSemaphore);
    GET_DEVICE_PROC(CreateShaderModule);
    GET_DEVICE_PROC(DestroyBuffer);
    GET_DEVICE_PROC(DestroyBufferView);
    GET_DEVICE_PROC(DestroyCommandPool);
    GET_DEVICE_PROC(DestroyDescriptorPool);
    GET_DEVICE_PROC(DestroyDescriptorSetLayout);
    GET_DEVICE_PROC(DestroyEvent);
    GET_DEVICE_PROC(DestroyFence);
    GET_DEVICE_PROC(DestroyFramebuffer);
    GET_DEVICE_PROC(DestroyImage);
    GET_DEVICE_PROC(DestroyImageView);
    GET_DEVICE_PROC(DestroyPipeline);
    GET_DEVICE_PROC(DestroyPipelineCache);
    GET_DEVICE_PROC(DestroyPipelineLayout);
    GET_DEVICE_PROC(DestroyQueryPool);
    GET_DEVICE_PROC(DestroyRenderPass);
    GET_DEVICE_PROC(DestroySampler);
    GET_DEVICE_PROC(DestroySemaphore);
    GET_DEVICE_PROC(DestroyShaderModule);
    GET_DEVICE_PROC(DeviceWaitIdle);
    GET_DEVICE_PROC(EndCommandBuffer);
    GET_DEVICE_PROC(FlushMappedMemoryRanges);
    GET_DEVICE_PROC(FreeCommandBuffers);
    GET_DEVICE_PROC(FreeDescriptorSets);
    GET_DEVICE_PROC(FreeMemory);
    GET_DEVICE_PROC(GetBufferMemoryRequirements);
    GET_DEVICE_PROC(GetDeviceMemoryCommitment);
    GET_DEVICE_PROC(GetDeviceQueue);
    GET_DEVICE_PROC(GetEventStatus);
    GET_DEVICE_PROC(GetFenceStatus);
    GET_DEVICE_PROC(GetImageMemoryRequirements);
    GET_DEVICE_PROC(GetImageSparseMemoryRequirements);
    GET_DEVICE_PROC(GetImageSubresourceLayout);
    GET_DEVICE_PROC(GetPipelineCacheData);
    GET_DEVICE_PROC(GetQueryPoolResults);
    GET_DEVICE_PROC(GetRenderAreaGranularity);
    GET_DEVICE_PROC(InvalidateMappedMemoryRanges);
    GET_DEVICE_PROC(MapMemory);
    GET_DEVICE_PROC(MergePipelineCaches);
    GET_DEVICE_PROC(QueueBindSparse);
    GET_DEVICE_PROC(QueueSubmit);
    GET_DEVICE_PROC(QueueWaitIdle);
    GET_DEVICE_PROC(ResetCommandBuffer);
    GET_DEVICE_PROC(ResetCommandPool);
    GET_DEVICE_PROC(ResetDescriptorPool);
    GET_DEVICE_PROC(ResetEvent);
    GET_DEVICE_PROC(ResetFences);
    GET_DEVICE_PROC(SetEvent);
    GET_DEVICE_PROC(UnmapMemory);
    GET_DEVICE_PROC(UpdateDescriptorSets);
    GET_DEVICE_PROC(WaitForFences);

    if (deviceInfo.HasExt(DeviceExt::ExternalMemoryFD)) {
        GET_DEVICE_PROC(GetMemoryFdKHR);
        GET_DEVICE_PROC(GetMemoryFdPropertiesKHR);
    }

    if (deviceInfo.HasExt(DeviceExt::ExternalMemoryHost)) {
        GET_DEVICE_PROC(GetMemoryHostPointerPropertiesEXT);
    }

    if (deviceInfo.HasExt(DeviceExt::ExternalSemaphoreFD)) {
        GET_DEVICE_PROC(ImportSemaphoreFdKHR);
        GET_DEVICE_PROC(GetSemaphoreFdKHR);
    }

    if (deviceInfo.HasExt(DeviceExt::Swapchain)) {
        GET_DEVICE_PROC(CreateSwapchainKHR);
        GET_DEVICE_PROC(DestroySwapchainKHR);
        GET_DEVICE_PROC(GetSwapchainImagesKHR);
        GET_DEVICE_PROC(AcquireNextImageKHR);
        GET_DEVICE_PROC(QueuePresentKHR);
    }

    if (deviceInfo.HasExt(DeviceExt::GetMemoryRequirements2)) {
        GET_DEVICE_PROC(GetBufferMemoryRequirements2);
        GET_DEVICE_PROC(GetImageMemoryRequirements2);
        GET_DEVICE_PROC(GetImageSparseMemoryRequirements2);
    }

    if (deviceInfo.HasExt(DeviceExt::SamplerYCbCrConversion)) {
        GET_DEVICE_PROC(CreateSamplerYcbcrConversion);
        GET_DEVICE_PROC(DestroySamplerYcbcrConversion);
    }

    if (deviceInfo.HasExt(DeviceExt::DrawIndirectCount)) {
        GET_DEVICE_PROC(CmdDrawIndirectCountKHR);
        GET_DEVICE_PROC(CmdDrawIndexedIndirectCountKHR);
    }

#if VK_USE_PLATFORM_FUCHSIA
    if (deviceInfo.HasExt(DeviceExt::ExternalMemoryZirconHandle)) {
        GET_DEVICE_PROC(GetMemoryZirconHandleFUCHSIA);
        GET_DEVICE_PROC(GetMemoryZirconHandlePropertiesFUCHSIA);
    }

    if (deviceInfo.HasExt(DeviceExt::ExternalSemaphoreZirconHandle)) {
        GET_DEVICE_PROC(ImportSemaphoreZirconHandleFUCHSIA);
        GET_DEVICE_PROC(GetSemaphoreZirconHandleFUCHSIA);
    }
#endif

#if DAWN_PLATFORM_IS(ANDROID)
    if (deviceInfo.HasExt(DeviceExt::ExternalMemoryAndroidHardwareBuffer)) {
        GET_DEVICE_PROC(GetAndroidHardwareBufferPropertiesANDROID);
        GET_DEVICE_PROC(GetMemoryAndroidHardwareBufferANDROID);
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

    return {};
}

}  // namespace dawn::native::vulkan
