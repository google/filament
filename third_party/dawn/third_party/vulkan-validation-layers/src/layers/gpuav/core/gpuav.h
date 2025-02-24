/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "gpuav/descriptor_validation/gpuav_descriptor_set.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/instrumentation/gpuav_shader_instrumentor.h"

#include <memory>

namespace chassis {
struct ShaderObject;
}  // namespace chassis

namespace gpuav {
class Buffer;
class BufferView;
class CommandBuffer;
class ImageView;
class Queue;
class Sampler;
class DescriptorSet;
}  // namespace gpuav

VALSTATETRACK_DERIVED_STATE_OBJECT(VkBuffer, gpuav::Buffer, vvl::Buffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkBufferView, gpuav::BufferView, vvl::BufferView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkCommandBuffer, gpuav::CommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkDescriptorSet, gpuav::DescriptorSet, vvl::DescriptorSet)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkImageView, gpuav::ImageView, vvl::ImageView)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkSampler, gpuav::Sampler, vvl::Sampler)
VALSTATETRACK_DERIVED_STATE_OBJECT(VkQueue, gpuav::Queue, vvl::Queue)

namespace gpuav {

class Instance : public vvl::Instance {
    using BaseClass = vvl::Instance;

  public:
    Instance(vvl::dispatch::Instance* dispatch) : BaseClass(dispatch, LayerObjectTypeGpuAssisted) {}

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) final;
    void ReserveBindingSlot(VkPhysicalDevice physicalDevice, VkPhysicalDeviceLimits& limits, const Location& loc);
    void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                                   VkPhysicalDeviceProperties* pPhysicalDeviceProperties,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                    VkPhysicalDeviceProperties2* pPhysicalDeviceProperties2,
                                                    const RecordObject& record_obj) final;
    void InternalWarning(LogObjectList objlist, const Location& loc, const char* const specific_message) const;
    bool timeline_khr_{false};
};

class Validator : public GpuShaderInstrumentor {
    using BaseClass = GpuShaderInstrumentor;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    Validator(vvl::dispatch::Device* dev, Instance* instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeGpuAssisted), indices_buffer_(*this) {}

    // gpuav_setup.cpp
    // -------------
  public:
    std::shared_ptr<vvl::Buffer> CreateBufferState(VkBuffer handle, const VkBufferCreateInfo* create_info) final;
    std::shared_ptr<vvl::BufferView> CreateBufferViewState(const std::shared_ptr<vvl::Buffer>& buffer, VkBufferView handle,
                                                           const VkBufferViewCreateInfo* create_info,
                                                           VkFormatFeatureFlags2 format_features) final;
    std::shared_ptr<vvl::ImageView> CreateImageViewState(const std::shared_ptr<vvl::Image>& image_state, VkImageView handle,
                                                         const VkImageViewCreateInfo* create_info,
                                                         VkFormatFeatureFlags2 format_features,
                                                         const VkFilterCubicImageViewImageFormatPropertiesEXT& cubic_props) final;
    std::shared_ptr<vvl::Sampler> CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo* create_info) final;
    std::shared_ptr<vvl::AccelerationStructureKHR> CreateAccelerationStructureState(
        VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR* create_info,
        std::shared_ptr<vvl::Buffer>&& buf_state) final;
    std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer handle,
                                                             const VkCommandBufferAllocateInfo* allocate_info,
                                                             const vvl::CommandPool* pool) final;
    std::shared_ptr<vvl::DescriptorSet> CreateDescriptorSet(VkDescriptorSet handle, vvl::DescriptorPool* pool,
                                                            const std::shared_ptr<vvl::DescriptorSetLayout const>& layout,
                                                            uint32_t variable_count) final;
    std::shared_ptr<vvl::Queue> CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                            VkDeviceQueueCreateFlags flags,
                                            const VkQueueFamilyProperties& queueFamilyProperties) override;

    void PostCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) final;

    void InternalVmaError(LogObjectList objlist, const Location& loc, const char* const specific_message) const;
    VkDeviceAddress GetBufferDeviceAddressHelper(VkBuffer buffer) const;

  private:
    void InitSettings(const Location& loc);

    // gpuav_record.cpp
    // --------------
  public:
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) final;
    void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkBuffer* pBuffer, const RecordObject& record_obj, chassis::CreateBuffer& chassis_state) final;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) final;

    void RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         const VkSubpassContents contents);
    void RecordCmdEndRenderPassLayouts(vvl::CommandBuffer& cb_state);
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject&) final;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) final;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject&) final;

    void RecordCmdNextSubpassLayouts(vvl::CommandBuffer& cb_state, VkSubpassContents contents);
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) final;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) final;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) final;

    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) final;
    void PostCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) final;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                       const RecordObject& record_obj) final;
    void PostCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                              const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                              const RecordObject& record_obj) final;
    void PostCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                 const RecordObject& record_obj) final;
    void PostCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                                             const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                             const uint32_t* pDynamicOffsets, const RecordObject& record_obj) final;
    void PreCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject&) final;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject&) final;
    void PreCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                            const RecordObject& record_obj) final;
    void PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                               const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                               const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer,
                                                                 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                 uint32_t set, const RecordObject& record_obj) final;
    void PreCallRecordCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo,
        const RecordObject& record_obj) final;
    void PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                              uint32_t firstInstance, const RecordObject& record_obj) final;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                      uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                      const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                       const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                     uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                     const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                      const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                             const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                             uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                             const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                              const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                      uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                             uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                            VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                            uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                  VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                  uint32_t vertexStride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance,
                                                   VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                   uint32_t vertexStride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                   uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                         const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                          const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                           uint32_t groupCountZ, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) final;
    void PostCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                  const RecordObject& record_obj) final;
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                   const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                          const RecordObject& record_obj) final;
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                      uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                      const RecordObject& record_obj) final;
    void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                       uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                       const RecordObject& record_obj) final;
    void PreCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                         uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                         const RecordObject& record_obj) final;
    void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY,
                                          uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ,
                                          const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                     VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                     VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                     VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                     VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                     VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                     uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) final;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) final;

    void PreCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                      const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                      const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                      uint32_t height, uint32_t depth, const RecordObject& record_obj) final;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                       const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                       uint32_t height, uint32_t depth, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                              const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                              const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                              VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) final;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) final;
    void PreCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                               const RecordObject& record_obj) final;
    void PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                const RecordObject& record_obj) final;
    void PreCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                     const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                     const RecordObject& record_obj) final;
    void PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                      const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                      const RecordObject& record_obj) final;

    // gpuav_image_layout.cpp
    // --------------------

    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) final;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject&) final;

    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges, const RecordObject&) final;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges, const RecordObject&) final;
    void PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                          const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects,
                                          const RecordObject&) final;
    void PostCallRecordTransitionImageLayout(VkDevice device, uint32_t transitionCount,
                                             const VkHostImageLayoutTransitionInfo* pTransitions,
                                             const RecordObject& record_obj) final;
    void PostCallRecordTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount,
                                                const VkHostImageLayoutTransitionInfoEXT* pTransitions,
                                                const RecordObject& record_obj) final;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                   const RecordObject&) final;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo,
                                       const RecordObject&) final;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject&) final;

    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) final;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                               const RecordObject&) final;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject&) final;

    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo2KHR,
                                               const RecordObject&) final;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject&) final;

    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                   const RecordObject&) final;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                       const RecordObject&) final;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                    const RecordObject&) final;

    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) final;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) final;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) final;

    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                    const RecordObject&) final;
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos, const RecordObject&) final;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject&) final;

    void PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                         VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                         uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                         uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                         uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                         const RecordObject&) final;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                             const RecordObject&) final;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject&) final;
    bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state, const Location& image_loc,
                                  const char* vuid, const char* more_message = "") const final;
    bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                 const char* vuid, const char* more_message = "") const final;
    bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state, const Location& buffer_loc,
                                   const char* vuid, const char* more_message = "") const final;

    bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::ImageView& image_view_state,
                           VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                           bool* error) const final;

  public:
    std::optional<DescriptorHeap> desc_heap_{};  // optional only to defer construction
    vko::SharedResourcesCache shared_resources_manager;

    PFN_vkSetDeviceLoaderData vk_set_device_loader_data_;

    VmaAllocator vma_allocator_ = {};
    VmaPool output_buffer_pool_ = VK_NULL_HANDLE;
    std::unique_ptr<vko::DescriptorSetManager> desc_set_manager_;

    vko::Buffer indices_buffer_;
    unsigned int indices_buffer_alignment_ = 0;

  private:
    std::string instrumented_shader_cache_path_{};

    // Make sure we call the right versions of any timeline semaphore functions.
    bool timeline_khr_{false};
};

}  // namespace gpuav
