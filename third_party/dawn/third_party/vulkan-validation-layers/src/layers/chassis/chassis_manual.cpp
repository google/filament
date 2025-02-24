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
#include "chassis.h"

#include <array>
#include <cstring>
#include <mutex>

#include "chassis/dispatch_object.h"
#include "chassis/validation_object.h"
#include "layer_options.h"
#include "state_tracker/descriptor_sets.h"
#include "chassis/chassis_modification_state.h"
#include "core_checks/core_validation.h"
#include "profiling/profiling.h"

namespace vulkan_layer_chassis {

// Check enabled instance extensions against supported instance extension whitelist
static void InstanceExtensionWhitelist(vvl::dispatch::Instance* layer_data, const VkInstanceCreateInfo* pCreateInfo,
                                       VkInstance instance) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized instance extensions
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        if (!IsInstanceExtension(extension)) {
            Location loc(vvl::Func::vkCreateInstance);
            layer_data->LogWarning(kVUIDUndefined, layer_data->instance,
                                   loc.dot(vvl::Field::pCreateInfo).dot(vvl::Field::ppEnabledExtensionNames, i),
                                   "%s is not supported by this layer.  Using this extension may adversely affect validation "
                                   "results and/or produce undefined behavior.",
                                   pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
}

// Check enabled device extensions against supported device extension whitelist
static void DeviceExtensionWhitelist(vvl::dispatch::Device* layer_data, const VkDeviceCreateInfo* pCreateInfo, VkDevice device) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized device extensions
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        if (!IsDeviceExtension(extension)) {
            Location loc(vvl::Func::vkCreateDevice);
            layer_data->LogWarning(kVUIDUndefined, layer_data->device,
                                   loc.dot(vvl::Field::pCreateInfo).dot(vvl::Field::ppEnabledExtensionNames, i),
                                   "%s is not supported by this layer.  Using this extension may adversely affect validation "
                                   "results and/or produce undefined behavior.",
                                   pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }
}

void OutputLayerStatusInfo(vvl::dispatch::Instance* context) {
    std::string list_of_enables;
    std::string list_of_disables;
    for (uint32_t i = 0; i < kMaxEnableFlags; i++) {
        if (context->settings.enabled[i]) {
            if (list_of_enables.size()) list_of_enables.append(", ");
            list_of_enables.append(GetEnableFlagNameHelper()[i]);
        }
    }
    if (list_of_enables.empty()) {
        list_of_enables.append("None");
    }
    for (uint32_t i = 0; i < kMaxDisableFlags; i++) {
        if (context->settings.disabled[i]) {
            if (list_of_disables.size()) list_of_disables.append(", ");
            list_of_disables.append(GetDisableFlagNameHelper()[i]);
        }
    }
    if (list_of_disables.empty()) {
        list_of_disables.append("None");
    }

    Location loc(vvl::Func::vkCreateInstance);
    // Output layer status information message
    // TODO - We should just dump all settings to a file (see https://github.com/KhronosGroup/Vulkan-Utility-Libraries/issues/188)
    context->LogInfo("WARNING-CreateInstance-status-message", context->instance, loc,
                     "Khronos Validation Layer Active:\n    Current Enables: %s.\n    Current Disables: %s.\n",
                     list_of_enables.c_str(), list_of_disables.c_str());

    // Create warning message if user is running debug layers.
#ifndef NDEBUG
    context->LogPerformanceWarning("WARNING-CreateInstance-debug-warning", context->instance, loc,
                                   "Using debug builds of the validation layers *will* adversely affect performance.");
#endif
    if (!context->settings.global_settings.fine_grained_locking) {
        context->LogPerformanceWarning(
            "WARNING-CreateInstance-locking-warning", context->instance, loc,
            "Fine-grained locking is disabled, this will adversely affect performance of multithreaded applications.");
    }
}
const vvl::unordered_map<std::string, function_data>& GetNameToFuncPtrMap();

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* funcName) {
    auto layer_data = vvl::dispatch::GetData(device);
    if (!ApiParentExtensionEnabled(funcName, &layer_data->extensions)) {
        return nullptr;
    }
    const auto& item = GetNameToFuncPtrMap().find(funcName);
    if (item != GetNameToFuncPtrMap().end()) {
        if (item->second.function_type != kFuncTypeDev) {
            Location loc(vvl::Func::vkGetDeviceProcAddr);
            // Was discussed in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6583
            // This has "valid" behavior to return null, but still worth warning users for this unqiue function
            layer_data->LogWarning("WARNING-vkGetDeviceProcAddr-device", device, loc.dot(vvl::Field::pName),
                                   "is trying to grab %s which is an instance level function", funcName);
            return nullptr;
        } else {
            return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
        }
    }
    auto& table = layer_data->device_dispatch_table;
    if (!table.GetDeviceProcAddr) return nullptr;
    return table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* funcName) {
    const auto& item = GetNameToFuncPtrMap().find(funcName);
    if (item != GetNameToFuncPtrMap().end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
    }
    auto layer_data = vvl::dispatch::GetData(instance);
    auto& table = layer_data->instance_dispatch_table;
    if (!table.GetInstanceProcAddr) return nullptr;
    return table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName) {
    const auto& item = GetNameToFuncPtrMap().find(funcName);
    if (item != GetNameToFuncPtrMap().end()) {
        if (item->second.function_type != kFuncTypePdev) {
            return nullptr;
        } else {
            return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
        }
    }
    auto layer_data = vvl::dispatch::GetData(instance);
    auto& table = layer_data->instance_dispatch_table;
    if (!table.GetPhysicalDeviceProcAddr) return nullptr;
    return table.GetPhysicalDeviceProcAddr(instance, funcName);
}

// This is here as some applications will call exit() which results in all our static allocations (like std::map) having their
// destructor called and destroyed from under us. It is not possible to detect as sometimes (when using things like robin hood) the
// size()/empty() will give false positive that memory is there there. We add this global hook that will go through and remove all
// the function calls such that things can safely run in the case the applicaiton still wants to make Vulkan calls in their atexit()
// handler
void ApplicationAtExit() {
    // On a "normal" application, this function is called after vkDestroyInstance and layer_data_map is empty
    //
    // If there are multiple devices we still want to delete them all as exit() is a global scope call
    vvl::dispatch::FreeAllData();
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                              VkInstance* pInstance) {
    atexit(ApplicationAtExit);

    VVL_ZoneScoped;
    VkLayerInstanceCreateInfo* chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(nullptr, "vkCreateInstance");
    if (fpCreateInstance == nullptr) return VK_ERROR_INITIALIZATION_FAILED;
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    auto instance_dispatch = std::make_unique<vvl::dispatch::Instance>(pCreateInfo);

    // Init dispatch array and call registration functions
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateInstance, VulkanTypedHandle());
    for (const auto& vo : instance_dispatch->object_dispatch) {
        skip |= vo->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance, error_obj);
        if (skip) {
            return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkCreateInstance);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, record_obj);
    }

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }
    record_obj.result = result;
    instance_dispatch->instance = *pInstance;
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->CopyDispatchState();
    }

    layer_init_instance_dispatch_table(*pInstance, &instance_dispatch->instance_dispatch_table, fpGetInstanceProcAddr);

    OutputLayerStatusInfo(instance_dispatch.get());
    InstanceExtensionWhitelist(instance_dispatch.get(), pCreateInfo, *pInstance);
    // save a raw pointer since the unique_ptr will be invalidate by the move() below
    auto* id = instance_dispatch.get();
    vvl::dispatch::SetData(*pInstance, std::move(instance_dispatch));

    for (auto& vo : id->object_dispatch) {
        vo->PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, record_obj);
    }

    DeactivateInstanceDebugCallbacks(id->debug_report);

    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    VVL_TracyCZone(tracy_zone_precall, true);
    auto* key = GetDispatchKey(instance);
    auto instance_dispatch = vvl::dispatch::GetData(instance);
    ActivateInstanceDebugCallbacks(instance_dispatch->debug_report);
    ErrorObject error_obj(vvl::Func::vkDestroyInstance, VulkanTypedHandle(instance, kVulkanObjectTypeInstance));

    for (const auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallValidateDestroyInstance(instance, pAllocator, error_obj);
    }

    RecordObject record_obj(vvl::Func::vkDestroyInstance);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallRecordDestroyInstance(instance, pAllocator, record_obj);
    }

    VVL_TracyCZoneEnd(tracy_zone_precall);
    VVL_TracyCZone(tracy_zone_dispatch, true);
    instance_dispatch->instance_dispatch_table.DestroyInstance(instance, pAllocator);
    VVL_TracyCZoneEnd(tracy_zone_dispatch);

    VVL_TracyCZone(tracy_zone_postcall, true);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PostCallRecordDestroyInstance(instance, pAllocator, record_obj);
    }

    DeactivateInstanceDebugCallbacks(instance_dispatch->debug_report);
    vvl::dispatch::FreeData(key, instance);

    VVL_TracyCZoneEnd(tracy_zone_postcall);

#if TRACY_MANUAL_LIFETIME
    tracy::ShutdownProfiler();
#endif
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    VkLayerDeviceCreateInfo* chain_info = GetChainInfo(pCreateInfo, VK_LAYER_LINK_INFO);

    auto instance_dispatch = vvl::dispatch::GetData(gpu);

    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(instance_dispatch->instance, "vkCreateDevice");
    if (fpCreateDevice == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    // use a unique pointer to make sure we destroy this object on error
    auto device_dispatch = std::make_unique<vvl::dispatch::Device>(instance_dispatch, gpu, pCreateInfo);

    // This is odd but we need to set the current extensions in all of the
    // instance validation objects so that they are available for validating CreateDevice
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->extensions = device_dispatch->extensions;
    }

    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateDevice, VulkanTypedHandle(gpu, kVulkanObjectTypePhysicalDevice));
    for (const auto& vo : instance_dispatch->object_dispatch) {
        skip |= vo->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, error_obj);
        if (skip) {
            return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    // Make copy to modify as some ValidationObjects will want to add extensions/features on
    // After PreCallValidate incase it is invalid
    vku::safe_VkDeviceCreateInfo modified_create_info(pCreateInfo);

    RecordObject record_obj(vvl::Func::vkCreateDevice);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, record_obj, &modified_create_info);
    }
    // Recalculate enabled_features based on any changes made
    GetEnabledDeviceFeatures(modified_create_info.ptr(), &device_dispatch->enabled_features, device_dispatch->api_version);

    VkResult result = fpCreateDevice(gpu, reinterpret_cast<VkDeviceCreateInfo*>(&modified_create_info), pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }
    record_obj.result = result;
    device_dispatch->device = *pDevice;
    // Save local info in device object
    device_dispatch->extensions = DeviceExtensions(instance_dispatch->extensions, device_dispatch->api_version,
                                                   reinterpret_cast<VkDeviceCreateInfo*>(&modified_create_info));
    layer_init_device_dispatch_table(*pDevice, &device_dispatch->device_dispatch_table, fpGetDeviceProcAddr);

    instance_dispatch->debug_report->device_created++;

    for (auto& vo : device_dispatch->object_dispatch) {
        vo->CopyDispatchState();
    }
    DeviceExtensionWhitelist(device_dispatch.get(), pCreateInfo, *pDevice);
    // NOTE: many PostCallRecords expect to be able to look up the device dispatch object so we need to populate the map here.
#if defined(VVL_TRACY_GPU)
    InitTracyVk(instance_dispatch->instance, gpu, *pDevice, fpGetInstanceProcAddr, fpGetDeviceProcAddr,
                device_dispatch->device_dispatch_table.ResetCommandBuffer,
                device_dispatch->device_dispatch_table.BeginCommandBuffer, device_dispatch->device_dispatch_table.EndCommandBuffer,
                device_dispatch->device_dispatch_table.QueueSubmit);
#endif

    vvl::dispatch::SetData(*pDevice, std::move(device_dispatch));
    for (auto& vo : instance_dispatch->object_dispatch) {
        // Send down modified create info as we want to mark enabled features that we sent down on behalf of the app
        vo->PostCallRecordCreateDevice(gpu, reinterpret_cast<VkDeviceCreateInfo*>(&modified_create_info), pAllocator, pDevice,
                                       record_obj);
    }

    return result;
}

// NOTE: Do _not_ skip the dispatch call when destroying a device. Whether or not there was a validation error,
//       the loader will destroy the device, and know nothing about future references to this device making it
//       impossible for the caller to use this device handle further. IOW, this is our _only_ chance to (potentially)
//       dispatch the driver's DestroyDevice function.
VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    auto* key = GetDispatchKey(device);
    auto device_dispatch = vvl::dispatch::GetData(device);
    ErrorObject error_obj(vvl::Func::vkDestroyDevice, VulkanTypedHandle(device, kVulkanObjectTypeDevice));
    for (const auto& vo : device_dispatch->object_dispatch) {
        vo->PreCallValidateDestroyDevice(device, pAllocator, error_obj);
    }

    RecordObject record_obj(vvl::Func::vkDestroyDevice);
    for (auto& vo : device_dispatch->object_dispatch) {
        vo->PreCallRecordDestroyDevice(device, pAllocator, record_obj);
    }

    // Before device is destroyed, allow aborted objects to clean up
    for (auto& vo : device_dispatch->aborted_object_dispatch) {
        vo->PreCallRecordDestroyDevice(device, pAllocator, record_obj);
    }

#if defined(VVL_TRACY_GPU)
    CleanupTracyVk(device);
#endif

    device_dispatch->DestroyDevice(device, pAllocator);

    for (auto& vo : device_dispatch->object_dispatch) {
        vo->PostCallRecordDestroyDevice(device, pAllocator, record_obj);
    }

    auto instance_dispatch = vvl::dispatch::GetData(device_dispatch->physical_device);
    instance_dispatch->debug_report->device_created--;

    vvl::dispatch::FreeData(key, device);
}

// Special-case APIs for which core_validation needs custom parameter lists and/or modifies parameters

VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                       const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateGraphicsPipelines, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    PipelineStates pipeline_states[LayerObjectTypeMaxEnum];
    chassis::CreateGraphicsPipelines chassis_state(pCreateInfos);

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                               pPipelines, error_obj, pipeline_states[vo->container_type],
                                                               chassis_state);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkCreateGraphicsPipelines);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                     record_obj, pipeline_states[vo->container_type], chassis_state);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateGraphicsPipelines(device, pipelineCache, createInfoCount, chassis_state.pCreateInfos,
                                                          pAllocator, pPipelines);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                      record_obj, pipeline_states[vo->container_type], chassis_state);
        }
    }
    return result;
}

// This API saves some core_validation pipeline state state on the stack for performance purposes
VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo* pCreateInfos,
                                                      const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateComputePipelines, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    PipelineStates pipeline_states[LayerObjectTypeMaxEnum];
    chassis::CreateComputePipelines chassis_state(pCreateInfos);

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                              pPipelines, error_obj, pipeline_states[vo->container_type],
                                                              chassis_state);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkCreateComputePipelines);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                    record_obj, pipeline_states[vo->container_type], chassis_state);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateComputePipelines(device, pipelineCache, createInfoCount, chassis_state.pCreateInfos,
                                                         pAllocator, pPipelines);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                     record_obj, pipeline_states[vo->container_type], chassis_state);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateRayTracingPipelinesNV, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    PipelineStates pipeline_states[LayerObjectTypeMaxEnum];
    chassis::CreateRayTracingPipelinesNV chassis_state(pCreateInfos);

    for (const auto& vo : device_dispatch->object_dispatch) {
        auto lock = vo->ReadLock();
        skip |= vo->PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                               pPipelines, error_obj, pipeline_states[vo->container_type],
                                                               chassis_state);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    RecordObject record_obj(vvl::Func::vkCreateRayTracingPipelinesNV);
    for (auto& vo : device_dispatch->object_dispatch) {
        auto lock = vo->WriteLock();
        vo->PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                     record_obj, pipeline_states[vo->container_type], chassis_state);
    }

    VkResult result = device_dispatch->CreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount,
                                                                   chassis_state.pCreateInfos, pAllocator, pPipelines);
    record_obj.result = result;

    for (auto& vo : device_dispatch->object_dispatch) {
        auto lock = vo->WriteLock();
        vo->PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                      record_obj, pipeline_states[vo->container_type], chassis_state);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                            const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateRayTracingPipelinesKHR, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    PipelineStates pipeline_states[LayerObjectTypeMaxEnum];
    auto chassis_state = std::make_shared<chassis::CreateRayTracingPipelinesKHR>(pCreateInfos);

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                                    pCreateInfos, pAllocator, pPipelines, error_obj,
                                                                    pipeline_states[vo->container_type], *chassis_state);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkCreateRayTracingPipelinesKHR);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                          pAllocator, pPipelines, record_obj, pipeline_states[vo->container_type],
                                                          *chassis_state);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                               chassis_state->pCreateInfos, pAllocator, pPipelines);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                           pAllocator, pPipelines, record_obj, pipeline_states[vo->container_type],
                                                           chassis_state);
        }
    }
    return result;
}

// This API needs the ability to modify a down-chain parameter
VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreatePipelineLayout, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallValidateCreatePipelineLayout]) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    chassis::CreatePipelineLayout chassis_state{};
    chassis_state.modified_create_info = *pCreateInfo;

    RecordObject record_obj(vvl::Func::vkCreatePipelineLayout);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj, chassis_state);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreatePipelineLayout(device, &chassis_state.modified_create_info, pAllocator, pPipelineLayout);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPostCallRecordCreatePipelineLayout]) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
        }
    }
    return result;
}

// This API needs some local stack data for performance reasons and also may modify a parameter
VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateShaderModule, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    chassis::CreateShaderModule chassis_state{};

    RecordObject record_obj(vvl::Func::vkCreateShaderModule);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, chassis_state);
        }
    }

    // Special extra check if SPIR-V itself fails runtime validation in PreCallRecord
    if (chassis_state.skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    }
    record_obj.result = result;
    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, chassis_state);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                VkShaderEXT* pShaders) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateShadersEXT, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    chassis::ShaderObject chassis_state(createInfoCount, pCreateInfos);

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkCreateShadersEXT);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                              chassis_state);
        }
    }

    // Special extra check if SPIR-V itself fails runtime validation in PreCallRecord
    if (chassis_state.skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateShadersEXT(device, createInfoCount, chassis_state.pCreateInfos, pAllocator, pShaders);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                               chassis_state);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                      VkDescriptorSet* pDescriptorSets) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkAllocateDescriptorSets, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    vvl::AllocateDescriptorSetsData ads_state[LayerObjectTypeMaxEnum];

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->object_dispatch) {
            ads_state[vo->container_type].Init(pAllocateInfo->descriptorSetCount);
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj,
                                                              ads_state[vo->container_type]);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkAllocateDescriptorSets);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallRecordAllocateDescriptorSets]) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj,
                                                     ads_state[vo->container_type]);
        }
    }
    return result;
}

// This API needs the ability to modify a down-chain parameter
VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(device);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkCreateBuffer, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallValidateCreateBuffer]) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    chassis::CreateBuffer chassis_state{};
    chassis_state.modified_create_info = *pCreateInfo;

    RecordObject record_obj(vvl::Func::vkCreateBuffer);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->object_dispatch) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj, chassis_state);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->CreateBuffer(device, &chassis_state.modified_create_info, pAllocator, pBuffer);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPostCallRecordCreateBuffer]) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj);
        }
    }
    return result;
}

// This API needs to ensure that per-swapchain VkResult results are available
VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(queue);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkQueuePresentKHR, VulkanTypedHandle(queue, kVulkanObjectTypeQueue));
    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallValidateQueuePresentKHR]) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateQueuePresentKHR(queue, pPresentInfo, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }
    RecordObject record_obj(vvl::Func::vkQueuePresentKHR);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallRecordQueuePresentKHR]) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
        }
    }

    // Track per-swapchain results when there is more than one swapchain and VkPresentInfoKHR::pResults is null
    small_vector<VkResult, 2> present_results;
    VkPresentInfoKHR modified_present_info;
    if (pPresentInfo && pPresentInfo->swapchainCount > 1 && pPresentInfo->pResults == nullptr) {
        present_results.resize(pPresentInfo->swapchainCount);
        modified_present_info = *pPresentInfo;
        modified_present_info.pResults = present_results.data();
        pPresentInfo = &modified_present_info;
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->QueuePresentKHR(queue, pPresentInfo);
    }
    VVL_TracyCFrameMark;
#if defined(VVL_TRACY_GPU)
    TracyVkCollector::GetTracyVkCollector(queue).Collect();
#endif
    record_obj.result = result;
    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPostCallRecordQueuePresentKHR]) {
            auto lock = vo->WriteLock();

            if (result == VK_ERROR_DEVICE_LOST) {
                vo->is_device_lost = true;
            }
            vo->PostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
        }
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    VVL_ZoneScoped;

    auto device_dispatch = vvl::dispatch::GetData(commandBuffer);
    bool skip = false;
    chassis::HandleData handle_data;

    ErrorObject error_obj(vvl::Func::vkBeginCommandBuffer, VulkanTypedHandle(commandBuffer, kVulkanObjectTypeCommandBuffer),
                          &handle_data);
    handle_data.command_buffer.is_secondary = device_dispatch->IsSecondary(commandBuffer);
    {
        VVL_ZoneScopedN("PreCallValidate");
        for (const auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallValidateBeginCommandBuffer]) {
            auto lock = vo->ReadLock();
            skip |= vo->PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo, error_obj);
            if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
        }
    }

    RecordObject record_obj(vvl::Func::vkBeginCommandBuffer, &handle_data);
    {
        VVL_ZoneScopedN("PreCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPreCallRecordBeginCommandBuffer]) {
            auto lock = vo->WriteLock();
            vo->PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);
        }
    }

    VkResult result;
    {
        VVL_ZoneScopedN("Dispatch");
        result = device_dispatch->BeginCommandBuffer(commandBuffer, pBeginInfo);
    }
    record_obj.result = result;

    {
        VVL_ZoneScopedN("PostCallRecord");
        for (auto& vo : device_dispatch->intercept_vectors[InterceptIdPostCallRecordBeginCommandBuffer]) {
            auto lock = vo->WriteLock();
            vo->PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);
        }
    }
    return result;
}

// Handle tooling queries manually as this is a request for layer information
static const VkPhysicalDeviceToolPropertiesEXT khronos_layer_tool_props = {
    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
    nullptr,
    "Khronos Validation Layer",
    STRINGIFY(VK_HEADER_VERSION),
    VK_TOOL_PURPOSE_VALIDATION_BIT | VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT | VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT |
        VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT,
    "Khronos Validation Layer",
    OBJECT_LAYER_NAME};

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                  VkPhysicalDeviceToolPropertiesEXT* pToolProperties) {
    auto instance_dispatch = vvl::dispatch::GetData(physicalDevice);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkGetPhysicalDeviceToolPropertiesEXT,
                          VulkanTypedHandle(physicalDevice, kVulkanObjectTypePhysicalDevice));

    auto original_pToolProperties = pToolProperties;

    if (pToolProperties != nullptr && *pToolCount > 0) {
        *pToolProperties = khronos_layer_tool_props;
        pToolProperties = ((*pToolCount > 1) ? &pToolProperties[1] : nullptr);
        (*pToolCount)--;
    }

    for (const auto& vo : instance_dispatch->object_dispatch) {
        skip |= vo->PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, error_obj);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    RecordObject record_obj(vvl::Func::vkGetPhysicalDeviceToolPropertiesEXT);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, record_obj);
    }

    VkResult result = instance_dispatch->GetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    record_obj.result = result;

    if (original_pToolProperties != nullptr) {
        pToolProperties = original_pToolProperties;
    }
    assert(*pToolCount != std::numeric_limits<uint32_t>::max());
    (*pToolCount)++;

    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, record_obj);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                               VkPhysicalDeviceToolProperties* pToolProperties) {
    auto instance_dispatch = vvl::dispatch::GetData(physicalDevice);
    bool skip = false;
    ErrorObject error_obj(vvl::Func::vkGetPhysicalDeviceToolProperties,
                          VulkanTypedHandle(physicalDevice, kVulkanObjectTypePhysicalDevice));

    auto original_pToolProperties = pToolProperties;

    if (pToolProperties != nullptr && *pToolCount > 0) {
        *pToolProperties = khronos_layer_tool_props;
        pToolProperties = ((*pToolCount > 1) ? &pToolProperties[1] : nullptr);
        (*pToolCount)--;
    }

    for (const auto& vo : instance_dispatch->object_dispatch) {
        skip |= vo->PreCallValidateGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, error_obj);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    RecordObject record_obj(vvl::Func::vkGetPhysicalDeviceToolProperties);
    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PreCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, record_obj);
    }

    VkResult result = instance_dispatch->GetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
    record_obj.result = result;

    if (original_pToolProperties != nullptr) {
        pToolProperties = original_pToolProperties;
    }
    assert(*pToolCount != std::numeric_limits<uint32_t>::max());
    (*pToolCount)++;

    for (auto& vo : instance_dispatch->object_dispatch) {
        vo->PostCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, record_obj);
    }
    return result;
}

// ValidationCache APIs do not dispatch

VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkValidationCacheEXT* pValidationCache) {
    auto device_dispatch = vvl::dispatch::GetData(device);
    if (auto core_checks = static_cast<CoreChecks*>(device_dispatch->GetValidationObject(LayerObjectTypeCoreValidation))) {
        auto lock = core_checks->WriteLock();
        return core_checks->CoreLayerCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
    }
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                     const VkAllocationCallbacks* pAllocator) {
    auto device_dispatch = vvl::dispatch::GetData(device);
    if (auto core_checks = static_cast<CoreChecks*>(device_dispatch->GetValidationObject(LayerObjectTypeCoreValidation))) {
        auto lock = core_checks->WriteLock();
        core_checks->CoreLayerDestroyValidationCacheEXT(device, validationCache, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                        const VkValidationCacheEXT* pSrcCaches) {
    auto device_dispatch = vvl::dispatch::GetData(device);
    if (auto core_checks = static_cast<CoreChecks*>(device_dispatch->GetValidationObject(LayerObjectTypeCoreValidation))) {
        auto lock = core_checks->WriteLock();
        return core_checks->CoreLayerMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
    }
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                         void* pData) {
    auto device_dispatch = vvl::dispatch::GetData(device);
    if (auto core_checks = static_cast<CoreChecks*>(device_dispatch->GetValidationObject(LayerObjectTypeCoreValidation))) {
        auto lock = core_checks->WriteLock();
        return core_checks->CoreLayerGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
    }
    return VK_SUCCESS;
}
}  // namespace vulkan_layer_chassis
