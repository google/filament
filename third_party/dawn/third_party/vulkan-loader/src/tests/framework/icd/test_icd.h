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

#include "test_util.h"

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
// clang-format on

// Move only type because it holds a DispatchableHandle<VkPhysicalDevice>
struct PhysicalDevice {
    PhysicalDevice() {}
    PhysicalDevice(std::string name) : deviceName(name) {}
    PhysicalDevice(const char* name) : deviceName(name) {}

    DispatchableHandle<VkPhysicalDevice> vk_physical_device;
    BUILDER_VALUE(PhysicalDevice, std::string, deviceName, "")
    BUILDER_VALUE(PhysicalDevice, VkPhysicalDeviceProperties, properties, {})
    BUILDER_VALUE(PhysicalDevice, VkPhysicalDeviceFeatures, features, {})
    BUILDER_VALUE(PhysicalDevice, VkPhysicalDeviceMemoryProperties, memory_properties, {})
    BUILDER_VALUE(PhysicalDevice, VkImageFormatProperties, image_format_properties, {})
    BUILDER_VALUE(PhysicalDevice, VkExternalMemoryProperties, external_memory_properties, {})
    BUILDER_VALUE(PhysicalDevice, VkExternalSemaphoreProperties, external_semaphore_properties, {})
    BUILDER_VALUE(PhysicalDevice, VkExternalFenceProperties, external_fence_properties, {})
    BUILDER_VALUE(PhysicalDevice, uint32_t, pci_bus, {})

    BUILDER_VECTOR(PhysicalDevice, MockQueueFamilyProperties, queue_family_properties, queue_family_properties)
    BUILDER_VECTOR(PhysicalDevice, VkFormatProperties, format_properties, format_properties)
    BUILDER_VECTOR(PhysicalDevice, VkSparseImageFormatProperties, sparse_image_format_properties, sparse_image_format_properties)

    BUILDER_VECTOR(PhysicalDevice, Extension, extensions, extension)

    BUILDER_VALUE(PhysicalDevice, VkSurfaceCapabilitiesKHR, surface_capabilities, {})
    BUILDER_VECTOR(PhysicalDevice, VkSurfaceFormatKHR, surface_formats, surface_format)
    BUILDER_VECTOR(PhysicalDevice, VkPresentModeKHR, surface_present_modes, surface_present_mode)
    BUILDER_VALUE(PhysicalDevice, VkSurfacePresentScalingCapabilitiesEXT, surface_present_scaling_capabilities, {})
    // No good way to make this a builder value. Each std::vector<VkPresentModeKHR> corresponds to each surface_present_modes
    // element
    std::vector<std::vector<VkPresentModeKHR>> surface_present_mode_compatibility{};

    BUILDER_VECTOR(PhysicalDevice, VkDisplayPropertiesKHR, display_properties, display_properties)
    BUILDER_VECTOR(PhysicalDevice, VkDisplayPlanePropertiesKHR, display_plane_properties, display_plane_properties)
    BUILDER_VECTOR(PhysicalDevice, VkDisplayKHR, displays, displays)
    BUILDER_VECTOR(PhysicalDevice, VkDisplayModePropertiesKHR, display_mode_properties, display_mode_properties)
    BUILDER_VALUE(PhysicalDevice, VkDisplayModeKHR, display_mode, {})
    BUILDER_VALUE(PhysicalDevice, VkDisplayPlaneCapabilitiesKHR, display_plane_capabilities, {})

    BUILDER_VALUE(PhysicalDevice, VkLayeredDriverUnderlyingApiMSFT, layered_driver_underlying_api,
                  VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT)

    PhysicalDevice& set_api_version(uint32_t version) {
        properties.apiVersion = version;
        return *this;
    }

    PhysicalDevice&& finish() { return std::move(*this); }

    // Objects created from this physical device
    std::vector<VkDevice> device_handles;
    std::vector<DeviceCreateInfo> device_create_infos;
    std::vector<DispatchableHandle<VkQueue>> queue_handles;

    // Unknown physical device functions. Add a `VulkanFunction` to this list which will be searched in
    // vkGetInstanceProcAddr for custom_instance_functions and vk_icdGetPhysicalDeviceProcAddr for custom_physical_device_functions.
    // To add unknown device functions, add it to the PhysicalDevice directly (in the known_device_functions member)
    BUILDER_VECTOR(PhysicalDevice, VulkanFunction, custom_physical_device_functions, custom_physical_device_function)

    // List of function names which are 'known' to the physical device but have test defined implementations
    // The purpose of this list is so that vkGetDeviceProcAddr returns 'a real function pointer' in tests
    // without actually implementing any of the logic inside of it.
    BUILDER_VECTOR(PhysicalDevice, VulkanFunction, known_device_functions, device_function)
};

struct PhysicalDeviceGroup {
    PhysicalDeviceGroup() {}
    PhysicalDeviceGroup(PhysicalDevice const& physical_device) { physical_device_handles.push_back(&physical_device); }
    PhysicalDeviceGroup(std::vector<PhysicalDevice*> const& physical_devices) {
        physical_device_handles.insert(physical_device_handles.end(), physical_devices.begin(), physical_devices.end());
    }
    PhysicalDeviceGroup& use_physical_device(PhysicalDevice const& physical_device) {
        physical_device_handles.push_back(&physical_device);
        return *this;
    }

    std::vector<PhysicalDevice const*> physical_device_handles;
    VkBool32 subset_allocation = false;
};

struct TestICD {
    std::filesystem::path manifest_file_path;

    BUILDER_VALUE(TestICD, bool, exposes_vk_icdNegotiateLoaderICDInterfaceVersion, true)
    BUILDER_VALUE(TestICD, bool, exposes_vkEnumerateInstanceExtensionProperties, true)
    BUILDER_VALUE(TestICD, bool, exposes_vkCreateInstance, true)
    BUILDER_VALUE(TestICD, bool, exposes_vk_icdGetPhysicalDeviceProcAddr, true)
#if defined(WIN32)
    BUILDER_VALUE(TestICD, bool, exposes_vk_icdEnumerateAdapterPhysicalDevices, true)
#endif

    CalledICDGIPA called_vk_icd_gipa = CalledICDGIPA::not_called;
    CalledNegotiateInterface called_negotiate_interface = CalledNegotiateInterface::not_called;

    InterfaceVersionCheck interface_version_check = InterfaceVersionCheck::not_called;
    BUILDER_VALUE(TestICD, uint32_t, min_icd_interface_version, 0)
    BUILDER_VALUE(TestICD, uint32_t, max_icd_interface_version, 7)
    uint32_t icd_interface_version_received = 0;

    bool called_enumerate_adapter_physical_devices = false;

    BUILDER_VALUE(TestICD, bool, enable_icd_wsi, false);
    bool is_using_icd_wsi = false;

    TestICD& setup_WSI(const char* api_selection = nullptr) {
        enable_icd_wsi = true;
        add_instance_extensions({"VK_KHR_surface", get_platform_wsi_extension(api_selection)});
        min_icd_interface_version = (3U < min_icd_interface_version) ? min_icd_interface_version : 3U;
        return *this;
    }

    BUILDER_VALUE(TestICD, uint32_t, icd_api_version, VK_API_VERSION_1_0)
    BUILDER_VECTOR(TestICD, LayerDefinition, instance_layers, instance_layer)
    BUILDER_VECTOR(TestICD, Extension, instance_extensions, instance_extension)
    std::vector<Extension> enabled_instance_extensions;

    BUILDER_VECTOR_MOVE_ONLY(TestICD, PhysicalDevice, physical_devices, physical_device);

    BUILDER_VECTOR(TestICD, PhysicalDeviceGroup, physical_device_groups, physical_device_group);

    DispatchableHandle<VkInstance> instance_handle;
    std::vector<DispatchableHandle<VkDevice>> device_handles;
    std::vector<uint64_t> surface_handles;
    std::vector<uint64_t> messenger_handles;
    std::vector<uint64_t> callback_handles;
    std::vector<uint64_t> swapchain_handles;

    BUILDER_VALUE(TestICD, bool, can_query_vkEnumerateInstanceVersion, true);
    BUILDER_VALUE(TestICD, bool, can_query_GetPhysicalDeviceFuncs, true);

    // Unknown instance functions Add a `VulkanFunction` to this list which will be searched in
    // vkGetInstanceProcAddr for custom_instance_functions and vk_icdGetPhysicalDeviceProcAddr for
    // custom_physical_device_functions. To add unknown device functions, add it to the PhysicalDevice directly (in the
    // known_device_functions member)
    BUILDER_VECTOR(TestICD, VulkanFunction, custom_instance_functions, custom_instance_function)

    // Must explicitely state support for the tooling info extension, that way we can control if vkGetInstanceProcAddr returns a
    // function pointer for vkGetPhysicalDeviceToolPropertiesEXT or vkGetPhysicalDeviceToolProperties (core version)
    BUILDER_VALUE(TestICD, bool, supports_tooling_info_ext, false);
    BUILDER_VALUE(TestICD, bool, supports_tooling_info_core, false);
    // List of tooling properties that this driver 'supports'
    BUILDER_VECTOR(TestICD, VkPhysicalDeviceToolPropertiesEXT, tooling_properties, tooling_property)
    std::vector<DispatchableHandle<VkCommandBuffer>> allocated_command_buffers;

    VkInstanceCreateFlags passed_in_instance_create_flags{};

    BUILDER_VALUE(TestICD, VkResult, enum_physical_devices_return_code, VK_SUCCESS);
    BUILDER_VALUE(TestICD, VkResult, enum_adapter_physical_devices_return_code, VK_SUCCESS);

    PhysicalDevice& GetPhysDevice(VkPhysicalDevice physicalDevice) {
        for (auto& phys_dev : physical_devices) {
            if (phys_dev.vk_physical_device.handle == physicalDevice) return phys_dev;
        }
        assert(false && "vkPhysicalDevice not found!");
        return physical_devices[0];
    }

    InstanceCreateInfo GetVkInstanceCreateInfo() {
        InstanceCreateInfo info;
        for (auto& layer : instance_layers) info.enabled_layers.push_back(layer.layerName.data());
        for (auto& ext : instance_extensions) info.enabled_extensions.push_back(ext.extensionName.data());
        return info;
    }

    struct FindDevice {
        bool found = false;
        uint32_t phys_dev_index = 0;
        uint32_t dev_index = 0;
    };

    FindDevice lookup_device(VkDevice device) {
        FindDevice fd{};
        for (uint32_t p = 0; p < physical_devices.size(); p++) {
            auto const& phys_dev = physical_devices.at(p);
            for (uint32_t d = 0; d < phys_dev.device_handles.size(); d++) {
                if (phys_dev.device_handles.at(d) == device) {
                    fd.found = true;
                    fd.phys_dev_index = p;
                    fd.dev_index = d;
                    return fd;
                }
            }
        }
        return fd;
    }

#if defined(WIN32)
    BUILDER_VALUE(TestICD, LUID, adapterLUID, {})
#endif  // defined(WIN32)
};

using GetTestICDFunc = TestICD* (*)();
#define GET_TEST_ICD_FUNC_STR "get_test_icd_func"

using GetNewTestICDFunc = TestICD* (*)();
#define RESET_ICD_FUNC_STR "reset_icd_func"
