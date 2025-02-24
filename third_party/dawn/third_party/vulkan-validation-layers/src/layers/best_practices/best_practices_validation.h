/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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
 */

#pragma once

#include "chassis/validation_object.h"
#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include <string>
#include <deque>
#include <chrono>
#include <set>

static const uint32_t kMemoryObjectWarningLimit = 250;

// Maximum number of instanced vertex buffers which should be used
static const uint32_t kMaxInstancedVertexBuffers = 1;

// Recommended allocation size for vkAllocateMemory
static const VkDeviceSize kMinDeviceAllocationSize = 256 * 1024;

// If a buffer or image is allocated and it consumes an entire VkDeviceMemory, it should at least be this large.
// This is slightly different from minDeviceAllocationSize since the 256K buffer can still be sensibly
// suballocated from. If we consume an entire allocation with one image or buffer, it should at least be for a
// very large allocation.
static const VkDeviceSize kMinDedicatedAllocationSize = 1024 * 1024;

// AMD best practices
// Note: These are initial ball park numbers for good performance
// We expect to adjust them as we get more data on layer usage
// Avoid small command buffers
static const uint32_t kMinRecommendedCommandBufferSizeAMD = 10;
// Avoid small secondary command buffers
static const uint32_t kMinRecommendedDrawsInSecondaryCommandBufferSizeAMD = 10;
// Idealy, only 1 fence per frame, so 3 for triple buffering
static const uint32_t kMaxRecommendedFenceObjectsSizeAMD = 3;
// Avoid excessive sempahores
static const uint32_t kMaxRecommendedSemaphoreObjectsSizeAMD = 10;
// Avoid excessive barriers
static const uint32_t kMaxRecommendedBarriersSizeAMD = 500;
// Avoid excessive pipelines
static const uint32_t kMaxRecommendedNumberOfPSOAMD = 5000;
// Unlikely that the user needs all the dynamic states enabled at the same time, and they encur a cost
static const uint32_t kDynamicStatesWarningLimitAMD = 7;
// Too many dynamic descriptor sets can cause a large pipeline layout
static const uint32_t kPipelineLayoutSizeWarningLimitAMD = 13;
// Check that the user is submitting excessivly to a queue
static const uint32_t kNumberOfSubmissionWarningLimitAMD = 20;
// Check that there is enough work per vertex stream change
static const float kVertexStreamToDrawRatioWarningLimitAMD = 0.8f;
// Check that there is enough work per pipeline change
static const float kDrawsPerPipelineRatioWarningLimitAMD = 5.f;
// Check that command buffers are used with an appropriatly sized pool
static const float kCmdBufferToCmdPoolRatioWarningLimitAMD = 0.1f;
// Size for fast descriptor reads on modern NVIDIA devices
static const uint32_t kPipelineLayoutFastDescriptorSpaceNVIDIA = 256;
// Time threshold for flagging allocations that could have been reused
static const auto kAllocateMemoryReuseTimeThresholdNVIDIA = std::chrono::seconds{5};
// Number of switches in tessellation, gemetry, and mesh shader state before signalling a message
static const uint32_t kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA = 4;
// Ratio where the Z-cull direction starts being considered balanced
static const int kZcullDirectionBalanceRatioNVIDIA = 20;
// Maximum number of custom clear colors
static const size_t kMaxRecommendedNumberOfClearColorsNVIDIA = 16;

// How many small indexed drawcalls in a command buffer before a warning is thrown
static const uint32_t kMaxSmallIndexedDrawcalls = 10;

// How many indices make a small indexed drawcall
static const int kSmallIndexedDrawcallIndices = 10;

// Minimum number of vertices/indices to take into account when doing depth pre-pass checks for Arm Mali GPUs
static const int kDepthPrePassMinDrawCountArm = 500;

// Minimum, number of draw calls in order to trigger depth pre-pass warnings for Arm Mali GPUs
static const int kDepthPrePassNumDrawCallsArm = 20;

// Maximum sample count for full throughput on Mali GPUs
static const VkSampleCountFlagBits kMaxEfficientSamplesArm = VK_SAMPLE_COUNT_4_BIT;

// On Arm Mali architectures, it's generally best to align work group dimensions to 4.
static const uint32_t kThreadGroupDispatchCountAlignmentArm = 4;

// Maximum number of threads which can efficiently be part of a compute workgroup when using thread group barriers.
static const uint32_t kMaxEfficientWorkGroupThreadCountArm = 64;

// Minimum number of vertices/indices a draw needs to have before considering it in depth prepass warnings on PowerVR
static const int kDepthPrePassMinDrawCountIMG = 300;

// Minimum, number of draw calls matching the above criteria before triggerring a depth prepass warning on PowerVR
static const int kDepthPrePassNumDrawCallsIMG = 10;

// Maximum sample count on PowerVR before showing a warning
static const VkSampleCountFlagBits kMaxEfficientSamplesImg = VK_SAMPLE_COUNT_4_BIT;

enum class DeprecationReason {
    Empty = 0,
    Promoted,
    Obsoleted,
    Deprecated,
};

struct DeprecationData {
    DeprecationReason reason;
    vvl::Requirement target;
};

DeprecationData GetDeprecatedData(vvl::Extension extension);
std::string GetSpecialUse(vvl::Extension extension);

struct SpecialUseVUIDs {
    const char* cadsupport;
    const char* d3demulation;
    const char* devtools;
    const char* debugging;
    const char* glemulation;
};

typedef enum {
    kBPVendorArm = 0x00000001,
    kBPVendorAMD = 0x00000002,
    kBPVendorIMG = 0x00000004,
    kBPVendorNVIDIA = 0x00000008,
} BPVendorFlagBits;
typedef VkFlags BPVendorFlags;

enum CALL_STATE {
    UNCALLED,       // Function has not been called
    QUERY_COUNT,    // Function called once to query a count
    QUERY_DETAILS,  // Function called w/ a count to query details
};

enum IMAGE_SUBRESOURCE_USAGE_BP {
    UNDEFINED,  // If it has never been used
    RENDER_PASS_CLEARED,
    RENDER_PASS_READ_TO_TILE,
    CLEARED,
    DESCRIPTOR_ACCESS,
    RENDER_PASS_STORED,
    RENDER_PASS_DISCARDED,
    BLIT_READ,
    BLIT_WRITE,
    RESOLVE_READ,
    RESOLVE_WRITE,
    COPY_READ,
    COPY_WRITE
};

enum class ZcullDirection {
    Unknown,
    Less,
    Greater,
};

namespace bp_state {
class PhysicalDevice;
class CommandBuffer;
class Swapchain;
class Image;
class DescriptorPool;
class Pipeline;
}  // namespace bp_state

VALSTATETRACK_DERIVED_STATE_OBJECT(VkPhysicalDevice, bp_state::PhysicalDevice, vvl::PhysicalDevice)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, bp_state::CommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSwapchainKHR, bp_state::Swapchain, vvl::Swapchain)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImage, bp_state::Image, vvl::Image)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorPool, bp_state::DescriptorPool, vvl::DescriptorPool)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkPipeline, bp_state::Pipeline, vvl::Pipeline)

namespace bp_state {
template <typename StateObject, typename Handle>
void LogResult(const StateObject& state, Handle handle, const RecordObject& record_obj) {
    if (record_obj.result == VK_SUCCESS) {
        return;
    }
    // Despite being error codes log these results as informational.
    // That is because they are returned frequently during window resizing.
    // They are expected to occur during the normal application lifecycle.
    constexpr std::array common_failure_codes = {VK_ERROR_OUT_OF_DATE_KHR, VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT};
    const auto result_string = string_VkResult(record_obj.result);

    if (record_obj.result > VK_SUCCESS) {
        state.LogVerbose("BestPractices-Verbose-Success-Logging", handle, record_obj.location, "Returned %s.", result_string);
    } else if (IsValueIn(record_obj.result, common_failure_codes)) {
        state.LogInfo("BestPractices-Failure-Result", handle, record_obj.location, "Returned error %s.", result_string);
    } else {
        state.LogWarning("BestPractices-Error-Result", handle, record_obj.location, "Returned error %s.", result_string);
    }
}

const char* VendorSpecificTag(BPVendorFlags vendors);

bool VendorCheckEnabled(const CHECK_ENABLED& enabled, BPVendorFlags vendors);

class Instance : public vvl::Instance {
    using BaseClass = vvl::Instance;

  public:
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    Instance(vvl::dispatch::Instance* dispatch) : BaseClass(dispatch, LayerObjectTypeBestPractices) {}
    std::shared_ptr<vvl::PhysicalDevice> CreatePhysicalDeviceState(VkPhysicalDevice handle) final;

    bool VendorCheckEnabled(BPVendorFlags vendors) const { return bp_state::VendorCheckEnabled(enabled, vendors); }

    bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkInstance* pInstance, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                     const ErrorObject& error_obj) const override;
    bool ValidateDeprecatedExtensions(const Location& loc, vvl::Extension extension, APIVersion version) const;

    bool ValidateSpecialUseExtensions(const Location& loc, vvl::Extension extension) const;

    bool ValidateGetPhysicalDeviceDisplayPlanePropertiesKHRQuery(VkPhysicalDevice physicalDevice, const Location& loc) const;
    bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                            uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                       VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                        VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties* pQueueFamilyProperties,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                   const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                           const ErrorObject& error_obj) const override;
    bool ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const vvl::PhysicalDevice& pd_state,
                                                              uint32_t requested_queue_family_property_count,
                                                              const CALL_STATE call_state, const Location& loc) const;
    void CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(CALL_STATE& call_state, bool no_pointer);
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties,
                                                              const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                               const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                  const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                 const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                  const RecordObject& record_obj) override;

    void PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                     const RecordObject& record_obj) override;

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                     const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                      VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                      const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                      const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                     const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                                const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                 uint32_t* pSurfaceFormatCount,
                                                                 VkSurfaceFormat2KHR* pSurfaceFormats,
                                                                 const RecordObject& record_obj);

    void ManualPostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkDisplayPlanePropertiesKHR* pProperties,
                                                                        const RecordObject& record_obj);

// Include code-generated functions
#include "generated/best_practices_instance_methods.h"
};
}  // namespace bp_state

class BestPractices : public vvl::Device {
    using BaseClass = vvl::Device;

  public:
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

    BestPractices(vvl::dispatch::Device* dev, bp_state::Instance* instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeBestPractices) {}

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    std::string GetAPIVersionName(uint32_t version) const;

    bool ValidateCmdDrawType(VkCommandBuffer cmd_buffer, const Location& loc) const;

    bool ValidateCmdDispatchType(VkCommandBuffer cmd_buffer, const Location& loc) const;

    bool ValidatePushConstants(VkCommandBuffer cmd_buffer, const Location& loc) const;

    void RecordCmdDrawType(bp_state::CommandBuffer& cb_state, uint32_t draw_count);

    bool PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                     const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkImage* pImage, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                  const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                  const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
    bool ValidateAttachments(const VkRenderPassCreateInfo2* rpci, uint32_t attachment_count, const VkImageView* attachments,
                             const Location& loc) const;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                               vvl::AllocateDescriptorSetsData& ads_state_data) const override;
    void ManualPostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                    VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                                    vvl::AllocateDescriptorSetsData& ads_state);
    void PostCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                          const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) override;
    bool PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                       const ErrorObject& error_obj) const override;
    void PreCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                     const RecordObject& record_obj) override;
    void ManualPostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                            const RecordObject& record_obj);
    bool ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, const Location& loc) const;
    bool PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                             const ErrorObject& error_obj) const override;
    bool ValidateBindImageMemory(VkImage image, VkDeviceMemory memory, const Location& loc) const;
    bool PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                            const ErrorObject& error_obj) const override;
    void PostCallRecordSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                  const RecordObject& record_obj) override;
    bool PreCallValidateGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                             uint32_t* pMemoryRequirementsCount,
                                                             VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                  uint32_t bindSessionMemoryInfoCount,
                                                  const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                               VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                 const RecordObject& record_obj) override;
    bool PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator,
                                   const ErrorObject& error_obj) const override;
    bool ValidateMultisampledBlendingArm(const VkGraphicsPipelineCreateInfo& create_info, const Location& create_info_loc) const;

    bool ValidateCreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info, const vvl::Pipeline& pipeline,
                                        const Location create_info_loc) const;
    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                chassis::CreateGraphicsPipelines& chassis_state) const override;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                               chassis::CreateComputePipelines& chassis_state) const override;

    bool ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& create_info, const Location& create_info_loc) const;

    bool ValidateCreateComputePipelineAmd(const VkComputePipelineCreateInfo& create_info, const Location& create_info_loc) const;

    bool CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc, VkPipelineStageFlags flags) const;
    bool CheckPipelineStageFlags(const LogObjectList& objlist, const Location& loc, VkPipelineStageFlags2KHR flags) const;
    bool CheckDependencyInfo(const LogObjectList& objlist, const Location& dep_loc, const VkDependencyInfo& dep_info) const;
    bool PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                     const ErrorObject& error_obj) const override;
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                         const RecordObject& record_obj) override;
    bool PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                           const ErrorObject& error_obj) const override;
    bool CheckEventSignalingState(const bp_state::CommandBuffer& command_buffer, VkEvent event, const Location& cb_loc) const;
    void RecordCmdSetEvent(bp_state::CommandBuffer& command_buffer, VkEvent event);
    void RecordCmdResetEvent(bp_state::CommandBuffer& command_buffer, VkEvent event);
    bool PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const ErrorObject& error_obj) const override;
    void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                  const RecordObject& record_obj) override;
    bool PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                     const ErrorObject& error_obj) const override;
    void PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo,
                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                   const RecordObject& record_obj) override;
    bool PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const ErrorObject& error_obj) const override;
    void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const RecordObject& record_obj) override;
    bool PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                       const ErrorObject& error_obj) const override;
    void PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                     const RecordObject& record_obj) override;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                      const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                          const VkDependencyInfoKHR* pDependencyInfos, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                       const VkDependencyInfo* pDependencyInfos, const ErrorObject& error_obj) const override;
    bool ValidateAccessLayoutCombination(const Location& loc, VkImage image, VkAccessFlags2 access, VkImageLayout layout,
                                         VkImageAspectFlags aspect) const;
    bool ValidateImageMemoryBarrier(const Location& loc, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                    VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask,
                                    VkImageAspectFlags aspectMask) const;
    bool PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                           VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                           uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                           uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                           uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                            const ErrorObject& error_obj) const override;

    template <typename ImageMemoryBarrier>
    bool ValidateCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier,
                                                const Location& loc) const;

    bool PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                          VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                              VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                           VkQueryPool queryPool, uint32_t query, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                            size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                            const ErrorObject& error_obj) const override;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                const RecordObject& record_obj) override;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                    const Location& loc) const;
    bool ValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo, const Location& loc) const;

    void PostCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                            const RecordObject& record_obj) override;

    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void RecordCmdNextSubpass(bp_state::CommandBuffer& cb_state);

    void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                        uint32_t offset, uint32_t size, const void* pValues,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR* pPushConstantsInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR* pSubpassEndInfo,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;

    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                               const VkSubpassBeginInfo* pSubpassBeginInfo,
                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                             const ErrorObject& error_obj) const override;
    void ValidateBoundDescriptorSets(bp_state::CommandBuffer& commandBuffer, VkPipelineBindPoint bind_point, Func command);
    bool PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;

    void PostRecordCmdBeginRenderPass(bp_state::CommandBuffer& cb_state, const VkRenderPassBeginInfo* pRenderPassBegin);
    void PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          VkSubpassContents contents, const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                              const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                uint32_t firstInstance, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                       uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                       const ErrorObject& error_obj) const override;
    bool ValidateIndexBufferArm(const bp_state::CommandBuffer& cb_state, uint32_t indexCount, uint32_t instanceCount,
                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance, const Location& loc) const;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                      const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                        uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               uint32_t drawCount, uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                    VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                    uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset,
                                                    uint32_t counterOffset, uint32_t vertexStride,
                                                    const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                   VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                   uint32_t vertexStride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                             uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride,
                                                   const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t strCmdDrawMeshTasksIndirectNVide,
                                                  const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                           const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                          const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                               const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                               uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                               const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                              const RecordObject& record_obj) override;
    bool PreCallValidateCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                        uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                        const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                       const RecordObject& record_obj) override;

    bool PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                        uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                           uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                           const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                             const ErrorObject& error_obj) const override;
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           const RecordObject& record_obj) override;
    bool PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                          const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                        const ErrorObject& error_obj) const override;
    void ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                             VkFence fence, const RecordObject& record_obj);
    bool PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                            const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                            const ErrorObject& error_obj) const override;
    bool ValidateCmdResolveImage(VkCommandBuffer command_buffer, VkImage src_image, VkImage dst_image, const Location& loc) const;
    bool PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                        VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                        const VkImageResolve* pRegions, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                         const ErrorObject& error_obj) const override;

    using QueueCallbacks = std::vector<vvl::CommandBuffer::QueueCallback>;

    void QueueValidateImageView(QueueCallbacks& func, Func command, vvl::ImageView* view, IMAGE_SUBRESOURCE_USAGE_BP usage);
    void QueueValidateImage(QueueCallbacks& func, Func command, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceRange& subresource_range);
    void QueueValidateImage(QueueCallbacks& func, Func command, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceLayers& range);
    void QueueValidateImage(QueueCallbacks& func, Func command, std::shared_ptr<bp_state::Image>& state,
                            IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueue(const vvl::Queue& qs, const vvl::CommandBuffer& cbs, Func command, bp_state::Image& state,
                              IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);
    void ValidateImageInQueueArmImg(Func command, const bp_state::Image& image, IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                    IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level);

    void PostCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageResolve* pRegions, const RecordObject& record_obj) override;
    void PostCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                          const VkClearColorValue* pColor, uint32_t rangeCount,
                                          const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;
    void PostCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                 const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                 const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                            VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions,
                                    VkFilter filter, const RecordObject& record_obj) override;
    template <typename RegionType>
    bool ValidateCmdBlitImage(VkCommandBuffer command_buffer, uint32_t region_count, const RegionType* regions,
                              const Location& loc) const;
    bool PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageBlit* pRegions, VkFilter filter, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                      const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                      const ErrorObject& error_obj) const override;
    void ManualPostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo, const RecordObject& record_obj);
    void ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                     chassis::CreateGraphicsPipelines& chassis_state);

    bool PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                            VkFence fence, uint32_t* pImageIndex, const ErrorObject& error_obj) const override;

    void ManualPostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                   VkImage* pSwapchainImages, const RecordObject& record_obj);

    void ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                         const RecordObject& record_obj);

    void ManualPostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj);

    void ManualPostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                               const RecordObject& record_obj);
    void ManualPostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                              const RecordObject& record_obj);

    void ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                    const VkComputePipelineCreateInfo* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                    chassis::CreateComputePipelines& chassis_state);

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                              const RecordObject& record_obj) override;
    template <typename ImageMemoryBarrier>
    void RecordCmdPipelineBarrierImageBarrier(VkCommandBuffer commandBuffer, const ImageMemoryBarrier& barrier);

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                              const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              const RecordObject& record_obj) override;

    bool PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                             const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                             const VkCopyDescriptorSet* pDescriptorCopies,
                                             const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                           const VkClearColorValue* pColor, uint32_t rangeCount,
                                           const VkImageSubresourceRange* pRanges, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                  const VkImageSubresourceRange* pRanges,
                                                  const ErrorObject& error_obj) const override;

    bool PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                             const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                     VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                     const VkImageCopy* pRegions, const ErrorObject& error_obj) const override;

    bool PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                        const ErrorObject& error_obj) const override;

    bool PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                        const ErrorObject& error_obj) const override;

    bool PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkFence* pFence, const ErrorObject& error_obj) const override;

    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                  const RecordObject& record_obj) override;

    void PostCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                           const VkClearAttachment* pClearAttachments, uint32_t rectCount,
                                           const VkClearRect* pRects, const RecordObject& record_obj) override;

    bool PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                           const VkCommandBuffer* pCommandBuffers, const ErrorObject& error_obj) const override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;

    bool PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                        VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                        VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                        VkBuffer scratch, VkDeviceSize scratchOffset,
                                                        const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                  const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                  const uint32_t* pIndirectStrides,
                                                                  const uint32_t* const* ppMaxPrimitiveCounts,
                                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                          const ErrorObject& error_obj) const override;

// Include code-generated functions
#include "generated/best_practices_device_methods.h"
  protected:
    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer handle,
                                                             const VkCommandBufferAllocateInfo* allocate_info,
                                                             const vvl::CommandPool* pool) final;

    std::shared_ptr<vvl::Swapchain> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info, VkSwapchainKHR handle) final;

    std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo* create_info,
                                                 VkFormatFeatureFlags2 features) final;

    std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo* create_info, VkSwapchainKHR swapchain,
                                                 uint32_t swapchain_index, VkFormatFeatureFlags2 features) final;

    std::shared_ptr<vvl::DescriptorPool> CreateDescriptorPoolState(VkDescriptorPool handle,
                                                                   const VkDescriptorPoolCreateInfo* create_info) final;

    std::shared_ptr<vvl::DeviceMemory> CreateDeviceMemoryState(VkDeviceMemory handle, const VkMemoryAllocateInfo* p_alloc_info,
                                                               uint64_t fake_address, const VkMemoryType& memory_type,
                                                               const VkMemoryHeap& memory_heap,
                                                               std::optional<vvl::DedicatedBinding>&& dedicated_binding,
                                                               uint32_t physical_device_count) final;

    std::shared_ptr<vvl::Pipeline> CreateGraphicsPipelineState(
        const VkGraphicsPipelineCreateInfo* create_info, std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
        std::shared_ptr<const vvl::RenderPass>&& render_pass, std::shared_ptr<const vvl::PipelineLayout>&& layout,
        spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) const final;

  private:
    // CacheEntry and PostTransformLRUCacheModel are used on the stack
    struct CacheEntry {
        uint32_t value;
        uint32_t age;
    };

    class PostTransformLRUCacheModel {
      public:
        typedef std::vector<CacheEntry>::iterator cache_iterator;

        void resize(size_t size);

        // Returns true if there was a cache hit - also models LRU behavior which will effect subsequent calls.
        bool query_cache(uint32_t value);

      private:
        std::vector<CacheEntry> _entries = {};
        uint32_t iteration = 0;
    };

    // Check that vendor-specific checks are enabled for at least one of the vendors
    bool VendorCheckEnabled(BPVendorFlags vendors) const { return bp_state::VendorCheckEnabled(enabled, vendors); }
    const char* VendorSpecificTag(BPVendorFlags vendors) const { return bp_state::VendorSpecificTag(vendors); }

    void RecordCmdDrawTypeArm(bp_state::CommandBuffer& cb_state, uint32_t draw_count);
    void RecordCmdDrawTypeNVIDIA(bp_state::CommandBuffer& cb_state);

    // Get BestPractices-specific for the current instance
    bp_state::PhysicalDevice* GetPhysicalDeviceState();
    const bp_state::PhysicalDevice* GetPhysicalDeviceState() const;

    void RecordAttachmentClearAttachments(bp_state::CommandBuffer& cb_state, uint32_t fb_attachment, uint32_t color_attachment,
                                          VkImageAspectFlags aspects, uint32_t rectCount, const VkClearRect* pRects);
    void RecordAttachmentAccess(bp_state::CommandBuffer& cb_state, uint32_t attachment, VkImageAspectFlags aspects);
    bool ClearAttachmentsIsFullClear(const bp_state::CommandBuffer& cb_state, uint32_t rectCount, const VkClearRect* pRects) const;
    bool ValidateClearAttachment(const bp_state::CommandBuffer& cb_state, uint32_t fb_attachment, uint32_t color_attachment,
                                 VkImageAspectFlags aspects, const Location& loc) const;

    bool ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const Location& loc) const;
    void RecordCmdBeginRenderPass(bp_state::CommandBuffer& cb_state, const VkRenderPassBeginInfo* pRenderPassBegin);

    bool ValidateBuildAccelerationStructure(VkCommandBuffer commandBuffer, const Location& loc) const;

    bool ValidateBindMemory(VkDevice device, VkDeviceMemory memory, const Location& loc) const;

    void RecordSetDepthTestState(bp_state::CommandBuffer& cb_state, VkCompareOp new_depth_compare_op, bool new_depth_test_enable);

    void RecordCmdBeginRenderingCommon(bp_state::CommandBuffer& cb_state, const VkRenderPassBeginInfo* pRenderPassBegin,
                                       const VkRenderingInfo* pRenderingInfo);
    void RecordCmdEndRenderingCommon(bp_state::CommandBuffer& cb_state, const vvl::RenderPass& rp_state);

    void RecordBindZcullScope(bp_state::CommandBuffer& cb_state, VkImage depth_attachment,
                              const VkImageSubresourceRange& subresource_range);
    void RecordUnbindZcullScope(bp_state::CommandBuffer& cb_state);
    void RecordResetScopeZcullDirection(bp_state::CommandBuffer& cb_state);
    void RecordResetZcullDirection(bp_state::CommandBuffer& cb_state, VkImage depth_image,
                                   const VkImageSubresourceRange& subresource_range);

    void RecordSetScopeZcullDirection(bp_state::CommandBuffer& cb_state, ZcullDirection mode);
    void RecordSetZcullDirection(bp_state::CommandBuffer& cb_state, VkImage depth_image,
                                 const VkImageSubresourceRange& subresource_range, ZcullDirection mode);

    void RecordZcullDraw(bp_state::CommandBuffer& cb_state);

    bool ValidateZcullScope(const bp_state::CommandBuffer& cb_state, const Location& loc) const;
    bool ValidateZcull(const bp_state::CommandBuffer& cb_state, VkImage image, const VkImageSubresourceRange& subresource_range,
                       const Location& loc) const;

    void RecordClearColor(VkFormat format, const VkClearColorValue& clear_value);
    bool ValidateClearColor(VkCommandBuffer commandBuffer, VkFormat format, const VkClearColorValue& clear_value,
                            const Location& loc) const;

    void PipelineUsedInFrame(VkPipeline pipeline) {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.insert(pipeline);
    }

    void ClearPipelinesUsedInFrame() {
        WriteLockGuard guard(pipeline_lock_);
        pipelines_used_in_frame_.clear();
    }

    bool IsPipelineUsedInFrame(VkPipeline pipeline) const {
        ReadLockGuard guard(pipeline_lock_);
        return pipelines_used_in_frame_.count(pipeline) != 0;
    }

    // AMD tracked
    std::atomic<uint32_t> num_barriers_objects_{0};
    std::atomic<uint32_t> num_pso_{0};
    std::atomic<uint32_t> num_queue_submissions_{0};

    std::atomic<VkPipelineCache> pipeline_cache_{VK_NULL_HANDLE};

    // NVIDIA tracked
    struct MemoryFreeEvent {
        typename std::chrono::high_resolution_clock::time_point time{};
        VkDeviceSize allocation_size = 0;
        uint32_t memory_type_index = 0;
    };
    std::deque<MemoryFreeEvent> memory_free_events_;
    mutable std::shared_mutex memory_free_events_lock_;

    // Can't get vvl::unordered_set to work with std::array
    std::set<std::array<uint32_t, 4>> clear_colors_;
    mutable std::shared_mutex clear_colors_lock_;

    vvl::unordered_set<VkPipeline> pipelines_used_in_frame_;
    mutable std::shared_mutex pipeline_lock_;
};
