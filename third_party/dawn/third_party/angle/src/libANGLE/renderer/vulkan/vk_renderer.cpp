//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_renderer.cpp:
//    Implements the class methods for Renderer.
//

#include "libANGLE/renderer/vulkan/vk_renderer.h"

// Placing this first seems to solve an intellisense bug.
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include <EGL/eglext.h>
#include <fstream>

#include "common/debug.h"
#include "common/platform.h"
#include "common/system_utils.h"
#include "common/vulkan/libvulkan_loader.h"
#include "common/vulkan/vulkan_icd.h"
#include "gpu_info_util/SystemInfo_vulkan.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/vulkan/CompilerVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/SyncVk.h"
#include "libANGLE/renderer/vulkan/VertexArrayVk.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"
#include "libANGLE/trace.h"
#include "platform/PlatformMethods.h"

// Consts
namespace
{
#if defined(ANGLE_PLATFORM_ANDROID)
constexpr const char *kDefaultPipelineCacheGraphDumpPath = "/data/local/tmp/angle_dumps/";
#else
constexpr const char *kDefaultPipelineCacheGraphDumpPath = "";
#endif  // ANGLE_PLATFORM_ANDROID

constexpr VkFormatFeatureFlags kInvalidFormatFeatureFlags = static_cast<VkFormatFeatureFlags>(-1);

#if defined(ANGLE_EXPOSE_NON_CONFORMANT_EXTENSIONS_AND_VERSIONS)
constexpr bool kExposeNonConformantExtensionsAndVersions = true;
#else
constexpr bool kExposeNonConformantExtensionsAndVersions = false;
#endif

#if defined(ANGLE_ENABLE_CRC_FOR_PIPELINE_CACHE)
constexpr bool kEnableCRCForPipelineCache = true;
#else
constexpr bool kEnableCRCForPipelineCache = false;
#endif

#if defined(ANGLE_ENABLE_VULKAN_API_DUMP_LAYER)
constexpr bool kEnableVulkanAPIDumpLayer = true;
#else
constexpr bool kEnableVulkanAPIDumpLayer = false;
#endif
}  // anonymous namespace

namespace rx
{
namespace vk
{
namespace
{
constexpr uint32_t kMinDefaultUniformBufferSize = 16 * 1024u;
// This size is picked based on experience. Majority of devices support 64K
// maxUniformBufferSize. Since this is per context buffer, a bigger buffer size reduces the
// number of descriptor set allocations, so we picked the maxUniformBufferSize that most
// devices supports. It may needs further tuning based on specific device needs and balance
// between performance and memory usage.
constexpr uint32_t kPreferredDefaultUniformBufferSize = 64 * 1024u;

// Maximum size to use VMA image suballocation. Any allocation greater than or equal to this
// value will use a dedicated VkDeviceMemory.
constexpr size_t kImageSizeThresholdForDedicatedMemoryAllocation = 4 * 1024 * 1024;

// Pipeline cache header version. It should be incremented any time there is an update to the cache
// header or data structure.
constexpr uint32_t kPipelineCacheVersion = 3;

// Update the pipeline cache every this many swaps.
constexpr uint32_t kPipelineCacheVkUpdatePeriod = 60;

// Per the Vulkan specification, ANGLE must indicate the highest version of Vulkan functionality
// that it uses.  The Vulkan validation layers will issue messages for any core functionality that
// requires a higher version.
//
// ANGLE specifically limits its core version to Vulkan 1.1 and relies on availability of
// extensions.  While implementations are not required to expose an extension that is promoted to
// later versions, they always do so in practice.  Avoiding later core versions helps keep the
// initialization logic simpler.
constexpr uint32_t kPreferredVulkanAPIVersion = VK_API_VERSION_1_1;

// Development flag for transition period when both old and new syncval fitlers are used
constexpr bool kSyncValCheckExtraProperties = false;

bool IsVulkan11(uint32_t apiVersion)
{
    return apiVersion >= VK_API_VERSION_1_1;
}

bool IsRADV(uint32_t vendorId, uint32_t driverId, const char *deviceName)
{
    // Check against RADV driver id first.
    if (driverId == VK_DRIVER_ID_MESA_RADV)
    {
        return true;
    }

    // Otherwise, look for RADV in the device name.  This works for both RADV
    // and Venus-over-RADV.
    return IsAMD(vendorId) && strstr(deviceName, "RADV") != nullptr;
}

bool IsQualcommOpenSource(uint32_t vendorId, uint32_t driverId, const char *deviceName)
{
    if (!IsQualcomm(vendorId))
    {
        return false;
    }

    // Where driver id is available, distinguish by driver id:
    if (driverId != 0)
    {
        return driverId != VK_DRIVER_ID_QUALCOMM_PROPRIETARY;
    }

    // Otherwise, look for Venus or Turnip in the device name.
    return strstr(deviceName, "Venus") != nullptr || strstr(deviceName, "Turnip") != nullptr;
}

bool IsXclipse()
{
    if (!IsAndroid())
    {
        return false;
    }

    std::string modelName;
    if (!angle::android::GetSystemProperty(angle::android::kModelSystemPropertyName, &modelName))
    {
        return 0;
    }

    // Improve this when more Xclipse devices are available
    return strstr(modelName.c_str(), "SM-S901B") != nullptr ||
           strstr(modelName.c_str(), "SM-S926B") != nullptr;
}

bool StrLess(const char *a, const char *b)
{
    return strcmp(a, b) < 0;
}

bool ExtensionFound(const char *needle, const vk::ExtensionNameList &haystack)
{
    // NOTE: The list must be sorted.
    return std::binary_search(haystack.begin(), haystack.end(), needle, StrLess);
}

VkResult VerifyExtensionsPresent(const vk::ExtensionNameList &haystack,
                                 const vk::ExtensionNameList &needles)
{
    // NOTE: The lists must be sorted.
    if (std::includes(haystack.begin(), haystack.end(), needles.begin(), needles.end(), StrLess))
    {
        return VK_SUCCESS;
    }
    for (const char *needle : needles)
    {
        if (!ExtensionFound(needle, haystack))
        {
            ERR() << "Extension not supported: " << needle;
        }
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// Array of Validation error/warning messages that will be ignored, should include bugID
constexpr const char *kSkippedMessages[] = {
    // http://anglebug.com/42266825
    "Undefined-Value-ShaderOutputNotConsumed",
    "Undefined-Value-ShaderInputNotProduced",
    // ANGLE sets gl_Layer when the framebuffer is not layered, but VVL does not see that.  When
    // layered, if gl_Layer is out of bounds, the results are undefined in both GL and Vulkan.
    // http://anglebug.com/372390039
    "Undefined-Layer-Written",
    // http://anglebug.com/42263850
    "VUID-vkCmdDraw-magFilter-04553",
    "VUID-vkCmdDrawIndexed-magFilter-04553",
    // http://anglebug.com/42265014
    "vkEnumeratePhysicalDevices: One or more layers modified physical devices",
    // http://anglebug.com/42265797
    "VUID-vkCmdBindVertexBuffers2-pStrides-06209",
    // http://anglebug.com/42266199
    "VUID-vkDestroySemaphore-semaphore-01137",
    "VUID-vkDestroySemaphore-semaphore-05149",
    // https://issuetracker.google.com/303219657
    "VUID-VkGraphicsPipelineCreateInfo-pStages-00738",
    // http://anglebug.com/42266334
    "VUID-vkCmdDraw-None-06887",
    "VUID-vkCmdDraw-None-06886",
    "VUID-vkCmdDrawIndexed-None-06887",
    // http://anglebug.com/42266819
    "VUID-vkCmdDraw-None-09000",
    "VUID-vkCmdDrawIndexed-None-09002",
    // http://anglebug.com/40644894
    "VUID-VkDescriptorImageInfo-imageView-06711",
    "VUID-VkDescriptorImageInfo-descriptorType-06713",
    // http://crbug.com/1412096
    "VUID-VkImageCreateInfo-pNext-00990",
    // http://anglebug.com/42266565
    "VUID-VkGraphicsPipelineCreateInfo-Input-07904",
    "VUID-VkGraphicsPipelineCreateInfo-Input-07905",
    "VUID-vkCmdDrawIndexed-None-07835",
    "VUID-VkGraphicsPipelineCreateInfo-Input-08733",
    "VUID-vkCmdDraw-Input-08734",
    // https://anglebug.com/42266575#comment4
    "VUID-VkBufferViewCreateInfo-format-08779",
    // https://anglebug.com/42266639
    "VUID-VkVertexInputBindingDivisorDescriptionKHR-divisor-01870",
    "VUID-VkVertexInputBindingDivisorDescription-divisor-01870",
    // https://anglebug.com/42266675
    "VUID-VkGraphicsPipelineCreateInfo-topology-08773",
    // https://anglebug.com/42265766
    "VUID-vkCmdBlitImage-srcImage-00240",
    // https://anglebug.com/42266678
    // VVL bug: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7858
    "VUID-vkCmdDraw-None-08608",
    "VUID-vkCmdDrawIndexed-None-08608",
    "VUID-vkCmdDraw-None-07843",
    "VUID-vkCmdDrawIndexed-None-07843",
    "VUID-vkCmdDraw-None-07844",
    "VUID-vkCmdDrawIndexed-None-07844",
    "VUID-vkCmdDraw-None-07847",
    "VUID-vkCmdDrawIndexed-None-07847",
    // Invalid feedback loop caused by the application
    "VUID-vkCmdDraw-None-09000",
    "VUID-vkCmdDrawIndexed-None-09000",
    "VUID-vkCmdDraw-None-09002",
    "VUID-vkCmdDrawIndexed-None-09002",
    "VUID-vkCmdDraw-None-09003",
    "VUID-vkCmdDrawIndexed-None-09003",
    // https://anglebug.com/42266764
    "VUID-VkDescriptorImageInfo-imageView-07796",
    // https://issuetracker.google.com/303441816
    "VUID-VkRenderPassBeginInfo-renderPass-00904",
    // http://anglebug.com/42266888
    "VUID-VkMemoryAllocateInfo-allocationSize-01742",
    "VUID-VkMemoryDedicatedAllocateInfo-image-01878",
    // http://anglebug.com/42266890
    "VUID-vkCmdDraw-pNext-09461",
    // http://anglebug.com/42266893
    "VUID-VkImportMemoryFdInfoKHR-handleType-00667",
    // http://anglebug.com/42266904
    "VUID-VkImportMemoryWin32HandleInfoKHR-handleType-00658",
    // https://anglebug.com/42266920
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912",
    // https://anglebug.com/42266947
    "VUID-VkPipelineVertexInputStateCreateInfo-pNext-pNext",
    // https://issuetracker.google.com/319228278
    "VUID-vkCmdDrawIndexed-format-07753",
    "VUID-vkCmdDraw-format-07753",
    "Undefined-Value-ShaderFragmentOutputMismatch",
    // https://anglebug.com/336652255
    "VUID-vkCmdDraw-None-09600",
    // https://issuetracker.google.com/336847261
    "VUID-VkImageCreateInfo-pNext-02397",
    "VUID-vkCmdDraw-None-06550",
    // https://anglebug.com/345304850
    "WARNING-Shader-OutputNotConsumed",
    // https://anglebug.com/383311444
    "VUID-vkCmdDraw-None-09462",
    // https://anglebug.com/394598758
    "VUID-vkBindBufferMemory-size-01037",
    "VUID-vkBindBufferMemory-buffer-01444",
};

// Validation messages that should be ignored only when VK_EXT_primitive_topology_list_restart is
// not present.
constexpr const char *kNoListRestartSkippedMessages[] = {
    // http://anglebug.com/42262476
    "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252",
};

// Validation messages that should be ignored only when exposeNonConformantExtensionsAndVersions is
// enabled on certain test platforms.
constexpr const char *kExposeNonConformantSkippedMessages[] = {
    // http://issuetracker.google.com/376899587
    "VUID-VkSwapchainCreateInfoKHR-presentMode-01427",
};

// VVL appears has a bug tracking stageMask on VkEvent with secondary command buffer.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7849
constexpr const char *kSkippedMessagesWithVulkanSecondaryCommandBuffer[] = {
    "VUID-vkCmdWaitEvents-srcStageMask-parameter",
};

// When using Vulkan secondary command buffers, the command buffer is begun with the current
// framebuffer specified in pInheritanceInfo::framebuffer.  If the framebuffer is multisampled
// and is resolved, an optimization would change the framebuffer to add the resolve target and
// use a subpass resolve operation instead.  The following error complains that the framebuffer
// used to start the render pass and the one specified in pInheritanceInfo::framebuffer must be
// equal, which is not true in that case.  In practice, this is benign, as the part of the
// framebuffer that's accessed by the command buffer is identically laid out.
// http://anglebug.com/42265307
constexpr const char *kSkippedMessagesWithRenderPassObjectsAndVulkanSCB[] = {
    "VUID-vkCmdExecuteCommands-pCommandBuffers-00099",
};

// VVL bugs with dynamic rendering
constexpr const char *kSkippedMessagesWithDynamicRendering[] = {
    // https://anglebug.com/42266678
    // VVL bugs with rasterizer discard:
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7858
    "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08914",
    "VUID-vkCmdDraw-dynamicRenderingUnusedAttachments-08917",
    "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08914",
    "VUID-vkCmdDrawIndexed-dynamicRenderingUnusedAttachments-08917",
    "VUID-vkCmdDraw-pDepthAttachment-08964",
    "VUID-vkCmdDraw-pStencilAttachment-08965",
    "VUID-vkCmdDrawIndexed-pDepthAttachment-08964",
    "VUID-vkCmdDrawIndexed-pStencilAttachment-08965",
    "VUID-vkCmdDraw-None-07843",
    "VUID-vkCmdDraw-None-07844",
    "VUID-vkCmdDraw-None-07847",
    "VUID-vkCmdDrawIndexed-None-07843",
    "VUID-vkCmdDrawIndexed-None-07844",
    "VUID-vkCmdDrawIndexed-None-07847",
    "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07285",
    "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07286",
    "VUID-vkCmdDraw-multisampledRenderToSingleSampled-07287",
    "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07285",
    "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07286",
    "VUID-vkCmdDrawIndexed-multisampledRenderToSingleSampled-07287",
};

// Some syncval errors are resolved in the presence of the NONE load or store render pass ops.  For
// those, ANGLE makes no further attempt to resolve them and expects vendor support for the
// extensions instead.  The list of skipped messages is split based on this support.
constexpr vk::SkippedSyncvalMessage kSkippedSyncvalMessages[] = {
    // http://anglebug.com/42264929
    // http://anglebug.com/42264934
    {
        "SYNC-HAZARD-WRITE-AFTER-WRITE",
        "Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
        "SYNC_IMAGE_LAYOUT_TRANSITION, "
        "write_barriers: 0",
    },
    // These errors are caused by a feedback loop tests that don't produce correct Vulkan to begin
    // with.
    // http://anglebug.com/42264930
    // http://anglebug.com/42265542
    //
    // Occasionally, this is due to VVL's lack of support for some extensions.  For example,
    // syncval doesn't properly account for VK_EXT_fragment_shader_interlock, which gives
    // synchronization guarantees without the need for an image barrier.
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4387
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "imageLayout: VK_IMAGE_LAYOUT_GENERAL",
        "usage: SYNC_FRAGMENT_SHADER_SHADER_",
    },
    // http://anglebug.com/42265049
    {
        "SYNC-HAZARD-WRITE-AFTER-WRITE",
        "Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
        "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, write_barriers: "
        "SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ|SYNC_EARLY_FRAGMENT_TESTS_DEPTH_"
        "STENCIL_ATTACHMENT_WRITE|SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ|SYNC_LATE_"
        "FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE|SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_"
        "ATTACHMENT_"
        "READ|SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE",
    },
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
     "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, write_barriers: "
     "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ|SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_"
     "ATTACHMENT_WRITE",
     "",
     false,
     {
         "message_type = RenderPassLayoutTransitionError",
         "access = SYNC_IMAGE_LAYOUT_TRANSITION",  // probably not needed, message_type implies this
         "prior_access = "
         "VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT(VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT)",
         "write_barriers = "
         "VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT(VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT|VK_"
         "ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT)",
     }},
    // From: TraceTest.manhattan_31 with SwiftShader and
    // VulkanPerformanceCounterTest.NewTextureDoesNotBreakRenderPass for both depth and stencil
    // aspect. http://anglebug.com/42265196.
    // Additionally hit in the asphalt_9 trace
    // https://issuetracker.google.com/316337308
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "with loadOp VK_ATTACHMENT_LOAD_OP_DONT_CARE. Access info (usage: "
     "SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_IMAGE_LAYOUT_TRANSITION",
     "",
     false,
     {
         "message_type = RenderPassLoadOpError",
         "load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE",
         "access = "
         "VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT(VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_"
         "BIT)",
         "prior_access = SYNC_IMAGE_LAYOUT_TRANSITION",
     }},
    // DifferentStencilMasksTest.DrawWithSameEffectiveMask/ES2_Vulkan_SwiftShader
    // Also other errors with similar message structure.
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "with loadOp VK_ATTACHMENT_LOAD_OP_DONT_CARE. Access info (usage: "
     "SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_IMAGE_LAYOUT_TRANSITION",
     "",
     false,
     {
         "message_type = BeginRenderingError",
         "load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE",
         "access = "
         "VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT(VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_"
         "BIT)",
         "prior_access = SYNC_IMAGE_LAYOUT_TRANSITION",
     }},
    // From various tests. The validation layer does not calculate the exact vertexCounts that's
    // being accessed. http://anglebug.com/42265220
    {"SYNC-HAZARD-READ-AFTER-WRITE",
     "Hazard READ_AFTER_WRITE for vertex",
     "usage: SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ",
     false,
     {
         "message_type = DrawVertexBufferError",
         "access = "
         "VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT(VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT)",
     }},
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "Hazard READ_AFTER_WRITE for index",
        "usage: SYNC_INDEX_INPUT_INDEX_READ",
    },
    {
        "SYNC-HAZARD-WRITE-AFTER-READ",
        "Hazard WRITE_AFTER_READ for",
        "Access info (usage: SYNC_VERTEX_SHADER_SHADER_STORAGE_WRITE, prior_usage: "
        "SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ",
    },
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "Hazard WRITE_AFTER_READ for dstBuffer VkBuffer",
     "Access info (usage: SYNC_COPY_TRANSFER_WRITE, prior_usage: "
     "SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ",
     false,
     {"message_type = BufferRegionError", "resource_parameter = dstBuffer",
      "access = VK_PIPELINE_STAGE_2_COPY_BIT(VK_ACCESS_2_TRANSFER_WRITE_BIT)",
      "prior_access = "
      "VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT(VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT)"}},
    {
        "SYNC-HAZARD-WRITE-AFTER-READ",
        "Hazard WRITE_AFTER_READ for VkBuffer",
        "Access info (usage: SYNC_COMPUTE_SHADER_SHADER_STORAGE_WRITE, prior_usage: "
        "SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ",
    },
    // http://anglebug.com/42266506 (VkNonDispatchableHandle on x86 bots)
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "Hazard READ_AFTER_WRITE for VkBuffer",
        "usage: SYNC_VERTEX_SHADER_SHADER_STORAGE_READ",
    },
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "Hazard READ_AFTER_WRITE for VkNonDispatchableHandle",
        "usage: SYNC_VERTEX_SHADER_SHADER_STORAGE_READ",
    },
    // Coherent framebuffer fetch is enabled on some platforms that are known a priori to have the
    // needed behavior, even though this is not specified in the Vulkan spec.  These generate
    // syncval errors that are benign on those platforms.
    // http://anglebug.com/42265363
    // From: TraceTest.dead_by_daylight
    // From: TraceTest.genshin_impact
    {"SYNC-HAZARD-READ-AFTER-WRITE",
     "with loadOp VK_ATTACHMENT_LOAD_OP_LOAD. Access info (usage: "
     "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, prior_usage: "
     "SYNC_IMAGE_LAYOUT_TRANSITION, write_barriers: 0",
     "",
     true,
     {
         "message_type = RenderPassLoadOpError",
         "load_op = VK_ATTACHMENT_LOAD_OP_LOAD",
         "access = "
         "VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT(VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT)",
         "prior_access = SYNC_IMAGE_LAYOUT_TRANSITION",
         "write_barriers = 0",
     }},
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "image layout transition (old_layout: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
     "new_layout: "
     "VK_IMAGE_LAYOUT_GENERAL). Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
     "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, write_barriers:",
     "",
     true,
     {
         "message_type = RenderPassLayoutTransitionError",
         "old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL",
         "new_layout = VK_IMAGE_LAYOUT_GENERAL",
         "access = SYNC_IMAGE_LAYOUT_TRANSITION",  // probably not needed, message_type implies this
         "prior_access = "
         "VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT(VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT)",
     }},
    // From: TraceTest.special_forces_group_2 http://anglebug.com/42264123
    {
        "SYNC-HAZARD-WRITE-AFTER-READ",
        "Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
        "SYNC_FRAGMENT_SHADER_SHADER_",
    },
    // http://anglebug.com/42265504
    {"SYNC-HAZARD-READ-AFTER-WRITE",
     "type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageLayout: "
     "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, binding #0, index 0. Access info (usage: "
     "SYNC_COMPUTE_SHADER_SHADER_",
     "", false},
    // http://anglebug.com/42265925
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
        "imageLayout: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL",
        "Access info (usage: SYNC_FRAGMENT_SHADER_SHADER_",
    },
    // From: TraceTest.life_is_strange http://anglebug.com/42266180
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "with storeOp VK_ATTACHMENT_STORE_OP_DONT_CARE. "
     "Access info (usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, "
     "prior_usage: SYNC_FRAGMENT_SHADER_SHADER_"},
    // From: TraceTest.life_is_strange http://anglebug.com/42266180
    {"SYNC-HAZARD-READ-AFTER-WRITE",
     "type: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "
     "imageLayout: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",
     "usage: SYNC_FRAGMENT_SHADER_SHADER_"},
    // From: TraceTest.diablo_immortal http://anglebug.com/42266309
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "Access info (usage: "
     "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_IMAGE_LAYOUT_TRANSITION, write_barriers: 0"},
    // From: TraceTest.diablo_immortal http://anglebug.com/42266309
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "with loadOp VK_ATTACHMENT_LOAD_OP_DONT_CARE. Access info (usage: "
     "SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_"},
    // From: TraceTest.catalyst_black http://anglebug.com/42266390
    {"SYNC-HAZARD-WRITE-AFTER-READ",
     "with storeOp VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
     "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, prior_usage: "
     "SYNC_FRAGMENT_SHADER_SHADER_"},
    // http://anglebug.com/352094384
    {
        "SYNC-HAZARD-WRITE-AFTER-WRITE",
        "Hazard WRITE_AFTER_WRITE for VkImageView",
        "Access info (usage: SYNC_ACCESS_INDEX_NONE, prior_usage: SYNC_IMAGE_LAYOUT_TRANSITION, ",
    },
    // http://anglebug.com/394598470
    {
        "SYNC-HAZARD-WRITE-AFTER-READ",
        "access = VK_PIPELINE_STAGE_2_COPY_BIT(VK_ACCESS_2_TRANSFER_WRITE_BIT)",
        "prior_access = "
        "VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT(VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT)",
    },
};

// Messages that shouldn't be generated if storeOp=NONE is supported, otherwise they are expected.
constexpr vk::SkippedSyncvalMessage kSkippedSyncvalMessagesWithoutStoreOpNone[] = {
    // These errors are generated when simultaneously using a read-only depth/stencil attachment as
    // sampler.  This is valid Vulkan.
    //
    // When storeOp=NONE is not present, ANGLE uses storeOp=STORE, but considers the image read-only
    // and produces a hazard.  ANGLE relies on storeOp=NONE and so this is not expected to be worked
    // around.
    //
    // With storeOp=NONE, there is another bug where a depth/stencil attachment may use storeOp=NONE
    // for depth while storeOp=DONT_CARE for stencil, and the latter causes a synchronization error
    // (similarly to the previous case as DONT_CARE is also a write operation).
    // http://anglebug.com/42264496
    {
        "SYNC-HAZARD-WRITE-AFTER-READ",
        "VK_ATTACHMENT_STORE_OP_STORE. Access info (usage: "
        "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE",
        "usage: SYNC_FRAGMENT_SHADER_SHADER_",
    },
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "imageLayout: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",
        "usage: SYNC_FRAGMENT_SHADER_SHADER_",
    },
    // From: TraceTest.antutu_refinery http://anglebug.com/42265159
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "imageLayout: VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL",
        "usage: SYNC_COMPUTE_SHADER_SHADER_SAMPLED_READ",
    },
};

// Messages that shouldn't be generated if both loadOp=NONE and storeOp=NONE are supported,
// otherwise they are expected.
constexpr vk::SkippedSyncvalMessage kSkippedSyncvalMessagesWithoutLoadStoreOpNone[] = {
    // This error is generated for multiple reasons:
    //
    // - http://anglebug.com/42264926
    // - http://anglebug.com/42263911: This is resolved with storeOp=NONE
    {
        "SYNC-HAZARD-WRITE-AFTER-WRITE",
        "Access info (usage: SYNC_IMAGE_LAYOUT_TRANSITION, prior_usage: "
        "SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, write_barriers: 0",
    },
    // http://anglebug.com/42264926
    // http://anglebug.com/42265079
    // http://anglebug.com/42264496
    {
        "SYNC-HAZARD-WRITE-AFTER-WRITE",
        "with loadOp VK_ATTACHMENT_LOAD_OP_DONT_CARE. Access info "
        "(usage: "
        "SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE",
    },
};

// Messages that are only generated with MSRTT emulation.  Some of these are syncval bugs (discussed
// in https://gitlab.khronos.org/vulkan/vulkan/-/issues/3840)
constexpr vk::SkippedSyncvalMessage kSkippedSyncvalMessagesWithMSRTTEmulation[] = {
    // False positive: https://gitlab.khronos.org/vulkan/vulkan/-/issues/3840
    {
        "SYNC-HAZARD-READ-AFTER-WRITE",
        "during depth/stencil resolve read",
        "SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ",
    },
    // Unknown whether ANGLE or syncval bug.
    {"SYNC-HAZARD-WRITE-AFTER-WRITE",
     "image layout transition (old_layout: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "
     "new_layout: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL). Access info (usage: "
     "SYNC_IMAGE_LAYOUT_TRANSITION",
     "",
     false,
     // TODO: it seems if this filter is removed then the error will be
     // intersepted by a different filter. Investigate the nature of the
     // error if necessary how to improve its detection.
     {
         "message_type = RenderPassLayoutTransitionError",
         "old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL",
         "new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL",
         "access = SYNC_IMAGE_LAYOUT_TRANSITION",  // probably not needed, message_type implies this
     }},
};

enum class DebugMessageReport
{
    Ignore,
    Print,
};

bool IsMessageInSkipList(const char *message,
                         const char *const skippedList[],
                         size_t skippedListSize)
{
    for (size_t index = 0; index < skippedListSize; ++index)
    {
        if (strstr(message, skippedList[index]) != nullptr)
        {
            return true;
        }
    }

    return false;
}

// Suppress validation errors that are known.  Returns DebugMessageReport::Ignore in that case.
DebugMessageReport ShouldReportDebugMessage(Renderer *renderer,
                                            const char *messageId,
                                            const char *message)
{
    if (message == nullptr || messageId == nullptr)
    {
        return DebugMessageReport::Print;
    }

    // Check with non-syncval messages:
    const std::vector<const char *> &skippedMessages = renderer->getSkippedValidationMessages();
    if (IsMessageInSkipList(message, skippedMessages.data(), skippedMessages.size()))
    {
        return DebugMessageReport::Ignore;
    }

    // Then check with syncval messages:
    const bool isColorFramebufferFetchUsed = renderer->isColorFramebufferFetchUsed();

    for (const vk::SkippedSyncvalMessage &msg : renderer->getSkippedSyncvalMessages())
    {
        if (kSyncValCheckExtraProperties && msg.extraProperties[0])
        {
            if (strstr(messageId, msg.messageId) == nullptr)
            {
                continue;
            }
            bool mismatch = false;
            for (uint32_t i = 0; i < kMaxSyncValExtraProperties; i++)
            {
                if (msg.extraProperties[i] == nullptr)
                {
                    break;
                }
                if (strstr(message, msg.extraProperties[i]) == nullptr)
                {
                    mismatch = true;
                    break;
                }
            }
            if (mismatch)
            {
                continue;
            }
        }
        else
        {
            if (strstr(messageId, msg.messageId) == nullptr ||
                strstr(message, msg.messageContents1) == nullptr ||
                strstr(message, msg.messageContents2) == nullptr)
            {
                continue;
            }
        }

        // If the error is due to exposing coherent framebuffer fetch (without
        // VK_EXT_rasterization_order_attachment_access), but framebuffer fetch has not been used by
        // the application, report it.
        //
        // Note that currently syncval doesn't support the
        // VK_EXT_rasterization_order_attachment_access extension, so the syncval messages would
        // continue to be produced despite the extension.
        constexpr bool kSyncValSupportsRasterizationOrderExtension = false;
        const bool hasRasterizationOrderExtension =
            renderer->getFeatures().supportsRasterizationOrderAttachmentAccess.enabled &&
            kSyncValSupportsRasterizationOrderExtension;
        if (msg.isDueToNonConformantCoherentColorFramebufferFetch &&
            (!isColorFramebufferFetchUsed || hasRasterizationOrderExtension))
        {
            return DebugMessageReport::Print;
        }

        // Otherwise ignore the message
        return DebugMessageReport::Ignore;
    }

    return DebugMessageReport::Print;
}

const char *GetVkObjectTypeName(VkObjectType type)
{
    switch (type)
    {
        case VK_OBJECT_TYPE_UNKNOWN:
            return "Unknown";
        case VK_OBJECT_TYPE_INSTANCE:
            return "Instance";
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
            return "Physical Device";
        case VK_OBJECT_TYPE_DEVICE:
            return "Device";
        case VK_OBJECT_TYPE_QUEUE:
            return "Queue";
        case VK_OBJECT_TYPE_SEMAPHORE:
            return "Semaphore";
        case VK_OBJECT_TYPE_COMMAND_BUFFER:
            return "Command Buffer";
        case VK_OBJECT_TYPE_FENCE:
            return "Fence";
        case VK_OBJECT_TYPE_DEVICE_MEMORY:
            return "Device Memory";
        case VK_OBJECT_TYPE_BUFFER:
            return "Buffer";
        case VK_OBJECT_TYPE_IMAGE:
            return "Image";
        case VK_OBJECT_TYPE_EVENT:
            return "Event";
        case VK_OBJECT_TYPE_QUERY_POOL:
            return "Query Pool";
        case VK_OBJECT_TYPE_BUFFER_VIEW:
            return "Buffer View";
        case VK_OBJECT_TYPE_IMAGE_VIEW:
            return "Image View";
        case VK_OBJECT_TYPE_SHADER_MODULE:
            return "Shader Module";
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
            return "Pipeline Cache";
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
            return "Pipeline Layout";
        case VK_OBJECT_TYPE_RENDER_PASS:
            return "Render Pass";
        case VK_OBJECT_TYPE_PIPELINE:
            return "Pipeline";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            return "Descriptor Set Layout";
        case VK_OBJECT_TYPE_SAMPLER:
            return "Sampler";
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
            return "Descriptor Pool";
        case VK_OBJECT_TYPE_DESCRIPTOR_SET:
            return "Descriptor Set";
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            return "Framebuffer";
        case VK_OBJECT_TYPE_COMMAND_POOL:
            return "Command Pool";
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
            return "Sampler YCbCr Conversion";
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
            return "Descriptor Update Template";
        case VK_OBJECT_TYPE_SURFACE_KHR:
            return "Surface";
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
            return "Swapchain";
        case VK_OBJECT_TYPE_DISPLAY_KHR:
            return "Display";
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
            return "Display Mode";
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV:
            return "Indirect Commands Layout";
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
            return "Debug Utils Messenger";
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
            return "Validation Cache";
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
            return "Acceleration Structure";
        default:
            return "<Unrecognized>";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                    void *userData)
{
    Renderer *renderer = static_cast<Renderer *>(userData);

    // VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessage-parameter
    // pMessage must be a null-terminated UTF-8 string
    ASSERT(callbackData->pMessage != nullptr);

    // See if it's an issue we are aware of and don't want to be spammed about.
    // Always report the debug message if message ID is missing
    if (callbackData->pMessageIdName != nullptr &&
        ShouldReportDebugMessage(renderer, callbackData->pMessageIdName, callbackData->pMessage) ==
            DebugMessageReport::Ignore)
    {
        return VK_FALSE;
    }

    std::ostringstream log;
    if (callbackData->pMessageIdName != nullptr)
    {
        log << "[ " << callbackData->pMessageIdName << " ] ";
    }
    log << callbackData->pMessage << std::endl;

    // Aesthetic value based on length of the function name, line number, etc.
    constexpr size_t kStartIndent = 28;

    // Output the debug marker hierarchy under which this error has occurred.
    size_t indent = kStartIndent;
    if (callbackData->queueLabelCount > 0)
    {
        log << std::string(indent++, ' ') << "<Queue Label Hierarchy:>" << std::endl;
        for (uint32_t i = 0; i < callbackData->queueLabelCount; ++i)
        {
            log << std::string(indent++, ' ') << callbackData->pQueueLabels[i].pLabelName
                << std::endl;
        }
    }
    if (callbackData->cmdBufLabelCount > 0)
    {
        log << std::string(indent++, ' ') << "<Command Buffer Label Hierarchy:>" << std::endl;
        for (uint32_t i = 0; i < callbackData->cmdBufLabelCount; ++i)
        {
            log << std::string(indent++, ' ') << callbackData->pCmdBufLabels[i].pLabelName
                << std::endl;
        }
    }
    // Output the objects involved in this error message.
    if (callbackData->objectCount > 0)
    {
        for (uint32_t i = 0; i < callbackData->objectCount; ++i)
        {
            const char *objectName = callbackData->pObjects[i].pObjectName;
            const char *objectType = GetVkObjectTypeName(callbackData->pObjects[i].objectType);
            uint64_t objectHandle  = callbackData->pObjects[i].objectHandle;
            log << std::string(indent, ' ') << "Object: ";
            if (objectHandle == 0)
            {
                log << "VK_NULL_HANDLE";
            }
            else
            {
                log << "0x" << std::hex << objectHandle << std::dec;
            }
            log << " (type = " << objectType << "(" << callbackData->pObjects[i].objectType << "))";
            if (objectName)
            {
                log << " [" << objectName << "]";
            }
            log << std::endl;
        }
    }

    bool isError    = (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
    std::string msg = log.str();

    renderer->onNewValidationMessage(msg);

    if (isError)
    {
        ERR() << msg;
    }
    else
    {
        WARN() << msg;
    }

    return VK_FALSE;
}

VKAPI_ATTR void VKAPI_CALL
MemoryReportCallback(const VkDeviceMemoryReportCallbackDataEXT *callbackData, void *userData)
{
    Renderer *renderer = static_cast<Renderer *>(userData);
    renderer->processMemoryReportCallback(*callbackData);
}

gl::Version LimitVersionTo(const gl::Version &current, const gl::Version &lower)
{
    return std::min(current, lower);
}

[[maybe_unused]] bool FencePropertiesCompatibleWithAndroid(
    const VkExternalFenceProperties &externalFenceProperties)
{
    // handleType here is the external fence type -
    // we want type compatible with creating and export/dup() Android FD

    // Imported handleType that can be exported - need for vkGetFenceFdKHR()
    if ((externalFenceProperties.exportFromImportedHandleTypes &
         VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR) == 0)
    {
        return false;
    }

    // HandleTypes which can be specified at creating a fence
    if ((externalFenceProperties.compatibleHandleTypes &
         VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR) == 0)
    {
        return false;
    }

    constexpr VkExternalFenceFeatureFlags kFeatureFlags =
        (VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR |
         VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR);
    if ((externalFenceProperties.externalFenceFeatures & kFeatureFlags) != kFeatureFlags)
    {
        return false;
    }

    return true;
}

[[maybe_unused]] bool SemaphorePropertiesCompatibleWithAndroid(
    const VkExternalSemaphoreProperties &externalSemaphoreProperties)
{
    // handleType here is the external semaphore type -
    // we want type compatible with importing an Android FD

    constexpr VkExternalSemaphoreFeatureFlags kFeatureFlags =
        (VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR);
    if ((externalSemaphoreProperties.externalSemaphoreFeatures & kFeatureFlags) != kFeatureFlags)
    {
        return false;
    }

    return true;
}

// Exclude memory type indices that include the host-visible bit from VMA image suballocation.
uint32_t GetMemoryTypeBitsExcludingHostVisible(Renderer *renderer,
                                               VkMemoryPropertyFlags propertyFlags,
                                               uint32_t availableMemoryTypeBits)
{
    const vk::MemoryProperties &memoryProperties = renderer->getMemoryProperties();
    ASSERT(memoryProperties.getMemoryTypeCount() <= 32);
    uint32_t memoryTypeBitsOut = availableMemoryTypeBits;

    // For best allocation results, the memory type indices that include the host-visible flag bit
    // are removed.
    for (size_t memoryIndex : angle::BitSet<32>(availableMemoryTypeBits))
    {
        VkMemoryPropertyFlags memoryFlags =
            memoryProperties.getMemoryType(static_cast<uint32_t>(memoryIndex)).propertyFlags;
        if ((memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
        {
            memoryTypeBitsOut &= ~(angle::Bit<uint32_t>(memoryIndex));
            continue;
        }

        // If the protected bit is not required, all memory type indices with this bit should be
        // ignored.
        if ((memoryFlags & ~propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)
        {
            memoryTypeBitsOut &= ~(angle::Bit<uint32_t>(memoryIndex));
        }
    }

    return memoryTypeBitsOut;
}

// Header data type used for the pipeline cache.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

class CacheDataHeader
{
  public:
    void setData(uint32_t compressedDataCRC,
                 uint32_t cacheDataSize,
                 size_t numChunks,
                 size_t chunkIndex,
                 uint32_t chunkCRC)
    {
        mVersion           = kPipelineCacheVersion;
        mCompressedDataCRC = compressedDataCRC;
        mCacheDataSize     = cacheDataSize;
        SetBitField(mNumChunks, numChunks);
        SetBitField(mChunkIndex, chunkIndex);
        mChunkCRC = chunkCRC;
    }

    void getData(uint32_t *versionOut,
                 uint32_t *compressedDataCRCOut,
                 uint32_t *cacheDataSizeOut,
                 size_t *numChunksOut,
                 size_t *chunkIndexOut,
                 uint32_t *chunkCRCOut) const
    {
        *versionOut           = mVersion;
        *compressedDataCRCOut = mCompressedDataCRC;
        *cacheDataSizeOut     = mCacheDataSize;
        *numChunksOut         = static_cast<size_t>(mNumChunks);
        *chunkIndexOut        = static_cast<size_t>(mChunkIndex);
        *chunkCRCOut          = mChunkCRC;
    }

  private:
    // For pipeline cache, the values stored in key data has the following order:
    // {headerVersion, compressedDataCRC, originalCacheSize, numChunks, chunkIndex, chunkCRC;
    // chunkCompressedData}. The header values are used to validate the data. For example, if the
    // original and compressed sizes are 70000 bytes (68k) and 68841 bytes (67k), the compressed
    // data will be divided into two chunks: {ver,crc0,70000,2,0;34421 bytes} and
    // {ver,crc1,70000,2,1;34420 bytes}.
    // The version is used to keep track of the cache format. Please note that kPipelineCacheVersion
    // must be incremented by 1 in case of any updates to the cache header or data structure. While
    // it is possible to modify the fields in the header, it is recommended to keep the version on
    // top and the same size unless absolutely necessary.

    uint32_t mVersion;
    uint32_t mCompressedDataCRC;
    uint32_t mCacheDataSize;
    uint16_t mNumChunks;
    uint16_t mChunkIndex;
    uint32_t mChunkCRC;
};

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

// Pack header data for the pipeline cache key data.
void PackHeaderDataForPipelineCache(uint32_t compressedDataCRC,
                                    uint32_t cacheDataSize,
                                    size_t numChunks,
                                    size_t chunkIndex,
                                    uint32_t chunkCRC,
                                    CacheDataHeader *dataOut)
{
    dataOut->setData(compressedDataCRC, cacheDataSize, numChunks, chunkIndex, chunkCRC);
}

// Unpack header data from the pipeline cache key data.
void UnpackHeaderDataForPipelineCache(CacheDataHeader *data,
                                      uint32_t *versionOut,
                                      uint32_t *compressedDataCRCOut,
                                      uint32_t *cacheDataSizeOut,
                                      size_t *numChunksOut,
                                      size_t *chunkIndexOut,
                                      uint32_t *chunkCRCOut)
{
    data->getData(versionOut, compressedDataCRCOut, cacheDataSizeOut, numChunksOut, chunkIndexOut,
                  chunkCRCOut);
}

void ComputePipelineCacheVkChunkKey(const VkPhysicalDeviceProperties &physicalDeviceProperties,
                                    const size_t slotIndex,
                                    const size_t chunkIndex,
                                    angle::BlobCacheKey *hashOut)
{
    std::ostringstream hashStream("ANGLE Pipeline Cache: ", std::ios_base::ate);
    // Add the pipeline cache UUID to make sure the blob cache always gives a compatible pipeline
    // cache.  It's not particularly necessary to write it as a hex number as done here, so long as
    // there is no '\0' in the result.
    for (const uint32_t c : physicalDeviceProperties.pipelineCacheUUID)
    {
        hashStream << std::hex << c;
    }
    // Add the vendor and device id too for good measure.
    hashStream << std::hex << physicalDeviceProperties.vendorID;
    hashStream << std::hex << physicalDeviceProperties.deviceID;

    // Add slotIndex to generate unique keys for each slot.
    hashStream << std::hex << static_cast<uint32_t>(slotIndex);

    // Add chunkIndex to generate unique key for chunks.
    hashStream << std::hex << static_cast<uint32_t>(chunkIndex);

    const std::string &hashString = hashStream.str();
    angle::base::SHA1HashBytes(reinterpret_cast<const unsigned char *>(hashString.c_str()),
                               hashString.length(), hashOut->data());
}

struct PipelineCacheVkChunkInfo
{
    const uint8_t *data;
    size_t dataSize;
    uint32_t crc;
    angle::BlobCacheKey cacheHash;
};

// Enough to store 32M data using 64K chunks.
constexpr size_t kFastPipelineCacheVkChunkInfosSize = 512;
using PipelineCacheVkChunkInfos =
    angle::FastVector<PipelineCacheVkChunkInfo, kFastPipelineCacheVkChunkInfosSize>;

PipelineCacheVkChunkInfos GetPipelineCacheVkChunkInfos(Renderer *renderer,
                                                       const angle::MemoryBuffer &compressedData,
                                                       const size_t numChunks,
                                                       const size_t chunkSize,
                                                       const size_t slotIndex);

// Returns the number of stored chunks.  "lastNumStoredChunks" is the number of chunks,
// stored in the last call.  If it is positive, function will only restore missing chunks.
size_t StorePipelineCacheVkChunks(vk::GlobalOps *globalOps,
                                  Renderer *renderer,
                                  const size_t lastNumStoredChunks,
                                  const PipelineCacheVkChunkInfos &chunkInfos,
                                  const size_t cacheDataSize,
                                  angle::MemoryBuffer *scratchBuffer);

// Erasing is done by writing 1/0-sized chunks starting from the startChunk.
void ErasePipelineCacheVkChunks(vk::GlobalOps *globalOps,
                                Renderer *renderer,
                                const size_t startChunk,
                                const size_t numChunks,
                                const size_t slotIndex,
                                angle::MemoryBuffer *scratchBuffer);

void CompressAndStorePipelineCacheVk(vk::GlobalOps *globalOps,
                                     Renderer *renderer,
                                     const std::vector<uint8_t> &cacheData,
                                     const size_t maxTotalSize)
{
    // Though the pipeline cache will be compressed and divided into several chunks to store in blob
    // cache, the largest total size of blob cache is only 2M in android now, so there is no use to
    // handle big pipeline cache when android will reject it finally.
    if (cacheData.size() >= maxTotalSize)
    {
        static bool warned = false;
        if (!warned)
        {
            // TODO: handle the big pipeline cache. http://anglebug.com/42263322
            WARN() << "Skip syncing pipeline cache data when it's larger than maxTotalSize. "
                      "(this message will no longer repeat)";
            warned = true;
        }
        return;
    }

    // To make it possible to store more pipeline cache data, compress the whole pipelineCache.
    angle::MemoryBuffer compressedData;

    if (!angle::CompressBlob(cacheData.size(), cacheData.data(), &compressedData))
    {
        WARN() << "Skip syncing pipeline cache data as it failed compression.";
        return;
    }

    // If the size of compressedData is larger than (kMaxBlobCacheSize - sizeof(numChunks)),
    // the pipelineCache still can't be stored in blob cache. Divide the large compressed
    // pipelineCache into several parts to store separately. There is no function to
    // query the limit size in android.
    constexpr size_t kMaxBlobCacheSize = 64 * 1024;

    const size_t numChunks = UnsignedCeilDivide(static_cast<unsigned int>(compressedData.size()),
                                                kMaxBlobCacheSize - sizeof(CacheDataHeader));
    ASSERT(numChunks <= UINT16_MAX);
    const size_t chunkSize = UnsignedCeilDivide(static_cast<unsigned int>(compressedData.size()),
                                                static_cast<unsigned int>(numChunks));

    angle::MemoryBuffer scratchBuffer;
    if (!scratchBuffer.resize(sizeof(CacheDataHeader) + chunkSize))
    {
        WARN() << "Skip syncing pipeline cache data due to out of memory.";
        return;
    }

    size_t previousSlotIndex = 0;
    const size_t slotIndex   = renderer->getNextPipelineCacheBlobCacheSlotIndex(&previousSlotIndex);
    const size_t previousNumChunks = renderer->updatePipelineCacheChunkCount(numChunks);
    const bool isSlotChanged       = (slotIndex != previousSlotIndex);

    PipelineCacheVkChunkInfos chunkInfos =
        GetPipelineCacheVkChunkInfos(renderer, compressedData, numChunks, chunkSize, slotIndex);

    // Store all chunks without checking if they already exist (because they can't).
    size_t numStoredChunks = StorePipelineCacheVkChunks(globalOps, renderer, 0, chunkInfos,
                                                        cacheData.size(), &scratchBuffer);
    ASSERT(numStoredChunks == numChunks);

    // Erase all chunks from the previous slot or any trailing chunks from the current slot.
    ASSERT(renderer->getFeatures().useDualPipelineBlobCacheSlots.enabled == isSlotChanged);
    if (isSlotChanged || previousNumChunks > numChunks)
    {
        const size_t startChunk = isSlotChanged ? 0 : numChunks;
        ErasePipelineCacheVkChunks(globalOps, renderer, startChunk, previousNumChunks,
                                   previousSlotIndex, &scratchBuffer);
    }

    if (!renderer->getFeatures().verifyPipelineCacheInBlobCache.enabled)
    {
        // No need to verify and restore possibly evicted chunks.
        return;
    }

    // Verify and restore possibly evicted chunks.
    do
    {
        const size_t lastNumStoredChunks = numStoredChunks;
        numStoredChunks = StorePipelineCacheVkChunks(globalOps, renderer, lastNumStoredChunks,
                                                     chunkInfos, cacheData.size(), &scratchBuffer);
        // Number of stored chunks must decrease so the loop can eventually exit.
        ASSERT(numStoredChunks < lastNumStoredChunks);

        // If blob cache evicts old items first, any possibly evicted chunks in the first call,
        // should have been restored in the above call without triggering another eviction, so no
        // need to continue the loop.
    } while (!renderer->getFeatures().hasBlobCacheThatEvictsOldItemsFirst.enabled &&
             numStoredChunks > 0);
}

PipelineCacheVkChunkInfos GetPipelineCacheVkChunkInfos(Renderer *renderer,
                                                       const angle::MemoryBuffer &compressedData,
                                                       const size_t numChunks,
                                                       const size_t chunkSize,
                                                       const size_t slotIndex)
{
    const VkPhysicalDeviceProperties &physicalDeviceProperties =
        renderer->getPhysicalDeviceProperties();

    PipelineCacheVkChunkInfos chunkInfos(numChunks);
    uint32_t chunkCrc = kEnableCRCForPipelineCache ? angle::InitCRC32() : 0;

    for (size_t chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex)
    {
        const size_t compressedOffset = chunkIndex * chunkSize;
        const uint8_t *data           = compressedData.data() + compressedOffset;
        const size_t dataSize = std::min(chunkSize, compressedData.size() - compressedOffset);

        // Create unique hash key.
        angle::BlobCacheKey cacheHash;
        ComputePipelineCacheVkChunkKey(physicalDeviceProperties, slotIndex, chunkIndex, &cacheHash);

        if (kEnableCRCForPipelineCache)
        {
            // Generate running CRC. Last chunk will have CRC of the entire data.
            chunkCrc = angle::UpdateCRC32(chunkCrc, data, dataSize);
        }

        chunkInfos[chunkIndex] = PipelineCacheVkChunkInfo{data, dataSize, chunkCrc, cacheHash};
    }

    return chunkInfos;
}

size_t StorePipelineCacheVkChunks(vk::GlobalOps *globalOps,
                                  Renderer *renderer,
                                  const size_t lastNumStoredChunks,
                                  const PipelineCacheVkChunkInfos &chunkInfos,
                                  const size_t cacheDataSize,
                                  angle::MemoryBuffer *scratchBuffer)
{
    // Store chunks in revers order, so when 0 chunk is available - all chunks are available.

    angle::FastVector<bool, kFastPipelineCacheVkChunkInfosSize> isMissing;
    size_t numChunksToStore = chunkInfos.size();

    // Need to check existing chunks if this is not the first time this function is called.
    if (lastNumStoredChunks > 0)
    {
        isMissing.resize(chunkInfos.size());
        numChunksToStore = 0;

        // Defer storing chunks until all missing chunks are found to avoid unnecessary stores.
        size_t chunkIndex = chunkInfos.size();
        while (chunkIndex > 0)
        {
            --chunkIndex;
            const PipelineCacheVkChunkInfo &chunkInfo = chunkInfos[chunkIndex];

            angle::BlobCacheValue value;
            if (globalOps->getBlob(chunkInfo.cacheHash, &value) &&
                value.size() == sizeof(CacheDataHeader) + chunkInfo.dataSize)
            {
                if (renderer->getFeatures().hasBlobCacheThatEvictsOldItemsFirst.enabled)
                {
                    // No need to check next chunks, since they are newer than the current and
                    // should also be present.
                    break;
                }
                continue;
            }

            isMissing[chunkIndex] = true;
            ++numChunksToStore;

            if (numChunksToStore == lastNumStoredChunks)
            {
                // No need to restore missing chunks, since new number is already same as was stored
                // last time.
                static bool warned = false;
                if (!warned)
                {
                    WARN() << "Skip syncing pipeline cache data due to not able to store "
                           << numChunksToStore << " chunks (out of " << chunkInfos.size()
                           << ") into the blob cache. (this message will no longer repeat)";
                    warned = true;
                }
                return 0;
            }
        }

        if (numChunksToStore == 0)
        {
            return 0;
        }
    }

    // Now store/restore chunks.

    // Last chunk have CRC of the entire data.
    const uint32_t compressedDataCRC = chunkInfos.back().crc;

    ASSERT(scratchBuffer != nullptr);
    angle::MemoryBuffer &keyData = *scratchBuffer;

    size_t chunkIndex = chunkInfos.size();
    while (chunkIndex > 0)
    {
        --chunkIndex;
        if (lastNumStoredChunks > 0 && !isMissing[chunkIndex])
        {
            // Skip restoring chunk if it is not missing.
            continue;
        }
        const PipelineCacheVkChunkInfo &chunkInfo = chunkInfos[chunkIndex];

        // Add the header data, followed by the compressed data.
        ASSERT(cacheDataSize <= UINT32_MAX);
        CacheDataHeader headerData = {};
        PackHeaderDataForPipelineCache(compressedDataCRC, static_cast<uint32_t>(cacheDataSize),
                                       chunkInfos.size(), chunkIndex, chunkInfo.crc, &headerData);
        keyData.setSize(sizeof(CacheDataHeader) + chunkInfo.dataSize);
        memcpy(keyData.data(), &headerData, sizeof(CacheDataHeader));
        memcpy(keyData.data() + sizeof(CacheDataHeader), chunkInfo.data, chunkInfo.dataSize);

        globalOps->putBlob(chunkInfo.cacheHash, keyData);
    }

    return numChunksToStore;
}

void ErasePipelineCacheVkChunks(vk::GlobalOps *globalOps,
                                Renderer *renderer,
                                const size_t startChunk,
                                const size_t numChunks,
                                const size_t slotIndex,
                                angle::MemoryBuffer *scratchBuffer)
{
    const VkPhysicalDeviceProperties &physicalDeviceProperties =
        renderer->getPhysicalDeviceProperties();

    ASSERT(scratchBuffer != nullptr);
    angle::MemoryBuffer &keyData = *scratchBuffer;

    keyData.setSize(
        renderer->getFeatures().useEmptyBlobsToEraseOldPipelineCacheFromBlobCache.enabled ? 0 : 1);

    // Fill data (if any) with zeroes for security.
    memset(keyData.data(), 0, keyData.size());

    for (size_t chunkIndex = startChunk; chunkIndex < numChunks; ++chunkIndex)
    {
        egl::BlobCache::Key chunkCacheHash;
        ComputePipelineCacheVkChunkKey(physicalDeviceProperties, slotIndex, chunkIndex,
                                       &chunkCacheHash);
        globalOps->putBlob(chunkCacheHash, keyData);
    }
}

class CompressAndStorePipelineCacheTask : public angle::Closure
{
  public:
    CompressAndStorePipelineCacheTask(vk::GlobalOps *globalOps,
                                      Renderer *renderer,
                                      std::vector<uint8_t> &&cacheData,
                                      size_t kMaxTotalSize)
        : mGlobalOps(globalOps),
          mRenderer(renderer),
          mCacheData(std::move(cacheData)),
          mMaxTotalSize(kMaxTotalSize)
    {}

    void operator()() override
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "CompressAndStorePipelineCacheVk");
        CompressAndStorePipelineCacheVk(mGlobalOps, mRenderer, mCacheData, mMaxTotalSize);
    }

  private:
    vk::GlobalOps *mGlobalOps;
    Renderer *mRenderer;
    std::vector<uint8_t> mCacheData;
    size_t mMaxTotalSize;
};

angle::Result GetAndDecompressPipelineCacheVk(vk::ErrorContext *context,
                                              vk::GlobalOps *globalOps,
                                              angle::MemoryBuffer *uncompressedData,
                                              bool *success)
{
    // Make sure that the bool output is initialized to false.
    *success = false;

    Renderer *renderer = context->getRenderer();

    const VkPhysicalDeviceProperties &physicalDeviceProperties =
        renderer->getPhysicalDeviceProperties();

    const size_t firstSlotIndex = renderer->getNextPipelineCacheBlobCacheSlotIndex(nullptr);
    size_t slotIndex            = firstSlotIndex;

    angle::BlobCacheKey chunkCacheHash;
    angle::BlobCacheValue keyData;

    // Iterate over available slots until data is found (only expected single slot with data).
    while (true)
    {
        // Compute the hash key of chunkIndex 0 and find the first cache data in blob cache.
        ComputePipelineCacheVkChunkKey(physicalDeviceProperties, slotIndex, 0, &chunkCacheHash);

        if (globalOps->getBlob(chunkCacheHash, &keyData) &&
            keyData.size() >= sizeof(CacheDataHeader))
        {
            // Found slot with data.
            break;
        }
        // Nothing in the cache for current slotIndex.

        slotIndex = renderer->getNextPipelineCacheBlobCacheSlotIndex(nullptr);
        if (slotIndex == firstSlotIndex)
        {
            // Nothing in all slots.
            return angle::Result::Continue;
        }
        // Try next slot.
    }

    // Get the number of chunks and other values from the header for data validation.
    uint32_t cacheVersion;
    uint32_t compressedDataCRC;
    uint32_t uncompressedCacheDataSize;
    size_t numChunks;
    size_t chunkIndex0;
    uint32_t chunkCRC;

    CacheDataHeader headerData = {};
    memcpy(&headerData, keyData.data(), sizeof(CacheDataHeader));
    UnpackHeaderDataForPipelineCache(&headerData, &cacheVersion, &compressedDataCRC,
                                     &uncompressedCacheDataSize, &numChunks, &chunkIndex0,
                                     &chunkCRC);
    if (cacheVersion == kPipelineCacheVersion)
    {
        // The data must not contain corruption.
        if (chunkIndex0 != 0 || numChunks == 0 || uncompressedCacheDataSize == 0)
        {
            FATAL() << "Unexpected values while unpacking chunk index 0: " << "cacheVersion = "
                    << cacheVersion << ", chunkIndex = " << chunkIndex0
                    << ", numChunks = " << numChunks
                    << ", uncompressedCacheDataSize = " << uncompressedCacheDataSize;
        }
    }
    else
    {
        WARN() << "Change in cache header version detected: " << "newVersion = "
               << kPipelineCacheVersion << ", existingVersion = " << cacheVersion;

        return angle::Result::Continue;
    }

    renderer->updatePipelineCacheChunkCount(numChunks);

    size_t chunkSize      = keyData.size() - sizeof(CacheDataHeader);
    size_t compressedSize = 0;

    uint32_t computedChunkCRC = kEnableCRCForPipelineCache ? angle::InitCRC32() : 0;

    // Allocate enough memory.
    angle::MemoryBuffer compressedData;
    ANGLE_VK_CHECK(context, compressedData.resize(chunkSize * numChunks),
                   VK_ERROR_INITIALIZATION_FAILED);

    // To combine the parts of the pipelineCache data.
    for (size_t chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex)
    {
        // Avoid processing 0 chunk again.
        if (chunkIndex > 0)
        {
            // Get the unique key by chunkIndex.
            ComputePipelineCacheVkChunkKey(physicalDeviceProperties, slotIndex, chunkIndex,
                                           &chunkCacheHash);

            if (!globalOps->getBlob(chunkCacheHash, &keyData) ||
                keyData.size() < sizeof(CacheDataHeader))
            {
                // Can't find every part of the cache data.
                WARN() << "Failed to get pipeline cache chunk " << chunkIndex << " of "
                       << numChunks;
                return angle::Result::Continue;
            }

            // Validate the header values and ensure there is enough space to store.
            uint32_t checkCacheVersion;
            uint32_t checkCompressedDataCRC;
            uint32_t checkUncompressedCacheDataSize;
            size_t checkNumChunks;
            size_t checkChunkIndex;

            memcpy(&headerData, keyData.data(), sizeof(CacheDataHeader));
            UnpackHeaderDataForPipelineCache(
                &headerData, &checkCacheVersion, &checkCompressedDataCRC,
                &checkUncompressedCacheDataSize, &checkNumChunks, &checkChunkIndex, &chunkCRC);

            chunkSize = keyData.size() - sizeof(CacheDataHeader);
            bool isHeaderDataCorrupted =
                (checkCacheVersion != cacheVersion) || (checkNumChunks != numChunks) ||
                (checkUncompressedCacheDataSize != uncompressedCacheDataSize) ||
                (checkCompressedDataCRC != compressedDataCRC) || (checkChunkIndex != chunkIndex) ||
                (compressedData.size() < compressedSize + chunkSize);
            if (isHeaderDataCorrupted)
            {
                WARN() << "Pipeline cache chunk header corrupted or old chunk: "
                       << "checkCacheVersion = " << checkCacheVersion
                       << ", cacheVersion = " << cacheVersion
                       << ", checkNumChunks = " << checkNumChunks << ", numChunks = " << numChunks
                       << ", checkUncompressedCacheDataSize = " << checkUncompressedCacheDataSize
                       << ", uncompressedCacheDataSize = " << uncompressedCacheDataSize
                       << ", checkCompressedDataCRC = " << checkCompressedDataCRC
                       << ", compressedDataCRC = " << compressedDataCRC
                       << ", checkChunkIndex = " << checkChunkIndex
                       << ", chunkIndex = " << chunkIndex
                       << ", compressedData.size() = " << compressedData.size()
                       << ", (compressedSize + chunkSize) = " << (compressedSize + chunkSize);
                return angle::Result::Continue;
            }
        }

        // CRC of the chunk should match the values in the header.
        if (kEnableCRCForPipelineCache)
        {
            computedChunkCRC = angle::UpdateCRC32(
                computedChunkCRC, keyData.data() + sizeof(CacheDataHeader), chunkSize);
            if (computedChunkCRC != chunkCRC)
            {
                if (chunkCRC == 0)
                {
                    // This could be due to the cache being populated before
                    // kEnableCRCForPipelineCache was enabled.
                    WARN() << "Expected chunk CRC = " << chunkCRC
                           << ", Actual chunk CRC = " << computedChunkCRC;
                    return angle::Result::Continue;
                }

                // If the expected CRC is non-zero and does not match the actual CRC from the data,
                // there has been an unexpected data corruption.
                ERR() << "Expected chunk CRC = " << chunkCRC
                      << ", Actual chunk CRC = " << computedChunkCRC;

                ERR() << "Data extracted from the cache headers: " << std::hex
                      << ", compressedDataCRC = 0x" << compressedDataCRC << "numChunks = 0x"
                      << numChunks << ", uncompressedCacheDataSize = 0x"
                      << uncompressedCacheDataSize;

                FATAL() << "CRC check failed; possible pipeline cache data corruption.";
                return angle::Result::Stop;
            }
        }

        memcpy(compressedData.data() + compressedSize, keyData.data() + sizeof(CacheDataHeader),
               chunkSize);
        compressedSize += chunkSize;
    }

    // CRC for compressed data and size for decompressed data should match the values in the header.
    if (kEnableCRCForPipelineCache)
    {
        // Last chunk have CRC of the entire data.
        uint32_t computedCompressedDataCRC = computedChunkCRC;
        // Per chunk CRC check must handle any data corruption.  Assert is possible only if header
        // was incorrectly written in the first place (bug in the code), or all chunks headers were
        // corrupted in the exact same way, which is almost impossible.
        ASSERT(computedCompressedDataCRC == compressedDataCRC);
    }

    ANGLE_VK_CHECK(context,
                   angle::DecompressBlob(compressedData.data(), compressedSize,
                                         uncompressedCacheDataSize, uncompressedData),
                   VK_ERROR_INITIALIZATION_FAILED);

    if (uncompressedData->size() != uncompressedCacheDataSize)
    {
        WARN() << "Expected uncompressed size = " << uncompressedCacheDataSize
               << ", Actual uncompressed size = " << uncompressedData->size();
        return angle::Result::Continue;
    }

    *success = true;
    return angle::Result::Continue;
}

// Environment variable (and associated Android property) to enable Vulkan debug-utils markers
constexpr char kEnableDebugMarkersVarName[]      = "ANGLE_ENABLE_DEBUG_MARKERS";
constexpr char kEnableDebugMarkersPropertyName[] = "debug.angle.markers";

ANGLE_INLINE gl::ShadingRate GetShadingRateFromVkExtent(const VkExtent2D &extent)
{
    if (extent.width == 1)
    {
        if (extent.height == 1)
        {
            return gl::ShadingRate::_1x1;
        }
        else if (extent.height == 2)
        {
            return gl::ShadingRate::_1x2;
        }
    }
    else if (extent.width == 2)
    {
        if (extent.height == 1)
        {
            return gl::ShadingRate::_2x1;
        }
        else if (extent.height == 2)
        {
            return gl::ShadingRate::_2x2;
        }
    }
    else if (extent.width == 4)
    {
        if (extent.height == 2)
        {
            return gl::ShadingRate::_4x2;
        }
        else if (extent.height == 4)
        {
            return gl::ShadingRate::_4x4;
        }
    }

    return gl::ShadingRate::Undefined;
}

void DumpPipelineCacheGraph(Renderer *renderer, const std::ostringstream &graph)
{
    std::string dumpPath = renderer->getPipelineCacheGraphDumpPath();
    if (dumpPath.size() == 0)
    {
        WARN() << "No path supplied for pipeline cache graph dump!";
        return;
    }

    static std::atomic<uint32_t> sContextIndex(0);
    std::string filename = dumpPath;
    filename += angle::GetExecutableName();
    filename += std::to_string(sContextIndex.fetch_add(1));
    filename += ".dump";

    INFO() << "Dumping pipeline cache transition graph to: \"" << filename << "\"";

    std::ofstream out = std::ofstream(filename, std::ofstream::binary);
    if (!out.is_open())
    {
        ERR() << "Failed to open \"" << filename << "\"";
    }

    out << "digraph {\n" << " node [shape=box";
    if (renderer->getFeatures().supportsPipelineCreationFeedback.enabled)
    {
        out << ",color=green";
    }
    out << "]\n";
    out << graph.str();
    out << "}\n";
    out.close();
}

bool CanSupportMSRTSSForRGBA8(Renderer *renderer)
{
    // The support is checked for a basic 2D texture.
    constexpr VkImageUsageFlags kImageUsageFlags =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageCreateFlags imageCreateFlags =
        GetMinimalImageCreateFlags(renderer, gl::TextureType::_2D, kImageUsageFlags) |
        VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;

    bool supportsMSRTTUsageRGBA8 = vk::ImageHelper::FormatSupportsUsage(
        renderer, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
        kImageUsageFlags, imageCreateFlags, nullptr, nullptr,
        vk::ImageHelper::FormatSupportCheck::RequireMultisampling);
    bool supportsMSRTTUsageRGBA8SRGB = vk::ImageHelper::FormatSupportsUsage(
        renderer, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
        kImageUsageFlags, imageCreateFlags, nullptr, nullptr,
        vk::ImageHelper::FormatSupportCheck::RequireMultisampling);

    return supportsMSRTTUsageRGBA8 && supportsMSRTTUsageRGBA8SRGB;
}
}  // namespace

// OneOffCommandPool implementation.
OneOffCommandPool::OneOffCommandPool() : mProtectionType(vk::ProtectionType::InvalidEnum) {}

void OneOffCommandPool::init(vk::ProtectionType protectionType)
{
    ASSERT(!mCommandPool.valid());
    mProtectionType = protectionType;
}

void OneOffCommandPool::destroy(VkDevice device)
{
    std::unique_lock<angle::SimpleMutex> lock(mMutex);
    for (PendingOneOffCommands &pending : mPendingCommands)
    {
        pending.commandBuffer.releaseHandle();
    }
    mCommandPool.destroy(device);
    mProtectionType = vk::ProtectionType::InvalidEnum;
}

angle::Result OneOffCommandPool::getCommandBuffer(vk::ErrorContext *context,
                                                  vk::ScopedPrimaryCommandBuffer *commandBufferOut)
{
    std::unique_lock<angle::SimpleMutex> lock(mMutex);

    if (!mPendingCommands.empty() &&
        context->getRenderer()->hasResourceUseFinished(mPendingCommands.front().use))
    {
        commandBufferOut->assign(std::move(lock),
                                 std::move(mPendingCommands.front().commandBuffer));
        mPendingCommands.pop_front();
        // No need to explicitly call reset() on |commandBufferOut|, since the begin() call below
        // will do it implicitly.
    }
    else
    {
        if (!mCommandPool.valid())
        {
            VkCommandPoolCreateInfo createInfo = {};
            createInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                               VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            ASSERT(mProtectionType == vk::ProtectionType::Unprotected ||
                   mProtectionType == vk::ProtectionType::Protected);
            if (mProtectionType == vk::ProtectionType::Protected)
            {
                createInfo.flags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
            }
            createInfo.queueFamilyIndex = context->getRenderer()->getQueueFamilyIndex();
            ANGLE_VK_TRY(context, mCommandPool.init(context->getDevice(), createInfo));
        }

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount          = 1;
        allocInfo.commandPool                 = mCommandPool.getHandle();

        PrimaryCommandBuffer newCommandBuffer;
        ANGLE_VK_TRY(context, newCommandBuffer.init(context->getDevice(), allocInfo));
        commandBufferOut->assign(std::move(lock), std::move(newCommandBuffer));
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo         = nullptr;
    ANGLE_VK_TRY(context, commandBufferOut->get().begin(beginInfo));

    return angle::Result::Continue;
}

void OneOffCommandPool::releaseCommandBuffer(const QueueSerial &submitQueueSerial,
                                             vk::PrimaryCommandBuffer &&primary)
{
    std::unique_lock<angle::SimpleMutex> lock(mMutex);
    mPendingCommands.push_back({vk::ResourceUse(submitQueueSerial), std::move(primary)});
}

// Renderer implementation.
Renderer::Renderer()
    : mGlobalOps(nullptr),
      mLibVulkanLibrary(nullptr),
      mCapsInitialized(false),
      mInstanceVersion(0),
      mDeviceVersion(0),
      mInstance(VK_NULL_HANDLE),
      mEnableValidationLayers(false),
      mEnableDebugUtils(false),
      mAngleDebuggerMode(false),
      mEnabledICD(angle::vk::ICD::Default),
      mDebugUtilsMessenger(VK_NULL_HANDLE),
      mPhysicalDevice(VK_NULL_HANDLE),
      mPhysicalDeviceProperties(mPhysicalDeviceProperties2.properties),
      mCurrentQueueFamilyIndex(std::numeric_limits<uint32_t>::max()),
      mMaxVertexAttribDivisor(1),
      mMaxVertexAttribStride(0),
      mMaxColorInputAttachmentCount(0),
      mDefaultUniformBufferSize(kPreferredDefaultUniformBufferSize),
      mDevice(VK_NULL_HANDLE),
      mDeviceLost(false),
      mStagingBufferAlignment(1),
      mHostVisibleVertexConversionBufferMemoryTypeIndex(kInvalidMemoryTypeIndex),
      mDeviceLocalVertexConversionBufferMemoryTypeIndex(kInvalidMemoryTypeIndex),
      mVertexConversionBufferAlignment(1),
      mCurrentPipelineCacheBlobCacheSlotIndex(0),
      mPipelineCacheChunkCount(0),
      mPipelineCacheVkUpdateTimeout(kPipelineCacheVkUpdatePeriod),
      mPipelineCacheSizeAtLastSync(0),
      mPipelineCacheInitialized(false),
      mValidationMessageCount(0),
      mIsColorFramebufferFetchCoherent(false),
      mIsColorFramebufferFetchUsed(false),
      mCleanUpThread(this, &mCommandQueue),
      mSupportedBufferWritePipelineStageMask(0),
      mSupportedVulkanShaderStageMask(0),
      mMemoryAllocationTracker(MemoryAllocationTracker(this))
{
    VkFormatProperties invalid = {0, 0, kInvalidFormatFeatureFlags};
    mFormatProperties.fill(invalid);
    mStagingBufferMemoryTypeIndex.fill(kInvalidMemoryTypeIndex);

    // We currently don't have any big-endian devices in the list of supported platforms.  There are
    // a number of places in the Vulkan backend that make this assumption.  This assertion is made
    // early to fail immediately on big-endian platforms.
    ASSERT(IsLittleEndian());

    mDumpPipelineCacheGraph =
        (angle::GetEnvironmentVarOrAndroidProperty("ANGLE_DUMP_PIPELINE_CACHE_GRAPH",
                                                   "angle.dump_pipeline_cache_graph") == "1");

    mPipelineCacheGraphDumpPath = angle::GetEnvironmentVarOrAndroidProperty(
        "ANGLE_PIPELINE_CACHE_GRAPH_DUMP_PATH", "angle.pipeline_cache_graph_dump_path");
    if (mPipelineCacheGraphDumpPath.size() == 0)
    {
        mPipelineCacheGraphDumpPath = kDefaultPipelineCacheGraphDumpPath;
    }
}

Renderer::~Renderer() {}

bool Renderer::hasSharedGarbage()
{
    return !mSharedGarbageList.empty() || !mSuballocationGarbageList.empty();
}

void Renderer::onDestroy(vk::ErrorContext *context)
{
    if (isDeviceLost())
    {
        handleDeviceLost();
    }

    if (mPlaceHolderDescriptorSetLayout)
    {
        ASSERT(mPlaceHolderDescriptorSetLayout.unique());
        mPlaceHolderDescriptorSetLayout.reset();
    }

    mCleanUpThread.destroy(context);
    mCommandQueue.destroy(context);

    // mCommandQueue.destroy should already set "last completed" serials to infinite.
    cleanupGarbage(nullptr);
    ASSERT(!hasSharedGarbage());
    ASSERT(mOrphanedBufferBlockList.empty());

    mRefCountedEventRecycler.destroy(mDevice);

    for (OneOffCommandPool &oneOffCommandPool : mOneOffCommandPoolMap)
    {
        oneOffCommandPool.destroy(mDevice);
    }

    mPipelineCacheInitialized = false;
    mPipelineCache.destroy(mDevice);

    mSamplerCache.destroy(this);
    mYuvConversionCache.destroy(this);
    mVkFormatDescriptorCountMap.clear();

    mOutsideRenderPassCommandBufferRecycler.onDestroy();
    mRenderPassCommandBufferRecycler.onDestroy();

    mImageMemorySuballocator.destroy(this);
    mAllocator.destroy();

    // When the renderer is being destroyed, it is possible to check if all the allocated memory
    // throughout the execution has been freed.
    mMemoryAllocationTracker.onDestroy();

    if (mDevice)
    {
        vkDestroyDevice(mDevice, nullptr);
        mDevice = VK_NULL_HANDLE;
    }

    if (mDebugUtilsMessenger)
    {
        vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, nullptr);
    }

    logCacheStats();

    if (mInstance)
    {
        vkDestroyInstance(mInstance, nullptr);
        mInstance = VK_NULL_HANDLE;
    }

    if (mCompressEvent)
    {
        mCompressEvent->wait();
        mCompressEvent.reset();
    }

    mMemoryProperties.destroy();
    mPhysicalDevice = VK_NULL_HANDLE;

    mEnabledInstanceExtensions.clear();
    mEnabledDeviceExtensions.clear();

    ASSERT(!hasSharedGarbage());

    if (mLibVulkanLibrary)
    {
        angle::CloseSystemLibrary(mLibVulkanLibrary);
        mLibVulkanLibrary = nullptr;
    }

    if (!mPipelineCacheGraph.str().empty())
    {
        DumpPipelineCacheGraph(this, mPipelineCacheGraph);
    }
}

void Renderer::notifyDeviceLost()
{
    mDeviceLost = true;
    mGlobalOps->notifyDeviceLost();
}

bool Renderer::isDeviceLost() const
{
    return mDeviceLost;
}

angle::Result Renderer::enableInstanceExtensions(vk::ErrorContext *context,
                                                 const VulkanLayerVector &enabledInstanceLayerNames,
                                                 const char *wsiExtension,
                                                 UseVulkanSwapchain useVulkanSwapchain,
                                                 bool canLoadDebugUtils)
{
    // Enumerate instance extensions that are provided by the vulkan implementation and implicit
    // layers.
    uint32_t instanceExtensionCount = 0;
    {
        ANGLE_SCOPED_DISABLE_LSAN();
        ANGLE_SCOPED_DISABLE_MSAN();
        ANGLE_VK_TRY(context, vkEnumerateInstanceExtensionProperties(
                                  nullptr, &instanceExtensionCount, nullptr));
    }

    std::vector<VkExtensionProperties> instanceExtensionProps(instanceExtensionCount);
    if (instanceExtensionCount > 0)
    {
        ANGLE_SCOPED_DISABLE_LSAN();
        ANGLE_SCOPED_DISABLE_MSAN();
        ANGLE_VK_TRY(context, vkEnumerateInstanceExtensionProperties(
                                  nullptr, &instanceExtensionCount, instanceExtensionProps.data()));
        // In case fewer items were returned than requested, resize instanceExtensionProps to the
        // number of extensions returned (i.e. instanceExtensionCount).
        instanceExtensionProps.resize(instanceExtensionCount);
    }

    // Enumerate instance extensions that are provided by explicit layers.
    for (const char *layerName : enabledInstanceLayerNames)
    {
        uint32_t previousExtensionCount      = static_cast<uint32_t>(instanceExtensionProps.size());
        uint32_t instanceLayerExtensionCount = 0;
        {
            ANGLE_SCOPED_DISABLE_LSAN();
            ANGLE_SCOPED_DISABLE_MSAN();
            ANGLE_VK_TRY(context, vkEnumerateInstanceExtensionProperties(
                                      layerName, &instanceLayerExtensionCount, nullptr));
        }
        instanceExtensionProps.resize(previousExtensionCount + instanceLayerExtensionCount);
        {
            ANGLE_SCOPED_DISABLE_LSAN();
            ANGLE_SCOPED_DISABLE_MSAN();
            ANGLE_VK_TRY(context, vkEnumerateInstanceExtensionProperties(
                                      layerName, &instanceLayerExtensionCount,
                                      instanceExtensionProps.data() + previousExtensionCount));
        }
        // In case fewer items were returned than requested, resize instanceExtensionProps to the
        // number of extensions returned (i.e. instanceLayerExtensionCount).
        instanceExtensionProps.resize(previousExtensionCount + instanceLayerExtensionCount);
    }

    // Get the list of instance extensions that are available.
    vk::ExtensionNameList instanceExtensionNames;
    if (!instanceExtensionProps.empty())
    {
        for (const VkExtensionProperties &i : instanceExtensionProps)
        {
            instanceExtensionNames.push_back(i.extensionName);
        }
        std::sort(instanceExtensionNames.begin(), instanceExtensionNames.end(), StrLess);
    }

    // Set ANGLE features that depend on instance extensions
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSurfaceCapabilities2Extension,
        ExtensionFound(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, instanceExtensionNames) &&
            useVulkanSwapchain == UseVulkanSwapchain::Yes);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSurfaceProtectedCapabilitiesExtension,
                            ExtensionFound(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME,
                                           instanceExtensionNames) &&
                                useVulkanSwapchain == UseVulkanSwapchain::Yes);

    // TODO: Validation layer has a bug when vkGetPhysicalDeviceSurfaceFormats2KHR is called
    // on Mock ICD with surface handle set as VK_NULL_HANDLE. http://anglebug.com/42266098
    // b/267953710: VK_GOOGLE_surfaceless_query isn't working on some Samsung Xclipse builds
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSurfacelessQueryExtension,
        ExtensionFound(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME, instanceExtensionNames) &&
            useVulkanSwapchain == UseVulkanSwapchain::Yes && !isMockICDEnabled() && !IsXclipse());

    // VK_KHR_external_fence_capabilities and VK_KHR_extenral_semaphore_capabilities are promoted to
    // core in Vulkan 1.1
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExternalFenceCapabilities, true);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExternalSemaphoreCapabilities, true);

    // On macOS, there is no native Vulkan driver, so we need to enable the
    // portability enumeration extension to allow use of MoltenVK.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsPortabilityEnumeration,
        ExtensionFound(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, instanceExtensionNames));

    ANGLE_FEATURE_CONDITION(&mFeatures, enablePortabilityEnumeration,
                            mFeatures.supportsPortabilityEnumeration.enabled && IsApple());

    // Enable extensions that could be used
    if (useVulkanSwapchain == UseVulkanSwapchain::Yes)
    {
        mEnabledInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        if (ExtensionFound(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, instanceExtensionNames))
        {
            mEnabledInstanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }

        ANGLE_FEATURE_CONDITION(
            &mFeatures, supportsSurfaceMaintenance1,
            !isMockICDEnabled() && ExtensionFound(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
                                                  instanceExtensionNames));

        if (mFeatures.supportsSurfaceMaintenance1.enabled)
        {
            mEnabledInstanceExtensions.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
        }
    }

    if (wsiExtension)
    {
        mEnabledInstanceExtensions.push_back(wsiExtension);
    }

    mEnableDebugUtils = canLoadDebugUtils && mEnableValidationLayers &&
                        ExtensionFound(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instanceExtensionNames);

    if (mEnableDebugUtils)
    {
        mEnabledInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (mFeatures.supportsSurfaceCapabilities2Extension.enabled)
    {
        mEnabledInstanceExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    }

    if (mFeatures.supportsSurfaceProtectedCapabilitiesExtension.enabled)
    {
        mEnabledInstanceExtensions.push_back(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);
    }

    if (mFeatures.supportsSurfacelessQueryExtension.enabled)
    {
        mEnabledInstanceExtensions.push_back(VK_GOOGLE_SURFACELESS_QUERY_EXTENSION_NAME);
    }

    if (mFeatures.enablePortabilityEnumeration.enabled)
    {
        mEnabledInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }

    // Verify the required extensions are in the extension names set. Fail if not.
    std::sort(mEnabledInstanceExtensions.begin(), mEnabledInstanceExtensions.end(), StrLess);
    ANGLE_VK_TRY(context,
                 VerifyExtensionsPresent(instanceExtensionNames, mEnabledInstanceExtensions));

    return angle::Result::Continue;
}

angle::Result Renderer::initialize(vk::ErrorContext *context,
                                   vk::GlobalOps *globalOps,
                                   angle::vk::ICD desiredICD,
                                   uint32_t preferredVendorId,
                                   uint32_t preferredDeviceId,
                                   const uint8_t *preferredDeviceUuid,
                                   const uint8_t *preferredDriverUuid,
                                   VkDriverId preferredDriverId,
                                   UseDebugLayers useDebugLayers,
                                   const char *wsiExtension,
                                   const char *wsiLayer,
                                   angle::NativeWindowSystem nativeWindowSystem,
                                   const angle::FeatureOverrides &featureOverrides)
{
    bool canLoadDebugUtils = true;
#if defined(ANGLE_SHARED_LIBVULKAN)
    {
        ANGLE_SCOPED_DISABLE_MSAN();
        mLibVulkanLibrary = angle::vk::OpenLibVulkan();
        ANGLE_VK_CHECK(context, mLibVulkanLibrary, VK_ERROR_INITIALIZATION_FAILED);

        PFN_vkGetInstanceProcAddr vulkanLoaderGetInstanceProcAddr =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                angle::GetLibrarySymbol(mLibVulkanLibrary, "vkGetInstanceProcAddr"));

        // Set all vk* function ptrs
        volkInitializeCustom(vulkanLoaderGetInstanceProcAddr);

        uint32_t ver = volkGetInstanceVersion();
        if (!IsAndroid() && ver < VK_MAKE_VERSION(1, 1, 91))
        {
            // http://crbug.com/1205999 - non-Android Vulkan Loader versions before 1.1.91 have a
            // bug which prevents loading VK_EXT_debug_utils function pointers.
            canLoadDebugUtils = false;
        }
    }
#endif  // defined(ANGLE_SHARED_LIBVULKAN)

    mGlobalOps = globalOps;

    // While the validation layer is loaded by default whenever present, apidump layer
    // activation is controlled by an environment variable/android property allowing
    // the two layers to be controlled independently.
    bool enableApiDumpLayer =
        kEnableVulkanAPIDumpLayer && angle::GetEnvironmentVarOrAndroidProperty(
                                         "ANGLE_ENABLE_VULKAN_API_DUMP_LAYER",
                                         "debug.angle.enable_vulkan_api_dump_layer") == "1";

    bool loadLayers = (useDebugLayers != UseDebugLayers::No) || enableApiDumpLayer;
    angle::vk::ScopedVkLoaderEnvironment scopedEnvironment(loadLayers, desiredICD);
    bool debugLayersLoaded  = scopedEnvironment.canEnableDebugLayers();
    mEnableValidationLayers = debugLayersLoaded;
    enableApiDumpLayer      = enableApiDumpLayer && debugLayersLoaded;
    mEnabledICD             = scopedEnvironment.getEnabledICD();

    // Gather global layer properties.
    uint32_t instanceLayerCount = 0;
    {
        ANGLE_SCOPED_DISABLE_LSAN();
        ANGLE_SCOPED_DISABLE_MSAN();
        ANGLE_VK_TRY(context, vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    }

    std::vector<VkLayerProperties> instanceLayerProps(instanceLayerCount);
    if (instanceLayerCount > 0)
    {
        ANGLE_SCOPED_DISABLE_LSAN();
        ANGLE_SCOPED_DISABLE_MSAN();
        ANGLE_VK_TRY(context, vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                                                 instanceLayerProps.data()));
    }

    VulkanLayerVector enabledInstanceLayerNames;

    if (enableApiDumpLayer)
    {
        enabledInstanceLayerNames.push_back("VK_LAYER_LUNARG_api_dump");
    }

    if (mEnableValidationLayers)
    {
        const bool layersRequested = useDebugLayers == UseDebugLayers::Yes;
        mEnableValidationLayers = GetAvailableValidationLayers(instanceLayerProps, layersRequested,
                                                               &enabledInstanceLayerNames);
    }

    if (wsiLayer != nullptr)
    {
        enabledInstanceLayerNames.push_back(wsiLayer);
    }

    auto enumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
        vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

    uint32_t highestApiVersion = mInstanceVersion = VK_API_VERSION_1_0;
    if (enumerateInstanceVersion)
    {
        {
            ANGLE_SCOPED_DISABLE_LSAN();
            ANGLE_SCOPED_DISABLE_MSAN();
            ANGLE_VK_TRY(context, enumerateInstanceVersion(&mInstanceVersion));
        }

        if (IsVulkan11(mInstanceVersion))
        {
            // This is the highest version of core Vulkan functionality that ANGLE uses.  Per the
            // Vulkan spec, the application is allowed to specify a higher version than supported by
            // the instance.  ANGLE still respects the *device's* version.
            highestApiVersion = kPreferredVulkanAPIVersion;
        }
    }

    if (mInstanceVersion < angle::vk::kMinimumVulkanAPIVersion)
    {
        WARN() << "ANGLE Requires a minimum Vulkan instance version of 1.1";
        ANGLE_VK_TRY(context, VK_ERROR_INCOMPATIBLE_DRIVER);
    }

    const UseVulkanSwapchain useVulkanSwapchain = wsiExtension != nullptr || wsiLayer != nullptr
                                                      ? UseVulkanSwapchain::Yes
                                                      : UseVulkanSwapchain::No;
    ANGLE_TRY(enableInstanceExtensions(context, enabledInstanceLayerNames, wsiExtension,
                                       useVulkanSwapchain, canLoadDebugUtils));

    const std::string appName = angle::GetExecutableName();

    mApplicationInfo                    = {};
    mApplicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    mApplicationInfo.pApplicationName   = appName.c_str();
    mApplicationInfo.applicationVersion = 1;
    mApplicationInfo.pEngineName        = "ANGLE";
    mApplicationInfo.engineVersion      = 1;
    mApplicationInfo.apiVersion         = highestApiVersion;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.flags                = 0;
    instanceInfo.pApplicationInfo     = &mApplicationInfo;

    // Enable requested layers and extensions.
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(mEnabledInstanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames =
        mEnabledInstanceExtensions.empty() ? nullptr : mEnabledInstanceExtensions.data();

    instanceInfo.enabledLayerCount   = static_cast<uint32_t>(enabledInstanceLayerNames.size());
    instanceInfo.ppEnabledLayerNames = enabledInstanceLayerNames.data();

    // On macOS, there is no native Vulkan driver, so we need to enable the
    // portability enumeration extension to allow use of MoltenVK.
    if (mFeatures.enablePortabilityEnumeration.enabled)
    {
        instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    // Fine grain control of validation layer features
    const char *name                     = "VK_LAYER_KHRONOS_validation";
    const VkBool32 setting_validate_core = VK_TRUE;
    // SyncVal is very slow (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7285)
    // for VkEvent which causes a few tests fail on the bots. Disable syncVal if VkEvent is enabled
    // for now.
    const VkBool32 setting_validate_sync = IsAndroid() ? VK_FALSE : VK_TRUE;
    const VkBool32 setting_thread_safety = VK_TRUE;
    // http://anglebug.com/42265520 - Shader validation caching is broken on Android
    const VkBool32 setting_check_shaders = IsAndroid() ? VK_FALSE : VK_TRUE;
    // http://b/316013423 Disable QueueSubmit Synchronization Validation. Lots of failures and some
    // test timeout due to https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7285
    const VkBool32 setting_syncval_submit_time_validation   = VK_FALSE;
    const VkBool32 setting_syncval_message_extra_properties = VK_TRUE;
    const VkLayerSettingEXT layerSettings[]                 = {
        {name, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_core},
        {name, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_sync},
        {name, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_thread_safety},
        {name, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_check_shaders},
        {name, "syncval_submit_time_validation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                         &setting_syncval_submit_time_validation},
        {name, "syncval_message_extra_properties", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                         &setting_syncval_message_extra_properties},
    };
    VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {
        VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
        static_cast<uint32_t>(std::size(layerSettings)), layerSettings};
    if (mEnableValidationLayers)
    {
        vk::AddToPNextChain(&instanceInfo, &layerSettingsCreateInfo);
    }

    {
        ANGLE_SCOPED_DISABLE_MSAN();
        ANGLE_VK_TRY(context, vkCreateInstance(&instanceInfo, nullptr, &mInstance));
#if defined(ANGLE_SHARED_LIBVULKAN)
        // Load volk if we are linking dynamically
        volkLoadInstance(mInstance);
#endif  // defined(ANGLE_SHARED_LIBVULKAN)

        // For promoted extensions, initialize their entry points from the core version.
        initializeInstanceExtensionEntryPointsFromCore();
    }

    if (mEnableDebugUtils)
    {
        // Use the newer EXT_debug_utils if it exists.
#if !defined(ANGLE_SHARED_LIBVULKAN)
        InitDebugUtilsEXTFunctions(mInstance);
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)

        // Create the messenger callback.
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};

        constexpr VkDebugUtilsMessageSeverityFlagsEXT kSeveritiesToLog =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

        constexpr VkDebugUtilsMessageTypeFlagsEXT kMessagesToLog =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        messengerInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity = kSeveritiesToLog;
        messengerInfo.messageType     = kMessagesToLog;
        messengerInfo.pfnUserCallback = &DebugUtilsMessenger;
        messengerInfo.pUserData       = this;

        ANGLE_VK_TRY(context, vkCreateDebugUtilsMessengerEXT(mInstance, &messengerInfo, nullptr,
                                                             &mDebugUtilsMessenger));
    }

    uint32_t physicalDeviceCount = 0;
    ANGLE_VK_TRY(context, vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr));
    ANGLE_VK_CHECK(context, physicalDeviceCount > 0, VK_ERROR_INITIALIZATION_FAILED);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    ANGLE_VK_TRY(context, vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount,
                                                     physicalDevices.data()));
    ChoosePhysicalDevice(vkGetPhysicalDeviceProperties2, physicalDevices, mEnabledICD,
                         preferredVendorId, preferredDeviceId, preferredDeviceUuid,
                         preferredDriverUuid, preferredDriverId, &mPhysicalDevice,
                         &mPhysicalDeviceProperties2, &mPhysicalDeviceIDProperties,
                         &mDriverProperties);

    // The device version that is assumed by ANGLE is the minimum of the actual device version and
    // the highest it's allowed to use.
    mDeviceVersion = std::min(mPhysicalDeviceProperties.apiVersion, highestApiVersion);

    if (mDeviceVersion < angle::vk::kMinimumVulkanAPIVersion)
    {
        WARN() << "ANGLE Requires a minimum Vulkan device version of 1.1";
        ANGLE_VK_TRY(context, VK_ERROR_INCOMPATIBLE_DRIVER);
    }

    mGarbageCollectionFlushThreshold =
        static_cast<uint32_t>(mPhysicalDeviceProperties.limits.maxMemoryAllocationCount *
                              kPercentMaxMemoryAllocationCount);
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &mPhysicalDeviceFeatures);

    // Ensure we can find a graphics queue family.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);

    ANGLE_VK_CHECK(context, queueFamilyCount > 0, VK_ERROR_INITIALIZATION_FAILED);

    mQueueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount,
                                             mQueueFamilyProperties.data());

    uint32_t queueFamilyMatchCount = 0;
    // Try first for a protected graphics queue family
    uint32_t firstGraphicsQueueFamily = vk::QueueFamily::FindIndex(
        mQueueFamilyProperties,
        (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_PROTECTED_BIT), 0,
        &queueFamilyMatchCount);
    // else just a graphics queue family
    if (queueFamilyMatchCount == 0)
    {
        firstGraphicsQueueFamily = vk::QueueFamily::FindIndex(
            mQueueFamilyProperties, (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT), 0,
            &queueFamilyMatchCount);
    }
    ANGLE_VK_CHECK(context, queueFamilyMatchCount > 0, VK_ERROR_INITIALIZATION_FAILED);

    // Store the physical device memory properties so we can find the right memory pools.
    mMemoryProperties.init(mPhysicalDevice);
    ANGLE_VK_CHECK(context, mMemoryProperties.getMemoryTypeCount() > 0,
                   VK_ERROR_INITIALIZATION_FAILED);

    // The counters for the memory allocation tracker should be initialized.
    // Each memory allocation could be made in one of the available memory heaps. We initialize the
    // per-heap memory allocation trackers for MemoryAllocationType objects here, after
    // mMemoryProperties has been set up.
    mMemoryAllocationTracker.initMemoryTrackers();

    // Determine the threshold for pending garbage sizes.
    calculatePendingGarbageSizeLimit();

    ANGLE_TRY(
        setupDevice(context, featureOverrides, wsiLayer, useVulkanSwapchain, nativeWindowSystem));

    // If only one queue family, that's the only choice and the device is initialize with that.  If
    // there is more than one queue, we still create the device with the first queue family and hope
    // for the best.  We cannot wait for a window surface to know which supports present because of
    // EGL_KHR_surfaceless_context or simply pbuffers.  So far, only MoltenVk seems to expose
    // multiple queue families, and using the first queue family is fine with it.
    ANGLE_TRY(createDeviceAndQueue(context, firstGraphicsQueueFamily));

    // Initialize the format table.
    mFormatTable.initialize(this, &mNativeTextureCaps);

    // Null terminate the extension list returned for EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE.
    mEnabledInstanceExtensions.push_back(nullptr);

    for (vk::ProtectionType protectionType : angle::AllEnums<vk::ProtectionType>())
    {
        mOneOffCommandPoolMap[protectionType].init(protectionType);
    }

    // Initialize place holder descriptor set layout for empty DescriptorSetLayoutDesc
    ASSERT(!mPlaceHolderDescriptorSetLayout);
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.flags        = 0;
    createInfo.bindingCount = 0;
    createInfo.pBindings    = nullptr;

    mPlaceHolderDescriptorSetLayout = vk::DescriptorSetLayoutPtr::MakeShared(context->getDevice());
    ANGLE_VK_TRY(context, mPlaceHolderDescriptorSetLayout->init(context->getDevice(), createInfo));
    ASSERT(mPlaceHolderDescriptorSetLayout->valid());

    return angle::Result::Continue;
}

angle::Result Renderer::initializeMemoryAllocator(vk::ErrorContext *context)
{
    // This number matches Chromium and was picked by looking at memory usage of
    // Android apps. The allocator will start making blocks at 1/8 the max size
    // and builds up block size as needed before capping at the max set here.
    mPreferredLargeHeapBlockSize = 4 * 1024 * 1024;

    // Create VMA allocator
    ANGLE_VK_TRY(context,
                 mAllocator.init(mPhysicalDevice, mDevice, mInstance, mApplicationInfo.apiVersion,
                                 mPreferredLargeHeapBlockSize));

    // Figure out the alignment for default buffer allocations
    VkBufferCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.size                  = 4096;
    createInfo.usage                 = GetDefaultBufferUsageFlags(this);
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;

    vk::DeviceScoped<vk::Buffer> tempBuffer(mDevice);
    tempBuffer.get().init(mDevice, createInfo);

    VkMemoryRequirements defaultBufferMemoryRequirements;
    tempBuffer.get().getMemoryRequirements(mDevice, &defaultBufferMemoryRequirements);
    ASSERT(gl::isPow2(defaultBufferMemoryRequirements.alignment));

    const VkPhysicalDeviceLimits &limitsVk = getPhysicalDeviceProperties().limits;
    ASSERT(gl::isPow2(limitsVk.minUniformBufferOffsetAlignment));
    ASSERT(gl::isPow2(limitsVk.minStorageBufferOffsetAlignment));
    ASSERT(gl::isPow2(limitsVk.minTexelBufferOffsetAlignment));
    ASSERT(gl::isPow2(limitsVk.minMemoryMapAlignment));

    mDefaultBufferAlignment =
        std::max({static_cast<size_t>(limitsVk.minUniformBufferOffsetAlignment),
                  static_cast<size_t>(limitsVk.minStorageBufferOffsetAlignment),
                  static_cast<size_t>(limitsVk.minTexelBufferOffsetAlignment),
                  static_cast<size_t>(limitsVk.minMemoryMapAlignment),
                  static_cast<size_t>(defaultBufferMemoryRequirements.alignment)});

    // Initialize staging buffer memory type index and alignment.
    // These buffers will only be used as transfer sources or transfer targets.
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags requiredFlags, preferredFlags;
    bool persistentlyMapped = mFeatures.persistentlyMappedBuffers.enabled;

    // Uncached coherent staging buffer.
    requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    ANGLE_VK_TRY(context,
                 mAllocator.findMemoryTypeIndexForBufferInfo(
                     createInfo, requiredFlags, preferredFlags, persistentlyMapped,
                     &mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::UnCachedCoherent]));
    ASSERT(mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::UnCachedCoherent] !=
           kInvalidMemoryTypeIndex);

    // Cached coherent staging buffer.  Note coherent is preferred but not required, which means we
    // may get non-coherent memory type.
    requiredFlags   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    preferredFlags  = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkResult result = mAllocator.findMemoryTypeIndexForBufferInfo(
        createInfo, requiredFlags, preferredFlags, persistentlyMapped,
        &mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedPreferCoherent]);
    if (result == VK_SUCCESS)
    {
        ASSERT(mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedPreferCoherent] !=
               kInvalidMemoryTypeIndex);
    }
    else
    {
        // Android studio may not expose host cached memory pool. Fall back to host uncached.
        mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedPreferCoherent] =
            mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::UnCachedCoherent];
    }

    // Cached Non-coherent staging buffer
    requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    preferredFlags = 0;
    result         = mAllocator.findMemoryTypeIndexForBufferInfo(
        createInfo, requiredFlags, preferredFlags, persistentlyMapped,
        &mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedNonCoherent]);
    if (result == VK_SUCCESS)
    {
        ASSERT(mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedNonCoherent] !=
               kInvalidMemoryTypeIndex);
    }
    else
    {
        // Android studio may not expose host cached memory pool. Fall back to host uncached.
        mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedNonCoherent] =
            mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::UnCachedCoherent];
    }

    // Alignment
    mStagingBufferAlignment =
        static_cast<size_t>(mPhysicalDeviceProperties.limits.minMemoryMapAlignment);
    ASSERT(gl::isPow2(mPhysicalDeviceProperties.limits.nonCoherentAtomSize));
    ASSERT(gl::isPow2(mPhysicalDeviceProperties.limits.optimalBufferCopyOffsetAlignment));
    // Usually minTexelBufferOffsetAlignment is much smaller than  nonCoherentAtomSize
    ASSERT(gl::isPow2(mPhysicalDeviceProperties.limits.minTexelBufferOffsetAlignment));
    mStagingBufferAlignment = std::max(
        {mStagingBufferAlignment,
         static_cast<size_t>(mPhysicalDeviceProperties.limits.optimalBufferCopyOffsetAlignment),
         static_cast<size_t>(mPhysicalDeviceProperties.limits.nonCoherentAtomSize),
         static_cast<size_t>(mPhysicalDeviceProperties.limits.minTexelBufferOffsetAlignment)});
    ASSERT(gl::isPow2(mStagingBufferAlignment));

    // Device local vertex conversion buffer
    createInfo.usage = vk::kVertexBufferUsageFlags;
    requiredFlags    = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    preferredFlags   = 0;
    ANGLE_VK_TRY(context, mAllocator.findMemoryTypeIndexForBufferInfo(
                              createInfo, requiredFlags, preferredFlags, persistentlyMapped,
                              &mDeviceLocalVertexConversionBufferMemoryTypeIndex));
    ASSERT(mDeviceLocalVertexConversionBufferMemoryTypeIndex != kInvalidMemoryTypeIndex);

    // Host visible and non-coherent vertex conversion buffer, which is the same as non-coherent
    // staging buffer
    mHostVisibleVertexConversionBufferMemoryTypeIndex =
        mStagingBufferMemoryTypeIndex[vk::MemoryCoherency::CachedNonCoherent];

    // We may use compute shader to do conversion, so we must meet
    // minStorageBufferOffsetAlignment requirement as well. Also take into account non-coherent
    // alignment requirements.
    mVertexConversionBufferAlignment = std::max(
        {vk::kVertexBufferAlignment,
         static_cast<size_t>(mPhysicalDeviceProperties.limits.minStorageBufferOffsetAlignment),
         static_cast<size_t>(mPhysicalDeviceProperties.limits.nonCoherentAtomSize),
         static_cast<size_t>(defaultBufferMemoryRequirements.alignment)});
    ASSERT(gl::isPow2(mVertexConversionBufferAlignment));

    return angle::Result::Continue;
}

// The following features and properties are not promoted to any core Vulkan versions (up to Vulkan
// 1.3):
//
// - VK_EXT_line_rasterization:                        bresenhamLines (feature)
// - VK_EXT_provoking_vertex:                          provokingVertexLast (feature)
// - VK_EXT_vertex_attribute_divisor:                  vertexAttributeInstanceRateDivisor (feature),
//                                                     maxVertexAttribDivisor (property)
// - VK_EXT_transform_feedback:                        transformFeedback (feature),
//                                                     geometryStreams (feature)
// - VK_EXT_index_type_uint8:                          indexTypeUint8 (feature)
// - VK_EXT_device_memory_report:                      deviceMemoryReport (feature)
// - VK_EXT_multisampled_render_to_single_sampled:     multisampledRenderToSingleSampled (feature)
// - VK_EXT_image_2d_view_of_3d:                       image2DViewOf3D (feature)
//                                                     sampler2DViewOf3D (feature)
// - VK_EXT_custom_border_color:                       customBorderColors (feature)
//                                                     customBorderColorWithoutFormat (feature)
// - VK_EXT_depth_clamp_zero_one:                      depthClampZeroOne (feature)
// - VK_EXT_depth_clip_control:                        depthClipControl (feature)
// - VK_EXT_primitives_generated_query:                primitivesGeneratedQuery (feature),
//                                                     primitivesGeneratedQueryWithRasterizerDiscard
//                                                                                        (property)
// - VK_EXT_primitive_topology_list_restart:           primitiveTopologyListRestart (feature)
// - VK_EXT_graphics_pipeline_library:                 graphicsPipelineLibrary (feature),
//                                                     graphicsPipelineLibraryFastLinking (property)
// - VK_KHR_fragment_shading_rate:                     pipelineFragmentShadingRate (feature)
// - VK_EXT_fragment_shader_interlock:                 fragmentShaderPixelInterlock (feature)
// - VK_EXT_pipeline_robustness:                       pipelineRobustness (feature)
// - VK_EXT_pipeline_protected_access:                 pipelineProtectedAccess (feature)
// - VK_EXT_rasterization_order_attachment_access or
//   VK_ARM_rasterization_order_attachment_access:     rasterizationOrderColorAttachmentAccess
//                                                                                   (feature)
//                                                     rasterizationOrderDepthAttachmentAccess
//                                                                                   (feature)
//                                                     rasterizationOrderStencilAttachmentAccess
//                                                                                   (feature)
// - VK_EXT_swapchain_maintenance1:                    swapchainMaintenance1 (feature)
// - VK_EXT_legacy_dithering:                          supportsLegacyDithering (feature)
// - VK_EXT_physical_device_drm:                       hasPrimary (property),
//                                                     hasRender (property)
// - VK_EXT_host_image_copy:                           hostImageCopy (feature),
//                                                     pCopySrcLayouts (property),
//                                                     pCopyDstLayouts (property),
//                                                     identicalMemoryTypeRequirements (property)
// - VK_ANDROID_external_format_resolve:               externalFormatResolve (feature)
// - VK_EXT_vertex_input_dynamic_state:                vertexInputDynamicState (feature)
// - VK_KHR_dynamic_rendering_local_read:              dynamicRenderingLocalRead (feature)
// - VK_EXT_shader_atomic_float                        shaderImageFloat32Atomics (feature)
// - VK_EXT_image_compression_control                  imageCompressionControl (feature)
// - VK_EXT_image_compression_control_swapchain        imageCompressionControlSwapchain (feature)
//
void Renderer::appendDeviceExtensionFeaturesNotPromoted(
    const vk::ExtensionNameList &deviceExtensionNames,
    VkPhysicalDeviceFeatures2KHR *deviceFeatures,
    VkPhysicalDeviceProperties2 *deviceProperties)
{
    if (ExtensionFound(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mLineRasterizationFeatures);
    }

    if (ExtensionFound(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mProvokingVertexFeatures);
    }

    if (ExtensionFound(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mVertexAttributeDivisorFeatures);
        vk::AddToPNextChain(deviceProperties, &mVertexAttributeDivisorProperties);
    }

    if (ExtensionFound(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mTransformFeedbackFeatures);
    }

    if (ExtensionFound(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mIndexTypeUint8Features);
    }

    if (ExtensionFound(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mMemoryReportFeatures);
    }

    if (ExtensionFound(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME,
                       deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mMultisampledRenderToSingleSampledFeatures);
    }

    if (ExtensionFound(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mImage2dViewOf3dFeatures);
    }

    if (ExtensionFound(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mCustomBorderColorFeatures);
    }

    if (ExtensionFound(VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mDepthClampZeroOneFeatures);
    }

    if (ExtensionFound(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mDepthClipControlFeatures);
    }

    if (ExtensionFound(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mPrimitivesGeneratedQueryFeatures);
    }

    if (ExtensionFound(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mPrimitiveTopologyListRestartFeatures);
    }

    if (ExtensionFound(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mGraphicsPipelineLibraryFeatures);
        vk::AddToPNextChain(deviceProperties, &mGraphicsPipelineLibraryProperties);
    }

    if (ExtensionFound(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mFragmentShadingRateFeatures);
        vk::AddToPNextChain(deviceProperties, &mFragmentShadingRateProperties);
    }

    if (ExtensionFound(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mFragmentShaderInterlockFeatures);
    }

    if (ExtensionFound(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mPipelineRobustnessFeatures);
    }

    if (ExtensionFound(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mPipelineProtectedAccessFeatures);
    }

    // The EXT and ARM versions are interchangeable. The structs and enums alias each other.
    if (ExtensionFound(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
                       deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mRasterizationOrderAttachmentAccessFeatures);
    }
    else if (ExtensionFound(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
                            deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mRasterizationOrderAttachmentAccessFeatures);
    }

    if (ExtensionFound(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mShaderAtomicFloatFeatures);
    }

    if (ExtensionFound(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mSwapchainMaintenance1Features);
    }

    if (ExtensionFound(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mDitheringFeatures);
    }

    if (ExtensionFound(VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceProperties, &mDrmProperties);
    }

    if (ExtensionFound(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME, deviceExtensionNames))
    {
        // VkPhysicalDeviceHostImageCopyPropertiesEXT has a count + array query.  Typically, that
        // requires getting the properties once with a nullptr array, to get the count, and then
        // again with an array of that size.  For simplicity, ANGLE just uses an array that's big
        // enough.  If that array goes terribly large in the future, ANGLE may lose knowledge of
        // some likely esoteric layouts, which doesn't really matter.
        constexpr uint32_t kMaxLayoutCount = 50;
        mHostImageCopySrcLayoutsStorage.resize(kMaxLayoutCount, VK_IMAGE_LAYOUT_UNDEFINED);
        mHostImageCopyDstLayoutsStorage.resize(kMaxLayoutCount, VK_IMAGE_LAYOUT_UNDEFINED);
        mHostImageCopyProperties.copySrcLayoutCount = kMaxLayoutCount;
        mHostImageCopyProperties.copyDstLayoutCount = kMaxLayoutCount;
        mHostImageCopyProperties.pCopySrcLayouts    = mHostImageCopySrcLayoutsStorage.data();
        mHostImageCopyProperties.pCopyDstLayouts    = mHostImageCopyDstLayoutsStorage.data();

        vk::AddToPNextChain(deviceFeatures, &mHostImageCopyFeatures);
        vk::AddToPNextChain(deviceProperties, &mHostImageCopyProperties);
    }

    if (ExtensionFound(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mVertexInputDynamicStateFeatures);
    }

#if defined(ANGLE_PLATFORM_ANDROID)
    if (ExtensionFound(VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mExternalFormatResolveFeatures);
        vk::AddToPNextChain(deviceProperties, &mExternalFormatResolveProperties);
    }
#endif

    if (ExtensionFound(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mDynamicRenderingLocalReadFeatures);
    }

    if (ExtensionFound(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mBlendOperationAdvancedFeatures);
    }

    if (ExtensionFound(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mImageCompressionControlFeatures);
    }
    if (ExtensionFound(VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME,
                       deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mImageCompressionControlSwapchainFeatures);
    }
}

// The following features and properties used by ANGLE have been promoted to Vulkan 1.1:
//
// - (unpublished VK_KHR_subgroup):         supportedStages (property),
//                                          supportedOperations (property)
// - (unpublished VK_KHR_protected_memory): protectedMemory (feature)
// - VK_KHR_sampler_ycbcr_conversion:       samplerYcbcrConversion (feature)
// - VK_KHR_multiview:                      multiview (feature),
//                                          maxMultiviewViewCount (property)
// - VK_KHR_16bit_storage:                  storageBuffer16BitAccess (feature)
//                                          uniformAndStorageBuffer16BitAccess (feature)
//                                          storagePushConstant16 (feature)
//                                          storageInputOutput16 (feature)
// - VK_KHR_variable_pointers:              variablePointers (feature)
//                                          variablePointersStorageBuffer (feature)
//
//
// Note that subgroup and protected memory features and properties came from unpublished extensions
// and are core in Vulkan 1.1.
//
void Renderer::appendDeviceExtensionFeaturesPromotedTo11(
    const vk::ExtensionNameList &deviceExtensionNames,
    VkPhysicalDeviceFeatures2KHR *deviceFeatures,
    VkPhysicalDeviceProperties2 *deviceProperties)
{
    vk::AddToPNextChain(deviceProperties, &mSubgroupProperties);
    vk::AddToPNextChain(deviceFeatures, &mProtectedMemoryFeatures);
    vk::AddToPNextChain(deviceFeatures, &mSamplerYcbcrConversionFeatures);
    vk::AddToPNextChain(deviceFeatures, &mMultiviewFeatures);
    vk::AddToPNextChain(deviceProperties, &mMultiviewProperties);
    vk::AddToPNextChain(deviceFeatures, &m16BitStorageFeatures);
    vk::AddToPNextChain(deviceFeatures, &mVariablePointersFeatures);
    vk::AddToPNextChain(deviceProperties, &mMaintenance3Properties);
}

// The following features and properties used by ANGLE have been promoted to Vulkan 1.2:
//
// - VK_KHR_shader_float16_int8:            shaderFloat16 (feature),
//                                          shaderInt8 (feature)
// - VK_KHR_depth_stencil_resolve:          supportedDepthResolveModes (property),
//                                          independentResolveNone (property)
// - VK_KHR_driver_properties:              driverName (property),
//                                          driverID (property)
// - VK_KHR_shader_subgroup_extended_types: shaderSubgroupExtendedTypes (feature)
// - VK_EXT_host_query_reset:               hostQueryReset (feature)
// - VK_KHR_imageless_framebuffer:          imagelessFramebuffer (feature)
// - VK_KHR_timeline_semaphore:             timelineSemaphore (feature)
// - VK_KHR_8bit_storage                    storageBuffer8BitAccess (feature)
//                                          uniformAndStorageBuffer8BitAccess (feature)
//                                          storagePushConstant8 (feature)
// - VK_KHR_shader_float_controls           shaderRoundingModeRTEFloat16 (property)
//                                          shaderRoundingModeRTEFloat32 (property)
//                                          shaderRoundingModeRTEFloat64 (property)
//                                          shaderRoundingModeRTZFloat16 (property)
//                                          shaderRoundingModeRTZFloat32 (property)
//                                          shaderRoundingModeRTZFloat64 (property)
//                                          shaderDenormPreserveFloat16 (property)
//                                          shaderDenormPreserveFloat16 (property)
//                                          shaderDenormPreserveFloat16 (property)
//                                          shaderDenormFlushToZeroFloat16 (property)
//                                          shaderDenormFlushToZeroFloat32 (property)
//                                          shaderDenormFlushToZeroFloat64 (property)
//                                          shaderSignedZeroInfNanPreserveFloat16 (property)
//                                          shaderSignedZeroInfNanPreserveFloat32 (property)
//                                          shaderSignedZeroInfNanPreserveFloat64 (property)
// - VK_KHR_uniform_buffer_standard_layout: uniformBufferStandardLayout (feature)
//
// Note that supportedDepthResolveModes is used just to check if the property struct is populated.
// ANGLE always uses VK_RESOLVE_MODE_SAMPLE_ZERO_BIT for both depth and stencil, and support for
// this bit is mandatory as long as the extension (or Vulkan 1.2) exists.
//
void Renderer::appendDeviceExtensionFeaturesPromotedTo12(
    const vk::ExtensionNameList &deviceExtensionNames,
    VkPhysicalDeviceFeatures2KHR *deviceFeatures,
    VkPhysicalDeviceProperties2 *deviceProperties)
{
    if (ExtensionFound(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceProperties, &mFloatControlProperties);
    }

    if (ExtensionFound(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mShaderFloat16Int8Features);
    }

    if (ExtensionFound(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceProperties, &mDepthStencilResolveProperties);
    }

    if (ExtensionFound(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceProperties, &mDriverProperties);
    }

    if (ExtensionFound(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mSubgroupExtendedTypesFeatures);
    }

    if (ExtensionFound(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mHostQueryResetFeatures);
    }

    if (ExtensionFound(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mImagelessFramebufferFeatures);
    }

    if (ExtensionFound(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mTimelineSemaphoreFeatures);
    }

    if (ExtensionFound(VK_KHR_8BIT_STORAGE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &m8BitStorageFeatures);
    }

    if (ExtensionFound(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mUniformBufferStandardLayoutFeatures);
    }
}

// The following features and properties used by ANGLE have been promoted to Vulkan 1.3:
//
// - VK_EXT_extended_dynamic_state:          extendedDynamicState (feature)
// - VK_EXT_extended_dynamic_state2:         extendedDynamicState2 (feature),
//                                           extendedDynamicState2LogicOp (feature)
// - VK_KHR_synchronization2:                synchronization2 (feature)
// - VK_KHR_dynamic_rendering:               dynamicRendering (feature)
// - VK_KHR_maintenance5:                    maintenance5 (feature)
// - VK_EXT_texture_compression_astc_hdr:    textureCompressionASTC_HDR(feature)
//
// Note that VK_EXT_extended_dynamic_state2 is partially promoted to Vulkan 1.3.  If ANGLE creates a
// Vulkan 1.3 device, it would still need to enable this extension separately for
// extendedDynamicState2LogicOp.
//
void Renderer::appendDeviceExtensionFeaturesPromotedTo13(
    const vk::ExtensionNameList &deviceExtensionNames,
    VkPhysicalDeviceFeatures2KHR *deviceFeatures,
    VkPhysicalDeviceProperties2 *deviceProperties)
{
    if (ExtensionFound(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mExtendedDynamicStateFeatures);
    }

    if (ExtensionFound(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mExtendedDynamicState2Features);
    }

    if (ExtensionFound(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mSynchronization2Features);
    }

    if (ExtensionFound(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mDynamicRenderingFeatures);
    }

    if (ExtensionFound(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mMaintenance5Features);
    }

    if (ExtensionFound(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME, deviceExtensionNames))
    {
        vk::AddToPNextChain(deviceFeatures, &mTextureCompressionASTCHDRFeatures);
    }
}

void Renderer::queryDeviceExtensionFeatures(const vk::ExtensionNameList &deviceExtensionNames)
{
    // Default initialize all extension features to false.
    mLineRasterizationFeatures = {};
    mLineRasterizationFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT;

    mProvokingVertexFeatures = {};
    mProvokingVertexFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT;

    mVertexAttributeDivisorFeatures = {};
    mVertexAttributeDivisorFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;

    mVertexAttributeDivisorProperties = {};
    mVertexAttributeDivisorProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;

    mTransformFeedbackFeatures = {};
    mTransformFeedbackFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT;

    mIndexTypeUint8Features       = {};
    mIndexTypeUint8Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT;

    mSubgroupProperties       = {};
    mSubgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

    mSubgroupExtendedTypesFeatures = {};
    mSubgroupExtendedTypesFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;

    mMemoryReportFeatures = {};
    mMemoryReportFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT;

    mShaderFloat16Int8Features = {};
    mShaderFloat16Int8Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;

    mDepthStencilResolveProperties = {};
    mDepthStencilResolveProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;

    mCustomBorderColorFeatures = {};
    mCustomBorderColorFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;

    mMultisampledRenderToSingleSampledFeatures = {};
    mMultisampledRenderToSingleSampledFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT;

    mImage2dViewOf3dFeatures = {};
    mImage2dViewOf3dFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT;

    mMultiviewFeatures       = {};
    mMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;

    mMultiviewProperties       = {};
    mMultiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;

    mMaintenance3Properties       = {};
    mMaintenance3Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;

    mDriverProperties       = {};
    mDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

    mSamplerYcbcrConversionFeatures = {};
    mSamplerYcbcrConversionFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;

    mProtectedMemoryFeatures       = {};
    mProtectedMemoryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;

    mHostQueryResetFeatures       = {};
    mHostQueryResetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;

    mDepthClampZeroOneFeatures = {};
    mDepthClampZeroOneFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_EXT;

    mDepthClipControlFeatures = {};
    mDepthClipControlFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;

    mPrimitivesGeneratedQueryFeatures = {};
    mPrimitivesGeneratedQueryFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT;

    mPrimitiveTopologyListRestartFeatures = {};
    mPrimitiveTopologyListRestartFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;

    mExtendedDynamicStateFeatures = {};
    mExtendedDynamicStateFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

    mExtendedDynamicState2Features = {};
    mExtendedDynamicState2Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;

    mGraphicsPipelineLibraryFeatures = {};
    mGraphicsPipelineLibraryFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT;

    mGraphicsPipelineLibraryProperties = {};
    mGraphicsPipelineLibraryProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT;

    mVertexInputDynamicStateFeatures = {};
    mVertexInputDynamicStateFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;

    mDynamicRenderingFeatures = {};
    mDynamicRenderingFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;

    mDynamicRenderingLocalReadFeatures = {};
    mDynamicRenderingLocalReadFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR;

    mFragmentShadingRateFeatures = {};
    mFragmentShadingRateFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;

    mFragmentShadingRateProperties = {};
    mFragmentShadingRateProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

    mFragmentShaderInterlockFeatures = {};
    mFragmentShaderInterlockFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;

    mImagelessFramebufferFeatures = {};
    mImagelessFramebufferFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;

    mPipelineRobustnessFeatures = {};
    mPipelineRobustnessFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT;

    mPipelineProtectedAccessFeatures = {};
    mPipelineProtectedAccessFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES_EXT;

    mRasterizationOrderAttachmentAccessFeatures = {};
    mRasterizationOrderAttachmentAccessFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT;

    mMaintenance5Features       = {};
    mMaintenance5Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR;

    mShaderAtomicFloatFeatures = {};
    mShaderAtomicFloatFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;

    mSwapchainMaintenance1Features = {};
    mSwapchainMaintenance1Features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT;

    mDitheringFeatures       = {};
    mDitheringFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT;

    mDrmProperties       = {};
    mDrmProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT;

    mTimelineSemaphoreFeatures = {};
    mTimelineSemaphoreFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;

    mHostImageCopyFeatures       = {};
    mHostImageCopyFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT;

    mHostImageCopyProperties = {};
    mHostImageCopyProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT;

    m8BitStorageFeatures       = {};
    m8BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;

    m16BitStorageFeatures       = {};
    m16BitStorageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;

    mSynchronization2Features       = {};
    mSynchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

    mBlendOperationAdvancedFeatures = {};
    mBlendOperationAdvancedFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;

    mVariablePointersFeatures = {};
    mVariablePointersFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES_KHR;

    // Rounding and denormal caps from VK_KHR_float_controls_properties
    mFloatControlProperties       = {};
    mFloatControlProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES;

    mImageCompressionControlFeatures = {};
    mImageCompressionControlFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT;

    mImageCompressionControlSwapchainFeatures = {};
    mImageCompressionControlSwapchainFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT;

    mTextureCompressionASTCHDRFeatures = {};
    mTextureCompressionASTCHDRFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;

    mUniformBufferStandardLayoutFeatures = {};
    mUniformBufferStandardLayoutFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;

#if defined(ANGLE_PLATFORM_ANDROID)
    mExternalFormatResolveFeatures = {};
    mExternalFormatResolveFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID;

    mExternalFormatResolveProperties = {};
    mExternalFormatResolveProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID;
#endif

    // Query features and properties.
    VkPhysicalDeviceFeatures2KHR deviceFeatures = {};
    deviceFeatures.sType                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceProperties2 deviceProperties = {};
    deviceProperties.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    appendDeviceExtensionFeaturesNotPromoted(deviceExtensionNames, &deviceFeatures,
                                             &deviceProperties);
    appendDeviceExtensionFeaturesPromotedTo11(deviceExtensionNames, &deviceFeatures,
                                              &deviceProperties);
    appendDeviceExtensionFeaturesPromotedTo12(deviceExtensionNames, &deviceFeatures,
                                              &deviceProperties);
    appendDeviceExtensionFeaturesPromotedTo13(deviceExtensionNames, &deviceFeatures,
                                              &deviceProperties);

    vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceProperties2(mPhysicalDevice, &deviceProperties);

    // Clean up pNext chains
    mLineRasterizationFeatures.pNext                  = nullptr;
    mMemoryReportFeatures.pNext                       = nullptr;
    mProvokingVertexFeatures.pNext                    = nullptr;
    mVertexAttributeDivisorFeatures.pNext             = nullptr;
    mVertexAttributeDivisorProperties.pNext           = nullptr;
    mTransformFeedbackFeatures.pNext                  = nullptr;
    mIndexTypeUint8Features.pNext                     = nullptr;
    mSubgroupProperties.pNext                         = nullptr;
    mSubgroupExtendedTypesFeatures.pNext              = nullptr;
    mCustomBorderColorFeatures.pNext                  = nullptr;
    mShaderFloat16Int8Features.pNext                  = nullptr;
    mDepthStencilResolveProperties.pNext              = nullptr;
    mMultisampledRenderToSingleSampledFeatures.pNext  = nullptr;
    mImage2dViewOf3dFeatures.pNext                    = nullptr;
    mMultiviewFeatures.pNext                          = nullptr;
    mMultiviewProperties.pNext                        = nullptr;
    mDriverProperties.pNext                           = nullptr;
    mSamplerYcbcrConversionFeatures.pNext             = nullptr;
    mProtectedMemoryFeatures.pNext                    = nullptr;
    mHostQueryResetFeatures.pNext                     = nullptr;
    mDepthClampZeroOneFeatures.pNext                  = nullptr;
    mDepthClipControlFeatures.pNext                   = nullptr;
    mPrimitivesGeneratedQueryFeatures.pNext           = nullptr;
    mPrimitiveTopologyListRestartFeatures.pNext       = nullptr;
    mExtendedDynamicStateFeatures.pNext               = nullptr;
    mExtendedDynamicState2Features.pNext              = nullptr;
    mGraphicsPipelineLibraryFeatures.pNext            = nullptr;
    mGraphicsPipelineLibraryProperties.pNext          = nullptr;
    mVertexInputDynamicStateFeatures.pNext            = nullptr;
    mDynamicRenderingFeatures.pNext                   = nullptr;
    mDynamicRenderingLocalReadFeatures.pNext          = nullptr;
    mFragmentShadingRateFeatures.pNext                = nullptr;
    mFragmentShaderInterlockFeatures.pNext            = nullptr;
    mImagelessFramebufferFeatures.pNext               = nullptr;
    mPipelineRobustnessFeatures.pNext                 = nullptr;
    mPipelineProtectedAccessFeatures.pNext            = nullptr;
    mRasterizationOrderAttachmentAccessFeatures.pNext = nullptr;
    mShaderAtomicFloatFeatures.pNext                  = nullptr;
    mMaintenance5Features.pNext                       = nullptr;
    mSwapchainMaintenance1Features.pNext              = nullptr;
    mDitheringFeatures.pNext                          = nullptr;
    mDrmProperties.pNext                              = nullptr;
    mTimelineSemaphoreFeatures.pNext                  = nullptr;
    mHostImageCopyFeatures.pNext                      = nullptr;
    mHostImageCopyProperties.pNext                    = nullptr;
    m8BitStorageFeatures.pNext                        = nullptr;
    m16BitStorageFeatures.pNext                       = nullptr;
    mSynchronization2Features.pNext                   = nullptr;
    mBlendOperationAdvancedFeatures.pNext             = nullptr;
    mVariablePointersFeatures.pNext                   = nullptr;
    mFloatControlProperties.pNext                     = nullptr;
    mImageCompressionControlFeatures.pNext            = nullptr;
    mImageCompressionControlSwapchainFeatures.pNext   = nullptr;
    mTextureCompressionASTCHDRFeatures.pNext          = nullptr;
    mUniformBufferStandardLayoutFeatures.pNext        = nullptr;
    mMaintenance3Properties.pNext                     = nullptr;
#if defined(ANGLE_PLATFORM_ANDROID)
    mExternalFormatResolveFeatures.pNext   = nullptr;
    mExternalFormatResolveProperties.pNext = nullptr;
#endif
}

// See comment above appendDeviceExtensionFeaturesNotPromoted.  Additional extensions are enabled
// here which don't have feature structs:
//
// - VK_KHR_shared_presentable_image
// - VK_EXT_memory_budget
// - VK_KHR_incremental_present
// - VK_EXT_queue_family_foreign
// - VK_ANDROID_external_memory_android_hardware_buffer
// - VK_GGP_frame_token
// - VK_KHR_external_memory_fd
// - VK_KHR_external_memory_fuchsia
// - VK_KHR_external_semaphore_fd
// - VK_KHR_external_fence_fd
// - VK_FUCHSIA_external_semaphore
// - VK_EXT_shader_stencil_export
// - VK_EXT_load_store_op_none
// - VK_QCOM_render_pass_store_ops
// - VK_GOOGLE_display_timing
// - VK_EXT_external_memory_dma_buf
// - VK_EXT_image_drm_format_modifier
// - VK_EXT_blend_operation_advanced
// - VK_EXT_full_screen_exclusive
// - VK_EXT_image_compression_control
// - VK_EXT_image_compression_control_swapchain
//
void Renderer::enableDeviceExtensionsNotPromoted(const vk::ExtensionNameList &deviceExtensionNames)
{
    if (mFeatures.supportsSharedPresentableImageExtension.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    }

    if (mFeatures.supportsDepthClampZeroOne.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLAMP_ZERO_ONE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mDepthClampZeroOneFeatures);
    }

    if (mFeatures.supportsMemoryBudget.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }

    if (mFeatures.supportsIncrementalPresent.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME);
    }

#if defined(ANGLE_PLATFORM_ANDROID)
    if (mFeatures.supportsAndroidHardwareBuffer.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
        mEnabledDeviceExtensions.push_back(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    }
#else
    ASSERT(!mFeatures.supportsAndroidHardwareBuffer.enabled);
#endif

#if defined(ANGLE_PLATFORM_GGP)
    if (mFeatures.supportsGGPFrameToken.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_GGP_FRAME_TOKEN_EXTENSION_NAME);
    }
#else
    ASSERT(!mFeatures.supportsGGPFrameToken.enabled);
#endif

    if (mFeatures.supportsExternalMemoryFd.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalMemoryFuchsia.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalSemaphoreFd.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalFenceFd.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalSemaphoreFuchsia.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    }

    if (mFeatures.supportsShaderStencilExport.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME);
    }

    if (mFeatures.supportsRenderPassLoadStoreOpNone.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);
    }
    else if (mFeatures.supportsRenderPassStoreOpNone.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME);
    }

    if (mFeatures.supportsTimestampSurfaceAttribute.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
    }

    if (mFeatures.bresenhamLineRasterization.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mLineRasterizationFeatures);
    }

    if (mFeatures.provokingVertex.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mProvokingVertexFeatures);
    }

    if (mVertexAttributeDivisorFeatures.vertexAttributeInstanceRateDivisor)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mVertexAttributeDivisorFeatures);

        // We only store 8 bit divisor in GraphicsPipelineDesc so capping value & we emulate if
        // exceeded
        mMaxVertexAttribDivisor =
            std::min(mVertexAttributeDivisorProperties.maxVertexAttribDivisor,
                     static_cast<uint32_t>(std::numeric_limits<uint8_t>::max()));
    }

    if (mFeatures.supportsTransformFeedbackExtension.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mTransformFeedbackFeatures);
    }

    if (mFeatures.supportsCustomBorderColor.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mCustomBorderColorFeatures);
    }

    if (mFeatures.supportsIndexTypeUint8.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mIndexTypeUint8Features);
    }

    if (mFeatures.supportsMultisampledRenderToSingleSampled.enabled)
    {
        mEnabledDeviceExtensions.push_back(
            VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mMultisampledRenderToSingleSampledFeatures);
    }

    if (mFeatures.logMemoryReportCallbacks.enabled || mFeatures.logMemoryReportStats.enabled)
    {
        ASSERT(mMemoryReportFeatures.deviceMemoryReport);
        mEnabledDeviceExtensions.push_back(VK_EXT_DEVICE_MEMORY_REPORT_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalMemoryDmaBufAndModifiers.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME);
        mEnabledDeviceExtensions.push_back(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    }

    if (mFeatures.supportsExternalMemoryHost.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
    }

    if (mFeatures.supportsDepthClipControl.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mDepthClipControlFeatures);
    }

    if (mFeatures.supportsPrimitivesGeneratedQuery.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mPrimitivesGeneratedQueryFeatures);
    }

    if (mFeatures.supportsPrimitiveTopologyListRestart.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mPrimitiveTopologyListRestartFeatures);
    }

    if (mFeatures.supportsBlendOperationAdvanced.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mBlendOperationAdvancedFeatures);
    }

    if (mFeatures.supportsGraphicsPipelineLibrary.enabled)
    {
        // VK_EXT_graphics_pipeline_library requires VK_KHR_pipeline_library
        ASSERT(ExtensionFound(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, deviceExtensionNames));
        mEnabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

        mEnabledDeviceExtensions.push_back(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mGraphicsPipelineLibraryFeatures);
    }

    if (mFeatures.supportsFragmentShadingRate.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mFragmentShadingRateFeatures);
    }

    if (mFeatures.supportsFragmentShaderPixelInterlock.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mFragmentShaderInterlockFeatures);
    }

    if (mFeatures.supportsPipelineRobustness.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mPipelineRobustnessFeatures);
    }

    if (mFeatures.supportsPipelineProtectedAccess.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PIPELINE_PROTECTED_ACCESS_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mPipelineProtectedAccessFeatures);
    }

    if (mFeatures.supportsRasterizationOrderAttachmentAccess.enabled)
    {
        if (ExtensionFound(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
                           deviceExtensionNames))
        {
            mEnabledDeviceExtensions.push_back(
                VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
        }
        else
        {
            ASSERT(ExtensionFound(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME,
                                  deviceExtensionNames));
            mEnabledDeviceExtensions.push_back(
                VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
        }
        vk::AddToPNextChain(&mEnabledFeatures, &mRasterizationOrderAttachmentAccessFeatures);
    }

    if (!mFeatures.emulateR32fImageAtomicExchange.enabled)
    {
        ASSERT(ExtensionFound(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, deviceExtensionNames));
        mEnabledDeviceExtensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mShaderAtomicFloatFeatures);
    }

    if (mFeatures.supportsImage2dViewOf3d.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mImage2dViewOf3dFeatures);
    }

    if (mFeatures.supportsSwapchainMaintenance1.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mSwapchainMaintenance1Features);
    }

    if (mFeatures.supportsLegacyDithering.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_LEGACY_DITHERING_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mDitheringFeatures);
    }

    if (mFeatures.supportsFormatFeatureFlags2.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    }

    if (mFeatures.supportsHostImageCopy.enabled)
    {
        // VK_EXT_host_image_copy requires VK_KHR_copy_commands2 and VK_KHR_format_feature_flags2.
        // VK_KHR_format_feature_flags2 is enabled separately.
        ASSERT(ExtensionFound(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME, deviceExtensionNames));
        ASSERT(ExtensionFound(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME, deviceExtensionNames));
        mEnabledDeviceExtensions.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

        mEnabledDeviceExtensions.push_back(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mHostImageCopyFeatures);
    }

    if (getFeatures().supportsVertexInputDynamicState.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mVertexInputDynamicStateFeatures);
    }

    if (getFeatures().supportsDynamicRenderingLocalRead.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mDynamicRenderingLocalReadFeatures);
    }

    if (getFeatures().supportsImageCompressionControl.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mImageCompressionControlFeatures);
    }

    if (getFeatures().supportsImageCompressionControlSwapchain.enabled)
    {
        mEnabledDeviceExtensions.push_back(
            VK_EXT_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mImageCompressionControlSwapchainFeatures);
    }

#if defined(ANGLE_PLATFORM_WINDOWS)
    // We only need the VK_EXT_full_screen_exclusive extension if we are opting
    // out of it via VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT (i.e. working
    // around driver bugs).
    if (getFeatures().supportsFullScreenExclusive.enabled &&
        getFeatures().forceDisableFullScreenExclusive.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
    }
#endif

#if defined(ANGLE_PLATFORM_ANDROID)
    if (mFeatures.supportsExternalFormatResolve.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mExternalFormatResolveFeatures);
    }
#endif
}

// See comment above appendDeviceExtensionFeaturesPromotedTo11.  Additional extensions are enabled
// here which don't have feature structs:
//
// - VK_KHR_get_memory_requirements2
// - VK_KHR_bind_memory2
// - VK_KHR_maintenance1
// - VK_KHR_external_memory
// - VK_KHR_external_semaphore
// - VK_KHR_external_fence
//
void Renderer::enableDeviceExtensionsPromotedTo11(const vk::ExtensionNameList &deviceExtensionNames)
{
    // OVR_multiview disallows multiview with geometry and tessellation, so don't request these
    // features.
    mMultiviewFeatures.multiviewGeometryShader     = VK_FALSE;
    mMultiviewFeatures.multiviewTessellationShader = VK_FALSE;

    if (mFeatures.supportsMultiview.enabled)
    {
        vk::AddToPNextChain(&mEnabledFeatures, &mMultiviewFeatures);
    }

    if (mFeatures.supportsYUVSamplerConversion.enabled)
    {
        vk::AddToPNextChain(&mEnabledFeatures, &mSamplerYcbcrConversionFeatures);
    }

    if (mFeatures.supportsProtectedMemory.enabled)
    {
        vk::AddToPNextChain(&mEnabledFeatures, &mProtectedMemoryFeatures);
    }

    if (mFeatures.supports16BitStorageBuffer.enabled ||
        mFeatures.supports16BitUniformAndStorageBuffer.enabled ||
        mFeatures.supports16BitPushConstant.enabled || mFeatures.supports16BitInputOutput.enabled)
    {
        vk::AddToPNextChain(&mEnabledFeatures, &m16BitStorageFeatures);
    }

    vk::AddToPNextChain(&mEnabledFeatures, &mVariablePointersFeatures);
}

// See comment above appendDeviceExtensionFeaturesPromotedTo12.  Additional extensions are enabled
// here which don't have feature structs:
//
// - VK_KHR_create_renderpass2
// - VK_KHR_image_format_list
// - VK_KHR_shader_float_controls
// - VK_KHR_spirv_1_4
// - VK_KHR_sampler_mirror_clamp_to_edge
// - VK_KHR_depth_stencil_resolve
//
void Renderer::enableDeviceExtensionsPromotedTo12(const vk::ExtensionNameList &deviceExtensionNames)
{
    if (mFeatures.supportsRenderpass2.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    }

    if (mFeatures.supportsImageFormatList.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    }

    // There are several FP related modes defined as properties from
    // VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION, and there could be a scenario where the extension is
    // supported but none of the modes are supported. Here we enable the extension if it is found.
    if (ExtensionFound(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, deviceExtensionNames))
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    }

    if (mFeatures.supportsSPIRV14.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    }

    if (mFeatures.supportsSamplerMirrorClampToEdge.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    }

    if (mFeatures.supportsDepthStencilResolve.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    }

    if (mFeatures.allowGenerateMipmapWithCompute.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mSubgroupExtendedTypesFeatures);
    }

    if (mFeatures.supportsShaderFloat16.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mShaderFloat16Int8Features);
    }

    if (mFeatures.supportsHostQueryReset.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mHostQueryResetFeatures);
    }

    if (mFeatures.supportsImagelessFramebuffer.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mImagelessFramebufferFeatures);
    }

    if (mFeatures.supportsTimelineSemaphore.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mTimelineSemaphoreFeatures);
    }

    if (mFeatures.supports8BitStorageBuffer.enabled ||
        mFeatures.supports8BitUniformAndStorageBuffer.enabled ||
        mFeatures.supports8BitPushConstant.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &m8BitStorageFeatures);
    }
    if (mFeatures.supportsUniformBufferStandardLayout.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mUniformBufferStandardLayoutFeatures);
    }
}

// See comment above appendDeviceExtensionFeaturesPromotedTo13.
void Renderer::enableDeviceExtensionsPromotedTo13(const vk::ExtensionNameList &deviceExtensionNames)
{
    if (mFeatures.supportsPipelineCreationFeedback.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    }

    if (mFeatures.supportsExtendedDynamicState.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mExtendedDynamicStateFeatures);
    }

    if (mFeatures.supportsExtendedDynamicState2.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mExtendedDynamicState2Features);
    }

    if (mFeatures.supportsSynchronization2.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mSynchronization2Features);
    }

    if (getFeatures().supportsDynamicRendering.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mDynamicRenderingFeatures);
    }

    if (getFeatures().supportsMaintenance5.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mMaintenance5Features);
    }

    if (getFeatures().supportsTextureCompressionAstcHdr.enabled)
    {
        mEnabledDeviceExtensions.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);
        vk::AddToPNextChain(&mEnabledFeatures, &mTextureCompressionASTCHDRFeatures);
    }
}

angle::Result Renderer::enableDeviceExtensions(vk::ErrorContext *context,
                                               const angle::FeatureOverrides &featureOverrides,
                                               UseVulkanSwapchain useVulkanSwapchain,
                                               angle::NativeWindowSystem nativeWindowSystem)
{
    // Enumerate device extensions that are provided by the vulkan
    // implementation and implicit layers.
    uint32_t deviceExtensionCount = 0;
    ANGLE_VK_TRY(context, vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr,
                                                               &deviceExtensionCount, nullptr));

    // Work-around a race condition in the Android platform during Android start-up, that can cause
    // the second call to vkEnumerateDeviceExtensionProperties to have an additional extension.  In
    // that case, the second call will return VK_INCOMPLETE.  To work-around that, add 1 to
    // deviceExtensionCount and ask for one more extension property than the first call said there
    // were.  See: http://anglebug.com/42265209 and internal-to-Google bug: b/206733351.
    deviceExtensionCount++;
    std::vector<VkExtensionProperties> deviceExtensionProps(deviceExtensionCount);
    ANGLE_VK_TRY(context,
                 vkEnumerateDeviceExtensionProperties(
                     mPhysicalDevice, nullptr, &deviceExtensionCount, deviceExtensionProps.data()));
    // In case fewer items were returned than requested, resize deviceExtensionProps to the number
    // of extensions returned (i.e. deviceExtensionCount).  See: b/208937840
    deviceExtensionProps.resize(deviceExtensionCount);

    // Enumerate device extensions that are provided by explicit layers.
    for (const char *layerName : mEnabledDeviceLayerNames)
    {
        uint32_t previousExtensionCount    = static_cast<uint32_t>(deviceExtensionProps.size());
        uint32_t deviceLayerExtensionCount = 0;
        ANGLE_VK_TRY(context, vkEnumerateDeviceExtensionProperties(
                                  mPhysicalDevice, layerName, &deviceLayerExtensionCount, nullptr));
        deviceExtensionProps.resize(previousExtensionCount + deviceLayerExtensionCount);
        ANGLE_VK_TRY(context, vkEnumerateDeviceExtensionProperties(
                                  mPhysicalDevice, layerName, &deviceLayerExtensionCount,
                                  deviceExtensionProps.data() + previousExtensionCount));
        // In case fewer items were returned than requested, resize deviceExtensionProps to the
        // number of extensions returned (i.e. deviceLayerExtensionCount).
        deviceExtensionProps.resize(previousExtensionCount + deviceLayerExtensionCount);
    }

    // Get the list of device extensions that are available.
    vk::ExtensionNameList deviceExtensionNames;
    if (!deviceExtensionProps.empty())
    {
        ASSERT(deviceExtensionNames.size() <= deviceExtensionProps.size());
        for (const VkExtensionProperties &prop : deviceExtensionProps)
        {
            deviceExtensionNames.push_back(prop.extensionName);

            if (strcmp(prop.extensionName, VK_EXT_LEGACY_DITHERING_EXTENSION_NAME) == 0)
            {
                mLegacyDitheringVersion = prop.specVersion;
            }
        }
        std::sort(deviceExtensionNames.begin(), deviceExtensionNames.end(), StrLess);
    }

    if (useVulkanSwapchain == UseVulkanSwapchain::Yes)
    {
        mEnabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    // Query extensions and their features.
    queryDeviceExtensionFeatures(deviceExtensionNames);

    // Initialize features and workarounds.
    initFeatures(deviceExtensionNames, featureOverrides, useVulkanSwapchain, nativeWindowSystem);

    // App based feature overrides.
    appBasedFeatureOverrides(deviceExtensionNames);

    // Enable extensions that could be used
    enableDeviceExtensionsNotPromoted(deviceExtensionNames);
    enableDeviceExtensionsPromotedTo11(deviceExtensionNames);
    enableDeviceExtensionsPromotedTo12(deviceExtensionNames);
    enableDeviceExtensionsPromotedTo13(deviceExtensionNames);

    std::sort(mEnabledDeviceExtensions.begin(), mEnabledDeviceExtensions.end(), StrLess);
    ANGLE_VK_TRY(context, VerifyExtensionsPresent(deviceExtensionNames, mEnabledDeviceExtensions));

    return angle::Result::Continue;
}

void Renderer::initDeviceExtensionEntryPoints()
{
#if !defined(ANGLE_SHARED_LIBVULKAN)
    // Device entry points
    if (mFeatures.supportsTransformFeedbackExtension.enabled)
    {
        InitTransformFeedbackEXTFunctions(mDevice);
    }
    if (getFeatures().supportsLogicOpDynamicState.enabled)
    {
        // VK_EXT_extended_dynamic_state2 is only partially core in Vulkan 1.3.  If the logicOp
        // dynamic state (only from the extension) is used, need to load the entry points from the
        // extension
        InitExtendedDynamicState2EXTFunctions(mDevice);
    }
    if (mFeatures.supportsFragmentShadingRate.enabled)
    {
        InitFragmentShadingRateKHRDeviceFunction(mDevice);
    }
    if (mFeatures.supportsTimestampSurfaceAttribute.enabled)
    {
        InitGetPastPresentationTimingGoogleFunction(mDevice);
    }
    if (mFeatures.supportsHostImageCopy.enabled)
    {
        InitHostImageCopyFunctions(mDevice);
    }
    if (mFeatures.supportsVertexInputDynamicState.enabled)
    {
        InitVertexInputDynamicStateEXTFunctions(mDevice);
    }
    if (mFeatures.supportsDynamicRenderingLocalRead.enabled)
    {
        InitDynamicRenderingLocalReadFunctions(mDevice);
    }
    if (mFeatures.supportsExternalSemaphoreFd.enabled ||
        mFeatures.supportsExternalSemaphoreFuchsia.enabled)
    {
        InitExternalSemaphoreFdFunctions(mDevice);
    }

    if (mFeatures.supportsExternalFenceFd.enabled)
    {
        InitExternalFenceFdFunctions(mDevice);
    }

#    if defined(ANGLE_PLATFORM_ANDROID)
    if (mFeatures.supportsAndroidHardwareBuffer.enabled)
    {
        InitExternalMemoryHardwareBufferANDROIDFunctions(mDevice);
    }
#    endif
    if (mFeatures.supportsSynchronization2.enabled)
    {
        InitSynchronization2Functions(mDevice);
    }
    // Extensions promoted to Vulkan 1.2
    {
        if (mFeatures.supportsHostQueryReset.enabled)
        {
            InitHostQueryResetFunctions(mDevice);
        }
        if (mFeatures.supportsRenderpass2.enabled)
        {
            InitRenderPass2KHRFunctions(mDevice);
        }
    }
    // Extensions promoted to Vulkan 1.3
    {
        if (mFeatures.supportsExtendedDynamicState.enabled)
        {
            InitExtendedDynamicStateEXTFunctions(mDevice);
        }
        if (mFeatures.supportsExtendedDynamicState2.enabled)
        {
            InitExtendedDynamicState2EXTFunctions(mDevice);
        }
        if (mFeatures.supportsDynamicRendering.enabled)
        {
            InitDynamicRenderingFunctions(mDevice);
        }
    }
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)

    // For promoted extensions, initialize their entry points from the core version.
    initializeDeviceExtensionEntryPointsFromCore();
}

angle::Result Renderer::setupDevice(vk::ErrorContext *context,
                                    const angle::FeatureOverrides &featureOverrides,
                                    const char *wsiLayer,
                                    UseVulkanSwapchain useVulkanSwapchain,
                                    angle::NativeWindowSystem nativeWindowSystem)
{
    uint32_t deviceLayerCount = 0;
    ANGLE_VK_TRY(context,
                 vkEnumerateDeviceLayerProperties(mPhysicalDevice, &deviceLayerCount, nullptr));

    std::vector<VkLayerProperties> deviceLayerProps(deviceLayerCount);
    ANGLE_VK_TRY(context, vkEnumerateDeviceLayerProperties(mPhysicalDevice, &deviceLayerCount,
                                                           deviceLayerProps.data()));

    mEnabledDeviceLayerNames.clear();
    if (mEnableValidationLayers)
    {
        mEnableValidationLayers =
            GetAvailableValidationLayers(deviceLayerProps, false, &mEnabledDeviceLayerNames);
    }

    if (wsiLayer != nullptr)
    {
        mEnabledDeviceLayerNames.push_back(wsiLayer);
    }

    mEnabledFeatures       = {};
    mEnabledFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    ANGLE_TRY(
        enableDeviceExtensions(context, featureOverrides, useVulkanSwapchain, nativeWindowSystem));

    // Used to support cubemap array:
    mEnabledFeatures.features.imageCubeArray = mFeatures.supportsImageCubeArray.enabled;
    // Used to support framebuffers with multiple attachments:
    mEnabledFeatures.features.independentBlend = mPhysicalDeviceFeatures.independentBlend;
    // Used to support multi_draw_indirect
    mEnabledFeatures.features.multiDrawIndirect = mPhysicalDeviceFeatures.multiDrawIndirect;
    mEnabledFeatures.features.drawIndirectFirstInstance =
        mPhysicalDeviceFeatures.drawIndirectFirstInstance;
    // Used to support robust buffer access, if VK_EXT_pipeline_robustness is not supported.
    if (!mFeatures.supportsPipelineRobustness.enabled)
    {
        mEnabledFeatures.features.robustBufferAccess = mPhysicalDeviceFeatures.robustBufferAccess;
    }
    // Used to support Anisotropic filtering:
    mEnabledFeatures.features.samplerAnisotropy = mPhysicalDeviceFeatures.samplerAnisotropy;
    // Used to support wide lines:
    mEnabledFeatures.features.wideLines = mPhysicalDeviceFeatures.wideLines;
    // Used to emulate transform feedback:
    mEnabledFeatures.features.vertexPipelineStoresAndAtomics =
        mPhysicalDeviceFeatures.vertexPipelineStoresAndAtomics;
    // Used to implement storage buffers and images in the fragment shader:
    mEnabledFeatures.features.fragmentStoresAndAtomics =
        mPhysicalDeviceFeatures.fragmentStoresAndAtomics;
    // Used to emulate the primitives generated query:
    mEnabledFeatures.features.pipelineStatisticsQuery =
        !mFeatures.supportsPrimitivesGeneratedQuery.enabled &&
        mFeatures.supportsPipelineStatisticsQuery.enabled;
    // Used to support geometry shaders:
    mEnabledFeatures.features.geometryShader = mPhysicalDeviceFeatures.geometryShader;
    // Used to support EXT/OES_gpu_shader5:
    mEnabledFeatures.features.shaderImageGatherExtended =
        mPhysicalDeviceFeatures.shaderImageGatherExtended;
    // Used to support EXT/OES_gpu_shader5:
    mEnabledFeatures.features.shaderUniformBufferArrayDynamicIndexing =
        mPhysicalDeviceFeatures.shaderUniformBufferArrayDynamicIndexing;
    mEnabledFeatures.features.shaderSampledImageArrayDynamicIndexing =
        mPhysicalDeviceFeatures.shaderSampledImageArrayDynamicIndexing;
    // Used to support APPLE_clip_distance
    mEnabledFeatures.features.shaderClipDistance = mPhysicalDeviceFeatures.shaderClipDistance;
    // Used to support OES_sample_shading
    mEnabledFeatures.features.sampleRateShading = mPhysicalDeviceFeatures.sampleRateShading;
    // Used to support EXT_depth_clamp and depth clears through draw calls
    mEnabledFeatures.features.depthClamp = mPhysicalDeviceFeatures.depthClamp;
    // Used to support EXT_polygon_offset_clamp
    mEnabledFeatures.features.depthBiasClamp = mPhysicalDeviceFeatures.depthBiasClamp;
    // Used to support NV_polygon_mode / ANGLE_polygon_mode
    mEnabledFeatures.features.fillModeNonSolid = mPhysicalDeviceFeatures.fillModeNonSolid;
    // Used to support EXT_clip_cull_distance
    mEnabledFeatures.features.shaderCullDistance = mPhysicalDeviceFeatures.shaderCullDistance;
    // Used to support tessellation Shader:
    mEnabledFeatures.features.tessellationShader = mPhysicalDeviceFeatures.tessellationShader;
    // Used to support EXT_blend_func_extended
    mEnabledFeatures.features.dualSrcBlend = mPhysicalDeviceFeatures.dualSrcBlend;
    // Used to support ANGLE_logic_op and GLES1
    mEnabledFeatures.features.logicOp = mPhysicalDeviceFeatures.logicOp;
    // Used to support EXT_multisample_compatibility
    mEnabledFeatures.features.alphaToOne = mPhysicalDeviceFeatures.alphaToOne;
    // Used to support 16bit-integers in shader code
    mEnabledFeatures.features.shaderInt16 = mPhysicalDeviceFeatures.shaderInt16;
    // Used to support 64bit-integers in shader code
    mEnabledFeatures.features.shaderInt64 = mPhysicalDeviceFeatures.shaderInt64;
    // Used to support 64bit-floats in shader code
    mEnabledFeatures.features.shaderFloat64 =
        mFeatures.supportsShaderFloat64.enabled && mPhysicalDeviceFeatures.shaderFloat64;

    if (!vk::OutsideRenderPassCommandBuffer::ExecutesInline() ||
        !vk::RenderPassCommandBuffer::ExecutesInline())
    {
        mEnabledFeatures.features.inheritedQueries = mPhysicalDeviceFeatures.inheritedQueries;
    }

    return angle::Result::Continue;
}

angle::Result Renderer::createDeviceAndQueue(vk::ErrorContext *context, uint32_t queueFamilyIndex)
{
    mCurrentQueueFamilyIndex = queueFamilyIndex;

    vk::QueueFamily queueFamily;
    queueFamily.initialize(mQueueFamilyProperties[queueFamilyIndex], queueFamilyIndex);
    ANGLE_VK_CHECK(context, queueFamily.getDeviceQueueCount() > 0, VK_ERROR_INITIALIZATION_FAILED);

    // We enable protected context only if both supportsProtectedMemory and device also supports
    // protected. There are cases we have to disable supportsProtectedMemory feature due to driver
    // bugs.
    bool enableProtectedContent =
        queueFamily.supportsProtected() && mFeatures.supportsProtectedMemory.enabled;

    uint32_t queueCount = std::min(queueFamily.getDeviceQueueCount(),
                                   static_cast<uint32_t>(egl::ContextPriority::EnumCount));

    uint32_t queueCreateInfoCount              = 1;
    VkDeviceQueueCreateInfo queueCreateInfo[1] = {};
    queueCreateInfo[0].sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[0].flags = enableProtectedContent ? VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT : 0;
    queueCreateInfo[0].queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo[0].queueCount       = queueCount;
    queueCreateInfo[0].pQueuePriorities = vk::QueueFamily::kQueuePriorities;

    // Setup device initialization struct
    VkDeviceCreateInfo createInfo    = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.flags                 = 0;
    createInfo.queueCreateInfoCount  = queueCreateInfoCount;
    createInfo.pQueueCreateInfos     = queueCreateInfo;
    createInfo.enabledLayerCount     = static_cast<uint32_t>(mEnabledDeviceLayerNames.size());
    createInfo.ppEnabledLayerNames   = mEnabledDeviceLayerNames.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(mEnabledDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames =
        mEnabledDeviceExtensions.empty() ? nullptr : mEnabledDeviceExtensions.data();
    mEnabledDeviceExtensions.push_back(nullptr);

    // Enable core features without assuming VkPhysicalDeviceFeatures2KHR is accepted in the
    // pNext chain of VkDeviceCreateInfo.
    createInfo.pEnabledFeatures = &mEnabledFeatures.features;

    // Append the feature structs chain to the end of createInfo structs chain.
    if (mEnabledFeatures.pNext)
    {
        vk::AppendToPNextChain(&createInfo, mEnabledFeatures.pNext);
    }

    if (mFeatures.logMemoryReportCallbacks.enabled || mFeatures.logMemoryReportStats.enabled)
    {
        ASSERT(mMemoryReportFeatures.deviceMemoryReport);

        mMemoryReportCallback       = {};
        mMemoryReportCallback.sType = VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT;
        mMemoryReportCallback.pfnUserCallback = &MemoryReportCallback;
        mMemoryReportCallback.pUserData       = this;
        vk::AddToPNextChain(&createInfo, &mMemoryReportCallback);
    }

    // Create the list of expected VVL messages to suppress.  Done before creating the device, as it
    // may also generate messages.
    initializeValidationMessageSuppressions();

    ANGLE_VK_TRY(context, vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));
#if defined(ANGLE_SHARED_LIBVULKAN)
    // Load volk if we are loading dynamically
    volkLoadDevice(mDevice);
#endif  // defined(ANGLE_SHARED_LIBVULKAN)

    initDeviceExtensionEntryPoints();

    ANGLE_TRY(mCommandQueue.init(context, queueFamily, enableProtectedContent, queueCount));
    ANGLE_TRY(mCleanUpThread.init());

    if (mFeatures.forceMaxUniformBufferSize16KB.enabled)
    {
        mDefaultUniformBufferSize = kMinDefaultUniformBufferSize;
    }
    // Cap it with the driver limit
    mDefaultUniformBufferSize = std::min(
        mDefaultUniformBufferSize, getPhysicalDeviceProperties().limits.maxUniformBufferRange);

    // Vulkan pipeline cache will be initialized lazily in ensurePipelineCacheInitialized() method.
    ASSERT(!mPipelineCacheInitialized);
    ASSERT(!mPipelineCache.valid());

    // Track the set of supported pipeline stages.  This is used when issuing image layout
    // transitions that cover many stages (such as AllGraphicsReadOnly) to mask out unsupported
    // stages, which avoids enumerating every possible combination of stages in the layouts.
    VkPipelineStageFlags unsupportedStages = 0;
    mSupportedVulkanShaderStageMask =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    mSupportedBufferWritePipelineStageMask =
        VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    if (!mPhysicalDeviceFeatures.tessellationShader)
    {
        unsupportedStages |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                             VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    else
    {
        mSupportedVulkanShaderStageMask |=
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        mSupportedBufferWritePipelineStageMask |=
            VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
            VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if (!mPhysicalDeviceFeatures.geometryShader)
    {
        unsupportedStages |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    else
    {
        mSupportedVulkanShaderStageMask |= VK_SHADER_STAGE_GEOMETRY_BIT;
        mSupportedBufferWritePipelineStageMask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }

    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mSupportedBufferWritePipelineStageMask |= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
    }

    // Initialize the barrierData tables by removing unsupported pipeline stage bits
    InitializeEventStageToVkPipelineStageFlagsMap(&mEventStageToPipelineStageFlagsMap,
                                                  ~unsupportedStages);
    InitializeImageLayoutAndMemoryBarrierDataMap(&mImageLayoutAndMemoryBarrierDataMap,
                                                 ~unsupportedStages);

    ANGLE_TRY(initializeMemoryAllocator(context));

    // Log the memory heap stats when the device has been initialized (when debugging).
    mMemoryAllocationTracker.onDeviceInit();

    return angle::Result::Continue;
}

void Renderer::calculatePendingGarbageSizeLimit()
{
    // To find the threshold, we want the memory heap that has the largest size among other heaps.
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);
    ASSERT(memoryProperties.memoryHeapCount > 0);

    VkDeviceSize maxHeapSize = memoryProperties.memoryHeaps[0].size;
    for (size_t i = 0; i < memoryProperties.memoryHeapCount; i++)
    {
        VkDeviceSize heapSize = memoryProperties.memoryHeaps[i].size;
        if (maxHeapSize < heapSize)
        {
            maxHeapSize = heapSize;
        }
    }

    // We set the limit to a portion of the heap size we found.
    constexpr float kGarbageSizeLimitCoefficient = 0.2f;
    mPendingGarbageSizeLimit =
        static_cast<VkDeviceSize>(maxHeapSize * kGarbageSizeLimitCoefficient);
}

void Renderer::initializeValidationMessageSuppressions()
{
    // Build the list of validation errors that are currently expected and should be skipped.
    mSkippedValidationMessages.insert(mSkippedValidationMessages.end(), kSkippedMessages,
                                      kSkippedMessages + ArraySize(kSkippedMessages));
    if (!getFeatures().supportsPrimitiveTopologyListRestart.enabled)
    {
        mSkippedValidationMessages.insert(
            mSkippedValidationMessages.end(), kNoListRestartSkippedMessages,
            kNoListRestartSkippedMessages + ArraySize(kNoListRestartSkippedMessages));
    }

    if (getFeatures().exposeES32ForTesting.enabled)
    {
        mSkippedValidationMessages.insert(
            mSkippedValidationMessages.end(), kExposeNonConformantSkippedMessages,
            kExposeNonConformantSkippedMessages + ArraySize(kExposeNonConformantSkippedMessages));
    }

    if (getFeatures().useVkEventForImageBarrier.enabled &&
        (!vk::OutsideRenderPassCommandBuffer::ExecutesInline() ||
         !vk::RenderPassCommandBuffer::ExecutesInline()))
    {
        mSkippedValidationMessages.insert(
            mSkippedValidationMessages.end(), kSkippedMessagesWithVulkanSecondaryCommandBuffer,
            kSkippedMessagesWithVulkanSecondaryCommandBuffer +
                ArraySize(kSkippedMessagesWithVulkanSecondaryCommandBuffer));
    }

    if (!getFeatures().preferDynamicRendering.enabled &&
        !vk::RenderPassCommandBuffer::ExecutesInline())
    {
        mSkippedValidationMessages.insert(
            mSkippedValidationMessages.end(), kSkippedMessagesWithRenderPassObjectsAndVulkanSCB,
            kSkippedMessagesWithRenderPassObjectsAndVulkanSCB +
                ArraySize(kSkippedMessagesWithRenderPassObjectsAndVulkanSCB));
    }

    if (getFeatures().preferDynamicRendering.enabled)
    {
        mSkippedValidationMessages.insert(
            mSkippedValidationMessages.end(), kSkippedMessagesWithDynamicRendering,
            kSkippedMessagesWithDynamicRendering + ArraySize(kSkippedMessagesWithDynamicRendering));
    }

    // Build the list of syncval errors that are currently expected and should be skipped.
    mSkippedSyncvalMessages.insert(mSkippedSyncvalMessages.end(), kSkippedSyncvalMessages,
                                   kSkippedSyncvalMessages + ArraySize(kSkippedSyncvalMessages));
    if (!getFeatures().supportsRenderPassStoreOpNone.enabled &&
        !getFeatures().supportsRenderPassLoadStoreOpNone.enabled)
    {
        mSkippedSyncvalMessages.insert(mSkippedSyncvalMessages.end(),
                                       kSkippedSyncvalMessagesWithoutStoreOpNone,
                                       kSkippedSyncvalMessagesWithoutStoreOpNone +
                                           ArraySize(kSkippedSyncvalMessagesWithoutStoreOpNone));
    }
    if (!getFeatures().supportsRenderPassLoadStoreOpNone.enabled)
    {
        mSkippedSyncvalMessages.insert(
            mSkippedSyncvalMessages.end(), kSkippedSyncvalMessagesWithoutLoadStoreOpNone,
            kSkippedSyncvalMessagesWithoutLoadStoreOpNone +
                ArraySize(kSkippedSyncvalMessagesWithoutLoadStoreOpNone));
    }
    if (getFeatures().enableMultisampledRenderToTexture.enabled &&
        !getFeatures().supportsMultisampledRenderToSingleSampled.enabled)
    {
        mSkippedSyncvalMessages.insert(mSkippedSyncvalMessages.end(),
                                       kSkippedSyncvalMessagesWithMSRTTEmulation,
                                       kSkippedSyncvalMessagesWithMSRTTEmulation +
                                           ArraySize(kSkippedSyncvalMessagesWithMSRTTEmulation));
    }
}

angle::Result Renderer::checkQueueForSurfacePresent(vk::ErrorContext *context,
                                                    VkSurfaceKHR surface,
                                                    bool *supportedOut)
{
    // We've already initialized a device, and can't re-create it unless it's never been used.
    // If recreation is ever necessary, it should be able to deal with contexts currently running in
    // other threads using the existing queue.  For example, multiple contexts (not in a share
    // group) may be currently recording commands and rendering to pbuffers or using
    // EGL_KHR_surfaceless_context.
    ASSERT(mDevice != VK_NULL_HANDLE);
    ASSERT(mCurrentQueueFamilyIndex != std::numeric_limits<uint32_t>::max());

    // Check if the current device supports present on this surface.
    VkBool32 supportsPresent = VK_FALSE;
    ANGLE_VK_TRY(context,
                 vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, mCurrentQueueFamilyIndex,
                                                      surface, &supportsPresent));

    *supportedOut = supportsPresent == VK_TRUE;
    return angle::Result::Continue;
}

std::string Renderer::getVendorString() const
{
    return GetVendorString(mPhysicalDeviceProperties.vendorID);
}

std::string Renderer::getRendererDescription() const
{
    std::stringstream strstr;

    uint32_t apiVersion = mPhysicalDeviceProperties.apiVersion;

    strstr << "Vulkan ";
    strstr << VK_VERSION_MAJOR(apiVersion) << ".";
    strstr << VK_VERSION_MINOR(apiVersion) << ".";
    strstr << VK_VERSION_PATCH(apiVersion);

    strstr << " (";

    // In the case of NVIDIA, deviceName does not necessarily contain "NVIDIA". Add "NVIDIA" so that
    // Vulkan end2end tests can be selectively disabled on NVIDIA. TODO(jmadill): should not be
    // needed after http://anglebug.com/40096421 is fixed and end2end_tests use more sophisticated
    // driver detection.
    if (mPhysicalDeviceProperties.vendorID == VENDOR_ID_NVIDIA)
    {
        strstr << GetVendorString(mPhysicalDeviceProperties.vendorID) << " ";
    }

    strstr << mPhysicalDeviceProperties.deviceName;
    strstr << " (" << gl::FmtHex(mPhysicalDeviceProperties.deviceID) << ")";

    strstr << ")";

    return strstr.str();
}

std::string Renderer::getVersionString(bool includeFullVersion) const
{
    std::stringstream strstr;

    uint32_t driverVersion = mPhysicalDeviceProperties.driverVersion;
    std::string driverName = std::string(mDriverProperties.driverName);

    if (!driverName.empty())
    {
        strstr << driverName;
    }
    else
    {
        strstr << GetVendorString(mPhysicalDeviceProperties.vendorID);
    }

    if (includeFullVersion)
    {
        strstr << "-";

        if (mPhysicalDeviceProperties.vendorID == VENDOR_ID_NVIDIA)
        {
            strstr << ANGLE_VK_VERSION_MAJOR_NVIDIA(driverVersion) << ".";
            strstr << ANGLE_VK_VERSION_MINOR_NVIDIA(driverVersion) << ".";
            strstr << ANGLE_VK_VERSION_SUB_MINOR_NVIDIA(driverVersion) << ".";
            strstr << ANGLE_VK_VERSION_PATCH_NVIDIA(driverVersion);
        }
        else if (mPhysicalDeviceProperties.vendorID == VENDOR_ID_INTEL && IsWindows())
        {
            strstr << ANGLE_VK_VERSION_MAJOR_WIN_INTEL(driverVersion) << ".";
            strstr << ANGLE_VK_VERSION_MINOR_WIN_INTEL(driverVersion);
        }
        // All other drivers use the Vulkan standard
        else
        {
            strstr << VK_VERSION_MAJOR(driverVersion) << ".";
            strstr << VK_VERSION_MINOR(driverVersion) << ".";
            strstr << VK_VERSION_PATCH(driverVersion);
        }
    }

    return strstr.str();
}

gl::Version Renderer::getMaxSupportedESVersion() const
{
    // Current highest supported version
    gl::Version maxVersion = gl::Version(3, 2);

    // Early out without downgrading ES version if mock ICD enabled.
    // Mock ICD doesn't expose sufficient capabilities yet.
    // https://github.com/KhronosGroup/Vulkan-Tools/issues/84
    if (isMockICDEnabled())
    {
        return maxVersion;
    }

    ensureCapsInitialized();

    // Limit to ES3.1 if there are any blockers for 3.2.
    if (mFeatures.exposeES32ForTesting.enabled)
    {
        return maxVersion;
    }
    if (!CanSupportGLES32(mNativeExtensions))
    {
        maxVersion = LimitVersionTo(maxVersion, {3, 1});
    }

    // Limit to ES3.0 if there are any blockers for 3.1.

    // ES3.1 requires at least one atomic counter buffer and four storage buffers in compute.
    // Atomic counter buffers are emulated with storage buffers.  For simplicity, we always support
    // either none or IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFERS atomic counter buffers.  So if
    // Vulkan doesn't support at least that many storage buffers in compute, we don't support 3.1.
    const uint32_t kMinimumStorageBuffersForES31 =
        gl::limits::kMinimumComputeStorageBuffers +
        gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS;
    if (mPhysicalDeviceProperties.limits.maxPerStageDescriptorStorageBuffers <
        kMinimumStorageBuffersForES31)
    {
        maxVersion = LimitVersionTo(maxVersion, {3, 0});
    }

    // ES3.1 requires at least a maximum offset of at least 2047.
    // If the Vulkan implementation can't support that, we cannot support 3.1.
    if (mPhysicalDeviceProperties.limits.maxVertexInputAttributeOffset < 2047)
    {
        maxVersion = LimitVersionTo(maxVersion, {3, 0});
    }

    // SSO is in ES3.1 core, so we have to cap to ES3.0 for SSO disablement.
    if (mFeatures.disableSeparateShaderObjects.enabled)
    {
        maxVersion = LimitVersionTo(maxVersion, {3, 0});
    }

    // Limit to ES2.0 if there are any blockers for 3.0.
    // TODO: http://anglebug.com/42262611 Limit to GLES 2.0 if flat shading can't be emulated

    // Multisample textures (ES3.1) and multisample renderbuffers (ES3.0) require the Vulkan driver
    // to support the standard sample locations (in order to pass dEQP tests that check these
    // locations).  If the Vulkan implementation can't support that, we cannot support 3.0/3.1.
    if (mPhysicalDeviceProperties.limits.standardSampleLocations != VK_TRUE)
    {
        maxVersion = LimitVersionTo(maxVersion, {2, 0});
    }

    // If independentBlend is not supported, we can't have a mix of has-alpha and emulated-alpha
    // render targets in a framebuffer.  We also cannot perform masked clears of multiple render
    // targets.
    if (!mPhysicalDeviceFeatures.independentBlend)
    {
        maxVersion = LimitVersionTo(maxVersion, {2, 0});
    }

    // If the Vulkan transform feedback extension is not present, we use an emulation path that
    // requires the vertexPipelineStoresAndAtomics feature. Without the extension or this feature,
    // we can't currently support transform feedback.
    if (!vk::CanSupportTransformFeedbackExtension(mTransformFeedbackFeatures) &&
        !vk::CanSupportTransformFeedbackEmulation(mPhysicalDeviceFeatures))
    {
        maxVersion = LimitVersionTo(maxVersion, {2, 0});
    }

    // Limit to GLES 2.0 if maxPerStageDescriptorUniformBuffers is too low.
    // Table 6.31 MAX_VERTEX_UNIFORM_BLOCKS minimum value = 12
    // Table 6.32 MAX_FRAGMENT_UNIFORM_BLOCKS minimum value = 12
    // NOTE: We reserve some uniform buffers for emulation, so use the NativeCaps which takes this
    // into account, rather than the physical device maxPerStageDescriptorUniformBuffers limits.
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        if (static_cast<GLuint>(getNativeCaps().maxShaderUniformBlocks[shaderType]) <
            gl::limits::kMinimumShaderUniformBlocks)
        {
            maxVersion = LimitVersionTo(maxVersion, {2, 0});
        }
    }

    // Limit to GLES 2.0 if maxVertexOutputComponents is too low.
    // Table 6.31 MAX VERTEX OUTPUT COMPONENTS minimum value = 64
    // NOTE: We reserve some vertex output components for emulation, so use the NativeCaps which
    // takes this into account, rather than the physical device maxVertexOutputComponents limits.
    if (static_cast<GLuint>(getNativeCaps().maxVertexOutputComponents) <
        gl::limits::kMinimumVertexOutputComponents)
    {
        maxVersion = LimitVersionTo(maxVersion, {2, 0});
    }

    return maxVersion;
}

gl::Version Renderer::getMaxConformantESVersion() const
{
    return getMaxSupportedESVersion();
}

uint32_t Renderer::getDeviceVersion() const
{
    return mDeviceVersion == 0 ? mInstanceVersion : mDeviceVersion;
}

void Renderer::queryAndCacheFragmentShadingRates()
{
    // Init required functions
#if !defined(ANGLE_SHARED_LIBVULKAN)
    InitFragmentShadingRateKHRInstanceFunction(mInstance);
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)
    ASSERT(vkGetPhysicalDeviceFragmentShadingRatesKHR);

    // Query number of supported shading rates first
    uint32_t shadingRatesCount = 0;
    VkResult result =
        vkGetPhysicalDeviceFragmentShadingRatesKHR(mPhysicalDevice, &shadingRatesCount, nullptr);
    ASSERT(result == VK_SUCCESS);
    ASSERT(shadingRatesCount > 0);

    std::vector<VkPhysicalDeviceFragmentShadingRateKHR> shadingRates(
        shadingRatesCount,
        {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR, nullptr, 0, {0, 0}});

    // Query supported shading rates
    result = vkGetPhysicalDeviceFragmentShadingRatesKHR(mPhysicalDevice, &shadingRatesCount,
                                                        shadingRates.data());
    ASSERT(result == VK_SUCCESS);

    // Cache supported fragment shading rates
    mSupportedFragmentShadingRates.reset();
    mSupportedFragmentShadingRateSampleCounts.fill(0u);
    for (const VkPhysicalDeviceFragmentShadingRateKHR &shadingRate : shadingRates)
    {
        if (shadingRate.sampleCounts == 0)
        {
            continue;
        }
        const gl::ShadingRate rate = GetShadingRateFromVkExtent(shadingRate.fragmentSize);
        mSupportedFragmentShadingRates.set(rate);
        mSupportedFragmentShadingRateSampleCounts[rate] = shadingRate.sampleCounts;
    }
}

bool Renderer::canSupportFragmentShadingRate() const
{
    // VK_KHR_create_renderpass2 is required for VK_KHR_fragment_shading_rate
    if (!mFeatures.supportsRenderpass2.enabled)
    {
        return false;
    }

    // Device needs to support VK_KHR_fragment_shading_rate and specifically
    // pipeline fragment shading rate.
    if (mFragmentShadingRateFeatures.pipelineFragmentShadingRate != VK_TRUE)
    {
        return false;
    }

    ASSERT(mSupportedFragmentShadingRates.any());

    // To implement GL_QCOM_shading_rate extension the Vulkan ICD needs to support at least the
    // following shading rates -
    //     {1, 1}
    //     {1, 2}
    //     {2, 1}
    //     {2, 2}
    return mSupportedFragmentShadingRates.test(gl::ShadingRate::_1x1) &&
           mSupportedFragmentShadingRates.test(gl::ShadingRate::_1x2) &&
           mSupportedFragmentShadingRates.test(gl::ShadingRate::_2x1) &&
           mSupportedFragmentShadingRates.test(gl::ShadingRate::_2x2);
}

bool Renderer::canSupportFoveatedRendering() const
{
    // Device needs to support attachment fragment shading rate.
    if (mFragmentShadingRateFeatures.attachmentFragmentShadingRate != VK_TRUE)
    {
        return false;
    }

    ASSERT(mSupportedFragmentShadingRates.any());
    ASSERT(!mSupportedFragmentShadingRateSampleCounts.empty());

    // To implement QCOM foveated rendering extensions the Vulkan ICD needs to support all sample
    // count bits listed in VkPhysicalDeviceLimits::framebufferColorSampleCounts for these shading
    // rates -
    //     {1, 1}
    //     {1, 2}
    //     {2, 1}
    //     {2, 2}
    VkSampleCountFlags framebufferSampleCounts =
        getPhysicalDeviceProperties().limits.framebufferColorSampleCounts &
        vk_gl::kSupportedSampleCounts;

    return (mSupportedFragmentShadingRateSampleCounts[gl::ShadingRate::_1x1] &
            framebufferSampleCounts) == framebufferSampleCounts &&
           (mSupportedFragmentShadingRateSampleCounts[gl::ShadingRate::_1x2] &
            framebufferSampleCounts) == framebufferSampleCounts &&
           (mSupportedFragmentShadingRateSampleCounts[gl::ShadingRate::_2x1] &
            framebufferSampleCounts) == framebufferSampleCounts &&
           (mSupportedFragmentShadingRateSampleCounts[gl::ShadingRate::_2x2] &
            framebufferSampleCounts) == framebufferSampleCounts;
}

bool Renderer::canPreferDeviceLocalMemoryHostVisible(VkPhysicalDeviceType deviceType)
{
    if (deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
    {
        const vk::MemoryProperties &memoryProperties = getMemoryProperties();
        static constexpr VkMemoryPropertyFlags kHostVisiableDeviceLocalFlags =
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        VkDeviceSize minHostVisiableDeviceLocalHeapSize = std::numeric_limits<VkDeviceSize>::max();
        VkDeviceSize maxDeviceLocalHeapSize             = 0;
        for (uint32_t i = 0; i < memoryProperties.getMemoryTypeCount(); ++i)
        {
            if ((memoryProperties.getMemoryType(i).propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
            {
                maxDeviceLocalHeapSize =
                    std::max(maxDeviceLocalHeapSize, memoryProperties.getHeapSizeForMemoryType(i));
            }
            if ((memoryProperties.getMemoryType(i).propertyFlags & kHostVisiableDeviceLocalFlags) ==
                kHostVisiableDeviceLocalFlags)
            {
                minHostVisiableDeviceLocalHeapSize =
                    std::min(minHostVisiableDeviceLocalHeapSize,
                             memoryProperties.getHeapSizeForMemoryType(i));
            }
        }
        return minHostVisiableDeviceLocalHeapSize != std::numeric_limits<VkDeviceSize>::max() &&
               minHostVisiableDeviceLocalHeapSize >=
                   static_cast<VkDeviceSize>(maxDeviceLocalHeapSize * 0.8);
    }
    return deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

void Renderer::initFeatures(const vk::ExtensionNameList &deviceExtensionNames,
                            const angle::FeatureOverrides &featureOverrides,
                            UseVulkanSwapchain useVulkanSwapchain,
                            angle::NativeWindowSystem nativeWindowSystem)
{
    ApplyFeatureOverrides(&mFeatures, featureOverrides);

    if (featureOverrides.allDisabled)
    {
        return;
    }

    const bool isAMD      = IsAMD(mPhysicalDeviceProperties.vendorID);
    const bool isApple    = IsAppleGPU(mPhysicalDeviceProperties.vendorID);
    const bool isARM      = IsARM(mPhysicalDeviceProperties.vendorID);
    const bool isIntel    = IsIntel(mPhysicalDeviceProperties.vendorID);
    const bool isNvidia   = IsNvidia(mPhysicalDeviceProperties.vendorID);
    const bool isPowerVR  = IsPowerVR(mPhysicalDeviceProperties.vendorID);
    const bool isQualcomm = IsQualcomm(mPhysicalDeviceProperties.vendorID);
    const bool isBroadcom = IsBroadcom(mPhysicalDeviceProperties.vendorID);
    const bool isSamsung  = IsSamsung(mPhysicalDeviceProperties.vendorID);
    const bool isSwiftShader =
        IsSwiftshader(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID);

    const bool isGalaxyS23 =
        IsGalaxyS23(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID);

    // Distinguish between the open source and proprietary Qualcomm drivers
    const bool isQualcommOpenSource =
        IsQualcommOpenSource(mPhysicalDeviceProperties.vendorID, mDriverProperties.driverID,
                             mPhysicalDeviceProperties.deviceName);
    const bool isQualcommProprietary = isQualcomm && !isQualcommOpenSource;

    // Lacking other explicit ways to tell if mali GPU is job manager based or command stream front
    // end based, we use maxDrawIndirectCount as equivalent since all JM based has
    // maxDrawIndirectCount==1 and all CSF based has maxDrawIndirectCount>1.
    bool isMaliJobManagerBasedGPU =
        isARM && getPhysicalDeviceProperties().limits.maxDrawIndirectCount <= 1;

    // Distinguish between the mesa and proprietary drivers
    const bool isRADV = IsRADV(mPhysicalDeviceProperties.vendorID, mDriverProperties.driverID,
                               mPhysicalDeviceProperties.deviceName);

    angle::VersionInfo driverVersion = {};
    if (isARM)
    {
        driverVersion = angle::ParseArmVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (isQualcommProprietary)
    {
        driverVersion =
            angle::ParseQualcommVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (isNvidia)
    {
        driverVersion =
            angle::ParseNvidiaVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (IsLinux() && (isIntel || isRADV))
    {
        driverVersion =
            angle::ParseMesaVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (IsWindows() && isIntel)
    {
        driverVersion =
            angle::ParseIntelWindowsVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (isAMD && !isRADV)
    {
        driverVersion = angle::ParseAMDVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }
    else if (isSamsung)
    {
        driverVersion =
            angle::ParseSamsungVulkanDriverVersion(mPhysicalDeviceProperties.driverVersion);
    }

    // Classify devices based on general architecture:
    //
    // - IMR (Immediate-Mode Rendering) devices generally progress through draw calls once and use
    //   the main GPU memory (accessed through caches) to store intermediate rendering results.
    // - TBR (Tile-Based Rendering) devices issue a pre-rendering geometry pass, then run through
    //   draw calls once per tile and store intermediate rendering results on the tile cache.
    //
    // Due to these key architectural differences, some operations improve performance on one while
    // deteriorating performance on the other.  ANGLE will accordingly make some decisions based on
    // the device architecture for optimal performance on both.
    const bool isImmediateModeRenderer = isNvidia || isAMD || isIntel || isSamsung || isSwiftShader;
    const bool isTileBasedRenderer     = isARM || isPowerVR || isQualcomm || isBroadcom || isApple;

    // Make sure all known architectures are accounted for.
    if (!isImmediateModeRenderer && !isTileBasedRenderer && !isMockICDEnabled())
    {
        WARN() << "Unknown GPU architecture";
    }

    ANGLE_FEATURE_CONDITION(&mFeatures, appendAliasedMemoryDecorations, true);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSharedPresentableImageExtension,
        ExtensionFound(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsGetMemoryRequirements2, true);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsBindMemory2, true);

    ANGLE_FEATURE_CONDITION(&mFeatures, bresenhamLineRasterization,
                            mLineRasterizationFeatures.bresenhamLines == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, provokingVertex,
                            mProvokingVertexFeatures.provokingVertexLast == VK_TRUE);

    // http://b/208458772. ARM driver supports this protected memory extension but we are seeing
    // excessive load/store unit activity when this extension is enabled, even if not been used.
    // Disable this extension on older ARM platforms that don't support
    // VK_EXT_pipeline_protected_access.
    // http://anglebug.com/42266183
    //
    // http://b/381285096. On Intel platforms, we want to prevent protected queues being used as
    // we cannot handle the teardown scenario if PXP termination occurs.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsProtectedMemory,
        mProtectedMemoryFeatures.protectedMemory == VK_TRUE &&
            (!isARM || mPipelineProtectedAccessFeatures.pipelineProtectedAccess == VK_TRUE) &&
            !isIntel);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsHostQueryReset,
                            mHostQueryResetFeatures.hostQueryReset == VK_TRUE);
    // Avoid any inefficiency that may be caused by host image copy by default.  To be experimented
    // with to see on which hardware VkHostImageCopyDevicePerformanceQueryEXT::optimalDeviceAccess
    // is really performing as well as
    // VkHostImageCopyDevicePerformanceQueryEXT::identicalMemoryLayout.
    ANGLE_FEATURE_CONDITION(&mFeatures, allowHostImageCopyDespiteNonIdenticalLayout, false);

    // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL and
    // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL are introduced by
    // VK_KHR_maintenance2 and promoted to Vulkan 1.1.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsMixedReadWriteDepthStencilLayouts, true);

    // VK_EXT_pipeline_creation_feedback is promoted to core in Vulkan 1.3.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsPipelineCreationFeedback,
        ExtensionFound(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME, deviceExtensionNames));

    // Note: Protected Swapchains is not determined until we have a VkSurface to query.
    // So here vendors should indicate support so that protected_content extension
    // is enabled.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSurfaceProtectedSwapchains, IsAndroid());

    // Work around incorrect NVIDIA point size range clamping.
    // http://anglebug.com/40644663#comment11
    // Clamp if driver version is:
    //   < 430 on Windows
    //   < 421 otherwise
    ANGLE_FEATURE_CONDITION(
        &mFeatures, clampPointSize,
        isNvidia && driverVersion < angle::VersionTriple(IsWindows() ? 430 : 421, 0, 0));

    // Affecting Nvidia drivers 535 through 551.
    ANGLE_FEATURE_CONDITION(&mFeatures, avoidOpSelectWithMismatchingRelaxedPrecision,
                            isNvidia && (driverVersion >= angle::VersionTriple(535, 0, 0) &&
                                         driverVersion < angle::VersionTriple(552, 0, 0)));

    // Affecting Linux/Intel (unknown range).
    ANGLE_FEATURE_CONDITION(&mFeatures, wrapSwitchInIfTrue, isIntel && IsLinux());

    // Vulkan implementations are not required to clamp gl_FragDepth to [0, 1] by default.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDepthClampZeroOne,
                            mDepthClampZeroOneFeatures.depthClampZeroOne == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, clampFragDepth,
                            isNvidia && !mFeatures.supportsDepthClampZeroOne.enabled);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsRenderpass2,
        ExtensionFound(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsIncrementalPresent,
        ExtensionFound(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME, deviceExtensionNames));

#if defined(ANGLE_PLATFORM_ANDROID)
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsAndroidHardwareBuffer,
        IsAndroid() &&
            ExtensionFound(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
                           deviceExtensionNames) &&
            ExtensionFound(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, deviceExtensionNames));
#endif

#if defined(ANGLE_PLATFORM_GGP)
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsGGPFrameToken,
        ExtensionFound(VK_GGP_FRAME_TOKEN_EXTENSION_NAME, deviceExtensionNames));
#endif

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalMemoryFd,
        ExtensionFound(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, deviceExtensionNames));

#if defined(ANGLE_PLATFORM_WINDOWS)
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsFullScreenExclusive,
        ExtensionFound(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME, deviceExtensionNames));

    // On Windows+AMD, drivers before version 0x800106 (2.0.262) would
    // implicitly enable VK_EXT_full_screen_exclusive and start returning
    // extension-specific error codes in swapchain functions. Since the
    // extension was not enabled by ANGLE, it was impossible to handle these
    // error codes correctly. On these earlier drivers, we want to explicitly
    // enable the extension and opt out of it to avoid seeing those error codes
    // entirely.
    ANGLE_FEATURE_CONDITION(&mFeatures, forceDisableFullScreenExclusive,
                            isAMD && driverVersion < angle::VersionTriple(2, 0, 262));
#endif

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalMemoryFuchsia,
        ExtensionFound(VK_FUCHSIA_EXTERNAL_MEMORY_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalSemaphoreFd,
        ExtensionFound(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalSemaphoreFuchsia,
        ExtensionFound(VK_FUCHSIA_EXTERNAL_SEMAPHORE_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalFenceFd,
        ExtensionFound(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME, deviceExtensionNames));

#if defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_LINUX)
    if (mFeatures.supportsExternalFenceCapabilities.enabled &&
        mFeatures.supportsExternalSemaphoreCapabilities.enabled)
    {
        VkExternalFenceProperties externalFenceProperties = {};
        externalFenceProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES;

        VkPhysicalDeviceExternalFenceInfo externalFenceInfo = {};
        externalFenceInfo.sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO;
        externalFenceInfo.handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT_KHR;

        vkGetPhysicalDeviceExternalFenceProperties(mPhysicalDevice, &externalFenceInfo,
                                                   &externalFenceProperties);

        VkExternalSemaphoreProperties externalSemaphoreProperties = {};
        externalSemaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;

        VkPhysicalDeviceExternalSemaphoreInfo externalSemaphoreInfo = {};
        externalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
        externalSemaphoreInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT_KHR;

        vkGetPhysicalDeviceExternalSemaphoreProperties(mPhysicalDevice, &externalSemaphoreInfo,
                                                       &externalSemaphoreProperties);

        ANGLE_FEATURE_CONDITION(
            &mFeatures, supportsAndroidNativeFenceSync,
            (mFeatures.supportsExternalFenceFd.enabled &&
             FencePropertiesCompatibleWithAndroid(externalFenceProperties) &&
             mFeatures.supportsExternalSemaphoreFd.enabled &&
             SemaphorePropertiesCompatibleWithAndroid(externalSemaphoreProperties)));
    }
    else
    {
        ANGLE_FEATURE_CONDITION(&mFeatures, supportsAndroidNativeFenceSync,
                                (mFeatures.supportsExternalFenceFd.enabled &&
                                 mFeatures.supportsExternalSemaphoreFd.enabled));
    }
#endif  // defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_LINUX)

    // Disabled on SwiftShader due to http://crbug.com/40942995
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsShaderStencilExport,
        ExtensionFound(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME, deviceExtensionNames) &&
            !isSwiftShader);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsRenderPassLoadStoreOpNone,
        ExtensionFound(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(&mFeatures, disallowMixedDepthStencilLoadOpNoneAndLoad,
                            isARM && driverVersion < angle::VersionTriple(38, 1, 0));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsRenderPassStoreOpNone,
        !mFeatures.supportsRenderPassLoadStoreOpNone.enabled &&
            ExtensionFound(VK_QCOM_RENDER_PASS_STORE_OPS_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDepthClipControl,
                            mDepthClipControlFeatures.depthClipControl == VK_TRUE);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsPrimitiveTopologyListRestart,
        mPrimitiveTopologyListRestartFeatures.primitiveTopologyListRestart == VK_TRUE);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsBlendOperationAdvanced,
        ExtensionFound(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsFormatFeatureFlags2,
        ExtensionFound(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsTransformFeedbackExtension,
                            vk::CanSupportTransformFeedbackExtension(mTransformFeedbackFeatures));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsGeometryStreamsCapability,
                            mFeatures.supportsTransformFeedbackExtension.enabled &&
                                mTransformFeedbackFeatures.geometryStreams == VK_TRUE);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsPrimitivesGeneratedQuery,
        mFeatures.supportsTransformFeedbackExtension.enabled &&
            mPrimitivesGeneratedQueryFeatures.primitivesGeneratedQuery == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, emulateTransformFeedback,
                            !mFeatures.supportsTransformFeedbackExtension.enabled &&
                                vk::CanSupportTransformFeedbackEmulation(mPhysicalDeviceFeatures));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsIndexTypeUint8,
                            mIndexTypeUint8Features.indexTypeUint8 == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDepthStencilResolve,
                            mFeatures.supportsRenderpass2.enabled &&
                                mDepthStencilResolveProperties.supportedDepthResolveModes != 0);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDepthStencilIndependentResolveNone,
                            mFeatures.supportsDepthStencilResolve.enabled &&
                                mDepthStencilResolveProperties.independentResolveNone);
    // Disable optimizing depth/stencil resolve through glBlitFramebuffer for buggy drivers:
    //
    // - Nvidia: http://anglebug.com/42267095
    // - Pixel4: http://anglebug.com/42267096
    //
    ANGLE_FEATURE_CONDITION(&mFeatures, disableDepthStencilResolveThroughAttachment,
                            isNvidia || isQualcommProprietary);

    // MSRTSS is disabled if the driver does not support it for RGBA8 and RGBA8_SRGB.
    // This is used to filter out known drivers where support for sRGB formats are missing.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsMultisampledRenderToSingleSampled,
        mMultisampledRenderToSingleSampledFeatures.multisampledRenderToSingleSampled == VK_TRUE &&
            mFeatures.supportsRenderpass2.enabled &&
            mFeatures.supportsDepthStencilResolve.enabled && CanSupportMSRTSSForRGBA8(this));

    // Preferring the MSRTSS flag is for texture initialization. If the MSRTSS is not used at first,
    // it will be used (if available) when recreating the image if it is bound to an MSRTT
    // framebuffer.
    ANGLE_FEATURE_CONDITION(&mFeatures, preferMSRTSSFlagByDefault,
                            mFeatures.supportsMultisampledRenderToSingleSampled.enabled && isARM);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsImage2dViewOf3d,
                            mImage2dViewOf3dFeatures.image2DViewOf3D == VK_TRUE);

    // Note: sampler2DViewOf3D is only useful for supporting EGL_KHR_gl_texture_3D_image.  If the
    // VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT added to 3D images measurable hurts sampling
    // performance, it might be better to remove support for EGL_KHR_gl_texture_3D_image in favor of
    // faster 3D images.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSampler2dViewOf3d,
                            mFeatures.supportsImage2dViewOf3d.enabled &&
                                mImage2dViewOf3dFeatures.sampler2DViewOf3D == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsMultiview, mMultiviewFeatures.multiview == VK_TRUE);

    // TODO: http://anglebug.com/42264464 - drop dependency on customBorderColorWithoutFormat.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsCustomBorderColor,
        mCustomBorderColorFeatures.customBorderColors == VK_TRUE &&
            mCustomBorderColorFeatures.customBorderColorWithoutFormat == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsMultiDrawIndirect,
                            mPhysicalDeviceFeatures.multiDrawIndirect == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, perFrameWindowSizeQuery,
                            IsAndroid() || isIntel || (IsWindows() && (isAMD || isNvidia)) ||
                                IsFuchsia() || isSamsung ||
                                nativeWindowSystem == angle::NativeWindowSystem::Wayland);

    ANGLE_FEATURE_CONDITION(&mFeatures, padBuffersToMaxVertexAttribStride, isAMD || isSamsung);
    mMaxVertexAttribStride = std::min(static_cast<uint32_t>(gl::limits::kMaxVertexAttribStride),
                                      mPhysicalDeviceProperties.limits.maxVertexInputBindingStride);

    ANGLE_FEATURE_CONDITION(&mFeatures, forceD16TexFilter, IsAndroid() && isQualcommProprietary);

    ANGLE_FEATURE_CONDITION(&mFeatures, disableFlippingBlitWithCommand,
                            IsAndroid() && isQualcommProprietary);

    // Allocation sanitization disabled by default because of a heaveyweight implementation
    // that can cause OOM and timeouts.
    ANGLE_FEATURE_CONDITION(&mFeatures, allocateNonZeroMemory, false);

    // ARM does buffer copy on geometry pipeline, which may create a GPU pipeline bubble that
    // prevents vertex shader to overlap with fragment shader on job manager based architecture. For
    // now we always choose CPU to do copy on ARM job manager based GPU.
    ANGLE_FEATURE_CONDITION(&mFeatures, preferCPUForBufferSubData, isARM);

    // On android, we usually are GPU limited, we try to use CPU to do data copy when other
    // conditions are the same. Set to zero will use GPU to do copy. This is subject to further
    // tuning for each platform https://issuetracker.google.com/201826021
    mMaxCopyBytesUsingCPUWhenPreservingBufferData =
        IsAndroid() ? std::numeric_limits<uint32_t>::max() : 0;

    ANGLE_FEATURE_CONDITION(&mFeatures, persistentlyMappedBuffers, true);

    ANGLE_FEATURE_CONDITION(&mFeatures, logMemoryReportCallbacks, false);
    ANGLE_FEATURE_CONDITION(&mFeatures, logMemoryReportStats, false);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalMemoryDmaBufAndModifiers,
        ExtensionFound(VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME, deviceExtensionNames) &&
            ExtensionFound(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsExternalMemoryHost,
        ExtensionFound(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME, deviceExtensionNames));

    // Android pre-rotation support can be disabled.
    ANGLE_FEATURE_CONDITION(&mFeatures, enablePreRotateSurfaces, IsAndroid());

    // http://anglebug.com/42261756
    // Precision qualifiers are disabled for Pixel 2 before the driver included relaxed precision.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, enablePrecisionQualifiers,
        !(IsPixel2(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID) &&
          (driverVersion < angle::VersionTriple(512, 490, 0))) &&
            !IsPixel4(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID));

    // http://anglebug.com/42265957
    ANGLE_FEATURE_CONDITION(&mFeatures, varyingsRequireMatchingPrecisionInSpirv, isPowerVR);

    // IMR devices are less sensitive to the src/dst stage masks in barriers, and behave more
    // efficiently when all barriers are aggregated, rather than individually and precisely
    // specified.
    ANGLE_FEATURE_CONDITION(&mFeatures, preferAggregateBarrierCalls, isImmediateModeRenderer);

    // For IMR devices, it's more efficient to ignore invalidate of framebuffer attachments with
    // emulated formats that have extra channels.  For TBR devices, the invalidate will be followed
    // by a clear to retain valid values in said extra channels.
    ANGLE_FEATURE_CONDITION(&mFeatures, preferSkippingInvalidateForEmulatedFormats,
                            isImmediateModeRenderer);

    ANGLE_FEATURE_CONDITION(&mFeatures, asyncCommandBufferResetAndGarbageCleanup, true);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsYUVSamplerConversion,
                            mSamplerYcbcrConversionFeatures.samplerYcbcrConversion != VK_FALSE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderFloat16,
                            mShaderFloat16Int8Features.shaderFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderInt8,
                            mShaderFloat16Int8Features.shaderInt8 == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderFloat64,
                            mPhysicalDeviceFeatures.shaderFloat64 == VK_TRUE);

    // Prefer driver uniforms over specialization constants in the following:
    //
    // - Older Qualcomm drivers where specialization constants severely degrade the performance of
    //   pipeline creation.  http://issuetracker.google.com/173636783
    // - ARM hardware
    // - Imagination hardware
    // - Samsung hardware
    // - SwiftShader
    //
    ANGLE_FEATURE_CONDITION(
        &mFeatures, preferDriverUniformOverSpecConst,
        (isQualcommProprietary && driverVersion < angle::VersionTriple(512, 513, 0)) || isARM ||
            isPowerVR || isSamsung || isSwiftShader);

    ANGLE_FEATURE_CONDITION(&mFeatures, preferCachedNoncoherentForDynamicStreamBufferUsage,
                            IsMeteorLake(mPhysicalDeviceProperties.deviceID));

    // The compute shader used to generate mipmaps needs -
    // 1. subgroup quad operations in compute shader stage.
    // 2. subgroup operations that can use extended types.
    // 3. 256-wide workgroup.
    //
    // Furthermore, VK_IMAGE_USAGE_STORAGE_BIT is detrimental to performance on many platforms, on
    // which this path is not enabled.  Platforms that are known to have better performance with
    // this path are:
    //
    // - AMD
    // - Nvidia
    // - Samsung
    //
    // Additionally, this path is disabled on buggy drivers:
    //
    // - AMD/Windows: Unfortunately the trybots use ancient AMD cards and drivers.
    const bool supportsSubgroupQuadOpsInComputeShader =
        (mSubgroupProperties.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT) &&
        (mSubgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT);

    const uint32_t maxComputeWorkGroupInvocations =
        mPhysicalDeviceProperties.limits.maxComputeWorkGroupInvocations;

    ANGLE_FEATURE_CONDITION(&mFeatures, allowGenerateMipmapWithCompute,
                            supportsSubgroupQuadOpsInComputeShader &&
                                mSubgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes &&
                                maxComputeWorkGroupInvocations >= 256 &&
                                ((isAMD && !IsWindows()) || isNvidia || isSamsung));

    bool isAdreno540 = mPhysicalDeviceProperties.deviceID == angle::kDeviceID_Adreno540;
    ANGLE_FEATURE_CONDITION(&mFeatures, forceMaxUniformBufferSize16KB,
                            isQualcommProprietary && isAdreno540);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsImageFormatList,
        ExtensionFound(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, deviceExtensionNames));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSamplerMirrorClampToEdge,
        ExtensionFound(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, deviceExtensionNames));

    // Emulation of GL_EXT_multisampled_render_to_texture is only really useful on tiling hardware,
    // but is exposed on any configuration deployed on Android, such as Samsung's AMD-based GPU.
    //
    // During testing, it was also discovered that emulation triggers bugs on some platforms:
    //
    // - Swiftshader:
    //   * Failure on mac: http://anglebug.com/40644747
    //   * OOM: http://crbug.com/1263046
    // - Intel on windows: http://anglebug.com/42263602
    // - AMD on windows: http://crbug.com/1132366
    // - Old ARM drivers on Android fail multiple tests, though newer drivers don't (although they
    //   support MSRTSS and emulation is unnecessary)
    //
    ANGLE_FEATURE_CONDITION(&mFeatures, allowMultisampledRenderToTextureEmulation,
                            (isTileBasedRenderer && !isARM) || isSamsung);
    ANGLE_FEATURE_CONDITION(&mFeatures, enableMultisampledRenderToTexture,
                            mFeatures.supportsMultisampledRenderToSingleSampled.enabled ||
                                (mFeatures.supportsDepthStencilResolve.enabled &&
                                 mFeatures.allowMultisampledRenderToTextureEmulation.enabled));

    // Currently we enable cube map arrays based on the imageCubeArray Vk feature.
    // TODO: Check device caps for full cube map array support. http://anglebug.com/42263705
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsImageCubeArray,
                            mPhysicalDeviceFeatures.imageCubeArray == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsPipelineStatisticsQuery,
                            mPhysicalDeviceFeatures.pipelineStatisticsQuery == VK_TRUE);

    // Android mistakenly destroys the old swapchain when creating a new one.
    ANGLE_FEATURE_CONDITION(&mFeatures, waitIdleBeforeSwapchainRecreation, IsAndroid() && isARM);

    // vkCmdClearAttachments races with draw calls on Qualcomm hardware as observed on Pixel2 and
    // Pixel4.  https://issuetracker.google.com/issues/166809097
    ANGLE_FEATURE_CONDITION(
        &mFeatures, preferDrawClearOverVkCmdClearAttachments,
        isQualcommProprietary && driverVersion < angle::VersionTriple(512, 762, 12));

    // R32F imageAtomicExchange emulation is done if shaderImageFloat32Atomics feature is not
    // supported.
    ANGLE_FEATURE_CONDITION(&mFeatures, emulateR32fImageAtomicExchange,
                            mShaderAtomicFloatFeatures.shaderImageFloat32Atomics != VK_TRUE);

    // Whether non-conformant configurations and extensions should be exposed.
    ANGLE_FEATURE_CONDITION(&mFeatures, exposeNonConformantExtensionsAndVersions,
                            kExposeNonConformantExtensionsAndVersions);

    // http://issuetracker.google.com/376899587
    // Currently some testing platforms do not fully support ES 3.2 due to lack of certain features
    // or extensions. For the purpose of testing coverage, we would still enable ES 3.2 on these
    // platforms. However, once a listed test platform is updated to a version that does support
    // ES 3.2, it should be unlisted.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, exposeES32ForTesting,
        mFeatures.exposeNonConformantExtensionsAndVersions.enabled &&
            (isSwiftShader ||
             IsPixel4(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID) ||
             (IsLinux() && isNvidia && driverVersion < angle::VersionTriple(441, 0, 0)) ||
             (IsWindows() && isIntel)));

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsMemoryBudget,
        ExtensionFound(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, deviceExtensionNames));

    // Disabled by default. Only enable it for experimental purpose, as this will cause various
    // tests to fail.
    ANGLE_FEATURE_CONDITION(&mFeatures, forceFragmentShaderPrecisionHighpToMediump, false);

    // Testing shows that on ARM and Qualcomm GPU, doing implicit flush at framebuffer boundary
    // improves performance. Most app traces shows frame time reduced and manhattan 3.1 offscreen
    // score improves 7%.
    ANGLE_FEATURE_CONDITION(&mFeatures, preferSubmitAtFBOBoundary,
                            isTileBasedRenderer || isSwiftShader);

    // In order to support immutable samplers tied to external formats, we need to overallocate
    // descriptor counts for such immutable samplers
    ANGLE_FEATURE_CONDITION(&mFeatures, useMultipleDescriptorsForExternalFormats, true);

    // http://anglebug.com/42265147
    // When creating a surface with the format GL_RGB8, override the format to be GL_RGBA8, since
    // Android prevents creating swapchain images with VK_FORMAT_R8G8B8_UNORM.
    // Do this for all platforms, since few (none?) IHVs support 24-bit formats with their HW
    // natively anyway.
    ANGLE_FEATURE_CONDITION(&mFeatures, overrideSurfaceFormatRGB8ToRGBA8, true);

    // We set the following when there is color framebuffer fetch:
    //
    // - VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT
    // - VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT
    //
    // and the following with depth/stencil framebuffer fetch:
    //
    // - VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT
    // -
    // VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT
    //
    // But the check for framebuffer fetch is not accurate enough and those bits can have great
    // impact on Qualcomm (it only affects the open source driver because the proprietary driver
    // does not expose the extension).  Let's disable it on Qualcomm.
    //
    // https://issuetracker.google.com/issues/255837430
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsRasterizationOrderAttachmentAccess,
        !isQualcomm &&
            mRasterizationOrderAttachmentAccessFeatures.rasterizationOrderColorAttachmentAccess ==
                VK_TRUE);

    // The VK_EXT_surface_maintenance1 and VK_EXT_swapchain_maintenance1 extensions are used for a
    // variety of improvements:
    //
    // - Recycling present semaphores
    // - Avoiding swapchain recreation when present modes change
    // - Amortizing the cost of memory allocation for swapchain creation over multiple frames
    //
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSwapchainMaintenance1,
                            mSwapchainMaintenance1Features.swapchainMaintenance1 == VK_TRUE &&
                                useVulkanSwapchain == UseVulkanSwapchain::Yes);

    // The VK_EXT_legacy_dithering extension enables dithering support without emulation
    // Disable the usage of VK_EXT_legacy_dithering on ARM until the driver bug
    // http://issuetracker.google.com/293136916, http://issuetracker.google.com/292282210 are fixed.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsLegacyDithering,
                            mDitheringFeatures.legacyDithering == VK_TRUE);

    // Applications on Android have come to rely on hardware dithering, and visually regress without
    // it.  On desktop GPUs, OpenGL's dithering is a no-op.  The following setting mimics that
    // behavior.  Dithering is also currently not enabled on SwiftShader, but can be as needed
    // (which would require Chromium and Capture/Replay test expectations updates).
    ANGLE_FEATURE_CONDITION(&mFeatures, emulateDithering,
                            IsAndroid() && !mFeatures.supportsLegacyDithering.enabled);

    ANGLE_FEATURE_CONDITION(&mFeatures, adjustClearColorPrecision,
                            IsAndroid() && mFeatures.supportsLegacyDithering.enabled && isARM &&
                                driverVersion < angle::VersionTriple(50, 0, 0));

    // ANGLE always exposes framebuffer fetch because too many apps assume it's there.  See comments
    // on |mIsColorFramebufferFetchCoherent| for details.  Non-coherent framebuffer fetch is always
    // supported by Vulkan.
    //
    // Without exposeNonConformantExtensionsAndVersions, this feature is disable on Intel/windows
    // due to lack of input attachment support for swapchain images, and Intel/mesa before mesa
    // 22.0 for the same reason.  Without VK_GOOGLE_surfaceless_query, there is no way to
    // automatically deduce this support.
    //
    // http://issuetracker.google.com/376899587
    // Advanced blend emulation depends on this functionality, lack of which prevents support for
    // ES 3.2; exposeNonConformantExtensionsAndVersions is used to force this.
    const bool supportsFramebufferFetchInSurface =
        IsAndroid() || !isIntel ||
        (isIntel && IsLinux() && driverVersion >= angle::VersionTriple(22, 0, 0)) ||
        mFeatures.exposeNonConformantExtensionsAndVersions.enabled;

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderFramebufferFetch,
                            supportsFramebufferFetchInSurface);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderFramebufferFetchNonCoherent,
                            supportsFramebufferFetchInSurface);

    // On ARM hardware, framebuffer-fetch-like behavior on Vulkan is known to be coherent even
    // without the Vulkan extension.
    //
    // On IMG hardware, similarly framebuffer-fetch-like behavior on Vulkan is known to be coherent,
    // but the Vulkan extension cannot be exposed.  This is because the Vulkan extension guarantees
    // coherence when accessing all samples of a pixel from any other sample, but IMG hardware is
    // _not_ coherent in that case.  This is not a problem for GLES because the invocation for each
    // sample can only access values for the same sample by reading "the current color value",
    // unlike Vulkan-GLSL's |subpassLoad()| which takes a sample index.
    mIsColorFramebufferFetchCoherent =
        isARM || isPowerVR || mFeatures.supportsRasterizationOrderAttachmentAccess.enabled;

    // Support EGL_KHR_lock_surface3 extension.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsLockSurfaceExtension, IsAndroid());

    // http://anglebug.com/42265370
    // Android needs swapbuffers to update image and present to display.
    ANGLE_FEATURE_CONDITION(&mFeatures, swapbuffersOnFlushOrFinishWithSingleBuffer, IsAndroid());

    // Workaround a Qualcomm imprecision with dithering
    ANGLE_FEATURE_CONDITION(&mFeatures, roundOutputAfterDithering, isQualcomm);

    // GL_KHR_blend_equation_advanced is emulated when the equivalent Vulkan extension is not
    // usable.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, emulateAdvancedBlendEquations,
        !mFeatures.supportsBlendOperationAdvanced.enabled && supportsFramebufferFetchInSurface);

    // GL_KHR_blend_equation_advanced_coherent ensures that the blending operations are performed in
    // API primitive order.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsBlendOperationAdvancedCoherent,
        mFeatures.supportsBlendOperationAdvanced.enabled &&
            mBlendOperationAdvancedFeatures.advancedBlendCoherentOperations == VK_TRUE);

    // http://anglebug.com/42265410
    // Android expects VkPresentRegionsKHR rectangles with a bottom-left origin, while spec
    // states they should have a top-left origin.
    ANGLE_FEATURE_CONDITION(&mFeatures, bottomLeftOriginPresentRegionRectangles, IsAndroid());

    // Use VMA for image suballocation.
    ANGLE_FEATURE_CONDITION(&mFeatures, useVmaForImageSuballocation, true);

    // Emit SPIR-V 1.4 when supported.  The following old drivers have various bugs with SPIR-V 1.4:
    //
    // - Nvidia drivers - Crashes when creating pipelines, not using any SPIR-V 1.4 features.  Known
    //                    good since at least version 525.  http://anglebug.com/343249127
    // - Qualcomm drivers - Crashes when creating pipelines in the presence of OpCopyLogical with
    //                      some types.  http://anglebug.com/343218484
    // - ARM drivers - Fail tests when OpSelect uses a scalar to select between vectors.  Known good
    //                 since at least version 47.  http://anglebug.com/343218491
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSPIRV14,
                            ExtensionFound(VK_KHR_SPIRV_1_4_EXTENSION_NAME, deviceExtensionNames) &&
                                !(isNvidia && driverVersion < angle::VersionTriple(525, 0, 0)) &&
                                !isQualcommProprietary &&
                                !(isARM && driverVersion < angle::VersionTriple(47, 0, 0)));

    // Rounding features from VK_KHR_float_controls extension
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormFtzFp16,
                            mFloatControlProperties.shaderDenormFlushToZeroFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormFtzFp32,
                            mFloatControlProperties.shaderDenormFlushToZeroFloat32 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormFtzFp64,
                            mFloatControlProperties.shaderDenormFlushToZeroFloat64 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormPreserveFp16,
                            mFloatControlProperties.shaderDenormPreserveFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormPreserveFp32,
                            mFloatControlProperties.shaderDenormPreserveFloat32 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDenormPreserveFp64,
                            mFloatControlProperties.shaderDenormPreserveFloat64 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRteFp16,
                            mFloatControlProperties.shaderRoundingModeRTEFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRteFp32,
                            mFloatControlProperties.shaderRoundingModeRTEFloat32 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRteFp64,
                            mFloatControlProperties.shaderRoundingModeRTEFloat64 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRtzFp16,
                            mFloatControlProperties.shaderRoundingModeRTZFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRtzFp32,
                            mFloatControlProperties.shaderRoundingModeRTZFloat32 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsRoundingModeRtzFp64,
                            mFloatControlProperties.shaderRoundingModeRTZFloat64 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSignedZeroInfNanPreserveFp16,
        mFloatControlProperties.shaderSignedZeroInfNanPreserveFloat16 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSignedZeroInfNanPreserveFp32,
        mFloatControlProperties.shaderSignedZeroInfNanPreserveFloat32 == VK_TRUE);
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsSignedZeroInfNanPreserveFp64,
        mFloatControlProperties.shaderSignedZeroInfNanPreserveFloat64 == VK_TRUE);

    // Retain debug info in SPIR-V blob.
    ANGLE_FEATURE_CONDITION(&mFeatures, retainSPIRVDebugInfo, getEnableValidationLayers());

    // For discrete GPUs, most of device local memory is host invisible. We should not force the
    // host visible flag for them and result in allocation failure.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, preferDeviceLocalMemoryHostVisible,
        canPreferDeviceLocalMemoryHostVisible(mPhysicalDeviceProperties.deviceType));

    // Multiple dynamic state issues on ARM have been fixed.
    // http://issuetracker.google.com/285124778
    // http://issuetracker.google.com/285196249
    // http://issuetracker.google.com/286224923
    // http://issuetracker.google.com/287318431
    //
    // On Pixel devices, the issues have been fixed since r44, but on others since r44p1.
    //
    // Regressions have been detected using r46 on older architectures though
    // http://issuetracker.google.com/336411904
    const bool isExtendedDynamicStateBuggy =
        (isARM && driverVersion < angle::VersionTriple(44, 1, 0)) ||
        (isMaliJobManagerBasedGPU && driverVersion >= angle::VersionTriple(46, 0, 0));

    // Vertex input binding stride is buggy for Windows/Intel drivers before 100.9684.
    const bool isVertexInputBindingStrideBuggy =
        IsWindows() && isIntel && driverVersion < angle::VersionTriple(100, 9684, 0);

    // Intel driver has issues with VK_EXT_vertex_input_dynamic_state
    // http://anglebug.com/42265637#comment9
    //
    // On ARM drivers prior to r48, |vkCmdBindVertexBuffers2| applies strides to the wrong index,
    // according to the errata: https://developer.arm.com/documentation/SDEN-3735689/0100/?lang=en
    //
    // On Qualcomm drivers prior to 777, this feature had a bug.
    // http://anglebug.com/381384988
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsVertexInputDynamicState,
        mVertexInputDynamicStateFeatures.vertexInputDynamicState == VK_TRUE &&
            !(IsWindows() && isIntel) &&
            !(isARM && driverVersion < angle::VersionTriple(48, 0, 0)) &&
            !(isQualcommProprietary && driverVersion < angle::VersionTriple(512, 777, 0)));

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExtendedDynamicState,
                            mExtendedDynamicStateFeatures.extendedDynamicState == VK_TRUE &&
                                !isExtendedDynamicStateBuggy);

    // VK_EXT_vertex_input_dynamic_state enables dynamic state for the full vertex input state. As
    // such, when available use supportsVertexInputDynamicState instead of
    // useVertexInputBindingStrideDynamicState.
    ANGLE_FEATURE_CONDITION(&mFeatures, useVertexInputBindingStrideDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled &&
                                !mFeatures.supportsVertexInputDynamicState.enabled &&
                                !isExtendedDynamicStateBuggy && !isVertexInputBindingStrideBuggy);
    // On ARM drivers prior to r52, |vkCmdSetCullMode| incorrectly culls non-triangle topologies,
    // according to the errata: https://developer.arm.com/documentation/SDEN-3735689/0100/?lang=en
    ANGLE_FEATURE_CONDITION(&mFeatures, useCullModeDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled &&
                                !isExtendedDynamicStateBuggy &&
                                !(isARM && driverVersion < angle::VersionTriple(52, 0, 0)));
    ANGLE_FEATURE_CONDITION(&mFeatures, useDepthCompareOpDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled);
    ANGLE_FEATURE_CONDITION(&mFeatures, useDepthTestEnableDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled);
    ANGLE_FEATURE_CONDITION(
        &mFeatures, useDepthWriteEnableDynamicState,
        mFeatures.supportsExtendedDynamicState.enabled && !isExtendedDynamicStateBuggy);
    ANGLE_FEATURE_CONDITION(&mFeatures, useFrontFaceDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled);
    ANGLE_FEATURE_CONDITION(&mFeatures, useStencilOpDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled);
    ANGLE_FEATURE_CONDITION(&mFeatures, useStencilTestEnableDynamicState,
                            mFeatures.supportsExtendedDynamicState.enabled);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExtendedDynamicState2,
                            mExtendedDynamicState2Features.extendedDynamicState2 == VK_TRUE &&
                                !isExtendedDynamicStateBuggy);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, usePrimitiveRestartEnableDynamicState,
        mFeatures.supportsExtendedDynamicState2.enabled && !isExtendedDynamicStateBuggy);
    ANGLE_FEATURE_CONDITION(&mFeatures, useRasterizerDiscardEnableDynamicState,
                            mFeatures.supportsExtendedDynamicState2.enabled);
    ANGLE_FEATURE_CONDITION(&mFeatures, useDepthBiasEnableDynamicState,
                            mFeatures.supportsExtendedDynamicState2.enabled);

    // Disabled on Intel/Mesa due to driver bug (crbug.com/1379201).  This bug is fixed since Mesa
    // 22.2.0.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsLogicOpDynamicState,
        mFeatures.supportsExtendedDynamicState2.enabled &&
            mExtendedDynamicState2Features.extendedDynamicState2LogicOp == VK_TRUE &&
            !(IsLinux() && isIntel && driverVersion < angle::VersionTriple(22, 2, 0)) &&
            !(IsAndroid() && isGalaxyS23));

    // Older Samsung drivers with version < 24.0.0 have a bug in imageless framebuffer support.
    const bool isSamsungDriverWithImagelessFramebufferBug =
        isSamsung && driverVersion < angle::VersionTriple(24, 0, 0);
    // Qualcomm with imageless framebuffers, vkCreateFramebuffer loops forever.
    // http://issuetracker.google.com/369693310
    const bool isQualcommWithImagelessFramebufferBug =
        isQualcommProprietary && driverVersion < angle::VersionTriple(512, 802, 0);
    // PowerVR with imageless framebuffer spends enormous amounts of time in framebuffer destruction
    // and creation. ANGLE doesn't cache imageless framebuffers, instead adding them to garbage
    // collection, expecting them to be lightweight.
    // http://issuetracker.google.com/372273294
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsImagelessFramebuffer,
                            mImagelessFramebufferFeatures.imagelessFramebuffer == VK_TRUE &&
                                !isSamsungDriverWithImagelessFramebufferBug &&
                                !isQualcommWithImagelessFramebufferBug && !isPowerVR);

    if (ExtensionFound(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, deviceExtensionNames))
    {
        queryAndCacheFragmentShadingRates();
    }

    // Support GL_QCOM_shading_rate extension
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsFragmentShadingRate,
                            canSupportFragmentShadingRate());

    // Support QCOM foveated rendering extensions.
    // Gated on supportsImagelessFramebuffer and supportsRenderPassLoadStoreOpNone
    // to reduce code complexity.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsFoveatedRendering,
                            mFeatures.supportsImagelessFramebuffer.enabled &&
                                mFeatures.supportsRenderPassLoadStoreOpNone.enabled &&
                                mFeatures.supportsFragmentShadingRate.enabled &&
                                canSupportFoveatedRendering());

    // Force CPU based generation of fragment shading rate attachment data if
    // VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats is not supported
    ANGLE_FEATURE_CONDITION(&mFeatures, generateFragmentShadingRateAttchementWithCpu,
                            mPhysicalDeviceFeatures.shaderStorageImageExtendedFormats != VK_TRUE);

    // We can use the interlock to support GL_ANGLE_shader_pixel_local_storage_coherent.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsFragmentShaderPixelInterlock,
        mFragmentShaderInterlockFeatures.fragmentShaderPixelInterlock == VK_TRUE);

    // The VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT behavior is used by
    // ANGLE, which requires the robustBufferAccess feature to be available.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsPipelineRobustness,
                            mPipelineRobustnessFeatures.pipelineRobustness == VK_TRUE &&
                                mPhysicalDeviceFeatures.robustBufferAccess);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsPipelineProtectedAccess,
                            mPipelineProtectedAccessFeatures.pipelineProtectedAccess == VK_TRUE &&
                                mProtectedMemoryFeatures.protectedMemory == VK_TRUE);

    // VK_EXT_graphics_pipeline_library is available on NVIDIA drivers earlier
    // than version 531, but there are transient visual glitches with rendering
    // on those earlier versions.  http://anglebug.com/42266655
    //
    // On RADV, creating graphics pipeline can crash in the driver.  http://crbug.com/1497512
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsGraphicsPipelineLibrary,
                            mGraphicsPipelineLibraryFeatures.graphicsPipelineLibrary == VK_TRUE &&
                                (!isNvidia || driverVersion >= angle::VersionTriple(531, 0, 0)) &&
                                !isRADV);

    // The following drivers are known to key the pipeline cache blobs with vertex input and
    // fragment output state, causing draw-time pipeline creation to miss the cache regardless of
    // warm up:
    //
    // - ARM drivers
    // - Imagination drivers
    //
    // The following drivers are instead known to _not_ include said state, and hit the cache at
    // draw time.
    //
    // - SwiftShader
    // - Open source Qualcomm drivers
    //
    // The situation is unknown for other drivers.
    //
    // Additionally, numerous tests that previously never created a Vulkan pipeline fail or crash on
    // proprietary Qualcomm drivers when they do during cache warm up.  On Intel/Linux, one trace
    // shows flakiness with this.
    const bool libraryBlobsAreReusedByMonolithicPipelines = !isARM && !isPowerVR;
    ANGLE_FEATURE_CONDITION(&mFeatures, warmUpPipelineCacheAtLink,
                            libraryBlobsAreReusedByMonolithicPipelines && !isQualcommProprietary &&
                                !(IsLinux() && isIntel) && !(IsChromeOS() && isSwiftShader));

    // On SwiftShader, no data is retrieved from the pipeline cache, so there is no reason to
    // serialize it or put it in the blob cache.
    // For Windows Nvidia Vulkan driver older than 520, Vulkan pipeline cache will only generate one
    // single huge cache for one process shared by all graphics pipelines in the same process, which
    // can be huge.
    const bool nvVersionLessThan520 = isNvidia && driverVersion < angle::VersionTriple(520, 0, 0);
    ANGLE_FEATURE_CONDITION(&mFeatures, hasEffectivePipelineCacheSerialization,
                            !isSwiftShader && !nvVersionLessThan520);

    // Practically all drivers still prefer to do cross-stage linking.
    // graphicsPipelineLibraryFastLinking allows them to quickly produce working pipelines, but it
    // is typically not as efficient as complete pipelines.
    //
    // This optimization is disabled on the Intel/windows driver before 31.0.101.5379 due to driver
    // bugs.
    // This optimization is disabled for Samsung.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, preferMonolithicPipelinesOverLibraries,
        mFeatures.supportsGraphicsPipelineLibrary.enabled &&
            !(IsWindows() && isIntel && driverVersion < angle::VersionTriple(101, 5379, 0)) &&
            !isSamsung);

    // Whether the pipeline caches should merge into the global pipeline cache.  This should only be
    // enabled on platforms if:
    //
    // - VK_EXT_graphics_pipeline_library is not supported.  In that case, only the program's cache
    //   used during warm up is merged into the global cache for later monolithic pipeline creation.
    // - VK_EXT_graphics_pipeline_library is supported, monolithic pipelines are preferred, and the
    //   driver is able to reuse blobs from partial pipelines when creating monolithic pipelines.
    ANGLE_FEATURE_CONDITION(&mFeatures, mergeProgramPipelineCachesToGlobalCache,
                            !mFeatures.supportsGraphicsPipelineLibrary.enabled ||
                                (mFeatures.preferMonolithicPipelinesOverLibraries.enabled &&
                                 libraryBlobsAreReusedByMonolithicPipelines));

    ANGLE_FEATURE_CONDITION(&mFeatures, enableAsyncPipelineCacheCompression, true);

    // Sync monolithic pipelines to the blob cache occasionally on platforms that would benefit from
    // it:
    //
    // - VK_EXT_graphics_pipeline_library is not supported, and the program cache is not warmed up:
    //   If the pipeline cache is being warmed up at link time, the blobs corresponding to each
    //   program is individually retrieved and stored in the blob cache already.
    // - VK_EXT_graphics_pipeline_library is supported, but monolithic pipelines are still
    //   preferred, and the cost of syncing the large cache is acceptable.
    //
    // Otherwise monolithic pipelines are recreated on every run.
    const bool hasNoPipelineWarmUp = !mFeatures.supportsGraphicsPipelineLibrary.enabled &&
                                     !mFeatures.warmUpPipelineCacheAtLink.enabled;
    const bool canSyncLargeMonolithicCache =
        mFeatures.supportsGraphicsPipelineLibrary.enabled &&
        mFeatures.preferMonolithicPipelinesOverLibraries.enabled &&
        (!IsAndroid() || mFeatures.enableAsyncPipelineCacheCompression.enabled);
    ANGLE_FEATURE_CONDITION(&mFeatures, syncMonolithicPipelinesToBlobCache,
                            mFeatures.hasEffectivePipelineCacheSerialization.enabled &&
                                (hasNoPipelineWarmUp || canSyncLargeMonolithicCache));

    // Enable the feature on Samsung by default, because it has big blob cache.
    ANGLE_FEATURE_CONDITION(&mFeatures, useDualPipelineBlobCacheSlots, isSamsung);

    // Disable by default, because currently it is uncommon that blob cache supports storing
    // zero sized blobs (or erasing blobs).
    ANGLE_FEATURE_CONDITION(&mFeatures, useEmptyBlobsToEraseOldPipelineCacheFromBlobCache, false);

    // Assume that platform has blob cache that has LRU eviction.
    ANGLE_FEATURE_CONDITION(&mFeatures, hasBlobCacheThatEvictsOldItemsFirst, true);
    // Also assume that platform blob cache evicts only minimum number of items when it has LRU,
    // in which case verification is not required.
    ANGLE_FEATURE_CONDITION(&mFeatures, verifyPipelineCacheInBlobCache,
                            !mFeatures.hasBlobCacheThatEvictsOldItemsFirst.enabled);

    // On ARM, dynamic state for stencil write mask doesn't work correctly in the presence of
    // discard or alpha to coverage, if the static state provided when creating the pipeline has a
    // value of 0.
    ANGLE_FEATURE_CONDITION(&mFeatures, useNonZeroStencilWriteMaskStaticState,
                            isARM && driverVersion < angle::VersionTriple(43, 0, 0));

    // On some vendors per-sample shading is not enabled despite the presence of a Sample
    // decoration. Guard against this by parsing shader for "sample" decoration and explicitly
    // enabling per-sample shading pipeline state.
    ANGLE_FEATURE_CONDITION(&mFeatures, explicitlyEnablePerSampleShading, !isQualcommProprietary);

    ANGLE_FEATURE_CONDITION(&mFeatures, explicitlyCastMediumpFloatTo16Bit, isARM);

    // Force to create swapchain with continuous refresh on shared present. Disabled by default.
    // Only enable it on integrations without EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID passthrough.
    ANGLE_FEATURE_CONDITION(&mFeatures, forceContinuousRefreshOnSharedPresent, false);

    // Enable setting frame timestamp surface attribute on Android platform.
    // Frame timestamp is enabled by calling into "vkGetPastPresentationTimingGOOGLE"
    // which, on Android platforms, makes the necessary ANativeWindow API calls.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsTimestampSurfaceAttribute,
                            IsAndroid() && ExtensionFound(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
                                                          deviceExtensionNames));

    // Only enable VK_EXT_host_image_copy on hardware where identicalMemoryTypeRequirements is set.
    // That lets ANGLE avoid having to fallback to non-host-copyable image allocations if the
    // host-copyable one fails due to out-of-that-specific-kind-of-memory.
    //
    // Disabled on Fuchsia until they upgrade their version of VVL.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsHostImageCopy,
                            mHostImageCopyFeatures.hostImageCopy == VK_TRUE &&
                                mHostImageCopyProperties.identicalMemoryTypeRequirements &&
                                !IsFuchsia());

    // 1) host vk driver does not natively support ETC format.
    // 2) host vk driver supports BC format.
    // 3) host vk driver supports subgroup instructions: clustered, shuffle.
    //    * This limitation can be removed if necessary.
    // 4) host vk driver has maxTexelBufferSize >= 64M.
    //    * Usually on desktop device the limit is more than 128M. we may switch to dynamic
    //    decide cpu or gpu upload texture based on texture size.
    constexpr VkSubgroupFeatureFlags kRequiredSubgroupOp =
        VK_SUBGROUP_FEATURE_SHUFFLE_BIT | VK_SUBGROUP_FEATURE_CLUSTERED_BIT;
    static constexpr bool kSupportTranscodeEtcToBc = false;
    static constexpr uint32_t kMaxTexelBufferSize  = 64 * 1024 * 1024;
    const VkPhysicalDeviceLimits &limitsVk         = mPhysicalDeviceProperties.limits;
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsComputeTranscodeEtcToBc,
                            !mPhysicalDeviceFeatures.textureCompressionETC2 &&
                                kSupportTranscodeEtcToBc &&
                                (mSubgroupProperties.supportedOperations & kRequiredSubgroupOp) ==
                                    kRequiredSubgroupOp &&
                                (limitsVk.maxTexelBufferElements >= kMaxTexelBufferSize));

    // Limit GL_MAX_SHADER_STORAGE_BLOCK_SIZE to 256MB on older ARM hardware.
    ANGLE_FEATURE_CONDITION(&mFeatures, limitMaxStorageBufferSize, isMaliJobManagerBasedGPU);

    // http://anglebug.com/42265782
    // Flushing mutable textures causes flakes in perf tests using Windows/Intel GPU. Failures are
    // due to lost context/device.
    // http://b/278600575
    // Flushing mutable texture is disabled for discrete GPUs to mitigate possible VRAM OOM.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, mutableMipmapTextureUpload,
        canPreferDeviceLocalMemoryHostVisible(mPhysicalDeviceProperties.deviceType));

    // Allow passthrough of EGL colorspace attributes on Android platform and for vendors that
    // are known to support wide color gamut.
    ANGLE_FEATURE_CONDITION(&mFeatures, eglColorspaceAttributePassthrough,
                            IsAndroid() && isSamsung);

    // GBM does not have a VkSurface hence it does not support presentation through a Vulkan queue.
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsPresentation,
                            nativeWindowSystem != angle::NativeWindowSystem::Gbm);

    // For tiled renderer, the renderpass query result may not available until the entire renderpass
    // is completed. This may cause a bubble in the application thread waiting result to be
    // available. When this feature flag is enabled, we will issue an immediate flush when we detect
    // there is switch from query enabled draw to query disabled draw. Since most apps uses bunch of
    // query back to back, this should only introduce one extra flush per frame.
    // https://issuetracker.google.com/250706693
    ANGLE_FEATURE_CONDITION(&mFeatures, preferSubmitOnAnySamplesPassedQueryEnd,
                            isTileBasedRenderer);

    // ARM driver appears having a bug that if we did not wait for submission to complete, but call
    // vkGetQueryPoolResults(VK_QUERY_RESULT_WAIT_BIT), it may result VK_NOT_READY.
    // https://issuetracker.google.com/253522366
    //
    // Workaround for nvidia earlier version driver which appears having a bug that On older nvidia
    // driver, vkGetQueryPoolResult() with VK_QUERY_RESULT_WAIT_BIT may result in incorrect result.
    // In that case we force into CPU wait for submission to complete. http://anglebug.com/42265186
    ANGLE_FEATURE_CONDITION(&mFeatures, forceWaitForSubmissionToCompleteForQueryResult,
                            isARM || (isNvidia && driverVersion < angle::VersionTriple(470, 0, 0)));

    // Some ARM drivers may not free memory in "vkFreeCommandBuffers()" without
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag.
    ANGLE_FEATURE_CONDITION(&mFeatures, useResetCommandBufferBitForSecondaryPools, isARM);

    // Intel and AMD mesa drivers need depthBiasConstantFactor to be doubled to align with GL.
    ANGLE_FEATURE_CONDITION(&mFeatures, doubleDepthBiasConstantFactor,
                            (isIntel && !IsWindows()) || isRADV || isNvidia);

    // Required to pass android.media.codec.cts.EncodeDecodeTest
    // https://issuetracker.google.com/246218584
    ANGLE_FEATURE_CONDITION(
        &mFeatures, mapUnspecifiedColorSpaceToPassThrough,
        ExtensionFound(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, mEnabledInstanceExtensions));

    ANGLE_FEATURE_CONDITION(&mFeatures, enablePipelineCacheDataCompression, true);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsTimelineSemaphore,
                            mTimelineSemaphoreFeatures.timelineSemaphore == VK_TRUE);

    // 8bit storage features
    ANGLE_FEATURE_CONDITION(&mFeatures, supports8BitStorageBuffer,
                            m8BitStorageFeatures.storageBuffer8BitAccess == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supports8BitUniformAndStorageBuffer,
                            m8BitStorageFeatures.uniformAndStorageBuffer8BitAccess == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supports8BitPushConstant,
                            m8BitStorageFeatures.storagePushConstant8 == VK_TRUE);

    // 16bit storage features
    ANGLE_FEATURE_CONDITION(&mFeatures, supports16BitStorageBuffer,
                            m16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supports16BitUniformAndStorageBuffer,
                            m16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supports16BitPushConstant,
                            m16BitStorageFeatures.storagePushConstant16 == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supports16BitInputOutput,
                            m16BitStorageFeatures.storageInputOutput16 == VK_TRUE);

#if defined(ANGLE_PLATFORM_ANDROID)
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExternalFormatResolve,
                            mExternalFormatResolveFeatures.externalFormatResolve == VK_TRUE);
#else
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsExternalFormatResolve, false);
#endif

    // VkEvent has much bigger overhead. Until we know that it helps desktop GPUs, we restrict it to
    // TBRs. Also enabled for SwiftShader so that we get more test coverage in bots.
    ANGLE_FEATURE_CONDITION(&mFeatures, useVkEventForImageBarrier,
                            isTileBasedRenderer || isSwiftShader);
    ANGLE_FEATURE_CONDITION(&mFeatures, useVkEventForBufferBarrier,
                            isTileBasedRenderer || isSwiftShader);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsMaintenance5,
                            mMaintenance5Features.maintenance5 == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsDynamicRendering,
                            mDynamicRenderingFeatures.dynamicRendering == VK_TRUE);

    // Disabled on Nvidia driver due to a bug with attachment location mapping, resulting in
    // incorrect rendering in the presence of gaps in locations.  http://anglebug.com/372883691.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsDynamicRenderingLocalRead,
        mDynamicRenderingLocalReadFeatures.dynamicRenderingLocalRead == VK_TRUE && !isNvidia);

    // Using dynamic rendering when VK_KHR_dynamic_rendering_local_read is available, because that's
    // needed for framebuffer fetch, MSRTT and advanced blend emulation.
    //
    // VK_EXT_legacy_dithering needs to be at version 2 and VK_KHR_maintenance5 to be usable with
    // dynamic rendering.  If only version 1 is exposed, it's not sacrificied for dynamic rendering
    // and render pass objects are continued to be used.
    //
    // Emulation of GL_EXT_multisampled_render_to_texture is not possible with dynamic rendering.
    // That support is also not sacrificed for dynamic rendering.
    //
    // Use of dynamic rendering is disabled on older ARM drivers due to driver bugs
    // (http://issuetracker.google.com/356051947).
    //
    // Use of dynamic rendering on PowerVR devices is disabled for performance reasons
    // (http://issuetracker.google.com/372273294).
    const bool hasLegacyDitheringV1 =
        mFeatures.supportsLegacyDithering.enabled &&
        (mLegacyDitheringVersion < 2 || !mFeatures.supportsMaintenance5.enabled);
    const bool emulatesMultisampledRenderToTexture =
        mFeatures.enableMultisampledRenderToTexture.enabled &&
        !mFeatures.supportsMultisampledRenderToSingleSampled.enabled;
    ANGLE_FEATURE_CONDITION(&mFeatures, preferDynamicRendering,
                            mFeatures.supportsDynamicRendering.enabled &&
                                mFeatures.supportsDynamicRenderingLocalRead.enabled &&
                                !hasLegacyDitheringV1 && !emulatesMultisampledRenderToTexture &&
                                !(isARM && driverVersion < angle::VersionTriple(52, 0, 0)) &&
                                !isPowerVR);

    // On tile-based renderers, breaking the render pass is costly.  Changing into and out of
    // framebuffer fetch causes the render pass to break so that the layout of the color attachments
    // can be adjusted.  On such hardware, the switch to framebuffer fetch mode is made permanent so
    // such render pass breaks don't happen.
    //
    // This only applies to legacy render passes; with dynamic rendering there is no render pass
    // break when switching framebuffer fetch usage.
    ANGLE_FEATURE_CONDITION(&mFeatures, permanentlySwitchToFramebufferFetchMode,
                            isTileBasedRenderer && !mFeatures.preferDynamicRendering.enabled);

    // Vulkan supports depth/stencil input attachments same as it does with color.
    // GL_ARM_shader_framebuffer_fetch_depth_stencil requires coherent behavior however, so this
    // extension is exposed only where coherent framebuffer fetch is available.
    //
    // Additionally, the implementation assumes VK_KHR_dynamic_rendering_local_read to avoid
    // complications with VkRenderPass objects.
    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsShaderFramebufferFetchDepthStencil,
        mFeatures.supportsShaderFramebufferFetch.enabled &&
            mRasterizationOrderAttachmentAccessFeatures.rasterizationOrderDepthAttachmentAccess ==
                VK_TRUE &&
            mRasterizationOrderAttachmentAccessFeatures.rasterizationOrderStencilAttachmentAccess ==
                VK_TRUE &&
            mFeatures.preferDynamicRendering.enabled);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsSynchronization2,
                            mSynchronization2Features.synchronization2 == VK_TRUE);

    // Disable descriptorSet cache for SwiftShader to ensure the code path gets tested.
    ANGLE_FEATURE_CONDITION(&mFeatures, descriptorSetCache, !isSwiftShader);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsImageCompressionControl,
                            mImageCompressionControlFeatures.imageCompressionControl == VK_TRUE);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsImageCompressionControlSwapchain,
        mImageCompressionControlSwapchainFeatures.imageCompressionControlSwapchain == VK_TRUE);

    ANGLE_FEATURE_CONDITION(&mFeatures, supportsAstcSliced3d, isARM);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsTextureCompressionAstcHdr,
        mTextureCompressionASTCHDRFeatures.textureCompressionASTC_HDR == VK_TRUE);

    ANGLE_FEATURE_CONDITION(
        &mFeatures, supportsUniformBufferStandardLayout,
        mUniformBufferStandardLayoutFeatures.uniformBufferStandardLayout == VK_TRUE);

    // Disable memory report feature overrides if extension is not supported.
    if ((mFeatures.logMemoryReportCallbacks.enabled || mFeatures.logMemoryReportStats.enabled) &&
        !mMemoryReportFeatures.deviceMemoryReport)
    {
        WARN() << "Disabling the following feature(s) because driver does not support "
                  "VK_EXT_device_memory_report extension:";
        if (getFeatures().logMemoryReportStats.enabled)
        {
            WARN() << "\tlogMemoryReportStats";
            mFeatures.logMemoryReportStats.applyOverride(false);
        }
        if (getFeatures().logMemoryReportCallbacks.enabled)
        {
            WARN() << "\tlogMemoryReportCallbacks";
            mFeatures.logMemoryReportCallbacks.applyOverride(false);
        }
    }

    // Check if VK implementation needs to strip-out non-semantic reflection info from shader module
    // (Default is to assume not supported)
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsShaderNonSemanticInfo, false);

    // Don't expose these 2 extensions on Samsung devices -
    // 1. ANGLE_rgbx_internal_format
    // 2. GL_APPLE_clip_distance
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsAngleRgbxInternalFormat, !isSamsung);
    ANGLE_FEATURE_CONDITION(&mFeatures, supportsAppleClipDistance, !isSamsung);

    // Enable the use of below native kernels
    // Each builtin kernel gets its own feature and condition, for now a single feature condition is
    // setup
    ANGLE_FEATURE_CONDITION(&mFeatures, usesNativeBuiltinClKernel, isSamsung);
}

void Renderer::appBasedFeatureOverrides(const vk::ExtensionNameList &extensions) {}

angle::Result Renderer::initPipelineCache(vk::ErrorContext *context,
                                          vk::PipelineCache *pipelineCache,
                                          bool *success)
{
    angle::MemoryBuffer initialData;
    if (!mFeatures.disablePipelineCacheLoadForTesting.enabled)
    {
        ANGLE_TRY(GetAndDecompressPipelineCacheVk(context, mGlobalOps, &initialData, success));
    }

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};

    pipelineCacheCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.flags           = 0;
    pipelineCacheCreateInfo.initialDataSize = *success ? initialData.size() : 0;
    pipelineCacheCreateInfo.pInitialData    = *success ? initialData.data() : nullptr;

    ANGLE_VK_TRY(context, pipelineCache->init(mDevice, pipelineCacheCreateInfo));

    return angle::Result::Continue;
}

angle::Result Renderer::ensurePipelineCacheInitialized(vk::ErrorContext *context)
{
    // If it is initialized already, there is nothing to do
    if (mPipelineCacheInitialized)
    {
        return angle::Result::Continue;
    }

    std::unique_lock<angle::SimpleMutex> lock(mPipelineCacheMutex);

    // If another thread initialized it first don't redo it
    if (mPipelineCacheInitialized)
    {
        return angle::Result::Continue;
    }

    // We should now create the pipeline cache with the blob cache pipeline data.
    bool loadedFromBlobCache = false;
    ANGLE_TRY(initPipelineCache(context, &mPipelineCache, &loadedFromBlobCache));
    if (loadedFromBlobCache)
    {
        ANGLE_TRY(getLockedPipelineCacheDataIfNew(context, &mPipelineCacheSizeAtLastSync,
                                                  mPipelineCacheSizeAtLastSync, nullptr));
    }

    mPipelineCacheInitialized = true;

    return angle::Result::Continue;
}

size_t Renderer::getNextPipelineCacheBlobCacheSlotIndex(size_t *previousSlotIndexOut)
{
    if (previousSlotIndexOut != nullptr)
    {
        *previousSlotIndexOut = mCurrentPipelineCacheBlobCacheSlotIndex;
    }
    if (getFeatures().useDualPipelineBlobCacheSlots.enabled)
    {
        mCurrentPipelineCacheBlobCacheSlotIndex = 1 - mCurrentPipelineCacheBlobCacheSlotIndex;
    }
    return mCurrentPipelineCacheBlobCacheSlotIndex;
}

size_t Renderer::updatePipelineCacheChunkCount(size_t chunkCount)
{
    const size_t previousChunkCount = mPipelineCacheChunkCount;
    mPipelineCacheChunkCount        = chunkCount;
    return previousChunkCount;
}

angle::Result Renderer::getPipelineCache(vk::ErrorContext *context,
                                         vk::PipelineCacheAccess *pipelineCacheOut)
{
    ANGLE_TRY(ensurePipelineCacheInitialized(context));

    angle::SimpleMutex *pipelineCacheMutex =
        context->getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled ||
                context->getFeatures().preferMonolithicPipelinesOverLibraries.enabled
            ? &mPipelineCacheMutex
            : nullptr;

    pipelineCacheOut->init(&mPipelineCache, pipelineCacheMutex);
    return angle::Result::Continue;
}

angle::Result Renderer::mergeIntoPipelineCache(vk::ErrorContext *context,
                                               const vk::PipelineCache &pipelineCache)
{
    // It is an error to call into this method when the feature is disabled.
    ASSERT(context->getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled);

    vk::PipelineCacheAccess globalCache;
    ANGLE_TRY(getPipelineCache(context, &globalCache));

    globalCache.merge(this, pipelineCache);

    return angle::Result::Continue;
}

const gl::Caps &Renderer::getNativeCaps() const
{
    ensureCapsInitialized();
    return mNativeCaps;
}

const gl::TextureCapsMap &Renderer::getNativeTextureCaps() const
{
    ensureCapsInitialized();
    return mNativeTextureCaps;
}

const gl::Extensions &Renderer::getNativeExtensions() const
{
    ensureCapsInitialized();
    return mNativeExtensions;
}

const gl::Limitations &Renderer::getNativeLimitations() const
{
    ensureCapsInitialized();
    return mNativeLimitations;
}

const ShPixelLocalStorageOptions &Renderer::getNativePixelLocalStorageOptions() const
{
    return mNativePLSOptions;
}

void Renderer::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    const bool isSwiftShader =
        IsSwiftshader(mPhysicalDeviceProperties.vendorID, mPhysicalDeviceProperties.deviceID);
    const bool isSamsung = IsSamsung(mPhysicalDeviceProperties.vendorID);

    // Hopefully-temporary work-around for a crash on SwiftShader.  An Android process is turning
    // off GL error checking, and then asking ANGLE to write past the end of a buffer.
    // https://issuetracker.google.com/issues/220069903
    ANGLE_FEATURE_CONDITION(features, forceGlErrorChecking, (IsAndroid() && isSwiftShader));

    // Disable shader and program caching on Samsung devices.
    ANGLE_FEATURE_CONDITION(features, cacheCompiledShader, !isSamsung);
    ANGLE_FEATURE_CONDITION(features, disableProgramCaching, isSamsung);

    // https://issuetracker.google.com/292285899
    ANGLE_FEATURE_CONDITION(features, uncurrentEglSurfaceUponSurfaceDestroy, true);

    // The Vulkan backend's handling of compile and link is thread-safe
    ANGLE_FEATURE_CONDITION(features, compileJobIsThreadSafe, true);
    ANGLE_FEATURE_CONDITION(features, linkJobIsThreadSafe, true);
    // Always run the link's warm up job in a thread.  It's an optimization only, and does not block
    // the link resolution.
    ANGLE_FEATURE_CONDITION(features, alwaysRunLinkSubJobsThreaded, true);
}

angle::Result Renderer::getLockedPipelineCacheDataIfNew(vk::ErrorContext *context,
                                                        size_t *pipelineCacheSizeOut,
                                                        size_t lastSyncSize,
                                                        std::vector<uint8_t> *pipelineCacheDataOut)
{
    // Because this function may call |getCacheData| twice, |mPipelineCacheMutex| is not passed to
    // |PipelineAccessCache|, and is expected to be locked once **by the caller**.
    mPipelineCacheMutex.assertLocked();

    vk::PipelineCacheAccess globalCache;
    globalCache.init(&mPipelineCache, nullptr);

    ANGLE_VK_TRY(context, globalCache.getCacheData(context, pipelineCacheSizeOut, nullptr));

    // If the cache data is unchanged since last sync, don't retrieve the data.  Also, make sure we
    // will receive enough data to hold the pipeline cache header Table 7.  Layout for pipeline
    // cache header version VK_PIPELINE_CACHE_HEADER_VERSION_ONE.
    const size_t kPipelineCacheHeaderSize = 16 + VK_UUID_SIZE;
    if (*pipelineCacheSizeOut <= lastSyncSize || *pipelineCacheSizeOut < kPipelineCacheHeaderSize ||
        pipelineCacheDataOut == nullptr)
    {
        return angle::Result::Continue;
    }

    pipelineCacheDataOut->resize(*pipelineCacheSizeOut);
    VkResult result =
        globalCache.getCacheData(context, pipelineCacheSizeOut, pipelineCacheDataOut->data());
    if (ANGLE_UNLIKELY(result == VK_INCOMPLETE))
    {
        WARN()
            << "Received VK_INCOMPLETE when retrieving pipeline cache data, which should be "
               "impossible as the size query was previously done under the same lock, but this is "
               "a recoverable error";
    }
    else
    {
        ANGLE_VK_TRY(context, result);
    }

    // If vkGetPipelineCacheData ends up writing fewer bytes than requested, shrink the buffer to
    // avoid leaking garbage memory and potential rejection of the data by subsequent
    // vkCreatePipelineCache call.  Some drivers may ignore entire buffer if there padding present.
    ASSERT(*pipelineCacheSizeOut <= pipelineCacheDataOut->size());
    pipelineCacheDataOut->resize(*pipelineCacheSizeOut);

    return angle::Result::Continue;
}

angle::Result Renderer::syncPipelineCacheVk(vk::ErrorContext *context,
                                            vk::GlobalOps *globalOps,
                                            const gl::Context *contextGL)
{
    // Skip syncing until pipeline cache is initialized.
    if (!mPipelineCacheInitialized)
    {
        return angle::Result::Continue;
    }
    ASSERT(mPipelineCache.valid());

    if (!mFeatures.syncMonolithicPipelinesToBlobCache.enabled)
    {
        return angle::Result::Continue;
    }

    if (--mPipelineCacheVkUpdateTimeout > 0)
    {
        return angle::Result::Continue;
    }

    mPipelineCacheVkUpdateTimeout = kPipelineCacheVkUpdatePeriod;

    ContextVk *contextVk = vk::GetImpl(contextGL);

    // Use worker thread pool to complete compression.
    // If the last task hasn't been finished, skip the syncing.
    if (mCompressEvent && !mCompressEvent->isReady())
    {
        ANGLE_PERF_WARNING(contextVk->getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Skip syncing pipeline cache data when the last task is not ready.");
        return angle::Result::Continue;
    }

    size_t pipelineCacheSize = 0;
    std::vector<uint8_t> pipelineCacheData;
    {
        std::unique_lock<angle::SimpleMutex> lock(mPipelineCacheMutex);
        ANGLE_TRY(getLockedPipelineCacheDataIfNew(
            context, &pipelineCacheSize, mPipelineCacheSizeAtLastSync, &pipelineCacheData));
    }
    if (pipelineCacheData.empty())
    {
        return angle::Result::Continue;
    }
    mPipelineCacheSizeAtLastSync = pipelineCacheSize;

    if (mFeatures.enableAsyncPipelineCacheCompression.enabled)
    {
        // zlib compression ratio normally ranges from 2:1 to 5:1. Set kMaxTotalSize to 64M to
        // ensure the size can fit into the 32MB blob cache limit on supported platforms.
        constexpr size_t kMaxTotalSize = 64 * 1024 * 1024;

        // Create task to compress.
        mCompressEvent = contextGL->getWorkerThreadPool()->postWorkerTask(
            std::make_shared<CompressAndStorePipelineCacheTask>(
                globalOps, this, std::move(pipelineCacheData), kMaxTotalSize));
    }
    else
    {
        // If enableAsyncPipelineCacheCompression is disabled, to avoid the risk, set kMaxTotalSize
        // to 64k.
        constexpr size_t kMaxTotalSize = 64 * 1024;
        CompressAndStorePipelineCacheVk(globalOps, this, pipelineCacheData, kMaxTotalSize);
    }

    return angle::Result::Continue;
}

// These functions look at the mandatory format for support, and fallback to querying the device (if
// necessary) to test the availability of the bits.
bool Renderer::hasLinearImageFormatFeatureBits(angle::FormatID formatID,
                                               const VkFormatFeatureFlags featureBits) const
{
    return hasFormatFeatureBits<&VkFormatProperties::linearTilingFeatures>(formatID, featureBits);
}

VkFormatFeatureFlags Renderer::getLinearImageFormatFeatureBits(
    angle::FormatID formatID,
    const VkFormatFeatureFlags featureBits) const
{
    return getFormatFeatureBits<&VkFormatProperties::linearTilingFeatures>(formatID, featureBits);
}

VkFormatFeatureFlags Renderer::getImageFormatFeatureBits(
    angle::FormatID formatID,
    const VkFormatFeatureFlags featureBits) const
{
    return getFormatFeatureBits<&VkFormatProperties::optimalTilingFeatures>(formatID, featureBits);
}

bool Renderer::hasImageFormatFeatureBits(angle::FormatID formatID,
                                         const VkFormatFeatureFlags featureBits) const
{
    return hasFormatFeatureBits<&VkFormatProperties::optimalTilingFeatures>(formatID, featureBits);
}

bool Renderer::hasBufferFormatFeatureBits(angle::FormatID formatID,
                                          const VkFormatFeatureFlags featureBits) const
{
    return hasFormatFeatureBits<&VkFormatProperties::bufferFeatures>(formatID, featureBits);
}

void Renderer::outputVmaStatString()
{
    // Output the VMA stats string
    // This JSON string can be passed to VmaDumpVis.py to generate a visualization of the
    // allocations the VMA has performed.
    char *statsString;
    mAllocator.buildStatsString(&statsString, true);
    INFO() << std::endl << statsString << std::endl;
    mAllocator.freeStatsString(statsString);
}

angle::Result Renderer::queueSubmitOneOff(vk::ErrorContext *context,
                                          vk::ScopedPrimaryCommandBuffer &&scopedCommandBuffer,
                                          vk::ProtectionType protectionType,
                                          egl::ContextPriority priority,
                                          VkSemaphore waitSemaphore,
                                          VkPipelineStageFlags waitSemaphoreStageMasks,
                                          QueueSerial *queueSerialOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::queueSubmitOneOff");
    DeviceScoped<PrimaryCommandBuffer> commandBuffer = scopedCommandBuffer.unlockAndRelease();
    PrimaryCommandBuffer &primary                    = commandBuffer.get();

    // Allocate a one off SerialIndex and generate a QueueSerial and then use it and release the
    // index.
    vk::ScopedQueueSerialIndex index;
    ANGLE_TRY(allocateScopedQueueSerialIndex(&index));
    QueueSerial submitQueueSerial(index.get(), generateQueueSerial(index.get()));

    ANGLE_TRY(mCommandQueue.queueSubmitOneOff(context, protectionType, priority,
                                              primary.getHandle(), waitSemaphore,
                                              waitSemaphoreStageMasks, submitQueueSerial));

    *queueSerialOut = submitQueueSerial;
    if (primary.valid())
    {
        mOneOffCommandPoolMap[protectionType].releaseCommandBuffer(submitQueueSerial,
                                                                   std::move(primary));
    }

    ANGLE_TRY(mCommandQueue.postSubmitCheck(context));

    return angle::Result::Continue;
}

angle::Result Renderer::queueSubmitWaitSemaphore(vk::ErrorContext *context,
                                                 egl::ContextPriority priority,
                                                 const vk::Semaphore &waitSemaphore,
                                                 VkPipelineStageFlags waitSemaphoreStageMasks,
                                                 QueueSerial submitQueueSerial)
{
    return mCommandQueue.queueSubmitOneOff(context, vk::ProtectionType::Unprotected, priority,
                                           VK_NULL_HANDLE, waitSemaphore.getHandle(),
                                           waitSemaphoreStageMasks, submitQueueSerial);
}

template <VkFormatFeatureFlags VkFormatProperties::*features>
VkFormatFeatureFlags Renderer::getFormatFeatureBits(angle::FormatID formatID,
                                                    const VkFormatFeatureFlags featureBits) const
{
    ASSERT(formatID != angle::FormatID::NONE);
    VkFormatProperties &deviceProperties = mFormatProperties[formatID];

    if (deviceProperties.bufferFeatures == kInvalidFormatFeatureFlags)
    {
        // If we don't have the actual device features, see if the requested features are mandatory.
        // If so, there's no need to query the device.
        const VkFormatProperties &mandatoryProperties = vk::GetMandatoryFormatSupport(formatID);
        if (IsMaskFlagSet(mandatoryProperties.*features, featureBits))
        {
            return featureBits;
        }

        if (vk::IsYUVExternalFormat(formatID))
        {
            const vk::ExternalYuvFormatInfo &externalFormatInfo =
                mExternalFormatTable.getExternalFormatInfo(formatID);
            deviceProperties.optimalTilingFeatures = externalFormatInfo.formatFeatures;
        }
        else
        {
            VkFormat vkFormat = vk::GetVkFormatFromFormatID(this, formatID);
            ASSERT(vkFormat != VK_FORMAT_UNDEFINED);

            // Otherwise query the format features and cache it.
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, vkFormat, &deviceProperties);
            // Workaround for some Android devices that don't indicate filtering
            // support on D16_UNORM and they should.
            if (mFeatures.forceD16TexFilter.enabled && vkFormat == VK_FORMAT_D16_UNORM)
            {
                deviceProperties.*features |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
            }
        }
    }

    return deviceProperties.*features & featureBits;
}

template <VkFormatFeatureFlags VkFormatProperties::*features>
bool Renderer::hasFormatFeatureBits(angle::FormatID formatID,
                                    const VkFormatFeatureFlags featureBits) const
{
    return IsMaskFlagSet(getFormatFeatureBits<features>(formatID, featureBits), featureBits);
}

bool Renderer::haveSameFormatFeatureBits(angle::FormatID formatID1, angle::FormatID formatID2) const
{
    if (formatID1 == angle::FormatID::NONE || formatID2 == angle::FormatID::NONE)
    {
        return false;
    }

    constexpr VkFormatFeatureFlags kImageUsageFeatureBits =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

    VkFormatFeatureFlags fmt1LinearFeatureBits =
        getLinearImageFormatFeatureBits(formatID1, kImageUsageFeatureBits);
    VkFormatFeatureFlags fmt1OptimalFeatureBits =
        getImageFormatFeatureBits(formatID1, kImageUsageFeatureBits);

    return hasLinearImageFormatFeatureBits(formatID2, fmt1LinearFeatureBits) &&
           hasImageFormatFeatureBits(formatID2, fmt1OptimalFeatureBits);
}

void Renderer::cleanupGarbage(bool *anyGarbageCleanedOut)
{
    bool anyCleaned = false;

    // Clean up general garbage
    anyCleaned = (mSharedGarbageList.cleanupSubmittedGarbage(this) > 0) || anyCleaned;

    // Clean up suballocation garbages
    anyCleaned = (mSuballocationGarbageList.cleanupSubmittedGarbage(this) > 0) || anyCleaned;

    // Note: do this after clean up mSuballocationGarbageList so that we will have more chances to
    // find orphaned blocks being empty.
    anyCleaned = (mOrphanedBufferBlockList.pruneEmptyBufferBlocks(this) > 0) || anyCleaned;

    // Clean up RefCountedEvent that are done resetting
    anyCleaned = (mRefCountedEventRecycler.cleanupResettingEvents(this) > 0) || anyCleaned;

    if (anyGarbageCleanedOut != nullptr)
    {
        *anyGarbageCleanedOut = anyCleaned;
    }
}

void Renderer::cleanupPendingSubmissionGarbage()
{
    // Check if pending garbage is still pending. If not, move them to the garbage list.
    mSharedGarbageList.cleanupUnsubmittedGarbage(this);
    mSuballocationGarbageList.cleanupUnsubmittedGarbage(this);
}

void Renderer::onNewValidationMessage(const std::string &message)
{
    mLastValidationMessage = message;
    ++mValidationMessageCount;
}

std::string Renderer::getAndClearLastValidationMessage(uint32_t *countSinceLastClear)
{
    *countSinceLastClear    = mValidationMessageCount;
    mValidationMessageCount = 0;

    return std::move(mLastValidationMessage);
}

uint64_t Renderer::getMaxFenceWaitTimeNs() const
{
    constexpr uint64_t kMaxFenceWaitTimeNs = std::numeric_limits<uint64_t>::max();

    return kMaxFenceWaitTimeNs;
}

void Renderer::setGlobalDebugAnnotator(bool *installedAnnotatorOut)
{
    // Install one of two DebugAnnotator classes:
    //
    // 1) The global class enables basic ANGLE debug functionality (e.g. Vulkan validation errors
    //    will cause dEQP tests to fail).
    //
    // 2) The DebugAnnotatorVk class processes OpenGL ES commands that the application uses.  It is
    //    installed for the following purposes:
    //
    //    1) To enable calling the vkCmd*DebugUtilsLabelEXT functions in order to communicate to
    //       debuggers (e.g. AGI) the OpenGL ES commands that the application uses.  In addition to
    //       simply installing DebugAnnotatorVk, also enable calling vkCmd*DebugUtilsLabelEXT.
    //
    //    2) To enable logging to Android logcat the OpenGL ES commands that the application uses.
    bool installDebugAnnotatorVk = false;

    // Enable calling the vkCmd*DebugUtilsLabelEXT functions if the vkCmd*DebugUtilsLabelEXT
    // functions exist, and if the kEnableDebugMarkersVarName environment variable is set.
    if (vkCmdBeginDebugUtilsLabelEXT)
    {
        // Use the GetAndSet variant to improve future lookup times
        std::string enabled = angle::GetAndSetEnvironmentVarOrUnCachedAndroidProperty(
            kEnableDebugMarkersVarName, kEnableDebugMarkersPropertyName);
        if (!enabled.empty() && enabled.compare("0") != 0)
        {
            mAngleDebuggerMode      = true;
            installDebugAnnotatorVk = true;
        }
    }
#if defined(ANGLE_ENABLE_TRACE_ANDROID_LOGCAT)
    // Only install DebugAnnotatorVk to log all API commands to Android's logcat.
    installDebugAnnotatorVk = true;
#endif

    {
        if (installDebugAnnotatorVk)
        {
            std::unique_lock<angle::SimpleMutex> lock(gl::GetDebugMutex());
            gl::InitializeDebugAnnotations(&mAnnotator);
        }
    }

    *installedAnnotatorOut = installDebugAnnotatorVk;
}

void Renderer::reloadVolkIfNeeded() const
{
#if defined(ANGLE_SHARED_LIBVULKAN)
    if ((mInstance != VK_NULL_HANDLE) && (volkGetLoadedInstance() != mInstance))
    {
        volkLoadInstance(mInstance);
    }

    if ((mDevice != VK_NULL_HANDLE) && (volkGetLoadedDevice() != mDevice))
    {
        volkLoadDevice(mDevice);
    }

    initializeInstanceExtensionEntryPointsFromCore();
    initializeDeviceExtensionEntryPointsFromCore();
#endif  // defined(ANGLE_SHARED_LIBVULKAN)
}

void Renderer::initializeInstanceExtensionEntryPointsFromCore() const
{
    // Initialize extension entry points from core ones.  In some cases, such as VMA, the extension
    // entry point is unconditionally used.
    InitGetPhysicalDeviceProperties2KHRFunctionsFromCore();
    if (mFeatures.supportsExternalFenceCapabilities.enabled)
    {
        InitExternalFenceCapabilitiesFunctionsFromCore();
    }
    if (mFeatures.supportsExternalSemaphoreCapabilities.enabled)
    {
        InitExternalSemaphoreCapabilitiesFunctionsFromCore();
    }
}

void Renderer::initializeDeviceExtensionEntryPointsFromCore() const
{
    if (mFeatures.supportsGetMemoryRequirements2.enabled)
    {
        InitGetMemoryRequirements2KHRFunctionsFromCore();
    }
    if (mFeatures.supportsBindMemory2.enabled)
    {
        InitBindMemory2KHRFunctionsFromCore();
    }
    if (mFeatures.supportsYUVSamplerConversion.enabled)
    {
        InitSamplerYcbcrKHRFunctionsFromCore();
    }
}

angle::Result Renderer::submitCommands(
    vk::ErrorContext *context,
    vk::ProtectionType protectionType,
    egl::ContextPriority contextPriority,
    const vk::Semaphore *signalSemaphore,
    const vk::SharedExternalFence *externalFence,
    std::vector<VkImageMemoryBarrier> &&imagesToTransitionToForeign,
    const QueueSerial &submitQueueSerial)
{
    ASSERT(signalSemaphore == nullptr || signalSemaphore->valid());
    const VkSemaphore signalVkSemaphore =
        signalSemaphore ? signalSemaphore->getHandle() : VK_NULL_HANDLE;

    vk::SharedExternalFence externalFenceCopy;
    if (externalFence != nullptr)
    {
        externalFenceCopy = *externalFence;
    }

    ANGLE_TRY(mCommandQueue.submitCommands(
        context, protectionType, contextPriority, signalVkSemaphore, std::move(externalFenceCopy),
        std::move(imagesToTransitionToForeign), submitQueueSerial));

    ANGLE_TRY(mCommandQueue.postSubmitCheck(context));

    return angle::Result::Continue;
}

angle::Result Renderer::submitPriorityDependency(vk::ErrorContext *context,
                                                 vk::ProtectionTypes protectionTypes,
                                                 egl::ContextPriority srcContextPriority,
                                                 egl::ContextPriority dstContextPriority,
                                                 SerialIndex index)
{
    RendererScoped<vk::ReleasableResource<vk::Semaphore>> semaphore(this);
    ANGLE_VK_TRY(context, semaphore.get().get().init(mDevice));

    // First, submit already flushed commands / wait semaphores into the source Priority VkQueue.
    // Commands that are in the Secondary Command Buffers will be flushed into the new VkQueue.

    // Submit commands and attach Signal Semaphore.
    ASSERT(protectionTypes.any());
    while (protectionTypes.any())
    {
        vk::ProtectionType protectionType = protectionTypes.first();
        protectionTypes.reset(protectionType);

        QueueSerial queueSerial(index, generateQueueSerial(index));
        // Submit semaphore only if this is the last submission (all into the same VkQueue).
        const vk::Semaphore *signalSemaphore = nullptr;
        if (protectionTypes.none())
        {
            // Update QueueSerial to collect semaphore using the latest possible queueSerial.
            semaphore.get().setQueueSerial(queueSerial);
            signalSemaphore = &semaphore.get().get();
        }
        ANGLE_TRY(submitCommands(context, protectionType, srcContextPriority, signalSemaphore,
                                 nullptr, {}, queueSerial));
    }

    // Submit only Wait Semaphore into the destination Priority (VkQueue).
    QueueSerial queueSerial(index, generateQueueSerial(index));
    semaphore.get().setQueueSerial(queueSerial);
    ANGLE_TRY(queueSubmitWaitSemaphore(context, dstContextPriority, semaphore.get().get(),
                                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, queueSerial));

    return angle::Result::Continue;
}

void Renderer::handleDeviceLost()
{
    mCommandQueue.handleDeviceLost(this);
}

angle::Result Renderer::finishResourceUse(vk::ErrorContext *context, const vk::ResourceUse &use)
{
    return mCommandQueue.finishResourceUse(context, use, getMaxFenceWaitTimeNs());
}

angle::Result Renderer::finishQueueSerial(vk::ErrorContext *context, const QueueSerial &queueSerial)
{
    ASSERT(queueSerial.valid());
    return mCommandQueue.finishQueueSerial(context, queueSerial, getMaxFenceWaitTimeNs());
}

angle::Result Renderer::waitForResourceUseToFinishWithUserTimeout(vk::ErrorContext *context,
                                                                  const vk::ResourceUse &use,
                                                                  uint64_t timeout,
                                                                  VkResult *result)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::waitForResourceUseToFinishWithUserTimeout");
    return mCommandQueue.waitForResourceUseToFinishWithUserTimeout(context, use, timeout, result);
}

angle::Result Renderer::flushWaitSemaphores(
    vk::ProtectionType protectionType,
    egl::ContextPriority priority,
    std::vector<VkSemaphore> &&waitSemaphores,
    std::vector<VkPipelineStageFlags> &&waitSemaphoreStageMasks)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::flushWaitSemaphores");
    mCommandQueue.flushWaitSemaphores(protectionType, priority, std::move(waitSemaphores),
                                      std::move(waitSemaphoreStageMasks));

    return angle::Result::Continue;
}

angle::Result Renderer::flushRenderPassCommands(
    vk::Context *context,
    vk::ProtectionType protectionType,
    egl::ContextPriority priority,
    const vk::RenderPass &renderPass,
    VkFramebuffer framebufferOverride,
    vk::RenderPassCommandBufferHelper **renderPassCommands)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::flushRenderPassCommands");
    return mCommandQueue.flushRenderPassCommands(context, protectionType, priority, renderPass,
                                                 framebufferOverride, renderPassCommands);
}

angle::Result Renderer::flushOutsideRPCommands(
    vk::Context *context,
    vk::ProtectionType protectionType,
    egl::ContextPriority priority,
    vk::OutsideRenderPassCommandBufferHelper **outsideRPCommands)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::flushOutsideRPCommands");
    return mCommandQueue.flushOutsideRPCommands(context, protectionType, priority,
                                                outsideRPCommands);
}

VkResult Renderer::queuePresent(vk::ErrorContext *context,
                                egl::ContextPriority priority,
                                const VkPresentInfoKHR &presentInfo)
{
    VkResult result = mCommandQueue.queuePresent(priority, presentInfo);

    if (getFeatures().logMemoryReportStats.enabled)
    {
        mMemoryReport.logMemoryReportStats();
    }

    return result;
}

template <typename CommandBufferHelperT, typename RecyclerT>
angle::Result Renderer::getCommandBufferImpl(vk::ErrorContext *context,
                                             vk::SecondaryCommandPool *commandPool,
                                             vk::SecondaryCommandMemoryAllocator *commandsAllocator,
                                             RecyclerT *recycler,
                                             CommandBufferHelperT **commandBufferHelperOut)
{
    return recycler->getCommandBufferHelper(context, commandPool, commandsAllocator,
                                            commandBufferHelperOut);
}

angle::Result Renderer::getOutsideRenderPassCommandBufferHelper(
    vk::ErrorContext *context,
    vk::SecondaryCommandPool *commandPool,
    vk::SecondaryCommandMemoryAllocator *commandsAllocator,
    vk::OutsideRenderPassCommandBufferHelper **commandBufferHelperOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::getOutsideRenderPassCommandBufferHelper");
    return getCommandBufferImpl(context, commandPool, commandsAllocator,
                                &mOutsideRenderPassCommandBufferRecycler, commandBufferHelperOut);
}

angle::Result Renderer::getRenderPassCommandBufferHelper(
    vk::ErrorContext *context,
    vk::SecondaryCommandPool *commandPool,
    vk::SecondaryCommandMemoryAllocator *commandsAllocator,
    vk::RenderPassCommandBufferHelper **commandBufferHelperOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::getRenderPassCommandBufferHelper");
    return getCommandBufferImpl(context, commandPool, commandsAllocator,
                                &mRenderPassCommandBufferRecycler, commandBufferHelperOut);
}

void Renderer::recycleOutsideRenderPassCommandBufferHelper(
    vk::OutsideRenderPassCommandBufferHelper **commandBuffer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::recycleOutsideRenderPassCommandBufferHelper");
    mOutsideRenderPassCommandBufferRecycler.recycleCommandBufferHelper(commandBuffer);
}

void Renderer::recycleRenderPassCommandBufferHelper(
    vk::RenderPassCommandBufferHelper **commandBuffer)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "Renderer::recycleRenderPassCommandBufferHelper");
    mRenderPassCommandBufferRecycler.recycleCommandBufferHelper(commandBuffer);
}

void Renderer::logCacheStats() const
{
    if (!vk::kOutputCumulativePerfCounters)
    {
        return;
    }

    std::unique_lock<angle::SimpleMutex> localLock(mCacheStatsMutex);

    int cacheType = 0;
    INFO() << "Vulkan object cache hit ratios: ";
    for (const CacheStats &stats : mVulkanCacheStats)
    {
        INFO() << "    CacheType " << cacheType++ << ": " << stats.getHitRatio();
    }
}

angle::Result Renderer::getFormatDescriptorCountForVkFormat(vk::ErrorContext *context,
                                                            VkFormat format,
                                                            uint32_t *descriptorCountOut)
{
    if (mVkFormatDescriptorCountMap.count(format) == 0)
    {
        // Query device for descriptor count with basic values for most of
        // VkPhysicalDeviceImageFormatInfo2 members.
        VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
        imageFormatInfo.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
        imageFormatInfo.format = format;
        imageFormatInfo.type   = VK_IMAGE_TYPE_2D;
        imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageFormatInfo.usage  = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageFormatInfo.flags  = 0;

        VkImageFormatProperties imageFormatProperties                            = {};
        VkSamplerYcbcrConversionImageFormatProperties ycbcrImageFormatProperties = {};
        ycbcrImageFormatProperties.sType =
            VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES;

        VkImageFormatProperties2 imageFormatProperties2 = {};
        imageFormatProperties2.sType                 = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        imageFormatProperties2.pNext                 = &ycbcrImageFormatProperties;
        imageFormatProperties2.imageFormatProperties = imageFormatProperties;

        ANGLE_VK_TRY(context, vkGetPhysicalDeviceImageFormatProperties2(
                                  mPhysicalDevice, &imageFormatInfo, &imageFormatProperties2));

        mVkFormatDescriptorCountMap[format] =
            ycbcrImageFormatProperties.combinedImageSamplerDescriptorCount;
    }

    ASSERT(descriptorCountOut);
    *descriptorCountOut = mVkFormatDescriptorCountMap[format];
    return angle::Result::Continue;
}

angle::Result Renderer::getFormatDescriptorCountForExternalFormat(vk::ErrorContext *context,
                                                                  uint64_t format,
                                                                  uint32_t *descriptorCountOut)
{
    ASSERT(descriptorCountOut);

    // TODO: need to query for external formats as well once spec is fixed.
    // http://anglebug.com/42264669
    ANGLE_VK_CHECK(context, getFeatures().useMultipleDescriptorsForExternalFormats.enabled,
                   VK_ERROR_INCOMPATIBLE_DRIVER);

    // Vulkan spec has a gap in that there is no mechanism available to query the immutable
    // sampler descriptor count of an external format. For now, return a default value.
    constexpr uint32_t kExternalFormatDefaultDescriptorCount = 4;
    *descriptorCountOut = kExternalFormatDefaultDescriptorCount;
    return angle::Result::Continue;
}

void Renderer::onAllocateHandle(vk::HandleType handleType)
{
    std::unique_lock<angle::SimpleMutex> localLock(mActiveHandleCountsMutex);
    mActiveHandleCounts.onAllocate(handleType);
}

void Renderer::onDeallocateHandle(vk::HandleType handleType, uint32_t count)
{
    std::unique_lock<angle::SimpleMutex> localLock(mActiveHandleCountsMutex);
    mActiveHandleCounts.onDeallocate(handleType, count);
}

VkDeviceSize Renderer::getPreferedBufferBlockSize(uint32_t memoryTypeIndex) const
{
    // Try not to exceed 1/64 of heap size to begin with.
    const VkDeviceSize heapSize = getMemoryProperties().getHeapSizeForMemoryType(memoryTypeIndex);
    return std::min(heapSize / 64, mPreferredLargeHeapBlockSize);
}

angle::Result Renderer::allocateScopedQueueSerialIndex(vk::ScopedQueueSerialIndex *indexOut)
{
    SerialIndex index;
    ANGLE_TRY(allocateQueueSerialIndex(&index));
    indexOut->init(index, &mQueueSerialIndexAllocator);
    return angle::Result::Continue;
}

angle::Result Renderer::allocateQueueSerialIndex(SerialIndex *serialIndexOut)
{
    *serialIndexOut = mQueueSerialIndexAllocator.allocate();
    if (*serialIndexOut == kInvalidQueueSerialIndex)
    {
        return angle::Result::Stop;
    }
    return angle::Result::Continue;
}

void Renderer::releaseQueueSerialIndex(SerialIndex index)
{
    mQueueSerialIndexAllocator.release(index);
}

angle::Result Renderer::cleanupSomeGarbage(ErrorContext *context, bool *anyGarbageCleanedOut)
{
    return mCommandQueue.cleanupSomeGarbage(context, 0, anyGarbageCleanedOut);
}

// static
const char *Renderer::GetVulkanObjectTypeName(VkObjectType type)
{
    return GetVkObjectTypeName(type);
}

ImageMemorySuballocator::ImageMemorySuballocator() {}
ImageMemorySuballocator::~ImageMemorySuballocator() {}

void ImageMemorySuballocator::destroy(Renderer *renderer) {}

VkResult ImageMemorySuballocator::allocateAndBindMemory(
    ErrorContext *context,
    Image *image,
    const VkImageCreateInfo *imageCreateInfo,
    VkMemoryPropertyFlags requiredFlags,
    VkMemoryPropertyFlags preferredFlags,
    const VkMemoryRequirements *memoryRequirements,
    const bool allocateDedicatedMemory,
    MemoryAllocationType memoryAllocationType,
    Allocation *allocationOut,
    VkMemoryPropertyFlags *memoryFlagsOut,
    uint32_t *memoryTypeIndexOut,
    VkDeviceSize *sizeOut)
{
    ASSERT(image && image->valid());
    ASSERT(allocationOut && !allocationOut->valid());
    Renderer *renderer         = context->getRenderer();
    const Allocator &allocator = renderer->getAllocator();

    // The required size must not be greater than the maximum allocation size allowed by the driver.
    if (memoryRequirements->size > renderer->getMaxMemoryAllocationSize())
    {
        renderer->getMemoryAllocationTracker()->onExceedingMaxMemoryAllocationSize(
            memoryRequirements->size);
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    // Avoid device-local and host-visible combinations if possible. Here, "preferredFlags" is
    // expected to be the same as "requiredFlags" except in the device-local bit.
    ASSERT((preferredFlags & ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
           (requiredFlags & ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

    uint32_t memoryTypeBits = memoryRequirements->memoryTypeBits;
    if ((requiredFlags & preferredFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
    {
        memoryTypeBits = GetMemoryTypeBitsExcludingHostVisible(renderer, preferredFlags,
                                                               memoryRequirements->memoryTypeBits);
    }

    // Allocate and bind memory for the image. Try allocating on the device first.
    VkResult result = vma::AllocateAndBindMemoryForImage(
        allocator.getHandle(), &image->mHandle, requiredFlags, preferredFlags, memoryTypeBits,
        allocateDedicatedMemory, &allocationOut->mHandle, memoryTypeIndexOut, sizeOut);

    // We need to get the property flags of the allocated memory if successful.
    if (result == VK_SUCCESS)
    {
        *memoryFlagsOut =
            renderer->getMemoryProperties().getMemoryType(*memoryTypeIndexOut).propertyFlags;

        renderer->onMemoryAlloc(memoryAllocationType, *sizeOut, *memoryTypeIndexOut,
                                allocationOut->getHandle());
    }
    return result;
}

VkResult ImageMemorySuballocator::mapMemoryAndInitWithNonZeroValue(Renderer *renderer,
                                                                   Allocation *allocation,
                                                                   VkDeviceSize size,
                                                                   int value,
                                                                   VkMemoryPropertyFlags flags)
{
    ASSERT(allocation && allocation->valid());
    const Allocator &allocator = renderer->getAllocator();

    void *mappedMemoryData;
    VkResult result = vma::MapMemory(allocator.getHandle(), allocation->mHandle, &mappedMemoryData);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    memset(mappedMemoryData, value, static_cast<size_t>(size));
    vma::UnmapMemory(allocator.getHandle(), allocation->mHandle);

    // If the memory type is not host coherent, we perform an explicit flush.
    if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
        vma::FlushAllocation(allocator.getHandle(), allocation->mHandle, 0, VK_WHOLE_SIZE);
    }

    return VK_SUCCESS;
}

bool ImageMemorySuballocator::needsDedicatedMemory(VkDeviceSize size) const
{
    return size >= kImageSizeThresholdForDedicatedMemoryAllocation;
}

}  // namespace vk
}  // namespace rx
