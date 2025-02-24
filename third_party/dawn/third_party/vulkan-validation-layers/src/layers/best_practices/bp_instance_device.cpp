/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "best_practices/best_practices_validation.h"
#include "generated/dispatch_functions.h"
#include "best_practices/bp_state.h"
#include "state_tracker/queue_state.h"

bool bp_state::Instance::ValidateDeprecatedExtensions(const Location& loc, vvl::Extension extension, APIVersion version) const {
    bool skip = false;
    const auto dep_info = GetDeprecatedData(extension);
    if (dep_info.reason != DeprecationReason::Empty) {
        auto reason_to_string = [](DeprecationReason reason) {
            switch (reason) {
                case DeprecationReason::Promoted:
                    return "promoted to";
                case DeprecationReason::Obsoleted:
                    return "obsoleted by";
                case DeprecationReason::Deprecated:
                    return "deprecated by";
                default:
                    return "";
            }
        };

        const char* vuid = "BestPractices-deprecated-extension";
        if ((dep_info.target.version == vvl::Version::_VK_VERSION_1_1 && (version >= VK_API_VERSION_1_1)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_2 && (version >= VK_API_VERSION_1_2)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_3 && (version >= VK_API_VERSION_1_3)) ||
            (dep_info.target.version == vvl::Version::_VK_VERSION_1_4 && (version >= VK_API_VERSION_1_4))) {
            skip |=
                LogWarning(vuid, instance, loc, "Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                           String(extension), reason_to_string(dep_info.reason), String(dep_info.target).c_str());
        } else if (dep_info.target.version == vvl::Version::Empty) {
            if (dep_info.target.extension == vvl::Extension::Empty) {
                skip |= LogWarning(vuid, instance, loc,
                                   "Attempting to enable deprecated extension %s, but this extension has been deprecated "
                                   "without replacement.",
                                   String(extension));
            } else {
                skip |= LogWarning(vuid, instance, loc,
                                   "Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                                   String(extension), reason_to_string(dep_info.reason), String(dep_info.target).c_str());
            }
        }
    }
    return skip;
}

bool bp_state::Instance::ValidateSpecialUseExtensions(const Location& loc, vvl::Extension extension) const {
    bool skip = false;
    const std::string special_uses = GetSpecialUse(extension);

    if (!special_uses.empty()) {
        const char* const format =
            "Attempting to enable extension %s, but this extension is intended to support %s "
            "and it is strongly recommended that it be otherwise avoided.";
        const char* vuid = "BestPractices-specialuse-extension";
        if (special_uses.find("cadsupport") != std::string::npos) {
            skip |= LogWarning(vuid, instance, loc, format, String(extension),
                               "specialized functionality used by CAD/CAM applications");
        }
        if (special_uses.find("d3demulation") != std::string::npos) {
            skip |= LogWarning(vuid, instance, loc, format, String(extension),
                               "D3D emulation layers, and applications ported from D3D, by adding functionality specific to D3D");
        }
        if (special_uses.find("devtools") != std::string::npos) {
            skip |= LogWarning(vuid, instance, loc, format, String(extension), "developer tools such as capture-replay libraries");
        }
        if (special_uses.find("debugging") != std::string::npos) {
            skip |= LogWarning(vuid, instance, loc, format, String(extension), "use by applications when debugging");
        }
        if (special_uses.find("glemulation") != std::string::npos) {
            skip |= LogWarning(
                vuid, instance, loc, format, String(extension),
                "OpenGL and/or OpenGL ES emulation layers, and applications ported from those APIs, by adding functionality "
                "specific to those APIs");
        }
    }
    return skip;
}

bool bp_state::Instance::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkInstance* pInstance,
                                                       const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        if (IsDeviceExtension(extension)) {
            skip |= LogWarning("BestPractices-vkCreateInstance-extension-mismatch", instance, error_obj.location,
                               "Attempting to enable Device Extension %s at CreateInstance time.", String(extension));
        }
        uint32_t specified_version =
            (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
        skip |= ValidateDeprecatedExtensions(error_obj.location, extension, specified_version);
        skip |= ValidateSpecialUseExtensions(error_obj.location, extension);
    }

    return skip;
}

bool bp_state::Instance::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                                     const ErrorObject& error_obj) const {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    auto device_api_version = physical_device_properties.apiVersion;

    // Check api versions and log an info message when instance api Version is higher than version on device.
    if (api_version > device_api_version) {
        std::string inst_api_name = StringAPIVersion(api_version);
        std::string dev_api_name = StringAPIVersion(device_api_version);

        LogInfo("BestPractices-vkCreateDevice-API-version-mismatch", instance, error_obj.location,
                "API Version of current instance, %s is higher than API Version on device, %s", inst_api_name.c_str(),
                dev_api_name.c_str());
    }

    std::vector<std::string> extension_names;
    {
        uint32_t property_count = 0;
        if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, nullptr) == VK_SUCCESS) {
            std::vector<VkExtensionProperties> property_list(property_count);
            if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, property_list.data()) ==
                VK_SUCCESS) {
                extension_names.reserve(property_list.size());
                for (const VkExtensionProperties& properties : property_list) {
                    extension_names.emplace_back(properties.extensionName);
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        const char* extension_name = pCreateInfo->ppEnabledExtensionNames[i];

        APIVersion extension_api_version = std::min(api_version, APIVersion(device_api_version));

        vvl::Extension extension = GetExtension(extension_name);
        if (IsInstanceExtension(extension)) {
            skip |= LogWarning("BestPractices-vkCreateDevice-extension-mismatch", instance, error_obj.location,
                               "Attempting to enable Instance Extension %s at CreateDevice time.", String(extension));
            extension_api_version = api_version;
        }

        skip |= ValidateDeprecatedExtensions(error_obj.location, extension, extension_api_version);
        skip |= ValidateSpecialUseExtensions(error_obj.location, extension);
    }

    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state && (bp_pd_state->vkGetPhysicalDeviceFeaturesState == UNCALLED) && (pCreateInfo->pEnabledFeatures != NULL)) {
        skip |= LogWarning("BestPractices-vkCreateDevice-physical-device-features-not-retrieved", instance, error_obj.location,
                           "called before getting physical device features from vkGetPhysicalDeviceFeatures().");
    }

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorIMG)) &&
        (pCreateInfo->pEnabledFeatures != nullptr) && (pCreateInfo->pEnabledFeatures->robustBufferAccess == VK_TRUE)) {
        skip |= LogPerformanceWarning(
            "BestPractices-vkCreateDevice-RobustBufferAccess", instance, error_obj.location,
            "%s %s %s: called with enabled robustBufferAccess. Use robustBufferAccess as a debugging tool during "
            "development. Enabling it causes loss in performance for accesses to uniform buffers and shader storage "
            "buffers. Disable robustBufferAccess in release builds. Only leave it enabled if the application use-case "
            "requires the additional level of reliability due to the use of unverified user-supplied draw parameters.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorIMG));
    }

    const bool enabled_pageable_device_local_memory = IsExtEnabled(extensions.vk_ext_pageable_device_local_memory);
    if (VendorCheckEnabled(kBPVendorNVIDIA) && !enabled_pageable_device_local_memory &&
        std::find(extension_names.begin(), extension_names.end(), VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME) !=
            extension_names.end()) {
        skip |=
            LogPerformanceWarning("BestPractices-NVIDIA-CreateDevice-PageableDeviceLocalMemory", instance, error_obj.location,
                                  "%s called without pageable device local memory. "
                                  "Use pageableDeviceLocalMemory from VK_EXT_pageable_device_local_memory when it is available.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
    }

    return skip;
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
bool bp_state::Instance::ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const vvl::PhysicalDevice& bp_pd_state,
                                                                              uint32_t requested_queue_family_property_count,
                                                                              const CALL_STATE call_state,
                                                                              const Location& loc) const {
    bool skip = false;
    if (bp_pd_state.queue_family_known_count != requested_queue_family_property_count) {
        skip |= LogWarning("BestPractices-GetPhysicalDeviceQueueFamilyProperties-CountMismatch", bp_pd_state.Handle(), loc,
                           "is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                           ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                           ". It is recommended to instead receive all the properties by calling %s with "
                           "pQueueFamilyPropertyCount that was "
                           "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                           requested_queue_family_property_count, bp_pd_state.queue_family_known_count, loc.StringFunc(),
                           loc.StringFunc());
    }
    return skip;
}

bool bp_state::Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                               uint32_t* pQueueFamilyPropertyCount,
                                                                               VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                               const ErrorObject& error_obj) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(*bp_pd_state, *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                    error_obj.location);
    }
    return false;
}

bool bp_state::Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                                uint32_t* pQueueFamilyPropertyCount,
                                                                                VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                                const ErrorObject& error_obj) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(*bp_pd_state, *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                    error_obj.location);
    }
    return false;
}

bool bp_state::Instance::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                                   const ErrorObject& error_obj) const {
    return PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                                  error_obj);
}

void bp_state::Instance::CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(CALL_STATE& call_state, bool no_pointer) {
    if (no_pointer) {
        if (UNCALLED == call_state) {
            call_state = QUERY_COUNT;
        }
    } else {  // Save queue family properties
        call_state = QUERY_DETAILS;
    }
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pQueueFamilyPropertyCount,
                                                                              VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                              const RecordObject& record_obj) {
    BaseClass::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                                 pQueueFamilyProperties, record_obj);
    if (auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice)) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                               uint32_t* pQueueFamilyPropertyCount,
                                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                               const RecordObject& record_obj) {
    BaseClass::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                  pQueueFamilyProperties, record_obj);
    if (auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice)) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                                  const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                          record_obj);
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceFeatures* pFeatures,
                                                                 const RecordObject& record_obj) {
    BaseClass::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures, record_obj);
    if (auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice)) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice,
                                                                  VkPhysicalDeviceFeatures2* pFeatures,
                                                                  const RecordObject& record_obj) {
    BaseClass::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures, record_obj);
    if (auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice)) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void bp_state::Instance::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                     VkPhysicalDeviceFeatures2* pFeatures,
                                                                     const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures, record_obj);
}

void BestPractices::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                             const RecordObject& record_obj) {
    BaseClass::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    auto queue_state = Get<vvl::Queue>(queue);
    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const auto& submit_info = pSubmits[submit];
        for (uint32_t cb_index = 0; cb_index < submit_info.commandBufferCount; cb_index++) {
            auto cb = GetWrite<bp_state::CommandBuffer>(submit_info.pCommandBuffers[cb_index]);
            for (auto& func : cb->queue_submit_functions) {
                func(*this, *queue_state, *cb);
            }
            cb->num_submits++;
        }
    }
}

namespace {
struct EventValidator {
    const vvl::Device& state_tracker;

    vvl::unordered_map<VkEvent, bool> signaling_state;

    EventValidator(const vvl::Device& state_tracker) : state_tracker(state_tracker) {}

    bool ValidateSubmittedCbSignalingState(const bp_state::CommandBuffer& cb, const Location& cb_loc) {
        bool skip = false;
        for (const auto& [event, info] : cb.event_signaling_state) {
            if (info.first_state_change_is_signal) {
                bool signaled = false;
                if (auto* p_signaled = vvl::Find(signaling_state, event)) {
                    // check local tracking map
                    signaled = *p_signaled;
                } else {
                    // check global event state
                    auto event_state = state_tracker.Get<vvl::Event>(event);
                    if (event_state) {
                        signaled = event_state->signaled;
                    }
                }
                if (signaled) {
                    const LogObjectList objlist(cb.VkHandle(), event);
                    skip |= state_tracker.LogWarning(
                        "BestPractices-Event-SignalSignaledEvent", objlist, cb_loc,
                        "%s sets event %s which is already in the signaled state (set by previously submitted command buffers or "
                        "from the host). If this is not the desired behavior, the event must be reset before it is set again.",
                        state_tracker.FormatHandle(cb.VkHandle()).c_str(), state_tracker.FormatHandle(event).c_str());
                }
            }
            signaling_state[event] = info.signaled;
        }
        return skip;
    }
};
}  // namespace

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                               const ErrorObject& error_obj) const {
    bool skip = false;
    EventValidator event_validator(*this);

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const Location submit_loc = error_obj.location.dot(Field::pSubmits, submit);
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags(queue, submit_loc.dot(Field::pWaitDstStageMask, semaphore),
                                            pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
        if (pSubmits[submit].signalSemaphoreCount == 0 && pSubmits[submit].pSignalSemaphores != nullptr) {
            LogInfo("BestPractices-SignalSemaphores-SemaphoreCount", queue, submit_loc.dot(Field::pSignalSemaphores),
                    "is set, but pSubmits[%" PRIu32 "].signalSemaphoreCount is 0.", submit);
        }
        if (pSubmits[submit].waitSemaphoreCount == 0 && pSubmits[submit].pWaitSemaphores != nullptr) {
            LogInfo("BestPractices-WaitSemaphores-SemaphoreCount", queue, submit_loc.dot(Field::pWaitSemaphores),
                    "is set, but pSubmits[%" PRIu32 "].waitSemaphoreCount is 0.", submit);
        }
        for (uint32_t cb_index = 0; cb_index < pSubmits[submit].commandBufferCount; cb_index++) {
            if (auto cb_state = GetRead<bp_state::CommandBuffer>(pSubmits[submit].pCommandBuffers[cb_index])) {
                const Location cb_loc = submit_loc.dot(vvl::Field::pCommandBuffers, cb_index);
                skip |= event_validator.ValidateSubmittedCbSignalingState(*cb_state, cb_loc);
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                                   VkFence fence, const ErrorObject& error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool BestPractices::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                const ErrorObject& error_obj) const {
    bool skip = false;
    EventValidator event_validator(*this);

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const Location submit_loc = error_obj.location.dot(Field::pSubmits, submit);
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreInfoCount; semaphore++) {
            const Location semaphore_loc = submit_loc.dot(Field::pWaitSemaphoreInfos, semaphore);
            skip |= CheckPipelineStageFlags(queue, semaphore_loc.dot(Field::stageMask),
                                            pSubmits[submit].pWaitSemaphoreInfos[semaphore].stageMask);
        }
        for (uint32_t cb_index = 0; cb_index < pSubmits[submit].commandBufferInfoCount; cb_index++) {
            if (auto cb_state = GetRead<bp_state::CommandBuffer>(pSubmits[submit].pCommandBufferInfos[cb_index].commandBuffer)) {
                const Location infos_loc = submit_loc.dot(vvl::Field::pCommandBufferInfos, cb_index);
                const Location cb_loc = infos_loc.dot(vvl::Field::commandBuffer);
                skip |= event_validator.ValidateSubmittedCbSignalingState(*cb_state, cb_loc);
            }
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                   VkFence fence, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const Location bind_info_loc = error_obj.location.dot(Field::pBindInfo, bind_idx);
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        vvl::unordered_set<const vvl::Image*> sparse_images;
        // Track images getting metadata bound by this call in a set, it'll be recorded into the image_state
        // in RecordQueueBindSparse.
        vvl::unordered_set<const vvl::Image*> sparse_images_with_metadata;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bind_info.imageBindCount; ++i) {
            const auto& image_bind = bind_info.pImageBinds[i];
            auto image_state = Get<vvl::Image>(image_bind.image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->sparse_residency) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning("BestPractices-vkQueueBindSparse-image-requirements2", image_state->Handle(),
                                       bind_info_loc.dot(Field::pImageBinds, i),
                                       "Binding sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       FormatHandle(image_state->Handle()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning("BestPractices-vkQueueBindSparse-image-requirements", image_state->Handle(),
                                   bind_info_loc.dot(Field::pImageBinds, i),
                                   "Binding sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   FormatHandle(image_state->Handle()).c_str());
            }
        }
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<vvl::Image>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->sparse_residency) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning("BestPractices-vkQueueBindSparse-image-opaque-requirements2", image_state->Handle(),
                                       bind_info_loc.dot(Field::pImageOpaqueBinds, i),
                                       "Binding opaque sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       FormatHandle(image_state->Handle()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning("BestPractices-vkQueueBindSparse-image-opaque-requirements", image_state->Handle(),
                                   bind_info_loc.dot(Field::pImageOpaqueBinds, i),
                                   "Binding opaque sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   FormatHandle(image_state->Handle()).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    sparse_images_with_metadata.insert(image_state.get());
                }
            }
        }
        for (const auto& sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound &&
                sparse_images_with_metadata.find(sparse_image_state) == sparse_images_with_metadata.end()) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                skip |= LogWarning("BestPractices-vkQueueBindSparse-image-metadata-requirements", sparse_image_state->Handle(),
                                   bind_info_loc,
                                   "Binding sparse memory to %s which requires a metadata aspect but no "
                                   "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                                   FormatHandle(sparse_image_state->Handle()).c_str());
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto queue_state = Get<vvl::Queue>(queue);
        if (queue_state &&
            queue_state->queue_family_properties.queueFlags != (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) {
            skip |= LogPerformanceWarning("BestPractices-NVIDIA-QueueBindSparse-NotAsync", queue, error_obj.location,
                                          "issued on queue %s. All binds should happen on an asynchronous copy "
                                          "queue to hide the OS scheduling and submit costs.",
                                          FormatHandle(queue).c_str());
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                        VkFence fence, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<vvl::Image>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
    }
}

void BestPractices::ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                                    VkFence fence, const RecordObject& record_obj) {
    // AMD best practice
    num_queue_submissions_ += submitCount;
}

std::shared_ptr<vvl::PhysicalDevice> bp_state::Instance::CreatePhysicalDeviceState(VkPhysicalDevice handle) {
    return std::static_pointer_cast<vvl::PhysicalDevice>(std::make_shared<bp_state::PhysicalDevice>(handle));
}

bp_state::PhysicalDevice* BestPractices::GetPhysicalDeviceState() {
    return static_cast<bp_state::PhysicalDevice*>(physical_device_state);
}
const bp_state::PhysicalDevice* BestPractices::GetPhysicalDeviceState() const {
    return static_cast<const bp_state::PhysicalDevice*>(physical_device_state);
}
