/*
 * Copyright (c) 2021-2023 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include <array>
#include <algorithm>
#include <filesystem>
#include <mutex>
#include <ostream>
#include <unordered_map>
#include <unordered_set>

#include "util/dispatchable_handle.h"
#include "util/platform_wsi.h"
#include "util/functions.h"

#include "layer/layer_util.h"

enum class CalledICDGIPA { not_called, vk_icd_gipa, vk_gipa };

enum class CalledNegotiateInterface { not_called, vk_icd_negotiate, vk_icd_gipa_first };

enum class InterfaceVersionCheck {
    not_called,
    loader_version_too_old,
    loader_version_too_new,
    icd_version_too_new,
    version_is_supported
};

// clang-format off
inline std::ostream& operator<<(std::ostream& os, const CalledICDGIPA& result) {
    switch (result) {
        case (CalledICDGIPA::not_called): return os << "CalledICDGIPA::not_called";
        case (CalledICDGIPA::vk_icd_gipa): return os << "CalledICDGIPA::vk_icd_gipa";
        case (CalledICDGIPA::vk_gipa): return os << "CalledICDGIPA::vk_gipa";
    }
    return os << static_cast<uint32_t>(result);
}
inline std::ostream& operator<<(std::ostream& os, const CalledNegotiateInterface& result) {
    switch (result) {
        case (CalledNegotiateInterface::not_called): return os << "CalledNegotiateInterface::not_called";
        case (CalledNegotiateInterface::vk_icd_negotiate): return os << "CalledNegotiateInterface::vk_icd_negotiate";
        case (CalledNegotiateInterface::vk_icd_gipa_first): return os << "CalledNegotiateInterface::vk_icd_gipa_first";
    }
    return os << static_cast<uint32_t>(result);
}
inline std::ostream& operator<<(std::ostream& os, const InterfaceVersionCheck& result) {
    switch (result) {
        case (InterfaceVersionCheck::not_called): return os << "InterfaceVersionCheck::not_called";
        case (InterfaceVersionCheck::loader_version_too_old): return os << "InterfaceVersionCheck::loader_version_too_old";
        case (InterfaceVersionCheck::loader_version_too_new): return os << "InterfaceVersionCheck::loader_version_too_new";
        case (InterfaceVersionCheck::icd_version_too_new): return os << "InterfaceVersionCheck::icd_version_too_new";
        case (InterfaceVersionCheck::version_is_supported): return os << "InterfaceVersionCheck::version_is_supported";
    }
    return os << static_cast<uint32_t>(result);
}

using VulkanUUID = std::array<uint8_t, VK_UUID_SIZE>;

using PFN_test_icd_internal_function = VKAPI_ATTR VkResult (VKAPI_CALL *)(VkPhysicalDevice physicalDevice, uint32_t a, uint32_t b, float c);
#define TEST_ICD_INTERNAL_FUNCTION_NAME_STRING "TEST_ICD_INTERNAL_FUNCTION_NAME"
// clang-format on

// Move only type because it holds a DispatchableHandle<VkPhysicalDevice>
struct PhysicalDevice {
    PhysicalDevice() {}
    PhysicalDevice(std::string name) : deviceName(name) {}
    PhysicalDevice(const char* name) : deviceName(name) {}

    BUILDER_VALUE(std::string, deviceName)
    BUILDER_VALUE(VulkanUUID, deviceUUID)
    BUILDER_VALUE(VulkanUUID, driverUUID)
    BUILDER_VALUE(VkPhysicalDeviceProperties, properties)
    BUILDER_VALUE(VkPhysicalDeviceFeatures, features)
    BUILDER_VALUE(VkPhysicalDeviceMemoryProperties, memory_properties)
    BUILDER_VALUE(VkPhysicalDeviceDriverProperties, driver_properties)
    BUILDER_VALUE(VkImageFormatProperties, image_format_properties)
    BUILDER_VALUE(VkExternalMemoryProperties, external_memory_properties)
    BUILDER_VALUE(VkExternalSemaphoreProperties, external_semaphore_properties)
    BUILDER_VALUE(VkExternalFenceProperties, external_fence_properties)
    BUILDER_VALUE(uint32_t, pci_bus)

    BUILDER_VECTOR(MockQueueFamilyProperties, queue_family_properties, queue_family_properties)
    BUILDER_VECTOR(VkFormatProperties, format_properties, format_properties)
    BUILDER_VECTOR(VkSparseImageFormatProperties, sparse_image_format_properties, sparse_image_format_properties)

    BUILDER_VECTOR(Extension, extensions, extension)

    BUILDER_VALUE(VkSurfaceCapabilitiesKHR, surface_capabilities)
    BUILDER_VALUE_WITH_DEFAULT(VkResult, surface_capabilities_result, VK_SUCCESS)
    BUILDER_VECTOR(VkSurfaceFormatKHR, surface_formats, surface_format)
    BUILDER_VECTOR(VkPresentModeKHR, surface_present_modes, surface_present_mode)
    BUILDER_VALUE(VkSurfacePresentScalingCapabilitiesEXT, surface_present_scaling_capabilities)
    // No good way to make this a builder value. Each std::vector<VkPresentModeKHR> corresponds to each surface_present_modes
    // element
    std::vector<std::vector<VkPresentModeKHR>> surface_present_mode_compatibility{};

    BUILDER_VECTOR(VkDisplayPropertiesKHR, display_properties, display_properties)
    BUILDER_VECTOR(VkDisplayPlanePropertiesKHR, display_plane_properties, display_plane_properties)
    BUILDER_VECTOR(VkDisplayKHR, displays, displays)
    BUILDER_VECTOR(VkDisplayModePropertiesKHR, display_mode_properties, display_mode_properties)
    BUILDER_VALUE(VkDisplayModeKHR, display_mode)
    BUILDER_VALUE(VkDisplayPlaneCapabilitiesKHR, display_plane_capabilities)

    BUILDER_VALUE_WITH_DEFAULT(VkLayeredDriverUnderlyingApiMSFT, layered_driver_underlying_api,
                               VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT)

    PhysicalDevice& set_api_version(uint32_t version) {
        properties.apiVersion = version;
        return *this;
    }

    PhysicalDevice& finish() { return *this; }

    // Unknown physical device functions. Add a `VulkanFunction` to this list which will be searched in
    // vkGetInstanceProcAddr for custom_instance_functions and vk_icdGetPhysicalDeviceProcAddr for custom_physical_device_functions.
    // To add unknown device functions, add it to the PhysicalDevice directly (in the known_device_functions member)
    BUILDER_VECTOR(VulkanFunction, custom_physical_device_functions, custom_physical_device_function)

    // List of function names which are 'known' to the physical device but have test defined implementations
    // The purpose of this list is so that vkGetDeviceProcAddr returns 'a real function pointer' in tests
    // without actually implementing any of the logic inside of it.
    BUILDER_VECTOR(VulkanFunction, known_device_functions, device_function)
};

struct PhysicalDeviceGroup {
    PhysicalDeviceGroup() {}
    PhysicalDeviceGroup(size_t physical_device_index) { physical_device_indexes.push_back(physical_device_index); }
    PhysicalDeviceGroup(std::vector<size_t> const& in_physical_device_indexes) {
        physical_device_indexes.insert(physical_device_indexes.end(), in_physical_device_indexes.begin(),
                                       in_physical_device_indexes.end());
    }
    PhysicalDeviceGroup& use_physical_device(size_t physical_device_index) {
        physical_device_indexes.push_back(physical_device_index);
        return *this;
    }
    PhysicalDeviceGroup& use_physical_devices(std::vector<size_t> const& in_physical_device_indexes) {
        physical_device_indexes.insert(physical_device_indexes.begin(), in_physical_device_indexes.begin(),
                                       in_physical_device_indexes.end());
        return *this;
    }

    std::vector<size_t> physical_device_indexes;
    VkBool32 subset_allocation = false;
};

struct CreatedDeviceDetails {
    DispatchableHandle<VkDevice> device;
    VkPhysicalDevice physical_device_created_from;
    VkInstance instance_created_from;

    std::vector<Extension> enabled_device_extensions;
    std::vector<DispatchableHandle<VkQueue>> queue_handles;

    std::vector<DispatchableHandle<VkCommandBuffer>> allocated_command_buffers;

    std::vector<uint64_t> swapchain_handles;
};

struct CreatedPhysicalDeviceDetails {
    DispatchableHandle<VkPhysicalDevice> vk_physical_device;
    VkInstance instance_created_from{};

    size_t index_physical_device{};  // index into the TestICD::physical_devices array this object represents
};

struct CreatedInstanceDetails {
    DispatchableHandle<VkInstance> instance;
    VkInstanceCreateFlags passed_in_instance_create_flags{};
    std::vector<Extension> enabled_instance_extensions;

    std::vector<uint64_t> surface_handles;
    std::vector<uint64_t> messenger_handles;
    std::vector<uint64_t> callback_handles;

    // Store the handles here in order for EnumeratePhysicalDevices
    std::vector<VkPhysicalDevice> physical_devices;
};
struct TestICD {
    std::recursive_mutex mutex;
    std::filesystem::path manifest_file_path;

    BUILDER_VALUE_WITH_DEFAULT(bool, exposes_vk_icdNegotiateLoaderICDInterfaceVersion, true)
    BUILDER_VALUE_WITH_DEFAULT(bool, exposes_vkEnumerateInstanceExtensionProperties, true)
    BUILDER_VALUE_WITH_DEFAULT(bool, exposes_vkCreateInstance, true)
    BUILDER_VALUE_WITH_DEFAULT(bool, exposes_vk_icdGetPhysicalDeviceProcAddr, true)
#if defined(WIN32)
    BUILDER_VALUE_WITH_DEFAULT(bool, exposes_vk_icdEnumerateAdapterPhysicalDevices, true)
#endif

    CalledICDGIPA called_vk_icd_gipa = CalledICDGIPA::not_called;
    CalledNegotiateInterface called_negotiate_interface = CalledNegotiateInterface::not_called;

    InterfaceVersionCheck interface_version_check = InterfaceVersionCheck::not_called;
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, min_icd_interface_version, 0)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, max_icd_interface_version, 7)
    uint32_t icd_interface_version_received = 0;

    bool called_enumerate_adapter_physical_devices = false;

    BUILDER_VALUE(bool, enable_icd_wsi);
    bool is_using_icd_wsi = false;

    TestICD& setup_WSI(const char* api_selection = nullptr) {
        enable_icd_wsi = true;
        add_instance_extensions({"VK_KHR_surface", get_platform_wsi_extension(api_selection)});
        min_icd_interface_version = (3U < min_icd_interface_version) ? min_icd_interface_version : 3U;
        return *this;
    }

    BUILDER_VALUE_WITH_DEFAULT(uint32_t, icd_api_version, VK_API_VERSION_1_0)
    BUILDER_VECTOR(LayerDefinition, instance_layers, instance_layer)
    BUILDER_VECTOR(Extension, instance_extensions, instance_extension)

    std::vector<PhysicalDevice> physical_devices;

    TestICD& add_physical_device(PhysicalDevice const& physical_device) {
        physical_devices.push_back(physical_device);
        return *this;
    }

    PhysicalDevice& add_and_get_physical_device(PhysicalDevice const& physical_device) {
        physical_devices.push_back(physical_device);
        return physical_devices.back();
    }

    PhysicalDevice& add_physical_device_at_index(size_t index, PhysicalDevice const& physical_device) {
        physical_devices.insert(physical_devices.begin() + index, physical_device);
        return physical_devices.at(index);
    }

    void remove_physical_device(size_t index) {
        // Remove all created physical devices which use this index.
        // Because we are modifying the map as we iterate, we need to use iterators rather than range-for
        for (auto iter = created_physical_device_details.begin(), end_iter = created_physical_device_details.end();
             iter != end_iter;) {
            if (iter->second.index_physical_device == index) {
                // Clean up the instance's cache of physical devices
                auto& ipd = created_instance_details.at(iter->second.instance_created_from).physical_devices;
                ipd.erase(std::remove(ipd.begin(), ipd.end(), iter->second.vk_physical_device.handle), ipd.end());

                iter = created_physical_device_details.erase(iter);
            } else
                ++iter;
        }

        physical_devices.erase(physical_devices.begin() + index);

        // Update all other detail structs to refer to their intended PhysicalDevice (accounting for the removed index)
        for (auto& [vk_physical_device, details] : created_physical_device_details) {
            if (details.index_physical_device > index) {
                details.index_physical_device--;
            }
        }
    }

    BUILDER_VECTOR(PhysicalDeviceGroup, physical_device_groups, physical_device_group);

    BUILDER_VALUE_WITH_DEFAULT(bool, can_query_vkEnumerateInstanceVersion, true);
    BUILDER_VALUE_WITH_DEFAULT(bool, can_query_GetPhysicalDeviceFuncs, true);

    // Unknown instance functions Add a `VulkanFunction` to this list which will be searched in
    // vkGetInstanceProcAddr for custom_instance_functions and vk_icdGetPhysicalDeviceProcAddr for
    // custom_physical_device_functions. To add unknown device functions, add it to the PhysicalDevice directly (in the
    // known_device_functions member)
    BUILDER_VECTOR(VulkanFunction, custom_instance_functions, custom_instance_function)

    // When true, this ICD returns function pointers when TEST_ICD_INTERNAL_FUNCTION_NAME is queried.
    BUILDER_VALUE(bool, supports_internal_function)

    // Must explicitely state support for the tooling info extension, that way we can control if vkGetInstanceProcAddr returns a
    // function pointer for vkGetPhysicalDeviceToolPropertiesEXT or vkGetPhysicalDeviceToolProperties (core version)
    BUILDER_VALUE(bool, supports_tooling_info_ext);
    BUILDER_VALUE(bool, supports_tooling_info_core);
    // List of tooling properties that this driver 'supports'
    BUILDER_VECTOR(VkPhysicalDeviceToolPropertiesEXT, tooling_properties, tooling_property)

    BUILDER_VALUE_WITH_DEFAULT(VkResult, enum_physical_devices_return_code, VK_SUCCESS);
    BUILDER_VALUE_WITH_DEFAULT(VkResult, enum_adapter_physical_devices_return_code, VK_SUCCESS);

    InstanceCreateInfo GetVkInstanceCreateInfo() {
        InstanceCreateInfo info;
        for (auto& layer : instance_layers) info.enabled_layers.push_back(layer.layerName.data());
        for (auto& ext : instance_extensions) info.enabled_extensions.push_back(ext.extensionName.data());
        return info;
    }

#if defined(WIN32)
    BUILDER_VALUE(LUID, adapterLUID)
#endif  // defined(WIN32)

    // Store all of the 'live' object details.
    std::unordered_map<VkInstance, CreatedInstanceDetails> created_instance_details;
    std::unordered_map<VkPhysicalDevice, CreatedPhysicalDeviceDetails> created_physical_device_details;
    std::unordered_map<VkDevice, CreatedDeviceDetails> created_device_details;
    std::unordered_set<VkQueue> created_queues;
    std::unordered_set<VkCommandBuffer> created_command_buffers;
};

using GetTestICDFunc = TestICD* (*)();
#define GET_TEST_ICD_FUNC_STR "get_test_icd_func"

using GetNewTestICDFunc = TestICD* (*)();
#define RESET_ICD_FUNC_STR "reset_icd_func"
