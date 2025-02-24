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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "generated/dispatch_functions.h"

namespace stateless {
// Traits objects to allow string_join to operate on collections of const char *
template <typename String>
struct StringJoinSizeTrait {
    static size_t size(const String &str) { return str.size(); }
};

template <>
struct StringJoinSizeTrait<const char *> {
    static size_t size(const char *str) {
        if (!str) return 0;
        return strlen(str);
    }
};
// Similar to perl/python join
//    * String must support size, reserve, append, and be default constructable
//    * StringCollection must support size, const forward iteration, and store
//      strings compatible with String::append
//    * Accessor trait can be set if default accessors (compatible with string
//      and const char *) don't support size(StringCollection::value_type &)
//
// Return type based on sep type
template <typename String = std::string, typename StringCollection = std::vector<String>,
          typename Accessor = StringJoinSizeTrait<typename StringCollection::value_type>>
static inline String string_join(const String &sep, const StringCollection &strings) {
    String joined;
    const size_t count = strings.size();
    if (!count) return joined;

    // Prereserved storage, s.t. we will execute in linear time (avoids reallocation copies)
    size_t reserve = (count - 1) * sep.size();
    for (const auto &str : strings) {
        reserve += Accessor::size(str);  // abstracted to allow const char * type in StringCollection
    }
    joined.reserve(reserve + 1);

    // Seps only occur *between* strings entries, so first is special
    auto current = strings.cbegin();
    joined.append(*current);
    ++current;
    for (; current != strings.cend(); ++current) {
        joined.append(sep);
        joined.append(*current);
    }
    return joined;
}

// Requires StringCollection::value_type has a const char * constructor and is compatible the string_join::String above
template <typename StringCollection = std::vector<std::string>, typename SepString = std::string>
static inline SepString string_join(const char *sep, const StringCollection &strings) {
    return string_join<SepString, StringCollection>(SepString(sep), strings);
}

template <typename ExtensionState>
bool Instance::ValidateExtensionReqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                                     vvl::Extension extension, const Location &extension_loc) const {
    bool skip = false;
    if (extension == vvl::Extension::Empty) {
        return skip;
    }
    auto info = ExtensionState::GetInfo(extension);

    if (!info.state) {
        return skip;  // Unknown extensions cannot be checked so report OK
    }

    // Check against the required list in the info
    std::vector<const char *> missing;
    for (const auto &req : info.requirements) {
        if (!(extensions.*(req.enabled))) {
            missing.push_back(req.name);
        }
    }

    // Report any missing requirements
    if (missing.size()) {
        std::string missing_joined_list = string_join(", ", missing);
        skip |= LogError(vuid, instance, extension_loc, "Missing extension%s required by the %s extension %s: %s.",
                         ((missing.size() > 1) ? "s" : ""), extension_type, String(extension), missing_joined_list.c_str());
    }
    return skip;
}

template <typename ExtensionState>
ExtEnabled ExtensionStateByName(const ExtensionState &extensions, vvl::Extension extension) {
    auto info = ExtensionState::GetInfo(extension);
    // unknown extensions can't be enabled in extension struct
    ExtEnabled state = info.state ? extensions.*(info.state) : kNotEnabled;
    return state;
}

bool Instance::PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                             VkInstance *pInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    Location loc = error_obj.location;
    // Note: From the spec--
    //  Providing a NULL VkInstanceCreateInfo::pApplicationInfo or providing an apiVersion of 0 is equivalent to providing
    //  an apiVersion of VK_MAKE_VERSION(1, 0, 0).  (a.k.a. VK_API_VERSION_1_0)
    uint32_t local_api_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
    // Create and use a local instance extension object, as an actual instance has not been created yet
    InstanceExtensions instance_extensions(local_api_version, pCreateInfo);
    DeviceExtensions device_extensions(instance_extensions, local_api_version);
    Context context(*this, error_obj, device_extensions);

    skip |= context.ValidateStructType(loc.dot(Field::pCreateInfo), pCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, true,
                                       "VUID-vkCreateInstance-pCreateInfo-parameter", "VUID-VkInstanceCreateInfo-sType-sType");

    if (pAllocator != nullptr) {
        [[maybe_unused]] const Location pAllocator_loc = loc.dot(Field::pAllocator);
        skip |= context.ValidateAllocationCallbacks(*pAllocator, pAllocator_loc);
    }
    skip |= context.ValidateRequiredPointer(loc.dot(Field::pInstance), pInstance, "VUID-vkCreateInstance-pInstance-parameter");

    uint32_t api_version_nopatch = VK_MAKE_VERSION(VK_VERSION_MAJOR(local_api_version), VK_VERSION_MINOR(local_api_version), 0);
    const Location create_info_loc = loc.dot(Field::pCreateInfo);
    if (api_version != api_version_nopatch) {
        if ((api_version_nopatch < VK_API_VERSION_1_0) && (local_api_version != 0)) {
            skip |= LogError("VUID-VkApplicationInfo-apiVersion-04010", instance,
                             create_info_loc.dot(Field::pApplicationInfo).dot(Field::apiVersion),
                             "is (0x%08x). "
                             "Using VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                             local_api_version, api_version.Major(), api_version.Minor());
        } else {
            skip |= LogWarning(kVUIDUndefined, instance, create_info_loc.dot(Field::pApplicationInfo).dot(Field::apiVersion),
                               "is (0x%08x). "
                               "Assuming VK_API_VERSION_%" PRIu32 "_%" PRIu32 ".",
                               local_api_version, api_version.Major(), api_version.Minor());
        }
    }

    if (pCreateInfo != nullptr) {
        skip |= context.ValidateFlags(create_info_loc.dot(Field::flags), vvl::FlagBitmask::VkInstanceCreateFlagBits,
                                      AllVkInstanceCreateFlagBits, pCreateInfo->flags, kOptionalFlags,
                                      "VUID-VkInstanceCreateInfo-flags-parameter");

        skip |= context.ValidateStructType(
            create_info_loc.dot(Field::pApplicationInfo), pCreateInfo->pApplicationInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO, false,
            "VUID-VkInstanceCreateInfo-pApplicationInfo-parameter", "VUID-VkApplicationInfo-sType-sType");

        if (pCreateInfo->pApplicationInfo != nullptr) {
            [[maybe_unused]] const Location pApplicationInfo_loc = create_info_loc.dot(Field::pApplicationInfo);
            skip |= context.ValidateStructPnext(pApplicationInfo_loc, pCreateInfo->pApplicationInfo->pNext, 0, nullptr,
                                                GeneratedVulkanHeaderVersion, "VUID-VkApplicationInfo-pNext-pNext", kVUIDUndefined,
                                                true);
        }

        skip |= context.ValidateStringArray(create_info_loc.dot(Field::enabledLayerCount),
                                            create_info_loc.dot(Field::ppEnabledLayerNames), pCreateInfo->enabledLayerCount,
                                            pCreateInfo->ppEnabledLayerNames, false, true, kVUIDUndefined,
                                            "VUID-VkInstanceCreateInfo-ppEnabledLayerNames-parameter");

        skip |= context.ValidateStringArray(create_info_loc.dot(Field::enabledExtensionCount),
                                            create_info_loc.dot(Field::ppEnabledExtensionNames), pCreateInfo->enabledExtensionCount,
                                            pCreateInfo->ppEnabledExtensionNames, false, true, kVUIDUndefined,
                                            "VUID-VkInstanceCreateInfo-ppEnabledExtensionNames-parameter");
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        skip |= ValidateExtensionReqs(instance_extensions, "VUID-vkCreateInstance-ppEnabledExtensionNames-01388", "instance",
                                      extension, create_info_loc.dot(Field::ppEnabledExtensionNames, i));
    }
    if (pCreateInfo->flags & VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR &&
        !instance_extensions.vk_khr_portability_enumeration) {
        skip |= LogError("VUID-VkInstanceCreateInfo-flags-06559", instance, create_info_loc.dot(Field::flags),
                         "has VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR set, but "
                         "ppEnabledExtensionNames does not include VK_KHR_portability_enumeration");
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT) &&
            (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT)) {
            skip |= LogError("VUID-VkInstanceCreateInfo-pNext-06779", instance, error_obj.location,
                             "The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose "
                             "exportObjectType = %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT

    // avoid redundant pNext-pNext errors from the cases where we have specific VUs by returning early
    const auto *debug_report_callback = vku::FindStructInPNextChain<VkDebugReportCallbackCreateInfoEXT>(pCreateInfo->pNext);
    if (debug_report_callback && !instance_extensions.vk_ext_debug_report) {
        skip |= LogError("VUID-VkInstanceCreateInfo-pNext-04925", instance, create_info_loc.dot(Field::ppEnabledExtensionNames),
                         "does not include VK_EXT_debug_report, but the pNext chain includes VkDebugReportCallbackCreateInfoEXT.");
        return skip;
    }
    const auto *debug_utils_messenger = vku::FindStructInPNextChain<VkDebugUtilsMessengerCreateInfoEXT>(pCreateInfo->pNext);
    if (debug_utils_messenger && !instance_extensions.vk_ext_debug_utils) {
        skip |= LogError("VUID-VkInstanceCreateInfo-pNext-04926", instance, create_info_loc.dot(Field::ppEnabledExtensionNames),
                         "does not include VK_EXT_debug_utils, but the pNext chain includes VkDebugUtilsMessengerCreateInfoEXT.");
        return skip;
    }
    const auto *direct_driver_loading_list = vku::FindStructInPNextChain<VkDirectDriverLoadingListLUNARG>(pCreateInfo->pNext);
    if (direct_driver_loading_list && !instance_extensions.vk_lunarg_direct_driver_loading) {
        skip |= LogError(
            "VUID-VkInstanceCreateInfo-pNext-09400", instance, create_info_loc.dot(Field::ppEnabledExtensionNames),
            "does not include VK_LUNARG_direct_driver_loading, but the pNext chain includes VkDirectDriverLoadingListLUNARG.");
        return skip;
    }

    const auto *validation_features = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(pCreateInfo->pNext);
    if (validation_features && !instance_extensions.vk_ext_validation_features) {
        skip |= LogError("VUID-VkInstanceCreateInfo-pNext-10243", instance, create_info_loc.dot(Field::ppEnabledExtensionNames),
                         "does not include VK_EXT_validation_features, but the pNext chain includes VkValidationFeaturesEXT");
        return skip;
    }
    if (validation_features) {
        bool debug_printf = false;
        bool gpu_assisted = false;
        bool reserve_slot = false;
        for (uint32_t i = 0; i < validation_features->enabledValidationFeatureCount; i++) {
            switch (validation_features->pEnabledValidationFeatures[i]) {
                case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
                    gpu_assisted = true;
                    break;

                case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
                    debug_printf = true;
                    break;

                case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
                    reserve_slot = true;
                    break;

                default:
                    break;
            }
        }
        if (reserve_slot && !gpu_assisted && !debug_printf) {
            skip |= LogError("VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02967", instance,
                             create_info_loc.pNext(Struct::VkValidationFeaturesEXT, Field::pEnabledValidationFeatures),
                             "includes VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT but no "
                             "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT or VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT.");
        }
    }

    constexpr std::array allowed_structs_VkInstanceCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                                                                 VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                                                                 VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG,
                                                                 VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT,
                                                                 VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                                                                 VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
                                                                 VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT};

    skip |= context.ValidateStructPnext(create_info_loc, pCreateInfo->pNext, allowed_structs_VkInstanceCreateInfo.size(),
                                        allowed_structs_VkInstanceCreateInfo.data(), GeneratedVulkanHeaderVersion,
                                        "VUID-VkInstanceCreateInfo-pNext-pNext", "VUID-VkInstanceCreateInfo-sType-unique", true);

    return skip;
}

void Instance::CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count) {
    // Assume phys_devices is valid
    assert(phys_devices);
    for (int i = 0; i < count; ++i) {
        const auto &phys_device = phys_devices[i];
        if (0 == physical_device_properties_map.count(phys_device)) {
            auto phys_dev_props = new VkPhysicalDeviceProperties;
            DispatchGetPhysicalDeviceProperties(phys_device, phys_dev_props);
            physical_device_properties_map[phys_device] = phys_dev_props;

            // Enumerate the Device Ext Properties to save the PhysicalDevice supported extension state
            uint32_t ext_count = 0;

            std::vector<VkExtensionProperties> ext_props{};
            DispatchEnumerateDeviceExtensionProperties(phys_device, nullptr, &ext_count, nullptr);
            ext_props.resize(ext_count);
            DispatchEnumerateDeviceExtensionProperties(phys_device, nullptr, &ext_count, ext_props.data());

            DeviceExtensions phys_dev_exts(extensions, phys_dev_props->apiVersion, ext_props);
            physical_device_extensions[phys_device] = std::move(phys_dev_exts);
        }
    }
}

void Instance::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                      VkPhysicalDevice *pPhysicalDevices, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) {
        return;
    }

    if (pPhysicalDeviceCount && pPhysicalDevices) {
        CommonPostCallRecordEnumeratePhysicalDevice(pPhysicalDevices, *pPhysicalDeviceCount);
    }
}

void Instance::PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
                                                           VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
                                                           const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) {
        return;
    }

    if (pPhysicalDeviceGroupCount && pPhysicalDeviceGroupProperties) {
        for (uint32_t i = 0; i < *pPhysicalDeviceGroupCount; i++) {
            const auto &group = pPhysicalDeviceGroupProperties[i];
            CommonPostCallRecordEnumeratePhysicalDevice(group.physicalDevices, group.physicalDeviceCount);
        }
    }
}

void Instance::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
                                            const RecordObject &record_obj) {
    for (auto it = physical_device_properties_map.begin(); it != physical_device_properties_map.end();) {
        delete (it->second);
        it = physical_device_properties_map.erase(it);
    }
}

void Instance::PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj) {
    auto device_data = vvl::dispatch::GetData(*pDevice);
    if (record_obj.result != VK_SUCCESS) return;
    auto stateless_device = static_cast<Device *>(device_data->GetValidationObject(container_type));

    memcpy(&stateless_device->device_limits, &stateless_device->phys_dev_props.limits, sizeof(VkPhysicalDeviceLimits));

    std::vector<VkExtensionProperties> ext_props{};
    uint32_t ext_count = 0;
    DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_count, nullptr);
    ext_props.resize(ext_count);
    DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_count, ext_props.data());
    for (const auto &prop : ext_props) {
        vvl::Extension extension = GetExtension(prop.extensionName);
        if (extension == vvl::Extension::_VK_EXT_discard_rectangles) {
            stateless_device->discard_rectangles_extension_version = prop.specVersion;
        } else if (extension == vvl::Extension::_VK_NV_scissor_exclusive) {
            stateless_device->scissor_exclusive_extension_version = prop.specVersion;
        }
    }
}

bool Instance::manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    for (size_t i = 0; i < pCreateInfo->enabledLayerCount; i++) {
        skip |=
            context.ValidateString(create_info_loc.dot(Field::ppEnabledLayerNames),
                                   "VUID-VkDeviceCreateInfo-ppEnabledLayerNames-parameter", pCreateInfo->ppEnabledLayerNames[i]);
    }

    // If this device supports VK_KHR_portability_subset, it must be enabled
    const auto &exposed_extensions = physical_device_extensions.at(physicalDevice);
    const bool portability_supported = exposed_extensions.vk_khr_portability_subset;
    bool portability_requested = false;
    bool fragmentmask_requested = false;

    vvl::unordered_set<vvl::Extension> enabled_extensions{};
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        vvl::Extension extension = GetExtension(pCreateInfo->ppEnabledExtensionNames[i]);
        enabled_extensions.insert(extension);
        skip |= context.ValidateString(create_info_loc.dot(Field::ppEnabledExtensionNames),
                                       "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-parameter",
                                       pCreateInfo->ppEnabledExtensionNames[i]);
        skip |= ValidateExtensionReqs(extensions, "VUID-vkCreateDevice-ppEnabledExtensionNames-01387", "device", extension,
                                      create_info_loc.dot(Field::ppEnabledExtensionNames, i));
        if (extension == vvl::Extension::_VK_KHR_portability_subset) {
            portability_requested = true;
        }
        if (extension == vvl::Extension::_VK_AMD_shader_fragment_mask) {
            fragmentmask_requested = true;
        }
    }

    if (portability_supported && !portability_requested) {
        skip |= LogError("VUID-VkDeviceCreateInfo-pProperties-04451", physicalDevice, error_obj.location,
                         "VK_KHR_portability_subset must be enabled because physical device %s supports it",
                         FormatHandle(physicalDevice).c_str());
    }

    if (IsExtEnabledByCreateinfo(ExtensionStateByName(extensions, vvl::Extension::_VK_AMD_negative_viewport_height))) {
        const bool maint1 = IsExtEnabledByCreateinfo(ExtensionStateByName(extensions, vvl::Extension::_VK_KHR_maintenance1));
        // Only need to check for VK_KHR_MAINTENANCE_1_EXTENSION_NAME if api version is 1.0, otherwise it's deprecated due to
        // integration into api version 1.1
        if (api_version >= VK_API_VERSION_1_1) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-01840", physicalDevice,
                             create_info_loc.dot(Field::ppEnabledExtensionNames),
                             "must not include "
                             "VK_AMD_negative_viewport_height if api version is greater than or equal to 1.1.");
        } else if (maint1) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374", physicalDevice,
                             create_info_loc.dot(Field::ppEnabledExtensionNames),
                             "must not simultaneously include "
                             "VK_KHR_maintenance1 and VK_AMD_negative_viewport_height.");
        }
    }

    if (fragmentmask_requested) {
        const auto *descriptor_buffer_features = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(pCreateInfo->pNext);
        if (descriptor_buffer_features && descriptor_buffer_features->descriptorBuffer) {
            skip |=
                LogError("VUID-VkDeviceCreateInfo-None-08095", physicalDevice, create_info_loc.dot(Field::ppEnabledExtensionNames),
                         "must not contain VK_AMD_shader_fragment_mask if the descriptorBuffer feature is enabled.");
        }
    }

    {
        const bool khr_bda =
            IsExtEnabledByCreateinfo(ExtensionStateByName(extensions, vvl::Extension::_VK_KHR_buffer_device_address));
        const bool ext_bda =
            IsExtEnabledByCreateinfo(ExtensionStateByName(extensions, vvl::Extension::_VK_EXT_buffer_device_address));
        if (khr_bda && ext_bda) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-03328", physicalDevice,
                             create_info_loc.dot(Field::ppEnabledExtensionNames),
                             "must not contain both VK_KHR_buffer_device_address and "
                             "VK_EXT_buffer_device_address.");
        }
    }

    const auto features2 = vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
    if (pCreateInfo->pNext != nullptr && pCreateInfo->pEnabledFeatures && features2) {
        // Cannot include VkPhysicalDeviceFeatures2 and have non-null pEnabledFeatures
        skip |= LogError("VUID-VkDeviceCreateInfo-pNext-00373", physicalDevice, create_info_loc.dot(Field::pNext),
                         "includes a VkPhysicalDeviceFeatures2 struct when pCreateInfo->pEnabledFeatures is not NULL.");
    }

    const VkPhysicalDeviceFeatures *features = features2 ? &features2->features : pCreateInfo->pEnabledFeatures;

    if (const auto *robustness2_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext)) {
        if (features && robustness2_features->robustBufferAccess2 && !features->robustBufferAccess) {
            skip |= LogError("VUID-VkPhysicalDeviceRobustness2FeaturesEXT-robustBufferAccess2-04000", physicalDevice,
                             error_obj.location, "If robustBufferAccess2 is enabled then robustBufferAccess must be enabled.");
        }
    }

    if (const auto *raytracing_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(pCreateInfo->pNext)) {
        if (raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed &&
            !raytracing_features->rayTracingPipelineShaderGroupHandleCaptureReplay) {
            skip |= LogError(
                "VUID-VkPhysicalDeviceRayTracingPipelineFeaturesKHR-rayTracingPipelineShaderGroupHandleCaptureReplayMixed-03575",
                physicalDevice, error_obj.location,
                "If rayTracingPipelineShaderGroupHandleCaptureReplayMixed is VK_TRUE, "
                "rayTracingPipelineShaderGroupHandleCaptureReplay "
                "must also be VK_TRUE.");
        }
    }

    // might be set in Feature12 struct
    bool any_update_after_bind_feature = false;
    if (const auto *di_features = vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(pCreateInfo->pNext)) {
        any_update_after_bind_feature = di_features->descriptorBindingUniformBufferUpdateAfterBind ||
                                        di_features->descriptorBindingStorageBufferUpdateAfterBind ||
                                        di_features->descriptorBindingUniformTexelBufferUpdateAfterBind ||
                                        di_features->descriptorBindingStorageTexelBufferUpdateAfterBind;
    }

    const auto *vulkan_11_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(pCreateInfo->pNext);
    if (vulkan_11_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        constexpr std::array illegal_feature_structs_with_11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES,
                                                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
                                                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES,
                                                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES,
                                                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
                                                                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES};
        while (current) {
            if (IsValueIn(current->sType, illegal_feature_structs_with_11)) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pNext-02829", physicalDevice, error_obj.location,
                                 "If the pNext chain includes a VkPhysicalDeviceVulkan11Features structure, then "
                                 "it must not include a %s structure",
                                 string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }

        // Check features are enabled if matching extension is passed in as well
        if (vulkan_11_features->shaderDrawParameters == VK_FALSE &&
            enabled_extensions.find(vvl::Extension::_VK_KHR_shader_draw_parameters) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-04476", physicalDevice, error_obj.location,
                             "%s is enabled but VkPhysicalDeviceVulkan11Features::shaderDrawParameters is not VK_TRUE.",
                             VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        }
    }

    const auto *vulkan_12_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(pCreateInfo->pNext);
    if (vulkan_12_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        constexpr std::array illegal_feature_structs_with_12 = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES};
        while (current) {
            if (IsValueIn(current->sType, illegal_feature_structs_with_12)) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pNext-02830", physicalDevice, create_info_loc.dot(Field::pNext),
                                 "chain includes a VkPhysicalDeviceVulkan12Features structure, then it must not "
                                 "include a %s structure",
                                 string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
        // Check features are enabled if matching extension is passed in as well
        if (vulkan_12_features->drawIndirectCount == VK_FALSE &&
            enabled_extensions.find(vvl::Extension::_VK_KHR_draw_indirect_count) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02831", physicalDevice, error_obj.location,
                             "%s is enabled but VkPhysicalDeviceVulkan12Features::drawIndirectCount is not VK_TRUE.",
                             VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        }
        if (vulkan_12_features->samplerMirrorClampToEdge == VK_FALSE &&
            enabled_extensions.find(vvl::Extension::_VK_KHR_sampler_mirror_clamp_to_edge) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02832", physicalDevice, error_obj.location,
                             " %s is enabled but VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge "
                             "is not VK_TRUE.",
                             VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        }
        if (vulkan_12_features->descriptorIndexing == VK_FALSE &&
            enabled_extensions.find(vvl::Extension::_VK_EXT_descriptor_indexing) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02833", physicalDevice, error_obj.location,
                             "%s is enabled but VkPhysicalDeviceVulkan12Features::descriptorIndexing is not VK_TRUE.",
                             VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        }
        if (vulkan_12_features->samplerFilterMinmax == VK_FALSE &&
            enabled_extensions.find(vvl::Extension::_VK_EXT_sampler_filter_minmax) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02834", physicalDevice, error_obj.location,
                             "%s is enabled but VkPhysicalDeviceVulkan12Features::samplerFilterMinmax is not VK_TRUE.",
                             VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        }

        if ((vulkan_12_features->shaderOutputViewportIndex == VK_FALSE || vulkan_12_features->shaderOutputLayer == VK_FALSE) &&
            enabled_extensions.find(vvl::Extension::_VK_EXT_shader_viewport_index_layer) != enabled_extensions.end()) {
            skip |= LogError("VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-02835", physicalDevice, error_obj.location,
                             "%s is enabled but both VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex "
                             "and VkPhysicalDeviceVulkan12Features::shaderOutputLayer are not VK_TRUE.",
                             VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        }

        if (vulkan_12_features->bufferDeviceAddress == VK_TRUE) {
            if (IsExtEnabledByCreateinfo(ExtensionStateByName(extensions, vvl::Extension::_VK_EXT_buffer_device_address))) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pNext-04748", physicalDevice, create_info_loc.dot(Field::pNext),
                                 "chain includes VkPhysicalDeviceVulkan12Features with bufferDeviceAddress "
                                 "set to VK_TRUE and ppEnabledExtensionNames contains VK_EXT_buffer_device_address");
            }
        }

        any_update_after_bind_feature = vulkan_12_features->descriptorBindingUniformBufferUpdateAfterBind ||
                                        vulkan_12_features->descriptorBindingStorageBufferUpdateAfterBind ||
                                        vulkan_12_features->descriptorBindingUniformTexelBufferUpdateAfterBind ||
                                        vulkan_12_features->descriptorBindingStorageTexelBufferUpdateAfterBind;
    }

    const auto *vulkan_13_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(pCreateInfo->pNext);
    if (vulkan_13_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        constexpr std::array illegal_feature_structs_with_13 = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES};
        while (current) {
            if (IsValueIn(current->sType, illegal_feature_structs_with_13)) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pNext-06532", physicalDevice, create_info_loc.dot(Field::pNext),
                                 "chain includes a VkPhysicalDeviceVulkan13Features structure, then it must not "
                                 "include a %s structure",
                                 string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
    }

    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8969
    const auto *vulkan_14_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(pCreateInfo->pNext);
    if (vulkan_14_features) {
        const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(pCreateInfo->pNext);
        constexpr std::array illegal_feature_structs_with_14 = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES,
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES};
        while (current) {
            if (IsValueIn(current->sType, illegal_feature_structs_with_14)) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pNext-10360", physicalDevice, create_info_loc.dot(Field::pNext),
                                 "chain includes a VkPhysicalDeviceVulkan14Features structure, then it must not "
                                 "include a %s structure",
                                 string_VkStructureType(current->sType));
                break;
            }
            current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
        }
    }

    // Validate pCreateInfo->pQueueCreateInfos
    if (pCreateInfo->pQueueCreateInfos) {
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const VkDeviceQueueCreateInfo &queue_create_info = pCreateInfo->pQueueCreateInfos[i];
            const uint32_t requested_queue_family = queue_create_info.queueFamilyIndex;
            if (requested_queue_family == VK_QUEUE_FAMILY_IGNORED) {
                skip |= LogError("VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381", physicalDevice,
                                 create_info_loc.dot(Field::pQueueCreateInfos, i).dot(Field::queueFamilyIndex),
                                 "is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.");
            }

            if (queue_create_info.pQueuePriorities != nullptr) {
                for (uint32_t j = 0; j < queue_create_info.queueCount; ++j) {
                    const float queue_priority = queue_create_info.pQueuePriorities[j];
                    if (!(queue_priority >= 0.f) || !(queue_priority <= 1.f)) {
                        skip |= LogError("VUID-VkDeviceQueueCreateInfo-pQueuePriorities-00383", physicalDevice,
                                         create_info_loc.dot(Field::pQueueCreateInfos, i).dot(Field::pQueuePriorities, j),
                                         "(%f) is not between 0 and 1 (inclusive).", queue_priority);
                    }
                }
            }

            // Need to know if protectedMemory feature is passed in preCall to creating the device
            VkBool32 protected_memory = VK_FALSE;
            const auto *protected_features =
                vku::FindStructInPNextChain<VkPhysicalDeviceProtectedMemoryFeatures>(pCreateInfo->pNext);
            if (protected_features) {
                protected_memory = protected_features->protectedMemory;
            } else if (vulkan_11_features) {
                protected_memory = vulkan_11_features->protectedMemory;
            }
            if (((queue_create_info.flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) != 0) && (protected_memory == VK_FALSE)) {
                skip |= LogError(
                    "VUID-VkDeviceQueueCreateInfo-flags-02861", physicalDevice, create_info_loc.dot(Field::flags),
                    "includes VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT, but the protectedMemory feature is not being enabled.");
            }
        }
    }

    // feature dependencies for VK_KHR_variable_pointers
    {
        const auto *variable_pointers_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceVariablePointersFeatures>(pCreateInfo->pNext);
        VkBool32 variable_pointers = VK_FALSE;
        VkBool32 variable_pointers_storage_buffer = VK_FALSE;
        if (vulkan_11_features) {
            variable_pointers = vulkan_11_features->variablePointers;
            variable_pointers_storage_buffer = vulkan_11_features->variablePointersStorageBuffer;
        } else if (variable_pointers_features) {
            variable_pointers = variable_pointers_features->variablePointers;
            variable_pointers_storage_buffer = variable_pointers_features->variablePointersStorageBuffer;
        }
        if ((variable_pointers == VK_TRUE) && (variable_pointers_storage_buffer == VK_FALSE)) {
            skip |=
                LogError("VUID-VkPhysicalDeviceVariablePointersFeatures-variablePointers-01431", physicalDevice, error_obj.location,
                         "If variablePointers is VK_TRUE then variablePointersStorageBuffer also needs to be VK_TRUE");
        }
    }

    // feature dependencies for VK_KHR_multiview
    {
        const auto *multiview_features = vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
        VkBool32 multiview = VK_FALSE;
        VkBool32 multiview_geometry_shader = VK_FALSE;
        VkBool32 multiview_tessellation_shader = VK_FALSE;
        if (vulkan_11_features) {
            multiview = vulkan_11_features->multiview;
            multiview_geometry_shader = vulkan_11_features->multiviewGeometryShader;
            multiview_tessellation_shader = vulkan_11_features->multiviewTessellationShader;
        } else if (multiview_features) {
            multiview = multiview_features->multiview;
            multiview_geometry_shader = multiview_features->multiviewGeometryShader;
            multiview_tessellation_shader = multiview_features->multiviewTessellationShader;
        }
        if ((multiview == VK_FALSE) && (multiview_geometry_shader == VK_TRUE)) {
            skip |= LogError("VUID-VkPhysicalDeviceMultiviewFeatures-multiviewGeometryShader-00580", physicalDevice,
                             error_obj.location, "If multiviewGeometryShader is VK_TRUE then multiview also needs to be VK_TRUE");
        }
        if ((multiview == VK_FALSE) && (multiview_tessellation_shader == VK_TRUE)) {
            skip |=
                LogError("VUID-VkPhysicalDeviceMultiviewFeatures-multiviewTessellationShader-00581", physicalDevice,
                         error_obj.location, "If multiviewTessellationShader is VK_TRUE then multiview also needs to be VK_TRUE");
        }

        const auto *fsr_features = vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext);
        const auto *mesh_shader_features = vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(pCreateInfo->pNext);
        if (mesh_shader_features) {
            if ((multiview == VK_FALSE) && (mesh_shader_features->multiviewMeshShader)) {
                skip |= LogError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-multiviewMeshShader-07032", physicalDevice,
                                 error_obj.location, "If multiviewMeshShader is VK_TRUE then multiview also needs to be VK_TRUE");
            }
            if ((!fsr_features || !fsr_features->primitiveFragmentShadingRate) &&
                (mesh_shader_features->primitiveFragmentShadingRateMeshShader)) {
                skip |= LogError("VUID-VkPhysicalDeviceMeshShaderFeaturesEXT-primitiveFragmentShadingRateMeshShader-07033",
                                 physicalDevice, error_obj.location,
                                 "If primitiveFragmentShadingRateMeshShader is VK_TRUE then primitiveFragmentShadingRate also "
                                 "needs to be VK_TRUE");
            }
        }
    }

    if (features && features->robustBufferAccess && any_update_after_bind_feature) {
        VkPhysicalDeviceDescriptorIndexingProperties di_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&di_props);
        DispatchGetPhysicalDeviceProperties2Helper(api_version, physicalDevice, &props2);
        if (!di_props.robustBufferAccessUpdateAfterBind) {
            skip |= LogError("VUID-VkDeviceCreateInfo-robustBufferAccess-10247", physicalDevice, error_obj.location,
                             "robustBufferAccessUpdateAfterBind is false, but both robustBufferAccess and a "
                             "descriptorBinding*UpdateAfterBind feature are enabled.");
        }
    }

    if (const auto *fragment_shading_rate_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext)) {
        const VkPhysicalDeviceShadingRateImageFeaturesNV *shading_rate_image_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);

        if (shading_rate_image_features && shading_rate_image_features->shadingRateImage) {
            if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04478", physicalDevice, error_obj.location,
                                 "Cannot enable shadingRateImage and pipelineFragmentShadingRate features simultaneously.");
            }
            if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04479", physicalDevice, error_obj.location,
                                 "Cannot enable shadingRateImage and primitiveFragmentShadingRate features simultaneously.");
            }
            if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04480", physicalDevice, error_obj.location,
                                 "Cannot enable shadingRateImage and attachmentFragmentShadingRate features "
                                 "simultaneously.");
            }
        }

        const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *fragment_density_map_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(pCreateInfo->pNext);

        if (fragment_density_map_features && fragment_density_map_features->fragmentDensityMap) {
            if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04481", physicalDevice, error_obj.location,
                                 "Cannot enable fragmentDensityMap and pipelineFragmentShadingRate features "
                                 "simultaneously.");
            }
            if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04482", physicalDevice, error_obj.location,
                                 "Cannot enable fragmentDensityMap and primitiveFragmentShadingRate features "
                                 "simultaneously.");
            }
            if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04483", physicalDevice, error_obj.location,
                                 "Cannot enable fragmentDensityMap and attachmentFragmentShadingRate features "
                                 "simultaneously.");
            }
        }
    }

    if (const auto *shader_image_atomic_int64_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(pCreateInfo->pNext)) {
        if (shader_image_atomic_int64_features->sparseImageInt64Atomics &&
            !shader_image_atomic_int64_features->shaderImageInt64Atomics) {
            skip |= LogError("VUID-VkDeviceCreateInfo-None-04896", physicalDevice, error_obj.location,
                             "If sparseImageInt64Atomics feature is enabled then shaderImageInt64Atomics "
                             "feature must also be enabled.");
        }
    }

    if (const auto *shader_atomic_float_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(pCreateInfo->pNext)) {
        if (shader_atomic_float_features->sparseImageFloat32Atomics && !shader_atomic_float_features->shaderImageFloat32Atomics) {
            skip |= LogError("VUID-VkDeviceCreateInfo-None-04897", physicalDevice, error_obj.location,
                             "If sparseImageFloat32Atomics feature is enabled then shaderImageFloat32Atomics "
                             "feature must also be enabled.");
        }
        if (shader_atomic_float_features->sparseImageFloat32AtomicAdd &&
            !shader_atomic_float_features->shaderImageFloat32AtomicAdd) {
            skip |= LogError("VUID-VkDeviceCreateInfo-None-04898", physicalDevice, error_obj.location,
                             "If sparseImageFloat32AtomicAdd feature is enabled then shaderImageFloat32AtomicAdd "
                             "feature must also be enabled.");
        }
    }

    if (const auto *shader_atomic_float2_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(pCreateInfo->pNext)) {
        if (shader_atomic_float2_features->sparseImageFloat32AtomicMinMax &&
            !shader_atomic_float2_features->shaderImageFloat32AtomicMinMax) {
            skip |= LogError("VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975", physicalDevice, error_obj.location,
                             "If sparseImageFloat32AtomicMinMax feature is enabled then shaderImageFloat32AtomicMinMax "
                             "feature must also be enabled.");
        }
    }

    if (const auto *device_group_ci = vku::FindStructInPNextChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext)) {
        for (uint32_t i = 0; i < device_group_ci->physicalDeviceCount - 1; ++i) {
            for (uint32_t j = i + 1; j < device_group_ci->physicalDeviceCount; ++j) {
                if (device_group_ci->pPhysicalDevices[i] == device_group_ci->pPhysicalDevices[j]) {
                    skip |=
                        LogError("VUID-VkDeviceGroupDeviceCreateInfo-pPhysicalDevices-00375", physicalDevice, error_obj.location,
                                 "VkDeviceGroupDeviceCreateInfo has a duplicated physical device "
                                 "in pPhysicalDevices [%" PRIu32 "] and [%" PRIu32 "].",
                                 i, j);
                }
            }
        }
    }

    const auto *cache_control = vku::FindStructInPNextChain<VkDevicePipelineBinaryInternalCacheControlKHR>(pCreateInfo->pNext);
    if (cache_control && cache_control->disableInternalCache) {
        VkPhysicalDevicePipelineBinaryPropertiesKHR pipeline_binary_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&pipeline_binary_props);
        DispatchGetPhysicalDeviceProperties2Helper(api_version, physicalDevice, &props2);

        if (!pipeline_binary_props.pipelineBinaryInternalCacheControl) {
            skip |= LogError("VUID-VkDevicePipelineBinaryInternalCacheControlKHR-disableInternalCache-09602", physicalDevice,
                             error_obj.location,
                             "If disableInternalCache is VK_TRUE then pipelineBinaryInternalCacheControl must also be VK_TRUE");
        }
    }
    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    VkImageFormatProperties2 *pImageFormatProperties, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pImageFormatInfo != nullptr) {
        const Location format_info_loc = error_obj.location.dot(Field::pImageFormatInfo);
        const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(pImageFormatInfo->pNext);
        if (image_stencil_struct != nullptr) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
                VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
                // No flags other than the legal attachment bits may be set
                legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
                    skip |= LogError("VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539", physicalDevice,
                                     format_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage), "is %s.",
                                     string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
                }
            }
        }
        const auto image_drm_format = vku::FindStructInPNextChain<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>(pImageFormatInfo->pNext);
        if (image_drm_format) {
            if (pImageFormatInfo->tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                skip |= LogError("VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249", physicalDevice,
                                 format_info_loc.dot(Field::tiling),
                                 "(%s) but no VkPhysicalDeviceImageDrmFormatModifierInfoEXT in pNext chain.",
                                 string_VkImageTiling(pImageFormatInfo->tiling));
            }
            if (image_drm_format->sharingMode == VK_SHARING_MODE_CONCURRENT) {
                if (image_drm_format->queueFamilyIndexCount <= 1) {
                    skip |=
                        LogError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315", physicalDevice,
                                 format_info_loc.pNext(Struct::VkPhysicalDeviceImageDrmFormatModifierInfoEXT, Field::sharingMode),
                                 "is VK_SHARING_MODE_CONCURRENT, but queueFamilyIndexCount is %" PRIu32 ".",
                                 image_drm_format->queueFamilyIndexCount);
                } else if (!image_drm_format->pQueueFamilyIndices) {
                    skip |= LogError(
                        "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02314", physicalDevice,
                        format_info_loc.pNext(Struct::VkPhysicalDeviceImageDrmFormatModifierInfoEXT, Field::sharingMode),
                        "is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount is %" PRIu32 ", but pQueueFamilyIndices is NULL.",
                        image_drm_format->queueFamilyIndexCount);
                } else {
                    uint32_t queue_family_property_count = 0;
                    DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(api_version, physicalDevice, &queue_family_property_count,
                                                                          nullptr);
                    vvl::unordered_set<uint32_t> queue_family_indices_set;
                    for (uint32_t i = 0; i < image_drm_format->queueFamilyIndexCount; i++) {
                        const uint32_t queue_index = image_drm_format->pQueueFamilyIndices[i];
                        if (queue_family_indices_set.find(queue_index) != queue_family_indices_set.end()) {
                            skip |= LogError("VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316", physicalDevice,
                                             format_info_loc.pNext(Struct::VkPhysicalDeviceImageDrmFormatModifierInfoEXT,
                                                                   Field::pQueueFamilyIndices, i),
                                             "is %" PRIu32 ", but is duplicated in pQueueFamilyIndices.", queue_index);
                            break;
                        } else if (queue_index >= queue_family_property_count) {
                            skip |= LogError(
                                "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316", physicalDevice,
                                format_info_loc.pNext(Struct::VkPhysicalDeviceImageDrmFormatModifierInfoEXT,
                                                      Field::pQueueFamilyIndices, i),
                                "is %" PRIu32
                                ", but vkGetPhysicalDeviceQueueFamilyProperties2::pQueueFamilyPropertyCount returned %" PRIu32 ".",
                                queue_index, queue_family_property_count);
                        }
                        queue_family_indices_set.emplace(queue_index);
                    }
                }
            }
        } else {
            if (pImageFormatInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                skip |= LogError("VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249", physicalDevice,
                                 format_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, but pNext chain not include "
                                 "VkPhysicalDeviceImageDrmFormatModifierInfoEXT.");
            }
        }
        if (pImageFormatInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT &&
            (pImageFormatInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
            const auto format_list = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(pImageFormatInfo->pNext);
            if (!format_list || format_list->viewFormatCount == 0) {
                skip |= LogError(
                    "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313", physicalDevice, format_info_loc,
                    "tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT and flags contain VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT "
                    "bit, but the pNext chain does not contain an instance of VkImageFormatListCreateInfo with non-zero "
                    "viewFormatCount.");
            }
        }
    }

    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                            VkImageType type, VkImageTiling tiling,
                                                                            VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                            VkImageFormatProperties *pImageFormatProperties,
                                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        skip |= LogError("VUID-vkGetPhysicalDeviceImageFormatProperties-tiling-02248", physicalDevice,
                         error_obj.location.dot(Field::tiling), "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.");
    }

    return skip;
}

bool Device::manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo,
                                                              const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location name_info_loc = error_obj.location.dot(Field::pNameInfo);
    if (pNameInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02587", device, name_info_loc.dot(Field::objectType),
                         "cannot be VK_OBJECT_TYPE_UNKNOWN.");
    }
    if (pNameInfo->objectHandle == HandleToUint64(VK_NULL_HANDLE)) {
        skip |= LogError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02588", device, name_info_loc.dot(Field::objectHandle),
                         "cannot be VK_NULL_HANDLE.");
    }

    if ((pNameInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) && (pNameInfo->objectHandle == HandleToUint64(VK_NULL_HANDLE))) {
        skip |= LogError("VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02589", device, name_info_loc.dot(Field::objectType),
                         "is VK_OBJECT_TYPE_UNKNOWN but objectHandle is VK_NULL_HANDLE");
    }

    return skip;
}

bool Device::manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pTagInfo->objectType == VK_OBJECT_TYPE_UNKNOWN) {
        skip |= LogError("VUID-VkDebugUtilsObjectTagInfoEXT-objectType-01908", device,
                         error_obj.location.dot(Field::pTagInfo).dot(Field::objectType), "cannot be VK_OBJECT_TYPE_UNKNOWN.");
    }
    return skip;
}

bool Instance::manual_PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                                  VkPhysicalDeviceProperties2 *pProperties,
                                                                  const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const auto *api_props_lists = vku::FindStructInPNextChain<VkPhysicalDeviceLayeredApiPropertiesListKHR>(pProperties->pNext);
    if (api_props_lists && api_props_lists->pLayeredApis) {
        for (uint32_t i = 0; i < api_props_lists->layeredApiCount; i++) {
            if (const auto *api_vulkan_props = vku::FindStructInPNextChain<VkPhysicalDeviceLayeredApiVulkanPropertiesKHR>(
                    api_props_lists->pLayeredApis[i].pNext)) {
                const VkBaseOutStructure *current = static_cast<const VkBaseOutStructure *>(api_vulkan_props->properties.pNext);
                while (current) {
                    // only VkPhysicalDeviceDriverProperties and VkPhysicalDeviceIDProperties allowed
                    if (current->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES &&
                        current->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES) {
                        skip |= LogError("VUID-VkPhysicalDeviceLayeredApiVulkanPropertiesKHR-pNext-10011", physicalDevice,
                                         error_obj.location.dot(Field::pProperties)
                                             .pNext(Struct::VkPhysicalDeviceLayeredApiPropertiesListKHR, Field::pLayeredApis, i)
                                             .dot(Field::properties)
                                             .dot(Field::pNext),
                                         "contains an invalid struct (%s).", string_VkStructureType(current->sType));
                    }
                    current = current->pNext;
                }
            }
        }
    }
    return skip;
}
}  // namespace stateless
