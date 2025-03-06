/*
 * Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2022 Valve Corporation
 * Copyright (c) 2021-2022 LunarG, Inc.
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

#include <functional>

#include "layer/layer_util.h"

#include "loader/generated/vk_layer_dispatch_table.h"

/*
Interface Version 0
*/

/*
must export the following: -- always exported
vkEnumerateInstanceLayerProperties
vkEnumerateInstanceExtensionProperties
Must export the following but nothing -- always exported
vkEnumerateDeviceLayerProperties
vkEnumerateDeviceExtensionProperties
*/

// export test_layer_GetInstanceProcAddr(instance, pName)
// TEST_LAYER_EXPORT_LAYER_NAMED_GIPA
// or (single layer binary)
// export vkGetInstanceProcAddr
// TEST_LAYER_EXPORT_NO_NAME_GIPA

// export test_layer_GetDeviceProcAddr(device, pName)
// TEST_LAYER_EXPORT_LAYER_NAMED_GDPA
// or (single layer binary)
// export vkGetDeviceProcAddr
// TEST_LAYER_EXPORT_NO_NAME_GDPA

/*
Interface Version 1
*/
// export GetInstanceProcAddr
// TEST_LAYER_EXPORT_NO_PREFIX_GIPA

// export GetDeviceProcAddr
// TEST_LAYER_EXPORT_NO_PREFIX_GDPA

// Layer Manifest can override the names of the GetInstanceProcAddr and GetDeviceProcAddrfunctions

/*
Interface Version 2
*/
// export vk_layerGetPhysicalDeviceProcAddr
// TEST_LAYER_EXPORT_GET_PHYSICAL_DEVICE_PROC_ADDR

// export vkNegotiateLoaderLayerInterfaceVersion
// TEST_LAYER_EXPORT_NEGOTIATE_LOADER_LAYER_INTERFACE_VERSION

// Added manifest version 1.1.0

struct TestLayer;

// Callbacks allow tests to implement custom functionality without modifying the layer binary
// TestLayer* layer - Access to the TestLayer object itself
// void* data - pointer to test specific thing, used to pass data from the test into the TestLayer
// Returns VkResult - This value will be used as the return value of the function
using FP_layer_callback = VkResult (*)(TestLayer& layer, void* data);

struct TestLayer {
    std::filesystem::path manifest_file_path;
    uint32_t manifest_version = VK_MAKE_API_VERSION(0, 1, 1, 2);

    BUILDER_VALUE(TestLayer, bool, is_meta_layer, false)

    BUILDER_VALUE(TestLayer, uint32_t, api_version, VK_API_VERSION_1_0)
    BUILDER_VALUE(TestLayer, uint32_t, reported_layer_props, 1)
    BUILDER_VALUE(TestLayer, uint32_t, reported_extension_props, 0)
    BUILDER_VALUE(TestLayer, uint32_t, reported_instance_version, VK_API_VERSION_1_0)
    BUILDER_VALUE(TestLayer, uint32_t, implementation_version, 2)
    BUILDER_VALUE(TestLayer, uint32_t, min_implementation_version, 0)
    BUILDER_VALUE(TestLayer, std::string, description, {})

    // Some layers may try to change the API version during instance creation - we should allow testing of such behavior
    BUILDER_VALUE(TestLayer, uint32_t, alter_api_version, VK_API_VERSION_1_0)

    BUILDER_VECTOR(TestLayer, std::string, alternative_function_names, alternative_function_name)

    BUILDER_VECTOR(TestLayer, Extension, instance_extensions, instance_extension)
    std::vector<Extension> enabled_instance_extensions;

    BUILDER_VECTOR(TestLayer, Extension, device_extensions, device_extension)

    BUILDER_VALUE(TestLayer, std::string, enable_environment, {});
    BUILDER_VALUE(TestLayer, std::string, disable_environment, {});

    // Modifies the extension list returned by vkEnumerateInstanceExtensionProperties to include what is in this vector
    BUILDER_VECTOR(TestLayer, Extension, injected_instance_extensions, injected_instance_extension)
    // Modifies the extension list returned by  vkEnumerateDeviceExtensionProperties to include what is in this vector
    BUILDER_VECTOR(TestLayer, Extension, injected_device_extensions, injected_device_extension)

    BUILDER_VECTOR(TestLayer, LayerDefinition, meta_component_layers, meta_component_layer);

    BUILDER_VALUE(TestLayer, bool, intercept_vkEnumerateInstanceExtensionProperties, false)
    BUILDER_VALUE(TestLayer, bool, intercept_vkEnumerateInstanceLayerProperties, false)
    // Called in vkCreateInstance after calling down the chain & returning
    BUILDER_VALUE(TestLayer, std::function<VkResult(TestLayer& layer)>, create_instance_callback, {})
    // Called in vkCreateDevice after calling down the chain & returning
    BUILDER_VALUE(TestLayer, std::function<VkResult(TestLayer& layer)>, create_device_callback, {})

    // Physical device modifier test flags and members.  This data is primarily used to test adding, removing and
    // re-ordering physical device data in a layer.
    BUILDER_VALUE(TestLayer, bool, add_phys_devs, false)
    BUILDER_VALUE(TestLayer, bool, remove_phys_devs, false)
    BUILDER_VALUE(TestLayer, bool, reorder_phys_devs, false)
    BUILDER_VECTOR(TestLayer, VkPhysicalDevice, complete_physical_devices, complete_physical_device)
    BUILDER_VECTOR(TestLayer, VkPhysicalDevice, removed_physical_devices, removed_physical_device)
    BUILDER_VECTOR(TestLayer, VkPhysicalDevice, added_physical_devices, added_physical_device)
    BUILDER_VECTOR(TestLayer, VkPhysicalDeviceGroupProperties, complete_physical_device_groups, complete_physical_device_group)
    BUILDER_VECTOR(TestLayer, VkPhysicalDeviceGroupProperties, removed_physical_device_groups, removed_physical_device_group)
    BUILDER_VECTOR(TestLayer, VkPhysicalDeviceGroupProperties, added_physical_device_groups, added_physical_device_group)

    BUILDER_VECTOR(TestLayer, VulkanFunction, custom_physical_device_implementation_functions,
                   custom_physical_device_implementation_function)
    BUILDER_VECTOR(TestLayer, VulkanFunction, custom_device_implementation_functions, custom_device_implementation_function)

    // Only need a single map for all 'custom' function - assumes that all function names are distinct, IE there cannot be a
    // physical device and device level function with the same name
    std::unordered_map<std::string, PFN_vkVoidFunction> custom_dispatch_functions;
    std::vector<VulkanFunction> custom_physical_device_interception_functions;
    TestLayer& add_custom_physical_device_intercept_function(std::string func_name, PFN_vkVoidFunction function) {
        custom_physical_device_interception_functions.push_back({func_name, function});
        custom_dispatch_functions[func_name] = nullptr;
        return *this;
    }
    std::vector<VulkanFunction> custom_device_interception_functions;
    TestLayer& add_custom_device_interception_function(std::string func_name, PFN_vkVoidFunction function) {
        custom_device_interception_functions.push_back({func_name, function});
        custom_dispatch_functions[func_name] = nullptr;
        return *this;
    }
    PFN_vkVoidFunction get_custom_intercept_function(const char* name) {
        if (custom_dispatch_functions.count(name) > 0) {
            return custom_dispatch_functions.at(name);
        }
        return nullptr;
    }

    // Allows distinguishing different layers (that use the same binary)
    BUILDER_VALUE(TestLayer, std::string, make_spurious_log_in_create_instance, "")
    BUILDER_VALUE(TestLayer, bool, do_spurious_allocations_in_create_instance, false)
    void* spurious_instance_memory_allocation = nullptr;
    BUILDER_VALUE(TestLayer, bool, do_spurious_allocations_in_create_device, false)
    struct DeviceMemAlloc {
        void* allocation;
        VkDevice device;
    };
    std::vector<DeviceMemAlloc> spurious_device_memory_allocations;

    // By default query GPDPA from GIPA, don't use value given from pNext
    BUILDER_VALUE(TestLayer, bool, use_gipa_GetPhysicalDeviceProcAddr, true)

    // Have a layer query for vkCreateDevice with a NULL instance handle
    BUILDER_VALUE(TestLayer, bool, buggy_query_of_vkCreateDevice, false)

    // Makes the layer try to create a device using the loader provided function in the layer chain
    BUILDER_VALUE(TestLayer, bool, call_create_device_while_create_device_is_called, false)
    BUILDER_VALUE(TestLayer, uint32_t, physical_device_index_to_use_during_create_device, 0)

    BUILDER_VALUE(TestLayer, bool, check_if_EnumDevExtProps_is_same_as_queried_function, false)

    // Clober the data pointed to by pInstance to overwrite the magic value
    BUILDER_VALUE(TestLayer, bool, clobber_pInstance, false)
    // Clober the data pointed to by pDevice to overwrite the magic value
    BUILDER_VALUE(TestLayer, bool, clobber_pDevice, false)

    BUILDER_VALUE(TestLayer, bool, query_vkEnumerateInstanceLayerProperties, false)
    BUILDER_VALUE(TestLayer, bool, query_vkEnumerateInstanceExtensionProperties, false)
    BUILDER_VALUE(TestLayer, bool, query_vkEnumerateInstanceVersion, false)

    PFN_vkGetInstanceProcAddr next_vkGetInstanceProcAddr = VK_NULL_HANDLE;
    PFN_GetPhysicalDeviceProcAddr next_GetPhysicalDeviceProcAddr = VK_NULL_HANDLE;
    PFN_vkGetDeviceProcAddr next_vkGetDeviceProcAddr = VK_NULL_HANDLE;

    VkInstance instance_handle = VK_NULL_HANDLE;
    VkLayerInstanceDispatchTable instance_dispatch_table{};

    struct Device {
        VkDevice device_handle = VK_NULL_HANDLE;
        VkLayerDispatchTable dispatch_table{};
        std::vector<Extension> enabled_extensions;
    };
    std::vector<Device> created_devices;

    // Stores the callback that allows layers to create devices on their own
    PFN_vkLayerCreateDevice callback_vkCreateDevice{};
    PFN_vkLayerDestroyDevice callback_vkDestroyDevice{};
    std::vector<VkPhysicalDevice> queried_physical_devices;
    Device second_device_created_during_create_device{};
};

using GetTestLayerFunc = TestLayer* (*)();
#define GET_TEST_LAYER_FUNC_STR "get_test_layer_func"

using GetNewTestLayerFunc = TestLayer* (*)();
#define RESET_LAYER_FUNC_STR "reset_layer_func"
