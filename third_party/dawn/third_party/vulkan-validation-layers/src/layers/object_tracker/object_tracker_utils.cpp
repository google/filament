/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
#include "object_lifetime_validation.h"
#include "chassis/dispatch_object.h"

namespace object_lifetimes {

static std::shared_mutex lifetime_set_mutex;
static vvl::unordered_set<Tracker*> lifetime_set;

Instance::Instance(vvl::dispatch::Instance *dispatch)
        : BaseClass(dispatch, LayerObjectTypeObjectTracker), tracker(debug_report) {
    WriteLockGuard lock(lifetime_set_mutex);
    lifetime_set.insert(&tracker);
}

Instance::~Instance() {
    WriteLockGuard lock(lifetime_set_mutex);
    lifetime_set.erase(&tracker);
}

Device::Device(vvl::dispatch::Device *dev, Instance *instance)
    : BaseClass(dev, instance, LayerObjectTypeObjectTracker), tracker(debug_report) {
    WriteLockGuard lock(lifetime_set_mutex);
    lifetime_set.insert(&tracker);
}

Device::~Device() {
    WriteLockGuard lock(lifetime_set_mutex);
    lifetime_set.erase(&tracker);
}

VulkanTypedHandle ObjTrackStateTypedHandle(const ObjTrackState &track_state) {
    // TODO: Unify Typed Handle representation (i.e. VulkanTypedHandle everywhere there are handle/type pairs)
    VulkanTypedHandle typed_handle;
    typed_handle.handle = track_state.handle;
    typed_handle.type = track_state.object_type;
    return typed_handle;
}

bool Tracker::TracksObject(uint64_t object_handle, VulkanObjectType object_type) const {
    // Look for object in object map
    return object_map[object_type].contains(object_handle);
}

bool Tracker::CheckObjectValidity(uint64_t object_handle, VulkanObjectType object_type, const char *invalid_handle_vuid,
                                   const char *wrong_parent_vuid, const Location &loc, VulkanObjectType parent_type) const {
    constexpr bool skip = false;

    // If this instance of lifetime validation tracks the object, report success
    if (TracksObject(object_handle, object_type)) {
        return skip;
    }
    // Object not found, look for it in other device object maps
    const Tracker *other_lifetimes = nullptr;
    {
        ReadLockGuard lock(lifetime_set_mutex);
        for (const auto *lifetimes : lifetime_set) {
            if (lifetimes != this && lifetimes->TracksObject(object_handle, object_type)) {
                other_lifetimes = lifetimes;
                break;
            }
        }
    }
    // Object was not found anywhere
    if (!other_lifetimes) {
        return LogError(invalid_handle_vuid, handle, loc, "Invalid %s Object 0x%" PRIxLEAST64 ".",
                        string_VulkanObjectType(object_type), object_handle);
    }
    // Anonymous object validation does not check parent, only that the object exists
    if (wrong_parent_vuid == kVUIDUndefined) {
        return skip;
    }

    // Object found on another device
    LogObjectList objlist(handle, other_lifetimes->handle);
    std::string handle_str(FormatHandle(handle));
    std::string other_handle_str(FormatHandle(other_lifetimes->handle));
    return LogError(wrong_parent_vuid, objlist, loc,
                    "(%s 0x%" PRIxLEAST64
                    ") was created, allocated or retrieved from %s, but command is using (or its dispatchable parameter is "
                    "associated with) %s",
                    string_VulkanObjectType(object_type), object_handle, other_handle_str.c_str(), handle_str.c_str());
}

bool Device::CheckPipelineObjectValidity(uint64_t object_handle, const char *invalid_handle_vuid, const Location &loc) const {
     bool skip = false;
     const auto &itr = linked_graphics_pipeline_map.find(object_handle);
     if (itr == linked_graphics_pipeline_map.end()) {
         return skip;  // no-linked
     }
     for (const auto &pipeline : itr->second) {
         if (!tracker.TracksObject(pipeline->handle, kVulkanObjectTypePipeline)) {
             skip |= LogError(invalid_handle_vuid, instance, loc,
                              "Invalid VkPipeline Object 0x%" PRIxLEAST64
                              " as it was created with VkPipelineLibraryCreateInfoKHR::pLibraries 0x%" PRIxLEAST64
                              " that doesn't exist anymore. The application must maintain the lifetime of a pipeline library based "
                              "on the pipelines that link with it.",
                              object_handle, pipeline->handle);
             break;
         } else {
             // Libaries pipeline can have their own nested libraries
             skip |= CheckPipelineObjectValidity(pipeline->handle, invalid_handle_vuid, loc);
         }
     }
     return skip;
}

void Tracker::DestroyObjectSilently(uint64_t object, VulkanObjectType object_type, const Location &loc) {
    assert(object != HandleToUint64(VK_NULL_HANDLE));

    auto item = object_map[object_type].pop(object);
    if (item == object_map[object_type].end()) {
        // We've already checked that the object exists. If we couldn't find and atomically remove it
        // from the map, there must have been a race condition in the app. Report an error and move on.
        (void)LogError("UNASSIGNED-ObjectTracker-Destroy", handle, loc,
                       "Couldn't destroy %s Object 0x%" PRIxLEAST64
                       ", not found. This should not happen and may indicate a race condition in the application.",
                       string_VulkanObjectType(object_type), object);

        return;
    }
}

void Tracker::DestroyUndestroyedObjects(VulkanObjectType object_type, const Location &loc) {
    auto snapshot = object_map[object_type].snapshot();
    for (const auto &item : snapshot) {
        auto object_info = item.second;
        DestroyObjectSilently(object_info->handle, object_type, loc);
    }
}

bool Device::ValidateAnonymousObject(uint64_t object, VkObjectType core_object_type, const char *invalid_handle_vuid,
                                     const char *wrong_parent_vuid, const Location &loc) const {
    auto object_type = ConvertCoreObjectToVulkanObject(core_object_type);
    return tracker.CheckObjectValidity(object, object_type, invalid_handle_vuid, wrong_parent_vuid, loc, kVulkanObjectTypeDevice);
}

void Device::AllocateCommandBuffer(const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
                                   VkCommandBufferLevel level, const Location &loc) {
    tracker.CreateObject(command_buffer, kVulkanObjectTypeCommandBuffer, nullptr, loc, command_pool);
}

bool Device::ValidateCommandBuffer(VkCommandPool command_pool, VkCommandBuffer command_buffer, const Location &loc) const {
    bool skip = false;
    uint64_t object_handle = HandleToUint64(command_buffer);
    auto iter = tracker.object_map[kVulkanObjectTypeCommandBuffer].find(object_handle);
    if (iter != tracker.object_map[kVulkanObjectTypeCommandBuffer].end()) {
        auto node = iter->second;

        if (node->parent_object != HandleToUint64(command_pool)) {
            // We know that the parent *must* be a command pool
            const auto parent_pool = CastFromUint64<VkCommandPool>(node->parent_object);
            const LogObjectList objlist(command_buffer, parent_pool, command_pool);
            skip |= LogError("VUID-vkFreeCommandBuffers-pCommandBuffers-parent", objlist, loc,
                             "attempting to free %s belonging to %s from %s.", FormatHandle(command_buffer).c_str(),
                             FormatHandle(parent_pool).c_str(), FormatHandle(command_pool).c_str());
        }
    } else {
        skip |= LogError("VUID-vkFreeCommandBuffers-pCommandBuffers-00048", command_buffer, loc, "Invalid %s.",
                         FormatHandle(command_buffer).c_str());
    }
    return skip;
}

void Device::AllocateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set, const Location &loc) {
    tracker.CreateObject(descriptor_set, kVulkanObjectTypeDescriptorSet, nullptr, loc, descriptor_pool);
    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptor_pool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        itr->second->child_objects->insert(HandleToUint64(descriptor_set));
    }
}

bool Device::ValidateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set, const Location &loc) const {
    bool skip = false;
    uint64_t object_handle = HandleToUint64(descriptor_set);
    auto ds_item = tracker.object_map[kVulkanObjectTypeDescriptorSet].find(object_handle);
    if (ds_item != tracker.object_map[kVulkanObjectTypeDescriptorSet].end()) {
        if (ds_item->second->parent_object != HandleToUint64(descriptor_pool)) {
            // We know that the parent *must* be a descriptor pool
            const auto parent_pool = CastFromUint64<VkDescriptorPool>(ds_item->second->parent_object);
            const LogObjectList objlist(descriptor_set, parent_pool, descriptor_pool);
            skip |= LogError("VUID-vkFreeDescriptorSets-pDescriptorSets-parent", objlist, loc,
                             "attempting to free %s"
                             " belonging to %s from %s.",
                             FormatHandle(descriptor_set).c_str(), FormatHandle(parent_pool).c_str(),
                             FormatHandle(descriptor_pool).c_str());
        }
    } else {
        skip |= LogError("VUID-vkFreeDescriptorSets-pDescriptorSets-00310", descriptor_set, loc, "Invalid %s.",
                         FormatHandle(descriptor_set).c_str());
    }
    return skip;
}

bool Device::ValidateDescriptorWrite(VkWriteDescriptorSet const *desc, bool isPush, const Location &loc) const {
    bool skip = false;

    // VkWriteDescriptorSet::dstSet is ignored for push vkCmdPushDescriptorSetKHR, so can be bad handle
    if (!isPush && desc->dstSet) {
        skip |= ValidateObject(desc->dstSet, kVulkanObjectTypeDescriptorSet, false, "VUID-VkWriteDescriptorSet-dstSet-00320",
                               "VUID-VkWriteDescriptorSet-commonparent", loc);
    }

    switch (desc->descriptorType) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                skip |= ValidateObject(desc->pTexelBufferView[i], kVulkanObjectTypeBufferView, true,
                                       "VUID-VkWriteDescriptorSet-descriptorType-02994",
                                       "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06236", loc.dot(Field::pTexelBufferView, i));
                if (!null_descriptor_enabled && desc->pTexelBufferView[i] == VK_NULL_HANDLE) {
                    skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02995", desc->dstSet,
                                     loc.dot(Field::pTexelBufferView, i), "is VK_NULL_HANDLE.");
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
            if (desc->pImageInfo) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(desc->pImageInfo[i].imageView, kVulkanObjectTypeImageView, true,
                                           "VUID-VkWriteDescriptorSet-descriptorType-02996",
                                           "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06239",
                                           loc.dot(Field::pImageInfo, i).dot(Field::imageView));
                    if (!null_descriptor_enabled && desc->pImageInfo[i].imageView == VK_NULL_HANDLE) {
                        skip |= LogError("VUID-VkWriteDescriptorSet-descriptorType-02997", desc->dstSet,
                                         loc.dot(Field::pImageInfo, i).dot(Field::imageView), "is VK_NULL_HANDLE.");
                    }
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            // Input attachments can never be null
            if (desc->pImageInfo) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(desc->pImageInfo[i].imageView, kVulkanObjectTypeImageView, false,
                                           "VUID-VkWriteDescriptorSet-descriptorType-07683",
                                           "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06239",
                                           loc.dot(Field::pImageInfo, i).dot(Field::imageView));
                }
            }
            break;
        }
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            if (desc->pBufferInfo) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(
                        desc->pBufferInfo[i].buffer, kVulkanObjectTypeBuffer, true, "VUID-VkDescriptorBufferInfo-buffer-parameter",
                        "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06237", loc.dot(Field::pBufferInfo, i).dot(Field::buffer));
                    if (!null_descriptor_enabled && desc->pBufferInfo[i].buffer == VK_NULL_HANDLE) {
                        skip |= LogError("VUID-VkDescriptorBufferInfo-buffer-02998", desc->dstSet,
                                         loc.dot(Field::pBufferInfo, i).dot(Field::buffer), "is VK_NULL_HANDLE.");
                    }
                }
            }
            break;
        }

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
            if (const auto *acc_info = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureKHR>(desc->pNext)) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(
                        acc_info->pAccelerationStructures[i], kVulkanObjectTypeAccelerationStructureKHR, true,
                        "VUID-VkWriteDescriptorSetAccelerationStructureKHR-pAccelerationStructures-parameter",
                        "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06240",
                        loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureKHR, Field::pAccelerationStructures, i));
                }
            }
            if (const auto *acc_info_nv = vku::FindStructInPNextChain<VkWriteDescriptorSetAccelerationStructureNV>(desc->pNext)) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(
                        acc_info_nv->pAccelerationStructures[i], kVulkanObjectTypeAccelerationStructureNV, true,
                        "VUID-VkWriteDescriptorSetAccelerationStructureNV-pAccelerationStructures-parameter",
                        "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06241",
                        loc.pNext(Struct::VkWriteDescriptorSetAccelerationStructureNV, Field::pAccelerationStructures, i));
                }
            }
            break;
        }

        case VK_DESCRIPTOR_TYPE_SAMPLER: {
            // These VUs talk about immutable samplers, we currently don't track the VkDescriptorSetLayout here to know if it uses
            // it or not, but for VK_DESCRIPTOR_TYPE_SAMPLER there is  VUID-VkWriteDescriptorSet-descriptorType-02752 which guards
            // from it containing an immutable sampler. So we are safe to validate the lifetime here. In theory this should be
            // checked for COMBINED_IMAGE_SAMPLER as well, but being discussed in
            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/4177
            if (desc->pImageInfo) {
                for (uint32_t i = 0; i < desc->descriptorCount; ++i) {
                    skip |= ValidateObject(desc->pImageInfo[i].sampler, kVulkanObjectTypeSampler, false,
                                           "VUID-VkWriteDescriptorSet-descriptorType-00325",
                                           "VUID-vkUpdateDescriptorSets-pDescriptorWrites-06238",
                                           loc.dot(Field::pImageInfo, i).dot(Field::sampler));
                }
            }
            break;
        }

        // VkWriteDescriptorSetPartitionedAccelerationStructureNV contains no VkObjects to validate
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV:

        // TODO - These need to be checked as well
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
        case VK_DESCRIPTOR_TYPE_MAX_ENUM:
            break;
    }

    return skip;
}

bool Device::PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                 const VkWriteDescriptorSet *pDescriptorWrites,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushDescriptorSet-commandBuffer-parameter"
    skip |= ValidateObject(layout, kVulkanObjectTypePipelineLayout, false, "VUID-vkCmdPushDescriptorSet-layout-parameter",
                           "VUID-vkCmdPushDescriptorSet-commonparent", error_obj.location.dot(Field::layout));
    if (pDescriptorWrites) {
        for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
            skip |=
                ValidateDescriptorWrite(&pDescriptorWrites[index0], true, error_obj.location.dot(Field::pDescriptorWrites, index0));
        }
    }
    return skip;
}

bool Device::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                               pDescriptorWrites, error_obj);
}

bool Device::PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                  const VkPushDescriptorSetInfo *pPushDescriptorSetInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdPushDescriptorSet2-commandBuffer-parameter"
    skip |= ValidateObject(pPushDescriptorSetInfo->layout, kVulkanObjectTypePipelineLayout, true,
                           "VUID-VkPushDescriptorSetInfo-layout-parameter", kVUIDUndefined,
                           error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::layout));

    if (pPushDescriptorSetInfo->pDescriptorWrites) {
        for (uint32_t index0 = 0; index0 < pPushDescriptorSetInfo->descriptorWriteCount; ++index0) {
            skip |= ValidateDescriptorWrite(
                &pPushDescriptorSetInfo->pDescriptorWrites[index0], true,
                error_obj.location.dot(Field::pPushDescriptorSetInfo).dot(Field::pDescriptorWrites, index0));
        }
    }
    return skip;
}

bool Device::PreCallValidateCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                     const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo, error_obj);
}

void Device::CreateQueue(VkQueue vkObj, const Location &loc) {
    tracker.CreateObject(vkObj, kVulkanObjectTypeQueue, nullptr, loc, device);
}

void Device::CreateSwapchainImageObject(VkImage swapchain_image, VkSwapchainKHR swapchain, const Location &loc) {
    tracker.CreateObject(swapchain_image, kVulkanObjectTypeImage, nullptr, loc, swapchain);
}

bool Instance::ReportLeakedObjects(VulkanObjectType object_type, const std::string &error_code,
                                           const Location &loc) const {
    bool skip = false;

    auto snapshot = tracker.object_map[object_type].snapshot();
    for (const auto &item : snapshot) {
        const auto object_info = item.second;
        const LogObjectList objlist(instance, ObjTrackStateTypedHandle(*object_info));
        skip |= LogError(error_code, objlist, loc, "OBJ ERROR : For %s, %s has not been destroyed.", FormatHandle(instance).c_str(),
                         FormatHandle(ObjTrackStateTypedHandle(*object_info)).c_str());
    }
    return skip;
}

bool Device::ReportLeakedObjects(VulkanObjectType object_type, const std::string &error_code,
                                       const Location &loc) const {
    bool skip = false;

    auto snapshot = tracker.object_map[object_type].snapshot();
    for (const auto &item : snapshot) {
        const auto object_info = item.second;
        const LogObjectList objlist(device, ObjTrackStateTypedHandle(*object_info));
        skip |= LogError(error_code, objlist, loc, "OBJ ERROR : For %s, %s has not been destroyed.", FormatHandle(device).c_str(),
                         FormatHandle(ObjTrackStateTypedHandle(*object_info)).c_str());
    }
    return skip;
}

bool Instance::PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
        const ErrorObject &error_obj) const {
    bool skip = false;

    // Checked by chassis: instance: "VUID-vkDestroyInstance-instance-parameter"

    auto snapshot = tracker.object_map[kVulkanObjectTypeDevice].snapshot();
    for (const auto &iit : snapshot) {
        auto node = iit.second;

        VkDevice device = reinterpret_cast<VkDevice>(node->handle);
        VkDebugReportObjectTypeEXT debug_object_type = GetDebugReport(node->object_type);

        skip |=
            LogError("VUID-vkDestroyInstance-instance-00629", instance, error_obj.location, "%s object %s has not been destroyed.",
                    string_VkDebugReportObjectTypeEXT(debug_object_type), FormatHandle(ObjTrackStateTypedHandle(*node)).c_str());

        // Throw errors if any device objects belonging to this instance have not been destroyed
        auto device_data = vvl::dispatch::GetData(device);
        auto obj_lifetimes_data = static_cast<Device *>(device_data->GetValidationObject(container_type));
        skip |= obj_lifetimes_data->ReportUndestroyedObjects(error_obj.location);

        skip |= ValidateDestroyObject(device, kVulkanObjectTypeDevice, pAllocator,
                "VUID-vkDestroyInstance-instance-00630",
                "VUID-vkDestroyInstance-instance-00631", error_obj.location);
    }

    skip |= ValidateDestroyObject(instance, kVulkanObjectTypeInstance, pAllocator,
            "VUID-vkDestroyInstance-instance-00630",
            "VUID-vkDestroyInstance-instance-00631", error_obj.location);

    // Report any remaining instance objects
    skip |= ReportUndestroyedObjects(error_obj.location);

    return skip;
}

void Instance::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
                                            const RecordObject &record_obj) {
    // Destroy physical devices
    auto snapshot = tracker.object_map[kVulkanObjectTypePhysicalDevice].snapshot();
    for (const auto &iit : snapshot) {
        auto node = iit.second;
        VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(node->handle);
        RecordDestroyObject(physical_device, kVulkanObjectTypePhysicalDevice, record_obj.location);
    }

    // Destroy child devices
    auto snapshot2 = tracker.object_map[kVulkanObjectTypeDevice].snapshot();
    for (const auto &iit : snapshot2) {
        auto node = iit.second;
        VkDevice device = reinterpret_cast<VkDevice>(node->handle);
        DestroyLeakedObjects();

        RecordDestroyObject(device, kVulkanObjectTypeDevice, record_obj.location);
    }
}

void Instance::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
                                             const RecordObject &record_obj) {
    RecordDestroyObject(instance, kVulkanObjectTypeInstance, record_obj.location);
}

bool Device::PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                          const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDevice-device-parameter"

    skip |= ValidateDestroyObject(device, kVulkanObjectTypeDevice, pAllocator, "VUID-vkDestroyDevice-device-00379",
                                  "VUID-vkDestroyDevice-device-00380", error_obj.location);
    // Report any remaining objects associated with this VkDevice object in LL
    skip |= ReportUndestroyedObjects(error_obj.location);

    return skip;
}

void Device::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto object_lifetimes = static_cast<Instance *>(dispatch_instance_->GetValidationObject(container_type));
    // If ObjectTracker was removed (in an early teardown) this might be null, could search in aborted_object_dispatch but if it is
    // there, no need to record anything else
    if (object_lifetimes) {
        object_lifetimes->RecordDestroyObject(device, kVulkanObjectTypeDevice, record_obj.location);
    }
    DestroyLeakedObjects();

    // Clean up Queue's MemRef Linked Lists
    tracker.DestroyUndestroyedObjects(kVulkanObjectTypeQueue, record_obj.location);
}

void Device::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue,
                                          const RecordObject &record_obj) {
    auto lock = WriteSharedLock();
    CreateQueue(*pQueue, record_obj.location);
}

void Device::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue,
                                           const RecordObject &record_obj) {
    auto lock = WriteSharedLock();
    CreateQueue(*pQueue, record_obj.location);
}

bool Device::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                 const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                 const VkCopyDescriptorSet *pDescriptorCopies, const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkUpdateDescriptorSets-device-parameter"
    if (pDescriptorCopies) {
        for (uint32_t idx0 = 0; idx0 < descriptorCopyCount; ++idx0) {
            const Location copies_loc = error_obj.location.dot(Field::pDescriptorCopies, idx0);
            if (pDescriptorCopies[idx0].dstSet) {
                skip |= ValidateObject(pDescriptorCopies[idx0].dstSet, kVulkanObjectTypeDescriptorSet, false,
                                       "VUID-VkCopyDescriptorSet-dstSet-parameter", "VUID-VkCopyDescriptorSet-commonparent",
                                       copies_loc.dot(Field::dstSet));
            }
            if (pDescriptorCopies[idx0].srcSet) {
                skip |= ValidateObject(pDescriptorCopies[idx0].srcSet, kVulkanObjectTypeDescriptorSet, false,
                                       "VUID-VkCopyDescriptorSet-srcSet-parameter", "VUID-VkCopyDescriptorSet-commonparent",
                                       copies_loc.dot(Field::srcSet));
            }
        }
    }
    if (pDescriptorWrites) {
        for (uint32_t idx1 = 0; idx1 < descriptorWriteCount; ++idx1) {
            skip |=
                ValidateDescriptorWrite(&pDescriptorWrites[idx1], false, error_obj.location.dot(Field::pDescriptorWrites, idx1));
        }
    }
    return skip;
}

bool Device::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    auto lock = ReadSharedLock();
    // Checked by chassis: device: "VUID-vkResetDescriptorPool-device-parameter"

    skip |= ValidateObject(descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                           "VUID-vkResetDescriptorPool-descriptorPool-parameter",
                           "VUID-vkResetDescriptorPool-descriptorPool-parent", error_obj.location.dot(Field::descriptorPool));

    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pool_node = itr->second;
        for (auto set : *pool_node->child_objects) {
            skip |= ValidateDestroyObject((VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined, error_obj.location);
        }
    }
    return skip;
}

void Device::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                              const RecordObject &record_obj) {
    auto lock = WriteSharedLock();
    // A DescriptorPool's descriptor sets are implicitly deleted when the pool is reset. Remove this pool's descriptor sets from
    // our descriptorSet map.
    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pool_node = itr->second;
        for (auto set : *pool_node->child_objects) {
            RecordDestroyObject((VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, record_obj.location);
        }
        pool_node->child_objects->clear();
    }
}

bool Device::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *begin_info,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkBeginCommandBuffer-commandBuffer-parameter"

    if (begin_info) {
        auto iter = tracker.object_map[kVulkanObjectTypeCommandBuffer].find(HandleToUint64(commandBuffer));
        if (iter != tracker.object_map[kVulkanObjectTypeCommandBuffer].end()) {
            auto node = iter->second;
            if ((begin_info->pInheritanceInfo) && error_obj.handle_data->command_buffer.is_secondary &&
                (begin_info->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);
                const Location inheritance_info_loc = begin_info_loc.dot(Field::pInheritanceInfo);
                skip |=
                    ValidateObject(begin_info->pInheritanceInfo->framebuffer, kVulkanObjectTypeFramebuffer, true,
                                   "VUID-VkCommandBufferBeginInfo-flags-00055", "VUID-VkCommandBufferInheritanceInfo-commonparent",
                                   inheritance_info_loc.dot(Field::framebuffer));
                skip |=
                    ValidateObject(begin_info->pInheritanceInfo->renderPass, kVulkanObjectTypeRenderPass, true,
                                   "VUID-VkCommandBufferBeginInfo-flags-06000", "VUID-VkCommandBufferInheritanceInfo-commonparent",
                                   inheritance_info_loc.dot(Field::renderPass));
            }
        }
    }
    return skip;
}

void Device::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                 VkImage *pSwapchainImages, const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    auto lock = WriteSharedLock();
    if (pSwapchainImages != NULL) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; i++) {
            CreateSwapchainImageObject(pSwapchainImages[i], swapchain, record_obj.location.dot(Field::pSwapchainImages, i));
        }
    }
}

bool Device::ValidateDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                   const Location &create_info_loc) const {
    bool skip = false;
    if (create_info.pBindings) {
        const char *parent_vuid = create_info_loc.function == vvl::Func::vkCreateDescriptorSetLayout
                                      ? "UNASSIGNED-vkCreateDescriptorSetLayout-pImmutableSamplers-device"
                                      : "UNASSIGNED-vkGetDescriptorSetLayoutSupport-pImmutableSamplers-device";
        for (uint32_t binding_index = 0; binding_index < create_info.bindingCount; ++binding_index) {
            const Location binding_loc = create_info_loc.dot(Field::pBindings, binding_index);
            const VkDescriptorSetLayoutBinding &binding = create_info.pBindings[binding_index];
            const bool is_sampler_type = binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                                         binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            if (binding.pImmutableSamplers && is_sampler_type) {
                for (uint32_t index2 = 0; index2 < binding.descriptorCount; ++index2) {
                    const VkSampler sampler = binding.pImmutableSamplers[index2];
                    skip |= ValidateObject(sampler, kVulkanObjectTypeSampler, false,
                                           "VUID-VkDescriptorSetLayoutBinding-descriptorType-00282", parent_vuid,
                                           binding_loc.dot(Field::pImmutableSamplers, index2));
                }
            }
        }
    }
    return skip;
}

bool Device::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateDescriptorSetLayout-device-parameter"
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool Device::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                          VkDescriptorSetLayoutSupport *pSupport,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorSetLayoutSupport-device-parameter"
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                     uint32_t *pQueueFamilyPropertyCount,
                                                                     VkQueueFamilyProperties *pQueueFamilyProperties,
                                                                     const ErrorObject &error_obj) const {
    constexpr bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter"
    return skip;
}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                    uint32_t *pQueueFamilyPropertyCount,
                                                                    VkQueueFamilyProperties *pQueueFamilyProperties,
                                                                    const RecordObject &record_obj) {}

void Instance::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                            VkInstance *pInstance, const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    tracker.CreateObject(*pInstance, kVulkanObjectTypeInstance, pAllocator, record_obj.location, *pInstance);
}

bool Instance::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                           const ErrorObject &error_obj) const {
    constexpr bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkCreateDevice-physicalDevice-parameter"
    return skip;
}

void Instance::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    tracker.CreateObject(*pDevice, kVulkanObjectTypeDevice, pAllocator, record_obj.location, physicalDevice);

    auto device_data = vvl::dispatch::GetData(*pDevice);
    auto object_tracking = static_cast<Device *>(device_data->GetValidationObject(container_type));

    const auto *robustness2_features = vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
    object_tracking->null_descriptor_enabled = robustness2_features && robustness2_features->nullDescriptor;
}

bool Device::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                   VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkAllocateCommandBuffers-device-parameter"

    skip |= ValidateObject(pAllocateInfo->commandPool, kVulkanObjectTypeCommandPool, false,
                           "VUID-VkCommandBufferAllocateInfo-commandPool-parameter", kVUIDUndefined,
                           error_obj.location.dot(Field::pAllocateInfo).dot(Field::commandPool));
    return skip;
}

void Device::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                  VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        AllocateCommandBuffer(pAllocateInfo->commandPool, pCommandBuffers[i], pAllocateInfo->level,
                              record_obj.location.dot(Field::pCommandBuffers, i));
    }
}

bool Device::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                   VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const {
    bool skip = false;
    auto lock = ReadSharedLock();
    // Checked by chassis: device: "VUID-vkAllocateDescriptorSets-device-parameter"

    const Location allocate_info = error_obj.location.dot(Field::pAllocateInfo);
    skip |= ValidateObject(pAllocateInfo->descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                           "VUID-VkDescriptorSetAllocateInfo-descriptorPool-parameter",
                           "VUID-VkDescriptorSetAllocateInfo-commonparent", allocate_info.dot(Field::descriptorPool));
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        skip |= ValidateObject(pAllocateInfo->pSetLayouts[i], kVulkanObjectTypeDescriptorSetLayout, false,
                               "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-parameter",
                               "VUID-VkDescriptorSetAllocateInfo-commonparent", allocate_info.dot(Field::pSetLayouts, i));
    }
    return skip;
}

void Device::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                  VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    auto lock = WriteSharedLock();
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        AllocateDescriptorSet(pAllocateInfo->descriptorPool, pDescriptorSets[i],
                              record_obj.location.dot(Field::pDescriptorSets, i));
    }
}

bool Device::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                               const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkFreeCommandBuffers-device-parameter"

    skip |= ValidateObject(commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkFreeCommandBuffers-commandPool-parameter",
                           "VUID-vkFreeCommandBuffers-commandPool-parent", error_obj.location.dot(Field::commandPool));
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        if (pCommandBuffers[i] != VK_NULL_HANDLE) {
            const Location command_buffer_loc = error_obj.location.dot(Field::pCommandBuffers, i);
            skip |= ValidateCommandBuffer(commandPool, pCommandBuffers[i], command_buffer_loc);
            skip |= ValidateDestroyObject(pCommandBuffers[i], kVulkanObjectTypeCommandBuffer, nullptr, kVUIDUndefined,
                                          kVUIDUndefined, command_buffer_loc);
        }
    }
    return skip;
}

void Device::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                             const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        RecordDestroyObject(pCommandBuffers[i], kVulkanObjectTypeCommandBuffer, record_obj.location);
    }
}

void Device::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    RecordDestroyObject(swapchain, kVulkanObjectTypeSwapchainKHR, record_obj.location);

    auto &image_map = tracker.object_map[kVulkanObjectTypeImage];
    auto snapshot = image_map.snapshot(
        [swapchain](const std::shared_ptr<ObjTrackState> &pNode) { return pNode->parent_object == HandleToUint64(swapchain); });
    for (const auto &itr : snapshot) {
        image_map.erase(itr.first);
    }
}

bool Device::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                               const VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj) const {
    auto lock = ReadSharedLock();
    bool skip = false;
    // Checked by chassis: device: "VUID-vkFreeDescriptorSets-device-parameter"

    skip |=
        ValidateObject(descriptorPool, kVulkanObjectTypeDescriptorPool, false, "VUID-vkFreeDescriptorSets-descriptorPool-parameter",
                       "VUID-vkFreeDescriptorSets-descriptorPool-parent", error_obj.location.dot(Field::descriptorPool));
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            const Location descriptor_sets_loc = error_obj.location.dot(Field::pDescriptorSets, i);
            skip |= ValidateDescriptorSet(descriptorPool, pDescriptorSets[i], descriptor_sets_loc);
            skip |= ValidateDestroyObject(pDescriptorSets[i], kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined, descriptor_sets_loc);
        }
    }
    return skip;
}
void Device::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                             const VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj) {
    auto lock = WriteSharedLock();
    std::shared_ptr<ObjTrackState> pool_node = nullptr;
    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        pool_node = itr->second;
    }
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        RecordDestroyObject(pDescriptorSets[i], kVulkanObjectTypeDescriptorSet, record_obj.location);
        if (pool_node) {
            pool_node->child_objects->erase(HandleToUint64(pDescriptorSets[i]));
        }
    }
}

bool Device::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                  const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    auto lock = ReadSharedLock();
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyDescriptorPool-device-parameter"

    const Location descriptor_pool_loc = error_obj.location.dot(Field::descriptorPool);
    skip |= ValidateObject(descriptorPool, kVulkanObjectTypeDescriptorPool, true,
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parameter",
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parent", descriptor_pool_loc);

    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pool_node = itr->second;
        for (auto set : *pool_node->child_objects) {
            skip |= ValidateDestroyObject((VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined, error_obj.location);
        }
    }
    skip |= ValidateDestroyObject(descriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator,
                                  "VUID-vkDestroyDescriptorPool-descriptorPool-00304",
                                  "VUID-vkDestroyDescriptorPool-descriptorPool-00305", descriptor_pool_loc);
    return skip;
}
void Device::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    auto lock = WriteSharedLock();
    auto itr = tracker.object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != tracker.object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pool_node = itr->second;
        for (auto set : *pool_node->child_objects) {
            RecordDestroyObject((VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, record_obj.location);
        }
        pool_node->child_objects->clear();
    }
    RecordDestroyObject(descriptorPool, kVulkanObjectTypeDescriptorPool, record_obj.location);
}

bool Device::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkDestroyCommandPool-device-parameter"

    const Location command_pool_loc = error_obj.location.dot(Field::commandPool);
    skip |= ValidateObject(commandPool, kVulkanObjectTypeCommandPool, true, "VUID-vkDestroyCommandPool-commandPool-parameter",
                           "VUID-vkDestroyCommandPool-commandPool-parent", command_pool_loc);

    auto snapshot = tracker.object_map[kVulkanObjectTypeCommandBuffer].snapshot(
        [commandPool](const std::shared_ptr<ObjTrackState> &pNode) { return pNode->parent_object == HandleToUint64(commandPool); });
    for (const auto &itr : snapshot) {
        auto node = itr.second;
        skip |= ValidateCommandBuffer(commandPool, reinterpret_cast<VkCommandBuffer>(itr.first), command_pool_loc);
        skip |= ValidateDestroyObject(reinterpret_cast<VkCommandBuffer>(itr.first), kVulkanObjectTypeCommandBuffer, nullptr,
                                      kVUIDUndefined, kVUIDUndefined, error_obj.location);
    }
    skip |=
        ValidateDestroyObject(commandPool, kVulkanObjectTypeCommandPool, pAllocator, "VUID-vkDestroyCommandPool-commandPool-00042",
                              "VUID-vkDestroyCommandPool-commandPool-00043", command_pool_loc);
    return skip;
}

void Device::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator,
                                             const RecordObject &record_obj) {
    auto snapshot = tracker.object_map[kVulkanObjectTypeCommandBuffer].snapshot(
        [commandPool](const std::shared_ptr<ObjTrackState> &pNode) { return pNode->parent_object == HandleToUint64(commandPool); });
    // A CommandPool's cmd buffers are implicitly deleted when pool is deleted. Remove this pool's cmdBuffers from cmd buffer map.
    for (const auto &itr : snapshot) {
        RecordDestroyObject(reinterpret_cast<VkCommandBuffer>(itr.first), kVulkanObjectTypeCommandBuffer, record_obj.location);
    }
    RecordDestroyObject(commandPool, kVulkanObjectTypeCommandPool, record_obj.location);
}

bool Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                      uint32_t *pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                      const ErrorObject &error_obj) const {
    constexpr bool skip = false;
    // Checked by chassis: physicalDevice: "VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter"
    return skip;
}

bool Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                         uint32_t *pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                         const ErrorObject &error_obj) const {
    return PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                                  error_obj);
}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                     uint32_t *pQueueFamilyPropertyCount,
                                                                     VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                     const RecordObject &record_obj) {}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                        uint32_t *pQueueFamilyPropertyCount,
                                                                        VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                        const RecordObject &record_obj) {}

void Instance::AllocateDisplayKHR(VkPhysicalDevice physical_device, VkDisplayKHR display, const Location &loc) {
    tracker.CreateObject(display, kVulkanObjectTypeDisplayKHR, nullptr, loc, physical_device);
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                   VkDisplayPropertiesKHR *pProperties,
                                                                   const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            AllocateDisplayKHR(physicalDevice, pProperties[i].display,
                               record_obj.location.dot(Field::pProperties, i).dot(Field::display));
        }
    }
}

void Instance::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                         uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties,
                                                         const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            tracker.CreateObject(pProperties[i].displayMode, kVulkanObjectTypeDisplayModeKHR, nullptr,
                                 record_obj.location.dot(Field::pProperties, i).dot(Field::displayMode), physicalDevice);
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                    VkDisplayProperties2KHR *pProperties,
                                                                    const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            AllocateDisplayKHR(
                physicalDevice, pProperties[index].displayProperties.display,
                record_obj.location.dot(Field::pProperties, index).dot(Field::displayProperties).dot(Field::display));
        }
    }
}

void Instance::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                          uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties,
                                                          const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            tracker.CreateObject(
                pProperties[index].displayModeProperties.displayMode, kVulkanObjectTypeDisplayModeKHR, nullptr,
                record_obj.location.dot(Field::pProperties, index).dot(Field::displayModeProperties).dot(Field::displayMode),
                physicalDevice);
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                        VkDisplayPlanePropertiesKHR *pProperties,
                                                                        const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            AllocateDisplayKHR(physicalDevice, pProperties[index].currentDisplay,
                               record_obj.location.dot(Field::pProperties, index).dot(Field::currentDisplay));
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                         VkDisplayPlaneProperties2KHR *pProperties,
                                                                         const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    if (pProperties) {
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            AllocateDisplayKHR(
                physicalDevice, pProperties[index].displayPlaneProperties.currentDisplay,
                record_obj.location.dot(Field::pProperties, index).dot(Field::displayPlaneProperties).dot(Field::currentDisplay));
        }
    }
}

bool Device::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                              const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateFramebuffer-device-parameter"

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateObject(pCreateInfo->renderPass, kVulkanObjectTypeRenderPass, false,
                           "VUID-VkFramebufferCreateInfo-renderPass-parameter", "VUID-VkFramebufferCreateInfo-commonparent",
                           create_info_loc.dot(Field::renderPass));
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        for (uint32_t index1 = 0; index1 < pCreateInfo->attachmentCount; ++index1) {
            skip |= ValidateObject(pCreateInfo->pAttachments[index1], kVulkanObjectTypeImageView, true,
                                   "VUID-VkFramebufferCreateInfo-flags-02778", "VUID-VkFramebufferCreateInfo-commonparent",
                                   create_info_loc.dot(Field::pAttachments, index1));
        }
    }

    return skip;
}

bool Device::PreCallValidateDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo,
                                                       const ErrorObject &error_obj) const {
    // Checked by chassis: device: "VUID-vkDebugMarkerSetObjectTagEXT-device-parameter"
    bool skip = false;
    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT) {
        skip |=
            LogError("VUID-VkDebugMarkerObjectTagInfoEXT-objectType-01493", device,
                     error_obj.location.dot(Field::pTagInfo).dot(Field::objectType), "is VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT.");
    } else {
        const auto object_type = ConvertDebugReportObjectToVulkanObject(pTagInfo->objectType);
        if (pTagInfo->object == (uint64_t)VK_NULL_HANDLE) {
            skip |= LogError("VUID-VkDebugMarkerObjectTagInfoEXT-object-01494", device,
                             error_obj.location.dot(Field::pTagInfo).dot(Field::object), "is VK_NULL_HANDLE.");
        } else if (!tracker.object_map[object_type].contains(pTagInfo->object)) {
            skip |= LogError("VUID-VkDebugMarkerObjectTagInfoEXT-object-01495", device,
                             error_obj.location.dot(Field::pTagInfo).dot(Field::objectType),
                             "(%s) doesn't match the object (0x%" PRIx64 ").",
                             string_VkDebugReportObjectTypeEXT(pTagInfo->objectType), pTagInfo->object);
        }
    }
    return skip;
}

bool Device::PreCallValidateDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo,
                                                        const ErrorObject &error_obj) const {
    // Checked by chassis: device: "VUID-vkDebugMarkerSetObjectNameEXT-device-parameter"
    bool skip = false;
    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT) {
        skip |= LogError("VUID-VkDebugMarkerObjectNameInfoEXT-objectType-01490", device,
                         error_obj.location.dot(Field::pNameInfo).dot(Field::objectType),
                         "is VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT.");
    } else {
        const auto object_type = ConvertDebugReportObjectToVulkanObject(pNameInfo->objectType);
        if (pNameInfo->object == (uint64_t)VK_NULL_HANDLE) {
            skip |= LogError("VUID-VkDebugMarkerObjectNameInfoEXT-object-01491", device,
                             error_obj.location.dot(Field::pNameInfo).dot(Field::object), "is VK_NULL_HANDLE.");
        } else if (!tracker.object_map[object_type].contains(pNameInfo->object)) {
            skip |= LogError("VUID-VkDebugMarkerObjectNameInfoEXT-object-01492", device,
                             error_obj.location.dot(Field::pNameInfo).dot(Field::objectType),
                             "(%s) doesn't match the object (0x%" PRIx64 ").",
                             string_VkDebugReportObjectTypeEXT(pNameInfo->objectType), pNameInfo->object);
        }
    }
    return skip;
}

bool Device::PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetDebugUtilsObjectNameEXT-device-parameter"
    const VkObjectType object_type = pNameInfo->objectType;
    const uint64_t object_handle = pNameInfo->objectHandle;

    if (IsInstanceVkObjectType(object_type)) {
        // TODO - need to check if device is from a valid instance/physical device
        // VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-07872 /  VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-07873
    } else if (object_type == VK_OBJECT_TYPE_DEVICE) {
        if (HandleToUint64(device) != object_handle) {
            skip |= LogError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-07874", device, error_obj.location.dot(Field::objectType),
                             "is VK_OBJECT_TYPE_DEVICE but objectHandle (0x%" PRIx64 ") != device (%s).", object_handle,
                             FormatHandle(device).c_str());
        }
    } else {
        skip |= ValidateAnonymousObject(object_handle, object_type, "VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02590",
                                        "VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-07874",
                                        error_obj.location.dot(Field::pNameInfo).dot(Field::objectHandle));
    }

    return skip;
}

bool Device::PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkSetDebugUtilsObjectTagEXT-device-parameter"

    const VkObjectType object_type = pTagInfo->objectType;
    const uint64_t object_handle = pTagInfo->objectHandle;

    if (IsInstanceVkObjectType(object_type)) {
        // TODO - need to check if device is from a valid instance/physical device
        // VUID-vkSetDebugUtilsObjectTagEXT-pNameInfo-07875 / VUID-vkSetDebugUtilsObjectTagEXT-pNameInfo-07876
    } else if (object_type == VK_OBJECT_TYPE_DEVICE) {
        if (HandleToUint64(device) != object_handle) {
            skip |= LogError("VUID-vkSetDebugUtilsObjectTagEXT-pNameInfo-07877", device,
                             error_obj.location.dot(Field::pTagInfo).dot(Field::objectType),
                             "is VK_OBJECT_TYPE_DEVICE but objectHandle (0x%" PRIx64 ") != device (%s).", object_handle,
                             FormatHandle(device).c_str());
        }
    } else {
        skip |= ValidateAnonymousObject(object_handle, object_type, "VUID-VkDebugUtilsObjectTagInfoEXT-objectHandle-01910",
                                        "VUID-vkSetDebugUtilsObjectTagEXT-pNameInfo-07877",
                                        error_obj.location.dot(Field::pTagInfo).dot(Field::objectHandle));
    }

    return skip;
}

bool Device::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateDescriptorUpdateTemplate-device-parameter"

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
        skip |= ValidateObject(pCreateInfo->descriptorSetLayout, kVulkanObjectTypeDescriptorSetLayout, false,
                               "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350",
                               "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent",
                               create_info_loc.dot(Field::descriptorSetLayout));
    }
    if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS) {
        skip |=
            ValidateObject(pCreateInfo->pipelineLayout, kVulkanObjectTypePipelineLayout, false,
                           "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352",
                           "VUID-VkDescriptorUpdateTemplateCreateInfo-commonparent", create_info_loc.dot(Field::pipelineLayout));
    }

    return skip;
}

bool Device::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                              const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                              const ErrorObject &error_obj) const {
    return PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, error_obj);
}

void Device::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                          const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;
    tracker.CreateObject(*pDescriptorUpdateTemplate, kVulkanObjectTypeDescriptorUpdateTemplate, pAllocator, record_obj.location,
                         device);
}

void Device::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                             const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                             const RecordObject &record_obj) {
    return PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

bool Device::ValidateAccelerationStructures(const char *src_handle_vuid, const char *dst_handle_vuid, uint32_t count,
                                            const VkAccelerationStructureBuildGeometryInfoKHR *infos, const Location &loc) const {
    bool skip = false;
    if (infos) {
        const char *device_vuid = "VUID-VkAccelerationStructureBuildGeometryInfoKHR-commonparent";
        for (uint32_t i = 0; i < count; ++i) {
            const Location info_loc = loc.dot(Field::pInfos, i);
            skip |= ValidateObject(infos[i].srcAccelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, true,
                                   src_handle_vuid, device_vuid, info_loc.dot(Field::srcAccelerationStructure));
            skip |= ValidateObject(infos[i].dstAccelerationStructure, kVulkanObjectTypeAccelerationStructureKHR, false,
                                   dst_handle_vuid, device_vuid, info_loc.dot(Field::dstAccelerationStructure));
        }
    }

    return skip;
}

bool Device::PreCallValidateCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBuildAccelerationStructuresKHR-commandBuffer-parameter"

    skip |= ValidateAccelerationStructures("VUID-vkCmdBuildAccelerationStructuresKHR-srcAccelerationStructure-04629",
                                           "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03800", infoCount,
                                           pInfos, error_obj.location);
    return skip;
}

bool Device::PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                      const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                      const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                      const uint32_t *pIndirectStrides,
                                                                      const uint32_t *const *ppMaxPrimitiveCounts,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: commandBuffer: "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-commandBuffer-parameter"

    skip |= ValidateAccelerationStructures("VUID-vkCmdBuildAccelerationStructuresIndirectKHR-srcAccelerationStructure-04629",
                                           "VUID-vkCmdBuildAccelerationStructuresIndirectKHR-dstAccelerationStructure-03800",
                                           infoCount, pInfos, error_obj.location);
    return skip;
}

bool Device::PreCallValidateBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                           uint32_t infoCount,
                                                           const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                           const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkBuildAccelerationStructuresKHR-device-parameter"

    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkBuildAccelerationStructuresKHR-deferredOperation-parameter",
                           "VUID-vkBuildAccelerationStructuresKHR-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    skip |= ValidateAccelerationStructures("VUID-vkBuildAccelerationStructuresKHR-srcAccelerationStructure-04629",
                                           "VUID-vkBuildAccelerationStructuresKHR-dstAccelerationStructure-03800", infoCount,
                                           pInfos, error_obj.location);
    return skip;
}

bool Device::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateRayTracingPipelinesKHR-device-parameter"

    skip |= ValidateObject(deferredOperation, kVulkanObjectTypeDeferredOperationKHR, true,
                           "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-parameter",
                           "VUID-vkCreateRayTracingPipelinesKHR-deferredOperation-parent",
                           error_obj.location.dot(Field::deferredOperation));
    skip |= ValidateObject(
        pipelineCache, kVulkanObjectTypePipelineCache, true, "VUID-vkCreateRayTracingPipelinesKHR-pipelineCache-parameter",
        "VUID-vkCreateRayTracingPipelinesKHR-pipelineCache-parent", error_obj.location.dot(Field::pipelineCache));
    if (pCreateInfos) {
        for (uint32_t index0 = 0; index0 < createInfoCount; ++index0) {
            const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, index0);
            if (pCreateInfos[index0].pStages) {
                for (uint32_t index1 = 0; index1 < pCreateInfos[index0].stageCount; ++index1) {
                    skip |= ValidateObject(pCreateInfos[index0].pStages[index1].module, kVulkanObjectTypeShaderModule, true,
                                           "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined,
                                           create_info_loc.dot(Field::pStages, index1).dot(Field::module));
                }
            }
            if (pCreateInfos[index0].pLibraryInfo) {
                if (pCreateInfos[index0].pLibraryInfo->pLibraries) {
                    for (uint32_t index2 = 0; index2 < pCreateInfos[index0].pLibraryInfo->libraryCount; ++index2) {
                        skip |= ValidateObject(pCreateInfos[index0].pLibraryInfo->pLibraries[index2], kVulkanObjectTypePipeline,
                                               false, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-parameter", kVUIDUndefined,
                                               create_info_loc.dot(Field::pLibraryInfo).dot(Field::pLibraries, index2));
                    }
                }
            }
            skip |= ValidateObject(pCreateInfos[index0].layout, kVulkanObjectTypePipelineLayout, false,
                                   "VUID-VkRayTracingPipelineCreateInfoKHR-layout-parameter",
                                   "VUID-VkRayTracingPipelineCreateInfoKHR-commonparent", create_info_loc.dot(Field::layout));
            if ((pCreateInfos[index0].flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) && (pCreateInfos[index0].basePipelineIndex == -1))
                skip |= ValidateObject(pCreateInfos[index0].basePipelineHandle, kVulkanObjectTypePipeline, false,
                                       "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07984",
                                       "VUID-VkRayTracingPipelineCreateInfoKHR-commonparent",
                                       create_info_loc.dot(Field::basePipelineHandle));
        }
    }

    return skip;
}

void Device::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                        const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                        std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    if (VK_ERROR_VALIDATION_FAILED_EXT == record_obj.result) return;
    if (pPipelines) {
        if (deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR) {
            auto register_fn = [this, pAllocator, record_obj, chassis_state](const std::vector<VkPipeline> &pipelines) {
                // Just need to capture chassis state to maintain pipeline creations parameters alive, see
                // https://vkdoc.net/chapters/deferred-host-operations#deferred-host-operations-requesting
                (void)chassis_state;
                for (VkPipeline pipe : pipelines) {
                    this->tracker.CreateObject(pipe, kVulkanObjectTypePipeline, pAllocator, record_obj.location, this->device);
                }
            };

            if (dispatch_device_->wrap_handles) {
                deferredOperation = dispatch_device_->Unwrap(deferredOperation);
            }
            std::vector<std::function<void(const std::vector<VkPipeline> &)>> cleanup_fn;
            auto find_res = dispatch_device_->deferred_operation_post_check.pop(deferredOperation);
            if (find_res->first) {
                cleanup_fn = std::move(find_res->second);
            }
            cleanup_fn.emplace_back(register_fn);
            dispatch_device_->deferred_operation_post_check.insert(deferredOperation, cleanup_fn);
        } else {
            for (uint32_t index = 0; index < createInfoCount; index++) {
                if (!pPipelines[index]) continue;
                tracker.CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator, record_obj.location, device);
            }
        }
    }
}
#ifdef VK_USE_PLATFORM_METAL_EXT
bool Device::PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkExportMetalObjectsEXT-device-parameter"

    const VkBaseOutStructure *metal_objects_info_ptr = reinterpret_cast<const VkBaseOutStructure *>(pMetalObjectsInfo->pNext);
    while (metal_objects_info_ptr) {
        switch (metal_objects_info_ptr->sType) {
            case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT: {
                auto metal_command_queue_ptr = reinterpret_cast<const VkExportMetalCommandQueueInfoEXT *>(metal_objects_info_ptr);
                skip |= ValidateObject(metal_command_queue_ptr->queue, kVulkanObjectTypeQueue, false,
                                       "VUID-VkExportMetalCommandQueueInfoEXT-queue-parameter", kVUIDUndefined, error_obj.location);
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
                auto metal_buffer_ptr = reinterpret_cast<const VkExportMetalBufferInfoEXT *>(metal_objects_info_ptr);
                skip |= ValidateObject(metal_buffer_ptr->memory, kVulkanObjectTypeDeviceMemory, false,
                                       "VUID-VkExportMetalBufferInfoEXT-memory-parameter", kVUIDUndefined, error_obj.location);
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
                auto metal_texture_ptr = reinterpret_cast<const VkExportMetalTextureInfoEXT *>(metal_objects_info_ptr);
                skip |= ValidateObject(metal_texture_ptr->image, kVulkanObjectTypeImage, true,
                                       "VUID-VkExportMetalTextureInfoEXT-image-parameter",
                                       "VUID-VkExportMetalTextureInfoEXT-commonparent", error_obj.location);
                skip |= ValidateObject(metal_texture_ptr->imageView, kVulkanObjectTypeImageView, true,
                                       "VUID-VkExportMetalTextureInfoEXT-imageView-parameter",
                                       "VUID-VkExportMetalTextureInfoEXT-commonparent", error_obj.location);
                skip |= ValidateObject(metal_texture_ptr->bufferView, kVulkanObjectTypeBufferView, true,
                                       "VUID-VkExportMetalTextureInfoEXT-bufferView-parameter",
                                       "VUID-VkExportMetalTextureInfoEXT-commonparent", error_obj.location);
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
                auto metal_iosurface_ptr = reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(metal_objects_info_ptr);
                skip |= ValidateObject(metal_iosurface_ptr->image, kVulkanObjectTypeImage, false,
                                       "VUID-VkExportMetalIOSurfaceInfoEXT-image-parameter", kVUIDUndefined, error_obj.location);
            } break;
            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
                auto metal_shared_event_ptr = reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(metal_objects_info_ptr);
                skip |= ValidateObject(metal_shared_event_ptr->semaphore, kVulkanObjectTypeSemaphore, true,
                                       "VUID-VkExportMetalSharedEventInfoEXT-semaphore-parameter",
                                       "VUID-VkExportMetalSharedEventInfoEXT-commonparent", error_obj.location);
                skip |= ValidateObject(metal_shared_event_ptr->event, kVulkanObjectTypeEvent, true,
                                       "VUID-VkExportMetalSharedEventInfoEXT-event-parameter",
                                       "VUID-VkExportMetalSharedEventInfoEXT-commonparent", error_obj.location);

            } break;
            default:
                break;
        }
        metal_objects_info_ptr = metal_objects_info_ptr->pNext;
    }
    return skip;
}
#endif  //  VK_USE_PLATFORM_METAL_EXT

bool Device::PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                             void *pDescriptor, const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkGetDescriptorEXT-device-parameter"
    skip |= ValidateObject(device, kVulkanObjectTypeDevice, false, kVUIDUndefined, kVUIDUndefined,
                           error_obj.location.dot(Field::device));

    return skip;
}

// Need to manually check if objectType and objectHandle are valid
bool Device::PreCallValidateSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                           VkPrivateDataSlot privateDataSlot, uint64_t data, const ErrorObject &error_obj) const {
    bool skip = false;

    if (IsInstanceVkObjectType(objectType) || objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError("VUID-vkSetPrivateData-objectHandle-04016", device, error_obj.location.dot(Field::objectType), "is %s.",
                         string_VkObjectType(objectType));
    } else if (objectType == VK_OBJECT_TYPE_DEVICE) {
        // Need to check device handle as has no parent to check as the caller is the same device object
        if (HandleToUint64(device) != objectHandle) {
            skip |= LogError("VUID-vkSetPrivateData-objectHandle-04016", device, error_obj.location.dot(Field::objectType),
                             "is VK_OBJECT_TYPE_DEVICE but objectHandle (0x%" PRIx64 ") != device (%s).", objectHandle,
                             FormatHandle(device).c_str());
        }
    } else {
        skip |= ValidateAnonymousObject(objectHandle, objectType, "VUID-vkSetPrivateData-objectHandle-04017",
                                        "VUID-vkSetPrivateData-objectHandle-04016", error_obj.location.dot(Field::objectHandle));
    }

    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkSetPrivateData-privateDataSlot-parameter",
                       "VUID-vkSetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

bool Device::PreCallValidateGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle,
                                           VkPrivateDataSlot privateDataSlot, uint64_t *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    if (IsInstanceVkObjectType(objectType) || objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError("VUID-vkGetPrivateData-objectType-04018", device, error_obj.location.dot(Field::objectType), "is %s.",
                         string_VkObjectType(objectType));
    } else if (objectType == VK_OBJECT_TYPE_DEVICE) {
        // Need to check device handle as has no parent to check as the caller is the same device object
        if (HandleToUint64(device) != objectHandle) {
            skip |= LogError("VUID-vkGetPrivateData-objectType-04018", device, error_obj.location.dot(Field::objectType),
                             "is VK_OBJECT_TYPE_DEVICE but objectHandle (0x%" PRIx64 ") != device (%s).", objectHandle,
                             FormatHandle(device).c_str());
        }
    } else {
        skip |= ValidateAnonymousObject(objectHandle, objectType, "VUID-vkGetPrivateData-objectHandle-09498",
                                        "VUID-vkGetPrivateData-objectType-04018", error_obj.location.dot(Field::objectHandle));
    }

    skip |=
        ValidateObject(privateDataSlot, kVulkanObjectTypePrivateDataSlot, false, "VUID-vkGetPrivateData-privateDataSlot-parameter",
                       "VUID-vkGetPrivateData-privateDataSlot-parent", error_obj.location.dot(Field::privateDataSlot));

    return skip;
}

void Device::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                   const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj) {
    if (VK_ERROR_VALIDATION_FAILED_EXT == record_obj.result) return;
    if (pPipelines) {
        for (uint32_t index = 0; index < createInfoCount; index++) {
            if (!pPipelines[index]) continue;
            tracker.CreateObject(pPipelines[index], kVulkanObjectTypePipeline, pAllocator,
                                 record_obj.location.dot(Field::pPipelines, index), device);

            if (auto pNext = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(pCreateInfos[index].pNext)) {
                if ((pNext->libraryCount > 0) && (pNext->pLibraries)) {
                    const uint64_t linked_handle = HandleToUint64(pPipelines[index]);
                    small_vector<std::shared_ptr<ObjTrackState>, 4> libraries;
                    for (uint32_t index2 = 0; index2 < pNext->libraryCount; ++index2) {
                        const uint64_t library_handle = HandleToUint64(pNext->pLibraries[index2]);
                        const auto &linked_pipeline = tracker.object_map[kVulkanObjectTypePipeline].find(library_handle);
                        libraries.emplace_back(linked_pipeline->second);
                    }
                    linked_graphics_pipeline_map.insert(linked_handle, libraries);
                }
            }
        }
    }
}

void Device::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                          const RecordObject &record_obj) {
    RecordDestroyObject(pipeline, kVulkanObjectTypePipeline, record_obj.location);

    linked_graphics_pipeline_map.erase(HandleToUint64(pipeline));
}

bool Device::PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkIndirectExecutionSetEXT *pIndirectExecutionSet,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkCreateIndirectExecutionSetEXT-device-parameter"

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const Location info_loc = create_info_loc.dot(Field::info);
    if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT && pCreateInfo->info.pPipelineInfo) {
        [[maybe_unused]] const Location pipeline_info_loc = info_loc.dot(Field::pPipelineInfo);
        skip |= ValidateObject(pCreateInfo->info.pPipelineInfo->initialPipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkIndirectExecutionSetPipelineInfoEXT-initialPipeline-parameter", kVUIDUndefined,
                               pipeline_info_loc.dot(Field::initialPipeline));
    }

    if (pCreateInfo->type == VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT && pCreateInfo->info.pShaderInfo) {
        const VkIndirectExecutionSetShaderInfoEXT &shader_info = *pCreateInfo->info.pShaderInfo;
        const Location shader_info_loc = info_loc.dot(Field::pShaderInfo);
        if (shader_info.pSetLayoutInfos && shader_info.pInitialShaders) {
            for (uint32_t i = 0; i < shader_info.shaderCount; ++i) {
                skip |= ValidateObject(shader_info.pInitialShaders[i], kVulkanObjectTypeShaderEXT, false,
                                       "VUID-VkIndirectExecutionSetShaderInfoEXT-pInitialShaders-parameter", kVUIDUndefined,
                                       shader_info_loc.dot(Field::pInitialShaders, i));

                const Location set_layout_info_loc = shader_info_loc.dot(Field::pSetLayoutInfos, i);
                const VkIndirectExecutionSetShaderLayoutInfoEXT &layout_info = shader_info.pSetLayoutInfos[i];
                if ((layout_info.setLayoutCount > 0) && (layout_info.pSetLayouts)) {
                    for (uint32_t layout_index = 0; layout_index < layout_info.setLayoutCount; ++layout_index) {
                        skip |= ValidateObject(layout_info.pSetLayouts[layout_index], kVulkanObjectTypeDescriptorSetLayout, true,
                                               "VUID-VkIndirectExecutionSetShaderLayoutInfoEXT-pSetLayouts-parameter",
                                               "UNASSIGNED-VkIndirectExecutionSetShaderLayoutInfoEXT-pSetLayouts-parent",
                                               set_layout_info_loc.dot(Field::pSetLayouts, layout_index));
                    }
                }
            }
        }
    }

    return skip;
}

bool Device::PreCallValidateReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    // Checked by chassis: device: "VUID-vkReleaseCapturedPipelineDataKHR-device-parameter"

    if (pInfo) {
        const Location pInfo_loc = error_obj.location.dot(Field::pInfo);
        skip |= ValidateObject(pInfo->pipeline, kVulkanObjectTypePipeline, false,
                               "VUID-VkReleaseCapturedPipelineDataInfoKHR-pipeline-parameter",
                               "UNASSIGNED-VkReleaseCapturedPipelineDataInfoKHR-pipeline-parent", pInfo_loc.dot(Field::pipeline));

        skip |= ValidateDestroyObject(pInfo->pipeline, kVulkanObjectTypePipeline, pAllocator,
                                      "VUID-vkReleaseCapturedPipelineDataKHR-pipeline-09611",
                                      "VUID-vkReleaseCapturedPipelineDataKHR-pipeline-09612", pInfo_loc.dot(Field::pipeline));
    }

    return skip;
}

void Device::PostCallRecordCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkPipelineBinaryHandlesInfoKHR *pBinaries, const RecordObject &record_obj) {
    if (record_obj.result < VK_SUCCESS) return;

    if (pBinaries->pPipelineBinaries) {
        for (uint32_t index = 0; index < pBinaries->pipelineBinaryCount; index++) {
            tracker.CreateObject(pBinaries->pPipelineBinaries[index], kVulkanObjectTypePipelineBinaryKHR, pAllocator,
                                 record_obj.location, device);
        }
    }
}
}  // namespace object_lifetimes
