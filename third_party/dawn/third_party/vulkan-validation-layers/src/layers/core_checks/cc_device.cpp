/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * This file deals with anything related to Phyiscal Devices, Logical Devices, or Device Queues Families, Device Masks, etc
 */

#include <fstream>
#include <vector>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
#include <unistd.h>
#endif

#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "state_tracker/image_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/render_pass_state.h"
#include <spirv-tools/libspirv.h>
#include "generated/dispatch_functions.h"

bool CoreChecks::ValidateDeviceQueueFamily(uint32_t queue_family, const Location &loc, const char *vuid,
                                           bool optional = false) const {
    bool skip = false;
    if (!optional && queue_family == VK_QUEUE_FAMILY_IGNORED) {
        skip |= LogError(vuid, device, loc,
                         "is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.");
    } else if (queue_family_index_set.find(queue_family) == queue_family_index_set.end()) {
        skip |=
            LogError(vuid, device, loc,
                     "(%" PRIu32
                     ") is not one of the queue families given via VkDeviceQueueCreateInfo structures when the device was created.",
                     queue_family);
    }

    return skip;
}

// Validate the specified queue families against the families supported by the physical device that owns this device
bool CoreChecks::ValidatePhysicalDeviceQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families,
                                                     const Location &loc, const char *vuid) const {
    bool skip = false;
    if (queue_families) {
        vvl::unordered_set<uint32_t> set;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (set.count(queue_families[i])) {
                skip |= LogError(vuid, device, loc.dot(Field::pQueueFamilyIndices, i),
                                 "(%" PRIu32 ") is also in pQueueFamilyIndices[0].", queue_families[i]);
            } else {
                set.insert(queue_families[i]);
                if (queue_families[i] == VK_QUEUE_FAMILY_IGNORED) {
                    skip |= LogError(vuid, device, loc.dot(Field::pQueueFamilyIndices, i),
                                     "is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.");
                } else if (queue_families[i] >= physical_device_state->queue_family_known_count) {
                    const LogObjectList objlist(physical_device, device);
                    skip |=
                        LogError(vuid, objlist, loc.dot(Field::pQueueFamilyIndices, i),
                                 "(%" PRIu32
                                 ") is not one of the queue families supported by the parent PhysicalDevice %s of this device %s.",
                                 queue_families[i], FormatHandle(physical_device).c_str(), FormatHandle(device).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::GetPhysicalDeviceImageFormatProperties(vvl::Image &image_state, const char *vuid_string,
                                                        const Location &loc) const {
    bool skip = false;
    const auto image_create_info = image_state.create_info;
    VkResult image_properties_result = VK_SUCCESS;
    Func command = Func::vkGetPhysicalDeviceImageFormatProperties;
    if (image_create_info.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        image_properties_result = DispatchGetPhysicalDeviceImageFormatProperties(
            physical_device, image_create_info.format, image_create_info.imageType, image_create_info.tiling,
            image_create_info.usage, image_create_info.flags, &image_state.image_format_properties);
    } else {
        command = Func::vkGetPhysicalDeviceImageFormatProperties2;
        VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
        image_format_info.type = image_create_info.imageType;
        image_format_info.format = image_create_info.format;
        image_format_info.tiling = image_create_info.tiling;
        image_format_info.usage = image_create_info.usage;
        image_format_info.flags = image_create_info.flags;
        VkImageFormatProperties2 image_format_properties = vku::InitStructHelper();
        image_properties_result = DispatchGetPhysicalDeviceImageFormatProperties2Helper(
            api_version, physical_device, &image_format_info, &image_format_properties);
        image_state.image_format_properties = image_format_properties.imageFormatProperties;
    }
    if (image_properties_result != VK_SUCCESS) {
        skip |= LogError(vuid_string, device, loc,
                         "internal call to %s unexpectedly "
                         "failed with result = %s, "
                         "when called for validation with following params: "
                         "format: %s, imageType: %s, "
                         "tiling: %s, usage: %s, "
                         "flags: %s.",
                         String(command), string_VkResult(image_properties_result), string_VkFormat(image_create_info.format),
                         string_VkImageType(image_create_info.imageType), string_VkImageTiling(image_create_info.tiling),
                         string_VkImageUsageFlags(image_create_info.usage).c_str(),
                         string_VkImageCreateFlags(image_create_info.flags).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const LogObjectList &objlist, const Location loc,
                                                         const char *vuid) const {
    bool skip = false;
    uint32_t count = 1 << physical_device_count;
    if (count <= deviceMask) {
        skip |= LogError(vuid, objlist, loc, "(0x%" PRIx32 ") is invalid, Physical device count is %" PRIu32 ".", deviceMask,
                         physical_device_count);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToZero(uint32_t deviceMask, const LogObjectList &objlist, const Location loc,
                                          const char *vuid) const {
    bool skip = false;
    if (deviceMask == 0) {
        skip |= LogError(vuid, objlist, loc, "is zero.");
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToCommandBuffer(const vvl::CommandBuffer &cb_state, uint32_t deviceMask,
                                                   const LogObjectList &objlist, const Location loc, const char *vuid) const {
    bool skip = false;
    if ((deviceMask & cb_state.initial_device_mask) != deviceMask) {
        skip |= LogError(vuid, objlist, loc, "(0x%" PRIx32 ") is not a subset of %s initial device mask (0x%" PRIx32 ").",
                         deviceMask, FormatHandle(cb_state).c_str(), cb_state.initial_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToRenderPass(const vvl::CommandBuffer &cb_state, uint32_t deviceMask, const Location loc,
                                                const char *vuid) const {
    bool skip = false;
    if (cb_state.active_render_pass && ((deviceMask & cb_state.render_pass_device_mask) != deviceMask)) {
        skip |= LogError(vuid, cb_state.Handle(), loc, "(0x%" PRIx32 ") is not a subset of %s device mask (0x%" PRIx32 ").",
                         deviceMask, FormatHandle(cb_state.active_render_pass->Handle()).c_str(), cb_state.render_pass_device_mask);
    }
    return skip;
}

bool core::Instance::ValidateQueueFamilyIndex(const vvl::PhysicalDevice &pd_state, uint32_t requested_queue_family,
                                              const char *vuid, const Location &loc) const {
    bool skip = false;

    if (requested_queue_family >= pd_state.queue_family_known_count) {
        const char *conditional_ext_cmd =
            extensions.vk_khr_get_physical_device_properties2 ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]" : "";

        skip |= LogError(vuid, pd_state.Handle(), loc,
                         "(%" PRIu32 ") is not less than any previously obtained pQueueFamilyPropertyCount %" PRIu32
                         " from "
                         "vkGetPhysicalDeviceQueueFamilyProperties%s.",
                         requested_queue_family, pd_state.queue_family_known_count, conditional_ext_cmd);
    }
    return skip;
}

bool core::Instance::ValidateDeviceQueueCreateInfos(const vvl::PhysicalDevice &pd_state, uint32_t info_count,
                                                    const VkDeviceQueueCreateInfo *infos, const Location &loc) const {
    bool skip = false;

    const uint32_t not_used = std::numeric_limits<uint32_t>::max();
    struct create_flags {
        // uint32_t is to represent the queue family index to allow for better error messages
        uint32_t unprocted_index;
        uint32_t protected_index;
        create_flags(uint32_t a, uint32_t b) : unprocted_index(a), protected_index(b) {}
    };
    vvl::unordered_map<uint32_t, create_flags> queue_family_map;
    vvl::unordered_map<uint32_t, VkQueueGlobalPriority> global_priorities;

    std::vector<uint32_t> queue_counts;
    for (uint32_t i = 0; i < info_count; ++i) {
        const Location info_loc = loc.dot(Field::pQueueCreateInfos, i);
        const uint32_t requested_queue_family = infos[i].queueFamilyIndex;
        const bool protected_create_bit = (infos[i].flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) != 0;

        skip |= ValidateQueueFamilyIndex(pd_state, requested_queue_family, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                         info_loc.dot(Field::queueFamilyIndex));
        if (skip) {  // Skip if queue family index is invalid, as it will be used as index in arrays
            continue;
        }

        if (api_version == VK_API_VERSION_1_0) {
            // Vulkan 1.0 didn't have protected memory so always needed unique info
            create_flags flags = {requested_queue_family, not_used};
            if (queue_family_map.emplace(requested_queue_family, flags).second == false) {
                skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state.Handle(),
                                 info_loc.dot(Field::queueFamilyIndex),
                                 "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32 "].",
                                 requested_queue_family, queue_family_map.at(requested_queue_family).unprocted_index);
            }
        } else {
            // Vulkan 1.1 and up can have 2 queues be same family index if one is protected and one isn't
            auto it = queue_family_map.find(requested_queue_family);
            if (it == queue_family_map.end()) {
                // Add first time seeing queue family index and what the create flags were
                create_flags new_flags = {not_used, not_used};
                if (protected_create_bit) {
                    new_flags.protected_index = requested_queue_family;
                } else {
                    new_flags.unprocted_index = requested_queue_family;
                }
                queue_family_map.emplace(requested_queue_family, new_flags);
            } else {
                // The queue family was seen, so now need to make sure the flags were different
                if (protected_create_bit) {
                    if (it->second.protected_index != not_used) {
                        skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state.Handle(),
                                         info_loc.dot(Field::queueFamilyIndex),
                                         "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32
                                         "] which both have "
                                         "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT.",
                                         requested_queue_family, queue_family_map.at(requested_queue_family).protected_index);
                    } else {
                        it->second.protected_index = requested_queue_family;
                    }
                } else {
                    if (it->second.unprocted_index != not_used) {
                        skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state.Handle(),
                                         info_loc.dot(Field::queueFamilyIndex),
                                         "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32
                                         "].",
                                         requested_queue_family, queue_family_map.at(requested_queue_family).unprocted_index);
                    } else {
                        it->second.unprocted_index = requested_queue_family;
                    }
                }
            }
        }

        VkQueueGlobalPriority global_priority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM;  // Implicit default value
        const auto *global_priority_ci = vku::FindStructInPNextChain<VkDeviceQueueGlobalPriorityCreateInfo>(infos[i].pNext);
        if (global_priority_ci) {
            global_priority = global_priority_ci->globalPriority;
        }
        const auto prev_global_priority = global_priorities.find(infos[i].queueFamilyIndex);
        if (prev_global_priority != global_priorities.end()) {
            if (prev_global_priority->second != global_priority) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-06654", pd_state.Handle(), info_loc,
                                 "Multiple queues are created with queueFamilyIndex %" PRIu32
                                 ", but one has global priority %s and another %s.",
                                 infos[i].queueFamilyIndex, string_VkQueueGlobalPriority(prev_global_priority->second),
                                 string_VkQueueGlobalPriority(global_priority));
            }
        } else {
            global_priorities.insert({infos[i].queueFamilyIndex, global_priority});
        }

        const VkQueueFamilyProperties requested_queue_family_props = pd_state.queue_family_properties[requested_queue_family];

        // if using protected flag, make sure queue supports it
        if (protected_create_bit && ((requested_queue_family_props.queueFlags & VK_QUEUE_PROTECTED_BIT) == 0)) {
            skip |= LogError("VUID-VkDeviceQueueCreateInfo-flags-06449", pd_state.Handle(), info_loc.dot(Field::queueFamilyIndex),
                             "(%" PRIu32 ") does not have VK_QUEUE_PROTECTED_BIT supported, but pQueueCreateInfos[%" PRIu32
                             "].flags has VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT.",
                             requested_queue_family, i);
        }

        // Verify that requested queue count of queue family is known to be valid at this point in time
        if (requested_queue_family < pd_state.queue_family_known_count) {
            const auto requested_queue_count = infos[i].queueCount;
            const bool queue_family_has_props = requested_queue_family < pd_state.queue_family_properties.size();
            // spec guarantees at least one queue for each queue family
            const uint32_t available_queue_count = queue_family_has_props ? requested_queue_family_props.queueCount : 1;

            if (requested_queue_count > available_queue_count) {
                const char *conditional_ext_cmd =
                    extensions.vk_khr_get_physical_device_properties2 ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]" : "";
                const std::string count_note =
                    queue_family_has_props
                        ? "i.e. is not less than or equal to " + std::to_string(requested_queue_family_props.queueCount)
                        : "the pQueueFamilyProperties[" + std::to_string(requested_queue_family) + "] was never obtained";

                skip |= LogError(
                    "VUID-VkDeviceQueueCreateInfo-queueCount-00382", pd_state.Handle(), info_loc.dot(Field::queueCount),
                    "(%" PRIu32
                    ") is not less than or equal to available queue count for this pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "].queueFamilyIndex} (%" PRIu32 ") obtained previously from vkGetPhysicalDeviceQueueFamilyProperties%s (%s).",
                    requested_queue_count, i, requested_queue_family, conditional_ext_cmd, count_note.c_str());
            } else {
                if (requested_queue_family >= queue_counts.size()) {
                    queue_counts.resize(requested_queue_family + 1);
                }
                queue_counts[requested_queue_family] += infos[i].queueCount;
            }
        }
    }
    for (uint32_t i = 0; i < static_cast<uint32_t>(queue_counts.size()); ++i) {
        if (queue_counts[i] > pd_state.queue_family_properties[i].queueCount) {
            skip |= LogError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-06755", pd_state.Handle(), loc,
                             "Total queue count requested from queue family index %" PRIu32 " is %" PRIu32
                             ", which is greater than queue count available in the queue family (%" PRIu32 ").",
                             i, queue_counts[i], pd_state.queue_family_properties[i].queueCount);
        }
    }

    return skip;
}

bool core::Instance::PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    // TODO: object_tracker should perhaps do this instead
    //       and it does not seem to currently work anyway -- the loader just crashes before this point
    auto pd_state = Get<vvl::PhysicalDevice>(gpu);
    if (!pd_state) {
        skip |= LogError("VUID-vkCreateDevice-physicalDevice-parameter", instance, error_obj.location,
                         "Have not called vkEnumeratePhysicalDevices() yet.");
        return skip;
    }

    skip |= ValidateDeviceQueueCreateInfos(*pd_state, pCreateInfo->queueCreateInfoCount, pCreateInfo->pQueueCreateInfos,
                                           error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

void CoreChecks::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    // The state tracker sets up the device state (also if extension and/or features are enabled)
    BaseClass::PostCreateDevice(pCreateInfo, loc);

    AdjustValidatorOptions(extensions, enabled_features, spirv_val_options, &spirv_val_option_hash);

    // Allocate shader validation cache
    if (!disabled[shader_validation_caching] && !disabled[shader_validation] && !core_validation_cache) {
        auto tmp_path = GetTempFilePath();
        validation_cache_path = tmp_path + "/shader_validation_cache";
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
        validation_cache_path += "-" + std::to_string(getuid());
#endif
        validation_cache_path += ".bin";

        std::vector<char> validation_cache_data;
        std::ifstream read_file(validation_cache_path.c_str(), std::ios::in | std::ios::binary);

        if (read_file) {
            std::copy(std::istreambuf_iterator<char>(read_file), {}, std::back_inserter(validation_cache_data));
            read_file.close();
        } else {
            LogInfo("WARNING-cache-file-error", device, loc,
                    "Cannot open shader validation cache at %s for reading (it may not exist yet)", validation_cache_path.c_str());
        }

        VkValidationCacheCreateInfoEXT cacheCreateInfo = vku::InitStructHelper();
        cacheCreateInfo.initialDataSize = validation_cache_data.size();
        cacheCreateInfo.pInitialData = validation_cache_data.data();
        cacheCreateInfo.flags = 0;
        CoreLayerCreateValidationCacheEXT(device, &cacheCreateInfo, nullptr, &core_validation_cache);
    }
}

void CoreChecks::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator,
                                            const RecordObject &record_obj) {
    if (!device) return;

    BaseClass::PreCallRecordDestroyDevice(device, pAllocator, record_obj);

    if (core_validation_cache) {
        Location loc(Func::vkDestroyDevice);
        size_t validation_cache_size = 0;
        void *validation_cache_data = nullptr;

        CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, nullptr);

        validation_cache_data = (char *)malloc(sizeof(char) * validation_cache_size);
        if (!validation_cache_data) {
            LogInfo("WARNING-cache-memory-error", device, loc, "Validation Cache Memory Error");
            return;
        }

        VkResult result =
            CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, validation_cache_data);

        if (result != VK_SUCCESS) {
            LogInfo("WARNING-cache-retrieval-error", device, loc, "Validation Cache Retrieval Error");
            free(validation_cache_data);
            return;
        }

        if (validation_cache_path.size() > 0) {
            std::ofstream write_file(validation_cache_path.c_str(), std::ios::out | std::ios::binary);
            if (write_file) {
                write_file.write(static_cast<char *>(validation_cache_data), validation_cache_size);
                write_file.close();
            } else {
                LogInfo("WARNING-cache-write-error", device, loc, "Cannot open shader validation cache at %s for writing",
                        validation_cache_path.c_str());
            }
        }
        free(validation_cache_data);
        CoreLayerDestroyValidationCacheEXT(device, core_validation_cache, NULL);
    }
}

bool CoreChecks::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue,
                                               const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDeviceQueueFamily(queueFamilyIndex, error_obj.location.dot(Field::queueFamilyIndex),
                                      "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");

    for (size_t i = 0; i < device_queue_info_list.size(); i++) {
        const auto device_queue_info = device_queue_info_list.at(i);
        if (device_queue_info.queue_family_index != queueFamilyIndex) {
            continue;
        }

        // flag must be zero
        if (device_queue_info.flags != 0) {
            skip |= LogError(
                "VUID-vkGetDeviceQueue-flags-01841", device, error_obj.location.dot(Field::queueFamilyIndex),
                "(%" PRIu32
                ") was created with a non-zero VkDeviceQueueCreateFlags in vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                "]. Need to use vkGetDeviceQueue2 instead.",
                queueIndex, device_queue_info.index);
        }

        if (device_queue_info.queue_count <= queueIndex) {
            skip |= LogError("VUID-vkGetDeviceQueue-queueIndex-00385", device, error_obj.location.dot(Field::queueIndex),
                             "(%" PRIu32 ") is not less than the number of queues requested from queueFamilyIndex (%" PRIu32
                             ") when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                             "] (i.e. is not less than %" PRIu32 ").",
                             queueIndex, queueFamilyIndex, device_queue_info.index, device_queue_info.queue_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue,
                                                const ErrorObject &error_obj) const {
    bool skip = false;

    if (pQueueInfo) {
        const Location queue_info_loc = error_obj.location.dot(Field::pQueueInfo);
        const uint32_t queueFamilyIndex = pQueueInfo->queueFamilyIndex;
        const uint32_t queueIndex = pQueueInfo->queueIndex;
        const VkDeviceQueueCreateFlags flags = pQueueInfo->flags;

        skip |= ValidateDeviceQueueFamily(queueFamilyIndex, queue_info_loc.dot(Field::queueFamilyIndex),
                                          "VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");

        // ValidateDeviceQueueFamily() already checks if queueFamilyIndex but need to make sure flags match with it
        bool valid_flags = false;

        for (size_t i = 0; i < device_queue_info_list.size(); i++) {
            const auto device_queue_info = device_queue_info_list.at(i);
            // vkGetDeviceQueue2 only checks if both family index AND flags are same as device creation
            // this handle case where the same queueFamilyIndex is used with/without the protected flag
            if ((device_queue_info.queue_family_index != queueFamilyIndex) || (device_queue_info.flags != flags)) {
                continue;
            }
            valid_flags = true;

            if (device_queue_info.queue_count <= queueIndex) {
                skip |= LogError(
                    "VUID-VkDeviceQueueInfo2-queueIndex-01843", device, error_obj.location.dot(Field::queueFamilyIndex),
                    "(%" PRIu32 ") is not less than the number of queues requested from [queueFamilyIndex (%" PRIu32
                    "), flags (%s)] combination when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "] (requested %" PRIu32 " queues).",
                    queueIndex, queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str(), device_queue_info.index,
                    device_queue_info.queue_count);
            }
        }

        // Don't double error message if already skipping from ValidateDeviceQueueFamily
        if (!valid_flags && !skip) {
            skip |= LogError("VUID-VkDeviceQueueInfo2-flags-06225", device, error_obj.location,
                             "The combination of queueFamilyIndex (%" PRIu32
                             ") and flags (%s) were never both set together in any element of "
                             "vkCreateDevice::pCreateInfo->pQueueCreateInfos at device creation time.",
                             queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str());
        }
    }
    return skip;
}

bool core::Instance::ValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice gpu,
                                                                     const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                     VkImageFormatProperties2 *pImageFormatProperties,
                                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *copy_perf_query = vku::FindStructInPNextChain<VkHostImageCopyDevicePerformanceQuery>(pImageFormatProperties->pNext);
    if (copy_perf_query) {
        if ((pImageFormatInfo->usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
            skip |= LogError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-09004", gpu, error_obj.location,
                             "pImageFormatProperties includes a chained "
                             "VkHostImageCopyDevicePerformanceQuery struct, but pImageFormatInfo->usage (%s) does not contain "
                             "VK_IMAGE_USAGE_HOST_TRANSFER_BIT",
                             string_VkBufferUsageFlags(pImageFormatInfo->usage).c_str());
        }
    }
    return skip;
}

bool core::Instance::PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties, const ErrorObject &error_obj) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = false;
    skip |=
        ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(physicalDevice, pImageFormatInfo, pImageFormatProperties, error_obj);
    skip |= ValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties, error_obj);
    return skip;
}

bool core::Instance::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties, const ErrorObject &error_obj) const {
    return PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties,
                                                                  error_obj);
}

// Access helper functions for external modules
VkFormatProperties3KHR CoreChecks::GetPDFormatProperties(const VkFormat format) const {
    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

    if (has_format_feature2) {
        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);
        fmt_props_3.linearTilingFeatures |= fmt_props_2.formatProperties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures |= fmt_props_2.formatProperties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures |= fmt_props_2.formatProperties.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        fmt_props_3.linearTilingFeatures = format_properties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures = format_properties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures = format_properties.bufferFeatures;
    }
    return fmt_props_3;
}

VkResult CoreChecks::CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       VkValidationCacheEXT *pValidationCache) {
    *pValidationCache = ValidationCache::Create(pCreateInfo, spirv_val_option_hash);
    return *pValidationCache ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

void CoreChecks::CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks *pAllocator) {
    delete CastFromHandle<ValidationCache *>(validationCache);
}

VkResult CoreChecks::CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize,
                                                        void *pData) {
    size_t in_size = *pDataSize;
    CastFromHandle<ValidationCache *>(validationCache)->Write(pDataSize, pData);
    return (pData && *pDataSize != in_size) ? VK_INCOMPLETE : VK_SUCCESS;
}

VkResult CoreChecks::CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                       const VkValidationCacheEXT *pSrcCaches) {
    bool skip = false;
    auto dst = CastFromHandle<ValidationCache *>(dstCache);
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < srcCacheCount; i++) {
        auto src = CastFromHandle<const ValidationCache *>(pSrcCaches[i]);
        if (src == dst) {
            const Location loc(Func::vkMergePipelineCaches, Field::dstCache);
            skip |= LogError("VUID-vkMergeValidationCachesEXT-dstCache-01536", device, loc,
                             "(0x%" PRIx64 ") must not appear in pSrcCaches array.", HandleToUint64(dstCache));
            result = VK_ERROR_VALIDATION_FAILED_EXT;
        }
        if (!skip) {
            dst->Merge(src);
        }
    }

    return result;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    const LogObjectList objlist(commandBuffer);
    skip |= ValidateCmd(cb_state, error_obj.location);
    const Location loc = error_obj.location.dot(Field::deviceMask);
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    skip |= ValidateDeviceMaskToZero(deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    skip |= ValidateDeviceMaskToCommandBuffer(cb_state, deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    skip |= ValidateDeviceMaskToRenderPass(cb_state, deviceMask, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask, error_obj);
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkPrivateDataSlotEXT *pPrivateDataSlot,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, error_obj);
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    if (!enabled_features.privateData) {
        skip |= LogError("VUID-vkCreatePrivateDataSlot-privateData-04564", device, error_obj.location,
                         "The privateData feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateDeviceQueueFamily(pCreateInfo->queueFamilyIndex, create_info_loc.dot(Field::queueFamilyIndex),
                                      "VUID-vkCreateCommandPool-queueFamilyIndex-01937");
    if ((enabled_features.protectedMemory == VK_FALSE) && ((pCreateInfo->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) != 0)) {
        skip |= LogError("VUID-VkCommandPoolCreateInfo-flags-02860", device, create_info_loc.dot(Field::flags),
                         "includes VK_COMMAND_POOL_CREATE_PROTECTED_BIT, but the protectedMemory feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    bool skip = false;
    // In case of DEVICE_LOST, all execution is considered over
    if (is_device_lost) return skip;
    auto cp_state = Get<vvl::CommandPool>(commandPool);
    if (!cp_state) return skip;
    // Verify that command buffers in pool are complete (not in-flight)
    for (auto &entry : cp_state->commandBuffers) {
        auto cb_state = entry.second;
        if (cb_state->InUse()) {
            const LogObjectList objlist(cb_state->Handle(), commandPool);
            skip |= LogError("VUID-vkDestroyCommandPool-commandPool-00041", objlist, error_obj.location, "(%s) is in use.",
                             FormatHandle(cb_state->Handle()).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cp_state = Get<vvl::CommandPool>(commandPool);
    ASSERT_AND_RETURN_SKIP(cp_state);
    // Verify that command buffers in pool are complete (not in-flight)
    for (auto &entry : cp_state->commandBuffers) {
        auto cb_state = entry.second;
        if (cb_state->InUse()) {
            const LogObjectList objlist(cb_state->Handle(), commandPool);
            skip |= LogError("VUID-vkResetCommandPool-commandPool-00040", objlist, error_obj.location, "(%s) is in use.",
                             FormatHandle(cb_state->Handle()).c_str());
        }
    }
    return skip;
}

// For given obj node, if it is use, flag a validation error and return callback result, else return false
bool CoreChecks::ValidateObjectNotInUse(const vvl::StateObject *obj_node, const Location &loc, const char *error_code) const {
    if (disabled[object_in_use]) return false;
    // In case of DEVICE_LOST, all execution is considered over
    if (is_device_lost) return false;
    auto obj_struct = obj_node->Handle();
    bool skip = false;

    const auto *used_handle = obj_node->InUse();
    if (used_handle) {
        skip |= LogError(error_code, device, loc, "can't be called on %s that is currently in use by %s.",
                         FormatHandle(obj_struct).c_str(), FormatHandle(*used_handle).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoEXT *pTimestampInfos,
                                                           uint64_t *pTimestamps, uint64_t *pMaxDeviation,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateGetCalibratedTimestampsKHR(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation,
                                                     error_obj);
}

bool CoreChecks::PreCallValidateGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoKHR *pTimestampInfos,
                                                           uint64_t *pTimestamps, uint64_t *pMaxDeviation,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;

    auto query_function = (error_obj.location.function == Func::vkGetCalibratedTimestampsKHR)
                              ? DispatchGetPhysicalDeviceCalibrateableTimeDomainsKHR
                              : DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT;
    uint32_t count = 0;
    query_function(physical_device, &count, nullptr);
    std::vector<VkTimeDomainKHR> valid_time_domains(count);
    query_function(physical_device, &count, valid_time_domains.data());

    vvl::unordered_map<VkTimeDomainKHR, uint32_t> time_domain_map;
    for (uint32_t i = 0; i < timestampCount; i++) {
        const VkTimeDomainKHR time_domain = pTimestampInfos[i].timeDomain;
        auto it = time_domain_map.find(time_domain);
        if (it != time_domain_map.end()) {
            skip |= LogError("VUID-vkGetCalibratedTimestampsKHR-timeDomain-09246", device,
                             error_obj.location.dot(Field::pTimestampInfos, i).dot(Field::timeDomain),
                             "and pTimestampInfos[%" PRIu32 "].timeDomain are both %s.", it->second,
                             string_VkTimeDomainKHR(time_domain));
            break;  // no reason to check after finding 1 duplicate
        } else if (!IsValueIn(time_domain, valid_time_domains)) {
            skip |= LogError("VUID-VkCalibratedTimestampInfoKHR-timeDomain-02354", device,
                             error_obj.location.dot(Field::pTimestampInfos, i).dot(Field::timeDomain), "is %s.",
                             string_VkTimeDomainKHR(time_domain));
        }
        time_domain_map[time_domain] = i;
    }
    return skip;
}

// These were all added from https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6672
bool CoreChecks::ValidateDeviceQueueSupport(const Location &loc) const {
    bool skip = false;
    const char *vuid = kVUIDUndefined;
    VkQueueFlags flags = 0;

    switch (loc.function) {
        case Func::vkCreateRayTracingPipelinesKHR:
            vuid = "VUID-vkCreateRayTracingPipelinesKHR-device-09677";
            flags = VK_QUEUE_COMPUTE_BIT;
            break;
        case Func::vkCreateRayTracingPipelinesNV:
            vuid = "VUID-vkCreateRayTracingPipelinesNV-device-09677";
            flags = VK_QUEUE_COMPUTE_BIT;
            break;
        case Func::vkCreateComputePipelines:
            vuid = "VUID-vkCreateComputePipelines-device-09661";
            flags = VK_QUEUE_COMPUTE_BIT;
            break;
        case Func::vkCreateGraphicsPipelines:
            vuid = "VUID-vkCreateGraphicsPipelines-device-09662";
            flags = VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateQueryPool:
            vuid = "VUID-vkCreateQueryPool-device-09663";
            flags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateBuffer:
            vuid = "VUID-vkCreateBuffer-device-09664";
            flags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_SPARSE_BINDING_BIT |
                    VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateBufferView:
            vuid = "VUID-vkCreateBufferView-device-09665";
            flags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateImage:
            vuid = "VUID-vkCreateImage-device-09666";
            flags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_OPTICAL_FLOW_BIT_NV |
                    VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateImageView:
            vuid = "VUID-vkCreateImageView-device-09667";
            flags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateSampler:
            vuid = "VUID-vkCreateSampler-device-09668";
            flags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateEvent:
            vuid = "VUID-vkCreateEvent-device-09672";
            flags = VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateRenderPass:
            vuid = "VUID-vkCreateRenderPass-device-10000";
            flags = VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateRenderPass2:
        case Func::vkCreateRenderPass2KHR:
            vuid = "VUID-vkCreateRenderPass2-device-10001";
            flags = VK_QUEUE_GRAPHICS_BIT;
            break;
        case Func::vkCreateFramebuffer:
            vuid = "VUID-vkCreateFramebuffer-device-10002";
            flags = VK_QUEUE_GRAPHICS_BIT;
            break;
        default:
            assert(false);  // missing case
            return skip;
    }

    if ((physical_device_state->supported_queues & flags) == 0) {
        skip |= LogError(vuid, device, loc, "device only supports (%s) but require one of (%s).",
                         string_VkQueueFlags(physical_device_state->supported_queues).c_str(), string_VkQueueFlags(flags).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT *pFaultCounts,
                                                      VkDeviceFaultInfoEXT *pFaultInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    if (!is_device_lost) {
        skip |= LogError("VUID-vkGetDeviceFaultInfoEXT-device-07336", device, error_obj.location,
                         "device has not been found to be in a lost state.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkPipelineBinaryHandlesInfoKHR *pBinaries,
                                                          const ErrorObject &error_obj) const {

    bool skip = false;

    uint32_t pointerCount = 0;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    if (pCreateInfo->pipeline != VK_NULL_HANDLE) {
        auto pipeline = pCreateInfo->pipeline;
        auto pipeline_state = Get<vvl::Pipeline>(pipeline);

        ASSERT_AND_RETURN_SKIP(pipeline_state);

        if (!(pipeline_state->create_flags & VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR)) {
            skip |= LogError("VUID-VkPipelineBinaryCreateInfoKHR-pipeline-09607", pipeline, create_info_loc.dot(Field::pipeline),
                             "called on a pipeline created without the "
                             "VK_PIPELINE_CREATE_2_CAPTURE_DATA_BIT_KHR flag set. (Make sure you set it with "
                             "VkPipelineCreateFlags2CreateInfo)");
        }

        if (pipeline_state->binary_data_released) {
            skip |= LogError("VUID-VkPipelineBinaryCreateInfoKHR-pipeline-09608", pipeline, create_info_loc.dot(Field::pipeline),
                             "called on a pipeline after vkReleaseCapturedPipelineDataKHR was called on it.");
        }

        pointerCount++;
    }

    if (pCreateInfo->pPipelineCreateInfo != nullptr) {
        auto *props = &phys_dev_ext_props.pipeline_binary_props;

        if (!props->pipelineBinaryInternalCache) {
            skip |=
                LogError("VUID-VkPipelineBinaryCreateInfoKHR-pipelineBinaryInternalCache-09609", device,
                         create_info_loc.dot(Field::pPipelineCreateInfo), "is not NULL, but pipelineBinaryInternalCache is false.");
        }

        if (props->pipelineBinaryInternalCacheControl && disable_internal_pipeline_cache) {
            skip |= LogError("VUID-VkPipelineBinaryCreateInfoKHR-device-09610", device,
                             create_info_loc.dot(Field::pPipelineCreateInfo), "is not NULL, but disableInternalCache is true.");
        }

        const auto *binary_info = vku::FindStructInPNextChain<VkPipelineBinaryInfoKHR>(pCreateInfo->pPipelineCreateInfo);
        if (binary_info && (binary_info->binaryCount > 0)) {
            skip |= LogError("VUID-VkPipelineBinaryCreateInfoKHR-pPipelineCreateInfo-09606", device,
                             create_info_loc.dot(Field::pPipelineCreateInfo).dot(Field::binaryCount), "(%" PRIu32 ") is not zero",
                             binary_info->binaryCount);
        }

        pointerCount++;
    }

    if (pCreateInfo->pKeysAndDataInfo != nullptr) {
        pointerCount++;
    }

    if (pointerCount != 1) {
        skip |= LogError("VUID-VkPipelineBinaryCreateInfoKHR-pKeysAndDataInfo-09619", device, create_info_loc,
                         "One and only one of pKeysAndDataInfo, pipeline, or pPipelineCreateInfo must be non_NULL.");
    }

    return skip;
}
