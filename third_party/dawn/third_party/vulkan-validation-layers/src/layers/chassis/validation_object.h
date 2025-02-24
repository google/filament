/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
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
#pragma once

#include <atomic>
#include <shared_mutex>
#include <cinttypes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <string_view>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vulkan/utility/vk_safe_struct.hpp>
#include "utils/cast_utils.h"
#include "layer_options.h"
#include "containers/custom_containers.h"
#include "error_message/logging.h"
#include "error_message/error_location.h"
#include "error_message/record_object.h"
#include "error_message/log_message_type.h"
#include "utils/vk_layer_extension_utils.h"
#include "utils/vk_layer_utils.h"
#include "generated/vk_dispatch_table_helper.h"
#include "chassis/dispatch_object.h"
#include "generated/vk_extension_helper.h"
#include "gpuav/core/gpuav_settings.h"
#include "sync/sync_settings.h"

namespace chassis {
struct CreateGraphicsPipelines;
struct CreateComputePipelines;
struct CreateRayTracingPipelinesNV;
struct CreateRayTracingPipelinesKHR;
struct CreateShaderModule;
struct ShaderObject;
struct CreatePipelineLayout;
struct CreateBuffer;
}  // namespace chassis

namespace vvl {
struct AllocateDescriptorSetsData;
class Pipeline;
}  // namespace vvl

// Because of GPL, we currently create our Pipeline state objects before the PreCallValidate
// Each chassis layer will need to track its own state
using PipelineStates = std::vector<std::shared_ptr<vvl::Pipeline>>;

// When testing for a valid value, allow a way to right away return how it might not be valid
enum class ValidValue {
    Valid = 0,
    NotFound,     // example, trying to use a random int for an enum
    NoExtension,  // trying to use a proper value, but the extension is required
};

// Validation Object base classes
namespace vvl::base {

class Instance : public Logger {
  public:
    const APIVersion api_version;
    vvl::dispatch::Instance* dispatch_instance_{};

    DeviceExtensions extensions;
    const GlobalSettings& global_settings;
    GpuAVSettings& gpuav_settings;
    const SyncValSettings& syncval_settings;

    const CHECK_DISABLED& disabled;
    const CHECK_ENABLED& enabled;

    VkInstance instance = VK_NULL_HANDLE;
    const LayerObjectTypeId container_type;

    Instance(vvl::dispatch::Instance* instance, LayerObjectTypeId type_id)
        : Logger(instance->debug_report),
          api_version(instance->api_version),
          dispatch_instance_(instance),
          extensions(instance->extensions),
          global_settings(instance->settings.global_settings),
          gpuav_settings(instance->settings.gpuav_settings),
          syncval_settings(instance->settings.syncval_settings),
          disabled(instance->settings.disabled),
          enabled(instance->settings.enabled),
          instance(instance->instance),
          container_type(type_id) {}
    virtual ~Instance() {}

    // Modify a parameter to CreateDevice
    virtual void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                           const RecordObject& record_obj, vku::safe_VkDeviceCreateInfo* modified_create_info) {
        PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice, record_obj);
    }
    void CopyDispatchState() { instance = dispatch_instance_->instance; }
    VkInstance VkHandle() const { return instance; }

#include "generated/validation_object_instance_methods.h"
};

class Device : public Logger {
  public:
    const APIVersion api_version;
    vvl::dispatch::Instance* dispatch_instance_{};
    vvl::dispatch::Device* dispatch_device_{};

    DeviceExtensions extensions;
    const DeviceFeatures& enabled_features;
    const VkPhysicalDeviceMemoryProperties& phys_dev_mem_props;
    const VkPhysicalDeviceProperties& phys_dev_props;
    const VkPhysicalDeviceVulkan11Properties& phys_dev_props_core11;
    const VkPhysicalDeviceVulkan12Properties& phys_dev_props_core12;
    const VkPhysicalDeviceVulkan13Properties& phys_dev_props_core13;
    const VkPhysicalDeviceVulkan14Properties& phys_dev_props_core14;
    const DeviceExtensionProperties& phys_dev_ext_props;
    const GlobalSettings& global_settings;
    GpuAVSettings& gpuav_settings;
    const SyncValSettings& syncval_settings;

    const CHECK_DISABLED& disabled;
    const CHECK_ENABLED& enabled;

    const VkInstance instance;
    const VkPhysicalDevice physical_device;
    VkDevice device = VK_NULL_HANDLE;

    const LayerObjectTypeId container_type;

    bool is_device_lost = false;

    mutable std::shared_mutex validation_object_mutex;
    // If the Record phase calls a function that blocks, we might need to release
    // the lock that protects Record itself in order to avoid mutual waiting.
    static thread_local WriteLockGuard* record_guard;

    Device(vvl::dispatch::Device* dispatch_dev, Instance* instance, LayerObjectTypeId type_id)
        : Logger(dispatch_dev->debug_report),
          api_version(dispatch_dev->api_version),
          dispatch_instance_(dispatch_dev->dispatch_instance),
          dispatch_device_(dispatch_dev),
          extensions(dispatch_dev->extensions),
          enabled_features(dispatch_dev->enabled_features),
          phys_dev_mem_props(dispatch_dev->phys_dev_mem_props),
          phys_dev_props(dispatch_dev->phys_dev_props),
          phys_dev_props_core11(dispatch_dev->phys_dev_props_core11),
          phys_dev_props_core12(dispatch_dev->phys_dev_props_core12),
          phys_dev_props_core13(dispatch_dev->phys_dev_props_core13),
          phys_dev_props_core14(dispatch_dev->phys_dev_props_core14),
          phys_dev_ext_props(dispatch_dev->phys_dev_ext_props),
          global_settings(dispatch_dev->settings.global_settings),
          gpuav_settings(dispatch_dev->settings.gpuav_settings),
          syncval_settings(dispatch_dev->settings.syncval_settings),
          disabled(dispatch_dev->settings.disabled),
          enabled(dispatch_dev->settings.enabled),
          instance(instance->instance),
          physical_device(dispatch_dev->physical_device),
          device(dispatch_dev->device),
          container_type(type_id) {}
    virtual ~Device() {}

    VkDevice VkHandle() const { return device; }

    void CopyDispatchState() {
        extensions = dispatch_device_->extensions;
        device = dispatch_device_->device;
    }
    virtual ReadLockGuard ReadLock() const { return ReadLockGuard(validation_object_mutex); }
    virtual WriteLockGuard WriteLock() { return WriteLockGuard(validation_object_mutex); }

    // Should be used instead of WriteLock() if the Record phase wants to release
    // its lock during the blocking operation.
    struct BlockingOperationGuard {
        WriteLockGuard lock;
        Device* dev = nullptr;

        BlockingOperationGuard(Device* dev_) : dev(dev_) {
            // This assert detects recursive calls. It is here mostly for documentation purposes
            // because WriteLock() also triggers errors during recursion.
            // Recursion is not allowed since record_guard is a thread-local variable and it can
            // reference only one frame of the callstack.
            assert(dev->record_guard == nullptr);

            lock = dev->WriteLock();

            // Initialize record_guard only when Record is actually protected by the
            // mutex. It's not the case when fine grained locking is enabled.
            record_guard = lock.owns_lock() ? &lock : nullptr;
        }

        ~BlockingOperationGuard() { dev->record_guard = nullptr; }
    };

    // The following Begin/End methods should be called during the Record phase
    // around blocking operation that causes mutual waiting (deadlock).
    void BeginBlockingOperation() {
        if (record_guard) {
            record_guard->unlock();
        }
    }
    void EndBlockingOperation() {
        if (record_guard) {
            record_guard->lock();
        }
    }

    virtual void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks* pAllocator) {}
    virtual VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                       const VkValidationCacheEXT* pSrcCaches) {
        return VK_SUCCESS;
    }
    virtual VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                        void* pData) {
        return VK_SUCCESS;
    }
    // Manually generated pre/post hooks
    // Allow additional state parameter for CreateGraphicsPipelines
    virtual bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                        const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                        chassis::CreateGraphicsPipelines& chassis_state) const {
        return PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                      error_obj);
    }
    virtual void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                      chassis::CreateGraphicsPipelines& chassis_state) {
        PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                             record_obj);
    }
    virtual void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                       chassis::CreateGraphicsPipelines& chassis_state) {
        PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                              record_obj);
    }

    // Allow additional state parameter for CreateComputePipelines
    virtual bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkComputePipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                       const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                       chassis::CreateComputePipelines& chassis_state) const {
        return PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                     error_obj);
    }
    virtual void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                     const VkComputePipelineCreateInfo* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                     chassis::CreateComputePipelines& chassis_state) {
        PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                            record_obj);
    }
    virtual void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                      const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                      chassis::CreateComputePipelines& chassis_state) {
        PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                             record_obj);
    }

    // Allow additional state parameter for CreateRayTracingPipelinesNV
    virtual bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                            uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                            chassis::CreateRayTracingPipelinesNV& chassis_state) const {
        return PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                          pPipelines, error_obj);
    }
    virtual void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                          chassis::CreateRayTracingPipelinesNV& chassis_state) {
        PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                 record_obj);
    }
    virtual void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                           chassis::CreateRayTracingPipelinesNV& chassis_state) {
        PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                  record_obj);
    }

    // Allow additional state parameter for CreateRayTracingPipelinesKHR
    virtual bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                             const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                             const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                             chassis::CreateRayTracingPipelinesKHR& chassis_state) const {
        return PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                           pAllocator, pPipelines, error_obj);
    }
    virtual void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                           VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                           chassis::CreateRayTracingPipelinesKHR& chassis_state) {
        PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                  pAllocator, pPipelines, record_obj);
    }
    virtual void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                            const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                            std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
        PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                   pAllocator, pPipelines, record_obj);
    }

    // Allow modification of a down-chain parameter for CreatePipelineLayout
    virtual void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                   const RecordObject& record_obj, chassis::CreatePipelineLayout& chassis_state) {
        PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
    }

    // Enable the CreateShaderModule/CreateShaderEXT API to take an extra argument for state preservation and paramter modification
    virtual void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                 const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) {
        PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj);
    }
    virtual void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                                  const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) {
        PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj);
    }
    virtual void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                               const RecordObject& record_obj, chassis::ShaderObject& chassis_state) {
        PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj);
    }
    virtual void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                VkShaderEXT* pShaders, const RecordObject& record_obj,
                                                chassis::ShaderObject& chassis_state) {
        PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj);
    }

    // Allow AllocateDescriptorSets to use some local stack storage for performance purposes
    virtual bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                       VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                                       vvl::AllocateDescriptorSetsData& ads_state) const {
        return PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj);
    }
    virtual void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                      VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                                      vvl::AllocateDescriptorSetsData& ads_state) {
        PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj);
    }

    // Allow modification of a down-chain parameter for CreateBuffer
    virtual void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer,
                                           const RecordObject& record_obj, chassis::CreateBuffer& chassis_state) {
        PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj);
    }

#include "generated/validation_object_device_methods.h"
};

}  // namespace vvl::base
