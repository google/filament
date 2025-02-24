/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "containers/custom_containers.h"
#include "error_message/error_location.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/state_tracker.h"
#include "gpuav/spirv/interface.h"

#include <vector>

// There is a spirv::Instruction used for normal validation.
// There is a gpuav::spirv::Instruction that is ONLY intended for shader instrumentation (designed so we can build the shader
// instrumentation as a seperate library). For logging GPU-AV will want to make use of the normal validaiton instruction class, just
// alias it with "Instruction" as that name shouldn't collide with anything.
using Instruction = ::spirv::Instruction;

namespace vvl {
struct LabelCommand;
}

namespace chassis {
struct ShaderInstrumentationMetadata;
struct ShaderObjectInstrumentationData;
}  // namespace chassis

namespace gpuav {
class Validator;

// There are 3 ways to have a null VkShaderModule
// 1. Use GPL for something like Vertex Input which won't have a shader
// 2. Use Shader Objects
// 3. Use VK_KHR_maintenance5 and inline your VkShaderModuleCreateInfo via VkPipelineShaderStageCreateInfo::pNext
//
// The first is handled because you have to link it in the end, but we need a way to differentiate 2 and 3
static const VkShaderModule kPipelineStageInfoHandle = CastFromUint64<VkShaderModule>(0xEEEEEEEEEEEEEEEE);

// GPU Info shows 99% of devices have a maxBoundDescriptorSets of 32 or less, but some are 2^30
// We set a reasonable max because we have to pad the pipeline layout with dummy descriptor set layouts.
static const uint32_t kMaxAdjustedBoundDescriptorSet = 33;

struct InstrumentedShader {
    VkPipeline pipeline;
    VkShaderModule shader_module;
    VkShaderEXT shader_object;
    std::vector<uint32_t> instrumented_spirv;
};

// Historically this was an common interface to both GPU-AV and DebugPrintf before the were merged together.
// We still keep this as encapsulates the complex code around shader instrumentation.
// Handles shader instrumentation (reserve a descriptor slot, create descriptor
// sets, pipeline layout, hook into pipeline creation, etc...)
class GpuShaderInstrumentor : public vvl::Device {
    using BaseClass = vvl::Device;

  public:
    GpuShaderInstrumentor(vvl::dispatch::Device *dev, vvl::Instance *instance, LayerObjectTypeId type)
        : BaseClass(dev, instance, type) {}

    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    void PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) override;
    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                    const RecordObject &record_obj) override;

    bool ValidateCmdWaitEvents(VkCommandBuffer command_buffer, VkPipelineStageFlags2 src_stage_mask, const Location &loc) const;
    bool PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                      const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                          const VkDependencyInfoKHR *pDependencyInfos, const ErrorObject &error_obj) const override;
    bool PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                       const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const override;
    void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                           const RecordObject &record_obj, chassis::CreatePipelineLayout &chassis_state) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                            const RecordObject &record_obj) override;

    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                          const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) override;
    void PreCallRecordShaderObjectInstrumentation(VkShaderCreateInfoEXT &create_info, const Location &create_info_loc,
                                                  chassis::ShaderObjectInstrumentationData &shader_instrumentation_data);
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                       const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                       const RecordObject &record_obj, chassis::ShaderObject &chassis_state) override;
    void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                        const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                        const RecordObject &record_obj, chassis::ShaderObject &chassis_state) override;
    void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) override;

    void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, PipelineStates &pipeline_states,
                                              chassis::CreateGraphicsPipelines &chassis_state) override;
    void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                             const VkComputePipelineCreateInfo *pCreateInfos,
                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                             const RecordObject &record_obj, PipelineStates &pipeline_states,
                                             chassis::CreateComputePipelines &chassis_state) override;
    void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                  chassis::CreateRayTracingPipelinesNV &chassis_state) override;
    void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                   VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateRayTracingPipelinesKHR &chassis_state) override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                               const RecordObject &record_obj, PipelineStates &pipeline_states,
                                               chassis::CreateGraphicsPipelines &chassis_state) override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo *pCreateInfos,
                                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                              const RecordObject &record_obj, PipelineStates &pipeline_states,
                                              chassis::CreateComputePipelines &chassis_state) override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateRayTracingPipelinesNV &chassis_state) override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) override;
    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                      const RecordObject &record_obj) override;

    void InternalError(LogObjectList objlist, const Location &loc, const char *const specific_message) const;
    void InternalWarning(LogObjectList objlist, const Location &loc, const char *const specific_message) const;
    void InternalInfo(LogObjectList objlist, const Location &loc, const char *const specific_message) const;

    bool IsSelectiveInstrumentationEnabled(const void *pNext);

    std::string GenerateDebugInfoMessage(VkCommandBuffer commandBuffer, uint32_t stage_id, uint32_t stage_info_0,
                                         uint32_t stage_info_1, uint32_t stage_info_2, uint32_t instruction_position,
                                         const InstrumentedShader *instrumented_shader, uint32_t shader_id,
                                         VkPipelineBindPoint pipeline_bind_point, uint32_t operation_index) const;

  protected:
    bool NeedPipelineCreationShaderInstrumentation(vvl::Pipeline &pipeline_state, const Location &loc);

    // When instrumenting, we need information about the array of VkDescriptorSetLayouts. The core issue is that for pipelines, we
    // might have to merge 2 pipeline layouts together (because of GPL) and therefore both ShaderObject and PipelineLayout state
    // objects don't have a single way to describe their VkDescriptorSetLayouts. If there are multiple shaders, we also want to only
    // build this information once. This struct is designed to be filled in from both Pipeline and ShaderObject and then passed down
    // to the SPIR-V Instrumentation, and afterwards we don't need to save it.
    struct InstrumentationDescriptorSetLayouts {
        bool has_bindless_descriptors = false;
        // < set , [ bindings ] >
        std::vector<std::vector<spirv::BindingLayout>> set_index_to_bindings_layout_lut;
    };
    void BuildDescriptorSetLayoutInfo(const vvl::Pipeline &pipeline_state,
                                      InstrumentationDescriptorSetLayouts &out_instrumentation_dsl);
    void BuildDescriptorSetLayoutInfo(const VkShaderCreateInfoEXT &create_info,
                                      InstrumentationDescriptorSetLayouts &out_instrumentation_dsl);
    void BuildDescriptorSetLayoutInfo(const vvl::DescriptorSetLayout &set_layout_state, const uint32_t set_layout_index,
                                      InstrumentationDescriptorSetLayouts &out_instrumentation_dsl);

    template <typename SafeCreateInfo>
    [[nodiscard]] bool PreCallRecordPipelineCreationShaderInstrumentation(
        const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state, SafeCreateInfo &modified_pipeline_ci,
        const Location &loc, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);
    void PostCallRecordPipelineCreationShaderInstrumentation(
        vvl::Pipeline &pipeline_state, std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);

    // We have GPL variations for graphics as they defer instrumentation until linking
    [[nodiscard]] bool PreCallRecordPipelineCreationShaderInstrumentationGPL(
        const VkAllocationCallbacks *pAllocator, vvl::Pipeline &pipeline_state,
        vku::safe_VkGraphicsPipelineCreateInfo &modified_pipeline_ci, const Location &loc,
        std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);
    void PostCallRecordPipelineCreationShaderInstrumentationGPL(
        vvl::Pipeline &pipeline_state, const VkAllocationCallbacks *pAllocator,
        std::vector<chassis::ShaderInstrumentationMetadata> &shader_instrumentation_metadata);

    // GPU-AV and DebugPrint are using the same way to do the actual shader instrumentation logic
    // Returns if shader was instrumented successfully or not
    bool InstrumentShader(const vvl::span<const uint32_t> &input_spirv, uint32_t unique_shader_id,
                          const InstrumentationDescriptorSetLayouts &instrumentation_dsl, const Location &loc,
                          std::vector<uint32_t> &out_instrumented_spirv);

  public:
    VkDescriptorSetLayout GetInstrumentationDescriptorSetLayout() { return instrumentation_desc_layout_; }
    VkPipelineLayout GetInstrumentationPipelineLayout() { return instrumentation_pipeline_layout_; }

    // When aborting we will disconnect all future chassis calls.
    // If we are deep into a call stack, we can use this to return up to the chassis call.
    // It should only be used after calls that might abort, not to be used for guarding a function (unless a case is found that make
    // sense too)
    mutable bool aborted_ = false;

    std::atomic<uint32_t> unique_shader_module_id_ = 1;  // zero represents no shader module found
    // The descriptor slot we will be injecting our error buffer into
    uint32_t instrumentation_desc_set_bind_index_ = 0;
    // This is a layout used to "pad" a pipeline layout to fill in any gaps to the selected bind index
    VkDescriptorSetLayout dummy_desc_layout_ = VK_NULL_HANDLE;
    vvl::concurrent_unordered_map<uint32_t, InstrumentedShader> instrumented_shaders_map_;
    std::vector<VkDescriptorSetLayoutBinding> instrumentation_bindings_;

    std::vector<spirv::InternalOnlyDebugPrintf> intenral_only_debug_printf_;

  private:
    void Cleanup();
    // These are objects used to inject our descriptor set into the command buffer
    VkDescriptorSetLayout instrumentation_desc_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout instrumentation_pipeline_layout_ = VK_NULL_HANDLE;

    // Pass select_instrumented_shaders from vkCreateShaderModule to CreatePipeline time
    vvl::unordered_set<VkShaderModule> selected_instrumented_shaders;
};

}  // namespace gpuav
