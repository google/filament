/*
 * Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2022 Valve Corporation
 * Copyright (c) 2021-2022 LunarG, Inc.
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
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "test_environment.h"

extern "C" {
#include "loader_common.h"
}

#include <array>
#include <cstring>
#include <random>
#include <set>
#include <string>
#include <unordered_map>

// On arm64x the loader has two views of each entry point. An x64 caller (an arm64ec build, or a plain
// x64 process emulated on an Arm64 host) gets an export's x64 fast-forward-sequence thunk back from
// GetProcAddress, while the loader resolves its own name to the native arm64ec function. Same command,
// different address, and the spec doesn't require them to match, so in that case compare behavior
// instead of raw pointer identity. Everywhere else the pointers are expected to be identical.
static bool proc_addr_identity_expected() {
#if defined(_M_ARM64EC)
    return false;
#elif defined(_WIN32)
    USHORT process_machine = 0, native_machine = 0;
    if (IsWow64Process2(GetCurrentProcess(), &process_machine, &native_machine)) {
        // On an Arm64 host vulkan-1.dll may be arm64x, in which case an x64 (emulated) or arm64ec
        // caller gets a fast-forward-sequence thunk that differs from the loader's native pointer.
        // Note x64-on-Arm64 is not classic WOW64, so process_machine is UNKNOWN there, not AMD64;
        // key off the native machine instead.
        return native_machine != IMAGE_FILE_MACHINE_ARM64;
    }
    return true;
#else
    return true;
#endif
}

static void assert_gipa_equivalent(PFN_vkGetInstanceProcAddr from_loader, PFN_vkGetInstanceProcAddr from_query) {
    if (proc_addr_identity_expected()) {
        ASSERT_EQ(from_loader, from_query);
        return;
    }
    ASSERT_NE(from_loader, nullptr);
    ASSERT_NE(from_query, nullptr);
    ASSERT_EQ(from_loader(nullptr, "vkThisCommandDoesNotExist"), nullptr);
    ASSERT_EQ(from_query(nullptr, "vkThisCommandDoesNotExist"), nullptr);
    PFN_vkVoidFunction create_from_loader = from_loader(nullptr, "vkCreateInstance");
    PFN_vkVoidFunction create_from_query = from_query(nullptr, "vkCreateInstance");
    ASSERT_NE(create_from_loader, nullptr);
    ASSERT_EQ(create_from_loader, create_from_query);
}

static void assert_gdpa_equivalent(PFN_vkGetDeviceProcAddr from_loader, PFN_vkGetDeviceProcAddr from_query, VkDevice device) {
    if (proc_addr_identity_expected()) {
        (void)device;
        ASSERT_EQ(from_loader, from_query);
        return;
    }
    ASSERT_NE(from_loader, nullptr);
    ASSERT_NE(from_query, nullptr);
    if (device != VK_NULL_HANDLE) {
        ASSERT_EQ(from_loader(device, "vkThisCommandDoesNotExist"), nullptr);
        ASSERT_EQ(from_query(device, "vkThisCommandDoesNotExist"), nullptr);
        PFN_vkVoidFunction destroy_from_loader = from_loader(device, "vkDestroyDevice");
        PFN_vkVoidFunction destroy_from_query = from_query(device, "vkDestroyDevice");
        ASSERT_NE(destroy_from_loader, nullptr);
        ASSERT_EQ(destroy_from_loader, destroy_from_query);
    }
}

// Every name gpa_helper.c's trampoline_get_proc_addr() hand-checks via
// `case 0x...u: if (!strcmp(...)) return ...; break;` (all core Vulkan 1.0-1.4 commands - see
// loader/gpa_helper.c). This list mirrors that function's checks
// exactly so a mistake made while mechanically threading the hash pre-filter through ~230 hand-written
// strcmp branches (wrong hash constant, dropped branch, mismatched name) shows up as a concrete failure
// here instead of silently returning NULL/wrong-pointer for some entry point.
static constexpr std::array kGpaHelperCoreInstanceNames = {
    "vkGetInstanceProcAddr",
    "vkDestroyInstance",
    "vkEnumeratePhysicalDevices",
    "vkGetPhysicalDeviceFeatures",
    "vkGetPhysicalDeviceFormatProperties",
    "vkGetPhysicalDeviceImageFormatProperties",
    "vkGetPhysicalDeviceSparseImageFormatProperties",
    "vkGetPhysicalDeviceProperties",
    "vkGetPhysicalDeviceQueueFamilyProperties",
    "vkGetPhysicalDeviceMemoryProperties",
    "vkEnumerateDeviceLayerProperties",
    "vkEnumerateDeviceExtensionProperties",
    "vkCreateDevice",
    "vkGetDeviceProcAddr",
    "vkDestroyDevice",
    "vkGetDeviceQueue",
    "vkQueueSubmit",
    "vkQueueWaitIdle",
    "vkDeviceWaitIdle",
    "vkAllocateMemory",
    "vkFreeMemory",
    "vkMapMemory",
    "vkUnmapMemory",
    "vkFlushMappedMemoryRanges",
    "vkInvalidateMappedMemoryRanges",
    "vkGetDeviceMemoryCommitment",
    "vkGetImageSparseMemoryRequirements",
    "vkGetImageMemoryRequirements",
    "vkGetBufferMemoryRequirements",
    "vkBindImageMemory",
    "vkBindBufferMemory",
    "vkQueueBindSparse",
    "vkCreateFence",
    "vkDestroyFence",
    "vkGetFenceStatus",
    "vkResetFences",
    "vkWaitForFences",
    "vkCreateSemaphore",
    "vkDestroySemaphore",
    "vkCreateEvent",
    "vkDestroyEvent",
    "vkGetEventStatus",
    "vkSetEvent",
    "vkResetEvent",
    "vkCreateQueryPool",
    "vkDestroyQueryPool",
    "vkGetQueryPoolResults",
    "vkCreateBuffer",
    "vkDestroyBuffer",
    "vkCreateBufferView",
    "vkDestroyBufferView",
    "vkCreateImage",
    "vkDestroyImage",
    "vkGetImageSubresourceLayout",
    "vkCreateImageView",
    "vkDestroyImageView",
    "vkCreateShaderModule",
    "vkDestroyShaderModule",
    "vkCreatePipelineCache",
    "vkDestroyPipelineCache",
    "vkGetPipelineCacheData",
    "vkMergePipelineCaches",
    "vkCreateGraphicsPipelines",
    "vkCreateComputePipelines",
    "vkDestroyPipeline",
    "vkCreatePipelineLayout",
    "vkDestroyPipelineLayout",
    "vkCreateSampler",
    "vkDestroySampler",
    "vkCreateDescriptorSetLayout",
    "vkDestroyDescriptorSetLayout",
    "vkCreateDescriptorPool",
    "vkDestroyDescriptorPool",
    "vkResetDescriptorPool",
    "vkAllocateDescriptorSets",
    "vkFreeDescriptorSets",
    "vkUpdateDescriptorSets",
    "vkCreateFramebuffer",
    "vkDestroyFramebuffer",
    "vkCreateRenderPass",
    "vkDestroyRenderPass",
    "vkGetRenderAreaGranularity",
    "vkCreateCommandPool",
    "vkDestroyCommandPool",
    "vkResetCommandPool",
    "vkAllocateCommandBuffers",
    "vkFreeCommandBuffers",
    "vkBeginCommandBuffer",
    "vkEndCommandBuffer",
    "vkResetCommandBuffer",
    "vkCmdBindPipeline",
    "vkCmdBindDescriptorSets",
    "vkCmdBindVertexBuffers",
    "vkCmdBindIndexBuffer",
    "vkCmdSetViewport",
    "vkCmdSetScissor",
    "vkCmdSetLineWidth",
    "vkCmdSetDepthBias",
    "vkCmdSetBlendConstants",
    "vkCmdSetDepthBounds",
    "vkCmdSetStencilCompareMask",
    "vkCmdSetStencilWriteMask",
    "vkCmdSetStencilReference",
    "vkCmdDraw",
    "vkCmdDrawIndexed",
    "vkCmdDrawIndirect",
    "vkCmdDrawIndexedIndirect",
    "vkCmdDispatch",
    "vkCmdDispatchIndirect",
    "vkCmdCopyBuffer",
    "vkCmdCopyImage",
    "vkCmdBlitImage",
    "vkCmdCopyBufferToImage",
    "vkCmdCopyImageToBuffer",
    "vkCmdUpdateBuffer",
    "vkCmdFillBuffer",
    "vkCmdClearColorImage",
    "vkCmdClearDepthStencilImage",
    "vkCmdClearAttachments",
    "vkCmdResolveImage",
    "vkCmdSetEvent",
    "vkCmdResetEvent",
    "vkCmdWaitEvents",
    "vkCmdPipelineBarrier",
    "vkCmdBeginQuery",
    "vkCmdEndQuery",
    "vkCmdResetQueryPool",
    "vkCmdWriteTimestamp",
    "vkCmdCopyQueryPoolResults",
    "vkCmdPushConstants",
    "vkCmdBeginRenderPass",
    "vkCmdNextSubpass",
    "vkCmdEndRenderPass",
    "vkCmdExecuteCommands",
    "vkEnumeratePhysicalDeviceGroups",
    "vkGetPhysicalDeviceFeatures2",
    "vkGetPhysicalDeviceProperties2",
    "vkGetPhysicalDeviceFormatProperties2",
    "vkGetPhysicalDeviceImageFormatProperties2",
    "vkGetPhysicalDeviceQueueFamilyProperties2",
    "vkGetPhysicalDeviceMemoryProperties2",
    "vkGetPhysicalDeviceSparseImageFormatProperties2",
    "vkGetPhysicalDeviceExternalBufferProperties",
    "vkGetPhysicalDeviceExternalSemaphoreProperties",
    "vkGetPhysicalDeviceExternalFenceProperties",
    "vkBindBufferMemory2",
    "vkBindImageMemory2",
    "vkGetDeviceGroupPeerMemoryFeatures",
    "vkCmdSetDeviceMask",
    "vkCmdDispatchBase",
    "vkGetImageMemoryRequirements2",
    "vkTrimCommandPool",
    "vkGetDeviceQueue2",
    "vkCreateSamplerYcbcrConversion",
    "vkDestroySamplerYcbcrConversion",
    "vkGetDescriptorSetLayoutSupport",
    "vkCreateDescriptorUpdateTemplate",
    "vkDestroyDescriptorUpdateTemplate",
    "vkUpdateDescriptorSetWithTemplate",
    "vkGetImageSparseMemoryRequirements2",
    "vkGetBufferMemoryRequirements2",
    "vkCreateRenderPass2",
    "vkCmdBeginRenderPass2",
    "vkCmdNextSubpass2",
    "vkCmdEndRenderPass2",
    "vkCmdDrawIndirectCount",
    "vkCmdDrawIndexedIndirectCount",
    "vkGetSemaphoreCounterValue",
    "vkWaitSemaphores",
    "vkSignalSemaphore",
    "vkGetBufferDeviceAddress",
    "vkGetBufferOpaqueCaptureAddress",
    "vkGetDeviceMemoryOpaqueCaptureAddress",
    "vkResetQueryPool",
    "vkGetPhysicalDeviceToolProperties",
    "vkCreatePrivateDataSlot",
    "vkDestroyPrivateDataSlot",
    "vkSetPrivateData",
    "vkGetPrivateData",
    "vkCmdSetEvent2",
    "vkCmdResetEvent2",
    "vkCmdWaitEvents2",
    "vkCmdPipelineBarrier2",
    "vkCmdWriteTimestamp2",
    "vkQueueSubmit2",
    "vkCmdCopyBuffer2",
    "vkCmdCopyImage2",
    "vkCmdCopyBufferToImage2",
    "vkCmdCopyImageToBuffer2",
    "vkCmdBlitImage2",
    "vkCmdResolveImage2",
    "vkCmdBeginRendering",
    "vkCmdEndRendering",
    "vkCmdSetCullMode",
    "vkCmdSetFrontFace",
    "vkCmdSetPrimitiveTopology",
    "vkCmdSetViewportWithCount",
    "vkCmdSetScissorWithCount",
    "vkCmdBindVertexBuffers2",
    "vkCmdSetDepthTestEnable",
    "vkCmdSetDepthWriteEnable",
    "vkCmdSetDepthCompareOp",
    "vkCmdSetDepthBoundsTestEnable",
    "vkCmdSetStencilTestEnable",
    "vkCmdSetStencilOp",
    "vkCmdSetRasterizerDiscardEnable",
    "vkCmdSetDepthBiasEnable",
    "vkCmdSetPrimitiveRestartEnable",
    "vkGetDeviceBufferMemoryRequirements",
    "vkGetDeviceImageMemoryRequirements",
    "vkGetDeviceImageSparseMemoryRequirements",
    "vkCmdSetLineStipple",
    "vkMapMemory2",
    "vkUnmapMemory2",
    "vkCmdBindIndexBuffer2",
    "vkGetRenderingAreaGranularity",
    "vkGetDeviceImageSubresourceLayout",
    "vkGetImageSubresourceLayout2",
    "vkCmdPushDescriptorSet",
    "vkCmdPushDescriptorSetWithTemplate",
    "vkCmdSetRenderingAttachmentLocations",
    "vkCmdSetRenderingInputAttachmentIndices",
    "vkCmdBindDescriptorSets2",
    "vkCmdPushConstants2",
    "vkCmdPushDescriptorSet2",
    "vkCmdPushDescriptorSetWithTemplate2",
    "vkCopyMemoryToImage",
    "vkCopyImageToMemory",
    "vkCopyImageToImage",
    "vkTransitionImageLayout",
};

// Every name loader_lookup_device_dispatch_table() (loader/generated/vk_loader_extensions.c) hand-checks
// for the core Vulkan 1.0-1.4 sections. Unlike the instance trampolines above, these entries only resolve
// to a non-null pointer if the underlying (mock) driver actually reports the function, so the test
// physical device must be told about every one of them via add_device_function() first.
static constexpr std::array kDeviceDispatchCoreNames = {
    "vkGetDeviceProcAddr",
    "vkDestroyDevice",
    "vkGetDeviceQueue",
    "vkQueueSubmit",
    "vkQueueWaitIdle",
    "vkDeviceWaitIdle",
    "vkAllocateMemory",
    "vkFreeMemory",
    "vkMapMemory",
    "vkUnmapMemory",
    "vkFlushMappedMemoryRanges",
    "vkInvalidateMappedMemoryRanges",
    "vkGetDeviceMemoryCommitment",
    "vkBindBufferMemory",
    "vkBindImageMemory",
    "vkGetBufferMemoryRequirements",
    "vkGetImageMemoryRequirements",
    "vkGetImageSparseMemoryRequirements",
    "vkQueueBindSparse",
    "vkCreateFence",
    "vkDestroyFence",
    "vkResetFences",
    "vkGetFenceStatus",
    "vkWaitForFences",
    "vkCreateSemaphore",
    "vkDestroySemaphore",
    "vkCreateQueryPool",
    "vkDestroyQueryPool",
    "vkGetQueryPoolResults",
    "vkCreateBuffer",
    "vkDestroyBuffer",
    "vkCreateImage",
    "vkDestroyImage",
    "vkGetImageSubresourceLayout",
    "vkCreateImageView",
    "vkDestroyImageView",
    "vkCreateCommandPool",
    "vkDestroyCommandPool",
    "vkResetCommandPool",
    "vkAllocateCommandBuffers",
    "vkFreeCommandBuffers",
    "vkBeginCommandBuffer",
    "vkEndCommandBuffer",
    "vkResetCommandBuffer",
    "vkCmdCopyBuffer",
    "vkCmdCopyImage",
    "vkCmdCopyBufferToImage",
    "vkCmdCopyImageToBuffer",
    "vkCmdUpdateBuffer",
    "vkCmdFillBuffer",
    "vkCmdPipelineBarrier",
    "vkCmdBeginQuery",
    "vkCmdEndQuery",
    "vkCmdResetQueryPool",
    "vkCmdWriteTimestamp",
    "vkCmdCopyQueryPoolResults",
    "vkCmdExecuteCommands",
    "vkCreateEvent",
    "vkDestroyEvent",
    "vkGetEventStatus",
    "vkSetEvent",
    "vkResetEvent",
    "vkCreateBufferView",
    "vkDestroyBufferView",
    "vkCreateShaderModule",
    "vkDestroyShaderModule",
    "vkCreatePipelineCache",
    "vkDestroyPipelineCache",
    "vkGetPipelineCacheData",
    "vkMergePipelineCaches",
    "vkCreateComputePipelines",
    "vkDestroyPipeline",
    "vkCreatePipelineLayout",
    "vkDestroyPipelineLayout",
    "vkCreateSampler",
    "vkDestroySampler",
    "vkCreateDescriptorSetLayout",
    "vkDestroyDescriptorSetLayout",
    "vkCreateDescriptorPool",
    "vkDestroyDescriptorPool",
    "vkResetDescriptorPool",
    "vkAllocateDescriptorSets",
    "vkFreeDescriptorSets",
    "vkUpdateDescriptorSets",
    "vkCmdBindPipeline",
    "vkCmdBindDescriptorSets",
    "vkCmdClearColorImage",
    "vkCmdDispatch",
    "vkCmdDispatchIndirect",
    "vkCmdSetEvent",
    "vkCmdResetEvent",
    "vkCmdWaitEvents",
    "vkCmdPushConstants",
    "vkCreateGraphicsPipelines",
    "vkCreateFramebuffer",
    "vkDestroyFramebuffer",
    "vkCreateRenderPass",
    "vkDestroyRenderPass",
    "vkGetRenderAreaGranularity",
    "vkCmdSetViewport",
    "vkCmdSetScissor",
    "vkCmdSetLineWidth",
    "vkCmdSetDepthBias",
    "vkCmdSetBlendConstants",
    "vkCmdSetDepthBounds",
    "vkCmdSetStencilCompareMask",
    "vkCmdSetStencilWriteMask",
    "vkCmdSetStencilReference",
    "vkCmdBindIndexBuffer",
    "vkCmdBindVertexBuffers",
    "vkCmdDraw",
    "vkCmdDrawIndexed",
    "vkCmdDrawIndirect",
    "vkCmdDrawIndexedIndirect",
    "vkCmdBlitImage",
    "vkCmdClearDepthStencilImage",
    "vkCmdClearAttachments",
    "vkCmdResolveImage",
    "vkCmdBeginRenderPass",
    "vkCmdNextSubpass",
    "vkCmdEndRenderPass",
    "vkBindBufferMemory2",
    "vkBindImageMemory2",
    "vkGetDeviceGroupPeerMemoryFeatures",
    "vkCmdSetDeviceMask",
    "vkGetImageMemoryRequirements2",
    "vkGetBufferMemoryRequirements2",
    "vkGetImageSparseMemoryRequirements2",
    "vkTrimCommandPool",
    "vkGetDeviceQueue2",
    "vkCmdDispatchBase",
    "vkCreateDescriptorUpdateTemplate",
    "vkDestroyDescriptorUpdateTemplate",
    "vkUpdateDescriptorSetWithTemplate",
    "vkGetDescriptorSetLayoutSupport",
    "vkCreateSamplerYcbcrConversion",
    "vkDestroySamplerYcbcrConversion",
    "vkResetQueryPool",
    "vkGetSemaphoreCounterValue",
    "vkWaitSemaphores",
    "vkSignalSemaphore",
    "vkGetBufferDeviceAddress",
    "vkGetBufferOpaqueCaptureAddress",
    "vkGetDeviceMemoryOpaqueCaptureAddress",
    "vkCmdDrawIndirectCount",
    "vkCmdDrawIndexedIndirectCount",
    "vkCreateRenderPass2",
    "vkCmdBeginRenderPass2",
    "vkCmdNextSubpass2",
    "vkCmdEndRenderPass2",
    "vkCreatePrivateDataSlot",
    "vkDestroyPrivateDataSlot",
    "vkSetPrivateData",
    "vkGetPrivateData",
    "vkCmdPipelineBarrier2",
    "vkCmdWriteTimestamp2",
    "vkQueueSubmit2",
    "vkCmdCopyBuffer2",
    "vkCmdCopyImage2",
    "vkCmdCopyBufferToImage2",
    "vkCmdCopyImageToBuffer2",
    "vkGetDeviceBufferMemoryRequirements",
    "vkGetDeviceImageMemoryRequirements",
    "vkGetDeviceImageSparseMemoryRequirements",
    "vkCmdSetEvent2",
    "vkCmdResetEvent2",
    "vkCmdWaitEvents2",
    "vkCmdBlitImage2",
    "vkCmdResolveImage2",
    "vkCmdBeginRendering",
    "vkCmdEndRendering",
    "vkCmdSetCullMode",
    "vkCmdSetFrontFace",
    "vkCmdSetPrimitiveTopology",
    "vkCmdSetViewportWithCount",
    "vkCmdSetScissorWithCount",
    "vkCmdBindVertexBuffers2",
    "vkCmdSetDepthTestEnable",
    "vkCmdSetDepthWriteEnable",
    "vkCmdSetDepthCompareOp",
    "vkCmdSetDepthBoundsTestEnable",
    "vkCmdSetStencilTestEnable",
    "vkCmdSetStencilOp",
    "vkCmdSetRasterizerDiscardEnable",
    "vkCmdSetDepthBiasEnable",
    "vkCmdSetPrimitiveRestartEnable",
    "vkMapMemory2",
    "vkUnmapMemory2",
    "vkGetDeviceImageSubresourceLayout",
    "vkGetImageSubresourceLayout2",
    "vkCopyMemoryToImage",
    "vkCopyImageToMemory",
    "vkCopyImageToImage",
    "vkTransitionImageLayout",
    "vkCmdPushDescriptorSet",
    "vkCmdPushDescriptorSetWithTemplate",
    "vkCmdBindDescriptorSets2",
    "vkCmdPushConstants2",
    "vkCmdPushDescriptorSet2",
    "vkCmdPushDescriptorSetWithTemplate2",
    "vkCmdSetLineStipple",
    "vkCmdBindIndexBuffer2",
    "vkGetRenderingAreaGranularity",
    "vkCmdSetRenderingAttachmentLocations",
    "vkCmdSetRenderingInputAttachmentIndices",
};

// The ~553 extension command names extension_instance_gpa() (loader/generated/vk_loader_extensions.c)
// hand-checks, minus ~72 that can't be exercised this way without extra per-name setup: ~48 compiled only
// under a platform-specific #if this Linux test build doesn't define (VK_USE_PLATFORM_WIN32_KHR,
// _ANDROID_KHR, _METAL_EXT, _FUCHSIA, etc.), and ~24 whose case only returns non-null when the backing
// extension is enabled at instance creation, so a bare resolve can't tell "recognized but disabled" apart
// from "a hash-mapping bug made this fall through to not-found". The remaining 481 names resolve
// unconditionally regardless of instance/ICD state, so they can be exhaustively checked like the arrays above.
//
// Explicit template arguments (not CTAD) are load-bearing: Clang caps the recursive fold expression in
// libstdc++'s std::array deduction guide at 256 instantiations, below this array's 481 entries. Naming the
// type and size directly skips that deduction guide.
static constexpr std::array<const char*, 481> kExtensionInstanceGpaNames = {
    "vkGetPhysicalDeviceVideoCapabilitiesKHR",
    "vkGetPhysicalDeviceVideoFormatPropertiesKHR",
    "vkCreateVideoSessionKHR",
    "vkDestroyVideoSessionKHR",
    "vkGetVideoSessionMemoryRequirementsKHR",
    "vkBindVideoSessionMemoryKHR",
    "vkCreateVideoSessionParametersKHR",
    "vkUpdateVideoSessionParametersKHR",
    "vkDestroyVideoSessionParametersKHR",
    "vkCmdBeginVideoCodingKHR",
    "vkCmdEndVideoCodingKHR",
    "vkCmdControlVideoCodingKHR",
    "vkCmdDecodeVideoKHR",
    "vkCmdBeginRenderingKHR",
    "vkCmdEndRenderingKHR",
    "vkGetDeviceGroupPeerMemoryFeaturesKHR",
    "vkCmdSetDeviceMaskKHR",
    "vkCmdDispatchBaseKHR",
    "vkTrimCommandPoolKHR",
    "vkGetMemoryFdKHR",
    "vkGetMemoryFdPropertiesKHR",
    "vkImportSemaphoreFdKHR",
    "vkGetSemaphoreFdKHR",
    "vkCmdPushDescriptorSetKHR",
    "vkCmdPushDescriptorSetWithTemplateKHR",
    "vkCreateDescriptorUpdateTemplateKHR",
    "vkDestroyDescriptorUpdateTemplateKHR",
    "vkUpdateDescriptorSetWithTemplateKHR",
    "vkCreateRenderPass2KHR",
    "vkCmdBeginRenderPass2KHR",
    "vkCmdNextSubpass2KHR",
    "vkCmdEndRenderPass2KHR",
    "vkGetSwapchainStatusKHR",
    "vkImportFenceFdKHR",
    "vkGetFenceFdKHR",
    "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR",
    "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR",
    "vkAcquireProfilingLockKHR",
    "vkReleaseProfilingLockKHR",
    "vkGetImageMemoryRequirements2KHR",
    "vkGetBufferMemoryRequirements2KHR",
    "vkGetImageSparseMemoryRequirements2KHR",
    "vkCreateSamplerYcbcrConversionKHR",
    "vkDestroySamplerYcbcrConversionKHR",
    "vkBindBufferMemory2KHR",
    "vkBindImageMemory2KHR",
    "vkGetDescriptorSetLayoutSupportKHR",
    "vkCmdDrawIndirectCountKHR",
    "vkCmdDrawIndexedIndirectCountKHR",
    "vkGetSemaphoreCounterValueKHR",
    "vkWaitSemaphoresKHR",
    "vkSignalSemaphoreKHR",
    "vkGetPhysicalDeviceFragmentShadingRatesKHR",
    "vkCmdSetFragmentShadingRateKHR",
    "vkCmdSetRenderingAttachmentLocationsKHR",
    "vkCmdSetRenderingInputAttachmentIndicesKHR",
    "vkWaitForPresentKHR",
    "vkGetBufferDeviceAddressKHR",
    "vkGetBufferOpaqueCaptureAddressKHR",
    "vkGetDeviceMemoryOpaqueCaptureAddressKHR",
    "vkCreateDeferredOperationKHR",
    "vkDestroyDeferredOperationKHR",
    "vkGetDeferredOperationMaxConcurrencyKHR",
    "vkGetDeferredOperationResultKHR",
    "vkDeferredOperationJoinKHR",
    "vkGetPipelineExecutablePropertiesKHR",
    "vkGetPipelineExecutableStatisticsKHR",
    "vkGetPipelineExecutableInternalRepresentationsKHR",
    "vkMapMemory2KHR",
    "vkUnmapMemory2KHR",
    "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR",
    "vkGetEncodedVideoSessionParametersKHR",
    "vkCmdEncodeVideoKHR",
    "vkCmdSetEvent2KHR",
    "vkCmdResetEvent2KHR",
    "vkCmdWaitEvents2KHR",
    "vkCmdPipelineBarrier2KHR",
    "vkCmdWriteTimestamp2KHR",
    "vkQueueSubmit2KHR",
    "vkCmdBindIndexBuffer3KHR",
    "vkCmdBindVertexBuffers3KHR",
    "vkCmdDrawIndirect2KHR",
    "vkCmdDrawIndexedIndirect2KHR",
    "vkCmdDispatchIndirect2KHR",
    "vkCmdCopyMemoryKHR",
    "vkCmdCopyMemoryToImageKHR",
    "vkCmdCopyImageToMemoryKHR",
    "vkCmdUpdateMemoryKHR",
    "vkCmdFillMemoryKHR",
    "vkCmdCopyQueryPoolResultsToMemoryKHR",
    "vkCmdDrawIndirectCount2KHR",
    "vkCmdDrawIndexedIndirectCount2KHR",
    "vkCmdBeginConditionalRendering2EXT",
    "vkCmdBindTransformFeedbackBuffers2EXT",
    "vkCmdBeginTransformFeedback2EXT",
    "vkCmdEndTransformFeedback2EXT",
    "vkCmdDrawIndirectByteCount2EXT",
    "vkCmdDrawMeshTasksIndirect2EXT",
    "vkCmdDrawMeshTasksIndirectCount2EXT",
    "vkCmdWriteMarkerToMemoryAMD",
    "vkCreateAccelerationStructure2KHR",
    "vkCmdCopyBuffer2KHR",
    "vkCmdCopyImage2KHR",
    "vkCmdCopyBufferToImage2KHR",
    "vkCmdCopyImageToBuffer2KHR",
    "vkCmdBlitImage2KHR",
    "vkCmdResolveImage2KHR",
    "vkCmdTraceRaysIndirect2KHR",
    "vkGetDeviceBufferMemoryRequirementsKHR",
    "vkGetDeviceImageMemoryRequirementsKHR",
    "vkGetDeviceImageSparseMemoryRequirementsKHR",
    "vkCmdBindIndexBuffer2KHR",
    "vkGetRenderingAreaGranularityKHR",
    "vkGetDeviceImageSubresourceLayoutKHR",
    "vkGetImageSubresourceLayout2KHR",
    "vkWaitForPresent2KHR",
    "vkCreatePipelineBinariesKHR",
    "vkDestroyPipelineBinaryKHR",
    "vkGetPipelineKeyKHR",
    "vkGetPipelineBinaryDataKHR",
    "vkReleaseCapturedPipelineDataKHR",
    "vkReleaseSwapchainImagesKHR",
    "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR",
    "vkCmdSetLineStippleKHR",
    "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR",
    "vkGetCalibratedTimestampsKHR",
    "vkCmdBindDescriptorSets2KHR",
    "vkCmdPushConstants2KHR",
    "vkCmdPushDescriptorSet2KHR",
    "vkCmdPushDescriptorSetWithTemplate2KHR",
    "vkCmdSetDescriptorBufferOffsets2EXT",
    "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT",
    "vkCmdCopyMemoryIndirectKHR",
    "vkCmdCopyMemoryToImageIndirectKHR",
    "vkGetDeviceFaultReportsKHR",
    "vkGetDeviceFaultDebugInfoKHR",
    "vkCmdEndRendering2KHR",
    "vkDebugMarkerSetObjectTagEXT",
    "vkDebugMarkerSetObjectNameEXT",
    "vkCmdDebugMarkerBeginEXT",
    "vkCmdDebugMarkerEndEXT",
    "vkCmdDebugMarkerInsertEXT",
    "vkCmdBindTransformFeedbackBuffersEXT",
    "vkCmdBeginTransformFeedbackEXT",
    "vkCmdEndTransformFeedbackEXT",
    "vkCmdBeginQueryIndexedEXT",
    "vkCmdEndQueryIndexedEXT",
    "vkCmdDrawIndirectByteCountEXT",
    "vkCreateCuModuleNVX",
    "vkCreateCuFunctionNVX",
    "vkDestroyCuModuleNVX",
    "vkDestroyCuFunctionNVX",
    "vkCmdCuLaunchKernelNVX",
    "vkGetImageViewHandleNVX",
    "vkGetImageViewHandle64NVX",
    "vkGetImageViewAddressNVX",
    "vkGetDeviceCombinedImageSamplerIndexNVX",
    "vkCmdDrawIndirectCountAMD",
    "vkCmdDrawIndexedIndirectCountAMD",
    "vkGetShaderInfoAMD",
    "vkCmdBeginConditionalRenderingEXT",
    "vkCmdEndConditionalRenderingEXT",
    "vkCmdSetViewportWScalingNV",
    "vkDisplayPowerControlEXT",
    "vkRegisterDeviceEventEXT",
    "vkRegisterDisplayEventEXT",
    "vkGetSwapchainCounterEXT",
    "vkGetRefreshCycleDurationGOOGLE",
    "vkGetPastPresentationTimingGOOGLE",
    "vkCmdSetDiscardRectangleEXT",
    "vkCmdSetDiscardRectangleEnableEXT",
    "vkCmdSetDiscardRectangleModeEXT",
    "vkSetHdrMetadataEXT",
    "vkCreateGpaSessionAMD",
    "vkDestroyGpaSessionAMD",
    "vkSetGpaDeviceClockModeAMD",
    "vkGetGpaDeviceClockInfoAMD",
    "vkCmdBeginGpaSessionAMD",
    "vkCmdEndGpaSessionAMD",
    "vkCmdBeginGpaSampleAMD",
    "vkCmdEndGpaSampleAMD",
    "vkGetGpaSessionStatusAMD",
    "vkGetGpaSessionResultsAMD",
    "vkResetGpaSessionAMD",
    "vkCmdCopyGpaSessionResultsAMD",
    "vkWriteSamplerDescriptorsEXT",
    "vkWriteResourceDescriptorsEXT",
    "vkCmdBindSamplerHeapEXT",
    "vkCmdBindResourceHeapEXT",
    "vkCmdPushDataEXT",
    "vkGetImageOpaqueCaptureDataEXT",
    "vkGetPhysicalDeviceDescriptorSizeEXT",
    "vkRegisterCustomBorderColorEXT",
    "vkUnregisterCustomBorderColorEXT",
    "vkGetTensorOpaqueCaptureDataARM",
    "vkCmdSetSampleLocationsEXT",
    "vkGetPhysicalDeviceMultisamplePropertiesEXT",
    "vkGetImageDrmFormatModifierPropertiesEXT",
    "vkCreateValidationCacheEXT",
    "vkDestroyValidationCacheEXT",
    "vkMergeValidationCachesEXT",
    "vkGetValidationCacheDataEXT",
    "vkCmdBindShadingRateImageNV",
    "vkCmdSetViewportShadingRatePaletteNV",
    "vkCmdSetCoarseSampleOrderNV",
    "vkCreateAccelerationStructureNV",
    "vkDestroyAccelerationStructureNV",
    "vkGetAccelerationStructureMemoryRequirementsNV",
    "vkBindAccelerationStructureMemoryNV",
    "vkCmdBuildAccelerationStructureNV",
    "vkCmdCopyAccelerationStructureNV",
    "vkCmdTraceRaysNV",
    "vkCreateRayTracingPipelinesNV",
    "vkGetRayTracingShaderGroupHandlesKHR",
    "vkGetRayTracingShaderGroupHandlesNV",
    "vkGetAccelerationStructureHandleNV",
    "vkCmdWriteAccelerationStructuresPropertiesNV",
    "vkCompileDeferredNV",
    "vkGetMemoryHostPointerPropertiesEXT",
    "vkCmdWriteBufferMarkerAMD",
    "vkCmdWriteBufferMarker2AMD",
    "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT",
    "vkGetCalibratedTimestampsEXT",
    "vkCmdDrawMeshTasksNV",
    "vkCmdDrawMeshTasksIndirectNV",
    "vkCmdDrawMeshTasksIndirectCountNV",
    "vkCmdSetExclusiveScissorEnableNV",
    "vkCmdSetExclusiveScissorNV",
    "vkCmdSetCheckpointNV",
    "vkGetQueueCheckpointDataNV",
    "vkGetQueueCheckpointData2NV",
    "vkSetSwapchainPresentTimingQueueSizeEXT",
    "vkGetSwapchainTimingPropertiesEXT",
    "vkGetSwapchainTimeDomainPropertiesEXT",
    "vkGetPastPresentationTimingEXT",
    "vkInitializePerformanceApiINTEL",
    "vkUninitializePerformanceApiINTEL",
    "vkCmdSetPerformanceMarkerINTEL",
    "vkCmdSetPerformanceStreamMarkerINTEL",
    "vkCmdSetPerformanceOverrideINTEL",
    "vkAcquirePerformanceConfigurationINTEL",
    "vkReleasePerformanceConfigurationINTEL",
    "vkQueueSetPerformanceConfigurationINTEL",
    "vkGetPerformanceParameterINTEL",
    "vkSetLocalDimmingAMD",
    "vkGetBufferDeviceAddressEXT",
    "vkGetPhysicalDeviceToolPropertiesEXT",
    "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV",
    "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV",
    "vkCmdSetLineStippleEXT",
    "vkResetQueryPoolEXT",
    "vkCmdSetCullModeEXT",
    "vkCmdSetFrontFaceEXT",
    "vkCmdSetPrimitiveTopologyEXT",
    "vkCmdSetViewportWithCountEXT",
    "vkCmdSetScissorWithCountEXT",
    "vkCmdBindVertexBuffers2EXT",
    "vkCmdSetDepthTestEnableEXT",
    "vkCmdSetDepthWriteEnableEXT",
    "vkCmdSetDepthCompareOpEXT",
    "vkCmdSetDepthBoundsTestEnableEXT",
    "vkCmdSetStencilTestEnableEXT",
    "vkCmdSetStencilOpEXT",
    "vkCopyMemoryToImageEXT",
    "vkCopyImageToMemoryEXT",
    "vkCopyImageToImageEXT",
    "vkTransitionImageLayoutEXT",
    "vkGetImageSubresourceLayout2EXT",
    "vkReleaseSwapchainImagesEXT",
    "vkGetGeneratedCommandsMemoryRequirementsNV",
    "vkCmdPreprocessGeneratedCommandsNV",
    "vkCmdExecuteGeneratedCommandsNV",
    "vkCmdBindPipelineShaderGroupNV",
    "vkCreateIndirectCommandsLayoutNV",
    "vkDestroyIndirectCommandsLayoutNV",
    "vkCmdSetDepthBias2EXT",
    "vkCreatePrivateDataSlotEXT",
    "vkDestroyPrivateDataSlotEXT",
    "vkSetPrivateDataEXT",
    "vkGetPrivateDataEXT",
    "vkQueueSetPerfHintQCOM",
    "vkCmdDispatchTileQCOM",
    "vkCmdBeginPerTileExecutionQCOM",
    "vkCmdEndPerTileExecutionQCOM",
    "vkSetLatencySleepModeLegacyNV",
    "vkLatencySleepLegacyNV",
    "vkSetLatencyMarkerLegacyNV",
    "vkGetLatencyTimingsLegacyNV",
    "vkQueueNotifyOutOfBandLegacyNV",
    "vkGetSleepStatusLegacyNV",
    "vkShutdownLatencyDeviceLegacyNV",
    "vkGetDescriptorSetLayoutSizeEXT",
    "vkGetDescriptorSetLayoutBindingOffsetEXT",
    "vkGetDescriptorEXT",
    "vkCmdBindDescriptorBuffersEXT",
    "vkCmdSetDescriptorBufferOffsetsEXT",
    "vkCmdBindDescriptorBufferEmbeddedSamplersEXT",
    "vkGetBufferOpaqueCaptureDescriptorDataEXT",
    "vkGetImageOpaqueCaptureDescriptorDataEXT",
    "vkGetImageViewOpaqueCaptureDescriptorDataEXT",
    "vkGetSamplerOpaqueCaptureDescriptorDataEXT",
    "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT",
    "vkCmdSetFragmentShadingRateEnumNV",
    "vkGetDeviceFaultInfoEXT",
    "vkCmdSetVertexInputEXT",
    "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI",
    "vkCmdSubpassShadingHUAWEI",
    "vkCmdBindInvocationMaskHUAWEI",
    "vkGetMemoryRemoteAddressNV",
    "vkGetPipelinePropertiesEXT",
    "vkCmdSetPatchControlPointsEXT",
    "vkCmdSetRasterizerDiscardEnableEXT",
    "vkCmdSetDepthBiasEnableEXT",
    "vkCmdSetLogicOpEXT",
    "vkCmdSetPrimitiveRestartEnableEXT",
    "vkCmdSetColorWriteEnableEXT",
    "vkCmdDrawMultiEXT",
    "vkCmdDrawMultiIndexedEXT",
    "vkCreateMicromapEXT",
    "vkDestroyMicromapEXT",
    "vkCmdBuildMicromapsEXT",
    "vkBuildMicromapsEXT",
    "vkCopyMicromapEXT",
    "vkCopyMicromapToMemoryEXT",
    "vkCopyMemoryToMicromapEXT",
    "vkWriteMicromapsPropertiesEXT",
    "vkCmdCopyMicromapEXT",
    "vkCmdCopyMicromapToMemoryEXT",
    "vkCmdCopyMemoryToMicromapEXT",
    "vkCmdWriteMicromapsPropertiesEXT",
    "vkGetDeviceMicromapCompatibilityEXT",
    "vkGetMicromapBuildSizesEXT",
    "vkCmdDrawClusterHUAWEI",
    "vkCmdDrawClusterIndirectHUAWEI",
    "vkSetDeviceMemoryPriorityEXT",
    "vkCmdSetDispatchParametersARM",
    "vkGetDescriptorSetLayoutHostMappingInfoVALVE",
    "vkGetDescriptorSetHostMappingVALVE",
    "vkCmdCopyMemoryIndirectNV",
    "vkCmdCopyMemoryToImageIndirectNV",
    "vkCmdDecompressMemoryNV",
    "vkCmdDecompressMemoryIndirectCountNV",
    "vkGetPipelineIndirectMemoryRequirementsNV",
    "vkCmdUpdatePipelineIndirectBufferNV",
    "vkGetPipelineIndirectDeviceAddressNV",
    "vkCmdSetDepthClampEnableEXT",
    "vkCmdSetPolygonModeEXT",
    "vkCmdSetRasterizationSamplesEXT",
    "vkCmdSetSampleMaskEXT",
    "vkCmdSetAlphaToCoverageEnableEXT",
    "vkCmdSetAlphaToOneEnableEXT",
    "vkCmdSetLogicOpEnableEXT",
    "vkCmdSetColorBlendEnableEXT",
    "vkCmdSetColorBlendEquationEXT",
    "vkCmdSetColorWriteMaskEXT",
    "vkCmdSetTessellationDomainOriginEXT",
    "vkCmdSetRasterizationStreamEXT",
    "vkCmdSetConservativeRasterizationModeEXT",
    "vkCmdSetExtraPrimitiveOverestimationSizeEXT",
    "vkCmdSetDepthClipEnableEXT",
    "vkCmdSetSampleLocationsEnableEXT",
    "vkCmdSetColorBlendAdvancedEXT",
    "vkCmdSetProvokingVertexModeEXT",
    "vkCmdSetLineRasterizationModeEXT",
    "vkCmdSetLineStippleEnableEXT",
    "vkCmdSetDepthClipNegativeOneToOneEXT",
    "vkCmdSetViewportWScalingEnableNV",
    "vkCmdSetViewportSwizzleNV",
    "vkCmdSetCoverageToColorEnableNV",
    "vkCmdSetCoverageToColorLocationNV",
    "vkCmdSetCoverageModulationModeNV",
    "vkCmdSetCoverageModulationTableEnableNV",
    "vkCmdSetCoverageModulationTableNV",
    "vkCmdSetShadingRateImageEnableNV",
    "vkCmdSetRepresentativeFragmentTestEnableNV",
    "vkCmdSetCoverageReductionModeNV",
    "vkCreateTensorARM",
    "vkDestroyTensorARM",
    "vkCreateTensorViewARM",
    "vkDestroyTensorViewARM",
    "vkGetTensorMemoryRequirementsARM",
    "vkBindTensorMemoryARM",
    "vkGetDeviceTensorMemoryRequirementsARM",
    "vkCmdCopyTensorARM",
    "vkGetPhysicalDeviceExternalTensorPropertiesARM",
    "vkGetTensorOpaqueCaptureDescriptorDataARM",
    "vkGetTensorViewOpaqueCaptureDescriptorDataARM",
    "vkGetShaderModuleIdentifierEXT",
    "vkGetShaderModuleCreateInfoIdentifierEXT",
    "vkGetPhysicalDeviceOpticalFlowImageFormatsNV",
    "vkCreateOpticalFlowSessionNV",
    "vkDestroyOpticalFlowSessionNV",
    "vkBindOpticalFlowSessionImageNV",
    "vkCmdOpticalFlowExecuteNV",
    "vkAntiLagUpdateAMD",
    "vkCreateShadersEXT",
    "vkDestroyShaderEXT",
    "vkGetShaderBinaryDataEXT",
    "vkCmdBindShadersEXT",
    "vkCmdSetDepthClampRangeEXT",
    "vkGetFramebufferTilePropertiesQCOM",
    "vkGetDynamicRenderingTilePropertiesQCOM",
    "vkGetPhysicalDeviceCooperativeVectorPropertiesNV",
    "vkConvertCooperativeVectorMatrixNV",
    "vkCmdConvertCooperativeVectorMatrixNV",
    "vkSetLatencySleepModeNV",
    "vkLatencySleepNV",
    "vkSetLatencyMarkerNV",
    "vkGetLatencyTimingsNV",
    "vkQueueNotifyOutOfBandNV",
    "vkCreateDataGraphPipelinesARM",
    "vkCreateDataGraphPipelineSessionARM",
    "vkGetDataGraphPipelineSessionBindPointRequirementsARM",
    "vkGetDataGraphPipelineSessionMemoryRequirementsARM",
    "vkBindDataGraphPipelineSessionMemoryARM",
    "vkDestroyDataGraphPipelineSessionARM",
    "vkCmdDispatchDataGraphARM",
    "vkGetDataGraphPipelineAvailablePropertiesARM",
    "vkGetDataGraphPipelinePropertiesARM",
    "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM",
    "vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM",
    "vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM",
    "vkCmdSetAttachmentFeedbackLoopEnableEXT",
    "vkCmdBindTileMemoryQCOM",
    "vkCmdDecompressMemoryEXT",
    "vkCmdDecompressMemoryIndirectCountEXT",
    "vkCreateExternalComputeQueueNV",
    "vkDestroyExternalComputeQueueNV",
    "vkGetExternalComputeQueueDataNV",
    "vkGetClusterAccelerationStructureBuildSizesNV",
    "vkCmdBuildClusterAccelerationStructureIndirectNV",
    "vkGetPartitionedAccelerationStructuresBuildSizesNV",
    "vkCmdBuildPartitionedAccelerationStructuresNV",
    "vkGetGeneratedCommandsMemoryRequirementsEXT",
    "vkCmdPreprocessGeneratedCommandsEXT",
    "vkCmdExecuteGeneratedCommandsEXT",
    "vkCreateIndirectCommandsLayoutEXT",
    "vkDestroyIndirectCommandsLayoutEXT",
    "vkCreateIndirectExecutionSetEXT",
    "vkDestroyIndirectExecutionSetEXT",
    "vkUpdateIndirectExecutionSetPipelineEXT",
    "vkUpdateIndirectExecutionSetShaderEXT",
    "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV",
    "vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM",
    "vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM",
    "vkCreateShaderInstrumentationARM",
    "vkDestroyShaderInstrumentationARM",
    "vkCmdBeginShaderInstrumentationARM",
    "vkCmdEndShaderInstrumentationARM",
    "vkGetShaderInstrumentationValuesARM",
    "vkClearShaderInstrumentationMetricsARM",
    "vkCmdEndRendering2EXT",
    "vkCmdBeginCustomResolveEXT",
    "vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM",
    "vkCmdSetComputeOccupancyPriorityNV",
    "vkCmdSetPrimitiveRestartIndexEXT",
    "vkCreateAccelerationStructureKHR",
    "vkDestroyAccelerationStructureKHR",
    "vkCmdBuildAccelerationStructuresKHR",
    "vkCmdBuildAccelerationStructuresIndirectKHR",
    "vkBuildAccelerationStructuresKHR",
    "vkCopyAccelerationStructureKHR",
    "vkCopyAccelerationStructureToMemoryKHR",
    "vkCopyMemoryToAccelerationStructureKHR",
    "vkWriteAccelerationStructuresPropertiesKHR",
    "vkCmdCopyAccelerationStructureKHR",
    "vkCmdCopyAccelerationStructureToMemoryKHR",
    "vkCmdCopyMemoryToAccelerationStructureKHR",
    "vkGetAccelerationStructureDeviceAddressKHR",
    "vkCmdWriteAccelerationStructuresPropertiesKHR",
    "vkGetDeviceAccelerationStructureCompatibilityKHR",
    "vkGetAccelerationStructureBuildSizesKHR",
    "vkCmdTraceRaysKHR",
    "vkCreateRayTracingPipelinesKHR",
    "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR",
    "vkCmdTraceRaysIndirectKHR",
    "vkGetRayTracingShaderGroupStackSizeKHR",
    "vkCmdSetRayTracingPipelineStackSizeKHR",
    "vkCmdDrawMeshTasksEXT",
    "vkCmdDrawMeshTasksIndirectEXT",
    "vkCmdDrawMeshTasksIndirectCountEXT",
};

// Exhaustively call every one of gpa_helper.c's ~230 hand-checked core instance trampoline names through
// the real vkGetInstanceProcAddr entry point and confirm each still resolves - i.e. the mechanical
// hash-prefilter transformation of that file didn't drop, misname, or mis-hash any branch.
TEST(GetProcAddr, ExhaustiveCoreInstanceTrampolineNames) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    // Every name must also resolve to a pointer distinct from every other name's - not just non-null -
    // so a hand-edit that pairs one branch's hash/strcmp condition with a different branch's return
    // statement (e.g. vkCreateBuffer's check returning vkCreateImage) can't hide behind "still non-null".
    std::set<PFN_vkVoidFunction> distinct_pointers;
    for (const auto* name : kGpaHelperCoreInstanceNames) {
        PFN_vkVoidFunction f = inst.load(name);
        ASSERT_NE(nullptr, f) << "Expected a non-null trampoline for " << name;
        ASSERT_TRUE(distinct_pointers.insert(f).second) << "Trampoline for " << name << " is not distinct from an earlier name's";
    }
}

// Unlike the three generated lookup functions, which get a check_no_hash_collisions() call at generation
// time, gpa_helper.c is hand-maintained with no equivalent - a real collision there would only ever surface
// as a bare "duplicate case value" compiler error naming neither string. This test recreates that same
// diagnostic in C++ over gpa_helper.c's own name list, so a colliding addition fails here with both names
// named instead of as an inscrutable compiler error.
TEST(GetProcAddr, GpaHelperCoreInstanceNamesHaveNoHashCollisions) {
    std::unordered_map<uint32_t, const char*> hash_to_name;
    for (const auto* name : kGpaHelperCoreInstanceNames) {
        uint32_t hash = loader_hash_string(name);
        auto it = hash_to_name.find(hash);
        if (it != hash_to_name.end()) {
            FAIL() << "Hash collision between \"" << it->second << "\" and \"" << name << "\" (both hash to 0x" << std::hex << hash
                   << ") - gpa_helper.c's hand-written switch would fail to compile with a duplicate case label";
        }
        hash_to_name.emplace(hash, name);
    }
}

// Exhaustively call every core Vulkan 1.0-1.4 name that loader_lookup_device_dispatch_table() (the
// generated device gpa fast path used by vkGetDeviceProcAddr) hand-checks, confirming the hash-prefilter
// added in front of each of its ~200 strcmp checks didn't drop or mis-hash any of them.
TEST(GetDeviceProcAddr, ExhaustiveCoreDeviceDispatchNames) {
    FrameworkEnvironment env{};
    auto& test_physical_device = env.add_icd(TEST_ICD_PATH_VERSION_2, {}, ManifestICD{}.set_api_version(VK_API_VERSION_1_4))
                                     .set_icd_api_version(VK_API_VERSION_1_4)
                                     .add_and_get_physical_device(PhysicalDevice{}.set_api_version(VK_API_VERSION_1_4));
    // Every name needs a registered mock function so the ICD reports it as supported. Use a distinct
    // pointer value per name (a captureless lambda would collapse to the same pointer every iteration).
    for (size_t i = 0; i < kDeviceDispatchCoreNames.size(); i++) {
        auto mock_ptr = reinterpret_cast<PFN_vkVoidFunction>(static_cast<uintptr_t>(0x1000 + i * 8));
        test_physical_device.add_device_function(VulkanFunction{kDeviceDispatchCoreNames[i], mock_ptr});
    }

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());

    // Every name must resolve to a pointer distinct from every other name's - not just non-null - so a
    // hand-edit that pairs one branch's hash/strcmp condition with a different branch's return statement
    // (e.g. vkGetDeviceQueue's check returning table->QueueSubmit) can't hide behind "still non-null".
    std::set<PFN_vkVoidFunction> distinct_pointers;
    for (const auto* name : kDeviceDispatchCoreNames) {
        PFN_vkVoidFunction f = dev->vkGetDeviceProcAddr(dev.dev, name);
        ASSERT_NE(nullptr, f) << "Expected a non-null dispatch table entry for " << name;
        ASSERT_TRUE(distinct_pointers.insert(f).second)
            << "Dispatch entry for " << name << " is not distinct from an earlier name's";
    }
}

// Real, engineered FNV-1a collisions against two of the actual embedded hash constants exercised above -
// one hitting trampoline_get_proc_addr()'s hand-checked instance chain, one hitting
// loader_lookup_device_dispatch_table()'s generated device chain - found offline by brute-force search
// (see loader_hash_string_tests.cpp for the synthetic, hand-simulated version of this same guarantee).
// Unlike that test, this one drives the collision through the real vkGet{Instance,Device}ProcAddr entry
// points, proving the switch/case-guarded strcmp actually protects the production lookup, not just the
// pattern.
TEST(GetProcAddr, EngineeredHashCollisionsAreRejectedByRealLookup) {
    // trampoline_get_proc_addr() (instance chain) hashes the full "vk"-prefixed name, so the instance
    // collider must collide with "vkCreateBuffer" on the FULL name. loader_lookup_device_dispatch_table()
    // (device chain) strips the "vk" prefix before hashing (see loader/trampoline.c's vkGetDeviceProcAddr),
    // so the device collider must instead collide with "GetDeviceQueue" on the STRIPPED name - a collider
    // engineered against the full "vkGetDeviceQueue" name would never reach that hash space at all.
    const char* kInstanceCollider = "vkZzTs0sr";  // collides with "vkCreateBuffer"
    const char* kDeviceCollider = "vkF5TyK";      // stripped "F5TyK" collides with stripped "GetDeviceQueue"
    ASSERT_NE(0, strcmp(kInstanceCollider, "vkCreateBuffer"));
    ASSERT_NE(0, strcmp(kDeviceCollider + 2, "GetDeviceQueue"));
    ASSERT_EQ(loader_hash_string(kInstanceCollider), loader_hash_string("vkCreateBuffer"));
    ASSERT_EQ(loader_hash_string(kDeviceCollider + 2), loader_hash_string("GetDeviceQueue"));

    FrameworkEnvironment env{};
    auto& test_physical_device = env.add_icd(TEST_ICD_PATH_VERSION_2, {}, ManifestICD{}.set_api_version(VK_API_VERSION_1_4))
                                     .set_icd_api_version(VK_API_VERSION_1_4)
                                     .add_and_get_physical_device(PhysicalDevice{}.set_api_version(VK_API_VERSION_1_4));
    auto queue_ptr = reinterpret_cast<PFN_vkVoidFunction>(static_cast<uintptr_t>(0x2000));
    test_physical_device.add_device_function(VulkanFunction{"vkGetDeviceQueue", queue_ptr});

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    // Real name still resolves through trampoline_get_proc_addr()'s hash-then-strcmp chain...
    PFN_vkVoidFunction real_instance_func = inst.load("vkCreateBuffer");
    ASSERT_NE(nullptr, real_instance_func);
    // ...but the hash-colliding impostor must not, since the hash match alone is never enough to return a
    // branch's pointer - strcmp still has to agree.
    PFN_vkVoidFunction colliding_instance_func = inst.load(kInstanceCollider);
    ASSERT_EQ(nullptr, colliding_instance_func);

    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());

    // Same guarantee through the generated device dispatch table lookup: the real name still resolves to
    // something (vkGetDeviceQueue wraps the returned VkQueue, so this isn't necessarily the raw mock
    // pointer registered above - see ExhaustiveCoreDeviceDispatchNames)...
    PFN_vkVoidFunction real_device_func = dev->vkGetDeviceProcAddr(dev.dev, "vkGetDeviceQueue");
    ASSERT_NE(nullptr, real_device_func);
    // ...while its hash-colliding impostor (colliding on the stripped-name hash actually used by
    // loader_lookup_device_dispatch_table()) must not resolve to anything, least of all vkGetDeviceQueue's
    // pointer.
    PFN_vkVoidFunction colliding_device_func = dev->vkGetDeviceProcAddr(dev.dev, kDeviceCollider);
    ASSERT_EQ(nullptr, colliding_device_func);
}

// Names that must never resolve, chosen to specifically probe the hash pre-filter for false positives:
// truncations and extensions of real names, wrong case, empty/near-empty strings, non-"vk"-prefixed
// strings, and single-character near-misses of real core commands taken from the start, middle, and end
// of the hand-checked list in gpa_helper.c. None of these are real Vulkan commands; if the hash pre-filter
// ever short-circuited past a real strcmp confirmation (e.g. compared only the hash) one of these could
// slip through as a false positive.
static const std::vector<const char*> kNamesThatMustNeverResolve = {
    "",
    "vk",
    "v",
    "CreateBuffer",
    "glCreateBuffer",
    "xkCreateBuffer",
    "vkCreateBuffe",     // truncated real name
    "vkDestroyInstanc",  // truncated real name
    "vkCreateBufferX",   // extended real name
    "vkDestroyInstanceX",
    "vkcreatebuffer",           // wrong case
    "VKCREATEBUFFER",           // wrong case
    "vkCREATEBuffer",           // wrong case
    "vkGetInstanceProcAddx",    // near-miss: start of gpa_helper.c list, last char substituted
    "vkCmdSetViewpost",         // near-miss: middle of the list, one char substituted
    "vkCmdBeginRenderPasS",     // near-miss: middle of the list, one char substituted
    "vkCreateBuffre",           // near-miss: adjacent-character transposition
    "vkTransitionImageLayous",  // near-miss: end of gpa_helper.c list, last char substituted
};

TEST(GetProcAddr, RejectsMalformedAndNearMissNames) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());

    for (const auto* name : kNamesThatMustNeverResolve) {
        PFN_vkVoidFunction from_instance = inst.load(name);
        ASSERT_EQ(nullptr, from_instance) << "Expected vkGetInstanceProcAddr to reject " << name;
        ASSERT_EQ(nullptr, dev->vkGetDeviceProcAddr(dev.dev, name)) << "Expected vkGetDeviceProcAddr to reject " << name;
    }
}

// Mutators used by FuzzOracleAgainstNaiveStrcmpChain below. Each takes a real core command name and
// perturbs it one way (truncate, extend, substitute a byte, swap two adjacent bytes, or flip one bit) to
// produce a string that is usually - but, given enough iterations, not always - different from every real
// command name.
static std::string mutate_truncate(const std::string& s, std::mt19937& rng) {
    if (s.size() <= 1) return s;
    std::uniform_int_distribution<size_t> dist(1, s.size() - 1);
    return s.substr(0, dist(rng));
}
static std::string mutate_extend(const std::string& s, std::mt19937& rng) {
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::string result = s;
    result.push_back(static_cast<char>(char_dist(rng)));
    return result;
}
static std::string mutate_substitute(const std::string& s, std::mt19937& rng) {
    if (s.empty()) return s;
    std::string result = s;
    std::uniform_int_distribution<size_t> idx_dist(0, s.size() - 1);
    std::uniform_int_distribution<int> char_dist(33, 126);
    size_t idx = idx_dist(rng);
    char replacement;
    do {
        replacement = static_cast<char>(char_dist(rng));
    } while (replacement == result[idx]);
    result[idx] = replacement;
    return result;
}
static std::string mutate_swap_adjacent(const std::string& s, std::mt19937& rng) {
    if (s.size() < 2) return s;
    std::uniform_int_distribution<size_t> idx_dist(0, s.size() - 2);
    std::string result = s;
    size_t idx = idx_dist(rng);
    std::swap(result[idx], result[idx + 1]);
    return result;
}
static std::string mutate_bit_flip(const std::string& s, std::mt19937& rng) {
    if (s.empty()) return s;
    std::string result = s;
    std::uniform_int_distribution<size_t> idx_dist(0, s.size() - 1);
    std::uniform_int_distribution<int> bit_dist(0, 7);
    size_t idx = idx_dist(rng);
    result[idx] = static_cast<char>(result[idx] ^ (1 << bit_dist(rng)));
    return result;
}

template <size_t N>
static bool exists_in(const std::array<const char*, N>& names, const std::string& s) {
    for (const auto* n : names) {
        if (s == n) return true;
    }
    return false;
}

// Property-style fuzz test: generate a large number of mutated real command names (deterministically, via
// a fixed seed, so any failure is reproducible) and confirm vkGetInstanceProcAddr's answer always agrees
// with a naive reference oracle - a plain linear strcmp scan over the exact same name list the hash
// pre-filter was mechanically derived from. This exercises far more of the input space than the hand-picked
// cases in RejectsMalformedAndNearMissNames/ExhaustiveCoreInstanceTrampolineNames can cover, and is the
// strongest available guarantee that hashing introduced no false positives or false negatives.
TEST(GetProcAddr, FuzzOracleAgainstNaiveStrcmpChain) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    std::mt19937 rng(0x16310001u);
    using Mutator = std::string (*)(const std::string&, std::mt19937&);
    const Mutator mutators[] = {mutate_truncate, mutate_extend, mutate_substitute, mutate_swap_adjacent, mutate_bit_flip};
    constexpr size_t kMutatorCount = sizeof(mutators) / sizeof(mutators[0]);

    constexpr int kIterations = 5000;
    for (int i = 0; i < kIterations; ++i) {
        const char* base = kGpaHelperCoreInstanceNames[static_cast<size_t>(i) % kGpaHelperCoreInstanceNames.size()];
        Mutator mutate = mutators[static_cast<size_t>(i) % kMutatorCount];
        std::string candidate = mutate(base, rng);

        bool expected_to_resolve = exists_in(kGpaHelperCoreInstanceNames, candidate);
        PFN_vkVoidFunction actual = inst.load(candidate.c_str());
        if (expected_to_resolve) {
            ASSERT_NE(nullptr, actual) << "candidate \"" << candidate << "\" (from \"" << base
                                       << "\") matches a known core command name but failed to resolve";
        } else {
            ASSERT_EQ(nullptr, actual) << "candidate \"" << candidate << "\" (from \"" << base
                                       << "\") is not a known core command name but resolved to a non-null pointer";
        }
    }
}

// Exhaustively call every one of the 481 portable names in kExtensionInstanceGpaNames through the real
// vkGetInstanceProcAddr entry point (no extensions enabled, no ICD-level mocking needed - see that array's
// comment for why). Together with ExhaustiveCoreInstanceTrampolineNames and ExhaustiveCoreDeviceDispatchNames,
// this gives all three switch-converted lookup functions exhaustive per-name regression coverage.
TEST(GetProcAddr, ExhaustiveExtensionInstanceGpaNames) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    std::set<PFN_vkVoidFunction> distinct_pointers;
    for (const auto* name : kExtensionInstanceGpaNames) {
        PFN_vkVoidFunction f = inst.load(name);
        ASSERT_NE(nullptr, f) << "Expected a non-null trampoline for " << name;
        ASSERT_TRUE(distinct_pointers.insert(f).second) << "Trampoline for " << name << " is not distinct from an earlier name's";
    }
}

// Same property-style fuzz as FuzzOracleAgainstNaiveStrcmpChain, but mutating kExtensionInstanceGpaNames so
// it also confirms *positive* resolution for extension_instance_gpa()'s switch, not just the incidental
// non-resolution the other fuzz test gets when a mutated core name happens to fall through to it.
//
// The oracle checks both name arrays, not just kExtensionInstanceGpaNames: GIPA tries gpa_helper.c's core
// chain first, and a single-character mutation of a versioned extension name often lands on exactly its
// unversioned core predecessor (e.g. truncating "vkCmdWaitEvents2KHR" produces the real core command
// "vkCmdWaitEvents") - checking only the extension array would misreport those as false positives.
TEST(GetProcAddr, FuzzExtensionInstanceGpaOracleAgainstNaiveStrcmpChain) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_4);
    inst.CheckCreate();

    std::mt19937 rng(0x16310002u);
    using Mutator = std::string (*)(const std::string&, std::mt19937&);
    const Mutator mutators[] = {mutate_truncate, mutate_extend, mutate_substitute, mutate_swap_adjacent, mutate_bit_flip};
    constexpr size_t kMutatorCount = sizeof(mutators) / sizeof(mutators[0]);

    constexpr int kIterations = 5000;
    for (int i = 0; i < kIterations; ++i) {
        const char* base = kExtensionInstanceGpaNames[static_cast<size_t>(i) % kExtensionInstanceGpaNames.size()];
        Mutator mutate = mutators[static_cast<size_t>(i) % kMutatorCount];
        std::string candidate = mutate(base, rng);

        bool expected_to_resolve =
            exists_in(kExtensionInstanceGpaNames, candidate) || exists_in(kGpaHelperCoreInstanceNames, candidate);
        PFN_vkVoidFunction actual = inst.load(candidate.c_str());
        if (expected_to_resolve) {
            ASSERT_NE(nullptr, actual) << "candidate \"" << candidate << "\" (from \"" << base
                                       << "\") matches a known extension or core command name but failed to resolve";
        } else {
            ASSERT_EQ(nullptr, actual) << "candidate \"" << candidate << "\" (from \"" << base
                                       << "\") is not a known extension or core command name but resolved to a non-null pointer";
        }
    }
}

// Verify that the various ways to get vkGetInstanceProcAddr return the same value
TEST(GetProcAddr, VerifyGetInstanceProcAddr) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        // NOTE: The vulkan_functions are queried using the platform get proc addr from the loader.  So we'll compare
        //       that to what is returned by asking it what the various Vulkan get proc addr functions are.
        PFN_vkGetInstanceProcAddr gipa_loader = env.vulkan_functions.vkGetInstanceProcAddr;
        PFN_vkGetInstanceProcAddr gipa_queried = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
            env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetInstanceProcAddr"));
        assert_gipa_equivalent(gipa_loader, gipa_queried);
    }

    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(VK_API_VERSION_1_3);
        inst.CheckCreate();

        // NOTE: The vulkan_functions are queried using the platform get proc addr from the loader.  So we'll compare
        //       that to what is returned by asking it what the various Vulkan get proc addr functions are.
        PFN_vkGetInstanceProcAddr gipa_loader = env.vulkan_functions.vkGetInstanceProcAddr;
        PFN_vkGetInstanceProcAddr gipa_queried = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
            env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetInstanceProcAddr"));
        assert_gipa_equivalent(gipa_loader, gipa_queried);
    }
}

// Verify that the various ways to get vkGetDeviceProcAddr return the same value
TEST(GetProcAddr, VerifyGetDeviceProcAddr) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    // NOTE: The vulkan_functions are queried using the platform get proc addr from the loader.  So we'll compare
    //       that to what is returned by asking it what the various Vulkan get proc addr functions are.
    PFN_vkGetDeviceProcAddr gdpa_loader = env.vulkan_functions.vkGetDeviceProcAddr;
    PFN_vkGetDeviceProcAddr gdpa_inst_queried = inst.load("vkGetDeviceProcAddr");
    assert_gdpa_equivalent(gdpa_loader, gdpa_inst_queried, VK_NULL_HANDLE);

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_dev);

    PFN_vkGetDeviceProcAddr gdpa_dev_queried = dev.load("vkGetDeviceProcAddr");
    assert_gdpa_equivalent(gdpa_loader, gdpa_dev_queried, dev);
}

// Load the global function pointers with and without a NULL vkInstance handle.
// Call the function to make sure it is callable, don't care about what is returned.
TEST(GetProcAddr, GlobalFunctions) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    auto& gipa = env.vulkan_functions.vkGetInstanceProcAddr;
    // global entry points with NULL instance handle
    {
        auto EnumerateInstanceExtensionProperties =
            reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(gipa(NULL, "vkEnumerateInstanceExtensionProperties"));
        handle_assert_has_value(EnumerateInstanceExtensionProperties);
        uint32_t ext_count = 0;
        ASSERT_EQ(VK_SUCCESS, EnumerateInstanceExtensionProperties("", &ext_count, nullptr));

        auto EnumerateInstanceLayerProperties =
            reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(gipa(NULL, "vkEnumerateInstanceLayerProperties"));
        handle_assert_has_value(EnumerateInstanceLayerProperties);
        uint32_t layer_count = 0;
        ASSERT_EQ(VK_SUCCESS, EnumerateInstanceLayerProperties(&layer_count, nullptr));

        auto EnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(gipa(NULL, "vkEnumerateInstanceVersion"));
        handle_assert_has_value(EnumerateInstanceVersion);
        uint32_t api_version = 0;
        EnumerateInstanceVersion(&api_version);

        auto GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(gipa(NULL, "vkGetInstanceProcAddr"));
        ASSERT_EQ(GetInstanceProcAddr,
                  reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetInstanceProcAddr(NULL, "vkGetInstanceProcAddr")));

        auto CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(gipa(NULL, "vkCreateInstance"));
        handle_assert_has_value(CreateInstance);
    }
    // Now create an instance and query the functions again - should work because the instance version is less than 1.2
    for (int i = 0; i <= 2; i++) {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.api_version = VK_MAKE_API_VERSION(0, 1, i, 0);
        inst.CheckCreate();

        PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties =
            inst.load("vkEnumerateInstanceExtensionProperties");
        handle_assert_has_value(EnumerateInstanceExtensionProperties);
        uint32_t ext_count = 0;
        ASSERT_EQ(VK_SUCCESS, EnumerateInstanceExtensionProperties("", &ext_count, nullptr));

        PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties = inst.load("vkEnumerateInstanceLayerProperties");
        handle_assert_has_value(EnumerateInstanceLayerProperties);
        uint32_t layer_count = 0;
        ASSERT_EQ(VK_SUCCESS, EnumerateInstanceLayerProperties(&layer_count, nullptr));

        PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion = inst.load("vkEnumerateInstanceVersion");
        handle_assert_has_value(EnumerateInstanceVersion);
        uint32_t api_version = 0;
        EnumerateInstanceVersion(&api_version);

        PFN_vkGetInstanceProcAddr GetInstanceProcAddr = inst.load("vkGetInstanceProcAddr");
        handle_assert_has_value(GetInstanceProcAddr);
        ASSERT_EQ(GetInstanceProcAddr,
                  reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetInstanceProcAddr(inst, "vkGetInstanceProcAddr")));

        PFN_vkCreateInstance CreateInstance = inst.load("vkCreateInstance");
        handle_assert_has_value(CreateInstance);
    }
    {
        // Create a 1.3 instance - now everything should return NULL
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.api_version = VK_MAKE_API_VERSION(0, 1, 3, 0);
        inst.CheckCreate();

        PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties =
            inst.load("vkEnumerateInstanceExtensionProperties");
        handle_assert_null(EnumerateInstanceExtensionProperties);

        PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties = inst.load("vkEnumerateInstanceLayerProperties");
        handle_assert_null(EnumerateInstanceLayerProperties);

        PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion = inst.load("vkEnumerateInstanceVersion");
        handle_assert_null(EnumerateInstanceVersion);

        PFN_vkCreateInstance CreateInstance = inst.load("vkCreateInstance");
        handle_assert_null(CreateInstance);

        PFN_vkGetInstanceProcAddr GetInstanceProcAddr = inst.load("vkGetInstanceProcAddr");
        assert_gipa_equivalent(env.vulkan_functions.vkGetInstanceProcAddr, GetInstanceProcAddr);
        ASSERT_EQ(GetInstanceProcAddr,
                  reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetInstanceProcAddr(inst, "vkGetInstanceProcAddr")));
        ASSERT_EQ(GetInstanceProcAddr,
                  reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetInstanceProcAddr(NULL, "vkGetInstanceProcAddr")));
        // get a non pre-instance function pointer
        PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices = inst.load("vkEnumeratePhysicalDevices");
        handle_assert_has_value(EnumeratePhysicalDevices);

        EnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(gipa(NULL, "vkEnumeratePhysicalDevices"));
        handle_assert_null(EnumeratePhysicalDevices);
    }
}

TEST(GetProcAddr, Verify10FunctionsFailToLoadWithSingleDriver) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2).add_physical_device({}).set_can_query_GetPhysicalDeviceFuncs(false);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
}

TEST(GetProcAddr, Verify10FunctionsLoadWithMultipleDrivers) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2).add_physical_device({});
    env.add_icd(TEST_ICD_PATH_VERSION_2).add_physical_device({}).set_can_query_GetPhysicalDeviceFuncs(false);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    inst.GetPhysDevs(1);
}

// Swapchain functions which require a terminator in all cases have situations where the driver may have a
// NULL function pointer but the loader shouldn't abort() if that is the case. Rather, it should log a message
// and return VK_SUCCESS to maintain previous behavior.
TEST(GetDeviceProcAddr, SwapchainFuncsWithTerminator) {
    FrameworkEnvironment env{};
    auto& test_physical_device =
        env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).setup_WSI().add_and_get_physical_device("physical_device_0");

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.add_extension("VK_EXT_debug_utils");
    inst.create_info.setup_WSI();
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));

    VkSurfaceKHR surface2{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface2));

    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));
    auto phys_dev = inst.GetPhysDev();
    {
        DeviceWrapper dev{inst};
        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));
        DeviceFunctions dev_funcs{env.vulkan_functions, dev};

        PFN_vkCreateSwapchainKHR CreateSwapchainKHR = dev.load("vkCreateSwapchainKHR");
        PFN_vkCreateSwapchainKHR inst_CreateSwapchainKHR = inst.load("vkCreateSwapchainKHR");
        PFN_vkGetDeviceGroupSurfacePresentModesKHR GetDeviceGroupSurfacePresentModesKHR =
            dev.load("vkGetDeviceGroupSurfacePresentModesKHR");
        PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR = dev.load("vkCreateSharedSwapchainsKHR");
        ASSERT_FALSE(CreateSwapchainKHR);
        ASSERT_TRUE(inst_CreateSwapchainKHR);
        ASSERT_FALSE(GetDeviceGroupSurfacePresentModesKHR);
        ASSERT_FALSE(CreateSharedSwapchainsKHR);

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;

        VkSwapchainKHR swapchain{};
        log.logger.clear();
        ASSERT_FALSE(dev_funcs.vkDestroySwapchainKHR);

        // try to call the vkCreateSwapchainKHR acquired from the instance - this *should* abort due to not enabling the extension
        ASSERT_DEATH(inst_CreateSwapchainKHR(dev.dev, &info, nullptr, &swapchain),
                     "vkCreateSwapchainKHR: Driver's function pointer was NULL, returning VK_SUCCESS. Was the VK_KHR_swapchain "
                     "extension enabled?");

        log.logger.clear();
        ASSERT_FALSE(dev_funcs.vkDestroySwapchainKHR);
    }
    test_physical_device.add_extensions({"VK_KHR_swapchain", "VK_KHR_display_swapchain", "VK_EXT_debug_marker"});
    {
        DeviceWrapper dev{inst};
        dev.create_info.add_extensions({"VK_KHR_swapchain", "VK_KHR_display_swapchain", "VK_EXT_debug_marker"});
        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));
        DeviceFunctions dev_funcs{env.vulkan_functions, dev};

        PFN_vkCreateSwapchainKHR CreateSwapchainKHR = dev.load("vkCreateSwapchainKHR");
        PFN_vkCreateSwapchainKHR inst_CreateSwapchainKHR = inst.load("vkCreateSwapchainKHR");
        PFN_vkGetDeviceGroupSurfacePresentModesKHR GetDeviceGroupSurfacePresentModesKHR =
            dev.load("vkGetDeviceGroupSurfacePresentModesKHR");
        PFN_vkCreateSharedSwapchainsKHR CreateSharedSwapchainsKHR = dev.load("vkCreateSharedSwapchainsKHR");
        ASSERT_TRUE(CreateSwapchainKHR);
        ASSERT_TRUE(inst_CreateSwapchainKHR);
        ASSERT_TRUE(GetDeviceGroupSurfacePresentModesKHR);
        ASSERT_TRUE(CreateSharedSwapchainsKHR);
        ASSERT_TRUE(dev_funcs.vkDestroySwapchainKHR);

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;

        VkSwapchainKHR swapchain{};
        CreateSwapchainKHR(dev.dev, &info, nullptr, &swapchain);
        ASSERT_FALSE(
            log.find("vkCreateSwapchainKHR: Driver's function pointer was NULL, returning VK_SUCCESS. Was the VK_KHR_swapchain "
                     "extension enabled?"));
        log.logger.clear();
        dev_funcs.vkDestroySwapchainKHR(dev.dev, swapchain, nullptr);
        inst_CreateSwapchainKHR(dev.dev, &info, nullptr, &swapchain);
        ASSERT_FALSE(
            log.find("vkCreateSwapchainKHR: Driver's function pointer was NULL, returning VK_SUCCESS. Was the VK_KHR_swapchain "
                     "extension enabled?"));
        log.logger.clear();
        dev_funcs.vkDestroySwapchainKHR(dev.dev, swapchain, nullptr);

        VkDeviceGroupPresentModeFlagsKHR modes{};
        GetDeviceGroupSurfacePresentModesKHR(dev.dev, surface, &modes);

        std::array<VkSwapchainCreateInfoKHR, 2> infos{};
        infos[0] = info;
        infos[1].sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        infos[1].surface = surface2;

        std::array<VkSwapchainKHR, 2> swapchains{};
        ASSERT_EQ(VK_SUCCESS, CreateSharedSwapchainsKHR(dev.dev, static_cast<uint32_t>(swapchains.size()), infos.data(), nullptr,
                                                        swapchains.data()));
        ASSERT_FALSE(log.find("vkCreateSharedSwapchainsKHR Terminator: No VkSurfaceKHR objects were created"));
    }
    env.vulkan_functions.vkDestroySurfaceKHR(inst.inst, surface, nullptr);
    env.vulkan_functions.vkDestroySurfaceKHR(inst.inst, surface2, nullptr);
}

// Verify that the various ways to get vkGetDeviceProcAddr return the same value
TEST(GetProcAddr, PreserveLayerGettingVkCreateDeviceWithNullInstance) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).add_physical_device("physical_device_0");

    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name("VK_LAYER_technically_buggy_layer")
                                                             .set_description("actually_layer_1")
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("if_you_can")));
    env.get_test_layer().set_buggy_query_of_vkCreateDevice(true);
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_dev);
}

// The following tests - AppQueries11FunctionsWhileOnlyEnabling10, AppQueries12FunctionsWhileOnlyEnabling11, and
// AppQueries13FunctionsWhileOnlyEnabling12 - check that vkGetDeviceProcAddr only returning functions from core versions up to
// the apiVersion declared in VkApplicationInfo. Function querying should succeed if VK_KHR_maintenance_5 is not enabled, and they
// should return zero when that extension is enabled.

TEST(GetDeviceProcAddr, AppQueries11FunctionsWhileOnlyEnabling10) {
    FrameworkEnvironment env{};
    auto& test_physical_device =
        env.add_icd(TEST_ICD_PATH_VERSION_2, {}, ManifestICD{}.set_api_version(VK_API_VERSION_1_1))
            .set_icd_api_version(VK_API_VERSION_1_1)
            .add_and_get_physical_device(
                PhysicalDevice{}.set_api_version(VK_API_VERSION_1_1).add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME));

    std::vector<const char*> functions = {"vkGetDeviceQueue2", "vkCmdDispatchBase", "vkCreateDescriptorUpdateTemplate"};
    for (const auto& f : functions) {
        test_physical_device.add_device_function(VulkanFunction{f, [] {}});
    }
    {  // doesn't enable the feature or extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 0, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.CheckCreate(inst.GetPhysDev());
        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // doesn't enable the feature
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 0, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.CheckCreate(inst.GetPhysDev());
        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // enables the feature and extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 0, 0);
        inst.CheckCreate();

        VkPhysicalDeviceMaintenance5FeaturesKHR features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR;
        features.maintenance5 = VK_TRUE;

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.create_info.dev.pNext = &features;
        dev.CheckCreate(inst.GetPhysDev());
        for (const auto& f : functions) {
            ASSERT_EQ(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
}

TEST(GetDeviceProcAddr, AppQueries12FunctionsWhileOnlyEnabling11) {
    FrameworkEnvironment env{};
    auto& test_physical_device =
        env.add_icd(TEST_ICD_PATH_VERSION_2, {}, ManifestICD{}.set_api_version(VK_API_VERSION_1_2))
            .set_icd_api_version(VK_API_VERSION_1_2)
            .add_and_get_physical_device(
                PhysicalDevice{}.set_api_version(VK_API_VERSION_1_2).add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME));
    std::vector<const char*> functions = {"vkCmdDrawIndirectCount", "vkCmdNextSubpass2", "vkGetBufferDeviceAddress",
                                          "vkGetDeviceMemoryOpaqueCaptureAddress"};
    for (const auto& f : functions) {
        test_physical_device.add_device_function(VulkanFunction{f, [] {}});
    }
    {  // doesn't enable the feature or extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // doesn't enable the feature
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // enables the feature and extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();

        VkPhysicalDeviceMaintenance5FeaturesKHR features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR;
        features.maintenance5 = VK_TRUE;

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.create_info.dev.pNext = &features;
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_EQ(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
}

TEST(GetDeviceProcAddr, AppQueries13FunctionsWhileOnlyEnabling12) {
    FrameworkEnvironment env{};
    auto& test_physical_device =
        env.add_icd(TEST_ICD_PATH_VERSION_2, {}, ManifestICD{}.set_api_version(VK_API_VERSION_1_3))
            .set_icd_api_version(VK_API_VERSION_1_3)
            .add_and_get_physical_device(
                PhysicalDevice{}.set_api_version(VK_API_VERSION_1_3).add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME));
    std::vector<const char*> functions = {"vkCreatePrivateDataSlot", "vkGetDeviceBufferMemoryRequirements", "vkCmdWaitEvents2",
                                          "vkGetDeviceImageSparseMemoryRequirements"};

    for (const auto& f : functions) {
        test_physical_device.add_device_function(VulkanFunction{f, [] {}});
    }
    {  // doesn't enable the feature or extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 2, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // doesn't enable the feature
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 2, 0);
        inst.CheckCreate();

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_NE(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
    {  // enables the feature and extension
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 2, 0);
        inst.CheckCreate();

        VkPhysicalDeviceMaintenance5FeaturesKHR features{};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR;
        features.maintenance5 = VK_TRUE;

        DeviceWrapper dev{inst};
        dev.create_info.add_extension(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        dev.create_info.dev.pNext = &features;
        dev.CheckCreate(inst.GetPhysDev());

        for (const auto& f : functions) {
            ASSERT_EQ(nullptr, dev->vkGetDeviceProcAddr(dev.dev, f));
        }
    }
}
