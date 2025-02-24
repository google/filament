// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_table_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
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
 ****************************************************************************/

// NOLINTBEGIN
#include "vk_dispatch_table_helper.h"
static VKAPI_ATTR VkResult VKAPI_CALL StubBindBufferMemory2(VkDevice, uint32_t, const VkBindBufferMemoryInfo*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindImageMemory2(VkDevice, uint32_t, const VkBindImageMemoryInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceGroupPeerMemoryFeatures(VkDevice, uint32_t, uint32_t, uint32_t,
                                                                       VkPeerMemoryFeatureFlags*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDeviceMask(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchBase(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceGroups(VkInstance, uint32_t*, VkPhysicalDeviceGroupProperties*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageMemoryRequirements2(VkDevice, const VkImageMemoryRequirementsInfo2*,
                                                                  VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetBufferMemoryRequirements2(VkDevice, const VkBufferMemoryRequirementsInfo2*,
                                                                   VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSparseMemoryRequirements2(VkDevice, const VkImageSparseMemoryRequirementsInfo2*,
                                                                        uint32_t*, VkSparseImageMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFeatures2(VkPhysicalDevice, VkPhysicalDeviceFeatures2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceProperties2(VkPhysicalDevice, VkPhysicalDeviceProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFormatProperties2(VkPhysicalDevice, VkFormat, VkFormatProperties2*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice,
                                                                                  const VkPhysicalDeviceImageFormatInfo2*,
                                                                                  VkImageFormatProperties2*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice, uint32_t*,
                                                                              VkQueueFamilyProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice,
                                                                                    const VkPhysicalDeviceSparseImageFormatInfo2*,
                                                                                    uint32_t*, VkSparseImageFormatProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubTrimCommandPool(VkDevice, VkCommandPool, VkCommandPoolTrimFlags) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceQueue2(VkDevice, const VkDeviceQueueInfo2*, VkQueue*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSamplerYcbcrConversion(VkDevice, const VkSamplerYcbcrConversionCreateInfo*,
                                                                       const VkAllocationCallbacks*, VkSamplerYcbcrConversion*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySamplerYcbcrConversion(VkDevice, VkSamplerYcbcrConversion,
                                                                    const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDescriptorUpdateTemplate(VkDevice, const VkDescriptorUpdateTemplateCreateInfo*,
                                                                         const VkAllocationCallbacks*,
                                                                         VkDescriptorUpdateTemplate*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDescriptorUpdateTemplate(VkDevice, VkDescriptorUpdateTemplate,
                                                                      const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateDescriptorSetWithTemplate(VkDevice, VkDescriptorSet, VkDescriptorUpdateTemplate,
                                                                      const void*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice,
                                                                                const VkPhysicalDeviceExternalBufferInfo*,
                                                                                VkExternalBufferProperties*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice,
                                                                               const VkPhysicalDeviceExternalFenceInfo*,
                                                                               VkExternalFenceProperties*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice,
                                                                                   const VkPhysicalDeviceExternalSemaphoreInfo*,
                                                                                   VkExternalSemaphoreProperties*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSupport(VkDevice, const VkDescriptorSetLayoutCreateInfo*,
                                                                    VkDescriptorSetLayoutSupport*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCount(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize,
                                                           uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCount(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize,
                                                                  uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRenderPass2(VkDevice, const VkRenderPassCreateInfo2*, const VkAllocationCallbacks*,
                                                            VkRenderPass*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo*,
                                                          const VkSubpassBeginInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdNextSubpass2(VkCommandBuffer, const VkSubpassBeginInfo*, const VkSubpassEndInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderPass2(VkCommandBuffer, const VkSubpassEndInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubResetQueryPool(VkDevice, VkQueryPool, uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreCounterValue(VkDevice, VkSemaphore, uint64_t*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitSemaphores(VkDevice, const VkSemaphoreWaitInfo*, uint64_t) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubSignalSemaphore(VkDevice, const VkSemaphoreSignalInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddress(VkDevice, const VkBufferDeviceAddressInfo*) { return 0; }
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetBufferOpaqueCaptureAddress(VkDevice, const VkBufferDeviceAddressInfo*) { return 0; }
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetDeviceMemoryOpaqueCaptureAddress(VkDevice,
                                                                              const VkDeviceMemoryOpaqueCaptureAddressInfo*) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceToolProperties(VkPhysicalDevice, uint32_t*,
                                                                          VkPhysicalDeviceToolProperties*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreatePrivateDataSlot(VkDevice, const VkPrivateDataSlotCreateInfo*,
                                                                const VkAllocationCallbacks*, VkPrivateDataSlot*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyPrivateDataSlot(VkDevice, VkPrivateDataSlot, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetPrivateData(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPrivateData(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetEvent2(VkCommandBuffer, VkEvent, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResetEvent2(VkCommandBuffer, VkEvent, VkPipelineStageFlags2) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWaitEvents2(VkCommandBuffer, uint32_t, const VkEvent*, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPipelineBarrier2(VkCommandBuffer, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteTimestamp2(VkCommandBuffer, VkPipelineStageFlags2, VkQueryPool, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSubmit2(VkQueue, uint32_t, const VkSubmitInfo2*, VkFence) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBuffer2(VkCommandBuffer, const VkCopyBufferInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImage2(VkCommandBuffer, const VkCopyImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBufferToImage2(VkCommandBuffer, const VkCopyBufferToImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImageToBuffer2(VkCommandBuffer, const VkCopyImageToBufferInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBlitImage2(VkCommandBuffer, const VkBlitImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResolveImage2(VkCommandBuffer, const VkResolveImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRendering(VkCommandBuffer, const VkRenderingInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRendering(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCullMode(VkCommandBuffer, VkCullModeFlags) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFrontFace(VkCommandBuffer, VkFrontFace) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveTopology(VkCommandBuffer, VkPrimitiveTopology) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWithCount(VkCommandBuffer, uint32_t, const VkViewport*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetScissorWithCount(VkCommandBuffer, uint32_t, const VkRect2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindVertexBuffers2(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*,
                                                            const VkDeviceSize*, const VkDeviceSize*, const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthTestEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthWriteEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthCompareOp(VkCommandBuffer, VkCompareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBoundsTestEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilTestEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilOp(VkCommandBuffer, VkStencilFaceFlags, VkStencilOp, VkStencilOp, VkStencilOp,
                                                      VkCompareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizerDiscardEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBiasEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveRestartEnable(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceBufferMemoryRequirements(VkDevice, const VkDeviceBufferMemoryRequirements*,
                                                                        VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageMemoryRequirements(VkDevice, const VkDeviceImageMemoryRequirements*,
                                                                       VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSparseMemoryRequirements(VkDevice, const VkDeviceImageMemoryRequirements*,
                                                                             uint32_t*, VkSparseImageMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStipple(VkCommandBuffer, uint32_t, uint16_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubMapMemory2(VkDevice, const VkMemoryMapInfo*, void**) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubUnmapMemory2(VkDevice, const VkMemoryUnmapInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubCmdBindIndexBuffer2(VkCommandBuffer, VkBuffer, VkDeviceSize, VkDeviceSize, VkIndexType) {}
static VKAPI_ATTR void VKAPI_CALL StubGetRenderingAreaGranularity(VkDevice, const VkRenderingAreaInfo*, VkExtent2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSubresourceLayout(VkDevice, const VkDeviceImageSubresourceInfo*,
                                                                      VkSubresourceLayout2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSubresourceLayout2(VkDevice, VkImage, const VkImageSubresource2*,
                                                                 VkSubresourceLayout2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSet(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t,
                                                           uint32_t, const VkWriteDescriptorSet*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetWithTemplate(VkCommandBuffer, VkDescriptorUpdateTemplate,
                                                                       VkPipelineLayout, uint32_t, const void*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRenderingAttachmentLocations(VkCommandBuffer,
                                                                         const VkRenderingAttachmentLocationInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRenderingInputAttachmentIndices(VkCommandBuffer,
                                                                            const VkRenderingInputAttachmentIndexInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorSets2(VkCommandBuffer, const VkBindDescriptorSetsInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushConstants2(VkCommandBuffer, const VkPushConstantsInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSet2(VkCommandBuffer, const VkPushDescriptorSetInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetWithTemplate2(VkCommandBuffer,
                                                                        const VkPushDescriptorSetWithTemplateInfo*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToImage(VkDevice, const VkCopyMemoryToImageInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToMemory(VkDevice, const VkCopyImageToMemoryInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToImage(VkDevice, const VkCopyImageToImageInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubTransitionImageLayout(VkDevice, uint32_t, const VkHostImageLayoutTransitionInfo*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySurfaceKHR(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice, uint32_t, VkSurfaceKHR, VkBool32*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice, VkSurfaceKHR,
                                                                                  VkSurfaceCapabilitiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice, VkSurfaceKHR, uint32_t*,
                                                                             VkSurfaceFormatKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice, VkSurfaceKHR, uint32_t*,
                                                                                  VkPresentModeKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSwapchainKHR(VkDevice, const VkSwapchainCreateInfoKHR*,
                                                             const VkAllocationCallbacks*, VkSwapchainKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySwapchainKHR(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainImagesKHR(VkDevice, VkSwapchainKHR, uint32_t*, VkImage*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImageKHR(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueuePresentKHR(VkQueue, const VkPresentInfoKHR*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupPresentCapabilitiesKHR(VkDevice, VkDeviceGroupPresentCapabilitiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupSurfacePresentModesKHR(VkDevice, VkSurfaceKHR,
                                                                               VkDeviceGroupPresentModeFlagsKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice, VkSurfaceKHR, uint32_t*,
                                                                                VkRect2D*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireNextImage2KHR(VkDevice, const VkAcquireNextImageInfoKHR*, uint32_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice, uint32_t*,
                                                                                VkDisplayPropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice, uint32_t*,
                                                                                     VkDisplayPlanePropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice, uint32_t, uint32_t*,
                                                                              VkDisplayKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModePropertiesKHR(VkPhysicalDevice, VkDisplayKHR, uint32_t*,
                                                                      VkDisplayModePropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayModeKHR(VkPhysicalDevice, VkDisplayKHR, const VkDisplayModeCreateInfoKHR*,
                                                               const VkAllocationCallbacks*, VkDisplayModeKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice, VkDisplayModeKHR, uint32_t,
                                                                         VkDisplayPlaneCapabilitiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDisplayPlaneSurfaceKHR(VkInstance, const VkDisplaySurfaceCreateInfoKHR*,
                                                                       const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSharedSwapchainsKHR(VkDevice, uint32_t, const VkSwapchainCreateInfoKHR*,
                                                                    const VkAllocationCallbacks*, VkSwapchainKHR*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_XLIB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateXlibSurfaceKHR(VkInstance, const VkXlibSurfaceCreateInfoKHR*,
                                                               const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice, uint32_t, Display*,
                                                                                      VisualID) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateXcbSurfaceKHR(VkInstance, const VkXcbSurfaceCreateInfoKHR*,
                                                              const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice, uint32_t, xcb_connection_t*,
                                                                                     xcb_visualid_t) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateWaylandSurfaceKHR(VkInstance, const VkWaylandSurfaceCreateInfoKHR*,
                                                                  const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice, uint32_t,
                                                                                         struct wl_display*) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAndroidSurfaceKHR(VkInstance, const VkAndroidSurfaceCreateInfoKHR*,
                                                                  const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateWin32SurfaceKHR(VkInstance, const VkWin32SurfaceCreateInfoKHR*,
                                                                const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice, uint32_t) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice, const VkVideoProfileInfoKHR*,
                                                                                VkVideoCapabilitiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice,
                                                                                    const VkPhysicalDeviceVideoFormatInfoKHR*,
                                                                                    uint32_t*, VkVideoFormatPropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateVideoSessionKHR(VkDevice, const VkVideoSessionCreateInfoKHR*,
                                                                const VkAllocationCallbacks*, VkVideoSessionKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyVideoSessionKHR(VkDevice, VkVideoSessionKHR, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetVideoSessionMemoryRequirementsKHR(VkDevice, VkVideoSessionKHR, uint32_t*,
                                                                               VkVideoSessionMemoryRequirementsKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindVideoSessionMemoryKHR(VkDevice, VkVideoSessionKHR, uint32_t,
                                                                    const VkBindVideoSessionMemoryInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateVideoSessionParametersKHR(VkDevice, const VkVideoSessionParametersCreateInfoKHR*,
                                                                          const VkAllocationCallbacks*,
                                                                          VkVideoSessionParametersKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubUpdateVideoSessionParametersKHR(VkDevice, VkVideoSessionParametersKHR,
                                                                          const VkVideoSessionParametersUpdateInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyVideoSessionParametersKHR(VkDevice, VkVideoSessionParametersKHR,
                                                                       const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginVideoCodingKHR(VkCommandBuffer, const VkVideoBeginCodingInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndVideoCodingKHR(VkCommandBuffer, const VkVideoEndCodingInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdControlVideoCodingKHR(VkCommandBuffer, const VkVideoCodingControlInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecodeVideoKHR(VkCommandBuffer, const VkVideoDecodeInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderingKHR(VkCommandBuffer, const VkRenderingInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderingKHR(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice, VkPhysicalDeviceFeatures2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceProperties2KHR(VkPhysicalDevice, VkPhysicalDeviceProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice, VkFormat, VkFormatProperties2*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice,
                                                                                     const VkPhysicalDeviceImageFormatInfo2*,
                                                                                     VkImageFormatProperties2*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice, uint32_t*,
                                                                                 VkQueueFamilyProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2*, uint32_t*, VkSparseImageFormatProperties2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice, uint32_t, uint32_t, uint32_t,
                                                                          VkPeerMemoryFeatureFlags*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDeviceMaskKHR(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchBaseKHR(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                                         uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubTrimCommandPoolKHR(VkDevice, VkCommandPool, VkCommandPoolTrimFlags) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceGroupsKHR(VkInstance, uint32_t*,
                                                                           VkPhysicalDeviceGroupProperties*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice,
                                                                                   const VkPhysicalDeviceExternalBufferInfo*,
                                                                                   VkExternalBufferProperties*) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryWin32HandleKHR(VkDevice, const VkMemoryGetWin32HandleInfoKHR*, HANDLE*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryWin32HandlePropertiesKHR(VkDevice, VkExternalMemoryHandleTypeFlagBits, HANDLE,
                                                                            VkMemoryWin32HandlePropertiesKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdKHR(VkDevice, const VkMemoryGetFdInfoKHR*, int*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryFdPropertiesKHR(VkDevice, VkExternalMemoryHandleTypeFlagBits, int,
                                                                   VkMemoryFdPropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice,
                                                                                      const VkPhysicalDeviceExternalSemaphoreInfo*,
                                                                                      VkExternalSemaphoreProperties*) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreWin32HandleKHR(VkDevice, const VkImportSemaphoreWin32HandleInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreWin32HandleKHR(VkDevice, const VkSemaphoreGetWin32HandleInfoKHR*, HANDLE*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreFdKHR(VkDevice, const VkImportSemaphoreFdInfoKHR*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreFdKHR(VkDevice, const VkSemaphoreGetFdInfoKHR*, int*) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetKHR(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t,
                                                              uint32_t, const VkWriteDescriptorSet*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer, VkDescriptorUpdateTemplate,
                                                                          VkPipelineLayout, uint32_t, const void*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDescriptorUpdateTemplateKHR(VkDevice, const VkDescriptorUpdateTemplateCreateInfo*,
                                                                            const VkAllocationCallbacks*,
                                                                            VkDescriptorUpdateTemplate*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDescriptorUpdateTemplateKHR(VkDevice, VkDescriptorUpdateTemplate,
                                                                         const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateDescriptorSetWithTemplateKHR(VkDevice, VkDescriptorSet, VkDescriptorUpdateTemplate,
                                                                         const void*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRenderPass2KHR(VkDevice, const VkRenderPassCreateInfo2*,
                                                               const VkAllocationCallbacks*, VkRenderPass*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginRenderPass2KHR(VkCommandBuffer, const VkRenderPassBeginInfo*,
                                                             const VkSubpassBeginInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdNextSubpass2KHR(VkCommandBuffer, const VkSubpassBeginInfo*, const VkSubpassEndInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndRenderPass2KHR(VkCommandBuffer, const VkSubpassEndInfo*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainStatusKHR(VkDevice, VkSwapchainKHR) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice,
                                                                                  const VkPhysicalDeviceExternalFenceInfo*,
                                                                                  VkExternalFenceProperties*) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportFenceWin32HandleKHR(VkDevice, const VkImportFenceWin32HandleInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFenceWin32HandleKHR(VkDevice, const VkFenceGetWin32HandleInfoKHR*, HANDLE*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubImportFenceFdKHR(VkDevice, const VkImportFenceFdInfoKHR*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFenceFdKHR(VkDevice, const VkFenceGetFdInfoKHR*, int*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice, uint32_t, uint32_t*, VkPerformanceCounterKHR*, VkPerformanceCounterDescriptionKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice, const VkQueryPoolPerformanceCreateInfoKHR*, uint32_t*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireProfilingLockKHR(VkDevice, const VkAcquireProfilingLockInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubReleaseProfilingLockKHR(VkDevice) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice,
                                                                                   const VkPhysicalDeviceSurfaceInfo2KHR*,
                                                                                   VkSurfaceCapabilities2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice,
                                                                              const VkPhysicalDeviceSurfaceInfo2KHR*, uint32_t*,
                                                                              VkSurfaceFormat2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice, uint32_t*,
                                                                                 VkDisplayProperties2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice, uint32_t*,
                                                                                      VkDisplayPlaneProperties2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayModeProperties2KHR(VkPhysicalDevice, VkDisplayKHR, uint32_t*,
                                                                       VkDisplayModeProperties2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice, const VkDisplayPlaneInfo2KHR*,
                                                                          VkDisplayPlaneCapabilities2KHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageMemoryRequirements2KHR(VkDevice, const VkImageMemoryRequirementsInfo2*,
                                                                     VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetBufferMemoryRequirements2KHR(VkDevice, const VkBufferMemoryRequirementsInfo2*,
                                                                      VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSparseMemoryRequirements2KHR(VkDevice, const VkImageSparseMemoryRequirementsInfo2*,
                                                                           uint32_t*, VkSparseImageMemoryRequirements2*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateSamplerYcbcrConversionKHR(VkDevice, const VkSamplerYcbcrConversionCreateInfo*,
                                                                          const VkAllocationCallbacks*, VkSamplerYcbcrConversion*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroySamplerYcbcrConversionKHR(VkDevice, VkSamplerYcbcrConversion,
                                                                       const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindBufferMemory2KHR(VkDevice, uint32_t, const VkBindBufferMemoryInfo*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindImageMemory2KHR(VkDevice, uint32_t, const VkBindImageMemoryInfo*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSupportKHR(VkDevice, const VkDescriptorSetLayoutCreateInfo*,
                                                                       VkDescriptorSetLayoutSupport*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCountKHR(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize,
                                                              uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCountKHR(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer,
                                                                     VkDeviceSize, uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreCounterValueKHR(VkDevice, VkSemaphore, uint64_t*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitSemaphoresKHR(VkDevice, const VkSemaphoreWaitInfo*, uint64_t) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubSignalSemaphoreKHR(VkDevice, const VkSemaphoreSignalInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice, uint32_t*,
                                                                                   VkPhysicalDeviceFragmentShadingRateKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFragmentShadingRateKHR(VkCommandBuffer, const VkExtent2D*,
                                                                   const VkFragmentShadingRateCombinerOpKHR[2]) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer,
                                                                            const VkRenderingAttachmentLocationInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer,
                                                                               const VkRenderingInputAttachmentIndexInfo*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubWaitForPresentKHR(VkDevice, VkSwapchainKHR, uint64_t, uint64_t) { return VK_SUCCESS; }
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddressKHR(VkDevice, const VkBufferDeviceAddressInfo*) { return 0; }
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetBufferOpaqueCaptureAddressKHR(VkDevice, const VkBufferDeviceAddressInfo*) { return 0; }
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice,
                                                                                 const VkDeviceMemoryOpaqueCaptureAddressInfo*) {
    return 0;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDeferredOperationKHR(VkDevice, const VkAllocationCallbacks*,
                                                                     VkDeferredOperationKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDeferredOperationKHR(VkDevice, VkDeferredOperationKHR, const VkAllocationCallbacks*) {}
static VKAPI_ATTR uint32_t VKAPI_CALL StubGetDeferredOperationMaxConcurrencyKHR(VkDevice, VkDeferredOperationKHR) { return 0; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeferredOperationResultKHR(VkDevice, VkDeferredOperationKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubDeferredOperationJoinKHR(VkDevice, VkDeferredOperationKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutablePropertiesKHR(VkDevice, const VkPipelineInfoKHR*, uint32_t*,
                                                                             VkPipelineExecutablePropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutableStatisticsKHR(VkDevice, const VkPipelineExecutableInfoKHR*,
                                                                             uint32_t*, VkPipelineExecutableStatisticKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice, const VkPipelineExecutableInfoKHR*, uint32_t*, VkPipelineExecutableInternalRepresentationKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubMapMemory2KHR(VkDevice, const VkMemoryMapInfo*, void**) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubUnmapMemory2KHR(VkDevice, const VkMemoryUnmapInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR*, VkVideoEncodeQualityLevelPropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetEncodedVideoSessionParametersKHR(VkDevice,
                                                                              const VkVideoEncodeSessionParametersGetInfoKHR*,
                                                                              VkVideoEncodeSessionParametersFeedbackInfoKHR*,
                                                                              size_t*, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdEncodeVideoKHR(VkCommandBuffer, const VkVideoEncodeInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetEvent2KHR(VkCommandBuffer, VkEvent, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResetEvent2KHR(VkCommandBuffer, VkEvent, VkPipelineStageFlags2) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWaitEvents2KHR(VkCommandBuffer, uint32_t, const VkEvent*, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPipelineBarrier2KHR(VkCommandBuffer, const VkDependencyInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteTimestamp2KHR(VkCommandBuffer, VkPipelineStageFlags2, VkQueryPool, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSubmit2KHR(VkQueue, uint32_t, const VkSubmitInfo2*, VkFence) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBuffer2KHR(VkCommandBuffer, const VkCopyBufferInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImage2KHR(VkCommandBuffer, const VkCopyImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyBufferToImage2KHR(VkCommandBuffer, const VkCopyBufferToImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyImageToBuffer2KHR(VkCommandBuffer, const VkCopyImageToBufferInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBlitImage2KHR(VkCommandBuffer, const VkBlitImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdResolveImage2KHR(VkCommandBuffer, const VkResolveImageInfo2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysIndirect2KHR(VkCommandBuffer, VkDeviceAddress) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceBufferMemoryRequirementsKHR(VkDevice, const VkDeviceBufferMemoryRequirements*,
                                                                           VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageMemoryRequirementsKHR(VkDevice, const VkDeviceImageMemoryRequirements*,
                                                                          VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSparseMemoryRequirementsKHR(VkDevice, const VkDeviceImageMemoryRequirements*,
                                                                                uint32_t*, VkSparseImageMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindIndexBuffer2KHR(VkCommandBuffer, VkBuffer, VkDeviceSize, VkDeviceSize, VkIndexType) {}
static VKAPI_ATTR void VKAPI_CALL StubGetRenderingAreaGranularityKHR(VkDevice, const VkRenderingAreaInfo*, VkExtent2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceImageSubresourceLayoutKHR(VkDevice, const VkDeviceImageSubresourceInfo*,
                                                                         VkSubresourceLayout2*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSubresourceLayout2KHR(VkDevice, VkImage, const VkImageSubresource2*,
                                                                    VkSubresourceLayout2*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreatePipelineBinariesKHR(VkDevice, const VkPipelineBinaryCreateInfoKHR*,
                                                                    const VkAllocationCallbacks*, VkPipelineBinaryHandlesInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyPipelineBinaryKHR(VkDevice, VkPipelineBinaryKHR, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineKeyKHR(VkDevice, const VkPipelineCreateInfoKHR*, VkPipelineBinaryKeyKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelineBinaryDataKHR(VkDevice, const VkPipelineBinaryDataInfoKHR*,
                                                                   VkPipelineBinaryKeyKHR*, size_t*, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseCapturedPipelineDataKHR(VkDevice, const VkReleaseCapturedPipelineDataInfoKHR*,
                                                                         const VkAllocationCallbacks*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice, uint32_t*,
                                                                                          VkCooperativeMatrixPropertiesKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleKHR(VkCommandBuffer, uint32_t, uint16_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice, uint32_t*,
                                                                                       VkTimeDomainKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetCalibratedTimestampsKHR(VkDevice, uint32_t, const VkCalibratedTimestampInfoKHR*,
                                                                     uint64_t*, uint64_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorSets2KHR(VkCommandBuffer, const VkBindDescriptorSetsInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushConstants2KHR(VkCommandBuffer, const VkPushConstantsInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSet2KHR(VkCommandBuffer, const VkPushDescriptorSetInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPushDescriptorSetWithTemplate2KHR(VkCommandBuffer,
                                                                           const VkPushDescriptorSetWithTemplateInfo*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer,
                                                                        const VkSetDescriptorBufferOffsetsInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL
StubCmdBindDescriptorBufferEmbeddedSamplers2EXT(VkCommandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugReportCallbackEXT(VkInstance, const VkDebugReportCallbackCreateInfoEXT*,
                                                                       const VkAllocationCallbacks*, VkDebugReportCallbackEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugReportCallbackEXT(VkInstance, VkDebugReportCallbackEXT,
                                                                    const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubDebugReportMessageEXT(VkInstance, VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t,
                                                            size_t, int32_t, const char*, const char*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectTagEXT(VkDevice, const VkDebugMarkerObjectTagInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDebugMarkerSetObjectNameEXT(VkDevice, const VkDebugMarkerObjectNameInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerBeginEXT(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerEndEXT(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDebugMarkerInsertEXT(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*,
                                                                         const VkDeviceSize*, const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginTransformFeedbackEXT(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*,
                                                                   const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndTransformFeedbackEXT(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*,
                                                                 const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginQueryIndexedEXT(VkCommandBuffer, VkQueryPool, uint32_t, VkQueryControlFlags,
                                                              uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndQueryIndexedEXT(VkCommandBuffer, VkQueryPool, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectByteCountEXT(VkCommandBuffer, uint32_t, uint32_t, VkBuffer, VkDeviceSize,
                                                                  uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCuModuleNVX(VkDevice, const VkCuModuleCreateInfoNVX*, const VkAllocationCallbacks*,
                                                            VkCuModuleNVX*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCuFunctionNVX(VkDevice, const VkCuFunctionCreateInfoNVX*,
                                                              const VkAllocationCallbacks*, VkCuFunctionNVX*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCuModuleNVX(VkDevice, VkCuModuleNVX, const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCuFunctionNVX(VkDevice, VkCuFunctionNVX, const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCuLaunchKernelNVX(VkCommandBuffer, const VkCuLaunchInfoNVX*) {}
static VKAPI_ATTR uint32_t VKAPI_CALL StubGetImageViewHandleNVX(VkDevice, const VkImageViewHandleInfoNVX*) { return 0; }
static VKAPI_ATTR uint64_t VKAPI_CALL StubGetImageViewHandle64NVX(VkDevice, const VkImageViewHandleInfoNVX*) { return 0; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageViewAddressNVX(VkDevice, VkImageView, VkImageViewAddressPropertiesNVX*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndirectCountAMD(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize,
                                                              uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawIndexedIndirectCountAMD(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer,
                                                                     VkDeviceSize, uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetShaderInfoAMD(VkDevice, VkPipeline, VkShaderStageFlagBits, VkShaderInfoTypeAMD,
                                                           size_t*, void*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_GGP
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateStreamDescriptorSurfaceGGP(VkInstance,
                                                                           const VkStreamDescriptorSurfaceCreateInfoGGP*,
                                                                           const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_GGP
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice, VkFormat, VkImageType,
                                                                                           VkImageTiling, VkImageUsageFlags,
                                                                                           VkImageCreateFlags,
                                                                                           VkExternalMemoryHandleTypeFlagsNV,
                                                                                           VkExternalImageFormatPropertiesNV*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryWin32HandleNV(VkDevice, VkDeviceMemory, VkExternalMemoryHandleTypeFlagsNV,
                                                                 HANDLE*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateViSurfaceNN(VkInstance, const VkViSurfaceCreateInfoNN*,
                                                            const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_VI_NN
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginConditionalRenderingEXT(VkCommandBuffer, const VkConditionalRenderingBeginInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndConditionalRenderingEXT(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWScalingNV(VkCommandBuffer, uint32_t, uint32_t, const VkViewportWScalingNV*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseDisplayEXT(VkPhysicalDevice, VkDisplayKHR) { return VK_SUCCESS; }
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireXlibDisplayEXT(VkPhysicalDevice, Display*, VkDisplayKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRandROutputDisplayEXT(VkPhysicalDevice, Display*, RROutput, VkDisplayKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice, VkSurfaceKHR,
                                                                                   VkSurfaceCapabilities2EXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubDisplayPowerControlEXT(VkDevice, VkDisplayKHR, const VkDisplayPowerInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDeviceEventEXT(VkDevice, const VkDeviceEventInfoEXT*,
                                                                 const VkAllocationCallbacks*, VkFence*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubRegisterDisplayEventEXT(VkDevice, VkDisplayKHR, const VkDisplayEventInfoEXT*,
                                                                  const VkAllocationCallbacks*, VkFence*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSwapchainCounterEXT(VkDevice, VkSwapchainKHR, VkSurfaceCounterFlagBitsEXT, uint64_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRefreshCycleDurationGOOGLE(VkDevice, VkSwapchainKHR, VkRefreshCycleDurationGOOGLE*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPastPresentationTimingGOOGLE(VkDevice, VkSwapchainKHR, uint32_t*,
                                                                          VkPastPresentationTimingGOOGLE*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleEXT(VkCommandBuffer, uint32_t, uint32_t, const VkRect2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDiscardRectangleModeEXT(VkCommandBuffer, VkDiscardRectangleModeEXT) {}
static VKAPI_ATTR void VKAPI_CALL StubSetHdrMetadataEXT(VkDevice, uint32_t, const VkSwapchainKHR*, const VkHdrMetadataEXT*) {}
#ifdef VK_USE_PLATFORM_IOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIOSSurfaceMVK(VkInstance, const VkIOSSurfaceCreateInfoMVK*,
                                                              const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMacOSSurfaceMVK(VkInstance, const VkMacOSSurfaceCreateInfoMVK*,
                                                                const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_MACOS_MVK
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectNameEXT(VkDevice, const VkDebugUtilsObjectNameInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetDebugUtilsObjectTagEXT(VkDevice, const VkDebugUtilsObjectTagInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubQueueBeginDebugUtilsLabelEXT(VkQueue, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueEndDebugUtilsLabelEXT(VkQueue) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueInsertDebugUtilsLabelEXT(VkQueue, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBeginDebugUtilsLabelEXT(VkCommandBuffer, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdEndDebugUtilsLabelEXT(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdInsertDebugUtilsLabelEXT(VkCommandBuffer, const VkDebugUtilsLabelEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*,
                                                                       const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT,
                                                                    const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubSubmitDebugUtilsMessageEXT(VkInstance, VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                                 VkDebugUtilsMessageTypeFlagsEXT,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT*) {}
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAndroidHardwareBufferPropertiesANDROID(VkDevice, const struct AHardwareBuffer*,
                                                                                    VkAndroidHardwareBufferPropertiesANDROID*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryAndroidHardwareBufferANDROID(VkDevice,
                                                                                const VkMemoryGetAndroidHardwareBufferInfoANDROID*,
                                                                                struct AHardwareBuffer**) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateExecutionGraphPipelinesAMDX(VkDevice, VkPipelineCache, uint32_t,
                                                                            const VkExecutionGraphPipelineCreateInfoAMDX*,
                                                                            const VkAllocationCallbacks*, VkPipeline*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetExecutionGraphPipelineScratchSizeAMDX(VkDevice, VkPipeline,
                                                                                   VkExecutionGraphPipelineScratchSizeAMDX*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetExecutionGraphPipelineNodeIndexAMDX(VkDevice, VkPipeline,
                                                                                 const VkPipelineShaderStageNodeCreateInfoAMDX*,
                                                                                 uint32_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdInitializeGraphScratchMemoryAMDX(VkCommandBuffer, VkPipeline, VkDeviceAddress,
                                                                          VkDeviceSize) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphAMDX(VkCommandBuffer, VkDeviceAddress, VkDeviceSize,
                                                           const VkDispatchGraphCountInfoAMDX*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphIndirectAMDX(VkCommandBuffer, VkDeviceAddress, VkDeviceSize,
                                                                   const VkDispatchGraphCountInfoAMDX*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDispatchGraphIndirectCountAMDX(VkCommandBuffer, VkDeviceAddress, VkDeviceSize,
                                                                        VkDeviceAddress) {}
#endif  // VK_ENABLE_BETA_EXTENSIONS
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleLocationsEXT(VkCommandBuffer, const VkSampleLocationsInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice, VkSampleCountFlagBits,
                                                                                VkMultisamplePropertiesEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageDrmFormatModifierPropertiesEXT(VkDevice, VkImage,
                                                                                 VkImageDrmFormatModifierPropertiesEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateValidationCacheEXT(VkDevice, const VkValidationCacheCreateInfoEXT*,
                                                                   const VkAllocationCallbacks*, VkValidationCacheEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyValidationCacheEXT(VkDevice, VkValidationCacheEXT, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubMergeValidationCachesEXT(VkDevice, VkValidationCacheEXT, uint32_t,
                                                                   const VkValidationCacheEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetValidationCacheDataEXT(VkDevice, VkValidationCacheEXT, size_t*, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindShadingRateImageNV(VkCommandBuffer, VkImageView, VkImageLayout) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportShadingRatePaletteNV(VkCommandBuffer, uint32_t, uint32_t,
                                                                         const VkShadingRatePaletteNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoarseSampleOrderNV(VkCommandBuffer, VkCoarseSampleOrderTypeNV, uint32_t,
                                                                const VkCoarseSampleOrderCustomNV*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAccelerationStructureNV(VkDevice, const VkAccelerationStructureCreateInfoNV*,
                                                                        const VkAllocationCallbacks*, VkAccelerationStructureNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyAccelerationStructureNV(VkDevice, VkAccelerationStructureNV,
                                                                     const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetAccelerationStructureMemoryRequirementsNV(
    VkDevice, const VkAccelerationStructureMemoryRequirementsInfoNV*, VkMemoryRequirements2KHR*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindAccelerationStructureMemoryNV(VkDevice, uint32_t,
                                                                            const VkBindAccelerationStructureMemoryInfoNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructureNV(VkCommandBuffer, const VkAccelerationStructureInfoNV*,
                                                                      VkBuffer, VkDeviceSize, VkBool32, VkAccelerationStructureNV,
                                                                      VkAccelerationStructureNV, VkBuffer, VkDeviceSize) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureNV(VkCommandBuffer, VkAccelerationStructureNV,
                                                                     VkAccelerationStructureNV,
                                                                     VkCopyAccelerationStructureModeKHR) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysNV(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize, VkDeviceSize,
                                                     VkBuffer, VkDeviceSize, VkDeviceSize, VkBuffer, VkDeviceSize, VkDeviceSize,
                                                     uint32_t, uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRayTracingPipelinesNV(VkDevice, VkPipelineCache, uint32_t,
                                                                      const VkRayTracingPipelineCreateInfoNV*,
                                                                      const VkAllocationCallbacks*, VkPipeline*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingShaderGroupHandlesKHR(VkDevice, VkPipeline, uint32_t, uint32_t, size_t,
                                                                             void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingShaderGroupHandlesNV(VkDevice, VkPipeline, uint32_t, uint32_t, size_t,
                                                                            void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAccelerationStructureHandleNV(VkDevice, VkAccelerationStructureNV, size_t, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer, uint32_t,
                                                                                 const VkAccelerationStructureNV*, VkQueryType,
                                                                                 VkQueryPool, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCompileDeferredNV(VkDevice, VkPipeline, uint32_t) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryHostPointerPropertiesEXT(VkDevice, VkExternalMemoryHandleTypeFlagBits,
                                                                            const void*, VkMemoryHostPointerPropertiesEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteBufferMarkerAMD(VkCommandBuffer, VkPipelineStageFlagBits, VkBuffer, VkDeviceSize,
                                                              uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteBufferMarker2AMD(VkCommandBuffer, VkPipelineStageFlags2, VkBuffer, VkDeviceSize,
                                                               uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice, uint32_t*,
                                                                                       VkTimeDomainKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetCalibratedTimestampsEXT(VkDevice, uint32_t, const VkCalibratedTimestampInfoKHR*,
                                                                     uint64_t*, uint64_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksNV(VkCommandBuffer, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectNV(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer,
                                                                      VkDeviceSize, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExclusiveScissorEnableNV(VkCommandBuffer, uint32_t, uint32_t, const VkBool32*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExclusiveScissorNV(VkCommandBuffer, uint32_t, uint32_t, const VkRect2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCheckpointNV(VkCommandBuffer, const void*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetQueueCheckpointDataNV(VkQueue, uint32_t*, VkCheckpointDataNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetQueueCheckpointData2NV(VkQueue, uint32_t*, VkCheckpointData2NV*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubInitializePerformanceApiINTEL(VkDevice, const VkInitializePerformanceApiInfoINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubUninitializePerformanceApiINTEL(VkDevice) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCmdSetPerformanceMarkerINTEL(VkCommandBuffer, const VkPerformanceMarkerInfoINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer,
                                                                             const VkPerformanceStreamMarkerInfoINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCmdSetPerformanceOverrideINTEL(VkCommandBuffer, const VkPerformanceOverrideInfoINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquirePerformanceConfigurationINTEL(VkDevice,
                                                                               const VkPerformanceConfigurationAcquireInfoINTEL*,
                                                                               VkPerformanceConfigurationINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleasePerformanceConfigurationINTEL(VkDevice, VkPerformanceConfigurationINTEL) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubQueueSetPerformanceConfigurationINTEL(VkQueue, VkPerformanceConfigurationINTEL) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPerformanceParameterINTEL(VkDevice, VkPerformanceParameterTypeINTEL,
                                                                       VkPerformanceValueINTEL*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubSetLocalDimmingAMD(VkDevice, VkSwapchainKHR, VkBool32) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateImagePipeSurfaceFUCHSIA(VkInstance, const VkImagePipeSurfaceCreateInfoFUCHSIA*,
                                                                        const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMetalSurfaceEXT(VkInstance, const VkMetalSurfaceCreateInfoEXT*,
                                                                const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetBufferDeviceAddressEXT(VkDevice, const VkBufferDeviceAddressInfo*) { return 0; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice, uint32_t*,
                                                                             VkPhysicalDeviceToolProperties*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice, uint32_t*,
                                                                                         VkCooperativeMatrixPropertiesNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice, uint32_t*, VkFramebufferMixedSamplesCombinationNV*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice,
                                                                                   const VkPhysicalDeviceSurfaceInfo2KHR*,
                                                                                   uint32_t*, VkPresentModeKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireFullScreenExclusiveModeEXT(VkDevice, VkSwapchainKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseFullScreenExclusiveModeEXT(VkDevice, VkSwapchainKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceGroupSurfacePresentModes2EXT(VkDevice, const VkPhysicalDeviceSurfaceInfo2KHR*,
                                                                                VkDeviceGroupPresentModeFlagsKHR*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateHeadlessSurfaceEXT(VkInstance, const VkHeadlessSurfaceCreateInfoEXT*,
                                                                   const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleEXT(VkCommandBuffer, uint32_t, uint16_t) {}
static VKAPI_ATTR void VKAPI_CALL StubResetQueryPoolEXT(VkDevice, VkQueryPool, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCullModeEXT(VkCommandBuffer, VkCullModeFlags) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFrontFaceEXT(VkCommandBuffer, VkFrontFace) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveTopologyEXT(VkCommandBuffer, VkPrimitiveTopology) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWithCountEXT(VkCommandBuffer, uint32_t, const VkViewport*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetScissorWithCountEXT(VkCommandBuffer, uint32_t, const VkRect2D*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindVertexBuffers2EXT(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*,
                                                               const VkDeviceSize*, const VkDeviceSize*, const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthTestEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthWriteEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthCompareOpEXT(VkCommandBuffer, VkCompareOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilTestEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetStencilOpEXT(VkCommandBuffer, VkStencilFaceFlags, VkStencilOp, VkStencilOp, VkStencilOp,
                                                         VkCompareOp) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToImageEXT(VkDevice, const VkCopyMemoryToImageInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToMemoryEXT(VkDevice, const VkCopyImageToMemoryInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyImageToImageEXT(VkDevice, const VkCopyImageToImageInfo*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubTransitionImageLayoutEXT(VkDevice, uint32_t, const VkHostImageLayoutTransitionInfo*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetImageSubresourceLayout2EXT(VkDevice, VkImage, const VkImageSubresource2*,
                                                                    VkSubresourceLayout2*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubReleaseSwapchainImagesEXT(VkDevice, const VkReleaseSwapchainImagesInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetGeneratedCommandsMemoryRequirementsNV(VkDevice,
                                                                               const VkGeneratedCommandsMemoryRequirementsInfoNV*,
                                                                               VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPreprocessGeneratedCommandsNV(VkCommandBuffer, const VkGeneratedCommandsInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdExecuteGeneratedCommandsNV(VkCommandBuffer, VkBool32, const VkGeneratedCommandsInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindPipelineShaderGroupNV(VkCommandBuffer, VkPipelineBindPoint, VkPipeline, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIndirectCommandsLayoutNV(VkDevice, const VkIndirectCommandsLayoutCreateInfoNV*,
                                                                         const VkAllocationCallbacks*,
                                                                         VkIndirectCommandsLayoutNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyIndirectCommandsLayoutNV(VkDevice, VkIndirectCommandsLayoutNV,
                                                                      const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBias2EXT(VkCommandBuffer, const VkDepthBiasInfoEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireDrmDisplayEXT(VkPhysicalDevice, int32_t, VkDisplayKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDrmDisplayEXT(VkPhysicalDevice, int32_t, uint32_t, VkDisplayKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreatePrivateDataSlotEXT(VkDevice, const VkPrivateDataSlotCreateInfo*,
                                                                   const VkAllocationCallbacks*, VkPrivateDataSlot*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyPrivateDataSlotEXT(VkDevice, VkPrivateDataSlot, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetPrivateDataEXT(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubGetPrivateDataEXT(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCudaModuleNV(VkDevice, const VkCudaModuleCreateInfoNV*,
                                                             const VkAllocationCallbacks*, VkCudaModuleNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetCudaModuleCacheNV(VkDevice, VkCudaModuleNV, size_t*, void*) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateCudaFunctionNV(VkDevice, const VkCudaFunctionCreateInfoNV*,
                                                               const VkAllocationCallbacks*, VkCudaFunctionNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCudaModuleNV(VkDevice, VkCudaModuleNV, const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubDestroyCudaFunctionNV(VkDevice, VkCudaFunctionNV, const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCudaLaunchKernelNV(VkCommandBuffer, const VkCudaLaunchInfoNV*) {}
#ifdef VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR void VKAPI_CALL StubExportMetalObjectsEXT(VkDevice, VkExportMetalObjectsInfoEXT*) {}
#endif  // VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutSizeEXT(VkDevice, VkDescriptorSetLayout, VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutBindingOffsetEXT(VkDevice, VkDescriptorSetLayout, uint32_t,
                                                                             VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorEXT(VkDevice, const VkDescriptorGetInfoEXT*, size_t, void*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorBuffersEXT(VkCommandBuffer, uint32_t,
                                                                  const VkDescriptorBufferBindingInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout,
                                                                       uint32_t, uint32_t, const uint32_t*, const VkDeviceSize*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer, VkPipelineBindPoint,
                                                                                 VkPipelineLayout, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice,
                                                                                  const VkBufferCaptureDescriptorDataInfoEXT*,
                                                                                  void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageOpaqueCaptureDescriptorDataEXT(VkDevice,
                                                                                 const VkImageCaptureDescriptorDataInfoEXT*,
                                                                                 void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice,
                                                                                     const VkImageViewCaptureDescriptorDataInfoEXT*,
                                                                                     void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice,
                                                                                   const VkSamplerCaptureDescriptorDataInfoEXT*,
                                                                                   void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice, const VkAccelerationStructureCaptureDescriptorDataInfoEXT*, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetFragmentShadingRateEnumNV(VkCommandBuffer, VkFragmentShadingRateNV,
                                                                      const VkFragmentShadingRateCombinerOpKHR[2]) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceFaultInfoEXT(VkDevice, VkDeviceFaultCountsEXT*, VkDeviceFaultInfoEXT*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_WIN32_KHR
static VKAPI_ATTR VkResult VKAPI_CALL StubAcquireWinrtDisplayNV(VkPhysicalDevice, VkDisplayKHR) { return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL StubGetWinrtDisplayNV(VkPhysicalDevice, uint32_t, VkDisplayKHR*) { return VK_SUCCESS; }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateDirectFBSurfaceEXT(VkInstance, const VkDirectFBSurfaceCreateInfoEXT*,
                                                                   const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice, uint32_t, IDirectFB*) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
static VKAPI_ATTR void VKAPI_CALL StubCmdSetVertexInputEXT(VkCommandBuffer, uint32_t, const VkVertexInputBindingDescription2EXT*,
                                                           uint32_t, const VkVertexInputAttributeDescription2EXT*) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryZirconHandleFUCHSIA(VkDevice, const VkMemoryGetZirconHandleInfoFUCHSIA*,
                                                                       zx_handle_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryZirconHandlePropertiesFUCHSIA(VkDevice, VkExternalMemoryHandleTypeFlagBits,
                                                                                 zx_handle_t,
                                                                                 VkMemoryZirconHandlePropertiesFUCHSIA*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubImportSemaphoreZirconHandleFUCHSIA(VkDevice,
                                                                             const VkImportSemaphoreZirconHandleInfoFUCHSIA*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetSemaphoreZirconHandleFUCHSIA(VkDevice, const VkSemaphoreGetZirconHandleInfoFUCHSIA*,
                                                                          zx_handle_t*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateBufferCollectionFUCHSIA(VkDevice, const VkBufferCollectionCreateInfoFUCHSIA*,
                                                                        const VkAllocationCallbacks*, VkBufferCollectionFUCHSIA*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetBufferCollectionImageConstraintsFUCHSIA(VkDevice, VkBufferCollectionFUCHSIA,
                                                                                     const VkImageConstraintsInfoFUCHSIA*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetBufferCollectionBufferConstraintsFUCHSIA(VkDevice, VkBufferCollectionFUCHSIA,
                                                                                      const VkBufferConstraintsInfoFUCHSIA*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyBufferCollectionFUCHSIA(VkDevice, VkBufferCollectionFUCHSIA,
                                                                     const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetBufferCollectionPropertiesFUCHSIA(VkDevice, VkBufferCollectionFUCHSIA,
                                                                               VkBufferCollectionPropertiesFUCHSIA*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_FUCHSIA
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice, VkRenderPass, VkExtent2D*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSubpassShadingHUAWEI(VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBindInvocationMaskHUAWEI(VkCommandBuffer, VkImageView, VkImageLayout) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryRemoteAddressNV(VkDevice, const VkMemoryGetRemoteAddressInfoNV*,
                                                                   VkRemoteAddressNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPipelinePropertiesEXT(VkDevice, const VkPipelineInfoEXT*, VkBaseOutStructure*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPatchControlPointsEXT(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthBiasEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLogicOpEXT(VkCommandBuffer, VkLogicOp) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer, VkBool32) {}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateScreenSurfaceQNX(VkInstance, const VkScreenSurfaceCreateInfoQNX*,
                                                                 const VkAllocationCallbacks*, VkSurfaceKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL StubGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice, uint32_t,
                                                                                        struct _screen_window*) {
    return VK_FALSE;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorWriteEnableEXT(VkCommandBuffer, uint32_t, const VkBool32*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMultiEXT(VkCommandBuffer, uint32_t, const VkMultiDrawInfoEXT*, uint32_t, uint32_t,
                                                      uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMultiIndexedEXT(VkCommandBuffer, uint32_t, const VkMultiDrawIndexedInfoEXT*, uint32_t,
                                                             uint32_t, uint32_t, const int32_t*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateMicromapEXT(VkDevice, const VkMicromapCreateInfoEXT*, const VkAllocationCallbacks*,
                                                            VkMicromapEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyMicromapEXT(VkDevice, VkMicromapEXT, const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildMicromapsEXT(VkCommandBuffer, uint32_t, const VkMicromapBuildInfoEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBuildMicromapsEXT(VkDevice, VkDeferredOperationKHR, uint32_t,
                                                            const VkMicromapBuildInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMicromapEXT(VkDevice, VkDeferredOperationKHR, const VkCopyMicromapInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMicromapToMemoryEXT(VkDevice, VkDeferredOperationKHR,
                                                                  const VkCopyMicromapToMemoryInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToMicromapEXT(VkDevice, VkDeferredOperationKHR,
                                                                  const VkCopyMemoryToMicromapInfoEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWriteMicromapsPropertiesEXT(VkDevice, uint32_t, const VkMicromapEXT*, VkQueryType, size_t,
                                                                      void*, size_t) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMicromapEXT(VkCommandBuffer, const VkCopyMicromapInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMicromapToMemoryEXT(VkCommandBuffer, const VkCopyMicromapToMemoryInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToMicromapEXT(VkCommandBuffer, const VkCopyMemoryToMicromapInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteMicromapsPropertiesEXT(VkCommandBuffer, uint32_t, const VkMicromapEXT*, VkQueryType,
                                                                     VkQueryPool, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceMicromapCompatibilityEXT(VkDevice, const VkMicromapVersionInfoEXT*,
                                                                        VkAccelerationStructureCompatibilityKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetMicromapBuildSizesEXT(VkDevice, VkAccelerationStructureBuildTypeKHR,
                                                               const VkMicromapBuildInfoEXT*, VkMicromapBuildSizesInfoEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawClusterHUAWEI(VkCommandBuffer, uint32_t, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawClusterIndirectHUAWEI(VkCommandBuffer, VkBuffer, VkDeviceSize) {}
static VKAPI_ATTR void VKAPI_CALL StubSetDeviceMemoryPriorityEXT(VkDevice, VkDeviceMemory, float) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice,
                                                                                 const VkDescriptorSetBindingReferenceVALVE*,
                                                                                 VkDescriptorSetLayoutHostMappingInfoVALVE*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDescriptorSetHostMappingVALVE(VkDevice, VkDescriptorSet, void**) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryIndirectNV(VkCommandBuffer, VkDeviceAddress, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToImageIndirectNV(VkCommandBuffer, VkDeviceAddress, uint32_t, uint32_t, VkImage,
                                                                     VkImageLayout, const VkImageSubresourceLayers*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecompressMemoryNV(VkCommandBuffer, uint32_t, const VkDecompressMemoryRegionNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDecompressMemoryIndirectCountNV(VkCommandBuffer, VkDeviceAddress, VkDeviceAddress,
                                                                         uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPipelineIndirectMemoryRequirementsNV(VkDevice, const VkComputePipelineCreateInfo*,
                                                                              VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer, VkPipelineBindPoint, VkPipeline) {}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL StubGetPipelineIndirectDeviceAddressNV(VkDevice,
                                                                                    const VkPipelineIndirectDeviceAddressInfoNV*) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClampEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetPolygonModeEXT(VkCommandBuffer, VkPolygonMode) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizationSamplesEXT(VkCommandBuffer, VkSampleCountFlagBits) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleMaskEXT(VkCommandBuffer, VkSampleCountFlagBits, const VkSampleMask*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAlphaToOneEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLogicOpEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendEnableEXT(VkCommandBuffer, uint32_t, uint32_t, const VkBool32*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendEquationEXT(VkCommandBuffer, uint32_t, uint32_t,
                                                                  const VkColorBlendEquationEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorWriteMaskEXT(VkCommandBuffer, uint32_t, uint32_t, const VkColorComponentFlags*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetTessellationDomainOriginEXT(VkCommandBuffer, VkTessellationDomainOrigin) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRasterizationStreamEXT(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetConservativeRasterizationModeEXT(VkCommandBuffer, VkConservativeRasterizationModeEXT) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer, float) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClipEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetSampleLocationsEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetColorBlendAdvancedEXT(VkCommandBuffer, uint32_t, uint32_t,
                                                                  const VkColorBlendAdvancedEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetProvokingVertexModeEXT(VkCommandBuffer, VkProvokingVertexModeEXT) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineRasterizationModeEXT(VkCommandBuffer, VkLineRasterizationModeEXT) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetLineStippleEnableEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportWScalingEnableNV(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetViewportSwizzleNV(VkCommandBuffer, uint32_t, uint32_t, const VkViewportSwizzleNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageToColorEnableNV(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageToColorLocationNV(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationModeNV(VkCommandBuffer, VkCoverageModulationModeNV) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationTableEnableNV(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageModulationTableNV(VkCommandBuffer, uint32_t, const float*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetShadingRateImageEnableNV(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer, VkBool32) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetCoverageReductionModeNV(VkCommandBuffer, VkCoverageReductionModeNV) {}
static VKAPI_ATTR void VKAPI_CALL StubGetShaderModuleIdentifierEXT(VkDevice, VkShaderModule, VkShaderModuleIdentifierEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetShaderModuleCreateInfoIdentifierEXT(VkDevice, const VkShaderModuleCreateInfo*,
                                                                             VkShaderModuleIdentifierEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice,
                                                                                     const VkOpticalFlowImageFormatInfoNV*,
                                                                                     uint32_t*,
                                                                                     VkOpticalFlowImageFormatPropertiesNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateOpticalFlowSessionNV(VkDevice, const VkOpticalFlowSessionCreateInfoNV*,
                                                                     const VkAllocationCallbacks*, VkOpticalFlowSessionNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyOpticalFlowSessionNV(VkDevice, VkOpticalFlowSessionNV, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBindOpticalFlowSessionImageNV(VkDevice, VkOpticalFlowSessionNV,
                                                                        VkOpticalFlowSessionBindingPointNV, VkImageView,
                                                                        VkImageLayout) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdOpticalFlowExecuteNV(VkCommandBuffer, VkOpticalFlowSessionNV,
                                                              const VkOpticalFlowExecuteInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubAntiLagUpdateAMD(VkDevice, const VkAntiLagDataAMD*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateShadersEXT(VkDevice, uint32_t, const VkShaderCreateInfoEXT*,
                                                           const VkAllocationCallbacks*, VkShaderEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyShaderEXT(VkDevice, VkShaderEXT, const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetShaderBinaryDataEXT(VkDevice, VkShaderEXT, size_t*, void*) { return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL StubCmdBindShadersEXT(VkCommandBuffer, uint32_t, const VkShaderStageFlagBits*,
                                                        const VkShaderEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetDepthClampRangeEXT(VkCommandBuffer, VkDepthClampModeEXT, const VkDepthClampRangeEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetFramebufferTilePropertiesQCOM(VkDevice, VkFramebuffer, uint32_t*,
                                                                           VkTilePropertiesQCOM*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetDynamicRenderingTilePropertiesQCOM(VkDevice, const VkRenderingInfo*,
                                                                                VkTilePropertiesQCOM*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice, uint32_t*,
                                                                                         VkCooperativeVectorPropertiesNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubConvertCooperativeVectorMatrixNV(VkDevice,
                                                                           const VkConvertCooperativeVectorMatrixInfoNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer, uint32_t,
                                                                          const VkConvertCooperativeVectorMatrixInfoNV*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubSetLatencySleepModeNV(VkDevice, VkSwapchainKHR, const VkLatencySleepModeInfoNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubLatencySleepNV(VkDevice, VkSwapchainKHR, const VkLatencySleepInfoNV*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubSetLatencyMarkerNV(VkDevice, VkSwapchainKHR, const VkSetLatencyMarkerInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetLatencyTimingsNV(VkDevice, VkSwapchainKHR, VkGetLatencyMarkerInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubQueueNotifyOutOfBandNV(VkQueue, const VkOutOfBandQueueTypeInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer, VkImageAspectFlags) {}
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR VkResult VKAPI_CALL StubGetScreenBufferPropertiesQNX(VkDevice, const struct _screen_buffer*,
                                                                       VkScreenBufferPropertiesQNX*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
static VKAPI_ATTR void VKAPI_CALL StubGetClusterAccelerationStructureBuildSizesNV(VkDevice,
                                                                                  const VkClusterAccelerationStructureInputInfoNV*,
                                                                                  VkAccelerationStructureBuildSizesInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL
StubCmdBuildClusterAccelerationStructureIndirectNV(VkCommandBuffer, const VkClusterAccelerationStructureCommandsInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetPartitionedAccelerationStructuresBuildSizesNV(
    VkDevice, const VkPartitionedAccelerationStructureInstancesInputNV*, VkAccelerationStructureBuildSizesInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL
StubCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetGeneratedCommandsMemoryRequirementsEXT(VkDevice,
                                                                                const VkGeneratedCommandsMemoryRequirementsInfoEXT*,
                                                                                VkMemoryRequirements2*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer, const VkGeneratedCommandsInfoEXT*,
                                                                        VkCommandBuffer) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdExecuteGeneratedCommandsEXT(VkCommandBuffer, VkBool32, const VkGeneratedCommandsInfoEXT*) {
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIndirectCommandsLayoutEXT(VkDevice, const VkIndirectCommandsLayoutCreateInfoEXT*,
                                                                          const VkAllocationCallbacks*,
                                                                          VkIndirectCommandsLayoutEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyIndirectCommandsLayoutEXT(VkDevice, VkIndirectCommandsLayoutEXT,
                                                                       const VkAllocationCallbacks*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateIndirectExecutionSetEXT(VkDevice, const VkIndirectExecutionSetCreateInfoEXT*,
                                                                        const VkAllocationCallbacks*, VkIndirectExecutionSetEXT*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyIndirectExecutionSetEXT(VkDevice, VkIndirectExecutionSetEXT,
                                                                     const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateIndirectExecutionSetPipelineEXT(VkDevice, VkIndirectExecutionSetEXT, uint32_t,
                                                                            const VkWriteIndirectExecutionSetPipelineEXT*) {}
static VKAPI_ATTR void VKAPI_CALL StubUpdateIndirectExecutionSetShaderEXT(VkDevice, VkIndirectExecutionSetEXT, uint32_t,
                                                                          const VkWriteIndirectExecutionSetShaderEXT*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice, uint32_t*, VkCooperativeMatrixFlexibleDimensionsPropertiesNV*) {
    return VK_SUCCESS;
}
#ifdef VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryMetalHandleEXT(VkDevice, const VkMemoryGetMetalHandleInfoEXT*, void**) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetMemoryMetalHandlePropertiesEXT(VkDevice, VkExternalMemoryHandleTypeFlagBits,
                                                                            const void*, VkMemoryMetalHandlePropertiesEXT*) {
    return VK_SUCCESS;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateAccelerationStructureKHR(VkDevice, const VkAccelerationStructureCreateInfoKHR*,
                                                                         const VkAllocationCallbacks*,
                                                                         VkAccelerationStructureKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubDestroyAccelerationStructureKHR(VkDevice, VkAccelerationStructureKHR,
                                                                      const VkAllocationCallbacks*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructuresKHR(VkCommandBuffer, uint32_t,
                                                                        const VkAccelerationStructureBuildGeometryInfoKHR*,
                                                                        const VkAccelerationStructureBuildRangeInfoKHR* const*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer, uint32_t,
                                                                                const VkAccelerationStructureBuildGeometryInfoKHR*,
                                                                                const VkDeviceAddress*, const uint32_t*,
                                                                                const uint32_t* const*) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubBuildAccelerationStructuresKHR(VkDevice, VkDeferredOperationKHR, uint32_t,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR*,
                                                                         const VkAccelerationStructureBuildRangeInfoKHR* const*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyAccelerationStructureKHR(VkDevice, VkDeferredOperationKHR,
                                                                       const VkCopyAccelerationStructureInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyAccelerationStructureToMemoryKHR(VkDevice, VkDeferredOperationKHR,
                                                                               const VkCopyAccelerationStructureToMemoryInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubCopyMemoryToAccelerationStructureKHR(VkDevice, VkDeferredOperationKHR,
                                                                               const VkCopyMemoryToAccelerationStructureInfoKHR*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubWriteAccelerationStructuresPropertiesKHR(VkDevice, uint32_t,
                                                                                   const VkAccelerationStructureKHR*, VkQueryType,
                                                                                   size_t, void*, size_t) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureKHR(VkCommandBuffer, const VkCopyAccelerationStructureInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer,
                                                                              const VkCopyAccelerationStructureToMemoryInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer,
                                                                              const VkCopyMemoryToAccelerationStructureInfoKHR*) {}
static VKAPI_ATTR VkDeviceAddress VKAPI_CALL
StubGetAccelerationStructureDeviceAddressKHR(VkDevice, const VkAccelerationStructureDeviceAddressInfoKHR*) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer, uint32_t,
                                                                                  const VkAccelerationStructureKHR*, VkQueryType,
                                                                                  VkQueryPool, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubGetDeviceAccelerationStructureCompatibilityKHR(VkDevice,
                                                                                     const VkAccelerationStructureVersionInfoKHR*,
                                                                                     VkAccelerationStructureCompatibilityKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubGetAccelerationStructureBuildSizesKHR(VkDevice, VkAccelerationStructureBuildTypeKHR,
                                                                            const VkAccelerationStructureBuildGeometryInfoKHR*,
                                                                            const uint32_t*,
                                                                            VkAccelerationStructureBuildSizesInfoKHR*) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysKHR(VkCommandBuffer, const VkStridedDeviceAddressRegionKHR*,
                                                      const VkStridedDeviceAddressRegionKHR*,
                                                      const VkStridedDeviceAddressRegionKHR*,
                                                      const VkStridedDeviceAddressRegionKHR*, uint32_t, uint32_t, uint32_t) {}
static VKAPI_ATTR VkResult VKAPI_CALL StubCreateRayTracingPipelinesKHR(VkDevice, VkDeferredOperationKHR, VkPipelineCache, uint32_t,
                                                                       const VkRayTracingPipelineCreateInfoKHR*,
                                                                       const VkAllocationCallbacks*, VkPipeline*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL StubGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice, VkPipeline, uint32_t, uint32_t,
                                                                                          size_t, void*) {
    return VK_SUCCESS;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdTraceRaysIndirectKHR(VkCommandBuffer, const VkStridedDeviceAddressRegionKHR*,
                                                              const VkStridedDeviceAddressRegionKHR*,
                                                              const VkStridedDeviceAddressRegionKHR*,
                                                              const VkStridedDeviceAddressRegionKHR*, VkDeviceAddress) {}
static VKAPI_ATTR VkDeviceSize VKAPI_CALL StubGetRayTracingShaderGroupStackSizeKHR(VkDevice, VkPipeline, uint32_t,
                                                                                   VkShaderGroupShaderKHR) {
    return 0;
}
static VKAPI_ATTR void VKAPI_CALL StubCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksEXT(VkCommandBuffer, uint32_t, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectEXT(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t) {}
static VKAPI_ATTR void VKAPI_CALL StubCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer,
                                                                       VkDeviceSize, uint32_t, uint32_t) {}

const auto& GetApiPromotedMap() {
    static const vvl::unordered_map<std::string, std::string> api_promoted_map{
        {"vkBindBufferMemory2", {"VK_VERSION_1_1"}},
        {"vkBindImageMemory2", {"VK_VERSION_1_1"}},
        {"vkGetDeviceGroupPeerMemoryFeatures", {"VK_VERSION_1_1"}},
        {"vkCmdSetDeviceMask", {"VK_VERSION_1_1"}},
        {"vkCmdDispatchBase", {"VK_VERSION_1_1"}},
        {"vkGetImageMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkGetBufferMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkGetImageSparseMemoryRequirements2", {"VK_VERSION_1_1"}},
        {"vkTrimCommandPool", {"VK_VERSION_1_1"}},
        {"vkGetDeviceQueue2", {"VK_VERSION_1_1"}},
        {"vkCreateSamplerYcbcrConversion", {"VK_VERSION_1_1"}},
        {"vkDestroySamplerYcbcrConversion", {"VK_VERSION_1_1"}},
        {"vkCreateDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
        {"vkDestroyDescriptorUpdateTemplate", {"VK_VERSION_1_1"}},
        {"vkUpdateDescriptorSetWithTemplate", {"VK_VERSION_1_1"}},
        {"vkGetDescriptorSetLayoutSupport", {"VK_VERSION_1_1"}},
        {"vkCmdDrawIndirectCount", {"VK_VERSION_1_2"}},
        {"vkCmdDrawIndexedIndirectCount", {"VK_VERSION_1_2"}},
        {"vkCreateRenderPass2", {"VK_VERSION_1_2"}},
        {"vkCmdBeginRenderPass2", {"VK_VERSION_1_2"}},
        {"vkCmdNextSubpass2", {"VK_VERSION_1_2"}},
        {"vkCmdEndRenderPass2", {"VK_VERSION_1_2"}},
        {"vkResetQueryPool", {"VK_VERSION_1_2"}},
        {"vkGetSemaphoreCounterValue", {"VK_VERSION_1_2"}},
        {"vkWaitSemaphores", {"VK_VERSION_1_2"}},
        {"vkSignalSemaphore", {"VK_VERSION_1_2"}},
        {"vkGetBufferDeviceAddress", {"VK_VERSION_1_2"}},
        {"vkGetBufferOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
        {"vkGetDeviceMemoryOpaqueCaptureAddress", {"VK_VERSION_1_2"}},
        {"vkCreatePrivateDataSlot", {"VK_VERSION_1_3"}},
        {"vkDestroyPrivateDataSlot", {"VK_VERSION_1_3"}},
        {"vkSetPrivateData", {"VK_VERSION_1_3"}},
        {"vkGetPrivateData", {"VK_VERSION_1_3"}},
        {"vkCmdSetEvent2", {"VK_VERSION_1_3"}},
        {"vkCmdResetEvent2", {"VK_VERSION_1_3"}},
        {"vkCmdWaitEvents2", {"VK_VERSION_1_3"}},
        {"vkCmdPipelineBarrier2", {"VK_VERSION_1_3"}},
        {"vkCmdWriteTimestamp2", {"VK_VERSION_1_3"}},
        {"vkQueueSubmit2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyBuffer2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyImage2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyBufferToImage2", {"VK_VERSION_1_3"}},
        {"vkCmdCopyImageToBuffer2", {"VK_VERSION_1_3"}},
        {"vkCmdBlitImage2", {"VK_VERSION_1_3"}},
        {"vkCmdResolveImage2", {"VK_VERSION_1_3"}},
        {"vkCmdBeginRendering", {"VK_VERSION_1_3"}},
        {"vkCmdEndRendering", {"VK_VERSION_1_3"}},
        {"vkCmdSetCullMode", {"VK_VERSION_1_3"}},
        {"vkCmdSetFrontFace", {"VK_VERSION_1_3"}},
        {"vkCmdSetPrimitiveTopology", {"VK_VERSION_1_3"}},
        {"vkCmdSetViewportWithCount", {"VK_VERSION_1_3"}},
        {"vkCmdSetScissorWithCount", {"VK_VERSION_1_3"}},
        {"vkCmdBindVertexBuffers2", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthWriteEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthCompareOp", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthBoundsTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetStencilTestEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetStencilOp", {"VK_VERSION_1_3"}},
        {"vkCmdSetRasterizerDiscardEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetDepthBiasEnable", {"VK_VERSION_1_3"}},
        {"vkCmdSetPrimitiveRestartEnable", {"VK_VERSION_1_3"}},
        {"vkGetDeviceBufferMemoryRequirements", {"VK_VERSION_1_3"}},
        {"vkGetDeviceImageMemoryRequirements", {"VK_VERSION_1_3"}},
        {"vkGetDeviceImageSparseMemoryRequirements", {"VK_VERSION_1_3"}},
        {"vkCmdSetLineStipple", {"VK_VERSION_1_4"}},
        {"vkMapMemory2", {"VK_VERSION_1_4"}},
        {"vkUnmapMemory2", {"VK_VERSION_1_4"}},
        {"vkCmdBindIndexBuffer2", {"VK_VERSION_1_4"}},
        {"vkGetRenderingAreaGranularity", {"VK_VERSION_1_4"}},
        {"vkGetDeviceImageSubresourceLayout", {"VK_VERSION_1_4"}},
        {"vkGetImageSubresourceLayout2", {"VK_VERSION_1_4"}},
        {"vkCmdPushDescriptorSet", {"VK_VERSION_1_4"}},
        {"vkCmdPushDescriptorSetWithTemplate", {"VK_VERSION_1_4"}},
        {"vkCmdSetRenderingAttachmentLocations", {"VK_VERSION_1_4"}},
        {"vkCmdSetRenderingInputAttachmentIndices", {"VK_VERSION_1_4"}},
        {"vkCmdBindDescriptorSets2", {"VK_VERSION_1_4"}},
        {"vkCmdPushConstants2", {"VK_VERSION_1_4"}},
        {"vkCmdPushDescriptorSet2", {"VK_VERSION_1_4"}},
        {"vkCmdPushDescriptorSetWithTemplate2", {"VK_VERSION_1_4"}},
        {"vkCopyMemoryToImage", {"VK_VERSION_1_4"}},
        {"vkCopyImageToMemory", {"VK_VERSION_1_4"}},
        {"vkCopyImageToImage", {"VK_VERSION_1_4"}},
        {"vkTransitionImageLayout", {"VK_VERSION_1_4"}},
    };
    return api_promoted_map;
}
const auto& GetApiExtensionMap() {
    static const vvl::unordered_map<std::string, small_vector<vvl::Extension, 2, size_t>> api_extension_map{
        {"vkCreateSwapchainKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkDestroySwapchainKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkGetSwapchainImagesKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkAcquireNextImageKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkQueuePresentKHR", {vvl::Extension::_VK_KHR_swapchain}},
        {"vkGetDeviceGroupPresentCapabilitiesKHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkGetDeviceGroupSurfacePresentModesKHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkAcquireNextImage2KHR", {vvl::Extension::_VK_KHR_swapchain, vvl::Extension::_VK_KHR_device_group}},
        {"vkCreateSharedSwapchainsKHR", {vvl::Extension::_VK_KHR_display_swapchain}},
        {"vkCreateVideoSessionKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkDestroyVideoSessionKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkGetVideoSessionMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkBindVideoSessionMemoryKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCreateVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkUpdateVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkDestroyVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdBeginVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdEndVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdControlVideoCodingKHR", {vvl::Extension::_VK_KHR_video_queue}},
        {"vkCmdDecodeVideoKHR", {vvl::Extension::_VK_KHR_video_decode_queue}},
        {"vkCmdBeginRenderingKHR", {vvl::Extension::_VK_KHR_dynamic_rendering}},
        {"vkCmdEndRenderingKHR", {vvl::Extension::_VK_KHR_dynamic_rendering}},
        {"vkGetDeviceGroupPeerMemoryFeaturesKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkCmdSetDeviceMaskKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkCmdDispatchBaseKHR", {vvl::Extension::_VK_KHR_device_group}},
        {"vkTrimCommandPoolKHR", {vvl::Extension::_VK_KHR_maintenance1}},
        {"vkGetMemoryWin32HandleKHR", {vvl::Extension::_VK_KHR_external_memory_win32}},
        {"vkGetMemoryWin32HandlePropertiesKHR", {vvl::Extension::_VK_KHR_external_memory_win32}},
        {"vkGetMemoryFdKHR", {vvl::Extension::_VK_KHR_external_memory_fd}},
        {"vkGetMemoryFdPropertiesKHR", {vvl::Extension::_VK_KHR_external_memory_fd}},
        {"vkImportSemaphoreWin32HandleKHR", {vvl::Extension::_VK_KHR_external_semaphore_win32}},
        {"vkGetSemaphoreWin32HandleKHR", {vvl::Extension::_VK_KHR_external_semaphore_win32}},
        {"vkImportSemaphoreFdKHR", {vvl::Extension::_VK_KHR_external_semaphore_fd}},
        {"vkGetSemaphoreFdKHR", {vvl::Extension::_VK_KHR_external_semaphore_fd}},
        {"vkCmdPushDescriptorSetKHR", {vvl::Extension::_VK_KHR_push_descriptor}},
        {"vkCmdPushDescriptorSetWithTemplateKHR",
         {vvl::Extension::_VK_KHR_push_descriptor, vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkCreateDescriptorUpdateTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkDestroyDescriptorUpdateTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkUpdateDescriptorSetWithTemplateKHR", {vvl::Extension::_VK_KHR_descriptor_update_template}},
        {"vkCreateRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdBeginRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdNextSubpass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkCmdEndRenderPass2KHR", {vvl::Extension::_VK_KHR_create_renderpass2}},
        {"vkGetSwapchainStatusKHR", {vvl::Extension::_VK_KHR_shared_presentable_image}},
        {"vkImportFenceWin32HandleKHR", {vvl::Extension::_VK_KHR_external_fence_win32}},
        {"vkGetFenceWin32HandleKHR", {vvl::Extension::_VK_KHR_external_fence_win32}},
        {"vkImportFenceFdKHR", {vvl::Extension::_VK_KHR_external_fence_fd}},
        {"vkGetFenceFdKHR", {vvl::Extension::_VK_KHR_external_fence_fd}},
        {"vkAcquireProfilingLockKHR", {vvl::Extension::_VK_KHR_performance_query}},
        {"vkReleaseProfilingLockKHR", {vvl::Extension::_VK_KHR_performance_query}},
        {"vkGetImageMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkGetBufferMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkGetImageSparseMemoryRequirements2KHR", {vvl::Extension::_VK_KHR_get_memory_requirements2}},
        {"vkCreateSamplerYcbcrConversionKHR", {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion}},
        {"vkDestroySamplerYcbcrConversionKHR", {vvl::Extension::_VK_KHR_sampler_ycbcr_conversion}},
        {"vkBindBufferMemory2KHR", {vvl::Extension::_VK_KHR_bind_memory2}},
        {"vkBindImageMemory2KHR", {vvl::Extension::_VK_KHR_bind_memory2}},
        {"vkGetDescriptorSetLayoutSupportKHR", {vvl::Extension::_VK_KHR_maintenance3}},
        {"vkCmdDrawIndirectCountKHR", {vvl::Extension::_VK_KHR_draw_indirect_count}},
        {"vkCmdDrawIndexedIndirectCountKHR", {vvl::Extension::_VK_KHR_draw_indirect_count}},
        {"vkGetSemaphoreCounterValueKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkWaitSemaphoresKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkSignalSemaphoreKHR", {vvl::Extension::_VK_KHR_timeline_semaphore}},
        {"vkCmdSetFragmentShadingRateKHR", {vvl::Extension::_VK_KHR_fragment_shading_rate}},
        {"vkCmdSetRenderingAttachmentLocationsKHR", {vvl::Extension::_VK_KHR_dynamic_rendering_local_read}},
        {"vkCmdSetRenderingInputAttachmentIndicesKHR", {vvl::Extension::_VK_KHR_dynamic_rendering_local_read}},
        {"vkWaitForPresentKHR", {vvl::Extension::_VK_KHR_present_wait}},
        {"vkGetBufferDeviceAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkGetBufferOpaqueCaptureAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkGetDeviceMemoryOpaqueCaptureAddressKHR", {vvl::Extension::_VK_KHR_buffer_device_address}},
        {"vkCreateDeferredOperationKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkDestroyDeferredOperationKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetDeferredOperationMaxConcurrencyKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetDeferredOperationResultKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkDeferredOperationJoinKHR", {vvl::Extension::_VK_KHR_deferred_host_operations}},
        {"vkGetPipelineExecutablePropertiesKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkGetPipelineExecutableStatisticsKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkGetPipelineExecutableInternalRepresentationsKHR", {vvl::Extension::_VK_KHR_pipeline_executable_properties}},
        {"vkMapMemory2KHR", {vvl::Extension::_VK_KHR_map_memory2}},
        {"vkUnmapMemory2KHR", {vvl::Extension::_VK_KHR_map_memory2}},
        {"vkGetEncodedVideoSessionParametersKHR", {vvl::Extension::_VK_KHR_video_encode_queue}},
        {"vkCmdEncodeVideoKHR", {vvl::Extension::_VK_KHR_video_encode_queue}},
        {"vkCmdSetEvent2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdResetEvent2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdWaitEvents2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdPipelineBarrier2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdWriteTimestamp2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkQueueSubmit2KHR", {vvl::Extension::_VK_KHR_synchronization2}},
        {"vkCmdCopyBuffer2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyBufferToImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdCopyImageToBuffer2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdBlitImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdResolveImage2KHR", {vvl::Extension::_VK_KHR_copy_commands2}},
        {"vkCmdTraceRaysIndirect2KHR", {vvl::Extension::_VK_KHR_ray_tracing_maintenance1}},
        {"vkGetDeviceBufferMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkGetDeviceImageMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkGetDeviceImageSparseMemoryRequirementsKHR", {vvl::Extension::_VK_KHR_maintenance4}},
        {"vkCmdBindIndexBuffer2KHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetRenderingAreaGranularityKHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetDeviceImageSubresourceLayoutKHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkGetImageSubresourceLayout2KHR", {vvl::Extension::_VK_KHR_maintenance5}},
        {"vkCreatePipelineBinariesKHR", {vvl::Extension::_VK_KHR_pipeline_binary}},
        {"vkDestroyPipelineBinaryKHR", {vvl::Extension::_VK_KHR_pipeline_binary}},
        {"vkGetPipelineKeyKHR", {vvl::Extension::_VK_KHR_pipeline_binary}},
        {"vkGetPipelineBinaryDataKHR", {vvl::Extension::_VK_KHR_pipeline_binary}},
        {"vkReleaseCapturedPipelineDataKHR", {vvl::Extension::_VK_KHR_pipeline_binary}},
        {"vkCmdSetLineStippleKHR", {vvl::Extension::_VK_KHR_line_rasterization}},
        {"vkGetCalibratedTimestampsKHR", {vvl::Extension::_VK_KHR_calibrated_timestamps}},
        {"vkCmdBindDescriptorSets2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushConstants2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushDescriptorSet2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdPushDescriptorSetWithTemplate2KHR", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdSetDescriptorBufferOffsets2EXT", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkCmdBindDescriptorBufferEmbeddedSamplers2EXT", {vvl::Extension::_VK_KHR_maintenance6}},
        {"vkDebugMarkerSetObjectTagEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkDebugMarkerSetObjectNameEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerBeginEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerEndEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdDebugMarkerInsertEXT", {vvl::Extension::_VK_EXT_debug_marker}},
        {"vkCmdBindTransformFeedbackBuffersEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdBeginTransformFeedbackEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdEndTransformFeedbackEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdBeginQueryIndexedEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdEndQueryIndexedEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCmdDrawIndirectByteCountEXT", {vvl::Extension::_VK_EXT_transform_feedback}},
        {"vkCreateCuModuleNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkCreateCuFunctionNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkDestroyCuModuleNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkDestroyCuFunctionNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkCmdCuLaunchKernelNVX", {vvl::Extension::_VK_NVX_binary_import}},
        {"vkGetImageViewHandleNVX", {vvl::Extension::_VK_NVX_image_view_handle}},
        {"vkGetImageViewHandle64NVX", {vvl::Extension::_VK_NVX_image_view_handle}},
        {"vkGetImageViewAddressNVX", {vvl::Extension::_VK_NVX_image_view_handle}},
        {"vkCmdDrawIndirectCountAMD", {vvl::Extension::_VK_AMD_draw_indirect_count}},
        {"vkCmdDrawIndexedIndirectCountAMD", {vvl::Extension::_VK_AMD_draw_indirect_count}},
        {"vkGetShaderInfoAMD", {vvl::Extension::_VK_AMD_shader_info}},
        {"vkGetMemoryWin32HandleNV", {vvl::Extension::_VK_NV_external_memory_win32}},
        {"vkCmdBeginConditionalRenderingEXT", {vvl::Extension::_VK_EXT_conditional_rendering}},
        {"vkCmdEndConditionalRenderingEXT", {vvl::Extension::_VK_EXT_conditional_rendering}},
        {"vkCmdSetViewportWScalingNV", {vvl::Extension::_VK_NV_clip_space_w_scaling}},
        {"vkDisplayPowerControlEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkRegisterDeviceEventEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkRegisterDisplayEventEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkGetSwapchainCounterEXT", {vvl::Extension::_VK_EXT_display_control}},
        {"vkGetRefreshCycleDurationGOOGLE", {vvl::Extension::_VK_GOOGLE_display_timing}},
        {"vkGetPastPresentationTimingGOOGLE", {vvl::Extension::_VK_GOOGLE_display_timing}},
        {"vkCmdSetDiscardRectangleEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkCmdSetDiscardRectangleEnableEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkCmdSetDiscardRectangleModeEXT", {vvl::Extension::_VK_EXT_discard_rectangles}},
        {"vkSetHdrMetadataEXT", {vvl::Extension::_VK_EXT_hdr_metadata}},
        {"vkSetDebugUtilsObjectNameEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkSetDebugUtilsObjectTagEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueBeginDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueEndDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkQueueInsertDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdBeginDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdEndDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkCmdInsertDebugUtilsLabelEXT", {vvl::Extension::_VK_EXT_debug_utils}},
        {"vkGetAndroidHardwareBufferPropertiesANDROID", {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer}},
        {"vkGetMemoryAndroidHardwareBufferANDROID", {vvl::Extension::_VK_ANDROID_external_memory_android_hardware_buffer}},
        {"vkCreateExecutionGraphPipelinesAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkGetExecutionGraphPipelineScratchSizeAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkGetExecutionGraphPipelineNodeIndexAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdInitializeGraphScratchMemoryAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphIndirectAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdDispatchGraphIndirectCountAMDX", {vvl::Extension::_VK_AMDX_shader_enqueue}},
        {"vkCmdSetSampleLocationsEXT", {vvl::Extension::_VK_EXT_sample_locations}},
        {"vkGetImageDrmFormatModifierPropertiesEXT", {vvl::Extension::_VK_EXT_image_drm_format_modifier}},
        {"vkCreateValidationCacheEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkDestroyValidationCacheEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkMergeValidationCachesEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkGetValidationCacheDataEXT", {vvl::Extension::_VK_EXT_validation_cache}},
        {"vkCmdBindShadingRateImageNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCmdSetViewportShadingRatePaletteNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCmdSetCoarseSampleOrderNV", {vvl::Extension::_VK_NV_shading_rate_image}},
        {"vkCreateAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkDestroyAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetAccelerationStructureMemoryRequirementsNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkBindAccelerationStructureMemoryNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdBuildAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdCopyAccelerationStructureNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdTraceRaysNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCreateRayTracingPipelinesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetRayTracingShaderGroupHandlesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingShaderGroupHandlesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetAccelerationStructureHandleNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCmdWriteAccelerationStructuresPropertiesNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkCompileDeferredNV", {vvl::Extension::_VK_NV_ray_tracing}},
        {"vkGetMemoryHostPointerPropertiesEXT", {vvl::Extension::_VK_EXT_external_memory_host}},
        {"vkCmdWriteBufferMarkerAMD", {vvl::Extension::_VK_AMD_buffer_marker}},
        {"vkCmdWriteBufferMarker2AMD", {vvl::Extension::_VK_AMD_buffer_marker}},
        {"vkGetCalibratedTimestampsEXT", {vvl::Extension::_VK_EXT_calibrated_timestamps}},
        {"vkCmdDrawMeshTasksNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectCountNV", {vvl::Extension::_VK_NV_mesh_shader}},
        {"vkCmdSetExclusiveScissorEnableNV", {vvl::Extension::_VK_NV_scissor_exclusive}},
        {"vkCmdSetExclusiveScissorNV", {vvl::Extension::_VK_NV_scissor_exclusive}},
        {"vkCmdSetCheckpointNV", {vvl::Extension::_VK_NV_device_diagnostic_checkpoints}},
        {"vkGetQueueCheckpointDataNV", {vvl::Extension::_VK_NV_device_diagnostic_checkpoints}},
        {"vkGetQueueCheckpointData2NV", {vvl::Extension::_VK_NV_device_diagnostic_checkpoints}},
        {"vkInitializePerformanceApiINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkUninitializePerformanceApiINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceMarkerINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceStreamMarkerINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkCmdSetPerformanceOverrideINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkAcquirePerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkReleasePerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkQueueSetPerformanceConfigurationINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkGetPerformanceParameterINTEL", {vvl::Extension::_VK_INTEL_performance_query}},
        {"vkSetLocalDimmingAMD", {vvl::Extension::_VK_AMD_display_native_hdr}},
        {"vkGetBufferDeviceAddressEXT", {vvl::Extension::_VK_EXT_buffer_device_address}},
        {"vkAcquireFullScreenExclusiveModeEXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkReleaseFullScreenExclusiveModeEXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkGetDeviceGroupSurfacePresentModes2EXT", {vvl::Extension::_VK_EXT_full_screen_exclusive}},
        {"vkCmdSetLineStippleEXT", {vvl::Extension::_VK_EXT_line_rasterization}},
        {"vkResetQueryPoolEXT", {vvl::Extension::_VK_EXT_host_query_reset}},
        {"vkCmdSetCullModeEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetFrontFaceEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPrimitiveTopologyEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportWithCountEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetScissorWithCountEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdBindVertexBuffers2EXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthTestEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthWriteEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthCompareOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthBoundsTestEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetStencilTestEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetStencilOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCopyMemoryToImageEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkCopyImageToMemoryEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkCopyImageToImageEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkTransitionImageLayoutEXT", {vvl::Extension::_VK_EXT_host_image_copy}},
        {"vkGetImageSubresourceLayout2EXT",
         {vvl::Extension::_VK_EXT_host_image_copy, vvl::Extension::_VK_EXT_image_compression_control}},
        {"vkReleaseSwapchainImagesEXT", {vvl::Extension::_VK_EXT_swapchain_maintenance1}},
        {"vkGetGeneratedCommandsMemoryRequirementsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdPreprocessGeneratedCommandsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdExecuteGeneratedCommandsNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdBindPipelineShaderGroupNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCreateIndirectCommandsLayoutNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkDestroyIndirectCommandsLayoutNV", {vvl::Extension::_VK_NV_device_generated_commands}},
        {"vkCmdSetDepthBias2EXT", {vvl::Extension::_VK_EXT_depth_bias_control}},
        {"vkCreatePrivateDataSlotEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkDestroyPrivateDataSlotEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkSetPrivateDataEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkGetPrivateDataEXT", {vvl::Extension::_VK_EXT_private_data}},
        {"vkCreateCudaModuleNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkGetCudaModuleCacheNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkCreateCudaFunctionNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkDestroyCudaModuleNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkDestroyCudaFunctionNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkCmdCudaLaunchKernelNV", {vvl::Extension::_VK_NV_cuda_kernel_launch}},
        {"vkExportMetalObjectsEXT", {vvl::Extension::_VK_EXT_metal_objects}},
        {"vkGetDescriptorSetLayoutSizeEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetDescriptorSetLayoutBindingOffsetEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetDescriptorEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdBindDescriptorBuffersEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdSetDescriptorBufferOffsetsEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdBindDescriptorBufferEmbeddedSamplersEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetBufferOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetImageOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetImageViewOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetSamplerOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", {vvl::Extension::_VK_EXT_descriptor_buffer}},
        {"vkCmdSetFragmentShadingRateEnumNV", {vvl::Extension::_VK_NV_fragment_shading_rate_enums}},
        {"vkGetDeviceFaultInfoEXT", {vvl::Extension::_VK_EXT_device_fault}},
        {"vkCmdSetVertexInputEXT", {vvl::Extension::_VK_EXT_vertex_input_dynamic_state, vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetMemoryZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_memory}},
        {"vkGetMemoryZirconHandlePropertiesFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_memory}},
        {"vkImportSemaphoreZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_semaphore}},
        {"vkGetSemaphoreZirconHandleFUCHSIA", {vvl::Extension::_VK_FUCHSIA_external_semaphore}},
        {"vkCreateBufferCollectionFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkSetBufferCollectionImageConstraintsFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkSetBufferCollectionBufferConstraintsFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkDestroyBufferCollectionFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkGetBufferCollectionPropertiesFUCHSIA", {vvl::Extension::_VK_FUCHSIA_buffer_collection}},
        {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", {vvl::Extension::_VK_HUAWEI_subpass_shading}},
        {"vkCmdSubpassShadingHUAWEI", {vvl::Extension::_VK_HUAWEI_subpass_shading}},
        {"vkCmdBindInvocationMaskHUAWEI", {vvl::Extension::_VK_HUAWEI_invocation_mask}},
        {"vkGetMemoryRemoteAddressNV", {vvl::Extension::_VK_NV_external_memory_rdma}},
        {"vkGetPipelinePropertiesEXT", {vvl::Extension::_VK_EXT_pipeline_properties}},
        {"vkCmdSetPatchControlPointsEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizerDiscardEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthBiasEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLogicOpEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPrimitiveRestartEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state2, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorWriteEnableEXT", {vvl::Extension::_VK_EXT_color_write_enable}},
        {"vkCmdDrawMultiEXT", {vvl::Extension::_VK_EXT_multi_draw}},
        {"vkCmdDrawMultiIndexedEXT", {vvl::Extension::_VK_EXT_multi_draw}},
        {"vkCreateMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkDestroyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdBuildMicromapsEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkBuildMicromapsEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMicromapToMemoryEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCopyMemoryToMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkWriteMicromapsPropertiesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMicromapToMemoryEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdCopyMemoryToMicromapEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdWriteMicromapsPropertiesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkGetDeviceMicromapCompatibilityEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkGetMicromapBuildSizesEXT", {vvl::Extension::_VK_EXT_opacity_micromap}},
        {"vkCmdDrawClusterHUAWEI", {vvl::Extension::_VK_HUAWEI_cluster_culling_shader}},
        {"vkCmdDrawClusterIndirectHUAWEI", {vvl::Extension::_VK_HUAWEI_cluster_culling_shader}},
        {"vkSetDeviceMemoryPriorityEXT", {vvl::Extension::_VK_EXT_pageable_device_local_memory}},
        {"vkGetDescriptorSetLayoutHostMappingInfoVALVE", {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping}},
        {"vkGetDescriptorSetHostMappingVALVE", {vvl::Extension::_VK_VALVE_descriptor_set_host_mapping}},
        {"vkCmdCopyMemoryIndirectNV", {vvl::Extension::_VK_NV_copy_memory_indirect}},
        {"vkCmdCopyMemoryToImageIndirectNV", {vvl::Extension::_VK_NV_copy_memory_indirect}},
        {"vkCmdDecompressMemoryNV", {vvl::Extension::_VK_NV_memory_decompression}},
        {"vkCmdDecompressMemoryIndirectCountNV", {vvl::Extension::_VK_NV_memory_decompression}},
        {"vkGetPipelineIndirectMemoryRequirementsNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkCmdUpdatePipelineIndirectBufferNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkGetPipelineIndirectDeviceAddressNV", {vvl::Extension::_VK_NV_device_generated_commands_compute}},
        {"vkCmdSetDepthClampEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetPolygonModeEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizationSamplesEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetSampleMaskEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetAlphaToCoverageEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetAlphaToOneEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLogicOpEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendEquationEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorWriteMaskEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetTessellationDomainOriginEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRasterizationStreamEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetConservativeRasterizationModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetExtraPrimitiveOverestimationSizeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthClipEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetSampleLocationsEnableEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetColorBlendAdvancedEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetProvokingVertexModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLineRasterizationModeEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetLineStippleEnableEXT", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthClipNegativeOneToOneEXT",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportWScalingEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetViewportSwizzleNV", {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageToColorEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageToColorLocationNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationModeNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationTableEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageModulationTableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetShadingRateImageEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetRepresentativeFragmentTestEnableNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetCoverageReductionModeNV",
         {vvl::Extension::_VK_EXT_extended_dynamic_state3, vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetShaderModuleIdentifierEXT", {vvl::Extension::_VK_EXT_shader_module_identifier}},
        {"vkGetShaderModuleCreateInfoIdentifierEXT", {vvl::Extension::_VK_EXT_shader_module_identifier}},
        {"vkCreateOpticalFlowSessionNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkDestroyOpticalFlowSessionNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkBindOpticalFlowSessionImageNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkCmdOpticalFlowExecuteNV", {vvl::Extension::_VK_NV_optical_flow}},
        {"vkAntiLagUpdateAMD", {vvl::Extension::_VK_AMD_anti_lag}},
        {"vkCreateShadersEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkDestroyShaderEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkGetShaderBinaryDataEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdBindShadersEXT", {vvl::Extension::_VK_EXT_shader_object}},
        {"vkCmdSetDepthClampRangeEXT", {vvl::Extension::_VK_EXT_shader_object, vvl::Extension::_VK_EXT_depth_clamp_control}},
        {"vkGetFramebufferTilePropertiesQCOM", {vvl::Extension::_VK_QCOM_tile_properties}},
        {"vkGetDynamicRenderingTilePropertiesQCOM", {vvl::Extension::_VK_QCOM_tile_properties}},
        {"vkConvertCooperativeVectorMatrixNV", {vvl::Extension::_VK_NV_cooperative_vector}},
        {"vkCmdConvertCooperativeVectorMatrixNV", {vvl::Extension::_VK_NV_cooperative_vector}},
        {"vkSetLatencySleepModeNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkLatencySleepNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkSetLatencyMarkerNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkGetLatencyTimingsNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkQueueNotifyOutOfBandNV", {vvl::Extension::_VK_NV_low_latency2}},
        {"vkCmdSetAttachmentFeedbackLoopEnableEXT", {vvl::Extension::_VK_EXT_attachment_feedback_loop_dynamic_state}},
        {"vkGetScreenBufferPropertiesQNX", {vvl::Extension::_VK_QNX_external_memory_screen_buffer}},
        {"vkGetClusterAccelerationStructureBuildSizesNV", {vvl::Extension::_VK_NV_cluster_acceleration_structure}},
        {"vkCmdBuildClusterAccelerationStructureIndirectNV", {vvl::Extension::_VK_NV_cluster_acceleration_structure}},
        {"vkGetPartitionedAccelerationStructuresBuildSizesNV", {vvl::Extension::_VK_NV_partitioned_acceleration_structure}},
        {"vkCmdBuildPartitionedAccelerationStructuresNV", {vvl::Extension::_VK_NV_partitioned_acceleration_structure}},
        {"vkGetGeneratedCommandsMemoryRequirementsEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkCmdPreprocessGeneratedCommandsEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkCmdExecuteGeneratedCommandsEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkCreateIndirectCommandsLayoutEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkDestroyIndirectCommandsLayoutEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkCreateIndirectExecutionSetEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkDestroyIndirectExecutionSetEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkUpdateIndirectExecutionSetPipelineEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkUpdateIndirectExecutionSetShaderEXT", {vvl::Extension::_VK_EXT_device_generated_commands}},
        {"vkGetMemoryMetalHandleEXT", {vvl::Extension::_VK_EXT_external_memory_metal}},
        {"vkGetMemoryMetalHandlePropertiesEXT", {vvl::Extension::_VK_EXT_external_memory_metal}},
        {"vkCreateAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkDestroyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdBuildAccelerationStructuresKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdBuildAccelerationStructuresIndirectKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkBuildAccelerationStructuresKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyAccelerationStructureToMemoryKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCopyMemoryToAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkWriteAccelerationStructuresPropertiesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyAccelerationStructureToMemoryKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdCopyMemoryToAccelerationStructureKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetAccelerationStructureDeviceAddressKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdWriteAccelerationStructuresPropertiesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetDeviceAccelerationStructureCompatibilityKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkGetAccelerationStructureBuildSizesKHR", {vvl::Extension::_VK_KHR_acceleration_structure}},
        {"vkCmdTraceRaysKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCreateRayTracingPipelinesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdTraceRaysIndirectKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkGetRayTracingShaderGroupStackSizeKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdSetRayTracingPipelineStackSizeKHR", {vvl::Extension::_VK_KHR_ray_tracing_pipeline}},
        {"vkCmdDrawMeshTasksEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
        {"vkCmdDrawMeshTasksIndirectCountEXT", {vvl::Extension::_VK_EXT_mesh_shader}},
    };
    return api_extension_map;
}

// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
//   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info) {
    auto promoted_api = GetApiPromotedMap().find(api_name);
    if (promoted_api != GetApiPromotedMap().end()) {
        auto info = GetDeviceVersionMap(promoted_api->second.c_str());
        assert(info.state);
        return (device_extension_info->*(info.state) == kEnabledByCreateinfo);
    }

    auto has_ext = GetApiExtensionMap().find(api_name);
    // Is this API part of an extension or feature group?
    if (has_ext != GetApiExtensionMap().end()) {
        // Was the extension for this API enabled in the CreateDevice call?
        for (const auto& extension : has_ext->second) {
            auto info = device_extension_info->GetInfo(extension);
            if (info.state) {
                if (device_extension_info->*(info.state) == kEnabledByCreateinfo ||
                    device_extension_info->*(info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }

        // Was the extension for this API enabled in the CreateInstance call?
        auto instance_extension_info = static_cast<const InstanceExtensions*>(device_extension_info);
        for (const auto& extension : has_ext->second) {
            auto info = instance_extension_info->GetInfo(extension);
            if (info.state) {
                if (instance_extension_info->*(info.state) == kEnabledByCreateinfo ||
                    instance_extension_info->*(info.state) == kEnabledByInteraction) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Device function pointers
    table->GetDeviceProcAddr = gpa;
    table->DestroyDevice = (PFN_vkDestroyDevice)gpa(device, "vkDestroyDevice");
    table->GetDeviceQueue = (PFN_vkGetDeviceQueue)gpa(device, "vkGetDeviceQueue");
    table->QueueSubmit = (PFN_vkQueueSubmit)gpa(device, "vkQueueSubmit");
    table->QueueWaitIdle = (PFN_vkQueueWaitIdle)gpa(device, "vkQueueWaitIdle");
    table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle)gpa(device, "vkDeviceWaitIdle");
    table->AllocateMemory = (PFN_vkAllocateMemory)gpa(device, "vkAllocateMemory");
    table->FreeMemory = (PFN_vkFreeMemory)gpa(device, "vkFreeMemory");
    table->MapMemory = (PFN_vkMapMemory)gpa(device, "vkMapMemory");
    table->UnmapMemory = (PFN_vkUnmapMemory)gpa(device, "vkUnmapMemory");
    table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)gpa(device, "vkFlushMappedMemoryRanges");
    table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)gpa(device, "vkInvalidateMappedMemoryRanges");
    table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)gpa(device, "vkGetDeviceMemoryCommitment");
    table->BindBufferMemory = (PFN_vkBindBufferMemory)gpa(device, "vkBindBufferMemory");
    table->BindImageMemory = (PFN_vkBindImageMemory)gpa(device, "vkBindImageMemory");
    table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)gpa(device, "vkGetBufferMemoryRequirements");
    table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)gpa(device, "vkGetImageMemoryRequirements");
    table->GetImageSparseMemoryRequirements =
        (PFN_vkGetImageSparseMemoryRequirements)gpa(device, "vkGetImageSparseMemoryRequirements");
    table->QueueBindSparse = (PFN_vkQueueBindSparse)gpa(device, "vkQueueBindSparse");
    table->CreateFence = (PFN_vkCreateFence)gpa(device, "vkCreateFence");
    table->DestroyFence = (PFN_vkDestroyFence)gpa(device, "vkDestroyFence");
    table->ResetFences = (PFN_vkResetFences)gpa(device, "vkResetFences");
    table->GetFenceStatus = (PFN_vkGetFenceStatus)gpa(device, "vkGetFenceStatus");
    table->WaitForFences = (PFN_vkWaitForFences)gpa(device, "vkWaitForFences");
    table->CreateSemaphore = (PFN_vkCreateSemaphore)gpa(device, "vkCreateSemaphore");
    table->DestroySemaphore = (PFN_vkDestroySemaphore)gpa(device, "vkDestroySemaphore");
    table->CreateEvent = (PFN_vkCreateEvent)gpa(device, "vkCreateEvent");
    table->DestroyEvent = (PFN_vkDestroyEvent)gpa(device, "vkDestroyEvent");
    table->GetEventStatus = (PFN_vkGetEventStatus)gpa(device, "vkGetEventStatus");
    table->SetEvent = (PFN_vkSetEvent)gpa(device, "vkSetEvent");
    table->ResetEvent = (PFN_vkResetEvent)gpa(device, "vkResetEvent");
    table->CreateQueryPool = (PFN_vkCreateQueryPool)gpa(device, "vkCreateQueryPool");
    table->DestroyQueryPool = (PFN_vkDestroyQueryPool)gpa(device, "vkDestroyQueryPool");
    table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults)gpa(device, "vkGetQueryPoolResults");
    table->CreateBuffer = (PFN_vkCreateBuffer)gpa(device, "vkCreateBuffer");
    table->DestroyBuffer = (PFN_vkDestroyBuffer)gpa(device, "vkDestroyBuffer");
    table->CreateBufferView = (PFN_vkCreateBufferView)gpa(device, "vkCreateBufferView");
    table->DestroyBufferView = (PFN_vkDestroyBufferView)gpa(device, "vkDestroyBufferView");
    table->CreateImage = (PFN_vkCreateImage)gpa(device, "vkCreateImage");
    table->DestroyImage = (PFN_vkDestroyImage)gpa(device, "vkDestroyImage");
    table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)gpa(device, "vkGetImageSubresourceLayout");
    table->CreateImageView = (PFN_vkCreateImageView)gpa(device, "vkCreateImageView");
    table->DestroyImageView = (PFN_vkDestroyImageView)gpa(device, "vkDestroyImageView");
    table->CreateShaderModule = (PFN_vkCreateShaderModule)gpa(device, "vkCreateShaderModule");
    table->DestroyShaderModule = (PFN_vkDestroyShaderModule)gpa(device, "vkDestroyShaderModule");
    table->CreatePipelineCache = (PFN_vkCreatePipelineCache)gpa(device, "vkCreatePipelineCache");
    table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache)gpa(device, "vkDestroyPipelineCache");
    table->GetPipelineCacheData = (PFN_vkGetPipelineCacheData)gpa(device, "vkGetPipelineCacheData");
    table->MergePipelineCaches = (PFN_vkMergePipelineCaches)gpa(device, "vkMergePipelineCaches");
    table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)gpa(device, "vkCreateGraphicsPipelines");
    table->CreateComputePipelines = (PFN_vkCreateComputePipelines)gpa(device, "vkCreateComputePipelines");
    table->DestroyPipeline = (PFN_vkDestroyPipeline)gpa(device, "vkDestroyPipeline");
    table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout)gpa(device, "vkCreatePipelineLayout");
    table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)gpa(device, "vkDestroyPipelineLayout");
    table->CreateSampler = (PFN_vkCreateSampler)gpa(device, "vkCreateSampler");
    table->DestroySampler = (PFN_vkDestroySampler)gpa(device, "vkDestroySampler");
    table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)gpa(device, "vkCreateDescriptorSetLayout");
    table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)gpa(device, "vkDestroyDescriptorSetLayout");
    table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool)gpa(device, "vkCreateDescriptorPool");
    table->DestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)gpa(device, "vkDestroyDescriptorPool");
    table->ResetDescriptorPool = (PFN_vkResetDescriptorPool)gpa(device, "vkResetDescriptorPool");
    table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)gpa(device, "vkAllocateDescriptorSets");
    table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets)gpa(device, "vkFreeDescriptorSets");
    table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)gpa(device, "vkUpdateDescriptorSets");
    table->CreateFramebuffer = (PFN_vkCreateFramebuffer)gpa(device, "vkCreateFramebuffer");
    table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer)gpa(device, "vkDestroyFramebuffer");
    table->CreateRenderPass = (PFN_vkCreateRenderPass)gpa(device, "vkCreateRenderPass");
    table->DestroyRenderPass = (PFN_vkDestroyRenderPass)gpa(device, "vkDestroyRenderPass");
    table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)gpa(device, "vkGetRenderAreaGranularity");
    table->CreateCommandPool = (PFN_vkCreateCommandPool)gpa(device, "vkCreateCommandPool");
    table->DestroyCommandPool = (PFN_vkDestroyCommandPool)gpa(device, "vkDestroyCommandPool");
    table->ResetCommandPool = (PFN_vkResetCommandPool)gpa(device, "vkResetCommandPool");
    table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)gpa(device, "vkAllocateCommandBuffers");
    table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers)gpa(device, "vkFreeCommandBuffers");
    table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer)gpa(device, "vkBeginCommandBuffer");
    table->EndCommandBuffer = (PFN_vkEndCommandBuffer)gpa(device, "vkEndCommandBuffer");
    table->ResetCommandBuffer = (PFN_vkResetCommandBuffer)gpa(device, "vkResetCommandBuffer");
    table->CmdBindPipeline = (PFN_vkCmdBindPipeline)gpa(device, "vkCmdBindPipeline");
    table->CmdSetViewport = (PFN_vkCmdSetViewport)gpa(device, "vkCmdSetViewport");
    table->CmdSetScissor = (PFN_vkCmdSetScissor)gpa(device, "vkCmdSetScissor");
    table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth)gpa(device, "vkCmdSetLineWidth");
    table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias)gpa(device, "vkCmdSetDepthBias");
    table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)gpa(device, "vkCmdSetBlendConstants");
    table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)gpa(device, "vkCmdSetDepthBounds");
    table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)gpa(device, "vkCmdSetStencilCompareMask");
    table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)gpa(device, "vkCmdSetStencilWriteMask");
    table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference)gpa(device, "vkCmdSetStencilReference");
    table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)gpa(device, "vkCmdBindDescriptorSets");
    table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)gpa(device, "vkCmdBindIndexBuffer");
    table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)gpa(device, "vkCmdBindVertexBuffers");
    table->CmdDraw = (PFN_vkCmdDraw)gpa(device, "vkCmdDraw");
    table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed)gpa(device, "vkCmdDrawIndexed");
    table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect)gpa(device, "vkCmdDrawIndirect");
    table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)gpa(device, "vkCmdDrawIndexedIndirect");
    table->CmdDispatch = (PFN_vkCmdDispatch)gpa(device, "vkCmdDispatch");
    table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)gpa(device, "vkCmdDispatchIndirect");
    table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer)gpa(device, "vkCmdCopyBuffer");
    table->CmdCopyImage = (PFN_vkCmdCopyImage)gpa(device, "vkCmdCopyImage");
    table->CmdBlitImage = (PFN_vkCmdBlitImage)gpa(device, "vkCmdBlitImage");
    table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)gpa(device, "vkCmdCopyBufferToImage");
    table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)gpa(device, "vkCmdCopyImageToBuffer");
    table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)gpa(device, "vkCmdUpdateBuffer");
    table->CmdFillBuffer = (PFN_vkCmdFillBuffer)gpa(device, "vkCmdFillBuffer");
    table->CmdClearColorImage = (PFN_vkCmdClearColorImage)gpa(device, "vkCmdClearColorImage");
    table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)gpa(device, "vkCmdClearDepthStencilImage");
    table->CmdClearAttachments = (PFN_vkCmdClearAttachments)gpa(device, "vkCmdClearAttachments");
    table->CmdResolveImage = (PFN_vkCmdResolveImage)gpa(device, "vkCmdResolveImage");
    table->CmdSetEvent = (PFN_vkCmdSetEvent)gpa(device, "vkCmdSetEvent");
    table->CmdResetEvent = (PFN_vkCmdResetEvent)gpa(device, "vkCmdResetEvent");
    table->CmdWaitEvents = (PFN_vkCmdWaitEvents)gpa(device, "vkCmdWaitEvents");
    table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)gpa(device, "vkCmdPipelineBarrier");
    table->CmdBeginQuery = (PFN_vkCmdBeginQuery)gpa(device, "vkCmdBeginQuery");
    table->CmdEndQuery = (PFN_vkCmdEndQuery)gpa(device, "vkCmdEndQuery");
    table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool)gpa(device, "vkCmdResetQueryPool");
    table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)gpa(device, "vkCmdWriteTimestamp");
    table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)gpa(device, "vkCmdCopyQueryPoolResults");
    table->CmdPushConstants = (PFN_vkCmdPushConstants)gpa(device, "vkCmdPushConstants");
    table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)gpa(device, "vkCmdBeginRenderPass");
    table->CmdNextSubpass = (PFN_vkCmdNextSubpass)gpa(device, "vkCmdNextSubpass");
    table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass)gpa(device, "vkCmdEndRenderPass");
    table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands)gpa(device, "vkCmdExecuteCommands");
    table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)gpa(device, "vkBindBufferMemory2");
    if (table->BindBufferMemory2 == nullptr) {
        table->BindBufferMemory2 = (PFN_vkBindBufferMemory2)StubBindBufferMemory2;
    }
    table->BindImageMemory2 = (PFN_vkBindImageMemory2)gpa(device, "vkBindImageMemory2");
    if (table->BindImageMemory2 == nullptr) {
        table->BindImageMemory2 = (PFN_vkBindImageMemory2)StubBindImageMemory2;
    }
    table->GetDeviceGroupPeerMemoryFeatures =
        (PFN_vkGetDeviceGroupPeerMemoryFeatures)gpa(device, "vkGetDeviceGroupPeerMemoryFeatures");
    if (table->GetDeviceGroupPeerMemoryFeatures == nullptr) {
        table->GetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)StubGetDeviceGroupPeerMemoryFeatures;
    }
    table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)gpa(device, "vkCmdSetDeviceMask");
    if (table->CmdSetDeviceMask == nullptr) {
        table->CmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)StubCmdSetDeviceMask;
    }
    table->CmdDispatchBase = (PFN_vkCmdDispatchBase)gpa(device, "vkCmdDispatchBase");
    if (table->CmdDispatchBase == nullptr) {
        table->CmdDispatchBase = (PFN_vkCmdDispatchBase)StubCmdDispatchBase;
    }
    table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)gpa(device, "vkGetImageMemoryRequirements2");
    if (table->GetImageMemoryRequirements2 == nullptr) {
        table->GetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)StubGetImageMemoryRequirements2;
    }
    table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)gpa(device, "vkGetBufferMemoryRequirements2");
    if (table->GetBufferMemoryRequirements2 == nullptr) {
        table->GetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)StubGetBufferMemoryRequirements2;
    }
    table->GetImageSparseMemoryRequirements2 =
        (PFN_vkGetImageSparseMemoryRequirements2)gpa(device, "vkGetImageSparseMemoryRequirements2");
    if (table->GetImageSparseMemoryRequirements2 == nullptr) {
        table->GetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)StubGetImageSparseMemoryRequirements2;
    }
    table->TrimCommandPool = (PFN_vkTrimCommandPool)gpa(device, "vkTrimCommandPool");
    if (table->TrimCommandPool == nullptr) {
        table->TrimCommandPool = (PFN_vkTrimCommandPool)StubTrimCommandPool;
    }
    table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)gpa(device, "vkGetDeviceQueue2");
    if (table->GetDeviceQueue2 == nullptr) {
        table->GetDeviceQueue2 = (PFN_vkGetDeviceQueue2)StubGetDeviceQueue2;
    }
    table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)gpa(device, "vkCreateSamplerYcbcrConversion");
    if (table->CreateSamplerYcbcrConversion == nullptr) {
        table->CreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)StubCreateSamplerYcbcrConversion;
    }
    table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)gpa(device, "vkDestroySamplerYcbcrConversion");
    if (table->DestroySamplerYcbcrConversion == nullptr) {
        table->DestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)StubDestroySamplerYcbcrConversion;
    }
    table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)gpa(device, "vkCreateDescriptorUpdateTemplate");
    if (table->CreateDescriptorUpdateTemplate == nullptr) {
        table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)StubCreateDescriptorUpdateTemplate;
    }
    table->DestroyDescriptorUpdateTemplate =
        (PFN_vkDestroyDescriptorUpdateTemplate)gpa(device, "vkDestroyDescriptorUpdateTemplate");
    if (table->DestroyDescriptorUpdateTemplate == nullptr) {
        table->DestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)StubDestroyDescriptorUpdateTemplate;
    }
    table->UpdateDescriptorSetWithTemplate =
        (PFN_vkUpdateDescriptorSetWithTemplate)gpa(device, "vkUpdateDescriptorSetWithTemplate");
    if (table->UpdateDescriptorSetWithTemplate == nullptr) {
        table->UpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)StubUpdateDescriptorSetWithTemplate;
    }
    table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)gpa(device, "vkGetDescriptorSetLayoutSupport");
    if (table->GetDescriptorSetLayoutSupport == nullptr) {
        table->GetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)StubGetDescriptorSetLayoutSupport;
    }
    table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)gpa(device, "vkCmdDrawIndirectCount");
    if (table->CmdDrawIndirectCount == nullptr) {
        table->CmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)StubCmdDrawIndirectCount;
    }
    table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)gpa(device, "vkCmdDrawIndexedIndirectCount");
    if (table->CmdDrawIndexedIndirectCount == nullptr) {
        table->CmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)StubCmdDrawIndexedIndirectCount;
    }
    table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)gpa(device, "vkCreateRenderPass2");
    if (table->CreateRenderPass2 == nullptr) {
        table->CreateRenderPass2 = (PFN_vkCreateRenderPass2)StubCreateRenderPass2;
    }
    table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)gpa(device, "vkCmdBeginRenderPass2");
    if (table->CmdBeginRenderPass2 == nullptr) {
        table->CmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)StubCmdBeginRenderPass2;
    }
    table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)gpa(device, "vkCmdNextSubpass2");
    if (table->CmdNextSubpass2 == nullptr) {
        table->CmdNextSubpass2 = (PFN_vkCmdNextSubpass2)StubCmdNextSubpass2;
    }
    table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)gpa(device, "vkCmdEndRenderPass2");
    if (table->CmdEndRenderPass2 == nullptr) {
        table->CmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)StubCmdEndRenderPass2;
    }
    table->ResetQueryPool = (PFN_vkResetQueryPool)gpa(device, "vkResetQueryPool");
    if (table->ResetQueryPool == nullptr) {
        table->ResetQueryPool = (PFN_vkResetQueryPool)StubResetQueryPool;
    }
    table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)gpa(device, "vkGetSemaphoreCounterValue");
    if (table->GetSemaphoreCounterValue == nullptr) {
        table->GetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)StubGetSemaphoreCounterValue;
    }
    table->WaitSemaphores = (PFN_vkWaitSemaphores)gpa(device, "vkWaitSemaphores");
    if (table->WaitSemaphores == nullptr) {
        table->WaitSemaphores = (PFN_vkWaitSemaphores)StubWaitSemaphores;
    }
    table->SignalSemaphore = (PFN_vkSignalSemaphore)gpa(device, "vkSignalSemaphore");
    if (table->SignalSemaphore == nullptr) {
        table->SignalSemaphore = (PFN_vkSignalSemaphore)StubSignalSemaphore;
    }
    table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)gpa(device, "vkGetBufferDeviceAddress");
    if (table->GetBufferDeviceAddress == nullptr) {
        table->GetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)StubGetBufferDeviceAddress;
    }
    table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)gpa(device, "vkGetBufferOpaqueCaptureAddress");
    if (table->GetBufferOpaqueCaptureAddress == nullptr) {
        table->GetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)StubGetBufferOpaqueCaptureAddress;
    }
    table->GetDeviceMemoryOpaqueCaptureAddress =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
    if (table->GetDeviceMemoryOpaqueCaptureAddress == nullptr) {
        table->GetDeviceMemoryOpaqueCaptureAddress =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)StubGetDeviceMemoryOpaqueCaptureAddress;
    }
    table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)gpa(device, "vkCreatePrivateDataSlot");
    if (table->CreatePrivateDataSlot == nullptr) {
        table->CreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)StubCreatePrivateDataSlot;
    }
    table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)gpa(device, "vkDestroyPrivateDataSlot");
    if (table->DestroyPrivateDataSlot == nullptr) {
        table->DestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)StubDestroyPrivateDataSlot;
    }
    table->SetPrivateData = (PFN_vkSetPrivateData)gpa(device, "vkSetPrivateData");
    if (table->SetPrivateData == nullptr) {
        table->SetPrivateData = (PFN_vkSetPrivateData)StubSetPrivateData;
    }
    table->GetPrivateData = (PFN_vkGetPrivateData)gpa(device, "vkGetPrivateData");
    if (table->GetPrivateData == nullptr) {
        table->GetPrivateData = (PFN_vkGetPrivateData)StubGetPrivateData;
    }
    table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)gpa(device, "vkCmdSetEvent2");
    if (table->CmdSetEvent2 == nullptr) {
        table->CmdSetEvent2 = (PFN_vkCmdSetEvent2)StubCmdSetEvent2;
    }
    table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)gpa(device, "vkCmdResetEvent2");
    if (table->CmdResetEvent2 == nullptr) {
        table->CmdResetEvent2 = (PFN_vkCmdResetEvent2)StubCmdResetEvent2;
    }
    table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)gpa(device, "vkCmdWaitEvents2");
    if (table->CmdWaitEvents2 == nullptr) {
        table->CmdWaitEvents2 = (PFN_vkCmdWaitEvents2)StubCmdWaitEvents2;
    }
    table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)gpa(device, "vkCmdPipelineBarrier2");
    if (table->CmdPipelineBarrier2 == nullptr) {
        table->CmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)StubCmdPipelineBarrier2;
    }
    table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)gpa(device, "vkCmdWriteTimestamp2");
    if (table->CmdWriteTimestamp2 == nullptr) {
        table->CmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)StubCmdWriteTimestamp2;
    }
    table->QueueSubmit2 = (PFN_vkQueueSubmit2)gpa(device, "vkQueueSubmit2");
    if (table->QueueSubmit2 == nullptr) {
        table->QueueSubmit2 = (PFN_vkQueueSubmit2)StubQueueSubmit2;
    }
    table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)gpa(device, "vkCmdCopyBuffer2");
    if (table->CmdCopyBuffer2 == nullptr) {
        table->CmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)StubCmdCopyBuffer2;
    }
    table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)gpa(device, "vkCmdCopyImage2");
    if (table->CmdCopyImage2 == nullptr) {
        table->CmdCopyImage2 = (PFN_vkCmdCopyImage2)StubCmdCopyImage2;
    }
    table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)gpa(device, "vkCmdCopyBufferToImage2");
    if (table->CmdCopyBufferToImage2 == nullptr) {
        table->CmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)StubCmdCopyBufferToImage2;
    }
    table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)gpa(device, "vkCmdCopyImageToBuffer2");
    if (table->CmdCopyImageToBuffer2 == nullptr) {
        table->CmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)StubCmdCopyImageToBuffer2;
    }
    table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)gpa(device, "vkCmdBlitImage2");
    if (table->CmdBlitImage2 == nullptr) {
        table->CmdBlitImage2 = (PFN_vkCmdBlitImage2)StubCmdBlitImage2;
    }
    table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)gpa(device, "vkCmdResolveImage2");
    if (table->CmdResolveImage2 == nullptr) {
        table->CmdResolveImage2 = (PFN_vkCmdResolveImage2)StubCmdResolveImage2;
    }
    table->CmdBeginRendering = (PFN_vkCmdBeginRendering)gpa(device, "vkCmdBeginRendering");
    if (table->CmdBeginRendering == nullptr) {
        table->CmdBeginRendering = (PFN_vkCmdBeginRendering)StubCmdBeginRendering;
    }
    table->CmdEndRendering = (PFN_vkCmdEndRendering)gpa(device, "vkCmdEndRendering");
    if (table->CmdEndRendering == nullptr) {
        table->CmdEndRendering = (PFN_vkCmdEndRendering)StubCmdEndRendering;
    }
    table->CmdSetCullMode = (PFN_vkCmdSetCullMode)gpa(device, "vkCmdSetCullMode");
    if (table->CmdSetCullMode == nullptr) {
        table->CmdSetCullMode = (PFN_vkCmdSetCullMode)StubCmdSetCullMode;
    }
    table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)gpa(device, "vkCmdSetFrontFace");
    if (table->CmdSetFrontFace == nullptr) {
        table->CmdSetFrontFace = (PFN_vkCmdSetFrontFace)StubCmdSetFrontFace;
    }
    table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)gpa(device, "vkCmdSetPrimitiveTopology");
    if (table->CmdSetPrimitiveTopology == nullptr) {
        table->CmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)StubCmdSetPrimitiveTopology;
    }
    table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)gpa(device, "vkCmdSetViewportWithCount");
    if (table->CmdSetViewportWithCount == nullptr) {
        table->CmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)StubCmdSetViewportWithCount;
    }
    table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)gpa(device, "vkCmdSetScissorWithCount");
    if (table->CmdSetScissorWithCount == nullptr) {
        table->CmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)StubCmdSetScissorWithCount;
    }
    table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)gpa(device, "vkCmdBindVertexBuffers2");
    if (table->CmdBindVertexBuffers2 == nullptr) {
        table->CmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)StubCmdBindVertexBuffers2;
    }
    table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)gpa(device, "vkCmdSetDepthTestEnable");
    if (table->CmdSetDepthTestEnable == nullptr) {
        table->CmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)StubCmdSetDepthTestEnable;
    }
    table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)gpa(device, "vkCmdSetDepthWriteEnable");
    if (table->CmdSetDepthWriteEnable == nullptr) {
        table->CmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)StubCmdSetDepthWriteEnable;
    }
    table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)gpa(device, "vkCmdSetDepthCompareOp");
    if (table->CmdSetDepthCompareOp == nullptr) {
        table->CmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)StubCmdSetDepthCompareOp;
    }
    table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)gpa(device, "vkCmdSetDepthBoundsTestEnable");
    if (table->CmdSetDepthBoundsTestEnable == nullptr) {
        table->CmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)StubCmdSetDepthBoundsTestEnable;
    }
    table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)gpa(device, "vkCmdSetStencilTestEnable");
    if (table->CmdSetStencilTestEnable == nullptr) {
        table->CmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)StubCmdSetStencilTestEnable;
    }
    table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)gpa(device, "vkCmdSetStencilOp");
    if (table->CmdSetStencilOp == nullptr) {
        table->CmdSetStencilOp = (PFN_vkCmdSetStencilOp)StubCmdSetStencilOp;
    }
    table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)gpa(device, "vkCmdSetRasterizerDiscardEnable");
    if (table->CmdSetRasterizerDiscardEnable == nullptr) {
        table->CmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)StubCmdSetRasterizerDiscardEnable;
    }
    table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)gpa(device, "vkCmdSetDepthBiasEnable");
    if (table->CmdSetDepthBiasEnable == nullptr) {
        table->CmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)StubCmdSetDepthBiasEnable;
    }
    table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)gpa(device, "vkCmdSetPrimitiveRestartEnable");
    if (table->CmdSetPrimitiveRestartEnable == nullptr) {
        table->CmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)StubCmdSetPrimitiveRestartEnable;
    }
    table->GetDeviceBufferMemoryRequirements =
        (PFN_vkGetDeviceBufferMemoryRequirements)gpa(device, "vkGetDeviceBufferMemoryRequirements");
    if (table->GetDeviceBufferMemoryRequirements == nullptr) {
        table->GetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)StubGetDeviceBufferMemoryRequirements;
    }
    table->GetDeviceImageMemoryRequirements =
        (PFN_vkGetDeviceImageMemoryRequirements)gpa(device, "vkGetDeviceImageMemoryRequirements");
    if (table->GetDeviceImageMemoryRequirements == nullptr) {
        table->GetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)StubGetDeviceImageMemoryRequirements;
    }
    table->GetDeviceImageSparseMemoryRequirements =
        (PFN_vkGetDeviceImageSparseMemoryRequirements)gpa(device, "vkGetDeviceImageSparseMemoryRequirements");
    if (table->GetDeviceImageSparseMemoryRequirements == nullptr) {
        table->GetDeviceImageSparseMemoryRequirements =
            (PFN_vkGetDeviceImageSparseMemoryRequirements)StubGetDeviceImageSparseMemoryRequirements;
    }
    table->CmdSetLineStipple = (PFN_vkCmdSetLineStipple)gpa(device, "vkCmdSetLineStipple");
    if (table->CmdSetLineStipple == nullptr) {
        table->CmdSetLineStipple = (PFN_vkCmdSetLineStipple)StubCmdSetLineStipple;
    }
    table->MapMemory2 = (PFN_vkMapMemory2)gpa(device, "vkMapMemory2");
    if (table->MapMemory2 == nullptr) {
        table->MapMemory2 = (PFN_vkMapMemory2)StubMapMemory2;
    }
    table->UnmapMemory2 = (PFN_vkUnmapMemory2)gpa(device, "vkUnmapMemory2");
    if (table->UnmapMemory2 == nullptr) {
        table->UnmapMemory2 = (PFN_vkUnmapMemory2)StubUnmapMemory2;
    }
    table->CmdBindIndexBuffer2 = (PFN_vkCmdBindIndexBuffer2)gpa(device, "vkCmdBindIndexBuffer2");
    if (table->CmdBindIndexBuffer2 == nullptr) {
        table->CmdBindIndexBuffer2 = (PFN_vkCmdBindIndexBuffer2)StubCmdBindIndexBuffer2;
    }
    table->GetRenderingAreaGranularity = (PFN_vkGetRenderingAreaGranularity)gpa(device, "vkGetRenderingAreaGranularity");
    if (table->GetRenderingAreaGranularity == nullptr) {
        table->GetRenderingAreaGranularity = (PFN_vkGetRenderingAreaGranularity)StubGetRenderingAreaGranularity;
    }
    table->GetDeviceImageSubresourceLayout =
        (PFN_vkGetDeviceImageSubresourceLayout)gpa(device, "vkGetDeviceImageSubresourceLayout");
    if (table->GetDeviceImageSubresourceLayout == nullptr) {
        table->GetDeviceImageSubresourceLayout = (PFN_vkGetDeviceImageSubresourceLayout)StubGetDeviceImageSubresourceLayout;
    }
    table->GetImageSubresourceLayout2 = (PFN_vkGetImageSubresourceLayout2)gpa(device, "vkGetImageSubresourceLayout2");
    if (table->GetImageSubresourceLayout2 == nullptr) {
        table->GetImageSubresourceLayout2 = (PFN_vkGetImageSubresourceLayout2)StubGetImageSubresourceLayout2;
    }
    table->CmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)gpa(device, "vkCmdPushDescriptorSet");
    if (table->CmdPushDescriptorSet == nullptr) {
        table->CmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)StubCmdPushDescriptorSet;
    }
    table->CmdPushDescriptorSetWithTemplate =
        (PFN_vkCmdPushDescriptorSetWithTemplate)gpa(device, "vkCmdPushDescriptorSetWithTemplate");
    if (table->CmdPushDescriptorSetWithTemplate == nullptr) {
        table->CmdPushDescriptorSetWithTemplate = (PFN_vkCmdPushDescriptorSetWithTemplate)StubCmdPushDescriptorSetWithTemplate;
    }
    table->CmdSetRenderingAttachmentLocations =
        (PFN_vkCmdSetRenderingAttachmentLocations)gpa(device, "vkCmdSetRenderingAttachmentLocations");
    if (table->CmdSetRenderingAttachmentLocations == nullptr) {
        table->CmdSetRenderingAttachmentLocations =
            (PFN_vkCmdSetRenderingAttachmentLocations)StubCmdSetRenderingAttachmentLocations;
    }
    table->CmdSetRenderingInputAttachmentIndices =
        (PFN_vkCmdSetRenderingInputAttachmentIndices)gpa(device, "vkCmdSetRenderingInputAttachmentIndices");
    if (table->CmdSetRenderingInputAttachmentIndices == nullptr) {
        table->CmdSetRenderingInputAttachmentIndices =
            (PFN_vkCmdSetRenderingInputAttachmentIndices)StubCmdSetRenderingInputAttachmentIndices;
    }
    table->CmdBindDescriptorSets2 = (PFN_vkCmdBindDescriptorSets2)gpa(device, "vkCmdBindDescriptorSets2");
    if (table->CmdBindDescriptorSets2 == nullptr) {
        table->CmdBindDescriptorSets2 = (PFN_vkCmdBindDescriptorSets2)StubCmdBindDescriptorSets2;
    }
    table->CmdPushConstants2 = (PFN_vkCmdPushConstants2)gpa(device, "vkCmdPushConstants2");
    if (table->CmdPushConstants2 == nullptr) {
        table->CmdPushConstants2 = (PFN_vkCmdPushConstants2)StubCmdPushConstants2;
    }
    table->CmdPushDescriptorSet2 = (PFN_vkCmdPushDescriptorSet2)gpa(device, "vkCmdPushDescriptorSet2");
    if (table->CmdPushDescriptorSet2 == nullptr) {
        table->CmdPushDescriptorSet2 = (PFN_vkCmdPushDescriptorSet2)StubCmdPushDescriptorSet2;
    }
    table->CmdPushDescriptorSetWithTemplate2 =
        (PFN_vkCmdPushDescriptorSetWithTemplate2)gpa(device, "vkCmdPushDescriptorSetWithTemplate2");
    if (table->CmdPushDescriptorSetWithTemplate2 == nullptr) {
        table->CmdPushDescriptorSetWithTemplate2 = (PFN_vkCmdPushDescriptorSetWithTemplate2)StubCmdPushDescriptorSetWithTemplate2;
    }
    table->CopyMemoryToImage = (PFN_vkCopyMemoryToImage)gpa(device, "vkCopyMemoryToImage");
    if (table->CopyMemoryToImage == nullptr) {
        table->CopyMemoryToImage = (PFN_vkCopyMemoryToImage)StubCopyMemoryToImage;
    }
    table->CopyImageToMemory = (PFN_vkCopyImageToMemory)gpa(device, "vkCopyImageToMemory");
    if (table->CopyImageToMemory == nullptr) {
        table->CopyImageToMemory = (PFN_vkCopyImageToMemory)StubCopyImageToMemory;
    }
    table->CopyImageToImage = (PFN_vkCopyImageToImage)gpa(device, "vkCopyImageToImage");
    if (table->CopyImageToImage == nullptr) {
        table->CopyImageToImage = (PFN_vkCopyImageToImage)StubCopyImageToImage;
    }
    table->TransitionImageLayout = (PFN_vkTransitionImageLayout)gpa(device, "vkTransitionImageLayout");
    if (table->TransitionImageLayout == nullptr) {
        table->TransitionImageLayout = (PFN_vkTransitionImageLayout)StubTransitionImageLayout;
    }
    table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)gpa(device, "vkCreateSwapchainKHR");
    if (table->CreateSwapchainKHR == nullptr) {
        table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)StubCreateSwapchainKHR;
    }
    table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)gpa(device, "vkDestroySwapchainKHR");
    if (table->DestroySwapchainKHR == nullptr) {
        table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)StubDestroySwapchainKHR;
    }
    table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)gpa(device, "vkGetSwapchainImagesKHR");
    if (table->GetSwapchainImagesKHR == nullptr) {
        table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)StubGetSwapchainImagesKHR;
    }
    table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)gpa(device, "vkAcquireNextImageKHR");
    if (table->AcquireNextImageKHR == nullptr) {
        table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)StubAcquireNextImageKHR;
    }
    table->QueuePresentKHR = (PFN_vkQueuePresentKHR)gpa(device, "vkQueuePresentKHR");
    if (table->QueuePresentKHR == nullptr) {
        table->QueuePresentKHR = (PFN_vkQueuePresentKHR)StubQueuePresentKHR;
    }
    table->GetDeviceGroupPresentCapabilitiesKHR =
        (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)gpa(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    if (table->GetDeviceGroupPresentCapabilitiesKHR == nullptr) {
        table->GetDeviceGroupPresentCapabilitiesKHR =
            (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)StubGetDeviceGroupPresentCapabilitiesKHR;
    }
    table->GetDeviceGroupSurfacePresentModesKHR =
        (PFN_vkGetDeviceGroupSurfacePresentModesKHR)gpa(device, "vkGetDeviceGroupSurfacePresentModesKHR");
    if (table->GetDeviceGroupSurfacePresentModesKHR == nullptr) {
        table->GetDeviceGroupSurfacePresentModesKHR =
            (PFN_vkGetDeviceGroupSurfacePresentModesKHR)StubGetDeviceGroupSurfacePresentModesKHR;
    }
    table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)gpa(device, "vkAcquireNextImage2KHR");
    if (table->AcquireNextImage2KHR == nullptr) {
        table->AcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)StubAcquireNextImage2KHR;
    }
    table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)gpa(device, "vkCreateSharedSwapchainsKHR");
    if (table->CreateSharedSwapchainsKHR == nullptr) {
        table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)StubCreateSharedSwapchainsKHR;
    }
    table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)gpa(device, "vkCreateVideoSessionKHR");
    if (table->CreateVideoSessionKHR == nullptr) {
        table->CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)StubCreateVideoSessionKHR;
    }
    table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)gpa(device, "vkDestroyVideoSessionKHR");
    if (table->DestroyVideoSessionKHR == nullptr) {
        table->DestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)StubDestroyVideoSessionKHR;
    }
    table->GetVideoSessionMemoryRequirementsKHR =
        (PFN_vkGetVideoSessionMemoryRequirementsKHR)gpa(device, "vkGetVideoSessionMemoryRequirementsKHR");
    if (table->GetVideoSessionMemoryRequirementsKHR == nullptr) {
        table->GetVideoSessionMemoryRequirementsKHR =
            (PFN_vkGetVideoSessionMemoryRequirementsKHR)StubGetVideoSessionMemoryRequirementsKHR;
    }
    table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)gpa(device, "vkBindVideoSessionMemoryKHR");
    if (table->BindVideoSessionMemoryKHR == nullptr) {
        table->BindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)StubBindVideoSessionMemoryKHR;
    }
    table->CreateVideoSessionParametersKHR =
        (PFN_vkCreateVideoSessionParametersKHR)gpa(device, "vkCreateVideoSessionParametersKHR");
    if (table->CreateVideoSessionParametersKHR == nullptr) {
        table->CreateVideoSessionParametersKHR = (PFN_vkCreateVideoSessionParametersKHR)StubCreateVideoSessionParametersKHR;
    }
    table->UpdateVideoSessionParametersKHR =
        (PFN_vkUpdateVideoSessionParametersKHR)gpa(device, "vkUpdateVideoSessionParametersKHR");
    if (table->UpdateVideoSessionParametersKHR == nullptr) {
        table->UpdateVideoSessionParametersKHR = (PFN_vkUpdateVideoSessionParametersKHR)StubUpdateVideoSessionParametersKHR;
    }
    table->DestroyVideoSessionParametersKHR =
        (PFN_vkDestroyVideoSessionParametersKHR)gpa(device, "vkDestroyVideoSessionParametersKHR");
    if (table->DestroyVideoSessionParametersKHR == nullptr) {
        table->DestroyVideoSessionParametersKHR = (PFN_vkDestroyVideoSessionParametersKHR)StubDestroyVideoSessionParametersKHR;
    }
    table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)gpa(device, "vkCmdBeginVideoCodingKHR");
    if (table->CmdBeginVideoCodingKHR == nullptr) {
        table->CmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)StubCmdBeginVideoCodingKHR;
    }
    table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)gpa(device, "vkCmdEndVideoCodingKHR");
    if (table->CmdEndVideoCodingKHR == nullptr) {
        table->CmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)StubCmdEndVideoCodingKHR;
    }
    table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)gpa(device, "vkCmdControlVideoCodingKHR");
    if (table->CmdControlVideoCodingKHR == nullptr) {
        table->CmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)StubCmdControlVideoCodingKHR;
    }
    table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)gpa(device, "vkCmdDecodeVideoKHR");
    if (table->CmdDecodeVideoKHR == nullptr) {
        table->CmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)StubCmdDecodeVideoKHR;
    }
    table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)gpa(device, "vkCmdBeginRenderingKHR");
    if (table->CmdBeginRenderingKHR == nullptr) {
        table->CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)StubCmdBeginRenderingKHR;
    }
    table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)gpa(device, "vkCmdEndRenderingKHR");
    if (table->CmdEndRenderingKHR == nullptr) {
        table->CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)StubCmdEndRenderingKHR;
    }
    table->GetDeviceGroupPeerMemoryFeaturesKHR =
        (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)gpa(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    if (table->GetDeviceGroupPeerMemoryFeaturesKHR == nullptr) {
        table->GetDeviceGroupPeerMemoryFeaturesKHR =
            (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)StubGetDeviceGroupPeerMemoryFeaturesKHR;
    }
    table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)gpa(device, "vkCmdSetDeviceMaskKHR");
    if (table->CmdSetDeviceMaskKHR == nullptr) {
        table->CmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)StubCmdSetDeviceMaskKHR;
    }
    table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)gpa(device, "vkCmdDispatchBaseKHR");
    if (table->CmdDispatchBaseKHR == nullptr) {
        table->CmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)StubCmdDispatchBaseKHR;
    }
    table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)gpa(device, "vkTrimCommandPoolKHR");
    if (table->TrimCommandPoolKHR == nullptr) {
        table->TrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)StubTrimCommandPoolKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)gpa(device, "vkGetMemoryWin32HandleKHR");
    if (table->GetMemoryWin32HandleKHR == nullptr) {
        table->GetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)StubGetMemoryWin32HandleKHR;
    }
    table->GetMemoryWin32HandlePropertiesKHR =
        (PFN_vkGetMemoryWin32HandlePropertiesKHR)gpa(device, "vkGetMemoryWin32HandlePropertiesKHR");
    if (table->GetMemoryWin32HandlePropertiesKHR == nullptr) {
        table->GetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)StubGetMemoryWin32HandlePropertiesKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)gpa(device, "vkGetMemoryFdKHR");
    if (table->GetMemoryFdKHR == nullptr) {
        table->GetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)StubGetMemoryFdKHR;
    }
    table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)gpa(device, "vkGetMemoryFdPropertiesKHR");
    if (table->GetMemoryFdPropertiesKHR == nullptr) {
        table->GetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)StubGetMemoryFdPropertiesKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)gpa(device, "vkImportSemaphoreWin32HandleKHR");
    if (table->ImportSemaphoreWin32HandleKHR == nullptr) {
        table->ImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)StubImportSemaphoreWin32HandleKHR;
    }
    table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)gpa(device, "vkGetSemaphoreWin32HandleKHR");
    if (table->GetSemaphoreWin32HandleKHR == nullptr) {
        table->GetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)StubGetSemaphoreWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)gpa(device, "vkImportSemaphoreFdKHR");
    if (table->ImportSemaphoreFdKHR == nullptr) {
        table->ImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)StubImportSemaphoreFdKHR;
    }
    table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)gpa(device, "vkGetSemaphoreFdKHR");
    if (table->GetSemaphoreFdKHR == nullptr) {
        table->GetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)StubGetSemaphoreFdKHR;
    }
    table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)gpa(device, "vkCmdPushDescriptorSetKHR");
    if (table->CmdPushDescriptorSetKHR == nullptr) {
        table->CmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)StubCmdPushDescriptorSetKHR;
    }
    table->CmdPushDescriptorSetWithTemplateKHR =
        (PFN_vkCmdPushDescriptorSetWithTemplateKHR)gpa(device, "vkCmdPushDescriptorSetWithTemplateKHR");
    if (table->CmdPushDescriptorSetWithTemplateKHR == nullptr) {
        table->CmdPushDescriptorSetWithTemplateKHR =
            (PFN_vkCmdPushDescriptorSetWithTemplateKHR)StubCmdPushDescriptorSetWithTemplateKHR;
    }
    table->CreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)gpa(device, "vkCreateDescriptorUpdateTemplateKHR");
    if (table->CreateDescriptorUpdateTemplateKHR == nullptr) {
        table->CreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)StubCreateDescriptorUpdateTemplateKHR;
    }
    table->DestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)gpa(device, "vkDestroyDescriptorUpdateTemplateKHR");
    if (table->DestroyDescriptorUpdateTemplateKHR == nullptr) {
        table->DestroyDescriptorUpdateTemplateKHR =
            (PFN_vkDestroyDescriptorUpdateTemplateKHR)StubDestroyDescriptorUpdateTemplateKHR;
    }
    table->UpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)gpa(device, "vkUpdateDescriptorSetWithTemplateKHR");
    if (table->UpdateDescriptorSetWithTemplateKHR == nullptr) {
        table->UpdateDescriptorSetWithTemplateKHR =
            (PFN_vkUpdateDescriptorSetWithTemplateKHR)StubUpdateDescriptorSetWithTemplateKHR;
    }
    table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)gpa(device, "vkCreateRenderPass2KHR");
    if (table->CreateRenderPass2KHR == nullptr) {
        table->CreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)StubCreateRenderPass2KHR;
    }
    table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)gpa(device, "vkCmdBeginRenderPass2KHR");
    if (table->CmdBeginRenderPass2KHR == nullptr) {
        table->CmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)StubCmdBeginRenderPass2KHR;
    }
    table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)gpa(device, "vkCmdNextSubpass2KHR");
    if (table->CmdNextSubpass2KHR == nullptr) {
        table->CmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)StubCmdNextSubpass2KHR;
    }
    table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)gpa(device, "vkCmdEndRenderPass2KHR");
    if (table->CmdEndRenderPass2KHR == nullptr) {
        table->CmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)StubCmdEndRenderPass2KHR;
    }
    table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)gpa(device, "vkGetSwapchainStatusKHR");
    if (table->GetSwapchainStatusKHR == nullptr) {
        table->GetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)StubGetSwapchainStatusKHR;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)gpa(device, "vkImportFenceWin32HandleKHR");
    if (table->ImportFenceWin32HandleKHR == nullptr) {
        table->ImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)StubImportFenceWin32HandleKHR;
    }
    table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)gpa(device, "vkGetFenceWin32HandleKHR");
    if (table->GetFenceWin32HandleKHR == nullptr) {
        table->GetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)StubGetFenceWin32HandleKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)gpa(device, "vkImportFenceFdKHR");
    if (table->ImportFenceFdKHR == nullptr) {
        table->ImportFenceFdKHR = (PFN_vkImportFenceFdKHR)StubImportFenceFdKHR;
    }
    table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)gpa(device, "vkGetFenceFdKHR");
    if (table->GetFenceFdKHR == nullptr) {
        table->GetFenceFdKHR = (PFN_vkGetFenceFdKHR)StubGetFenceFdKHR;
    }
    table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)gpa(device, "vkAcquireProfilingLockKHR");
    if (table->AcquireProfilingLockKHR == nullptr) {
        table->AcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)StubAcquireProfilingLockKHR;
    }
    table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)gpa(device, "vkReleaseProfilingLockKHR");
    if (table->ReleaseProfilingLockKHR == nullptr) {
        table->ReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)StubReleaseProfilingLockKHR;
    }
    table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)gpa(device, "vkGetImageMemoryRequirements2KHR");
    if (table->GetImageMemoryRequirements2KHR == nullptr) {
        table->GetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)StubGetImageMemoryRequirements2KHR;
    }
    table->GetBufferMemoryRequirements2KHR =
        (PFN_vkGetBufferMemoryRequirements2KHR)gpa(device, "vkGetBufferMemoryRequirements2KHR");
    if (table->GetBufferMemoryRequirements2KHR == nullptr) {
        table->GetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)StubGetBufferMemoryRequirements2KHR;
    }
    table->GetImageSparseMemoryRequirements2KHR =
        (PFN_vkGetImageSparseMemoryRequirements2KHR)gpa(device, "vkGetImageSparseMemoryRequirements2KHR");
    if (table->GetImageSparseMemoryRequirements2KHR == nullptr) {
        table->GetImageSparseMemoryRequirements2KHR =
            (PFN_vkGetImageSparseMemoryRequirements2KHR)StubGetImageSparseMemoryRequirements2KHR;
    }
    table->CreateSamplerYcbcrConversionKHR =
        (PFN_vkCreateSamplerYcbcrConversionKHR)gpa(device, "vkCreateSamplerYcbcrConversionKHR");
    if (table->CreateSamplerYcbcrConversionKHR == nullptr) {
        table->CreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)StubCreateSamplerYcbcrConversionKHR;
    }
    table->DestroySamplerYcbcrConversionKHR =
        (PFN_vkDestroySamplerYcbcrConversionKHR)gpa(device, "vkDestroySamplerYcbcrConversionKHR");
    if (table->DestroySamplerYcbcrConversionKHR == nullptr) {
        table->DestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)StubDestroySamplerYcbcrConversionKHR;
    }
    table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)gpa(device, "vkBindBufferMemory2KHR");
    if (table->BindBufferMemory2KHR == nullptr) {
        table->BindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)StubBindBufferMemory2KHR;
    }
    table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)gpa(device, "vkBindImageMemory2KHR");
    if (table->BindImageMemory2KHR == nullptr) {
        table->BindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)StubBindImageMemory2KHR;
    }
    table->GetDescriptorSetLayoutSupportKHR =
        (PFN_vkGetDescriptorSetLayoutSupportKHR)gpa(device, "vkGetDescriptorSetLayoutSupportKHR");
    if (table->GetDescriptorSetLayoutSupportKHR == nullptr) {
        table->GetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR)StubGetDescriptorSetLayoutSupportKHR;
    }
    table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)gpa(device, "vkCmdDrawIndirectCountKHR");
    if (table->CmdDrawIndirectCountKHR == nullptr) {
        table->CmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)StubCmdDrawIndirectCountKHR;
    }
    table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)gpa(device, "vkCmdDrawIndexedIndirectCountKHR");
    if (table->CmdDrawIndexedIndirectCountKHR == nullptr) {
        table->CmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)StubCmdDrawIndexedIndirectCountKHR;
    }
    table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)gpa(device, "vkGetSemaphoreCounterValueKHR");
    if (table->GetSemaphoreCounterValueKHR == nullptr) {
        table->GetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)StubGetSemaphoreCounterValueKHR;
    }
    table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)gpa(device, "vkWaitSemaphoresKHR");
    if (table->WaitSemaphoresKHR == nullptr) {
        table->WaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)StubWaitSemaphoresKHR;
    }
    table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)gpa(device, "vkSignalSemaphoreKHR");
    if (table->SignalSemaphoreKHR == nullptr) {
        table->SignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)StubSignalSemaphoreKHR;
    }
    table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)gpa(device, "vkCmdSetFragmentShadingRateKHR");
    if (table->CmdSetFragmentShadingRateKHR == nullptr) {
        table->CmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)StubCmdSetFragmentShadingRateKHR;
    }
    table->CmdSetRenderingAttachmentLocationsKHR =
        (PFN_vkCmdSetRenderingAttachmentLocationsKHR)gpa(device, "vkCmdSetRenderingAttachmentLocationsKHR");
    if (table->CmdSetRenderingAttachmentLocationsKHR == nullptr) {
        table->CmdSetRenderingAttachmentLocationsKHR =
            (PFN_vkCmdSetRenderingAttachmentLocationsKHR)StubCmdSetRenderingAttachmentLocationsKHR;
    }
    table->CmdSetRenderingInputAttachmentIndicesKHR =
        (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)gpa(device, "vkCmdSetRenderingInputAttachmentIndicesKHR");
    if (table->CmdSetRenderingInputAttachmentIndicesKHR == nullptr) {
        table->CmdSetRenderingInputAttachmentIndicesKHR =
            (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)StubCmdSetRenderingInputAttachmentIndicesKHR;
    }
    table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)gpa(device, "vkWaitForPresentKHR");
    if (table->WaitForPresentKHR == nullptr) {
        table->WaitForPresentKHR = (PFN_vkWaitForPresentKHR)StubWaitForPresentKHR;
    }
    table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)gpa(device, "vkGetBufferDeviceAddressKHR");
    if (table->GetBufferDeviceAddressKHR == nullptr) {
        table->GetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)StubGetBufferDeviceAddressKHR;
    }
    table->GetBufferOpaqueCaptureAddressKHR =
        (PFN_vkGetBufferOpaqueCaptureAddressKHR)gpa(device, "vkGetBufferOpaqueCaptureAddressKHR");
    if (table->GetBufferOpaqueCaptureAddressKHR == nullptr) {
        table->GetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR)StubGetBufferOpaqueCaptureAddressKHR;
    }
    table->GetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)gpa(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
    if (table->GetDeviceMemoryOpaqueCaptureAddressKHR == nullptr) {
        table->GetDeviceMemoryOpaqueCaptureAddressKHR =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)StubGetDeviceMemoryOpaqueCaptureAddressKHR;
    }
    table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)gpa(device, "vkCreateDeferredOperationKHR");
    if (table->CreateDeferredOperationKHR == nullptr) {
        table->CreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)StubCreateDeferredOperationKHR;
    }
    table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)gpa(device, "vkDestroyDeferredOperationKHR");
    if (table->DestroyDeferredOperationKHR == nullptr) {
        table->DestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)StubDestroyDeferredOperationKHR;
    }
    table->GetDeferredOperationMaxConcurrencyKHR =
        (PFN_vkGetDeferredOperationMaxConcurrencyKHR)gpa(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    if (table->GetDeferredOperationMaxConcurrencyKHR == nullptr) {
        table->GetDeferredOperationMaxConcurrencyKHR =
            (PFN_vkGetDeferredOperationMaxConcurrencyKHR)StubGetDeferredOperationMaxConcurrencyKHR;
    }
    table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)gpa(device, "vkGetDeferredOperationResultKHR");
    if (table->GetDeferredOperationResultKHR == nullptr) {
        table->GetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)StubGetDeferredOperationResultKHR;
    }
    table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)gpa(device, "vkDeferredOperationJoinKHR");
    if (table->DeferredOperationJoinKHR == nullptr) {
        table->DeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)StubDeferredOperationJoinKHR;
    }
    table->GetPipelineExecutablePropertiesKHR =
        (PFN_vkGetPipelineExecutablePropertiesKHR)gpa(device, "vkGetPipelineExecutablePropertiesKHR");
    if (table->GetPipelineExecutablePropertiesKHR == nullptr) {
        table->GetPipelineExecutablePropertiesKHR =
            (PFN_vkGetPipelineExecutablePropertiesKHR)StubGetPipelineExecutablePropertiesKHR;
    }
    table->GetPipelineExecutableStatisticsKHR =
        (PFN_vkGetPipelineExecutableStatisticsKHR)gpa(device, "vkGetPipelineExecutableStatisticsKHR");
    if (table->GetPipelineExecutableStatisticsKHR == nullptr) {
        table->GetPipelineExecutableStatisticsKHR =
            (PFN_vkGetPipelineExecutableStatisticsKHR)StubGetPipelineExecutableStatisticsKHR;
    }
    table->GetPipelineExecutableInternalRepresentationsKHR =
        (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)gpa(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
    if (table->GetPipelineExecutableInternalRepresentationsKHR == nullptr) {
        table->GetPipelineExecutableInternalRepresentationsKHR =
            (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)StubGetPipelineExecutableInternalRepresentationsKHR;
    }
    table->MapMemory2KHR = (PFN_vkMapMemory2KHR)gpa(device, "vkMapMemory2KHR");
    if (table->MapMemory2KHR == nullptr) {
        table->MapMemory2KHR = (PFN_vkMapMemory2KHR)StubMapMemory2KHR;
    }
    table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)gpa(device, "vkUnmapMemory2KHR");
    if (table->UnmapMemory2KHR == nullptr) {
        table->UnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)StubUnmapMemory2KHR;
    }
    table->GetEncodedVideoSessionParametersKHR =
        (PFN_vkGetEncodedVideoSessionParametersKHR)gpa(device, "vkGetEncodedVideoSessionParametersKHR");
    if (table->GetEncodedVideoSessionParametersKHR == nullptr) {
        table->GetEncodedVideoSessionParametersKHR =
            (PFN_vkGetEncodedVideoSessionParametersKHR)StubGetEncodedVideoSessionParametersKHR;
    }
    table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)gpa(device, "vkCmdEncodeVideoKHR");
    if (table->CmdEncodeVideoKHR == nullptr) {
        table->CmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)StubCmdEncodeVideoKHR;
    }
    table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)gpa(device, "vkCmdSetEvent2KHR");
    if (table->CmdSetEvent2KHR == nullptr) {
        table->CmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)StubCmdSetEvent2KHR;
    }
    table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)gpa(device, "vkCmdResetEvent2KHR");
    if (table->CmdResetEvent2KHR == nullptr) {
        table->CmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)StubCmdResetEvent2KHR;
    }
    table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)gpa(device, "vkCmdWaitEvents2KHR");
    if (table->CmdWaitEvents2KHR == nullptr) {
        table->CmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)StubCmdWaitEvents2KHR;
    }
    table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)gpa(device, "vkCmdPipelineBarrier2KHR");
    if (table->CmdPipelineBarrier2KHR == nullptr) {
        table->CmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)StubCmdPipelineBarrier2KHR;
    }
    table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)gpa(device, "vkCmdWriteTimestamp2KHR");
    if (table->CmdWriteTimestamp2KHR == nullptr) {
        table->CmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)StubCmdWriteTimestamp2KHR;
    }
    table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)gpa(device, "vkQueueSubmit2KHR");
    if (table->QueueSubmit2KHR == nullptr) {
        table->QueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)StubQueueSubmit2KHR;
    }
    table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)gpa(device, "vkCmdCopyBuffer2KHR");
    if (table->CmdCopyBuffer2KHR == nullptr) {
        table->CmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)StubCmdCopyBuffer2KHR;
    }
    table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)gpa(device, "vkCmdCopyImage2KHR");
    if (table->CmdCopyImage2KHR == nullptr) {
        table->CmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)StubCmdCopyImage2KHR;
    }
    table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)gpa(device, "vkCmdCopyBufferToImage2KHR");
    if (table->CmdCopyBufferToImage2KHR == nullptr) {
        table->CmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)StubCmdCopyBufferToImage2KHR;
    }
    table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)gpa(device, "vkCmdCopyImageToBuffer2KHR");
    if (table->CmdCopyImageToBuffer2KHR == nullptr) {
        table->CmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)StubCmdCopyImageToBuffer2KHR;
    }
    table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)gpa(device, "vkCmdBlitImage2KHR");
    if (table->CmdBlitImage2KHR == nullptr) {
        table->CmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)StubCmdBlitImage2KHR;
    }
    table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)gpa(device, "vkCmdResolveImage2KHR");
    if (table->CmdResolveImage2KHR == nullptr) {
        table->CmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)StubCmdResolveImage2KHR;
    }
    table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)gpa(device, "vkCmdTraceRaysIndirect2KHR");
    if (table->CmdTraceRaysIndirect2KHR == nullptr) {
        table->CmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)StubCmdTraceRaysIndirect2KHR;
    }
    table->GetDeviceBufferMemoryRequirementsKHR =
        (PFN_vkGetDeviceBufferMemoryRequirementsKHR)gpa(device, "vkGetDeviceBufferMemoryRequirementsKHR");
    if (table->GetDeviceBufferMemoryRequirementsKHR == nullptr) {
        table->GetDeviceBufferMemoryRequirementsKHR =
            (PFN_vkGetDeviceBufferMemoryRequirementsKHR)StubGetDeviceBufferMemoryRequirementsKHR;
    }
    table->GetDeviceImageMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageMemoryRequirementsKHR");
    if (table->GetDeviceImageMemoryRequirementsKHR == nullptr) {
        table->GetDeviceImageMemoryRequirementsKHR =
            (PFN_vkGetDeviceImageMemoryRequirementsKHR)StubGetDeviceImageMemoryRequirementsKHR;
    }
    table->GetDeviceImageSparseMemoryRequirementsKHR =
        (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)gpa(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
    if (table->GetDeviceImageSparseMemoryRequirementsKHR == nullptr) {
        table->GetDeviceImageSparseMemoryRequirementsKHR =
            (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)StubGetDeviceImageSparseMemoryRequirementsKHR;
    }
    table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)gpa(device, "vkCmdBindIndexBuffer2KHR");
    if (table->CmdBindIndexBuffer2KHR == nullptr) {
        table->CmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)StubCmdBindIndexBuffer2KHR;
    }
    table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)gpa(device, "vkGetRenderingAreaGranularityKHR");
    if (table->GetRenderingAreaGranularityKHR == nullptr) {
        table->GetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)StubGetRenderingAreaGranularityKHR;
    }
    table->GetDeviceImageSubresourceLayoutKHR =
        (PFN_vkGetDeviceImageSubresourceLayoutKHR)gpa(device, "vkGetDeviceImageSubresourceLayoutKHR");
    if (table->GetDeviceImageSubresourceLayoutKHR == nullptr) {
        table->GetDeviceImageSubresourceLayoutKHR =
            (PFN_vkGetDeviceImageSubresourceLayoutKHR)StubGetDeviceImageSubresourceLayoutKHR;
    }
    table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)gpa(device, "vkGetImageSubresourceLayout2KHR");
    if (table->GetImageSubresourceLayout2KHR == nullptr) {
        table->GetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)StubGetImageSubresourceLayout2KHR;
    }
    table->CreatePipelineBinariesKHR = (PFN_vkCreatePipelineBinariesKHR)gpa(device, "vkCreatePipelineBinariesKHR");
    if (table->CreatePipelineBinariesKHR == nullptr) {
        table->CreatePipelineBinariesKHR = (PFN_vkCreatePipelineBinariesKHR)StubCreatePipelineBinariesKHR;
    }
    table->DestroyPipelineBinaryKHR = (PFN_vkDestroyPipelineBinaryKHR)gpa(device, "vkDestroyPipelineBinaryKHR");
    if (table->DestroyPipelineBinaryKHR == nullptr) {
        table->DestroyPipelineBinaryKHR = (PFN_vkDestroyPipelineBinaryKHR)StubDestroyPipelineBinaryKHR;
    }
    table->GetPipelineKeyKHR = (PFN_vkGetPipelineKeyKHR)gpa(device, "vkGetPipelineKeyKHR");
    if (table->GetPipelineKeyKHR == nullptr) {
        table->GetPipelineKeyKHR = (PFN_vkGetPipelineKeyKHR)StubGetPipelineKeyKHR;
    }
    table->GetPipelineBinaryDataKHR = (PFN_vkGetPipelineBinaryDataKHR)gpa(device, "vkGetPipelineBinaryDataKHR");
    if (table->GetPipelineBinaryDataKHR == nullptr) {
        table->GetPipelineBinaryDataKHR = (PFN_vkGetPipelineBinaryDataKHR)StubGetPipelineBinaryDataKHR;
    }
    table->ReleaseCapturedPipelineDataKHR = (PFN_vkReleaseCapturedPipelineDataKHR)gpa(device, "vkReleaseCapturedPipelineDataKHR");
    if (table->ReleaseCapturedPipelineDataKHR == nullptr) {
        table->ReleaseCapturedPipelineDataKHR = (PFN_vkReleaseCapturedPipelineDataKHR)StubReleaseCapturedPipelineDataKHR;
    }
    table->CmdSetLineStippleKHR = (PFN_vkCmdSetLineStippleKHR)gpa(device, "vkCmdSetLineStippleKHR");
    if (table->CmdSetLineStippleKHR == nullptr) {
        table->CmdSetLineStippleKHR = (PFN_vkCmdSetLineStippleKHR)StubCmdSetLineStippleKHR;
    }
    table->GetCalibratedTimestampsKHR = (PFN_vkGetCalibratedTimestampsKHR)gpa(device, "vkGetCalibratedTimestampsKHR");
    if (table->GetCalibratedTimestampsKHR == nullptr) {
        table->GetCalibratedTimestampsKHR = (PFN_vkGetCalibratedTimestampsKHR)StubGetCalibratedTimestampsKHR;
    }
    table->CmdBindDescriptorSets2KHR = (PFN_vkCmdBindDescriptorSets2KHR)gpa(device, "vkCmdBindDescriptorSets2KHR");
    if (table->CmdBindDescriptorSets2KHR == nullptr) {
        table->CmdBindDescriptorSets2KHR = (PFN_vkCmdBindDescriptorSets2KHR)StubCmdBindDescriptorSets2KHR;
    }
    table->CmdPushConstants2KHR = (PFN_vkCmdPushConstants2KHR)gpa(device, "vkCmdPushConstants2KHR");
    if (table->CmdPushConstants2KHR == nullptr) {
        table->CmdPushConstants2KHR = (PFN_vkCmdPushConstants2KHR)StubCmdPushConstants2KHR;
    }
    table->CmdPushDescriptorSet2KHR = (PFN_vkCmdPushDescriptorSet2KHR)gpa(device, "vkCmdPushDescriptorSet2KHR");
    if (table->CmdPushDescriptorSet2KHR == nullptr) {
        table->CmdPushDescriptorSet2KHR = (PFN_vkCmdPushDescriptorSet2KHR)StubCmdPushDescriptorSet2KHR;
    }
    table->CmdPushDescriptorSetWithTemplate2KHR =
        (PFN_vkCmdPushDescriptorSetWithTemplate2KHR)gpa(device, "vkCmdPushDescriptorSetWithTemplate2KHR");
    if (table->CmdPushDescriptorSetWithTemplate2KHR == nullptr) {
        table->CmdPushDescriptorSetWithTemplate2KHR =
            (PFN_vkCmdPushDescriptorSetWithTemplate2KHR)StubCmdPushDescriptorSetWithTemplate2KHR;
    }
    table->CmdSetDescriptorBufferOffsets2EXT =
        (PFN_vkCmdSetDescriptorBufferOffsets2EXT)gpa(device, "vkCmdSetDescriptorBufferOffsets2EXT");
    if (table->CmdSetDescriptorBufferOffsets2EXT == nullptr) {
        table->CmdSetDescriptorBufferOffsets2EXT = (PFN_vkCmdSetDescriptorBufferOffsets2EXT)StubCmdSetDescriptorBufferOffsets2EXT;
    }
    table->CmdBindDescriptorBufferEmbeddedSamplers2EXT =
        (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)gpa(device, "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");
    if (table->CmdBindDescriptorBufferEmbeddedSamplers2EXT == nullptr) {
        table->CmdBindDescriptorBufferEmbeddedSamplers2EXT =
            (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)StubCmdBindDescriptorBufferEmbeddedSamplers2EXT;
    }
    table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)gpa(device, "vkDebugMarkerSetObjectTagEXT");
    if (table->DebugMarkerSetObjectTagEXT == nullptr) {
        table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)StubDebugMarkerSetObjectTagEXT;
    }
    table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)gpa(device, "vkDebugMarkerSetObjectNameEXT");
    if (table->DebugMarkerSetObjectNameEXT == nullptr) {
        table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)StubDebugMarkerSetObjectNameEXT;
    }
    table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)gpa(device, "vkCmdDebugMarkerBeginEXT");
    if (table->CmdDebugMarkerBeginEXT == nullptr) {
        table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)StubCmdDebugMarkerBeginEXT;
    }
    table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)gpa(device, "vkCmdDebugMarkerEndEXT");
    if (table->CmdDebugMarkerEndEXT == nullptr) {
        table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)StubCmdDebugMarkerEndEXT;
    }
    table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)gpa(device, "vkCmdDebugMarkerInsertEXT");
    if (table->CmdDebugMarkerInsertEXT == nullptr) {
        table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)StubCmdDebugMarkerInsertEXT;
    }
    table->CmdBindTransformFeedbackBuffersEXT =
        (PFN_vkCmdBindTransformFeedbackBuffersEXT)gpa(device, "vkCmdBindTransformFeedbackBuffersEXT");
    if (table->CmdBindTransformFeedbackBuffersEXT == nullptr) {
        table->CmdBindTransformFeedbackBuffersEXT =
            (PFN_vkCmdBindTransformFeedbackBuffersEXT)StubCmdBindTransformFeedbackBuffersEXT;
    }
    table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)gpa(device, "vkCmdBeginTransformFeedbackEXT");
    if (table->CmdBeginTransformFeedbackEXT == nullptr) {
        table->CmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)StubCmdBeginTransformFeedbackEXT;
    }
    table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)gpa(device, "vkCmdEndTransformFeedbackEXT");
    if (table->CmdEndTransformFeedbackEXT == nullptr) {
        table->CmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)StubCmdEndTransformFeedbackEXT;
    }
    table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)gpa(device, "vkCmdBeginQueryIndexedEXT");
    if (table->CmdBeginQueryIndexedEXT == nullptr) {
        table->CmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)StubCmdBeginQueryIndexedEXT;
    }
    table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)gpa(device, "vkCmdEndQueryIndexedEXT");
    if (table->CmdEndQueryIndexedEXT == nullptr) {
        table->CmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)StubCmdEndQueryIndexedEXT;
    }
    table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)gpa(device, "vkCmdDrawIndirectByteCountEXT");
    if (table->CmdDrawIndirectByteCountEXT == nullptr) {
        table->CmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)StubCmdDrawIndirectByteCountEXT;
    }
    table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)gpa(device, "vkCreateCuModuleNVX");
    if (table->CreateCuModuleNVX == nullptr) {
        table->CreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)StubCreateCuModuleNVX;
    }
    table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)gpa(device, "vkCreateCuFunctionNVX");
    if (table->CreateCuFunctionNVX == nullptr) {
        table->CreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)StubCreateCuFunctionNVX;
    }
    table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)gpa(device, "vkDestroyCuModuleNVX");
    if (table->DestroyCuModuleNVX == nullptr) {
        table->DestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)StubDestroyCuModuleNVX;
    }
    table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)gpa(device, "vkDestroyCuFunctionNVX");
    if (table->DestroyCuFunctionNVX == nullptr) {
        table->DestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)StubDestroyCuFunctionNVX;
    }
    table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)gpa(device, "vkCmdCuLaunchKernelNVX");
    if (table->CmdCuLaunchKernelNVX == nullptr) {
        table->CmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)StubCmdCuLaunchKernelNVX;
    }
    table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)gpa(device, "vkGetImageViewHandleNVX");
    if (table->GetImageViewHandleNVX == nullptr) {
        table->GetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)StubGetImageViewHandleNVX;
    }
    table->GetImageViewHandle64NVX = (PFN_vkGetImageViewHandle64NVX)gpa(device, "vkGetImageViewHandle64NVX");
    if (table->GetImageViewHandle64NVX == nullptr) {
        table->GetImageViewHandle64NVX = (PFN_vkGetImageViewHandle64NVX)StubGetImageViewHandle64NVX;
    }
    table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)gpa(device, "vkGetImageViewAddressNVX");
    if (table->GetImageViewAddressNVX == nullptr) {
        table->GetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)StubGetImageViewAddressNVX;
    }
    table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)gpa(device, "vkCmdDrawIndirectCountAMD");
    if (table->CmdDrawIndirectCountAMD == nullptr) {
        table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)StubCmdDrawIndirectCountAMD;
    }
    table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)gpa(device, "vkCmdDrawIndexedIndirectCountAMD");
    if (table->CmdDrawIndexedIndirectCountAMD == nullptr) {
        table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)StubCmdDrawIndexedIndirectCountAMD;
    }
    table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)gpa(device, "vkGetShaderInfoAMD");
    if (table->GetShaderInfoAMD == nullptr) {
        table->GetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)StubGetShaderInfoAMD;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)gpa(device, "vkGetMemoryWin32HandleNV");
    if (table->GetMemoryWin32HandleNV == nullptr) {
        table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)StubGetMemoryWin32HandleNV;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)gpa(device, "vkCmdBeginConditionalRenderingEXT");
    if (table->CmdBeginConditionalRenderingEXT == nullptr) {
        table->CmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)StubCmdBeginConditionalRenderingEXT;
    }
    table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)gpa(device, "vkCmdEndConditionalRenderingEXT");
    if (table->CmdEndConditionalRenderingEXT == nullptr) {
        table->CmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)StubCmdEndConditionalRenderingEXT;
    }
    table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)gpa(device, "vkCmdSetViewportWScalingNV");
    if (table->CmdSetViewportWScalingNV == nullptr) {
        table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)StubCmdSetViewportWScalingNV;
    }
    table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)gpa(device, "vkDisplayPowerControlEXT");
    if (table->DisplayPowerControlEXT == nullptr) {
        table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)StubDisplayPowerControlEXT;
    }
    table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)gpa(device, "vkRegisterDeviceEventEXT");
    if (table->RegisterDeviceEventEXT == nullptr) {
        table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)StubRegisterDeviceEventEXT;
    }
    table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)gpa(device, "vkRegisterDisplayEventEXT");
    if (table->RegisterDisplayEventEXT == nullptr) {
        table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)StubRegisterDisplayEventEXT;
    }
    table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)gpa(device, "vkGetSwapchainCounterEXT");
    if (table->GetSwapchainCounterEXT == nullptr) {
        table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)StubGetSwapchainCounterEXT;
    }
    table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)gpa(device, "vkGetRefreshCycleDurationGOOGLE");
    if (table->GetRefreshCycleDurationGOOGLE == nullptr) {
        table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)StubGetRefreshCycleDurationGOOGLE;
    }
    table->GetPastPresentationTimingGOOGLE =
        (PFN_vkGetPastPresentationTimingGOOGLE)gpa(device, "vkGetPastPresentationTimingGOOGLE");
    if (table->GetPastPresentationTimingGOOGLE == nullptr) {
        table->GetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)StubGetPastPresentationTimingGOOGLE;
    }
    table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)gpa(device, "vkCmdSetDiscardRectangleEXT");
    if (table->CmdSetDiscardRectangleEXT == nullptr) {
        table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)StubCmdSetDiscardRectangleEXT;
    }
    table->CmdSetDiscardRectangleEnableEXT =
        (PFN_vkCmdSetDiscardRectangleEnableEXT)gpa(device, "vkCmdSetDiscardRectangleEnableEXT");
    if (table->CmdSetDiscardRectangleEnableEXT == nullptr) {
        table->CmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT)StubCmdSetDiscardRectangleEnableEXT;
    }
    table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)gpa(device, "vkCmdSetDiscardRectangleModeEXT");
    if (table->CmdSetDiscardRectangleModeEXT == nullptr) {
        table->CmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)StubCmdSetDiscardRectangleModeEXT;
    }
    table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)gpa(device, "vkSetHdrMetadataEXT");
    if (table->SetHdrMetadataEXT == nullptr) {
        table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)StubSetHdrMetadataEXT;
    }
    table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)gpa(device, "vkSetDebugUtilsObjectNameEXT");
    if (table->SetDebugUtilsObjectNameEXT == nullptr) {
        table->SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)StubSetDebugUtilsObjectNameEXT;
    }
    table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)gpa(device, "vkSetDebugUtilsObjectTagEXT");
    if (table->SetDebugUtilsObjectTagEXT == nullptr) {
        table->SetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)StubSetDebugUtilsObjectTagEXT;
    }
    table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)gpa(device, "vkQueueBeginDebugUtilsLabelEXT");
    if (table->QueueBeginDebugUtilsLabelEXT == nullptr) {
        table->QueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)StubQueueBeginDebugUtilsLabelEXT;
    }
    table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)gpa(device, "vkQueueEndDebugUtilsLabelEXT");
    if (table->QueueEndDebugUtilsLabelEXT == nullptr) {
        table->QueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)StubQueueEndDebugUtilsLabelEXT;
    }
    table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)gpa(device, "vkQueueInsertDebugUtilsLabelEXT");
    if (table->QueueInsertDebugUtilsLabelEXT == nullptr) {
        table->QueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)StubQueueInsertDebugUtilsLabelEXT;
    }
    table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)gpa(device, "vkCmdBeginDebugUtilsLabelEXT");
    if (table->CmdBeginDebugUtilsLabelEXT == nullptr) {
        table->CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)StubCmdBeginDebugUtilsLabelEXT;
    }
    table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)gpa(device, "vkCmdEndDebugUtilsLabelEXT");
    if (table->CmdEndDebugUtilsLabelEXT == nullptr) {
        table->CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)StubCmdEndDebugUtilsLabelEXT;
    }
    table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)gpa(device, "vkCmdInsertDebugUtilsLabelEXT");
    if (table->CmdInsertDebugUtilsLabelEXT == nullptr) {
        table->CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)StubCmdInsertDebugUtilsLabelEXT;
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->GetAndroidHardwareBufferPropertiesANDROID =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)gpa(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
    if (table->GetAndroidHardwareBufferPropertiesANDROID == nullptr) {
        table->GetAndroidHardwareBufferPropertiesANDROID =
            (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)StubGetAndroidHardwareBufferPropertiesANDROID;
    }
    table->GetMemoryAndroidHardwareBufferANDROID =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)gpa(device, "vkGetMemoryAndroidHardwareBufferANDROID");
    if (table->GetMemoryAndroidHardwareBufferANDROID == nullptr) {
        table->GetMemoryAndroidHardwareBufferANDROID =
            (PFN_vkGetMemoryAndroidHardwareBufferANDROID)StubGetMemoryAndroidHardwareBufferANDROID;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
    table->CreateExecutionGraphPipelinesAMDX =
        (PFN_vkCreateExecutionGraphPipelinesAMDX)gpa(device, "vkCreateExecutionGraphPipelinesAMDX");
    if (table->CreateExecutionGraphPipelinesAMDX == nullptr) {
        table->CreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX)StubCreateExecutionGraphPipelinesAMDX;
    }
    table->GetExecutionGraphPipelineScratchSizeAMDX =
        (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)gpa(device, "vkGetExecutionGraphPipelineScratchSizeAMDX");
    if (table->GetExecutionGraphPipelineScratchSizeAMDX == nullptr) {
        table->GetExecutionGraphPipelineScratchSizeAMDX =
            (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)StubGetExecutionGraphPipelineScratchSizeAMDX;
    }
    table->GetExecutionGraphPipelineNodeIndexAMDX =
        (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)gpa(device, "vkGetExecutionGraphPipelineNodeIndexAMDX");
    if (table->GetExecutionGraphPipelineNodeIndexAMDX == nullptr) {
        table->GetExecutionGraphPipelineNodeIndexAMDX =
            (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)StubGetExecutionGraphPipelineNodeIndexAMDX;
    }
    table->CmdInitializeGraphScratchMemoryAMDX =
        (PFN_vkCmdInitializeGraphScratchMemoryAMDX)gpa(device, "vkCmdInitializeGraphScratchMemoryAMDX");
    if (table->CmdInitializeGraphScratchMemoryAMDX == nullptr) {
        table->CmdInitializeGraphScratchMemoryAMDX =
            (PFN_vkCmdInitializeGraphScratchMemoryAMDX)StubCmdInitializeGraphScratchMemoryAMDX;
    }
    table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)gpa(device, "vkCmdDispatchGraphAMDX");
    if (table->CmdDispatchGraphAMDX == nullptr) {
        table->CmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)StubCmdDispatchGraphAMDX;
    }
    table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)gpa(device, "vkCmdDispatchGraphIndirectAMDX");
    if (table->CmdDispatchGraphIndirectAMDX == nullptr) {
        table->CmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)StubCmdDispatchGraphIndirectAMDX;
    }
    table->CmdDispatchGraphIndirectCountAMDX =
        (PFN_vkCmdDispatchGraphIndirectCountAMDX)gpa(device, "vkCmdDispatchGraphIndirectCountAMDX");
    if (table->CmdDispatchGraphIndirectCountAMDX == nullptr) {
        table->CmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX)StubCmdDispatchGraphIndirectCountAMDX;
    }
#endif  // VK_ENABLE_BETA_EXTENSIONS
    table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)gpa(device, "vkCmdSetSampleLocationsEXT");
    if (table->CmdSetSampleLocationsEXT == nullptr) {
        table->CmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)StubCmdSetSampleLocationsEXT;
    }
    table->GetImageDrmFormatModifierPropertiesEXT =
        (PFN_vkGetImageDrmFormatModifierPropertiesEXT)gpa(device, "vkGetImageDrmFormatModifierPropertiesEXT");
    if (table->GetImageDrmFormatModifierPropertiesEXT == nullptr) {
        table->GetImageDrmFormatModifierPropertiesEXT =
            (PFN_vkGetImageDrmFormatModifierPropertiesEXT)StubGetImageDrmFormatModifierPropertiesEXT;
    }
    table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)gpa(device, "vkCreateValidationCacheEXT");
    if (table->CreateValidationCacheEXT == nullptr) {
        table->CreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)StubCreateValidationCacheEXT;
    }
    table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)gpa(device, "vkDestroyValidationCacheEXT");
    if (table->DestroyValidationCacheEXT == nullptr) {
        table->DestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)StubDestroyValidationCacheEXT;
    }
    table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)gpa(device, "vkMergeValidationCachesEXT");
    if (table->MergeValidationCachesEXT == nullptr) {
        table->MergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)StubMergeValidationCachesEXT;
    }
    table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)gpa(device, "vkGetValidationCacheDataEXT");
    if (table->GetValidationCacheDataEXT == nullptr) {
        table->GetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)StubGetValidationCacheDataEXT;
    }
    table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)gpa(device, "vkCmdBindShadingRateImageNV");
    if (table->CmdBindShadingRateImageNV == nullptr) {
        table->CmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)StubCmdBindShadingRateImageNV;
    }
    table->CmdSetViewportShadingRatePaletteNV =
        (PFN_vkCmdSetViewportShadingRatePaletteNV)gpa(device, "vkCmdSetViewportShadingRatePaletteNV");
    if (table->CmdSetViewportShadingRatePaletteNV == nullptr) {
        table->CmdSetViewportShadingRatePaletteNV =
            (PFN_vkCmdSetViewportShadingRatePaletteNV)StubCmdSetViewportShadingRatePaletteNV;
    }
    table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)gpa(device, "vkCmdSetCoarseSampleOrderNV");
    if (table->CmdSetCoarseSampleOrderNV == nullptr) {
        table->CmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)StubCmdSetCoarseSampleOrderNV;
    }
    table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)gpa(device, "vkCreateAccelerationStructureNV");
    if (table->CreateAccelerationStructureNV == nullptr) {
        table->CreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)StubCreateAccelerationStructureNV;
    }
    table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)gpa(device, "vkDestroyAccelerationStructureNV");
    if (table->DestroyAccelerationStructureNV == nullptr) {
        table->DestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)StubDestroyAccelerationStructureNV;
    }
    table->GetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)gpa(device, "vkGetAccelerationStructureMemoryRequirementsNV");
    if (table->GetAccelerationStructureMemoryRequirementsNV == nullptr) {
        table->GetAccelerationStructureMemoryRequirementsNV =
            (PFN_vkGetAccelerationStructureMemoryRequirementsNV)StubGetAccelerationStructureMemoryRequirementsNV;
    }
    table->BindAccelerationStructureMemoryNV =
        (PFN_vkBindAccelerationStructureMemoryNV)gpa(device, "vkBindAccelerationStructureMemoryNV");
    if (table->BindAccelerationStructureMemoryNV == nullptr) {
        table->BindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)StubBindAccelerationStructureMemoryNV;
    }
    table->CmdBuildAccelerationStructureNV =
        (PFN_vkCmdBuildAccelerationStructureNV)gpa(device, "vkCmdBuildAccelerationStructureNV");
    if (table->CmdBuildAccelerationStructureNV == nullptr) {
        table->CmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)StubCmdBuildAccelerationStructureNV;
    }
    table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)gpa(device, "vkCmdCopyAccelerationStructureNV");
    if (table->CmdCopyAccelerationStructureNV == nullptr) {
        table->CmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)StubCmdCopyAccelerationStructureNV;
    }
    table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)gpa(device, "vkCmdTraceRaysNV");
    if (table->CmdTraceRaysNV == nullptr) {
        table->CmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)StubCmdTraceRaysNV;
    }
    table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)gpa(device, "vkCreateRayTracingPipelinesNV");
    if (table->CreateRayTracingPipelinesNV == nullptr) {
        table->CreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)StubCreateRayTracingPipelinesNV;
    }
    table->GetRayTracingShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingShaderGroupHandlesKHR");
    if (table->GetRayTracingShaderGroupHandlesKHR == nullptr) {
        table->GetRayTracingShaderGroupHandlesKHR =
            (PFN_vkGetRayTracingShaderGroupHandlesKHR)StubGetRayTracingShaderGroupHandlesKHR;
    }
    table->GetRayTracingShaderGroupHandlesNV =
        (PFN_vkGetRayTracingShaderGroupHandlesNV)gpa(device, "vkGetRayTracingShaderGroupHandlesNV");
    if (table->GetRayTracingShaderGroupHandlesNV == nullptr) {
        table->GetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV)StubGetRayTracingShaderGroupHandlesNV;
    }
    table->GetAccelerationStructureHandleNV =
        (PFN_vkGetAccelerationStructureHandleNV)gpa(device, "vkGetAccelerationStructureHandleNV");
    if (table->GetAccelerationStructureHandleNV == nullptr) {
        table->GetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)StubGetAccelerationStructureHandleNV;
    }
    table->CmdWriteAccelerationStructuresPropertiesNV =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesNV");
    if (table->CmdWriteAccelerationStructuresPropertiesNV == nullptr) {
        table->CmdWriteAccelerationStructuresPropertiesNV =
            (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)StubCmdWriteAccelerationStructuresPropertiesNV;
    }
    table->CompileDeferredNV = (PFN_vkCompileDeferredNV)gpa(device, "vkCompileDeferredNV");
    if (table->CompileDeferredNV == nullptr) {
        table->CompileDeferredNV = (PFN_vkCompileDeferredNV)StubCompileDeferredNV;
    }
    table->GetMemoryHostPointerPropertiesEXT =
        (PFN_vkGetMemoryHostPointerPropertiesEXT)gpa(device, "vkGetMemoryHostPointerPropertiesEXT");
    if (table->GetMemoryHostPointerPropertiesEXT == nullptr) {
        table->GetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)StubGetMemoryHostPointerPropertiesEXT;
    }
    table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)gpa(device, "vkCmdWriteBufferMarkerAMD");
    if (table->CmdWriteBufferMarkerAMD == nullptr) {
        table->CmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)StubCmdWriteBufferMarkerAMD;
    }
    table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)gpa(device, "vkCmdWriteBufferMarker2AMD");
    if (table->CmdWriteBufferMarker2AMD == nullptr) {
        table->CmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)StubCmdWriteBufferMarker2AMD;
    }
    table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)gpa(device, "vkGetCalibratedTimestampsEXT");
    if (table->GetCalibratedTimestampsEXT == nullptr) {
        table->GetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)StubGetCalibratedTimestampsEXT;
    }
    table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)gpa(device, "vkCmdDrawMeshTasksNV");
    if (table->CmdDrawMeshTasksNV == nullptr) {
        table->CmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)StubCmdDrawMeshTasksNV;
    }
    table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)gpa(device, "vkCmdDrawMeshTasksIndirectNV");
    if (table->CmdDrawMeshTasksIndirectNV == nullptr) {
        table->CmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)StubCmdDrawMeshTasksIndirectNV;
    }
    table->CmdDrawMeshTasksIndirectCountNV =
        (PFN_vkCmdDrawMeshTasksIndirectCountNV)gpa(device, "vkCmdDrawMeshTasksIndirectCountNV");
    if (table->CmdDrawMeshTasksIndirectCountNV == nullptr) {
        table->CmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)StubCmdDrawMeshTasksIndirectCountNV;
    }
    table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)gpa(device, "vkCmdSetExclusiveScissorEnableNV");
    if (table->CmdSetExclusiveScissorEnableNV == nullptr) {
        table->CmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)StubCmdSetExclusiveScissorEnableNV;
    }
    table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)gpa(device, "vkCmdSetExclusiveScissorNV");
    if (table->CmdSetExclusiveScissorNV == nullptr) {
        table->CmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)StubCmdSetExclusiveScissorNV;
    }
    table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)gpa(device, "vkCmdSetCheckpointNV");
    if (table->CmdSetCheckpointNV == nullptr) {
        table->CmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)StubCmdSetCheckpointNV;
    }
    table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)gpa(device, "vkGetQueueCheckpointDataNV");
    if (table->GetQueueCheckpointDataNV == nullptr) {
        table->GetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)StubGetQueueCheckpointDataNV;
    }
    table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)gpa(device, "vkGetQueueCheckpointData2NV");
    if (table->GetQueueCheckpointData2NV == nullptr) {
        table->GetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)StubGetQueueCheckpointData2NV;
    }
    table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)gpa(device, "vkInitializePerformanceApiINTEL");
    if (table->InitializePerformanceApiINTEL == nullptr) {
        table->InitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)StubInitializePerformanceApiINTEL;
    }
    table->UninitializePerformanceApiINTEL =
        (PFN_vkUninitializePerformanceApiINTEL)gpa(device, "vkUninitializePerformanceApiINTEL");
    if (table->UninitializePerformanceApiINTEL == nullptr) {
        table->UninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)StubUninitializePerformanceApiINTEL;
    }
    table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)gpa(device, "vkCmdSetPerformanceMarkerINTEL");
    if (table->CmdSetPerformanceMarkerINTEL == nullptr) {
        table->CmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)StubCmdSetPerformanceMarkerINTEL;
    }
    table->CmdSetPerformanceStreamMarkerINTEL =
        (PFN_vkCmdSetPerformanceStreamMarkerINTEL)gpa(device, "vkCmdSetPerformanceStreamMarkerINTEL");
    if (table->CmdSetPerformanceStreamMarkerINTEL == nullptr) {
        table->CmdSetPerformanceStreamMarkerINTEL =
            (PFN_vkCmdSetPerformanceStreamMarkerINTEL)StubCmdSetPerformanceStreamMarkerINTEL;
    }
    table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)gpa(device, "vkCmdSetPerformanceOverrideINTEL");
    if (table->CmdSetPerformanceOverrideINTEL == nullptr) {
        table->CmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)StubCmdSetPerformanceOverrideINTEL;
    }
    table->AcquirePerformanceConfigurationINTEL =
        (PFN_vkAcquirePerformanceConfigurationINTEL)gpa(device, "vkAcquirePerformanceConfigurationINTEL");
    if (table->AcquirePerformanceConfigurationINTEL == nullptr) {
        table->AcquirePerformanceConfigurationINTEL =
            (PFN_vkAcquirePerformanceConfigurationINTEL)StubAcquirePerformanceConfigurationINTEL;
    }
    table->ReleasePerformanceConfigurationINTEL =
        (PFN_vkReleasePerformanceConfigurationINTEL)gpa(device, "vkReleasePerformanceConfigurationINTEL");
    if (table->ReleasePerformanceConfigurationINTEL == nullptr) {
        table->ReleasePerformanceConfigurationINTEL =
            (PFN_vkReleasePerformanceConfigurationINTEL)StubReleasePerformanceConfigurationINTEL;
    }
    table->QueueSetPerformanceConfigurationINTEL =
        (PFN_vkQueueSetPerformanceConfigurationINTEL)gpa(device, "vkQueueSetPerformanceConfigurationINTEL");
    if (table->QueueSetPerformanceConfigurationINTEL == nullptr) {
        table->QueueSetPerformanceConfigurationINTEL =
            (PFN_vkQueueSetPerformanceConfigurationINTEL)StubQueueSetPerformanceConfigurationINTEL;
    }
    table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)gpa(device, "vkGetPerformanceParameterINTEL");
    if (table->GetPerformanceParameterINTEL == nullptr) {
        table->GetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)StubGetPerformanceParameterINTEL;
    }
    table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)gpa(device, "vkSetLocalDimmingAMD");
    if (table->SetLocalDimmingAMD == nullptr) {
        table->SetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)StubSetLocalDimmingAMD;
    }
    table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)gpa(device, "vkGetBufferDeviceAddressEXT");
    if (table->GetBufferDeviceAddressEXT == nullptr) {
        table->GetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)StubGetBufferDeviceAddressEXT;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireFullScreenExclusiveModeEXT =
        (PFN_vkAcquireFullScreenExclusiveModeEXT)gpa(device, "vkAcquireFullScreenExclusiveModeEXT");
    if (table->AcquireFullScreenExclusiveModeEXT == nullptr) {
        table->AcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)StubAcquireFullScreenExclusiveModeEXT;
    }
    table->ReleaseFullScreenExclusiveModeEXT =
        (PFN_vkReleaseFullScreenExclusiveModeEXT)gpa(device, "vkReleaseFullScreenExclusiveModeEXT");
    if (table->ReleaseFullScreenExclusiveModeEXT == nullptr) {
        table->ReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)StubReleaseFullScreenExclusiveModeEXT;
    }
    table->GetDeviceGroupSurfacePresentModes2EXT =
        (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)gpa(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
    if (table->GetDeviceGroupSurfacePresentModes2EXT == nullptr) {
        table->GetDeviceGroupSurfacePresentModes2EXT =
            (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)StubGetDeviceGroupSurfacePresentModes2EXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)gpa(device, "vkCmdSetLineStippleEXT");
    if (table->CmdSetLineStippleEXT == nullptr) {
        table->CmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)StubCmdSetLineStippleEXT;
    }
    table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)gpa(device, "vkResetQueryPoolEXT");
    if (table->ResetQueryPoolEXT == nullptr) {
        table->ResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)StubResetQueryPoolEXT;
    }
    table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)gpa(device, "vkCmdSetCullModeEXT");
    if (table->CmdSetCullModeEXT == nullptr) {
        table->CmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)StubCmdSetCullModeEXT;
    }
    table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)gpa(device, "vkCmdSetFrontFaceEXT");
    if (table->CmdSetFrontFaceEXT == nullptr) {
        table->CmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)StubCmdSetFrontFaceEXT;
    }
    table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)gpa(device, "vkCmdSetPrimitiveTopologyEXT");
    if (table->CmdSetPrimitiveTopologyEXT == nullptr) {
        table->CmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)StubCmdSetPrimitiveTopologyEXT;
    }
    table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)gpa(device, "vkCmdSetViewportWithCountEXT");
    if (table->CmdSetViewportWithCountEXT == nullptr) {
        table->CmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)StubCmdSetViewportWithCountEXT;
    }
    table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)gpa(device, "vkCmdSetScissorWithCountEXT");
    if (table->CmdSetScissorWithCountEXT == nullptr) {
        table->CmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)StubCmdSetScissorWithCountEXT;
    }
    table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)gpa(device, "vkCmdBindVertexBuffers2EXT");
    if (table->CmdBindVertexBuffers2EXT == nullptr) {
        table->CmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)StubCmdBindVertexBuffers2EXT;
    }
    table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)gpa(device, "vkCmdSetDepthTestEnableEXT");
    if (table->CmdSetDepthTestEnableEXT == nullptr) {
        table->CmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)StubCmdSetDepthTestEnableEXT;
    }
    table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)gpa(device, "vkCmdSetDepthWriteEnableEXT");
    if (table->CmdSetDepthWriteEnableEXT == nullptr) {
        table->CmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)StubCmdSetDepthWriteEnableEXT;
    }
    table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)gpa(device, "vkCmdSetDepthCompareOpEXT");
    if (table->CmdSetDepthCompareOpEXT == nullptr) {
        table->CmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)StubCmdSetDepthCompareOpEXT;
    }
    table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)gpa(device, "vkCmdSetDepthBoundsTestEnableEXT");
    if (table->CmdSetDepthBoundsTestEnableEXT == nullptr) {
        table->CmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)StubCmdSetDepthBoundsTestEnableEXT;
    }
    table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)gpa(device, "vkCmdSetStencilTestEnableEXT");
    if (table->CmdSetStencilTestEnableEXT == nullptr) {
        table->CmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)StubCmdSetStencilTestEnableEXT;
    }
    table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)gpa(device, "vkCmdSetStencilOpEXT");
    if (table->CmdSetStencilOpEXT == nullptr) {
        table->CmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)StubCmdSetStencilOpEXT;
    }
    table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)gpa(device, "vkCopyMemoryToImageEXT");
    if (table->CopyMemoryToImageEXT == nullptr) {
        table->CopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)StubCopyMemoryToImageEXT;
    }
    table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)gpa(device, "vkCopyImageToMemoryEXT");
    if (table->CopyImageToMemoryEXT == nullptr) {
        table->CopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)StubCopyImageToMemoryEXT;
    }
    table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)gpa(device, "vkCopyImageToImageEXT");
    if (table->CopyImageToImageEXT == nullptr) {
        table->CopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)StubCopyImageToImageEXT;
    }
    table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)gpa(device, "vkTransitionImageLayoutEXT");
    if (table->TransitionImageLayoutEXT == nullptr) {
        table->TransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)StubTransitionImageLayoutEXT;
    }
    table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)gpa(device, "vkGetImageSubresourceLayout2EXT");
    if (table->GetImageSubresourceLayout2EXT == nullptr) {
        table->GetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)StubGetImageSubresourceLayout2EXT;
    }
    table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)gpa(device, "vkReleaseSwapchainImagesEXT");
    if (table->ReleaseSwapchainImagesEXT == nullptr) {
        table->ReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)StubReleaseSwapchainImagesEXT;
    }
    table->GetGeneratedCommandsMemoryRequirementsNV =
        (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)gpa(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
    if (table->GetGeneratedCommandsMemoryRequirementsNV == nullptr) {
        table->GetGeneratedCommandsMemoryRequirementsNV =
            (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)StubGetGeneratedCommandsMemoryRequirementsNV;
    }
    table->CmdPreprocessGeneratedCommandsNV =
        (PFN_vkCmdPreprocessGeneratedCommandsNV)gpa(device, "vkCmdPreprocessGeneratedCommandsNV");
    if (table->CmdPreprocessGeneratedCommandsNV == nullptr) {
        table->CmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV)StubCmdPreprocessGeneratedCommandsNV;
    }
    table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)gpa(device, "vkCmdExecuteGeneratedCommandsNV");
    if (table->CmdExecuteGeneratedCommandsNV == nullptr) {
        table->CmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)StubCmdExecuteGeneratedCommandsNV;
    }
    table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)gpa(device, "vkCmdBindPipelineShaderGroupNV");
    if (table->CmdBindPipelineShaderGroupNV == nullptr) {
        table->CmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)StubCmdBindPipelineShaderGroupNV;
    }
    table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)gpa(device, "vkCreateIndirectCommandsLayoutNV");
    if (table->CreateIndirectCommandsLayoutNV == nullptr) {
        table->CreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)StubCreateIndirectCommandsLayoutNV;
    }
    table->DestroyIndirectCommandsLayoutNV =
        (PFN_vkDestroyIndirectCommandsLayoutNV)gpa(device, "vkDestroyIndirectCommandsLayoutNV");
    if (table->DestroyIndirectCommandsLayoutNV == nullptr) {
        table->DestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV)StubDestroyIndirectCommandsLayoutNV;
    }
    table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)gpa(device, "vkCmdSetDepthBias2EXT");
    if (table->CmdSetDepthBias2EXT == nullptr) {
        table->CmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)StubCmdSetDepthBias2EXT;
    }
    table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)gpa(device, "vkCreatePrivateDataSlotEXT");
    if (table->CreatePrivateDataSlotEXT == nullptr) {
        table->CreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)StubCreatePrivateDataSlotEXT;
    }
    table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)gpa(device, "vkDestroyPrivateDataSlotEXT");
    if (table->DestroyPrivateDataSlotEXT == nullptr) {
        table->DestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)StubDestroyPrivateDataSlotEXT;
    }
    table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)gpa(device, "vkSetPrivateDataEXT");
    if (table->SetPrivateDataEXT == nullptr) {
        table->SetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)StubSetPrivateDataEXT;
    }
    table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)gpa(device, "vkGetPrivateDataEXT");
    if (table->GetPrivateDataEXT == nullptr) {
        table->GetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)StubGetPrivateDataEXT;
    }
    table->CreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)gpa(device, "vkCreateCudaModuleNV");
    if (table->CreateCudaModuleNV == nullptr) {
        table->CreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)StubCreateCudaModuleNV;
    }
    table->GetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)gpa(device, "vkGetCudaModuleCacheNV");
    if (table->GetCudaModuleCacheNV == nullptr) {
        table->GetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)StubGetCudaModuleCacheNV;
    }
    table->CreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)gpa(device, "vkCreateCudaFunctionNV");
    if (table->CreateCudaFunctionNV == nullptr) {
        table->CreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)StubCreateCudaFunctionNV;
    }
    table->DestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)gpa(device, "vkDestroyCudaModuleNV");
    if (table->DestroyCudaModuleNV == nullptr) {
        table->DestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)StubDestroyCudaModuleNV;
    }
    table->DestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)gpa(device, "vkDestroyCudaFunctionNV");
    if (table->DestroyCudaFunctionNV == nullptr) {
        table->DestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)StubDestroyCudaFunctionNV;
    }
    table->CmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)gpa(device, "vkCmdCudaLaunchKernelNV");
    if (table->CmdCudaLaunchKernelNV == nullptr) {
        table->CmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)StubCmdCudaLaunchKernelNV;
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)gpa(device, "vkExportMetalObjectsEXT");
    if (table->ExportMetalObjectsEXT == nullptr) {
        table->ExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)StubExportMetalObjectsEXT;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)gpa(device, "vkGetDescriptorSetLayoutSizeEXT");
    if (table->GetDescriptorSetLayoutSizeEXT == nullptr) {
        table->GetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)StubGetDescriptorSetLayoutSizeEXT;
    }
    table->GetDescriptorSetLayoutBindingOffsetEXT =
        (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)gpa(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    if (table->GetDescriptorSetLayoutBindingOffsetEXT == nullptr) {
        table->GetDescriptorSetLayoutBindingOffsetEXT =
            (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)StubGetDescriptorSetLayoutBindingOffsetEXT;
    }
    table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)gpa(device, "vkGetDescriptorEXT");
    if (table->GetDescriptorEXT == nullptr) {
        table->GetDescriptorEXT = (PFN_vkGetDescriptorEXT)StubGetDescriptorEXT;
    }
    table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)gpa(device, "vkCmdBindDescriptorBuffersEXT");
    if (table->CmdBindDescriptorBuffersEXT == nullptr) {
        table->CmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)StubCmdBindDescriptorBuffersEXT;
    }
    table->CmdSetDescriptorBufferOffsetsEXT =
        (PFN_vkCmdSetDescriptorBufferOffsetsEXT)gpa(device, "vkCmdSetDescriptorBufferOffsetsEXT");
    if (table->CmdSetDescriptorBufferOffsetsEXT == nullptr) {
        table->CmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)StubCmdSetDescriptorBufferOffsetsEXT;
    }
    table->CmdBindDescriptorBufferEmbeddedSamplersEXT =
        (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)gpa(device, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    if (table->CmdBindDescriptorBufferEmbeddedSamplersEXT == nullptr) {
        table->CmdBindDescriptorBufferEmbeddedSamplersEXT =
            (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)StubCmdBindDescriptorBufferEmbeddedSamplersEXT;
    }
    table->GetBufferOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    if (table->GetBufferOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetBufferOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)StubGetBufferOpaqueCaptureDescriptorDataEXT;
    }
    table->GetImageOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    if (table->GetImageOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetImageOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)StubGetImageOpaqueCaptureDescriptorDataEXT;
    }
    table->GetImageViewOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    if (table->GetImageViewOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetImageViewOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)StubGetImageViewOpaqueCaptureDescriptorDataEXT;
    }
    table->GetSamplerOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)gpa(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    if (table->GetSamplerOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetSamplerOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)StubGetSamplerOpaqueCaptureDescriptorDataEXT;
    }
    table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
        (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)gpa(
            device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    if (table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT == nullptr) {
        table->GetAccelerationStructureOpaqueCaptureDescriptorDataEXT =
            (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)
                StubGetAccelerationStructureOpaqueCaptureDescriptorDataEXT;
    }
    table->CmdSetFragmentShadingRateEnumNV =
        (PFN_vkCmdSetFragmentShadingRateEnumNV)gpa(device, "vkCmdSetFragmentShadingRateEnumNV");
    if (table->CmdSetFragmentShadingRateEnumNV == nullptr) {
        table->CmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV)StubCmdSetFragmentShadingRateEnumNV;
    }
    table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)gpa(device, "vkGetDeviceFaultInfoEXT");
    if (table->GetDeviceFaultInfoEXT == nullptr) {
        table->GetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)StubGetDeviceFaultInfoEXT;
    }
    table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)gpa(device, "vkCmdSetVertexInputEXT");
    if (table->CmdSetVertexInputEXT == nullptr) {
        table->CmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)StubCmdSetVertexInputEXT;
    }
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)gpa(device, "vkGetMemoryZirconHandleFUCHSIA");
    if (table->GetMemoryZirconHandleFUCHSIA == nullptr) {
        table->GetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)StubGetMemoryZirconHandleFUCHSIA;
    }
    table->GetMemoryZirconHandlePropertiesFUCHSIA =
        (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)gpa(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
    if (table->GetMemoryZirconHandlePropertiesFUCHSIA == nullptr) {
        table->GetMemoryZirconHandlePropertiesFUCHSIA =
            (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)StubGetMemoryZirconHandlePropertiesFUCHSIA;
    }
    table->ImportSemaphoreZirconHandleFUCHSIA =
        (PFN_vkImportSemaphoreZirconHandleFUCHSIA)gpa(device, "vkImportSemaphoreZirconHandleFUCHSIA");
    if (table->ImportSemaphoreZirconHandleFUCHSIA == nullptr) {
        table->ImportSemaphoreZirconHandleFUCHSIA =
            (PFN_vkImportSemaphoreZirconHandleFUCHSIA)StubImportSemaphoreZirconHandleFUCHSIA;
    }
    table->GetSemaphoreZirconHandleFUCHSIA =
        (PFN_vkGetSemaphoreZirconHandleFUCHSIA)gpa(device, "vkGetSemaphoreZirconHandleFUCHSIA");
    if (table->GetSemaphoreZirconHandleFUCHSIA == nullptr) {
        table->GetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA)StubGetSemaphoreZirconHandleFUCHSIA;
    }
    table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)gpa(device, "vkCreateBufferCollectionFUCHSIA");
    if (table->CreateBufferCollectionFUCHSIA == nullptr) {
        table->CreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)StubCreateBufferCollectionFUCHSIA;
    }
    table->SetBufferCollectionImageConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
    if (table->SetBufferCollectionImageConstraintsFUCHSIA == nullptr) {
        table->SetBufferCollectionImageConstraintsFUCHSIA =
            (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)StubSetBufferCollectionImageConstraintsFUCHSIA;
    }
    table->SetBufferCollectionBufferConstraintsFUCHSIA =
        (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)gpa(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    if (table->SetBufferCollectionBufferConstraintsFUCHSIA == nullptr) {
        table->SetBufferCollectionBufferConstraintsFUCHSIA =
            (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)StubSetBufferCollectionBufferConstraintsFUCHSIA;
    }
    table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)gpa(device, "vkDestroyBufferCollectionFUCHSIA");
    if (table->DestroyBufferCollectionFUCHSIA == nullptr) {
        table->DestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)StubDestroyBufferCollectionFUCHSIA;
    }
    table->GetBufferCollectionPropertiesFUCHSIA =
        (PFN_vkGetBufferCollectionPropertiesFUCHSIA)gpa(device, "vkGetBufferCollectionPropertiesFUCHSIA");
    if (table->GetBufferCollectionPropertiesFUCHSIA == nullptr) {
        table->GetBufferCollectionPropertiesFUCHSIA =
            (PFN_vkGetBufferCollectionPropertiesFUCHSIA)StubGetBufferCollectionPropertiesFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
    table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI =
        (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)gpa(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    if (table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI == nullptr) {
        table->GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI =
            (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)StubGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI;
    }
    table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)gpa(device, "vkCmdSubpassShadingHUAWEI");
    if (table->CmdSubpassShadingHUAWEI == nullptr) {
        table->CmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)StubCmdSubpassShadingHUAWEI;
    }
    table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)gpa(device, "vkCmdBindInvocationMaskHUAWEI");
    if (table->CmdBindInvocationMaskHUAWEI == nullptr) {
        table->CmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)StubCmdBindInvocationMaskHUAWEI;
    }
    table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)gpa(device, "vkGetMemoryRemoteAddressNV");
    if (table->GetMemoryRemoteAddressNV == nullptr) {
        table->GetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)StubGetMemoryRemoteAddressNV;
    }
    table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)gpa(device, "vkGetPipelinePropertiesEXT");
    if (table->GetPipelinePropertiesEXT == nullptr) {
        table->GetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)StubGetPipelinePropertiesEXT;
    }
    table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)gpa(device, "vkCmdSetPatchControlPointsEXT");
    if (table->CmdSetPatchControlPointsEXT == nullptr) {
        table->CmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)StubCmdSetPatchControlPointsEXT;
    }
    table->CmdSetRasterizerDiscardEnableEXT =
        (PFN_vkCmdSetRasterizerDiscardEnableEXT)gpa(device, "vkCmdSetRasterizerDiscardEnableEXT");
    if (table->CmdSetRasterizerDiscardEnableEXT == nullptr) {
        table->CmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT)StubCmdSetRasterizerDiscardEnableEXT;
    }
    table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)gpa(device, "vkCmdSetDepthBiasEnableEXT");
    if (table->CmdSetDepthBiasEnableEXT == nullptr) {
        table->CmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)StubCmdSetDepthBiasEnableEXT;
    }
    table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)gpa(device, "vkCmdSetLogicOpEXT");
    if (table->CmdSetLogicOpEXT == nullptr) {
        table->CmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)StubCmdSetLogicOpEXT;
    }
    table->CmdSetPrimitiveRestartEnableEXT =
        (PFN_vkCmdSetPrimitiveRestartEnableEXT)gpa(device, "vkCmdSetPrimitiveRestartEnableEXT");
    if (table->CmdSetPrimitiveRestartEnableEXT == nullptr) {
        table->CmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT)StubCmdSetPrimitiveRestartEnableEXT;
    }
    table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)gpa(device, "vkCmdSetColorWriteEnableEXT");
    if (table->CmdSetColorWriteEnableEXT == nullptr) {
        table->CmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)StubCmdSetColorWriteEnableEXT;
    }
    table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)gpa(device, "vkCmdDrawMultiEXT");
    if (table->CmdDrawMultiEXT == nullptr) {
        table->CmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)StubCmdDrawMultiEXT;
    }
    table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)gpa(device, "vkCmdDrawMultiIndexedEXT");
    if (table->CmdDrawMultiIndexedEXT == nullptr) {
        table->CmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)StubCmdDrawMultiIndexedEXT;
    }
    table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)gpa(device, "vkCreateMicromapEXT");
    if (table->CreateMicromapEXT == nullptr) {
        table->CreateMicromapEXT = (PFN_vkCreateMicromapEXT)StubCreateMicromapEXT;
    }
    table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)gpa(device, "vkDestroyMicromapEXT");
    if (table->DestroyMicromapEXT == nullptr) {
        table->DestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)StubDestroyMicromapEXT;
    }
    table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)gpa(device, "vkCmdBuildMicromapsEXT");
    if (table->CmdBuildMicromapsEXT == nullptr) {
        table->CmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)StubCmdBuildMicromapsEXT;
    }
    table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)gpa(device, "vkBuildMicromapsEXT");
    if (table->BuildMicromapsEXT == nullptr) {
        table->BuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)StubBuildMicromapsEXT;
    }
    table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)gpa(device, "vkCopyMicromapEXT");
    if (table->CopyMicromapEXT == nullptr) {
        table->CopyMicromapEXT = (PFN_vkCopyMicromapEXT)StubCopyMicromapEXT;
    }
    table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)gpa(device, "vkCopyMicromapToMemoryEXT");
    if (table->CopyMicromapToMemoryEXT == nullptr) {
        table->CopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)StubCopyMicromapToMemoryEXT;
    }
    table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)gpa(device, "vkCopyMemoryToMicromapEXT");
    if (table->CopyMemoryToMicromapEXT == nullptr) {
        table->CopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)StubCopyMemoryToMicromapEXT;
    }
    table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)gpa(device, "vkWriteMicromapsPropertiesEXT");
    if (table->WriteMicromapsPropertiesEXT == nullptr) {
        table->WriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)StubWriteMicromapsPropertiesEXT;
    }
    table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)gpa(device, "vkCmdCopyMicromapEXT");
    if (table->CmdCopyMicromapEXT == nullptr) {
        table->CmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)StubCmdCopyMicromapEXT;
    }
    table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)gpa(device, "vkCmdCopyMicromapToMemoryEXT");
    if (table->CmdCopyMicromapToMemoryEXT == nullptr) {
        table->CmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)StubCmdCopyMicromapToMemoryEXT;
    }
    table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)gpa(device, "vkCmdCopyMemoryToMicromapEXT");
    if (table->CmdCopyMemoryToMicromapEXT == nullptr) {
        table->CmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)StubCmdCopyMemoryToMicromapEXT;
    }
    table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)gpa(device, "vkCmdWriteMicromapsPropertiesEXT");
    if (table->CmdWriteMicromapsPropertiesEXT == nullptr) {
        table->CmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)StubCmdWriteMicromapsPropertiesEXT;
    }
    table->GetDeviceMicromapCompatibilityEXT =
        (PFN_vkGetDeviceMicromapCompatibilityEXT)gpa(device, "vkGetDeviceMicromapCompatibilityEXT");
    if (table->GetDeviceMicromapCompatibilityEXT == nullptr) {
        table->GetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT)StubGetDeviceMicromapCompatibilityEXT;
    }
    table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)gpa(device, "vkGetMicromapBuildSizesEXT");
    if (table->GetMicromapBuildSizesEXT == nullptr) {
        table->GetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)StubGetMicromapBuildSizesEXT;
    }
    table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)gpa(device, "vkCmdDrawClusterHUAWEI");
    if (table->CmdDrawClusterHUAWEI == nullptr) {
        table->CmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)StubCmdDrawClusterHUAWEI;
    }
    table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)gpa(device, "vkCmdDrawClusterIndirectHUAWEI");
    if (table->CmdDrawClusterIndirectHUAWEI == nullptr) {
        table->CmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)StubCmdDrawClusterIndirectHUAWEI;
    }
    table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)gpa(device, "vkSetDeviceMemoryPriorityEXT");
    if (table->SetDeviceMemoryPriorityEXT == nullptr) {
        table->SetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)StubSetDeviceMemoryPriorityEXT;
    }
    table->GetDescriptorSetLayoutHostMappingInfoVALVE =
        (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)gpa(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    if (table->GetDescriptorSetLayoutHostMappingInfoVALVE == nullptr) {
        table->GetDescriptorSetLayoutHostMappingInfoVALVE =
            (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)StubGetDescriptorSetLayoutHostMappingInfoVALVE;
    }
    table->GetDescriptorSetHostMappingVALVE =
        (PFN_vkGetDescriptorSetHostMappingVALVE)gpa(device, "vkGetDescriptorSetHostMappingVALVE");
    if (table->GetDescriptorSetHostMappingVALVE == nullptr) {
        table->GetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE)StubGetDescriptorSetHostMappingVALVE;
    }
    table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)gpa(device, "vkCmdCopyMemoryIndirectNV");
    if (table->CmdCopyMemoryIndirectNV == nullptr) {
        table->CmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)StubCmdCopyMemoryIndirectNV;
    }
    table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)gpa(device, "vkCmdCopyMemoryToImageIndirectNV");
    if (table->CmdCopyMemoryToImageIndirectNV == nullptr) {
        table->CmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)StubCmdCopyMemoryToImageIndirectNV;
    }
    table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)gpa(device, "vkCmdDecompressMemoryNV");
    if (table->CmdDecompressMemoryNV == nullptr) {
        table->CmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)StubCmdDecompressMemoryNV;
    }
    table->CmdDecompressMemoryIndirectCountNV =
        (PFN_vkCmdDecompressMemoryIndirectCountNV)gpa(device, "vkCmdDecompressMemoryIndirectCountNV");
    if (table->CmdDecompressMemoryIndirectCountNV == nullptr) {
        table->CmdDecompressMemoryIndirectCountNV =
            (PFN_vkCmdDecompressMemoryIndirectCountNV)StubCmdDecompressMemoryIndirectCountNV;
    }
    table->GetPipelineIndirectMemoryRequirementsNV =
        (PFN_vkGetPipelineIndirectMemoryRequirementsNV)gpa(device, "vkGetPipelineIndirectMemoryRequirementsNV");
    if (table->GetPipelineIndirectMemoryRequirementsNV == nullptr) {
        table->GetPipelineIndirectMemoryRequirementsNV =
            (PFN_vkGetPipelineIndirectMemoryRequirementsNV)StubGetPipelineIndirectMemoryRequirementsNV;
    }
    table->CmdUpdatePipelineIndirectBufferNV =
        (PFN_vkCmdUpdatePipelineIndirectBufferNV)gpa(device, "vkCmdUpdatePipelineIndirectBufferNV");
    if (table->CmdUpdatePipelineIndirectBufferNV == nullptr) {
        table->CmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV)StubCmdUpdatePipelineIndirectBufferNV;
    }
    table->GetPipelineIndirectDeviceAddressNV =
        (PFN_vkGetPipelineIndirectDeviceAddressNV)gpa(device, "vkGetPipelineIndirectDeviceAddressNV");
    if (table->GetPipelineIndirectDeviceAddressNV == nullptr) {
        table->GetPipelineIndirectDeviceAddressNV =
            (PFN_vkGetPipelineIndirectDeviceAddressNV)StubGetPipelineIndirectDeviceAddressNV;
    }
    table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)gpa(device, "vkCmdSetDepthClampEnableEXT");
    if (table->CmdSetDepthClampEnableEXT == nullptr) {
        table->CmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)StubCmdSetDepthClampEnableEXT;
    }
    table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)gpa(device, "vkCmdSetPolygonModeEXT");
    if (table->CmdSetPolygonModeEXT == nullptr) {
        table->CmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)StubCmdSetPolygonModeEXT;
    }
    table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)gpa(device, "vkCmdSetRasterizationSamplesEXT");
    if (table->CmdSetRasterizationSamplesEXT == nullptr) {
        table->CmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)StubCmdSetRasterizationSamplesEXT;
    }
    table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)gpa(device, "vkCmdSetSampleMaskEXT");
    if (table->CmdSetSampleMaskEXT == nullptr) {
        table->CmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)StubCmdSetSampleMaskEXT;
    }
    table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)gpa(device, "vkCmdSetAlphaToCoverageEnableEXT");
    if (table->CmdSetAlphaToCoverageEnableEXT == nullptr) {
        table->CmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)StubCmdSetAlphaToCoverageEnableEXT;
    }
    table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)gpa(device, "vkCmdSetAlphaToOneEnableEXT");
    if (table->CmdSetAlphaToOneEnableEXT == nullptr) {
        table->CmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)StubCmdSetAlphaToOneEnableEXT;
    }
    table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)gpa(device, "vkCmdSetLogicOpEnableEXT");
    if (table->CmdSetLogicOpEnableEXT == nullptr) {
        table->CmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)StubCmdSetLogicOpEnableEXT;
    }
    table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)gpa(device, "vkCmdSetColorBlendEnableEXT");
    if (table->CmdSetColorBlendEnableEXT == nullptr) {
        table->CmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)StubCmdSetColorBlendEnableEXT;
    }
    table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)gpa(device, "vkCmdSetColorBlendEquationEXT");
    if (table->CmdSetColorBlendEquationEXT == nullptr) {
        table->CmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)StubCmdSetColorBlendEquationEXT;
    }
    table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)gpa(device, "vkCmdSetColorWriteMaskEXT");
    if (table->CmdSetColorWriteMaskEXT == nullptr) {
        table->CmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)StubCmdSetColorWriteMaskEXT;
    }
    table->CmdSetTessellationDomainOriginEXT =
        (PFN_vkCmdSetTessellationDomainOriginEXT)gpa(device, "vkCmdSetTessellationDomainOriginEXT");
    if (table->CmdSetTessellationDomainOriginEXT == nullptr) {
        table->CmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)StubCmdSetTessellationDomainOriginEXT;
    }
    table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)gpa(device, "vkCmdSetRasterizationStreamEXT");
    if (table->CmdSetRasterizationStreamEXT == nullptr) {
        table->CmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)StubCmdSetRasterizationStreamEXT;
    }
    table->CmdSetConservativeRasterizationModeEXT =
        (PFN_vkCmdSetConservativeRasterizationModeEXT)gpa(device, "vkCmdSetConservativeRasterizationModeEXT");
    if (table->CmdSetConservativeRasterizationModeEXT == nullptr) {
        table->CmdSetConservativeRasterizationModeEXT =
            (PFN_vkCmdSetConservativeRasterizationModeEXT)StubCmdSetConservativeRasterizationModeEXT;
    }
    table->CmdSetExtraPrimitiveOverestimationSizeEXT =
        (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)gpa(device, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    if (table->CmdSetExtraPrimitiveOverestimationSizeEXT == nullptr) {
        table->CmdSetExtraPrimitiveOverestimationSizeEXT =
            (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)StubCmdSetExtraPrimitiveOverestimationSizeEXT;
    }
    table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)gpa(device, "vkCmdSetDepthClipEnableEXT");
    if (table->CmdSetDepthClipEnableEXT == nullptr) {
        table->CmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)StubCmdSetDepthClipEnableEXT;
    }
    table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)gpa(device, "vkCmdSetSampleLocationsEnableEXT");
    if (table->CmdSetSampleLocationsEnableEXT == nullptr) {
        table->CmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)StubCmdSetSampleLocationsEnableEXT;
    }
    table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)gpa(device, "vkCmdSetColorBlendAdvancedEXT");
    if (table->CmdSetColorBlendAdvancedEXT == nullptr) {
        table->CmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)StubCmdSetColorBlendAdvancedEXT;
    }
    table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)gpa(device, "vkCmdSetProvokingVertexModeEXT");
    if (table->CmdSetProvokingVertexModeEXT == nullptr) {
        table->CmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)StubCmdSetProvokingVertexModeEXT;
    }
    table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)gpa(device, "vkCmdSetLineRasterizationModeEXT");
    if (table->CmdSetLineRasterizationModeEXT == nullptr) {
        table->CmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)StubCmdSetLineRasterizationModeEXT;
    }
    table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)gpa(device, "vkCmdSetLineStippleEnableEXT");
    if (table->CmdSetLineStippleEnableEXT == nullptr) {
        table->CmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)StubCmdSetLineStippleEnableEXT;
    }
    table->CmdSetDepthClipNegativeOneToOneEXT =
        (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)gpa(device, "vkCmdSetDepthClipNegativeOneToOneEXT");
    if (table->CmdSetDepthClipNegativeOneToOneEXT == nullptr) {
        table->CmdSetDepthClipNegativeOneToOneEXT =
            (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)StubCmdSetDepthClipNegativeOneToOneEXT;
    }
    table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)gpa(device, "vkCmdSetViewportWScalingEnableNV");
    if (table->CmdSetViewportWScalingEnableNV == nullptr) {
        table->CmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)StubCmdSetViewportWScalingEnableNV;
    }
    table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)gpa(device, "vkCmdSetViewportSwizzleNV");
    if (table->CmdSetViewportSwizzleNV == nullptr) {
        table->CmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)StubCmdSetViewportSwizzleNV;
    }
    table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)gpa(device, "vkCmdSetCoverageToColorEnableNV");
    if (table->CmdSetCoverageToColorEnableNV == nullptr) {
        table->CmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)StubCmdSetCoverageToColorEnableNV;
    }
    table->CmdSetCoverageToColorLocationNV =
        (PFN_vkCmdSetCoverageToColorLocationNV)gpa(device, "vkCmdSetCoverageToColorLocationNV");
    if (table->CmdSetCoverageToColorLocationNV == nullptr) {
        table->CmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV)StubCmdSetCoverageToColorLocationNV;
    }
    table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)gpa(device, "vkCmdSetCoverageModulationModeNV");
    if (table->CmdSetCoverageModulationModeNV == nullptr) {
        table->CmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)StubCmdSetCoverageModulationModeNV;
    }
    table->CmdSetCoverageModulationTableEnableNV =
        (PFN_vkCmdSetCoverageModulationTableEnableNV)gpa(device, "vkCmdSetCoverageModulationTableEnableNV");
    if (table->CmdSetCoverageModulationTableEnableNV == nullptr) {
        table->CmdSetCoverageModulationTableEnableNV =
            (PFN_vkCmdSetCoverageModulationTableEnableNV)StubCmdSetCoverageModulationTableEnableNV;
    }
    table->CmdSetCoverageModulationTableNV =
        (PFN_vkCmdSetCoverageModulationTableNV)gpa(device, "vkCmdSetCoverageModulationTableNV");
    if (table->CmdSetCoverageModulationTableNV == nullptr) {
        table->CmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV)StubCmdSetCoverageModulationTableNV;
    }
    table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)gpa(device, "vkCmdSetShadingRateImageEnableNV");
    if (table->CmdSetShadingRateImageEnableNV == nullptr) {
        table->CmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)StubCmdSetShadingRateImageEnableNV;
    }
    table->CmdSetRepresentativeFragmentTestEnableNV =
        (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)gpa(device, "vkCmdSetRepresentativeFragmentTestEnableNV");
    if (table->CmdSetRepresentativeFragmentTestEnableNV == nullptr) {
        table->CmdSetRepresentativeFragmentTestEnableNV =
            (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)StubCmdSetRepresentativeFragmentTestEnableNV;
    }
    table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)gpa(device, "vkCmdSetCoverageReductionModeNV");
    if (table->CmdSetCoverageReductionModeNV == nullptr) {
        table->CmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)StubCmdSetCoverageReductionModeNV;
    }
    table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)gpa(device, "vkGetShaderModuleIdentifierEXT");
    if (table->GetShaderModuleIdentifierEXT == nullptr) {
        table->GetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)StubGetShaderModuleIdentifierEXT;
    }
    table->GetShaderModuleCreateInfoIdentifierEXT =
        (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)gpa(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
    if (table->GetShaderModuleCreateInfoIdentifierEXT == nullptr) {
        table->GetShaderModuleCreateInfoIdentifierEXT =
            (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)StubGetShaderModuleCreateInfoIdentifierEXT;
    }
    table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)gpa(device, "vkCreateOpticalFlowSessionNV");
    if (table->CreateOpticalFlowSessionNV == nullptr) {
        table->CreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)StubCreateOpticalFlowSessionNV;
    }
    table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)gpa(device, "vkDestroyOpticalFlowSessionNV");
    if (table->DestroyOpticalFlowSessionNV == nullptr) {
        table->DestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)StubDestroyOpticalFlowSessionNV;
    }
    table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)gpa(device, "vkBindOpticalFlowSessionImageNV");
    if (table->BindOpticalFlowSessionImageNV == nullptr) {
        table->BindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)StubBindOpticalFlowSessionImageNV;
    }
    table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)gpa(device, "vkCmdOpticalFlowExecuteNV");
    if (table->CmdOpticalFlowExecuteNV == nullptr) {
        table->CmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)StubCmdOpticalFlowExecuteNV;
    }
    table->AntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD)gpa(device, "vkAntiLagUpdateAMD");
    if (table->AntiLagUpdateAMD == nullptr) {
        table->AntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD)StubAntiLagUpdateAMD;
    }
    table->CreateShadersEXT = (PFN_vkCreateShadersEXT)gpa(device, "vkCreateShadersEXT");
    if (table->CreateShadersEXT == nullptr) {
        table->CreateShadersEXT = (PFN_vkCreateShadersEXT)StubCreateShadersEXT;
    }
    table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)gpa(device, "vkDestroyShaderEXT");
    if (table->DestroyShaderEXT == nullptr) {
        table->DestroyShaderEXT = (PFN_vkDestroyShaderEXT)StubDestroyShaderEXT;
    }
    table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)gpa(device, "vkGetShaderBinaryDataEXT");
    if (table->GetShaderBinaryDataEXT == nullptr) {
        table->GetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)StubGetShaderBinaryDataEXT;
    }
    table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)gpa(device, "vkCmdBindShadersEXT");
    if (table->CmdBindShadersEXT == nullptr) {
        table->CmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)StubCmdBindShadersEXT;
    }
    table->CmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT)gpa(device, "vkCmdSetDepthClampRangeEXT");
    if (table->CmdSetDepthClampRangeEXT == nullptr) {
        table->CmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT)StubCmdSetDepthClampRangeEXT;
    }
    table->GetFramebufferTilePropertiesQCOM =
        (PFN_vkGetFramebufferTilePropertiesQCOM)gpa(device, "vkGetFramebufferTilePropertiesQCOM");
    if (table->GetFramebufferTilePropertiesQCOM == nullptr) {
        table->GetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM)StubGetFramebufferTilePropertiesQCOM;
    }
    table->GetDynamicRenderingTilePropertiesQCOM =
        (PFN_vkGetDynamicRenderingTilePropertiesQCOM)gpa(device, "vkGetDynamicRenderingTilePropertiesQCOM");
    if (table->GetDynamicRenderingTilePropertiesQCOM == nullptr) {
        table->GetDynamicRenderingTilePropertiesQCOM =
            (PFN_vkGetDynamicRenderingTilePropertiesQCOM)StubGetDynamicRenderingTilePropertiesQCOM;
    }
    table->ConvertCooperativeVectorMatrixNV =
        (PFN_vkConvertCooperativeVectorMatrixNV)gpa(device, "vkConvertCooperativeVectorMatrixNV");
    if (table->ConvertCooperativeVectorMatrixNV == nullptr) {
        table->ConvertCooperativeVectorMatrixNV = (PFN_vkConvertCooperativeVectorMatrixNV)StubConvertCooperativeVectorMatrixNV;
    }
    table->CmdConvertCooperativeVectorMatrixNV =
        (PFN_vkCmdConvertCooperativeVectorMatrixNV)gpa(device, "vkCmdConvertCooperativeVectorMatrixNV");
    if (table->CmdConvertCooperativeVectorMatrixNV == nullptr) {
        table->CmdConvertCooperativeVectorMatrixNV =
            (PFN_vkCmdConvertCooperativeVectorMatrixNV)StubCmdConvertCooperativeVectorMatrixNV;
    }
    table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)gpa(device, "vkSetLatencySleepModeNV");
    if (table->SetLatencySleepModeNV == nullptr) {
        table->SetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)StubSetLatencySleepModeNV;
    }
    table->LatencySleepNV = (PFN_vkLatencySleepNV)gpa(device, "vkLatencySleepNV");
    if (table->LatencySleepNV == nullptr) {
        table->LatencySleepNV = (PFN_vkLatencySleepNV)StubLatencySleepNV;
    }
    table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)gpa(device, "vkSetLatencyMarkerNV");
    if (table->SetLatencyMarkerNV == nullptr) {
        table->SetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)StubSetLatencyMarkerNV;
    }
    table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)gpa(device, "vkGetLatencyTimingsNV");
    if (table->GetLatencyTimingsNV == nullptr) {
        table->GetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)StubGetLatencyTimingsNV;
    }
    table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)gpa(device, "vkQueueNotifyOutOfBandNV");
    if (table->QueueNotifyOutOfBandNV == nullptr) {
        table->QueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)StubQueueNotifyOutOfBandNV;
    }
    table->CmdSetAttachmentFeedbackLoopEnableEXT =
        (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)gpa(device, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
    if (table->CmdSetAttachmentFeedbackLoopEnableEXT == nullptr) {
        table->CmdSetAttachmentFeedbackLoopEnableEXT =
            (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)StubCmdSetAttachmentFeedbackLoopEnableEXT;
    }
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)gpa(device, "vkGetScreenBufferPropertiesQNX");
    if (table->GetScreenBufferPropertiesQNX == nullptr) {
        table->GetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)StubGetScreenBufferPropertiesQNX;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->GetClusterAccelerationStructureBuildSizesNV =
        (PFN_vkGetClusterAccelerationStructureBuildSizesNV)gpa(device, "vkGetClusterAccelerationStructureBuildSizesNV");
    if (table->GetClusterAccelerationStructureBuildSizesNV == nullptr) {
        table->GetClusterAccelerationStructureBuildSizesNV =
            (PFN_vkGetClusterAccelerationStructureBuildSizesNV)StubGetClusterAccelerationStructureBuildSizesNV;
    }
    table->CmdBuildClusterAccelerationStructureIndirectNV =
        (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV)gpa(device, "vkCmdBuildClusterAccelerationStructureIndirectNV");
    if (table->CmdBuildClusterAccelerationStructureIndirectNV == nullptr) {
        table->CmdBuildClusterAccelerationStructureIndirectNV =
            (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV)StubCmdBuildClusterAccelerationStructureIndirectNV;
    }
    table->GetPartitionedAccelerationStructuresBuildSizesNV =
        (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV)gpa(device, "vkGetPartitionedAccelerationStructuresBuildSizesNV");
    if (table->GetPartitionedAccelerationStructuresBuildSizesNV == nullptr) {
        table->GetPartitionedAccelerationStructuresBuildSizesNV =
            (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV)StubGetPartitionedAccelerationStructuresBuildSizesNV;
    }
    table->CmdBuildPartitionedAccelerationStructuresNV =
        (PFN_vkCmdBuildPartitionedAccelerationStructuresNV)gpa(device, "vkCmdBuildPartitionedAccelerationStructuresNV");
    if (table->CmdBuildPartitionedAccelerationStructuresNV == nullptr) {
        table->CmdBuildPartitionedAccelerationStructuresNV =
            (PFN_vkCmdBuildPartitionedAccelerationStructuresNV)StubCmdBuildPartitionedAccelerationStructuresNV;
    }
    table->GetGeneratedCommandsMemoryRequirementsEXT =
        (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT)gpa(device, "vkGetGeneratedCommandsMemoryRequirementsEXT");
    if (table->GetGeneratedCommandsMemoryRequirementsEXT == nullptr) {
        table->GetGeneratedCommandsMemoryRequirementsEXT =
            (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT)StubGetGeneratedCommandsMemoryRequirementsEXT;
    }
    table->CmdPreprocessGeneratedCommandsEXT =
        (PFN_vkCmdPreprocessGeneratedCommandsEXT)gpa(device, "vkCmdPreprocessGeneratedCommandsEXT");
    if (table->CmdPreprocessGeneratedCommandsEXT == nullptr) {
        table->CmdPreprocessGeneratedCommandsEXT = (PFN_vkCmdPreprocessGeneratedCommandsEXT)StubCmdPreprocessGeneratedCommandsEXT;
    }
    table->CmdExecuteGeneratedCommandsEXT = (PFN_vkCmdExecuteGeneratedCommandsEXT)gpa(device, "vkCmdExecuteGeneratedCommandsEXT");
    if (table->CmdExecuteGeneratedCommandsEXT == nullptr) {
        table->CmdExecuteGeneratedCommandsEXT = (PFN_vkCmdExecuteGeneratedCommandsEXT)StubCmdExecuteGeneratedCommandsEXT;
    }
    table->CreateIndirectCommandsLayoutEXT =
        (PFN_vkCreateIndirectCommandsLayoutEXT)gpa(device, "vkCreateIndirectCommandsLayoutEXT");
    if (table->CreateIndirectCommandsLayoutEXT == nullptr) {
        table->CreateIndirectCommandsLayoutEXT = (PFN_vkCreateIndirectCommandsLayoutEXT)StubCreateIndirectCommandsLayoutEXT;
    }
    table->DestroyIndirectCommandsLayoutEXT =
        (PFN_vkDestroyIndirectCommandsLayoutEXT)gpa(device, "vkDestroyIndirectCommandsLayoutEXT");
    if (table->DestroyIndirectCommandsLayoutEXT == nullptr) {
        table->DestroyIndirectCommandsLayoutEXT = (PFN_vkDestroyIndirectCommandsLayoutEXT)StubDestroyIndirectCommandsLayoutEXT;
    }
    table->CreateIndirectExecutionSetEXT = (PFN_vkCreateIndirectExecutionSetEXT)gpa(device, "vkCreateIndirectExecutionSetEXT");
    if (table->CreateIndirectExecutionSetEXT == nullptr) {
        table->CreateIndirectExecutionSetEXT = (PFN_vkCreateIndirectExecutionSetEXT)StubCreateIndirectExecutionSetEXT;
    }
    table->DestroyIndirectExecutionSetEXT = (PFN_vkDestroyIndirectExecutionSetEXT)gpa(device, "vkDestroyIndirectExecutionSetEXT");
    if (table->DestroyIndirectExecutionSetEXT == nullptr) {
        table->DestroyIndirectExecutionSetEXT = (PFN_vkDestroyIndirectExecutionSetEXT)StubDestroyIndirectExecutionSetEXT;
    }
    table->UpdateIndirectExecutionSetPipelineEXT =
        (PFN_vkUpdateIndirectExecutionSetPipelineEXT)gpa(device, "vkUpdateIndirectExecutionSetPipelineEXT");
    if (table->UpdateIndirectExecutionSetPipelineEXT == nullptr) {
        table->UpdateIndirectExecutionSetPipelineEXT =
            (PFN_vkUpdateIndirectExecutionSetPipelineEXT)StubUpdateIndirectExecutionSetPipelineEXT;
    }
    table->UpdateIndirectExecutionSetShaderEXT =
        (PFN_vkUpdateIndirectExecutionSetShaderEXT)gpa(device, "vkUpdateIndirectExecutionSetShaderEXT");
    if (table->UpdateIndirectExecutionSetShaderEXT == nullptr) {
        table->UpdateIndirectExecutionSetShaderEXT =
            (PFN_vkUpdateIndirectExecutionSetShaderEXT)StubUpdateIndirectExecutionSetShaderEXT;
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->GetMemoryMetalHandleEXT = (PFN_vkGetMemoryMetalHandleEXT)gpa(device, "vkGetMemoryMetalHandleEXT");
    if (table->GetMemoryMetalHandleEXT == nullptr) {
        table->GetMemoryMetalHandleEXT = (PFN_vkGetMemoryMetalHandleEXT)StubGetMemoryMetalHandleEXT;
    }
    table->GetMemoryMetalHandlePropertiesEXT =
        (PFN_vkGetMemoryMetalHandlePropertiesEXT)gpa(device, "vkGetMemoryMetalHandlePropertiesEXT");
    if (table->GetMemoryMetalHandlePropertiesEXT == nullptr) {
        table->GetMemoryMetalHandlePropertiesEXT = (PFN_vkGetMemoryMetalHandlePropertiesEXT)StubGetMemoryMetalHandlePropertiesEXT;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)gpa(device, "vkCreateAccelerationStructureKHR");
    if (table->CreateAccelerationStructureKHR == nullptr) {
        table->CreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)StubCreateAccelerationStructureKHR;
    }
    table->DestroyAccelerationStructureKHR =
        (PFN_vkDestroyAccelerationStructureKHR)gpa(device, "vkDestroyAccelerationStructureKHR");
    if (table->DestroyAccelerationStructureKHR == nullptr) {
        table->DestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)StubDestroyAccelerationStructureKHR;
    }
    table->CmdBuildAccelerationStructuresKHR =
        (PFN_vkCmdBuildAccelerationStructuresKHR)gpa(device, "vkCmdBuildAccelerationStructuresKHR");
    if (table->CmdBuildAccelerationStructuresKHR == nullptr) {
        table->CmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)StubCmdBuildAccelerationStructuresKHR;
    }
    table->CmdBuildAccelerationStructuresIndirectKHR =
        (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)gpa(device, "vkCmdBuildAccelerationStructuresIndirectKHR");
    if (table->CmdBuildAccelerationStructuresIndirectKHR == nullptr) {
        table->CmdBuildAccelerationStructuresIndirectKHR =
            (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)StubCmdBuildAccelerationStructuresIndirectKHR;
    }
    table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)gpa(device, "vkBuildAccelerationStructuresKHR");
    if (table->BuildAccelerationStructuresKHR == nullptr) {
        table->BuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)StubBuildAccelerationStructuresKHR;
    }
    table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)gpa(device, "vkCopyAccelerationStructureKHR");
    if (table->CopyAccelerationStructureKHR == nullptr) {
        table->CopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)StubCopyAccelerationStructureKHR;
    }
    table->CopyAccelerationStructureToMemoryKHR =
        (PFN_vkCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCopyAccelerationStructureToMemoryKHR");
    if (table->CopyAccelerationStructureToMemoryKHR == nullptr) {
        table->CopyAccelerationStructureToMemoryKHR =
            (PFN_vkCopyAccelerationStructureToMemoryKHR)StubCopyAccelerationStructureToMemoryKHR;
    }
    table->CopyMemoryToAccelerationStructureKHR =
        (PFN_vkCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCopyMemoryToAccelerationStructureKHR");
    if (table->CopyMemoryToAccelerationStructureKHR == nullptr) {
        table->CopyMemoryToAccelerationStructureKHR =
            (PFN_vkCopyMemoryToAccelerationStructureKHR)StubCopyMemoryToAccelerationStructureKHR;
    }
    table->WriteAccelerationStructuresPropertiesKHR =
        (PFN_vkWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkWriteAccelerationStructuresPropertiesKHR");
    if (table->WriteAccelerationStructuresPropertiesKHR == nullptr) {
        table->WriteAccelerationStructuresPropertiesKHR =
            (PFN_vkWriteAccelerationStructuresPropertiesKHR)StubWriteAccelerationStructuresPropertiesKHR;
    }
    table->CmdCopyAccelerationStructureKHR =
        (PFN_vkCmdCopyAccelerationStructureKHR)gpa(device, "vkCmdCopyAccelerationStructureKHR");
    if (table->CmdCopyAccelerationStructureKHR == nullptr) {
        table->CmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)StubCmdCopyAccelerationStructureKHR;
    }
    table->CmdCopyAccelerationStructureToMemoryKHR =
        (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)gpa(device, "vkCmdCopyAccelerationStructureToMemoryKHR");
    if (table->CmdCopyAccelerationStructureToMemoryKHR == nullptr) {
        table->CmdCopyAccelerationStructureToMemoryKHR =
            (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)StubCmdCopyAccelerationStructureToMemoryKHR;
    }
    table->CmdCopyMemoryToAccelerationStructureKHR =
        (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)gpa(device, "vkCmdCopyMemoryToAccelerationStructureKHR");
    if (table->CmdCopyMemoryToAccelerationStructureKHR == nullptr) {
        table->CmdCopyMemoryToAccelerationStructureKHR =
            (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)StubCmdCopyMemoryToAccelerationStructureKHR;
    }
    table->GetAccelerationStructureDeviceAddressKHR =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)gpa(device, "vkGetAccelerationStructureDeviceAddressKHR");
    if (table->GetAccelerationStructureDeviceAddressKHR == nullptr) {
        table->GetAccelerationStructureDeviceAddressKHR =
            (PFN_vkGetAccelerationStructureDeviceAddressKHR)StubGetAccelerationStructureDeviceAddressKHR;
    }
    table->CmdWriteAccelerationStructuresPropertiesKHR =
        (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)gpa(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    if (table->CmdWriteAccelerationStructuresPropertiesKHR == nullptr) {
        table->CmdWriteAccelerationStructuresPropertiesKHR =
            (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)StubCmdWriteAccelerationStructuresPropertiesKHR;
    }
    table->GetDeviceAccelerationStructureCompatibilityKHR =
        (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)gpa(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    if (table->GetDeviceAccelerationStructureCompatibilityKHR == nullptr) {
        table->GetDeviceAccelerationStructureCompatibilityKHR =
            (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)StubGetDeviceAccelerationStructureCompatibilityKHR;
    }
    table->GetAccelerationStructureBuildSizesKHR =
        (PFN_vkGetAccelerationStructureBuildSizesKHR)gpa(device, "vkGetAccelerationStructureBuildSizesKHR");
    if (table->GetAccelerationStructureBuildSizesKHR == nullptr) {
        table->GetAccelerationStructureBuildSizesKHR =
            (PFN_vkGetAccelerationStructureBuildSizesKHR)StubGetAccelerationStructureBuildSizesKHR;
    }
    table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)gpa(device, "vkCmdTraceRaysKHR");
    if (table->CmdTraceRaysKHR == nullptr) {
        table->CmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)StubCmdTraceRaysKHR;
    }
    table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)gpa(device, "vkCreateRayTracingPipelinesKHR");
    if (table->CreateRayTracingPipelinesKHR == nullptr) {
        table->CreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)StubCreateRayTracingPipelinesKHR;
    }
    table->GetRayTracingCaptureReplayShaderGroupHandlesKHR =
        (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)gpa(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    if (table->GetRayTracingCaptureReplayShaderGroupHandlesKHR == nullptr) {
        table->GetRayTracingCaptureReplayShaderGroupHandlesKHR =
            (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)StubGetRayTracingCaptureReplayShaderGroupHandlesKHR;
    }
    table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)gpa(device, "vkCmdTraceRaysIndirectKHR");
    if (table->CmdTraceRaysIndirectKHR == nullptr) {
        table->CmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)StubCmdTraceRaysIndirectKHR;
    }
    table->GetRayTracingShaderGroupStackSizeKHR =
        (PFN_vkGetRayTracingShaderGroupStackSizeKHR)gpa(device, "vkGetRayTracingShaderGroupStackSizeKHR");
    if (table->GetRayTracingShaderGroupStackSizeKHR == nullptr) {
        table->GetRayTracingShaderGroupStackSizeKHR =
            (PFN_vkGetRayTracingShaderGroupStackSizeKHR)StubGetRayTracingShaderGroupStackSizeKHR;
    }
    table->CmdSetRayTracingPipelineStackSizeKHR =
        (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)gpa(device, "vkCmdSetRayTracingPipelineStackSizeKHR");
    if (table->CmdSetRayTracingPipelineStackSizeKHR == nullptr) {
        table->CmdSetRayTracingPipelineStackSizeKHR =
            (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)StubCmdSetRayTracingPipelineStackSizeKHR;
    }
    table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)gpa(device, "vkCmdDrawMeshTasksEXT");
    if (table->CmdDrawMeshTasksEXT == nullptr) {
        table->CmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)StubCmdDrawMeshTasksEXT;
    }
    table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)gpa(device, "vkCmdDrawMeshTasksIndirectEXT");
    if (table->CmdDrawMeshTasksIndirectEXT == nullptr) {
        table->CmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)StubCmdDrawMeshTasksIndirectEXT;
    }
    table->CmdDrawMeshTasksIndirectCountEXT =
        (PFN_vkCmdDrawMeshTasksIndirectCountEXT)gpa(device, "vkCmdDrawMeshTasksIndirectCountEXT");
    if (table->CmdDrawMeshTasksIndirectCountEXT == nullptr) {
        table->CmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)StubCmdDrawMeshTasksIndirectCountEXT;
    }
}

void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table, PFN_vkGetInstanceProcAddr gpa) {
    memset(table, 0, sizeof(*table));
    // Instance function pointers
    table->GetInstanceProcAddr = gpa;
    table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr)gpa(instance, "vk_layerGetPhysicalDeviceProcAddr");
    table->DestroyInstance = (PFN_vkDestroyInstance)gpa(instance, "vkDestroyInstance");
    table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)gpa(instance, "vkEnumeratePhysicalDevices");
    table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)gpa(instance, "vkGetPhysicalDeviceFeatures");
    table->GetPhysicalDeviceFormatProperties =
        (PFN_vkGetPhysicalDeviceFormatProperties)gpa(instance, "vkGetPhysicalDeviceFormatProperties");
    table->GetPhysicalDeviceImageFormatProperties =
        (PFN_vkGetPhysicalDeviceImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
    table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)gpa(instance, "vkGetPhysicalDeviceProperties");
    table->GetPhysicalDeviceQueueFamilyProperties =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    table->GetPhysicalDeviceMemoryProperties =
        (PFN_vkGetPhysicalDeviceMemoryProperties)gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
    table->EnumerateDeviceExtensionProperties =
        (PFN_vkEnumerateDeviceExtensionProperties)gpa(instance, "vkEnumerateDeviceExtensionProperties");
    table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)gpa(instance, "vkEnumerateDeviceLayerProperties");
    table->GetPhysicalDeviceSparseImageFormatProperties =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
    table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)gpa(instance, "vkEnumeratePhysicalDeviceGroups");
    if (table->EnumeratePhysicalDeviceGroups == nullptr) {
        table->EnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)StubEnumeratePhysicalDeviceGroups;
    }
    table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)gpa(instance, "vkGetPhysicalDeviceFeatures2");
    if (table->GetPhysicalDeviceFeatures2 == nullptr) {
        table->GetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)StubGetPhysicalDeviceFeatures2;
    }
    table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)gpa(instance, "vkGetPhysicalDeviceProperties2");
    if (table->GetPhysicalDeviceProperties2 == nullptr) {
        table->GetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)StubGetPhysicalDeviceProperties2;
    }
    table->GetPhysicalDeviceFormatProperties2 =
        (PFN_vkGetPhysicalDeviceFormatProperties2)gpa(instance, "vkGetPhysicalDeviceFormatProperties2");
    if (table->GetPhysicalDeviceFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceFormatProperties2 =
            (PFN_vkGetPhysicalDeviceFormatProperties2)StubGetPhysicalDeviceFormatProperties2;
    }
    table->GetPhysicalDeviceImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    if (table->GetPhysicalDeviceImageFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceImageFormatProperties2 =
            (PFN_vkGetPhysicalDeviceImageFormatProperties2)StubGetPhysicalDeviceImageFormatProperties2;
    }
    table->GetPhysicalDeviceQueueFamilyProperties2 =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (table->GetPhysicalDeviceQueueFamilyProperties2 == nullptr) {
        table->GetPhysicalDeviceQueueFamilyProperties2 =
            (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)StubGetPhysicalDeviceQueueFamilyProperties2;
    }
    table->GetPhysicalDeviceMemoryProperties2 =
        (PFN_vkGetPhysicalDeviceMemoryProperties2)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2");
    if (table->GetPhysicalDeviceMemoryProperties2 == nullptr) {
        table->GetPhysicalDeviceMemoryProperties2 =
            (PFN_vkGetPhysicalDeviceMemoryProperties2)StubGetPhysicalDeviceMemoryProperties2;
    }
    table->GetPhysicalDeviceSparseImageFormatProperties2 =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    if (table->GetPhysicalDeviceSparseImageFormatProperties2 == nullptr) {
        table->GetPhysicalDeviceSparseImageFormatProperties2 =
            (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)StubGetPhysicalDeviceSparseImageFormatProperties2;
    }
    table->GetPhysicalDeviceExternalBufferProperties =
        (PFN_vkGetPhysicalDeviceExternalBufferProperties)gpa(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    if (table->GetPhysicalDeviceExternalBufferProperties == nullptr) {
        table->GetPhysicalDeviceExternalBufferProperties =
            (PFN_vkGetPhysicalDeviceExternalBufferProperties)StubGetPhysicalDeviceExternalBufferProperties;
    }
    table->GetPhysicalDeviceExternalFenceProperties =
        (PFN_vkGetPhysicalDeviceExternalFenceProperties)gpa(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    if (table->GetPhysicalDeviceExternalFenceProperties == nullptr) {
        table->GetPhysicalDeviceExternalFenceProperties =
            (PFN_vkGetPhysicalDeviceExternalFenceProperties)StubGetPhysicalDeviceExternalFenceProperties;
    }
    table->GetPhysicalDeviceExternalSemaphoreProperties =
        (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)gpa(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    if (table->GetPhysicalDeviceExternalSemaphoreProperties == nullptr) {
        table->GetPhysicalDeviceExternalSemaphoreProperties =
            (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)StubGetPhysicalDeviceExternalSemaphoreProperties;
    }
    table->GetPhysicalDeviceToolProperties =
        (PFN_vkGetPhysicalDeviceToolProperties)gpa(instance, "vkGetPhysicalDeviceToolProperties");
    if (table->GetPhysicalDeviceToolProperties == nullptr) {
        table->GetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)StubGetPhysicalDeviceToolProperties;
    }
    table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)gpa(instance, "vkDestroySurfaceKHR");
    if (table->DestroySurfaceKHR == nullptr) {
        table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)StubDestroySurfaceKHR;
    }
    table->GetPhysicalDeviceSurfaceSupportKHR =
        (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    if (table->GetPhysicalDeviceSurfaceSupportKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceSupportKHR =
            (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)StubGetPhysicalDeviceSurfaceSupportKHR;
    }
    table->GetPhysicalDeviceSurfaceCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    if (table->GetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilitiesKHR =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)StubGetPhysicalDeviceSurfaceCapabilitiesKHR;
    }
    table->GetPhysicalDeviceSurfaceFormatsKHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    if (table->GetPhysicalDeviceSurfaceFormatsKHR == nullptr) {
        table->GetPhysicalDeviceSurfaceFormatsKHR =
            (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)StubGetPhysicalDeviceSurfaceFormatsKHR;
    }
    table->GetPhysicalDeviceSurfacePresentModesKHR =
        (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    if (table->GetPhysicalDeviceSurfacePresentModesKHR == nullptr) {
        table->GetPhysicalDeviceSurfacePresentModesKHR =
            (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)StubGetPhysicalDeviceSurfacePresentModesKHR;
    }
    table->GetPhysicalDevicePresentRectanglesKHR =
        (PFN_vkGetPhysicalDevicePresentRectanglesKHR)gpa(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
    if (table->GetPhysicalDevicePresentRectanglesKHR == nullptr) {
        table->GetPhysicalDevicePresentRectanglesKHR =
            (PFN_vkGetPhysicalDevicePresentRectanglesKHR)StubGetPhysicalDevicePresentRectanglesKHR;
    }
    table->GetPhysicalDeviceDisplayPropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceDisplayPropertiesKHR =
            (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)StubGetPhysicalDeviceDisplayPropertiesKHR;
    }
    table->GetPhysicalDeviceDisplayPlanePropertiesKHR =
        (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    if (table->GetPhysicalDeviceDisplayPlanePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceDisplayPlanePropertiesKHR =
            (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)StubGetPhysicalDeviceDisplayPlanePropertiesKHR;
    }
    table->GetDisplayPlaneSupportedDisplaysKHR =
        (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    if (table->GetDisplayPlaneSupportedDisplaysKHR == nullptr) {
        table->GetDisplayPlaneSupportedDisplaysKHR =
            (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)StubGetDisplayPlaneSupportedDisplaysKHR;
    }
    table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)gpa(instance, "vkGetDisplayModePropertiesKHR");
    if (table->GetDisplayModePropertiesKHR == nullptr) {
        table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)StubGetDisplayModePropertiesKHR;
    }
    table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)gpa(instance, "vkCreateDisplayModeKHR");
    if (table->CreateDisplayModeKHR == nullptr) {
        table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)StubCreateDisplayModeKHR;
    }
    table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    if (table->GetDisplayPlaneCapabilitiesKHR == nullptr) {
        table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)StubGetDisplayPlaneCapabilitiesKHR;
    }
    table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");
    if (table->CreateDisplayPlaneSurfaceKHR == nullptr) {
        table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)StubCreateDisplayPlaneSurfaceKHR;
    }
#ifdef VK_USE_PLATFORM_XLIB_KHR
    table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)gpa(instance, "vkCreateXlibSurfaceKHR");
    if (table->CreateXlibSurfaceKHR == nullptr) {
        table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)StubCreateXlibSurfaceKHR;
    }
    table->GetPhysicalDeviceXlibPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
    if (table->GetPhysicalDeviceXlibPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceXlibPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)StubGetPhysicalDeviceXlibPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)gpa(instance, "vkCreateXcbSurfaceKHR");
    if (table->CreateXcbSurfaceKHR == nullptr) {
        table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)StubCreateXcbSurfaceKHR;
    }
    table->GetPhysicalDeviceXcbPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
    if (table->GetPhysicalDeviceXcbPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)StubGetPhysicalDeviceXcbPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)gpa(instance, "vkCreateWaylandSurfaceKHR");
    if (table->CreateWaylandSurfaceKHR == nullptr) {
        table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)StubCreateWaylandSurfaceKHR;
    }
    table->GetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (table->GetPhysicalDeviceWaylandPresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceWaylandPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)StubGetPhysicalDeviceWaylandPresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)gpa(instance, "vkCreateAndroidSurfaceKHR");
    if (table->CreateAndroidSurfaceKHR == nullptr) {
        table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)StubCreateAndroidSurfaceKHR;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)gpa(instance, "vkCreateWin32SurfaceKHR");
    if (table->CreateWin32SurfaceKHR == nullptr) {
        table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)StubCreateWin32SurfaceKHR;
    }
    table->GetPhysicalDeviceWin32PresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)gpa(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (table->GetPhysicalDeviceWin32PresentationSupportKHR == nullptr) {
        table->GetPhysicalDeviceWin32PresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)StubGetPhysicalDeviceWin32PresentationSupportKHR;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceVideoCapabilitiesKHR =
        (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    if (table->GetPhysicalDeviceVideoCapabilitiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoCapabilitiesKHR =
            (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)StubGetPhysicalDeviceVideoCapabilitiesKHR;
    }
    table->GetPhysicalDeviceVideoFormatPropertiesKHR =
        (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
    if (table->GetPhysicalDeviceVideoFormatPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoFormatPropertiesKHR =
            (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)StubGetPhysicalDeviceVideoFormatPropertiesKHR;
    }
    table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)gpa(instance, "vkGetPhysicalDeviceFeatures2KHR");
    if (table->GetPhysicalDeviceFeatures2KHR == nullptr) {
        table->GetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)StubGetPhysicalDeviceFeatures2KHR;
    }
    table->GetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)gpa(instance, "vkGetPhysicalDeviceProperties2KHR");
    if (table->GetPhysicalDeviceProperties2KHR == nullptr) {
        table->GetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)StubGetPhysicalDeviceProperties2KHR;
    }
    table->GetPhysicalDeviceFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    if (table->GetPhysicalDeviceFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceFormatProperties2KHR)StubGetPhysicalDeviceFormatProperties2KHR;
    }
    table->GetPhysicalDeviceImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    if (table->GetPhysicalDeviceImageFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceImageFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)StubGetPhysicalDeviceImageFormatProperties2KHR;
    }
    table->GetPhysicalDeviceQueueFamilyProperties2KHR =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    if (table->GetPhysicalDeviceQueueFamilyProperties2KHR == nullptr) {
        table->GetPhysicalDeviceQueueFamilyProperties2KHR =
            (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)StubGetPhysicalDeviceQueueFamilyProperties2KHR;
    }
    table->GetPhysicalDeviceMemoryProperties2KHR =
        (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)gpa(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    if (table->GetPhysicalDeviceMemoryProperties2KHR == nullptr) {
        table->GetPhysicalDeviceMemoryProperties2KHR =
            (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)StubGetPhysicalDeviceMemoryProperties2KHR;
    }
    table->GetPhysicalDeviceSparseImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    if (table->GetPhysicalDeviceSparseImageFormatProperties2KHR == nullptr) {
        table->GetPhysicalDeviceSparseImageFormatProperties2KHR =
            (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)StubGetPhysicalDeviceSparseImageFormatProperties2KHR;
    }
    table->EnumeratePhysicalDeviceGroupsKHR =
        (PFN_vkEnumeratePhysicalDeviceGroupsKHR)gpa(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
    if (table->EnumeratePhysicalDeviceGroupsKHR == nullptr) {
        table->EnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)StubEnumeratePhysicalDeviceGroupsKHR;
    }
    table->GetPhysicalDeviceExternalBufferPropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    if (table->GetPhysicalDeviceExternalBufferPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalBufferPropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)StubGetPhysicalDeviceExternalBufferPropertiesKHR;
    }
    table->GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    if (table->GetPhysicalDeviceExternalSemaphorePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalSemaphorePropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)StubGetPhysicalDeviceExternalSemaphorePropertiesKHR;
    }
    table->GetPhysicalDeviceExternalFencePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)gpa(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    if (table->GetPhysicalDeviceExternalFencePropertiesKHR == nullptr) {
        table->GetPhysicalDeviceExternalFencePropertiesKHR =
            (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)StubGetPhysicalDeviceExternalFencePropertiesKHR;
    }
    table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
        (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)gpa(
            instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    if (table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR == nullptr) {
        table->EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)
                StubEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
    }
    table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)gpa(
        instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    if (table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR == nullptr) {
        table->GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR =
            (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)StubGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR;
    }
    table->GetPhysicalDeviceSurfaceCapabilities2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    if (table->GetPhysicalDeviceSurfaceCapabilities2KHR == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilities2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)StubGetPhysicalDeviceSurfaceCapabilities2KHR;
    }
    table->GetPhysicalDeviceSurfaceFormats2KHR =
        (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)gpa(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    if (table->GetPhysicalDeviceSurfaceFormats2KHR == nullptr) {
        table->GetPhysicalDeviceSurfaceFormats2KHR =
            (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)StubGetPhysicalDeviceSurfaceFormats2KHR;
    }
    table->GetPhysicalDeviceDisplayProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
    if (table->GetPhysicalDeviceDisplayProperties2KHR == nullptr) {
        table->GetPhysicalDeviceDisplayProperties2KHR =
            (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)StubGetPhysicalDeviceDisplayProperties2KHR;
    }
    table->GetPhysicalDeviceDisplayPlaneProperties2KHR =
        (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)gpa(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    if (table->GetPhysicalDeviceDisplayPlaneProperties2KHR == nullptr) {
        table->GetPhysicalDeviceDisplayPlaneProperties2KHR =
            (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)StubGetPhysicalDeviceDisplayPlaneProperties2KHR;
    }
    table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)gpa(instance, "vkGetDisplayModeProperties2KHR");
    if (table->GetDisplayModeProperties2KHR == nullptr) {
        table->GetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)StubGetDisplayModeProperties2KHR;
    }
    table->GetDisplayPlaneCapabilities2KHR =
        (PFN_vkGetDisplayPlaneCapabilities2KHR)gpa(instance, "vkGetDisplayPlaneCapabilities2KHR");
    if (table->GetDisplayPlaneCapabilities2KHR == nullptr) {
        table->GetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)StubGetDisplayPlaneCapabilities2KHR;
    }
    table->GetPhysicalDeviceFragmentShadingRatesKHR =
        (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)gpa(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    if (table->GetPhysicalDeviceFragmentShadingRatesKHR == nullptr) {
        table->GetPhysicalDeviceFragmentShadingRatesKHR =
            (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)StubGetPhysicalDeviceFragmentShadingRatesKHR;
    }
    table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)gpa(
        instance, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
    if (table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR =
            (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)StubGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;
    }
    table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    if (table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR == nullptr) {
        table->GetPhysicalDeviceCooperativeMatrixPropertiesKHR =
            (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)StubGetPhysicalDeviceCooperativeMatrixPropertiesKHR;
    }
    table->GetPhysicalDeviceCalibrateableTimeDomainsKHR =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR");
    if (table->GetPhysicalDeviceCalibrateableTimeDomainsKHR == nullptr) {
        table->GetPhysicalDeviceCalibrateableTimeDomainsKHR =
            (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)StubGetPhysicalDeviceCalibrateableTimeDomainsKHR;
    }
    table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)gpa(instance, "vkCreateDebugReportCallbackEXT");
    if (table->CreateDebugReportCallbackEXT == nullptr) {
        table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)StubCreateDebugReportCallbackEXT;
    }
    table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)gpa(instance, "vkDestroyDebugReportCallbackEXT");
    if (table->DestroyDebugReportCallbackEXT == nullptr) {
        table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)StubDestroyDebugReportCallbackEXT;
    }
    table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)gpa(instance, "vkDebugReportMessageEXT");
    if (table->DebugReportMessageEXT == nullptr) {
        table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)StubDebugReportMessageEXT;
    }
#ifdef VK_USE_PLATFORM_GGP
    table->CreateStreamDescriptorSurfaceGGP =
        (PFN_vkCreateStreamDescriptorSurfaceGGP)gpa(instance, "vkCreateStreamDescriptorSurfaceGGP");
    if (table->CreateStreamDescriptorSurfaceGGP == nullptr) {
        table->CreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)StubCreateStreamDescriptorSurfaceGGP;
    }
#endif  // VK_USE_PLATFORM_GGP
    table->GetPhysicalDeviceExternalImageFormatPropertiesNV =
        (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)gpa(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
    if (table->GetPhysicalDeviceExternalImageFormatPropertiesNV == nullptr) {
        table->GetPhysicalDeviceExternalImageFormatPropertiesNV =
            (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)StubGetPhysicalDeviceExternalImageFormatPropertiesNV;
    }
#ifdef VK_USE_PLATFORM_VI_NN
    table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)gpa(instance, "vkCreateViSurfaceNN");
    if (table->CreateViSurfaceNN == nullptr) {
        table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)StubCreateViSurfaceNN;
    }
#endif  // VK_USE_PLATFORM_VI_NN
    table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)gpa(instance, "vkReleaseDisplayEXT");
    if (table->ReleaseDisplayEXT == nullptr) {
        table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)StubReleaseDisplayEXT;
    }
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)gpa(instance, "vkAcquireXlibDisplayEXT");
    if (table->AcquireXlibDisplayEXT == nullptr) {
        table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)StubAcquireXlibDisplayEXT;
    }
    table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)gpa(instance, "vkGetRandROutputDisplayEXT");
    if (table->GetRandROutputDisplayEXT == nullptr) {
        table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)StubGetRandROutputDisplayEXT;
    }
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
    table->GetPhysicalDeviceSurfaceCapabilities2EXT =
        (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    if (table->GetPhysicalDeviceSurfaceCapabilities2EXT == nullptr) {
        table->GetPhysicalDeviceSurfaceCapabilities2EXT =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)StubGetPhysicalDeviceSurfaceCapabilities2EXT;
    }
#ifdef VK_USE_PLATFORM_IOS_MVK
    table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)gpa(instance, "vkCreateIOSSurfaceMVK");
    if (table->CreateIOSSurfaceMVK == nullptr) {
        table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)StubCreateIOSSurfaceMVK;
    }
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)gpa(instance, "vkCreateMacOSSurfaceMVK");
    if (table->CreateMacOSSurfaceMVK == nullptr) {
        table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)StubCreateMacOSSurfaceMVK;
    }
#endif  // VK_USE_PLATFORM_MACOS_MVK
    table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)gpa(instance, "vkCreateDebugUtilsMessengerEXT");
    if (table->CreateDebugUtilsMessengerEXT == nullptr) {
        table->CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)StubCreateDebugUtilsMessengerEXT;
    }
    table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)gpa(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (table->DestroyDebugUtilsMessengerEXT == nullptr) {
        table->DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)StubDestroyDebugUtilsMessengerEXT;
    }
    table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)gpa(instance, "vkSubmitDebugUtilsMessageEXT");
    if (table->SubmitDebugUtilsMessageEXT == nullptr) {
        table->SubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)StubSubmitDebugUtilsMessageEXT;
    }
    table->GetPhysicalDeviceMultisamplePropertiesEXT =
        (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)gpa(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    if (table->GetPhysicalDeviceMultisamplePropertiesEXT == nullptr) {
        table->GetPhysicalDeviceMultisamplePropertiesEXT =
            (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)StubGetPhysicalDeviceMultisamplePropertiesEXT;
    }
    table->GetPhysicalDeviceCalibrateableTimeDomainsEXT =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)gpa(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    if (table->GetPhysicalDeviceCalibrateableTimeDomainsEXT == nullptr) {
        table->GetPhysicalDeviceCalibrateableTimeDomainsEXT =
            (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)StubGetPhysicalDeviceCalibrateableTimeDomainsEXT;
    }
#ifdef VK_USE_PLATFORM_FUCHSIA
    table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)gpa(instance, "vkCreateImagePipeSurfaceFUCHSIA");
    if (table->CreateImagePipeSurfaceFUCHSIA == nullptr) {
        table->CreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)StubCreateImagePipeSurfaceFUCHSIA;
    }
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
    table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)gpa(instance, "vkCreateMetalSurfaceEXT");
    if (table->CreateMetalSurfaceEXT == nullptr) {
        table->CreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)StubCreateMetalSurfaceEXT;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    table->GetPhysicalDeviceToolPropertiesEXT =
        (PFN_vkGetPhysicalDeviceToolPropertiesEXT)gpa(instance, "vkGetPhysicalDeviceToolPropertiesEXT");
    if (table->GetPhysicalDeviceToolPropertiesEXT == nullptr) {
        table->GetPhysicalDeviceToolPropertiesEXT =
            (PFN_vkGetPhysicalDeviceToolPropertiesEXT)StubGetPhysicalDeviceToolPropertiesEXT;
    }
    table->GetPhysicalDeviceCooperativeMatrixPropertiesNV =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)gpa(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    if (table->GetPhysicalDeviceCooperativeMatrixPropertiesNV == nullptr) {
        table->GetPhysicalDeviceCooperativeMatrixPropertiesNV =
            (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)StubGetPhysicalDeviceCooperativeMatrixPropertiesNV;
    }
    table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
        (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)gpa(
            instance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
    if (table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV == nullptr) {
        table->GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV =
            (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)
                StubGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->GetPhysicalDeviceSurfacePresentModes2EXT =
        (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)gpa(instance, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
    if (table->GetPhysicalDeviceSurfacePresentModes2EXT == nullptr) {
        table->GetPhysicalDeviceSurfacePresentModes2EXT =
            (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)StubGetPhysicalDeviceSurfacePresentModes2EXT;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
    table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)gpa(instance, "vkCreateHeadlessSurfaceEXT");
    if (table->CreateHeadlessSurfaceEXT == nullptr) {
        table->CreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)StubCreateHeadlessSurfaceEXT;
    }
    table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)gpa(instance, "vkAcquireDrmDisplayEXT");
    if (table->AcquireDrmDisplayEXT == nullptr) {
        table->AcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)StubAcquireDrmDisplayEXT;
    }
    table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)gpa(instance, "vkGetDrmDisplayEXT");
    if (table->GetDrmDisplayEXT == nullptr) {
        table->GetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)StubGetDrmDisplayEXT;
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)gpa(instance, "vkAcquireWinrtDisplayNV");
    if (table->AcquireWinrtDisplayNV == nullptr) {
        table->AcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)StubAcquireWinrtDisplayNV;
    }
    table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)gpa(instance, "vkGetWinrtDisplayNV");
    if (table->GetWinrtDisplayNV == nullptr) {
        table->GetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)StubGetWinrtDisplayNV;
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)gpa(instance, "vkCreateDirectFBSurfaceEXT");
    if (table->CreateDirectFBSurfaceEXT == nullptr) {
        table->CreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)StubCreateDirectFBSurfaceEXT;
    }
    table->GetPhysicalDeviceDirectFBPresentationSupportEXT =
        (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)gpa(instance, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
    if (table->GetPhysicalDeviceDirectFBPresentationSupportEXT == nullptr) {
        table->GetPhysicalDeviceDirectFBPresentationSupportEXT =
            (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)StubGetPhysicalDeviceDirectFBPresentationSupportEXT;
    }
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)gpa(instance, "vkCreateScreenSurfaceQNX");
    if (table->CreateScreenSurfaceQNX == nullptr) {
        table->CreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)StubCreateScreenSurfaceQNX;
    }
    table->GetPhysicalDeviceScreenPresentationSupportQNX =
        (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)gpa(instance, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
    if (table->GetPhysicalDeviceScreenPresentationSupportQNX == nullptr) {
        table->GetPhysicalDeviceScreenPresentationSupportQNX =
            (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)StubGetPhysicalDeviceScreenPresentationSupportQNX;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    table->GetPhysicalDeviceOpticalFlowImageFormatsNV =
        (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)gpa(instance, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
    if (table->GetPhysicalDeviceOpticalFlowImageFormatsNV == nullptr) {
        table->GetPhysicalDeviceOpticalFlowImageFormatsNV =
            (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)StubGetPhysicalDeviceOpticalFlowImageFormatsNV;
    }
    table->GetPhysicalDeviceCooperativeVectorPropertiesNV =
        (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)gpa(instance, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
    if (table->GetPhysicalDeviceCooperativeVectorPropertiesNV == nullptr) {
        table->GetPhysicalDeviceCooperativeVectorPropertiesNV =
            (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)StubGetPhysicalDeviceCooperativeVectorPropertiesNV;
    }
    table->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV =
        (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)gpa(
            instance, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
    if (table->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV == nullptr) {
        table->GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV =
            (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)
                StubGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV;
    }
}

// NOLINTEND
