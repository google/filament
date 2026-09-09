/*
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "gpa_helper.h"

#include <string.h>

#include "debug_utils.h"
#include "unknown_function_handling.h"
#include "wsi.h"

// Unlike the generated dispatch tables in loader/generated/vk_loader_extensions.c, this switch is
// hand-maintained: there is no generator or check_no_hash_collisions() call guarding it, so a real hash
// collision among the cases below would surface only as a bare "duplicate case value" compiler error naming
// neither string. Every case still gates its return on strcmp(), so a wrong or stale hash constant can never
// cause a wrong lookup - at worst the case silently never matches and the name falls through to the
// extension/unknown-function handling below, returning NULL for what should have been a valid core function.
//
// To add a new core command: compute its hash with loader_hash_string() (loader/loader_common.h), e.g.
//   python3 -c "
//   h = 2166136261
//   for c in b'vkNewFunc':
//       h ^= c; h = (h * 16777619) & 0xFFFFFFFF
//   print(hex(h))"
// and also add the name to kGpaHelperCoreInstanceNames in tests/loader_get_proc_addr_tests.cpp, which
// exhaustively calls every name here and fails loudly if this switch and that list ever drift apart.
void *trampoline_get_proc_addr(struct loader_instance *inst, const char *funcName) {
    const uint32_t name_hash = loader_hash_string(funcName);
    // Don't include or check global functions
    switch (name_hash) {
        case 0x9d4599a6u:
            if (!strcmp(funcName, "vkGetInstanceProcAddr")) return vkGetInstanceProcAddr;
            break;
        case 0xa64dcfc3u:
            if (!strcmp(funcName, "vkDestroyInstance")) return vkDestroyInstance;
            break;
        case 0x09612094u:
            if (!strcmp(funcName, "vkEnumeratePhysicalDevices")) return vkEnumeratePhysicalDevices;
            break;
        case 0x3407e5aau:
            if (!strcmp(funcName, "vkGetPhysicalDeviceFeatures")) return vkGetPhysicalDeviceFeatures;
            break;
        case 0xf571955fu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceFormatProperties")) return vkGetPhysicalDeviceFormatProperties;
            break;
        case 0x8bfced20u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceImageFormatProperties")) return vkGetPhysicalDeviceImageFormatProperties;
            break;
        case 0x763c38a0u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceSparseImageFormatProperties"))
                return vkGetPhysicalDeviceSparseImageFormatProperties;
            break;
        case 0x89b3ebbcu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceProperties")) return vkGetPhysicalDeviceProperties;
            break;
        case 0x3d926eedu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceQueueFamilyProperties")) return vkGetPhysicalDeviceQueueFamilyProperties;
            break;
        case 0x278fa9bbu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceMemoryProperties")) return vkGetPhysicalDeviceMemoryProperties;
            break;
        case 0xa37d674au:
            if (!strcmp(funcName, "vkEnumerateDeviceLayerProperties")) return vkEnumerateDeviceLayerProperties;
            break;
        case 0x53cd225eu:
            if (!strcmp(funcName, "vkEnumerateDeviceExtensionProperties")) return vkEnumerateDeviceExtensionProperties;
            break;
        case 0x87b9b4e4u:
            if (!strcmp(funcName, "vkCreateDevice")) return vkCreateDevice;
            break;
        case 0x228d66e7u:
            if (!strcmp(funcName, "vkGetDeviceProcAddr")) return vkGetDeviceProcAddr;
            break;
        case 0x76ce3712u:
            if (!strcmp(funcName, "vkDestroyDevice")) return vkDestroyDevice;
            break;
        case 0x0ef70c2bu:
            if (!strcmp(funcName, "vkGetDeviceQueue")) return vkGetDeviceQueue;
            break;
        case 0xa702ecf9u:
            if (!strcmp(funcName, "vkQueueSubmit")) return vkQueueSubmit;
            break;
        case 0x33b1f316u:
            if (!strcmp(funcName, "vkQueueWaitIdle")) return vkQueueWaitIdle;
            break;
        case 0xd8f8605fu:
            if (!strcmp(funcName, "vkDeviceWaitIdle")) return vkDeviceWaitIdle;
            break;
        case 0x8fd182beu:
            if (!strcmp(funcName, "vkAllocateMemory")) return vkAllocateMemory;
            break;
        case 0x3486e387u:
            if (!strcmp(funcName, "vkFreeMemory")) return vkFreeMemory;
            break;
        case 0xeade61a3u:
            if (!strcmp(funcName, "vkMapMemory")) return vkMapMemory;
            break;
        case 0x45ae8caau:
            if (!strcmp(funcName, "vkUnmapMemory")) return vkUnmapMemory;
            break;
        case 0xd4cde7deu:
            if (!strcmp(funcName, "vkFlushMappedMemoryRanges")) return vkFlushMappedMemoryRanges;
            break;
        case 0x242f4435u:
            if (!strcmp(funcName, "vkInvalidateMappedMemoryRanges")) return vkInvalidateMappedMemoryRanges;
            break;
        case 0x7edaff3eu:
            if (!strcmp(funcName, "vkGetDeviceMemoryCommitment")) return vkGetDeviceMemoryCommitment;
            break;
        case 0x0f6505a4u:
            if (!strcmp(funcName, "vkGetImageSparseMemoryRequirements")) return vkGetImageSparseMemoryRequirements;
            break;
        case 0x5b923818u:
            if (!strcmp(funcName, "vkGetImageMemoryRequirements")) return vkGetImageMemoryRequirements;
            break;
        case 0x5f571cdbu:
            if (!strcmp(funcName, "vkGetBufferMemoryRequirements")) return vkGetBufferMemoryRequirements;
            break;
        case 0xb59c1405u:
            if (!strcmp(funcName, "vkBindImageMemory")) return vkBindImageMemory;
            break;
        case 0x8fcb92d4u:
            if (!strcmp(funcName, "vkBindBufferMemory")) return vkBindBufferMemory;
            break;
        case 0xe4d99d8eu:
            if (!strcmp(funcName, "vkQueueBindSparse")) return vkQueueBindSparse;
            break;
        case 0xb758c6a3u:
            if (!strcmp(funcName, "vkCreateFence")) return vkCreateFence;
            break;
        case 0x60073b51u:
            if (!strcmp(funcName, "vkDestroyFence")) return vkDestroyFence;
            break;
        case 0xd870b975u:
            if (!strcmp(funcName, "vkGetFenceStatus")) return vkGetFenceStatus;
            break;
        case 0xdbb395d3u:
            if (!strcmp(funcName, "vkResetFences")) return vkResetFences;
            break;
        case 0xe7c2c7dcu:
            if (!strcmp(funcName, "vkWaitForFences")) return vkWaitForFences;
            break;
        case 0xd9b8366cu:
            if (!strcmp(funcName, "vkCreateSemaphore")) return vkCreateSemaphore;
            break;
        case 0xaf0834d2u:
            if (!strcmp(funcName, "vkDestroySemaphore")) return vkDestroySemaphore;
            break;
        case 0x42b3f55eu:
            if (!strcmp(funcName, "vkCreateEvent")) return vkCreateEvent;
            break;
        case 0x624815e0u:
            if (!strcmp(funcName, "vkDestroyEvent")) return vkDestroyEvent;
            break;
        case 0x66007620u:
            if (!strcmp(funcName, "vkGetEventStatus")) return vkGetEventStatus;
            break;
        case 0xa73c166au:
            if (!strcmp(funcName, "vkSetEvent")) return vkSetEvent;
            break;
        case 0x7de046a3u:
            if (!strcmp(funcName, "vkResetEvent")) return vkResetEvent;
            break;
        case 0x0a03b3a4u:
            if (!strcmp(funcName, "vkCreateQueryPool")) return vkCreateQueryPool;
            break;
        case 0xfc8c9742u:
            if (!strcmp(funcName, "vkDestroyQueryPool")) return vkDestroyQueryPool;
            break;
        case 0x300fc14au:
            if (!strcmp(funcName, "vkGetQueryPoolResults")) return vkGetQueryPoolResults;
            break;
        case 0x507f87ecu:
            if (!strcmp(funcName, "vkCreateBuffer")) return vkCreateBuffer;
            break;
        case 0x8b9013b2u:
            if (!strcmp(funcName, "vkDestroyBuffer")) return vkDestroyBuffer;
            break;
        case 0x11b7e8d5u:
            if (!strcmp(funcName, "vkCreateBufferView")) return vkCreateBufferView;
            break;
        case 0x827878bbu:
            if (!strcmp(funcName, "vkDestroyBufferView")) return vkDestroyBufferView;
            break;
        case 0x29aa0da7u:
            if (!strcmp(funcName, "vkCreateImage")) return vkCreateImage;
            break;
        case 0x2d239345u:
            if (!strcmp(funcName, "vkDestroyImage")) return vkDestroyImage;
            break;
        case 0x3bb2e87du:
            if (!strcmp(funcName, "vkGetImageSubresourceLayout")) return vkGetImageSubresourceLayout;
            break;
        case 0xb838b382u:
            if (!strcmp(funcName, "vkCreateImageView")) return vkCreateImageView;
            break;
        case 0xb32c7798u:
            if (!strcmp(funcName, "vkDestroyImageView")) return vkDestroyImageView;
            break;
        case 0x6f596ac7u:
            if (!strcmp(funcName, "vkCreateShaderModule")) return vkCreateShaderModule;
            break;
        case 0x6892d8b1u:
            if (!strcmp(funcName, "vkDestroyShaderModule")) return vkDestroyShaderModule;
            break;
        case 0x3e71c4c6u:
            if (!strcmp(funcName, "vkCreatePipelineCache")) return vkCreatePipelineCache;
            break;
        case 0x396c2a34u:
            if (!strcmp(funcName, "vkDestroyPipelineCache")) return vkDestroyPipelineCache;
            break;
        case 0x6855c1a6u:
            if (!strcmp(funcName, "vkGetPipelineCacheData")) return vkGetPipelineCacheData;
            break;
        case 0xc71137ebu:
            if (!strcmp(funcName, "vkMergePipelineCaches")) return vkMergePipelineCaches;
            break;
        case 0xffdc1506u:
            if (!strcmp(funcName, "vkCreateGraphicsPipelines")) return vkCreateGraphicsPipelines;
            break;
        case 0xc45193c6u:
            if (!strcmp(funcName, "vkCreateComputePipelines")) return vkCreateComputePipelines;
            break;
        case 0x84481076u:
            if (!strcmp(funcName, "vkDestroyPipeline")) return vkDestroyPipeline;
            break;
        case 0xaffa89a2u:
            if (!strcmp(funcName, "vkCreatePipelineLayout")) return vkCreatePipelineLayout;
            break;
        case 0xe5b1e1d0u:
            if (!strcmp(funcName, "vkDestroyPipelineLayout")) return vkDestroyPipelineLayout;
            break;
        case 0x5d12d31eu:
            if (!strcmp(funcName, "vkCreateSampler")) return vkCreateSampler;
            break;
        case 0x2842f278u:
            if (!strcmp(funcName, "vkDestroySampler")) return vkDestroySampler;
            break;
        case 0x452f43a7u:
            if (!strcmp(funcName, "vkCreateDescriptorSetLayout")) return vkCreateDescriptorSetLayout;
            break;
        case 0x123433f5u:
            if (!strcmp(funcName, "vkDestroyDescriptorSetLayout")) return vkDestroyDescriptorSetLayout;
            break;
        case 0x6392b8cdu:
            if (!strcmp(funcName, "vkCreateDescriptorPool")) return vkCreateDescriptorPool;
            break;
        case 0x752f3e6fu:
            if (!strcmp(funcName, "vkDestroyDescriptorPool")) return vkDestroyDescriptorPool;
            break;
        case 0x19c5052au:
            if (!strcmp(funcName, "vkResetDescriptorPool")) return vkResetDescriptorPool;
            break;
        case 0x14a3b709u:
            if (!strcmp(funcName, "vkAllocateDescriptorSets")) return vkAllocateDescriptorSets;
            break;
        case 0x2d791f80u:
            if (!strcmp(funcName, "vkFreeDescriptorSets")) return vkFreeDescriptorSets;
            break;
        case 0x13d7722fu:
            if (!strcmp(funcName, "vkUpdateDescriptorSets")) return vkUpdateDescriptorSets;
            break;
        case 0x8c0b3c69u:
            if (!strcmp(funcName, "vkCreateFramebuffer")) return vkCreateFramebuffer;
            break;
        case 0xf60843dbu:
            if (!strcmp(funcName, "vkDestroyFramebuffer")) return vkDestroyFramebuffer;
            break;
        case 0xe75b2217u:
            if (!strcmp(funcName, "vkCreateRenderPass")) return vkCreateRenderPass;
            break;
        case 0x235b7d71u:
            if (!strcmp(funcName, "vkDestroyRenderPass")) return vkDestroyRenderPass;
            break;
        case 0x0984abf1u:
            if (!strcmp(funcName, "vkGetRenderAreaGranularity")) return vkGetRenderAreaGranularity;
            break;
        case 0x1d0cde71u:
            if (!strcmp(funcName, "vkCreateCommandPool")) return vkCreateCommandPool;
            break;
        case 0x46d07137u:
            if (!strcmp(funcName, "vkDestroyCommandPool")) return vkDestroyCommandPool;
            break;
        case 0x32d3032cu:
            if (!strcmp(funcName, "vkResetCommandPool")) return vkResetCommandPool;
            break;
        case 0x9b476eedu:
            if (!strcmp(funcName, "vkAllocateCommandBuffers")) return vkAllocateCommandBuffers;
            break;
        case 0x60cb1fd0u:
            if (!strcmp(funcName, "vkFreeCommandBuffers")) return vkFreeCommandBuffers;
            break;
        case 0x38f0e672u:
            if (!strcmp(funcName, "vkBeginCommandBuffer")) return vkBeginCommandBuffer;
            break;
        case 0xcf8708eau:
            if (!strcmp(funcName, "vkEndCommandBuffer")) return vkEndCommandBuffer;
            break;
        case 0x6ee4b1d0u:
            if (!strcmp(funcName, "vkResetCommandBuffer")) return vkResetCommandBuffer;
            break;
        case 0x3306d993u:
            if (!strcmp(funcName, "vkCmdBindPipeline")) return vkCmdBindPipeline;
            break;
        case 0x4f8ac889u:
            if (!strcmp(funcName, "vkCmdBindDescriptorSets")) return vkCmdBindDescriptorSets;
            break;
        case 0xadd4e8dau:
            if (!strcmp(funcName, "vkCmdBindVertexBuffers")) return vkCmdBindVertexBuffers;
            break;
        case 0xf1ac7a71u:
            if (!strcmp(funcName, "vkCmdBindIndexBuffer")) return vkCmdBindIndexBuffer;
            break;
        case 0x64e0e218u:
            if (!strcmp(funcName, "vkCmdSetViewport")) return vkCmdSetViewport;
            break;
        case 0xbcc3cad4u:
            if (!strcmp(funcName, "vkCmdSetScissor")) return vkCmdSetScissor;
            break;
        case 0x89505460u:
            if (!strcmp(funcName, "vkCmdSetLineWidth")) return vkCmdSetLineWidth;
            break;
        case 0x9da9f23eu:
            if (!strcmp(funcName, "vkCmdSetDepthBias")) return vkCmdSetDepthBias;
            break;
        case 0xb11418dau:
            if (!strcmp(funcName, "vkCmdSetBlendConstants")) return vkCmdSetBlendConstants;
            break;
        case 0x8aad23c2u:
            if (!strcmp(funcName, "vkCmdSetDepthBounds")) return vkCmdSetDepthBounds;
            break;
        case 0xb6ee9565u:
            if (!strcmp(funcName, "vkCmdSetStencilCompareMask")) return vkCmdSetStencilCompareMask;
            break;
        case 0xe559d0e1u:
            if (!strcmp(funcName, "vkCmdSetStencilWriteMask")) return vkCmdSetStencilWriteMask;
            break;
        case 0x1d031387u:
            if (!strcmp(funcName, "vkCmdSetStencilReference")) return vkCmdSetStencilReference;
            break;
        case 0xa9fdc2c0u:
            if (!strcmp(funcName, "vkCmdDraw")) return vkCmdDraw;
            break;
        case 0x44b84ecdu:
            if (!strcmp(funcName, "vkCmdDrawIndexed")) return vkCmdDrawIndexed;
            break;
        case 0x570978d6u:
            if (!strcmp(funcName, "vkCmdDrawIndirect")) return vkCmdDrawIndirect;
            break;
        case 0xa2dd69bfu:
            if (!strcmp(funcName, "vkCmdDrawIndexedIndirect")) return vkCmdDrawIndexedIndirect;
            break;
        case 0x1f28c0e0u:
            if (!strcmp(funcName, "vkCmdDispatch")) return vkCmdDispatch;
            break;
        case 0x6d599ab6u:
            if (!strcmp(funcName, "vkCmdDispatchIndirect")) return vkCmdDispatchIndirect;
            break;
        case 0x54db4621u:
            if (!strcmp(funcName, "vkCmdCopyBuffer")) return vkCmdCopyBuffer;
            break;
        case 0x7bf994f0u:
            if (!strcmp(funcName, "vkCmdCopyImage")) return vkCmdCopyImage;
            break;
        case 0xef654202u:
            if (!strcmp(funcName, "vkCmdBlitImage")) return vkCmdBlitImage;
            break;
        case 0xb0b1777du:
            if (!strcmp(funcName, "vkCmdCopyBufferToImage")) return vkCmdCopyBufferToImage;
            break;
        case 0x0220566fu:
            if (!strcmp(funcName, "vkCmdCopyImageToBuffer")) return vkCmdCopyImageToBuffer;
            break;
        case 0x4cafab95u:
            if (!strcmp(funcName, "vkCmdUpdateBuffer")) return vkCmdUpdateBuffer;
            break;
        case 0xe2498171u:
            if (!strcmp(funcName, "vkCmdFillBuffer")) return vkCmdFillBuffer;
            break;
        case 0x146f399bu:
            if (!strcmp(funcName, "vkCmdClearColorImage")) return vkCmdClearColorImage;
            break;
        case 0x13bc9e59u:
            if (!strcmp(funcName, "vkCmdClearDepthStencilImage")) return vkCmdClearDepthStencilImage;
            break;
        case 0x049d5213u:
            if (!strcmp(funcName, "vkCmdClearAttachments")) return vkCmdClearAttachments;
            break;
        case 0xd84ff14bu:
            if (!strcmp(funcName, "vkCmdResolveImage")) return vkCmdResolveImage;
            break;
        case 0x797f43fau:
            if (!strcmp(funcName, "vkCmdSetEvent")) return vkCmdSetEvent;
            break;
        case 0x7231a513u:
            if (!strcmp(funcName, "vkCmdResetEvent")) return vkCmdResetEvent;
            break;
        case 0x18acad2eu:
            if (!strcmp(funcName, "vkCmdWaitEvents")) return vkCmdWaitEvents;
            break;
        case 0x0223af53u:
            if (!strcmp(funcName, "vkCmdPipelineBarrier")) return vkCmdPipelineBarrier;
            break;
        case 0x976db859u:
            if (!strcmp(funcName, "vkCmdBeginQuery")) return vkCmdBeginQuery;
            break;
        case 0xb93efb21u:
            if (!strcmp(funcName, "vkCmdEndQuery")) return vkCmdEndQuery;
            break;
        case 0x21657da5u:
            if (!strcmp(funcName, "vkCmdResetQueryPool")) return vkCmdResetQueryPool;
            break;
        case 0xf4fa04efu:
            if (!strcmp(funcName, "vkCmdWriteTimestamp")) return vkCmdWriteTimestamp;
            break;
        case 0xb2cd2b3fu:
            if (!strcmp(funcName, "vkCmdCopyQueryPoolResults")) return vkCmdCopyQueryPoolResults;
            break;
        case 0xd617972fu:
            if (!strcmp(funcName, "vkCmdPushConstants")) return vkCmdPushConstants;
            break;
        case 0x79052222u:
            if (!strcmp(funcName, "vkCmdBeginRenderPass")) return vkCmdBeginRenderPass;
            break;
        case 0x94036722u:
            if (!strcmp(funcName, "vkCmdNextSubpass")) return vkCmdNextSubpass;
            break;
        case 0xeb0917fau:
            if (!strcmp(funcName, "vkCmdEndRenderPass")) return vkCmdEndRenderPass;
            break;
        case 0xf5f7e073u:
            if (!strcmp(funcName, "vkCmdExecuteCommands")) return vkCmdExecuteCommands;
            break;

        // Core 1.1 functions
        case 0x297b509fu:
            if (!strcmp(funcName, "vkEnumeratePhysicalDeviceGroups")) return vkEnumeratePhysicalDeviceGroups;
            break;
        case 0x806e6e48u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceFeatures2")) return vkGetPhysicalDeviceFeatures2;
            break;
        case 0x543bd08au:
            if (!strcmp(funcName, "vkGetPhysicalDeviceProperties2")) return vkGetPhysicalDeviceProperties2;
            break;
        case 0xcece3a97u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceFormatProperties2")) return vkGetPhysicalDeviceFormatProperties2;
            break;
        case 0x71293356u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceImageFormatProperties2")) return vkGetPhysicalDeviceImageFormatProperties2;
            break;
        case 0xcc84890du:
            if (!strcmp(funcName, "vkGetPhysicalDeviceQueueFamilyProperties2")) return vkGetPhysicalDeviceQueueFamilyProperties2;
            break;
        case 0xd027e2abu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceMemoryProperties2")) return vkGetPhysicalDeviceMemoryProperties2;
            break;
        case 0xb2cd0dd6u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceSparseImageFormatProperties2"))
                return vkGetPhysicalDeviceSparseImageFormatProperties2;
            break;
        case 0x0b1f96f9u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceExternalBufferProperties"))
                return vkGetPhysicalDeviceExternalBufferProperties;
            break;
        case 0xc98f5959u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceExternalSemaphoreProperties"))
                return vkGetPhysicalDeviceExternalSemaphoreProperties;
            break;
        case 0x9e415ee2u:
            if (!strcmp(funcName, "vkGetPhysicalDeviceExternalFenceProperties")) return vkGetPhysicalDeviceExternalFenceProperties;
            break;
        case 0x43784012u:
            if (!strcmp(funcName, "vkBindBufferMemory2")) return vkBindBufferMemory2;
            break;
        case 0x1bb3d295u:
            if (!strcmp(funcName, "vkBindImageMemory2")) return vkBindImageMemory2;
            break;
        case 0xf5a1feb7u:
            if (!strcmp(funcName, "vkGetDeviceGroupPeerMemoryFeatures")) return vkGetDeviceGroupPeerMemoryFeatures;
            break;
        case 0xf0313ec8u:
            if (!strcmp(funcName, "vkCmdSetDeviceMask")) return vkCmdSetDeviceMask;
            break;
        case 0x95d77705u:
            if (!strcmp(funcName, "vkCmdDispatchBase")) return vkCmdDispatchBase;
            break;
        case 0x512e6a1eu:
            if (!strcmp(funcName, "vkGetImageMemoryRequirements2")) return vkGetImageMemoryRequirements2;
            break;
        case 0x59c4eabdu:
            if (!strcmp(funcName, "vkTrimCommandPool")) return vkTrimCommandPool;
            break;
        case 0xa7e80b5bu:
            if (!strcmp(funcName, "vkGetDeviceQueue2")) return vkGetDeviceQueue2;
            break;
        case 0xf850dd41u:
            if (!strcmp(funcName, "vkCreateSamplerYcbcrConversion")) return vkCreateSamplerYcbcrConversion;
            break;
        case 0x8b04a087u:
            if (!strcmp(funcName, "vkDestroySamplerYcbcrConversion")) return vkDestroySamplerYcbcrConversion;
            break;
        case 0xfd510f82u:
            if (!strcmp(funcName, "vkGetDescriptorSetLayoutSupport")) return vkGetDescriptorSetLayoutSupport;
            break;
        case 0xdde2a868u:
            if (!strcmp(funcName, "vkCreateDescriptorUpdateTemplate")) return vkCreateDescriptorUpdateTemplate;
            break;
        case 0x8769c7c2u:
            if (!strcmp(funcName, "vkDestroyDescriptorUpdateTemplate")) return vkDestroyDescriptorUpdateTemplate;
            break;
        case 0xe9650ec4u:
            if (!strcmp(funcName, "vkUpdateDescriptorSetWithTemplate")) return vkUpdateDescriptorSetWithTemplate;
            break;
        case 0xd207cb22u:
            if (!strcmp(funcName, "vkGetImageSparseMemoryRequirements2")) return vkGetImageSparseMemoryRequirements2;
            break;
        case 0xff2282cbu:
            if (!strcmp(funcName, "vkGetBufferMemoryRequirements2")) return vkGetBufferMemoryRequirements2;
            break;

        // Core 1.2 functions
        case 0x5976c03fu:
            if (!strcmp(funcName, "vkCreateRenderPass2")) return vkCreateRenderPass2;
            break;
        case 0x93149f30u:
            if (!strcmp(funcName, "vkCmdBeginRenderPass2")) return vkCmdBeginRenderPass2;
            break;
        case 0x115b3e30u:
            if (!strcmp(funcName, "vkCmdNextSubpass2")) return vkCmdNextSubpass2;
            break;
        case 0xc7506fd8u:
            if (!strcmp(funcName, "vkCmdEndRenderPass2")) return vkCmdEndRenderPass2;
            break;
        case 0x772b9685u:
            if (!strcmp(funcName, "vkCmdDrawIndirectCount")) return vkCmdDrawIndirectCount;
            break;
        case 0x570f1d66u:
            if (!strcmp(funcName, "vkCmdDrawIndexedIndirectCount")) return vkCmdDrawIndexedIndirectCount;
            break;
        case 0x036431c1u:
            if (!strcmp(funcName, "vkGetSemaphoreCounterValue")) return vkGetSemaphoreCounterValue;
            break;
        case 0xc2aca738u:
            if (!strcmp(funcName, "vkWaitSemaphores")) return vkWaitSemaphores;
            break;
        case 0x307d1a20u:
            if (!strcmp(funcName, "vkSignalSemaphore")) return vkSignalSemaphore;
            break;
        case 0x5a76a96eu:
            if (!strcmp(funcName, "vkGetBufferDeviceAddress")) return vkGetBufferDeviceAddress;
            break;
        case 0x7e52a54du:
            if (!strcmp(funcName, "vkGetBufferOpaqueCaptureAddress")) return vkGetBufferOpaqueCaptureAddress;
            break;
        case 0xcf71e086u:
            if (!strcmp(funcName, "vkGetDeviceMemoryOpaqueCaptureAddress")) return vkGetDeviceMemoryOpaqueCaptureAddress;
            break;
        case 0x4f91a755u:
            if (!strcmp(funcName, "vkResetQueryPool")) return vkResetQueryPool;
            break;

        // Core 1.3 functions
        case 0x2191b19eu:
            if (!strcmp(funcName, "vkGetPhysicalDeviceToolProperties")) return vkGetPhysicalDeviceToolProperties;
            break;
        case 0x26afd535u:
            if (!strcmp(funcName, "vkCreatePrivateDataSlot")) return vkCreatePrivateDataSlot;
            break;
        case 0xd5976337u:
            if (!strcmp(funcName, "vkDestroyPrivateDataSlot")) return vkDestroyPrivateDataSlot;
            break;
        case 0x8ea4745du:
            if (!strcmp(funcName, "vkSetPrivateData")) return vkSetPrivateData;
            break;
        case 0xe9a91971u:
            if (!strcmp(funcName, "vkGetPrivateData")) return vkGetPrivateData;
            break;
        case 0x0b57b3d8u:
            if (!strcmp(funcName, "vkCmdSetEvent2")) return vkCmdSetEvent2;
            break;
        case 0xe526f2f3u:
            if (!strcmp(funcName, "vkCmdResetEvent2")) return vkCmdResetEvent2;
            break;
        case 0xf3d48314u:
            if (!strcmp(funcName, "vkCmdWaitEvents2")) return vkCmdWaitEvents2;
            break;
        case 0xbf2d15b3u:
            if (!strcmp(funcName, "vkCmdPipelineBarrier2")) return vkCmdPipelineBarrier2;
            break;
        case 0x8295a7e7u:
            if (!strcmp(funcName, "vkCmdWriteTimestamp2")) return vkCmdWriteTimestamp2;
            break;
        case 0xb49ac391u:
            if (!strcmp(funcName, "vkQueueSubmit2")) return vkQueueSubmit2;
            break;
        case 0xa82f4fe9u:
            if (!strcmp(funcName, "vkCmdCopyBuffer2")) return vkCmdCopyBuffer2;
            break;
        case 0xebe52d66u:
            if (!strcmp(funcName, "vkCmdCopyImage2")) return vkCmdCopyImage2;
            break;
        case 0x765ed15du:
            if (!strcmp(funcName, "vkCmdCopyBufferToImage2")) return vkCmdCopyBufferToImage2;
            break;
        case 0xb5e7f467u:
            if (!strcmp(funcName, "vkCmdCopyImageToBuffer2")) return vkCmdCopyImageToBuffer2;
            break;
        case 0x0c673190u:
            if (!strcmp(funcName, "vkCmdBlitImage2")) return vkCmdBlitImage2;
            break;
        case 0xfed9217bu:
            if (!strcmp(funcName, "vkCmdResolveImage2")) return vkCmdResolveImage2;
            break;
        case 0xc23aecb9u:
            if (!strcmp(funcName, "vkCmdBeginRendering")) return vkCmdBeginRendering;
            break;
        case 0x504cb441u:
            if (!strcmp(funcName, "vkCmdEndRendering")) return vkCmdEndRendering;
            break;
        case 0xbb1fdbfdu:
            if (!strcmp(funcName, "vkCmdSetCullMode")) return vkCmdSetCullMode;
            break;
        case 0xec690b60u:
            if (!strcmp(funcName, "vkCmdSetFrontFace")) return vkCmdSetFrontFace;
            break;
        case 0xb4387882u:
            if (!strcmp(funcName, "vkCmdSetPrimitiveTopology")) return vkCmdSetPrimitiveTopology;
            break;
        case 0xa0611d7fu:
            if (!strcmp(funcName, "vkCmdSetViewportWithCount")) return vkCmdSetViewportWithCount;
            break;
        case 0xfc5a2533u:
            if (!strcmp(funcName, "vkCmdSetScissorWithCount")) return vkCmdSetScissorWithCount;
            break;
        case 0x8e2aa538u:
            if (!strcmp(funcName, "vkCmdBindVertexBuffers2")) return vkCmdBindVertexBuffers2;
            break;
        case 0x4874399cu:
            if (!strcmp(funcName, "vkCmdSetDepthTestEnable")) return vkCmdSetDepthTestEnable;
            break;
        case 0x48a8eb79u:
            if (!strcmp(funcName, "vkCmdSetDepthWriteEnable")) return vkCmdSetDepthWriteEnable;
            break;
        case 0xeee7ee11u:
            if (!strcmp(funcName, "vkCmdSetDepthCompareOp")) return vkCmdSetDepthCompareOp;
            break;
        case 0x47579071u:
            if (!strcmp(funcName, "vkCmdSetDepthBoundsTestEnable")) return vkCmdSetDepthBoundsTestEnable;
            break;
        case 0xb486d021u:
            if (!strcmp(funcName, "vkCmdSetStencilTestEnable")) return vkCmdSetStencilTestEnable;
            break;
        case 0x4d91cb75u:
            if (!strcmp(funcName, "vkCmdSetStencilOp")) return vkCmdSetStencilOp;
            break;
        case 0x3fb78c34u:
            if (!strcmp(funcName, "vkCmdSetRasterizerDiscardEnable")) return vkCmdSetRasterizerDiscardEnable;
            break;
        case 0x9b2a547du:
            if (!strcmp(funcName, "vkCmdSetDepthBiasEnable")) return vkCmdSetDepthBiasEnable;
            break;
        case 0xd42a0ba9u:
            if (!strcmp(funcName, "vkCmdSetPrimitiveRestartEnable")) return vkCmdSetPrimitiveRestartEnable;
            break;
        case 0xe338387du:
            if (!strcmp(funcName, "vkGetDeviceBufferMemoryRequirements")) return vkGetDeviceBufferMemoryRequirements;
            break;
        case 0xb451de1au:
            if (!strcmp(funcName, "vkGetDeviceImageMemoryRequirements")) return vkGetDeviceImageMemoryRequirements;
            break;
        case 0xbbff0df2u:
            if (!strcmp(funcName, "vkGetDeviceImageSparseMemoryRequirements")) return vkGetDeviceImageSparseMemoryRequirements;
            break;

        // Core 1.4 functions
        case 0x84520929u:
            if (!strcmp(funcName, "vkCmdSetLineStipple")) return vkCmdSetLineStipple;
            break;
        case 0x4d139743u:
            if (!strcmp(funcName, "vkMapMemory2")) return vkMapMemory2;
            break;
        case 0x49c75348u:
            if (!strcmp(funcName, "vkUnmapMemory2")) return vkUnmapMemory2;
            break;
        case 0xb5847779u:
            if (!strcmp(funcName, "vkCmdBindIndexBuffer2")) return vkCmdBindIndexBuffer2;
            break;
        case 0x2154c31du:
            if (!strcmp(funcName, "vkGetRenderingAreaGranularity")) return vkGetRenderingAreaGranularity;
            break;
        case 0xa108f16bu:
            if (!strcmp(funcName, "vkGetDeviceImageSubresourceLayout")) return vkGetDeviceImageSubresourceLayout;
            break;
        case 0x49a3b45du:
            if (!strcmp(funcName, "vkGetImageSubresourceLayout2")) return vkGetImageSubresourceLayout2;
            break;
        case 0x05d80881u:
            if (!strcmp(funcName, "vkCmdPushDescriptorSet")) return vkCmdPushDescriptorSet;
            break;
        case 0xcb42dffbu:
            if (!strcmp(funcName, "vkCmdPushDescriptorSetWithTemplate")) return vkCmdPushDescriptorSetWithTemplate;
            break;
        case 0x4c431a0du:
            if (!strcmp(funcName, "vkCmdSetRenderingAttachmentLocations")) return vkCmdSetRenderingAttachmentLocations;
            break;
        case 0x9a656a9cu:
            if (!strcmp(funcName, "vkCmdSetRenderingInputAttachmentIndices")) return vkCmdSetRenderingInputAttachmentIndices;
            break;
        case 0xf279fe61u:
            if (!strcmp(funcName, "vkCmdBindDescriptorSets2")) return vkCmdBindDescriptorSets2;
            break;
        case 0x2422e2a7u:
            if (!strcmp(funcName, "vkCmdPushConstants2")) return vkCmdPushConstants2;
            break;
        case 0xe615b1c9u:
            if (!strcmp(funcName, "vkCmdPushDescriptorSet2")) return vkCmdPushDescriptorSet2;
            break;
        case 0xc346496bu:
            if (!strcmp(funcName, "vkCmdPushDescriptorSetWithTemplate2")) return vkCmdPushDescriptorSetWithTemplate2;
            break;
        case 0x5fb583a2u:
            if (!strcmp(funcName, "vkCopyMemoryToImage")) return vkCopyMemoryToImage;
            break;
        case 0x6e1e2502u:
            if (!strcmp(funcName, "vkCopyImageToMemory")) return vkCopyImageToMemory;
            break;
        case 0x7118af0eu:
            if (!strcmp(funcName, "vkCopyImageToImage")) return vkCopyImageToImage;
            break;
        case 0xe09b90fcu:
            if (!strcmp(funcName, "vkTransitionImageLayout")) return vkTransitionImageLayout;
            break;
        default:
            break;
    }

    // Instance extensions
    void *addr;
    if (debug_extensions_InstanceGpa(inst, funcName, &addr)) return addr;

    if (wsi_swapchain_instance_gpa(inst, funcName, &addr)) return addr;

    if (extension_instance_gpa(inst, funcName, name_hash, &addr)) return addr;

    // Unknown physical device extensions
    addr = loader_phys_dev_ext_gpa_tramp(inst, funcName);
    if (NULL != addr) return addr;

    // Unknown device extensions
    addr = loader_dev_ext_gpa_tramp(inst, funcName);
    return addr;
}

void *globalGetProcAddr(const char *name) {
    if (!name || name[0] != 'v' || name[1] != 'k') return NULL;

    name += 2;
    if (!strcmp(name, "CreateInstance")) return vkCreateInstance;
    if (!strcmp(name, "EnumerateInstanceExtensionProperties")) return vkEnumerateInstanceExtensionProperties;
    if (!strcmp(name, "EnumerateInstanceLayerProperties")) return vkEnumerateInstanceLayerProperties;
    if (!strcmp(name, "EnumerateInstanceVersion")) return vkEnumerateInstanceVersion;

    return NULL;
}
