/*
 * Copyright (c) 2021-2023 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
 * Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * Copyright (c) 2023-2023 RasterGrid Kft.
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

#include "test_environment.h"

// Test case origin
// LX = lunar exchange
// LVLGH = loader and validation github
// LVLGL = loader and validation gitlab
// VL = Vulkan Loader github
// VVL = Vulkan Validation Layers github

TEST(CreateInstance, BasicRun) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}

// LX435
TEST(CreateInstance, ConstInstanceInfo) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    VkInstance inst = VK_NULL_HANDLE;
    VkInstanceCreateInfo const info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr};
    ASSERT_EQ(env.vulkan_functions.vkCreateInstance(&info, VK_NULL_HANDLE, &inst), VK_SUCCESS);
    // Must clean up
    env.vulkan_functions.vkDestroyInstance(inst, nullptr);
}

// VUID-vkDestroyInstance-instance-parameter, VUID-vkDestroyInstance-pAllocator-parameter
TEST(CreateInstance, DestroyInstanceNullHandle) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.vulkan_functions.vkDestroyInstance(VK_NULL_HANDLE, nullptr);
}

// VUID-vkDestroyDevice-device-parameter, VUID-vkDestroyDevice-pAllocator-parameter
TEST(CreateInstance, DestroyDeviceNullHandle) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.vulkan_functions.vkDestroyDevice(VK_NULL_HANDLE, nullptr);
}

// VUID-vkCreateInstance-ppEnabledExtensionNames-01388
TEST(CreateInstance, ExtensionNotPresent) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension("VK_EXT_validation_features");  // test icd won't report this as supported
        inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension("Non_existant_extension");  // unknown instance extension
        inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);
    }
}

TEST(CreateInstance, LayerNotPresent) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer("VK_NON_EXISTANT_LAYER");
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
}

TEST(CreateInstance, LayerPresent) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}).add_physical_device({});

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();
}

TEST(CreateInstance, RelativePaths) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}.set_library_path_type(LibraryPathType::relative)).add_physical_device({});

    const char* layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "test_layer.json"}
            .set_library_path_type(LibraryPathType::relative));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
}

TEST(CreateInstance, ApiVersionBelow1_0) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.create_info.api_version = 1;
    inst.CheckCreate();
    ASSERT_TRUE(
        debug_log.find("VkInstanceCreateInfo::pApplicationInfo::apiVersion has value of 1 which is not permitted. If apiVersion is "
                       "not 0, then it must be "
                       "greater than or equal to the value of VK_API_VERSION_1_0 [VUID-VkApplicationInfo-apiVersion]"));
}

TEST(CreateInstance, ConsecutiveCreate) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    for (uint32_t i = 0; i < 100; i++) {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
    }
}

TEST(CreateInstance, ConsecutiveCreateWithoutDestruction) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    std::vector<InstWrapper> instances;
    for (uint32_t i = 0; i < 100; i++) {
        instances.emplace_back(env.vulkan_functions);
        instances.back().CheckCreate();
    }
}

TEST(NoDrivers, CreateInstance) {
    FrameworkEnvironment env{};
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
}

TEST(EnumerateInstanceLayerProperties, UsageChecks) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    const char* layer_name_1 = "TestLayer1";
    const char* layer_name_2 = "TestLayer1";

    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer_1.json");

    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer_2.json");

    {  // OnePass
        VkLayerProperties layer_props[2] = {};
        uint32_t layer_count = 2;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&layer_count, layer_props));
        ASSERT_EQ(layer_count, 2U);
        auto layers = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_name_1, layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_2, layer_props[1].layerName));
    }
    {  // OnePass
        uint32_t layer_count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        ASSERT_EQ(layer_count, 2U);

        VkLayerProperties layer_props[2] = {};
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&layer_count, layer_props));
        ASSERT_EQ(layer_count, 2U);
        ASSERT_TRUE(string_eq(layer_name_1, layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_2, layer_props[1].layerName));
    }
    {  // PropertyCountLessThanAvailable
        VkLayerProperties layer_props{};
        uint32_t layer_count = 1;
        ASSERT_EQ(VK_INCOMPLETE, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&layer_count, &layer_props));
        ASSERT_TRUE(string_eq(layer_name_1, layer_props.layerName));
    }
}

TEST(EnumerateInstanceExtensionProperties, UsageChecks) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    Extension first_ext{"VK_EXT_validation_features"};  // known instance extensions
    Extension second_ext{"VK_EXT_headless_surface"};
    env.reset_icd().add_instance_extensions({first_ext, second_ext});

    {  // One Pass
        uint32_t extension_count = 6;
        std::array<VkExtensionProperties, 6> extensions;
        ASSERT_EQ(VK_SUCCESS,
                  env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));
        ASSERT_EQ(extension_count, 6U);  // default extensions + our two extensions

        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, first_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, second_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(5).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }
    {  // Two Pass
        auto extensions = env.GetInstanceExtensions(6);
        // loader always adds the debug report & debug utils extensions
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, first_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, second_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(5).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }
}

TEST(EnumerateInstanceExtensionProperties, PropertyCountLessThanAvailable) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    uint32_t extension_count = 0;
    std::array<VkExtensionProperties, 4> extensions;
    {  // use nullptr for null string
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
        ASSERT_EQ(extension_count, 4U);  // return debug report & debug utils & portability enumeration & direct driver loading
        extension_count = 1;             // artificially remove one extension

        ASSERT_EQ(VK_INCOMPLETE,
                  env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));
        ASSERT_EQ(extension_count, 1U);
        // loader always adds the debug report & debug utils extensions
        ASSERT_TRUE(string_eq(extensions[0].extensionName, "VK_EXT_debug_report"));
    }
    {  // use "" for null string
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties("", &extension_count, nullptr));
        ASSERT_EQ(extension_count, 4U);  // return debug report & debug utils & portability enumeration & direct driver loading
        extension_count = 1;             // artificially remove one extension

        ASSERT_EQ(VK_INCOMPLETE,
                  env.vulkan_functions.vkEnumerateInstanceExtensionProperties("", &extension_count, extensions.data()));
        ASSERT_EQ(extension_count, 1U);
        // loader always adds the debug report & debug utils extensions
        ASSERT_TRUE(string_eq(extensions[0].extensionName, "VK_EXT_debug_report"));
    }
}

TEST(EnumerateInstanceExtensionProperties, FilterUnkownInstanceExtensions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    Extension first_ext{"FirstTestExtension"};  // unknown instance extensions
    Extension second_ext{"SecondTestExtension"};
    env.reset_icd().add_instance_extensions({first_ext, second_ext});
    {
        auto extensions = env.GetInstanceExtensions(4);
        // loader always adds the debug report & debug utils extensions
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }
    {  // Disable unknown instance extension filtering
        EnvVarWrapper disable_inst_ext_filter_env_var{"VK_LOADER_DISABLE_INST_EXT_FILTER", "1"};

        auto extensions = env.GetInstanceExtensions(6);
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, first_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, second_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(5).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }
}

TEST(EnumerateDeviceLayerProperties, LayersMatch) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();

    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    {  // LayersMatch
        auto layer_props = inst.GetActiveLayers(phys_dev, 1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
    }
    {  // Property count less than available
        VkLayerProperties layer_props;
        uint32_t layer_count = 0;
        ASSERT_EQ(VK_INCOMPLETE, env.vulkan_functions.vkEnumerateDeviceLayerProperties(phys_dev, &layer_count, &layer_props));
        ASSERT_EQ(layer_count, 0U);
    }
}

TEST(EnumerateDeviceExtensionProperties, DeviceExtensionEnumerated) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    std::array<Extension, 2> device_extensions = {Extension{"MyExtension0", 4}, Extension{"MyExtension1", 7}};
    for (auto& ext : device_extensions) {
        driver.physical_devices.front().extensions.push_back(ext);
    }
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));

    uint32_t extension_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr));
    ASSERT_EQ(extension_count, device_extensions.size());

    std::array<VkExtensionProperties, 2> enumerated_device_exts;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
                                                                     enumerated_device_exts.data()));
    ASSERT_EQ(extension_count, device_extensions.size());
    ASSERT_TRUE(device_extensions[0].extensionName == enumerated_device_exts[0].extensionName);
    ASSERT_TRUE(device_extensions[0].specVersion == enumerated_device_exts[0].specVersion);
}

TEST(EnumerateDeviceExtensionProperties, PropertyCountLessThanAvailable) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    std::array<Extension, 2> device_extensions = {Extension{"MyExtension0", 4}, Extension{"MyExtension1", 7}};
    for (auto& ext : device_extensions) {
        driver.physical_devices.front().extensions.push_back(ext);
    }
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));

    uint32_t extension_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, "", &extension_count, nullptr));
    ASSERT_EQ(extension_count, device_extensions.size());
    extension_count -= 1;

    std::array<VkExtensionProperties, 2> enumerated_device_exts;
    ASSERT_EQ(VK_INCOMPLETE,
              inst->vkEnumerateDeviceExtensionProperties(physical_device, "", &extension_count, enumerated_device_exts.data()));
    ASSERT_EQ(extension_count, device_extensions.size() - 1);
    ASSERT_TRUE(device_extensions[0].extensionName == enumerated_device_exts[0].extensionName);
    ASSERT_TRUE(device_extensions[0].specVersion == enumerated_device_exts[0].specVersion);
}

TEST(EnumerateDeviceExtensionProperties, ZeroPhysicalDeviceExtensions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));
    inst.CheckCreate();

    auto phys_dev = inst.GetPhysDev();
    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_dev);

    uint32_t ext_count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_count, nullptr));
    ASSERT_EQ(ext_count, 0U);
    VkExtensionProperties ext_props{};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateDeviceExtensionProperties(phys_dev, nullptr, &ext_count, &ext_props));
    ASSERT_EQ(ext_count, 0U);
}

void exercise_EnumerateDeviceExtensionProperties(InstWrapper& inst, VkPhysicalDevice physical_device,
                                                 std::vector<Extension>& exts_to_expect) {
    {  // "expected enumeration pattern"
        uint32_t extension_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr));
        ASSERT_EQ(extension_count, exts_to_expect.size());

        std::vector<VkExtensionProperties> enumerated_device_exts{extension_count};
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
                                                                         enumerated_device_exts.data()));
        ASSERT_EQ(extension_count, exts_to_expect.size());
        for (uint32_t i = 0; i < exts_to_expect.size(); i++) {
            ASSERT_TRUE(exts_to_expect[i].extensionName == enumerated_device_exts[i].extensionName);
            ASSERT_EQ(exts_to_expect[i].specVersion, enumerated_device_exts[i].specVersion);
        }
    }
    {  // "Single call pattern"
        uint32_t extension_count = static_cast<uint32_t>(exts_to_expect.size());
        std::vector<VkExtensionProperties> enumerated_device_exts{extension_count};
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
                                                                         enumerated_device_exts.data()));
        ASSERT_EQ(extension_count, exts_to_expect.size());
        enumerated_device_exts.resize(extension_count);

        ASSERT_EQ(extension_count, exts_to_expect.size());
        for (uint32_t i = 0; i < exts_to_expect.size(); i++) {
            ASSERT_TRUE(exts_to_expect[i].extensionName == enumerated_device_exts[i].extensionName);
            ASSERT_EQ(exts_to_expect[i].specVersion, enumerated_device_exts[i].specVersion);
        }
    }
    {  // pPropertiesCount == NULL
        ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, nullptr, nullptr));
    }
    {  // 2nd call pass in way more than in reality
        uint32_t extension_count = std::numeric_limits<uint32_t>::max();
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr));
        ASSERT_EQ(extension_count, exts_to_expect.size());

        // reset size to a not earthshatteringly large number of extensions
        extension_count = static_cast<uint32_t>(exts_to_expect.size()) * 4;
        std::vector<VkExtensionProperties> enumerated_device_exts{extension_count};

        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
                                                                         enumerated_device_exts.data()));
        ASSERT_EQ(extension_count, exts_to_expect.size());
        for (uint32_t i = 0; i < exts_to_expect.size(); i++) {
            ASSERT_TRUE(exts_to_expect[i].extensionName == enumerated_device_exts[i].extensionName);
            ASSERT_EQ(exts_to_expect[i].specVersion, enumerated_device_exts[i].specVersion);
        }
    }
    {  // 2nd call pass in not enough, go through all possible values from 0 to exts_to_expect.size()
        uint32_t extension_count = std::numeric_limits<uint32_t>::max();
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr));
        ASSERT_EQ(extension_count, exts_to_expect.size());
        std::vector<VkExtensionProperties> enumerated_device_exts{extension_count};
        for (uint32_t i = 0; i < exts_to_expect.size() - 1; i++) {
            extension_count = i;
            ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count,
                                                                                enumerated_device_exts.data()));
            ASSERT_EQ(extension_count, i);
            for (uint32_t j = 0; j < i; j++) {
                ASSERT_TRUE(exts_to_expect[j].extensionName == enumerated_device_exts[j].extensionName);
                ASSERT_EQ(exts_to_expect[j].specVersion, enumerated_device_exts[j].specVersion);
            }
        }
    }
}

TEST(EnumerateDeviceExtensionProperties, ImplicitLayerPresentNoExtensions) {
    FrameworkEnvironment env{};

    std::vector<Extension> exts = {Extension{"MyDriverExtension0", 4}, Extension{"MyDriverExtension1", 7},
                                   Extension{"MyDriverExtension2", 6}, Extension{"MyDriverExtension3", 10}};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device("physical_device_0")
        .physical_devices.at(0)
        .add_extensions(exts);

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    exercise_EnumerateDeviceExtensionProperties(inst, inst.GetPhysDev(), exts);
}

TEST(EnumerateDeviceExtensionProperties, ImplicitLayerPresentWithExtensions) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 6; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension0", 4);
    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension1", 7);

    exts.insert(exts.begin(), driver.physical_devices.front().extensions.begin(), driver.physical_devices.front().extensions.end());

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumerateDeviceExtensionProperties, ImplicitLayerPresentWithLotsOfExtensions) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 26; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension0", 4);
    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension1", 7);
    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension2", 6);
    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension3", 9);

    exts.insert(exts.begin(), driver.physical_devices.front().extensions.begin(), driver.physical_devices.front().extensions.end());

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumerateDeviceExtensionProperties, NoDriverExtensionsImplicitLayerPresentWithExtensions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 6; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumerateDeviceExtensionProperties, NoDriverExtensionsImplicitLayerPresentWithLotsOfExtensions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 6; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumerateDeviceExtensionProperties, ImplicitLayerPresentWithDuplicateExtensions) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 26; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension0", 4);
    driver.physical_devices.front().extensions.emplace_back("MyDriverExtension1", 7);

    driver.physical_devices.front().extensions.insert(driver.physical_devices.front().extensions.end(), exts.begin(), exts.end());
    exts.emplace_back("MyDriverExtension0", 4);
    exts.emplace_back("MyDriverExtension1", 7);

    driver.physical_devices.front().extensions = exts;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumerateDeviceExtensionProperties, ImplicitLayerPresentWithOnlyDuplicateExtensions) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<Extension> exts;
    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 26; i++) {
        exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
        layer_exts.emplace_back(std::string("LayerExtNumba") + std::to_string(i), i + 10);
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_device_extensions({layer_exts})),
                           "implicit_test_layer.json");
    auto& layer = env.get_test_layer();
    layer.device_extensions = exts;

    driver.physical_devices.front().extensions = exts;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice physical_device = inst.GetPhysDev();
    exercise_EnumerateDeviceExtensionProperties(inst, physical_device, exts);
}

TEST(EnumeratePhysicalDevices, OneCall) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    driver.add_physical_device("physical_device_0");
    driver.add_physical_device("physical_device_1");
    driver.add_physical_device("physical_device_2");
    driver.add_physical_device("physical_device_3");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    std::vector<VkPhysicalDevice> physical_device_handles = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
}

TEST(EnumeratePhysicalDevices, TwoCall) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
                       .set_min_icd_interface_version(5)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    const uint32_t real_device_count = 2;
    for (uint32_t i = 0; i < real_device_count; i++) {
        driver.add_physical_device(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);

    std::array<VkPhysicalDevice, real_device_count> physical_device_handles;
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
}

TEST(EnumeratePhysicalDevices, MatchOneAndTwoCallNumbers) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
                       .set_min_icd_interface_version(5)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    const uint32_t real_device_count = 3;
    for (uint32_t i = 0; i < real_device_count; i++) {
        driver.add_physical_device(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    }

    InstWrapper inst1{env.vulkan_functions};
    inst1.CheckCreate();

    uint32_t physical_count_one_call = static_cast<uint32_t>(driver.physical_devices.size());
    std::array<VkPhysicalDevice, real_device_count> physical_device_handles_one_call;
    ASSERT_EQ(VK_SUCCESS,
              inst1->vkEnumeratePhysicalDevices(inst1, &physical_count_one_call, physical_device_handles_one_call.data()));
    ASSERT_EQ(real_device_count, physical_count_one_call);

    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst2->vkEnumeratePhysicalDevices(inst2, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);

    std::array<VkPhysicalDevice, real_device_count> physical_device_handles;
    ASSERT_EQ(VK_SUCCESS, inst2->vkEnumeratePhysicalDevices(inst2, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(real_device_count, returned_physical_count);

    ASSERT_EQ(physical_count_one_call, returned_physical_count);
}

TEST(EnumeratePhysicalDevices, TwoCallIncomplete) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
                       .set_min_icd_interface_version(5)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    const uint32_t real_device_count = 2;
    for (uint32_t i = 0; i < real_device_count; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &physical_count, nullptr));
    ASSERT_EQ(physical_count, driver.physical_devices.size());

    std::array<VkPhysicalDevice, real_device_count> physical;

    auto temp_ptr = std::unique_ptr<int>(new int());
    physical[0] = reinterpret_cast<VkPhysicalDevice>(temp_ptr.get());
    physical[1] = reinterpret_cast<VkPhysicalDevice>(temp_ptr.get());

    // Use zero for the device count so we can get the VK_INCOMPLETE message and verify nothing was written into physical
    physical_count = 0;
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &physical_count, physical.data()));
    ASSERT_EQ(physical_count, 0U);
    ASSERT_EQ(static_cast<void*>(physical[0]), static_cast<void*>(temp_ptr.get()));
    ASSERT_EQ(static_cast<void*>(physical[1]), static_cast<void*>(temp_ptr.get()));

    // Remove one from the physical device count so we can get the VK_INCOMPLETE message
    physical_count = 1;
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &physical_count, physical.data()));
    ASSERT_EQ(physical_count, 1U);
    ASSERT_EQ(static_cast<void*>(physical[1]), static_cast<void*>(temp_ptr.get()));

    physical_count = 2;
    std::array<VkPhysicalDevice, real_device_count> physical_2;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &physical_count, physical_2.data()));

    // Verify that the first physical device shows up in the list of the second ones
    ASSERT_TRUE(std::find(physical_2.begin(), physical_2.end(), physical[0]) != physical_2.end());
}

TEST(EnumeratePhysicalDevices, ZeroPhysicalDevices) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));
    inst.CheckCreate();

    uint32_t count = 0;
    ASSERT_EQ(VK_ERROR_INITIALIZATION_FAILED, env.vulkan_functions.vkEnumeratePhysicalDevices(inst, &count, nullptr));
    ASSERT_EQ(count, 0U);
}

TEST(EnumeratePhysicalDevices, ZeroPhysicalDevicesAfterCreateInstance) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1)).set_min_icd_interface_version(5);
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();
    driver.physical_devices.clear();

    uint32_t physical_device_count = 1000;  // not zero starting value
    VkPhysicalDevice physical_device{};

    EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED, inst->vkEnumeratePhysicalDevices(inst, &physical_device_count, nullptr));
    EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED, inst->vkEnumeratePhysicalDevices(inst, &physical_device_count, &physical_device));

    uint32_t physical_device_group_count = 1000;  // not zero starting value
    VkPhysicalDeviceGroupProperties physical_device_group_properties{};

    EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED, inst->vkEnumeratePhysicalDeviceGroups(inst, &physical_device_group_count, nullptr));
    EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
              inst->vkEnumeratePhysicalDeviceGroups(inst, &physical_device_group_count, &physical_device_group_properties));
}

TEST(EnumeratePhysicalDevices, CallTwiceNormal) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    for (size_t i = 0; i < 4; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Call twice in a row and make sure nothing bad happened
    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    std::vector<VkPhysicalDevice> physical_device_handles_1 = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_1.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    std::vector<VkPhysicalDevice> physical_device_handles_2 = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_2.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    // Make sure devices are same between the two
    for (uint32_t count = 0; count < driver.physical_devices.size(); ++count) {
        ASSERT_EQ(physical_device_handles_1[count], physical_device_handles_2[count]);
    }
}

TEST(EnumeratePhysicalDevices, CallTwiceIncompleteOnceNormal) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    for (size_t i = 0; i < 8; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Query 3, then 5, then all
    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = 3;
    std::vector<VkPhysicalDevice> physical_device_handles_1 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_1.data()));
    ASSERT_EQ(3U, returned_physical_count);
    returned_physical_count = 5;
    std::vector<VkPhysicalDevice> physical_device_handles_2 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_2.data()));
    ASSERT_EQ(5U, returned_physical_count);
    returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_3 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_3.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    // Make sure devices are same between the three
    for (uint32_t count = 0; count < driver.physical_devices.size(); ++count) {
        if (count < physical_device_handles_1.size()) {
            ASSERT_EQ(physical_device_handles_1[count], physical_device_handles_3[count]);
        }
        if (count < physical_device_handles_2.size()) {
            ASSERT_EQ(physical_device_handles_2[count], physical_device_handles_3[count]);
        }
    }
}

TEST(EnumeratePhysicalDevices, CallThriceSuccessReduce) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    for (size_t i = 0; i < 8; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Query all at first, then 5, then 3
    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_1 = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_1.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    returned_physical_count = 5;
    std::vector<VkPhysicalDevice> physical_device_handles_2 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_2.data()));
    ASSERT_EQ(5U, returned_physical_count);
    returned_physical_count = 3;
    std::vector<VkPhysicalDevice> physical_device_handles_3 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_3.data()));
    ASSERT_EQ(3U, returned_physical_count);
    // Make sure devices are same between the three
    for (uint32_t count = 0; count < driver.physical_devices.size(); ++count) {
        if (count < physical_device_handles_2.size()) {
            ASSERT_EQ(physical_device_handles_2[count], physical_device_handles_1[count]);
        }
        if (count < physical_device_handles_3.size()) {
            ASSERT_EQ(physical_device_handles_3[count], physical_device_handles_1[count]);
        }
    }
}

TEST(EnumeratePhysicalDevices, CallThriceAddInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    driver.physical_devices.emplace_back("physical_device_0");
    driver.physical_devices.emplace_back("physical_device_1");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_1 = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_1.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    driver.physical_devices.emplace_back("physical_device_2");
    driver.physical_devices.emplace_back("physical_device_3");

    std::vector<VkPhysicalDevice> physical_device_handles_2 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_2.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_3 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_3.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    // Make sure devices are same between the three
    for (uint32_t count = 0; count < physical_device_handles_3.size(); ++count) {
        if (count < physical_device_handles_1.size()) {
            ASSERT_EQ(physical_device_handles_1[count], physical_device_handles_3[count]);
        }
        if (count < physical_device_handles_2.size()) {
            ASSERT_EQ(physical_device_handles_2[count], physical_device_handles_3[count]);
        }
    }
}

TEST(EnumeratePhysicalDevices, CallThriceRemoveInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    for (size_t i = 0; i < 4; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_1 = std::vector<VkPhysicalDevice>(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_1.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    // Delete the 2nd physical device
    driver.physical_devices.erase(std::next(driver.physical_devices.begin()));

    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    std::vector<VkPhysicalDevice> physical_device_handles_2 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_2.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    physical_device_handles_2.resize(returned_physical_count);

    returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles_3 = std::vector<VkPhysicalDevice>(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_device_handles_3.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    // Make sure one has 1 more device that two or three
    ASSERT_EQ(physical_device_handles_1.size(), physical_device_handles_2.size() + 1);
    ASSERT_EQ(physical_device_handles_1.size(), physical_device_handles_3.size() + 1);

    // Make sure the devices in two and three are all found in one
    uint32_t two_found = 0;
    uint32_t three_found = 0;
    for (uint32_t count = 0; count < physical_device_handles_1.size(); ++count) {
        for (uint32_t int_count = 0; int_count < physical_device_handles_2.size(); ++int_count) {
            if (physical_device_handles_2[int_count] == physical_device_handles_1[count]) {
                two_found++;
                break;
            }
        }
        for (uint32_t int_count = 0; int_count < physical_device_handles_3.size(); ++int_count) {
            if (physical_device_handles_3[int_count] == physical_device_handles_1[count]) {
                three_found++;
                break;
            }
        }
    }
    ASSERT_EQ(two_found, returned_physical_count);
    ASSERT_EQ(three_found, returned_physical_count);
}

TEST(EnumeratePhysicalDevices, MultipleAddRemoves) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).set_min_icd_interface_version(5);

    for (size_t i = 0; i < 4; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
    }
    std::array<std::vector<VkPhysicalDevice>, 8> physical_dev_handles;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = physical_count;
    physical_dev_handles[0].resize(physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[0].data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    // Delete the 2nd physical device (0, 2, 3)
    driver.physical_devices.erase(std::next(driver.physical_devices.begin()));

    // Query using old number from last call (4), but it should only return 3
    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    physical_dev_handles[1].resize(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[1].data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    physical_dev_handles[1].resize(returned_physical_count);

    // Add two new physical devices to the front (A, B, 0, 2, 3)
    driver.physical_devices.emplace(driver.physical_devices.begin(), "physical_device_B");
    driver.physical_devices.emplace(driver.physical_devices.begin(), "physical_device_A");

    // Query using old number from last call (3), but it should be 5
    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    physical_dev_handles[2].resize(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[2].data()));
    ASSERT_EQ(physical_count - 2, returned_physical_count);
    physical_dev_handles[2].resize(returned_physical_count);

    // Query again to get all 5
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, nullptr));
    physical_dev_handles[3].resize(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[3].data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    // Delete last two physical devices (A, B, 0, 2)
    driver.physical_devices.pop_back();

    // Query using old number from last call (5), but it should be 4
    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    physical_dev_handles[4].resize(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[4].data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    physical_dev_handles[4].resize(returned_physical_count);
    // Adjust size and query again, should be the same
    physical_dev_handles[5].resize(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[5].data()));

    // Insert a new physical device (A, B, C, 0, 2)
    driver.physical_devices.insert(driver.physical_devices.begin() + 2, "physical_device_C");

    // Query using old number from last call (4), but it should be 5
    physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    physical_dev_handles[6].resize(returned_physical_count);
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[6].data()));
    ASSERT_EQ(physical_count - 1, returned_physical_count);
    // Query again to get all 5
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, nullptr));
    physical_dev_handles[7].resize(returned_physical_count);
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_physical_count, physical_dev_handles[7].data()));

    // Check final results
    // One   [4] - 0, 1, 2, 3
    // Two   [3] - 0, 2, 3
    // Three [3] - A, B, 0
    // Four  [5] - A, B, 0, 2, 3
    // Five  [4] - A, B, 0, 2
    // Six   [4] - A, B, 0, 2
    // Seven [4] - A, B, C, 0
    // Eight [5] - A, B, C, 0, 2
    ASSERT_EQ(4U, physical_dev_handles[0].size());
    ASSERT_EQ(3U, physical_dev_handles[1].size());
    ASSERT_EQ(3U, physical_dev_handles[2].size());
    ASSERT_EQ(5U, physical_dev_handles[3].size());
    ASSERT_EQ(4U, physical_dev_handles[4].size());
    ASSERT_EQ(4U, physical_dev_handles[5].size());
    ASSERT_EQ(4U, physical_dev_handles[6].size());
    ASSERT_EQ(5U, physical_dev_handles[7].size());

    // Make sure the devices in two and three are all found in one
    uint32_t found_items[8]{};
    for (uint32_t handle = 1; handle < 8; ++handle) {
        for (uint32_t count = 0; count < physical_dev_handles[0].size(); ++count) {
            for (uint32_t int_count = 0; int_count < physical_dev_handles[handle].size(); ++int_count) {
                if (physical_dev_handles[handle][int_count] == physical_dev_handles[0][count]) {
                    found_items[handle]++;
                    break;
                }
            }
        }
    }
    // Items matching from first call (must be >= since handle re-use does occur)
    ASSERT_EQ(found_items[1], 3U);
    ASSERT_GE(found_items[2], 1U);
    ASSERT_GE(found_items[3], 3U);
    ASSERT_GE(found_items[4], 2U);
    ASSERT_GE(found_items[5], 2U);
    ASSERT_GE(found_items[6], 1U);
    ASSERT_GE(found_items[7], 2U);

    memset(found_items, 0, 8 * sizeof(uint32_t));
    for (uint32_t handle = 0; handle < 7; ++handle) {
        for (uint32_t count = 0; count < physical_dev_handles[7].size(); ++count) {
            for (uint32_t int_count = 0; int_count < physical_dev_handles[handle].size(); ++int_count) {
                if (physical_dev_handles[handle][int_count] == physical_dev_handles[7][count]) {
                    found_items[handle]++;
                    break;
                }
            }
        }
    }
    // Items matching from last call (must be >= since handle re-use does occur)
    ASSERT_GE(found_items[0], 2U);
    ASSERT_GE(found_items[1], 2U);
    ASSERT_GE(found_items[2], 3U);
    ASSERT_GE(found_items[3], 4U);
    ASSERT_GE(found_items[4], 4U);
    ASSERT_GE(found_items[5], 4U);
    ASSERT_GE(found_items[6], 4U);
}

TEST(EnumeratePhysicalDevices, OneDriverWithWrongErrorCodes) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    {
        env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_INITIALIZATION_FAILED);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);
    }
    {
        env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_INCOMPATIBLE_DRIVER);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);
    }
    {
        env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_SURFACE_LOST_KHR);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);
    }
}

TEST(EnumeratePhysicalDevices, TwoDriversOneWithWrongErrorCodes) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    TestICD& icd1 = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    {
        icd1.set_enum_physical_devices_return_code(VK_ERROR_INITIALIZATION_FAILED);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }
    {
        icd1.set_enum_physical_devices_return_code(VK_ERROR_INCOMPATIBLE_DRIVER);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }
    {
        icd1.set_enum_physical_devices_return_code(VK_ERROR_SURFACE_LOST_KHR);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }
}

TEST(CreateDevice, ExtensionNotPresent) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_extension("NotPresent");

    dev.CheckCreate(phys_dev, VK_ERROR_EXTENSION_NOT_PRESENT);
}

// LX535 / MI-76: Device layers are deprecated.
// Ensure that no errors occur if a bogus device layer list is passed to vkCreateDevice.
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
TEST(CreateDevice, LayersNotPresent) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_layer("NotPresent");

    dev.CheckCreate(phys_dev);
}

// Device layers are deprecated.
// Ensure that no error occur if instance and device are created with the same list of layers.
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
TEST(CreateDevice, MatchInstanceAndDeviceLayers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();

    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_layer(layer_name);

    dev.CheckCreate(phys_dev);
}

// Device layers are deprecated.
// Ensure that a message is generated when instance and device are created with different list of layers.
// At best , the user can list only instance layers in the device layer list
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
TEST(CreateDevice, UnmatchInstanceAndDeviceLayers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.CheckCreate();

    DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_layer(layer_name);

    dev.CheckCreate(phys_dev);

    ASSERT_TRUE(
        log.find("loader_create_device_chain: Using deprecated and ignored 'ppEnabledLayerNames' member of 'VkDeviceCreateInfo' "
                 "when creating a Vulkan device."));
}

// Device layers are deprecated.
// Ensure that when VkInstanceCreateInfo is deleted, the check of the instance layer lists is running correctly during VkDevice
// creation
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
TEST(CreateDevice, CheckCopyOfInstanceLayerNames) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    {
        // We intentionally create a local InstanceCreateInfo that goes out of scope at the } so that when dev.CheckCreate is called
        // the layer name pointers are no longer valid
        InstanceCreateInfo create_info{};
        create_info.add_layer(layer_name);
        inst.CheckCreateWithInfo(create_info);
    }

    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_layer(layer_name);

    dev.CheckCreate(phys_dev);
}

TEST(CreateDevice, ConsecutiveCreate) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    for (uint32_t i = 0; i < 100; i++) {
        driver.physical_devices.emplace_back("physical_device_0");
    }
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto phys_devs = inst.GetPhysDevs(100);
    for (uint32_t i = 0; i < 100; i++) {
        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_devs[i]);
    }
}

TEST(CreateDevice, ConsecutiveCreateWithoutDestruction) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    for (uint32_t i = 0; i < 100; i++) {
        driver.physical_devices.emplace_back("physical_device_0");
    }
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto phys_devs = inst.GetPhysDevs(100);

    std::vector<DeviceWrapper> devices;
    for (uint32_t i = 0; i < 100; i++) {
        devices.emplace_back(inst);
        DeviceWrapper& dev = devices.back();

        dev.CheckCreate(phys_devs[i]);
    }
}

TEST(TryLoadWrongBinaries, WrongICD) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");
    env.add_icd(TestICDDetails(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE).set_is_fake(true));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();

#if _WIN32 || _WIN64
    ASSERT_TRUE(log.find("Failed to open dynamic library"));
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__) || defined(__QNX__)
#if defined(__x86_64__) || __ppc64__ || __aarch64__
    ASSERT_TRUE(log.find("wrong ELF class: ELFCLASS32"));
#else
    ASSERT_TRUE(log.find("wrong ELF class: ELFCLASS64"));
#endif
#endif

    uint32_t driver_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, nullptr));
    ASSERT_EQ(driver_count, 1U);
}

TEST(TryLoadWrongBinaries, WrongExplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "DummyLayerExplicit";
    env.add_fake_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)),
        "dummy_test_layer.json");

    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // Explicit layer not found should generate a VK_ERROR_LAYER_NOT_PRESENT error message.
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);

    // Should get an error message for the explicit layer
#if !defined(__APPLE__)
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" was wrong bit-type!")));
#else   // __APPLE__
    // Apple only throws a wrong library type of error
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" failed to load!")));
#endif  // __APPLE__
}

TEST(TryLoadWrongBinaries, WrongImplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "DummyLayerImplicit0";
    env.add_fake_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                              .set_name(layer_name)
                                                              .set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)
                                                              .set_disable_environment("DISABLE_ENV")),
                                "dummy_test_layer.json");

    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // We don't want to return VK_ERROR_LAYER_NOT_PRESENT for missing implicit layers because it's not the
    // application asking for them.
    inst.CheckCreate();

#if !defined(__APPLE__)
    // Should get an info message for the bad implicit layer
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" was wrong bit-type.")));
#else   // __APPLE__
    // Apple only throws a wrong library type of error
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" failed to load.")));
#endif  // __APPLE__
}

TEST(TryLoadWrongBinaries, WrongExplicitAndImplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name_0 = "DummyLayerExplicit";
    env.add_fake_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name_0).set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)),
        "dummy_test_layer_0.json");
    const char* layer_name_1 = "DummyLayerImplicit";
    env.add_fake_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                              .set_name(layer_name_1)
                                                              .set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)
                                                              .set_disable_environment("DISABLE_ENV")),
                                "dummy_test_layer_1.json");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(check_permutation({layer_name_0, layer_name_1}, layer_props));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name_0);
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // Explicit layer not found should generate a VK_ERROR_LAYER_NOT_PRESENT error message.
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);

#if !defined(__APPLE__)
    // Should get error messages for both (the explicit is second and we don't want the implicit to return before the explicit
    // triggers a failure during vkCreateInstance)
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" was wrong bit-type!")));
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" was wrong bit-type.")));
#else   // __APPLE__
    // Apple only throws a wrong library type of error
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" failed to load!")));
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" failed to load.")));
#endif  // __APPLE__
}

TEST(TryLoadWrongBinaries, WrongExplicitAndImplicitErrorOnly) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name_0 = "DummyLayerExplicit";
    env.add_fake_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name_0).set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)),
        "dummy_test_layer_0.json");
    const char* layer_name_1 = "DummyLayerImplicit";
    env.add_fake_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                              .set_name(layer_name_1)
                                                              .set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE)
                                                              .set_disable_environment("DISABLE_ENV")),
                                "dummy_test_layer_1.json");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(check_permutation({layer_name_0, layer_name_1}, layer_props));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name_0);
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // Explicit layer not found should generate a VK_ERROR_LAYER_NOT_PRESENT error message.
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);

#if !defined(__APPLE__)
    // Should not get an error messages for either
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" was wrong bit-type!")));
    ASSERT_FALSE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" was wrong bit-type.")));
#else   // __APPLE__
    // Apple only throws a wrong library type of error
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" failed to load!")));
    ASSERT_FALSE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" failed to load.")));
#endif  // __APPLE__
}

TEST(TryLoadWrongBinaries, BadExplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "DummyLayerExplicit";
    env.add_fake_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_BAD)),
        "dummy_test_layer.json");

    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // Explicit layer not found should generate a VK_ERROR_LAYER_NOT_PRESENT error message.
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);

// Should get an error message for the bad explicit
#if defined(WIN32)
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" was wrong bit-type!")));
#else
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" failed to load!")));
#endif  // defined(WIN32)
}

TEST(TryLoadWrongBinaries, BadImplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name = "DummyLayerImplicit0";
    env.add_fake_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                              .set_name(layer_name)
                                                              .set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_BAD)
                                                              .set_disable_environment("DISABLE_ENV")),
                                "dummy_test_layer.json");

    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // We don't want to return VK_ERROR_LAYER_NOT_PRESENT for missing implicit layers because it's not the
    // application asking for them.
    inst.CheckCreate();

    // Should get an info message for the bad implicit
#if defined(WIN32)
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" was wrong bit-type.")));
#else
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name) + std::string("\" failed to load.")));
#endif  // defined(WIN32)
}

TEST(TryLoadWrongBinaries, BadExplicitAndImplicit) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    const char* layer_name_0 = "DummyLayerExplicit";
    env.add_fake_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name_0).set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_BAD)),
        "dummy_test_layer_0.json");
    const char* layer_name_1 = "DummyLayerImplicit0";
    env.add_fake_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                              .set_name(layer_name_1)
                                                              .set_lib_path(CURRENT_PLATFORM_DUMMY_BINARY_BAD)
                                                              .set_disable_environment("DISABLE_ENV")),
                                "dummy_test_layer_1.json");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(check_permutation({layer_name_0, layer_name_1}, layer_props));

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name_0);
    FillDebugUtilsCreateDetails(inst.create_info, log);

    // Explicit layer not found should generate a VK_ERROR_LAYER_NOT_PRESENT error message.
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
#if defined(WIN32)
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" was wrong bit-type!")));
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" was wrong bit-type.")));
#else
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_0) + std::string("\" failed to load!")));
    ASSERT_TRUE(log.find(std::string("Requested layer \"") + std::string(layer_name_1) + std::string("\" failed to load.")));
#endif  // defined(WIN32)
}

TEST(TryLoadWrongBinaries, WrongArchDriver) {
    FrameworkEnvironment env{};
    // Intentionally set the wrong arch
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}.icd_manifest.set_library_arch(sizeof(void*) == 4 ? "64" : "32"))
        .add_physical_device("physical_device_0");

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
    ASSERT_TRUE(
        log.find("loader_parse_icd_manifest: Driver library architecture doesn't match the current running architecture, skipping "
                 "this driver"));
}

TEST(TryLoadWrongBinaries, WrongArchLayer) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}).add_physical_device("physical_device_0");

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         // Intentionally set the wrong arch
                                                         .set_library_arch(sizeof(void*) == 4 ? "64" : "32")),
                           "test_layer.json");

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_TRUE(log.find(std::string("The library architecture in layer ") + env.get_shimmed_layer_manifest_path(0).string() +
                         " doesn't match the current running architecture, skipping this layer"));
}

TEST(EnumeratePhysicalDeviceGroups, OneCall) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    // ICD contains 3 devices in two groups
    for (size_t i = 0; i < 3; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[1]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[2]);
    const uint32_t max_physical_device_count = 3;

    // Core function
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        auto physical_devices = std::vector<VkPhysicalDevice>(max_physical_device_count);
        uint32_t returned_phys_dev_count = max_physical_device_count;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
        handle_assert_has_values(physical_devices);

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = group_count;
        std::vector<VkPhysicalDeviceGroupProperties> group_props{};
        group_props.resize(group_count, VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(group_count, returned_group_count);

        // Make sure each physical device shows up in a group, but only once
        std::array<bool, max_physical_device_count> found{false};
        for (uint32_t group = 0; group < group_count; ++group) {
            for (uint32_t g_dev = 0; g_dev < group_props[group].physicalDeviceCount; ++g_dev) {
                for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
                    if (physical_devices[dev] == group_props[group].physicalDevices[g_dev]) {
                        ASSERT_EQ(false, found[dev]);
                        found[dev] = true;
                        break;
                    }
                }
            }
        }
        for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
            ASSERT_EQ(true, found[dev]);
        }
        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            VkBaseInStructure spacer_structure{};
            spacer_structure.sType = static_cast<VkStructureType>(100000);
            spacer_structure.pNext = reinterpret_cast<const VkBaseInStructure*>(&group_info);
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &spacer_structure;
            dev.CheckCreate(group.physicalDevices[0]);

            // This convoluted logic makes sure that the pNext chain is unmolested after being passed into vkCreateDevice
            // While not expected for applications to iterate over this chain, since it is const it is important to make sure
            // that the chain didn't change somehow, and especially so that iterating it doesn't crash.
            int count = 0;
            const VkBaseInStructure* pNext = reinterpret_cast<const VkBaseInStructure*>(dev.create_info.dev.pNext);
            while (pNext != nullptr) {
                if (pNext->sType == VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO) {
                    ASSERT_EQ(&group_info, reinterpret_cast<const VkDeviceGroupDeviceCreateInfo*>(pNext));
                }
                if (pNext->sType == 100000) {
                    ASSERT_EQ(&spacer_structure, pNext);
                }
                pNext = pNext->pNext;
                count++;
            }
            ASSERT_EQ(count, 2);
        }
    }
    driver.add_instance_extension({VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME});
    // Extension
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
        inst.CheckCreate();

        PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR = inst.load("vkEnumeratePhysicalDeviceGroupsKHR");

        auto physical_devices = std::vector<VkPhysicalDevice>(max_physical_device_count);
        uint32_t returned_phys_dev_count = max_physical_device_count;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
        handle_assert_has_values(physical_devices);

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = group_count;
        std::vector<VkPhysicalDeviceGroupProperties> group_props{};
        group_props.resize(group_count, VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
        ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(group_count, returned_group_count);

        // Make sure each physical device shows up in a group, but only once
        std::array<bool, max_physical_device_count> found{false};
        for (uint32_t group = 0; group < group_count; ++group) {
            for (uint32_t g_dev = 0; g_dev < group_props[group].physicalDeviceCount; ++g_dev) {
                for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
                    if (physical_devices[dev] == group_props[group].physicalDevices[g_dev]) {
                        ASSERT_EQ(false, found[dev]);
                        found[dev] = true;
                        break;
                    }
                }
            }
        }
        for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
            ASSERT_EQ(true, found[dev]);
        }
        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            VkBaseInStructure spacer_structure{};
            spacer_structure.sType = static_cast<VkStructureType>(100000);
            spacer_structure.pNext = reinterpret_cast<const VkBaseInStructure*>(&group_info);
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &spacer_structure;
            dev.CheckCreate(group.physicalDevices[0]);
        }
    }
}

TEST(EnumeratePhysicalDeviceGroups, TwoCall) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    // ICD contains 3 devices in two groups
    for (size_t i = 0; i < 3; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[1]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[2]);
    const uint32_t max_physical_device_count = 3;

    // Core function
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        auto physical_devices = std::vector<VkPhysicalDevice>(max_physical_device_count);
        uint32_t returned_phys_dev_count = max_physical_device_count;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
        handle_assert_has_values(physical_devices);

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, nullptr));
        ASSERT_EQ(group_count, returned_group_count);

        std::vector<VkPhysicalDeviceGroupProperties> group_props{};
        group_props.resize(group_count, VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(group_count, returned_group_count);

        // Make sure each physical device shows up in a group, but only once
        std::array<bool, max_physical_device_count> found{false};
        for (uint32_t group = 0; group < group_count; ++group) {
            for (uint32_t g_dev = 0; g_dev < group_props[group].physicalDeviceCount; ++g_dev) {
                for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
                    if (physical_devices[dev] == group_props[group].physicalDevices[g_dev]) {
                        ASSERT_EQ(false, found[dev]);
                        found[dev] = true;
                        break;
                    }
                }
            }
        }
        for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
            ASSERT_EQ(true, found[dev]);
        }
        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &group_info;
            dev.CheckCreate(group.physicalDevices[0]);
        }
    }
    driver.add_instance_extension({VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME});
    // Extension
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension("VK_KHR_device_group_creation");
        inst.CheckCreate();

        auto physical_devices = std::vector<VkPhysicalDevice>(max_physical_device_count);
        uint32_t returned_phys_dev_count = max_physical_device_count;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
        handle_assert_has_values(physical_devices);

        PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR = inst.load("vkEnumeratePhysicalDeviceGroupsKHR");

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = 0;
        ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, nullptr));
        ASSERT_EQ(group_count, returned_group_count);

        std::vector<VkPhysicalDeviceGroupProperties> group_props{};
        group_props.resize(group_count, VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
        ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(group_count, returned_group_count);

        // Make sure each physical device shows up in a group, but only once
        std::array<bool, max_physical_device_count> found{false};
        for (uint32_t group = 0; group < group_count; ++group) {
            for (uint32_t g_dev = 0; g_dev < group_props[group].physicalDeviceCount; ++g_dev) {
                for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
                    if (physical_devices[dev] == group_props[group].physicalDevices[g_dev]) {
                        ASSERT_EQ(false, found[dev]);
                        found[dev] = true;
                        break;
                    }
                }
            }
        }
        for (uint32_t dev = 0; dev < max_physical_device_count; ++dev) {
            ASSERT_EQ(true, found[dev]);
        }
        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &group_info;
            dev.CheckCreate(group.physicalDevices[0]);
        }
    }
}

TEST(EnumeratePhysicalDeviceGroups, TwoCallIncomplete) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});

    // ICD contains 3 devices in two groups
    for (size_t i = 0; i < 3; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[1]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[2]);

    // Core function
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, nullptr));
        ASSERT_EQ(group_count, returned_group_count);

        returned_group_count = 1;
        std::array<VkPhysicalDeviceGroupProperties, 1> group_props{};
        group_props[0].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(1U, returned_group_count);

        returned_group_count = 2;
        std::array<VkPhysicalDeviceGroupProperties, 2> group_props_2{};
        group_props_2[0].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        group_props_2[1].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_2.data()));
        ASSERT_EQ(2U, returned_group_count);

        ASSERT_EQ(group_props[0].physicalDeviceCount, group_props_2[0].physicalDeviceCount);
        ASSERT_EQ(group_props[0].physicalDevices[0], group_props_2[0].physicalDevices[0]);
        ASSERT_EQ(group_props[0].physicalDevices[1], group_props_2[0].physicalDevices[1]);

        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &group_info;
            dev.CheckCreate(group.physicalDevices[0]);
        }
    }
    driver.add_instance_extension({VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME});
    // Extension
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension("VK_KHR_device_group_creation");
        inst.CheckCreate();

        PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR = inst.load("vkEnumeratePhysicalDeviceGroupsKHR");

        uint32_t group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
        uint32_t returned_group_count = 0;
        ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, nullptr));
        ASSERT_EQ(group_count, returned_group_count);

        returned_group_count = 1;
        std::array<VkPhysicalDeviceGroupProperties, 1> group_props{};
        group_props[0].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        ASSERT_EQ(VK_INCOMPLETE, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, group_props.data()));
        ASSERT_EQ(1U, returned_group_count);

        returned_group_count = 2;
        std::array<VkPhysicalDeviceGroupProperties, 2> group_props_2{};
        group_props_2[0].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        group_props_2[1].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, group_props_2.data()));
        ASSERT_EQ(2U, returned_group_count);

        ASSERT_EQ(group_props[0].physicalDeviceCount, group_props_2[0].physicalDeviceCount);
        ASSERT_EQ(group_props[0].physicalDevices[0], group_props_2[0].physicalDevices[0]);
        ASSERT_EQ(group_props[0].physicalDevices[1], group_props_2[0].physicalDevices[1]);

        for (auto& group : group_props) {
            VkDeviceGroupDeviceCreateInfo group_info{};
            group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
            group_info.physicalDeviceCount = group.physicalDeviceCount;
            group_info.pPhysicalDevices = &group.physicalDevices[0];
            DeviceWrapper dev{inst};
            dev.create_info.dev.pNext = &group_info;
            dev.CheckCreate(group.physicalDevices[0]);
        }
    }
}

// Call the core vkEnumeratePhysicalDeviceGroups and the extension
// vkEnumeratePhysicalDeviceGroupsKHR, and make sure they return the same info.
TEST(EnumeratePhysicalDeviceGroups, TestCoreVersusExtensionSameReturns) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1)
                       .add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME})
                       .add_instance_extension({VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME});

    // Generate the devices
    for (size_t i = 0; i < 6; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[5]);

    uint32_t expected_counts[3] = {1, 3, 2};
    uint32_t core_group_count = 0;
    std::vector<VkPhysicalDeviceGroupProperties> core_group_props{};
    uint32_t ext_group_count = 0;
    std::vector<VkPhysicalDeviceGroupProperties> ext_group_props{};

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.create_info.add_extension("VK_KHR_device_group_creation");
    inst.CheckCreate();

    // Core function
    core_group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
    uint32_t returned_group_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, nullptr));
    ASSERT_EQ(core_group_count, returned_group_count);

    core_group_props.resize(returned_group_count,
                            VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, core_group_props.data()));
    ASSERT_EQ(core_group_count, returned_group_count);

    PFN_vkEnumeratePhysicalDeviceGroupsKHR vkEnumeratePhysicalDeviceGroupsKHR = inst.load("vkEnumeratePhysicalDeviceGroupsKHR");

    ext_group_count = static_cast<uint32_t>(driver.physical_device_groups.size());
    returned_group_count = 0;
    ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, nullptr));
    ASSERT_EQ(ext_group_count, returned_group_count);

    ext_group_props.resize(returned_group_count,
                           VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, vkEnumeratePhysicalDeviceGroupsKHR(inst, &returned_group_count, ext_group_props.data()));
    ASSERT_EQ(ext_group_count, returned_group_count);

    // Make sure data from each matches
    ASSERT_EQ(core_group_count, 3U);
    ASSERT_EQ(ext_group_count, 3U);
    for (uint32_t group = 0; group < core_group_count; ++group) {
        ASSERT_EQ(core_group_props[group].physicalDeviceCount, expected_counts[group]);
        ASSERT_EQ(ext_group_props[group].physicalDeviceCount, expected_counts[group]);
        for (uint32_t dev = 0; dev < core_group_props[group].physicalDeviceCount; ++dev) {
            ASSERT_EQ(core_group_props[group].physicalDevices[dev], ext_group_props[group].physicalDevices[dev]);
        }
    }
    // Make sure no physical device appears in more than one group
    for (uint32_t group1 = 0; group1 < core_group_count; ++group1) {
        for (uint32_t group2 = group1 + 1; group2 < core_group_count; ++group2) {
            for (uint32_t dev1 = 0; dev1 < core_group_props[group1].physicalDeviceCount; ++dev1) {
                for (uint32_t dev2 = 0; dev2 < core_group_props[group1].physicalDeviceCount; ++dev2) {
                    ASSERT_NE(core_group_props[group1].physicalDevices[dev1], core_group_props[group2].physicalDevices[dev2]);
                }
            }
        }
    }
    for (auto& group : core_group_props) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Start with 6 devices in 3 different groups, and then add a group,
// querying vkEnumeratePhysicalDeviceGroups before and after the add.
TEST(EnumeratePhysicalDeviceGroups, CallThriceAddGroupInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1);

    // Generate the devices
    for (size_t i = 0; i < 7; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[5]);

    uint32_t before_expected_counts[3] = {1, 3, 2};
    uint32_t after_expected_counts[4] = {1, 3, 1, 2};
    uint32_t before_group_count = 3;
    uint32_t after_group_count = 4;
    uint32_t returned_group_count = 0;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    std::vector<VkPhysicalDeviceGroupProperties> group_props_before{};
    group_props_before.resize(before_group_count,
                              VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = before_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_before.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_before[group].physicalDeviceCount, before_expected_counts[group]);
    }

    // Insert new group after first two
    driver.physical_device_groups.insert(driver.physical_device_groups.begin() + 2, driver.physical_devices[6]);

    std::vector<VkPhysicalDeviceGroupProperties> group_props_after{};
    group_props_after.resize(before_group_count,
                             VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_after[group].physicalDeviceCount, after_expected_counts[group]);
    }

    group_props_after.resize(after_group_count,
                             VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = after_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after.data()));
    ASSERT_EQ(after_group_count, returned_group_count);
    for (uint32_t group = 0; group < after_group_count; ++group) {
        ASSERT_EQ(group_props_after[group].physicalDeviceCount, after_expected_counts[group]);
    }

    // Make sure all devices in the old group info are still found in the new group info
    for (uint32_t group1 = 0; group1 < group_props_before.size(); ++group1) {
        for (uint32_t group2 = 0; group2 < group_props_after.size(); ++group2) {
            if (group_props_before[group1].physicalDeviceCount == group_props_after[group2].physicalDeviceCount) {
                uint32_t found_count = 0;
                bool found = false;
                for (uint32_t dev1 = 0; dev1 < group_props_before[group1].physicalDeviceCount; ++dev1) {
                    found = false;
                    for (uint32_t dev2 = 0; dev2 < group_props_after[group2].physicalDeviceCount; ++dev2) {
                        if (group_props_before[group1].physicalDevices[dev1] == group_props_after[group2].physicalDevices[dev2]) {
                            found_count++;
                            found = true;
                            break;
                        }
                    }
                }
                ASSERT_EQ(found, found_count == group_props_before[group1].physicalDeviceCount);
            }
        }
    }
    for (auto& group : group_props_after) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Start with 7 devices in 4 different groups, and then remove a group,
// querying vkEnumeratePhysicalDeviceGroups before and after the remove.
TEST(EnumeratePhysicalDeviceGroups, CallTwiceRemoveGroupInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1);

    // Generate the devices
    for (size_t i = 0; i < 7; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[5]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[6]);

    uint32_t before_expected_counts[4] = {1, 3, 1, 2};
    uint32_t after_expected_counts[3] = {1, 3, 2};
    uint32_t before_group_count = 4;
    uint32_t after_group_count = 3;
    uint32_t returned_group_count = 0;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    std::vector<VkPhysicalDeviceGroupProperties> group_props_before{};
    group_props_before.resize(before_group_count,
                              VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = before_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_before.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_before[group].physicalDeviceCount, before_expected_counts[group]);
    }

    // Insert new group after first two
    driver.physical_device_groups.erase(driver.physical_device_groups.begin() + 2);

    std::vector<VkPhysicalDeviceGroupProperties> group_props_after{};
    group_props_after.resize(after_group_count,
                             VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after.data()));
    ASSERT_EQ(after_group_count, returned_group_count);
    for (uint32_t group = 0; group < after_group_count; ++group) {
        ASSERT_EQ(group_props_after[group].physicalDeviceCount, after_expected_counts[group]);
    }

    // Make sure all devices in the new group info are found in the old group info
    for (uint32_t group1 = 0; group1 < group_props_after.size(); ++group1) {
        for (uint32_t group2 = 0; group2 < group_props_before.size(); ++group2) {
            if (group_props_after[group1].physicalDeviceCount == group_props_before[group2].physicalDeviceCount) {
                uint32_t found_count = 0;
                bool found = false;
                for (uint32_t dev1 = 0; dev1 < group_props_after[group1].physicalDeviceCount; ++dev1) {
                    found = false;
                    for (uint32_t dev2 = 0; dev2 < group_props_before[group2].physicalDeviceCount; ++dev2) {
                        if (group_props_after[group1].physicalDevices[dev1] == group_props_before[group2].physicalDevices[dev2]) {
                            found_count++;
                            found = true;
                            break;
                        }
                    }
                }
                ASSERT_EQ(found, found_count == group_props_after[group1].physicalDeviceCount);
            }
        }
    }
    for (auto& group : group_props_after) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Start with 6 devices in 3 different groups, and then add a device to the middle group,
// querying vkEnumeratePhysicalDeviceGroups before and after the add.
TEST(EnumeratePhysicalDeviceGroups, CallTwiceAddDeviceInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1);

    // Generate the devices
    for (size_t i = 0; i < 7; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[5]);

    uint32_t expected_group_count = 3;
    uint32_t before_expected_counts[3] = {1, 3, 2};
    uint32_t after_expected_counts[3] = {1, 4, 2};
    uint32_t returned_group_count = 0;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    std::vector<VkPhysicalDeviceGroupProperties> group_props_before{};
    group_props_before.resize(expected_group_count,
                              VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = expected_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_before.data()));
    ASSERT_EQ(expected_group_count, returned_group_count);
    for (uint32_t group = 0; group < expected_group_count; ++group) {
        ASSERT_EQ(group_props_before[group].physicalDeviceCount, before_expected_counts[group]);
    }

    // Insert new device to 2nd group
    driver.physical_device_groups[1].use_physical_device(driver.physical_devices[6]);

    std::vector<VkPhysicalDeviceGroupProperties> group_props_after{};
    group_props_after.resize(expected_group_count,
                             VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after.data()));
    ASSERT_EQ(expected_group_count, returned_group_count);
    for (uint32_t group = 0; group < expected_group_count; ++group) {
        ASSERT_EQ(group_props_after[group].physicalDeviceCount, after_expected_counts[group]);
    }

    // Make sure all devices in the old group info are still found in the new group info
    for (uint32_t group1 = 0; group1 < group_props_before.size(); ++group1) {
        for (uint32_t group2 = 0; group2 < group_props_after.size(); ++group2) {
            uint32_t found_count = 0;
            bool found = false;
            for (uint32_t dev1 = 0; dev1 < group_props_before[group1].physicalDeviceCount; ++dev1) {
                found = false;
                for (uint32_t dev2 = 0; dev2 < group_props_after[group2].physicalDeviceCount; ++dev2) {
                    if (group_props_before[group1].physicalDevices[dev1] == group_props_after[group2].physicalDevices[dev2]) {
                        found_count++;
                        found = true;
                        break;
                    }
                }
            }
            ASSERT_EQ(found, found_count != 0 && found_count == before_expected_counts[group1]);
            if (found) {
                break;
            }
        }
    }
    for (auto& group : group_props_after) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Start with 6 devices in 3 different groups, and then remove a device to the middle group,
// querying vkEnumeratePhysicalDeviceGroups before and after the remove.
TEST(EnumeratePhysicalDeviceGroups, CallTwiceRemoveDeviceInBetween) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1);

    // Generate the devices
    for (size_t i = 0; i < 6; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[5]);

    uint32_t before_expected_counts[3] = {1, 3, 2};
    uint32_t after_expected_counts[3] = {1, 2, 2};
    uint32_t expected_group_count = 3;
    uint32_t returned_group_count = 0;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    std::vector<VkPhysicalDeviceGroupProperties> group_props_before{};
    group_props_before.resize(expected_group_count,
                              VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = expected_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_before.data()));
    ASSERT_EQ(expected_group_count, returned_group_count);
    printf("Before:\n");
    for (uint32_t group = 0; group < expected_group_count; ++group) {
        printf("  Group %u:\n", group);
        ASSERT_EQ(group_props_before[group].physicalDeviceCount, before_expected_counts[group]);
        for (uint32_t dev = 0; dev < group_props_before[group].physicalDeviceCount; ++dev) {
            printf("    Dev %u: %p\n", dev, group_props_before[group].physicalDevices[dev]);
        }
    }

    // Remove middle device in middle group
    driver.physical_device_groups[1].physical_device_handles.erase(
        driver.physical_device_groups[1].physical_device_handles.begin() + 1);

    std::vector<VkPhysicalDeviceGroupProperties> group_props_after{};
    group_props_after.resize(expected_group_count,
                             VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after.data()));
    ASSERT_EQ(expected_group_count, returned_group_count);
    printf("After:\n");
    for (uint32_t group = 0; group < expected_group_count; ++group) {
        printf("  Group %u:\n", group);
        ASSERT_EQ(group_props_after[group].physicalDeviceCount, after_expected_counts[group]);
        for (uint32_t dev = 0; dev < group_props_after[group].physicalDeviceCount; ++dev) {
            printf("    Dev %u: %p\n", dev, group_props_after[group].physicalDevices[dev]);
        }
    }

    // Make sure all devices in the new group info are found in the old group info
    for (uint32_t group1 = 0; group1 < group_props_after.size(); ++group1) {
        for (uint32_t group2 = 0; group2 < group_props_before.size(); ++group2) {
            uint32_t found_count = 0;
            bool found = false;
            for (uint32_t dev1 = 0; dev1 < group_props_after[group1].physicalDeviceCount; ++dev1) {
                found = false;
                for (uint32_t dev2 = 0; dev2 < group_props_before[group2].physicalDeviceCount; ++dev2) {
                    if (group_props_after[group1].physicalDevices[dev1] == group_props_before[group2].physicalDevices[dev2]) {
                        found_count++;
                        found = true;
                        break;
                    }
                }
            }
            ASSERT_EQ(found, found_count != 0 && found_count == after_expected_counts[group1]);
            if (found) {
                break;
            }
        }
    }
    for (auto& group : group_props_after) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Start with 9 devices but only some in 3 different groups, add and remove
// various devices and groups while querying in between.
TEST(EnumeratePhysicalDeviceGroups, MultipleAddRemoves) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
                       .set_min_icd_interface_version(5)
                       .set_icd_api_version(VK_API_VERSION_1_1);

    // Generate the devices
    for (size_t i = 0; i < 9; i++) {
        driver.physical_devices.emplace_back(std::string("physical_device_") + std::to_string(i));
        driver.physical_devices.back().properties.apiVersion = VK_API_VERSION_1_1;
    }

    // Generate the starting groups
    driver.physical_device_groups.emplace_back(driver.physical_devices[0]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[1]);
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[2])
        .use_physical_device(driver.physical_devices[3]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[4]);
    driver.physical_device_groups.back().use_physical_device(driver.physical_devices[5]);

    uint32_t before_expected_counts[3] = {1, 3, 2};
    uint32_t after_add_group_expected_counts[4] = {1, 3, 1, 2};
    uint32_t after_remove_dev_expected_counts[4] = {1, 2, 1, 2};
    uint32_t after_remove_group_expected_counts[3] = {2, 1, 2};
    uint32_t after_add_dev_expected_counts[3] = {2, 1, 4};
    uint32_t before_group_count = 3;
    uint32_t after_group_count = 4;
    uint32_t returned_group_count = 0;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    // Should be: 3 Groups { { 0 }, { 1, 2, 3 }, { 4, 5 } }
    std::vector<VkPhysicalDeviceGroupProperties> group_props_before{};
    group_props_before.resize(before_group_count,
                              VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = before_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_before.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_before[group].physicalDeviceCount, before_expected_counts[group]);
    }

    // Insert new group after first two
    driver.physical_device_groups.insert(driver.physical_device_groups.begin() + 2, driver.physical_devices[6]);

    // Should be: 4 Groups { { 0 }, { 1, 2, 3 }, { 6 }, { 4, 5 } }
    std::vector<VkPhysicalDeviceGroupProperties> group_props_after_add_group{};
    group_props_after_add_group.resize(after_group_count,
                                       VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = after_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after_add_group.data()));
    ASSERT_EQ(after_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_after_add_group[group].physicalDeviceCount, after_add_group_expected_counts[group]);
    }

    // Remove first device in 2nd group
    driver.physical_device_groups[1].physical_device_handles.erase(
        driver.physical_device_groups[1].physical_device_handles.begin());

    // Should be: 4 Groups { { 0 }, { 2, 3 }, { 6 }, { 4, 5 } }
    std::vector<VkPhysicalDeviceGroupProperties> group_props_after_remove_device{};
    group_props_after_remove_device.resize(after_group_count,
                                           VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = after_group_count;
    ASSERT_EQ(VK_SUCCESS,
              inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after_remove_device.data()));
    ASSERT_EQ(after_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_after_remove_device[group].physicalDeviceCount, after_remove_dev_expected_counts[group]);
    }

    // Remove first group
    driver.physical_device_groups.erase(driver.physical_device_groups.begin());

    // Should be: 3 Groups { { 2, 3 }, { 6 }, { 4, 5 } }
    std::vector<VkPhysicalDeviceGroupProperties> group_props_after_remove_group{};
    group_props_after_remove_group.resize(before_group_count,
                                          VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = before_group_count;
    ASSERT_EQ(VK_SUCCESS,
              inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after_remove_group.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_after_remove_group[group].physicalDeviceCount, after_remove_group_expected_counts[group]);
    }

    // Add two devices to last group
    driver.physical_device_groups.back()
        .use_physical_device(driver.physical_devices[7])
        .use_physical_device(driver.physical_devices[8]);

    // Should be: 3 Groups { { 2, 3 }, { 6 }, { 4, 5, 7, 8 } }
    std::vector<VkPhysicalDeviceGroupProperties> group_props_after_add_device{};
    group_props_after_add_device.resize(before_group_count,
                                        VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    returned_group_count = before_group_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props_after_add_device.data()));
    ASSERT_EQ(before_group_count, returned_group_count);
    for (uint32_t group = 0; group < before_group_count; ++group) {
        ASSERT_EQ(group_props_after_add_device[group].physicalDeviceCount, after_add_dev_expected_counts[group]);
    }
    for (auto& group : group_props_after_add_device) {
        VkDeviceGroupDeviceCreateInfo group_info{};
        group_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
        group_info.physicalDeviceCount = group.physicalDeviceCount;
        group_info.pPhysicalDevices = &group.physicalDevices[0];
        DeviceWrapper dev{inst};
        dev.create_info.dev.pNext = &group_info;
        dev.CheckCreate(group.physicalDevices[0]);
    }
}

// Fill in random but valid data into the device properties struct for the current physical device
void FillInRandomDeviceProps(VkPhysicalDeviceProperties& props, VkPhysicalDeviceType dev_type, uint32_t api_vers, uint32_t vendor,
                             uint32_t device) {
    props.apiVersion = api_vers;
    props.vendorID = vendor;
    props.deviceID = device;
    props.deviceType = dev_type;
    for (uint8_t idx = 0; idx < VK_UUID_SIZE; ++idx) {
        props.pipelineCacheUUID[idx] = static_cast<uint8_t>(rand() % 255);
    }
}

// Pass in a PNext that the fake ICD will fill in some data for.
TEST(EnumeratePhysicalDeviceGroups, FakePNext) {
    FrameworkEnvironment env{};

    // ICD 0: Vulkan 1.1
    //   PhysDev 0: pd0, Discrete, Vulkan 1.1, Bus 7
    //   PhysDev 1: pd1, Integrated, Vulkan 1.1, Bus 3
    //   PhysDev 2: pd2, Discrete, Vulkan 1.1, Bus 6
    //   Group 0: PhysDev 0, PhysDev 2
    //   Group 1: PhysDev 1
    // ICD 1: Vulkan 1.1
    //   PhysDev 4: pd4, Discrete, Vulkan 1.1, Bus 1
    //   PhysDev 5: pd5, Discrete, Vulkan 1.1, Bus 4
    //   PhysDev 6: pd6, Discrete, Vulkan 1.1, Bus 2
    //   Group 0: PhysDev 5, PhysDev 6
    //   Group 1: PhysDev 4
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_1));
    auto& cur_icd_0 = env.get_test_icd(0);
    cur_icd_0.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_0.physical_devices.push_back({"pd0"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA001);
    cur_icd_0.physical_devices.push_back({"pd1"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA002);
    cur_icd_0.physical_devices.push_back({"pd2"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA003);
    cur_icd_0.physical_device_groups.push_back({});
    cur_icd_0.physical_device_groups.back()
        .use_physical_device(cur_icd_0.physical_devices[0])
        .use_physical_device(cur_icd_0.physical_devices[2]);
    cur_icd_0.physical_device_groups.push_back({cur_icd_0.physical_devices[1]});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_1));
    auto& cur_icd_1 = env.get_test_icd(1);
    cur_icd_1.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_1.physical_devices.push_back({"pd4"});
    cur_icd_1.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_1.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC001);
    cur_icd_1.physical_devices.push_back({"pd5"});
    cur_icd_1.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_1.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC002);
    cur_icd_1.physical_devices.push_back({"pd6"});
    cur_icd_1.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_1.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC003);
    cur_icd_1.physical_device_groups.push_back({});
    cur_icd_1.physical_device_groups.back()
        .use_physical_device(cur_icd_1.physical_devices[1])
        .use_physical_device(cur_icd_1.physical_devices[2]);
    cur_icd_1.physical_device_groups.push_back({cur_icd_1.physical_devices[0]});

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = inst.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    // NOTE: This is a fake struct to make sure the pNext chain is properly passed down to the ICD
    //       vkEnumeratePhysicalDeviceGroups.
    //       The two versions must match:
    //           "FakePNext" test in loader_regression_tests.cpp
    //           "test_vkEnumeratePhysicalDeviceGroups" in test_icd.cpp
    struct FakePnextSharedWithICD {
        VkStructureType sType;
        void* pNext;
        uint32_t value;
    };

    const uint32_t max_phys_dev_groups = 4;
    uint32_t group_count = max_phys_dev_groups;
    std::array<FakePnextSharedWithICD, max_phys_dev_groups> fake_structs;
    std::array<VkPhysicalDeviceGroupProperties, max_phys_dev_groups> physical_device_groups{};
    for (uint32_t group = 0; group < max_phys_dev_groups; ++group) {
        physical_device_groups[group].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        fake_structs[group].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT;
        physical_device_groups[group].pNext = &fake_structs[group];
    }
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &group_count, physical_device_groups.data()));
    ASSERT_EQ(group_count, max_phys_dev_groups);

    // Value should get written to 0xDECAFBADD by the fake ICD
    for (uint32_t group = 0; group < max_phys_dev_groups; ++group) {
        ASSERT_EQ(fake_structs[group].value, 0xDECAFBAD);
    }
}

TEST(ExtensionManual, ToolingProperties) {
    VkPhysicalDeviceToolPropertiesEXT icd_tool_props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
                                                     nullptr,
                                                     "FakeICDTool",
                                                     "version_0_0_0_1.b",
                                                     VK_TOOL_PURPOSE_VALIDATION_BIT_EXT,
                                                     "This tool does not exist",
                                                     "No-Layer"};
    {  // No support in driver
        FrameworkEnvironment env{};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto phys_dev = inst.GetPhysDev();

        PFN_vkGetPhysicalDeviceToolPropertiesEXT getToolProperties = inst.load("vkGetPhysicalDeviceToolPropertiesEXT");
        handle_assert_has_value(getToolProperties);

        uint32_t tool_count = 0;
        ASSERT_EQ(VK_SUCCESS, getToolProperties(phys_dev, &tool_count, nullptr));
        ASSERT_EQ(tool_count, 0U);
    }
    {  // extension is supported in driver
        FrameworkEnvironment env{};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
            .set_supports_tooling_info_ext(true)
            .add_tooling_property(icd_tool_props)
            .add_physical_device(PhysicalDevice{}.add_extension(VK_EXT_TOOLING_INFO_EXTENSION_NAME).finish());

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto phys_dev = inst.GetPhysDev();

        PFN_vkGetPhysicalDeviceToolPropertiesEXT getToolProperties = inst.load("vkGetPhysicalDeviceToolPropertiesEXT");
        handle_assert_has_value(getToolProperties);
        uint32_t tool_count = 0;
        ASSERT_EQ(VK_SUCCESS, getToolProperties(phys_dev, &tool_count, nullptr));
        ASSERT_EQ(tool_count, 1U);
        VkPhysicalDeviceToolPropertiesEXT props{};
        ASSERT_EQ(VK_SUCCESS, getToolProperties(phys_dev, &tool_count, &props));
        ASSERT_EQ(tool_count, 1U);
        string_eq(props.name, icd_tool_props.name);
    }
    {  // core
        FrameworkEnvironment env{};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_3))
            .add_physical_device({})
            .set_supports_tooling_info_core(true)
            .add_tooling_property(icd_tool_props)
            .physical_devices.back()
            .properties.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto phys_dev = inst.GetPhysDev();

        PFN_vkGetPhysicalDeviceToolProperties getToolProperties = inst.load("vkGetPhysicalDeviceToolProperties");
        handle_assert_has_value(getToolProperties);
        uint32_t tool_count = 0;
        ASSERT_EQ(VK_SUCCESS, getToolProperties(phys_dev, &tool_count, nullptr));
        ASSERT_EQ(tool_count, 1U);
        VkPhysicalDeviceToolProperties props{};
        ASSERT_EQ(VK_SUCCESS, getToolProperties(phys_dev, &tool_count, &props));
        ASSERT_EQ(tool_count, 1U);
        string_eq(props.name, icd_tool_props.name);
    }
}
TEST(CreateInstance, InstanceNullLayerPtr) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    VkInstance inst = VK_NULL_HANDLE;
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.enabledLayerCount = 1;

    ASSERT_EQ(env.vulkan_functions.vkCreateInstance(&info, VK_NULL_HANDLE, &inst), VK_ERROR_LAYER_NOT_PRESENT);
}
TEST(CreateInstance, InstanceNullExtensionPtr) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    VkInstance inst = VK_NULL_HANDLE;
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.enabledExtensionCount = 1;

    ASSERT_EQ(env.vulkan_functions.vkCreateInstance(&info, VK_NULL_HANDLE, &inst), VK_ERROR_EXTENSION_NOT_PRESENT);
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__) || defined(__QNX__)
// NOTE: Sort order only affects Linux
TEST(SortedPhysicalDevices, DevicesSortEnabled10NoAppExt) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({"pd0"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(7);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA001);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.push_back({"pd1"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(3);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA002);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(1).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(1).physical_devices.push_back({"pd2"});
    env.get_test_icd(1).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(1).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_0,
                            1, 0xBBBB001);
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(2).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(2).physical_devices.push_back({"pd3"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(1);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 75, 0xCCCC001);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(2).physical_devices.push_back({"pd4"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(4);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 75, 0xCCCC002);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(3).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(3).physical_devices.push_back({"pd5"});
    env.get_test_icd(3).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(3).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                            VK_API_VERSION_1_1, 6940, 0xDDDD001);
    env.get_test_icd(3).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    const uint32_t max_phys_devs = 6;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);

        switch (dev) {
            case 0:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd3", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC001);
                break;
            case 1:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd4", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC002);
                break;
            case 2:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd0", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA001);
                break;
            case 3:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
                ASSERT_EQ(true, !strcmp("pd1", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA002);
                break;
            case 4:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
                ASSERT_EQ(true, !strcmp("pd5", props.deviceName));
                ASSERT_EQ(props.vendorID, 6940U);
                ASSERT_EQ(props.deviceID, 0xDDDD001);
                break;
            case 5:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
                ASSERT_EQ(true, !strcmp("pd2", props.deviceName));
                ASSERT_EQ(props.vendorID, 1U);
                ASSERT_EQ(props.deviceID, 0xBBBB001);
                break;
            default:
                ASSERT_EQ(false, true);
        }
    }

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices_again;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices_again.data()));
    ASSERT_EQ(device_count, max_phys_devs);
    for (uint32_t dev = 0; dev < device_count; ++dev) {
        ASSERT_EQ(physical_devices[dev], physical_devices_again[dev]);
    }
}

TEST(SortedPhysicalDevices, DevicesSortEnabled10AppExt) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({"pd0"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(7);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA001);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.push_back({"pd1"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(3);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA002);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(1).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(1).physical_devices.push_back({"pd2"});
    env.get_test_icd(1).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(1).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_0,
                            1, 0xBBBB001);
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(2).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(2).physical_devices.push_back({"pd3"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(1);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 75, 0xCCCC001);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(2).physical_devices.push_back({"pd4"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(4);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 75, 0xCCCC002);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(3).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(3).physical_devices.push_back({"pd5"});
    env.get_test_icd(3).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(3).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                            VK_API_VERSION_1_1, 6940, 0xDDDD001);
    env.get_test_icd(3).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    const uint32_t max_phys_devs = 6;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        VkPhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT};
        props2.pNext = &pci_bus_info;
        GetPhysDevProps2(physical_devices[dev], &props2);

        switch (dev) {
            case 0:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd3", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC001);
                ASSERT_EQ(pci_bus_info.pciBus, 1U);
                break;
            case 1:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd4", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC002);
                ASSERT_EQ(pci_bus_info.pciBus, 4U);
                break;
            case 2:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd0", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA001);
                ASSERT_EQ(pci_bus_info.pciBus, 7);
                break;
            case 3:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
                ASSERT_EQ(true, !strcmp("pd1", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA002);
                ASSERT_EQ(pci_bus_info.pciBus, 3U);
                break;
            case 4:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
                ASSERT_EQ(true, !strcmp("pd5", props.deviceName));
                ASSERT_EQ(props.vendorID, 6940U);
                ASSERT_EQ(props.deviceID, 0xDDDD001);
                ASSERT_EQ(pci_bus_info.pciBus, 0U);
                break;
            case 5:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
                ASSERT_EQ(true, !strcmp("pd2", props.deviceName));
                ASSERT_EQ(props.vendorID, 1U);
                ASSERT_EQ(props.deviceID, 0xBBBB001);
                ASSERT_EQ(pci_bus_info.pciBus, 0U);
                break;
            default:
                ASSERT_EQ(false, true);
        }
    }

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices_again;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices_again.data()));
    ASSERT_EQ(device_count, max_phys_devs);
    for (uint32_t dev = 0; dev < device_count; ++dev) {
        ASSERT_EQ(physical_devices[dev], physical_devices_again[dev]);
    }
}

TEST(SortedPhysicalDevices, DevicesSortEnabled11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(0).set_icd_api_version(VK_API_VERSION_1_1);
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({"pd0"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(7);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 888, 0xAAA001);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.push_back({"pd1"});
    env.get_test_icd(0).physical_devices.back().set_pci_bus(3);
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_0, 888, 0xAAA002);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(1).set_icd_api_version(VK_API_VERSION_1_1);
    env.get_test_icd(1).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(1).physical_devices.push_back({"pd2"});
    env.get_test_icd(1).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(1).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_0,
                            1, 0xBBBB001);
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(2).set_icd_api_version(VK_API_VERSION_1_1);
    env.get_test_icd(2).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(2).physical_devices.push_back({"pd3"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(1);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 75, 0xCCCC001);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(2).physical_devices.push_back({"pd4"});
    env.get_test_icd(2).physical_devices.back().set_pci_bus(4);
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_1, 75, 0xCCCC002);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.get_test_icd(3).set_icd_api_version(VK_API_VERSION_1_1);
    env.get_test_icd(3).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(3).physical_devices.push_back({"pd5"});
    env.get_test_icd(3).physical_devices.back().set_pci_bus(0);
    FillInRandomDeviceProps(env.get_test_icd(3).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                            VK_API_VERSION_1_1, 6940, 0xDDDD001);
    env.get_test_icd(3).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    const uint32_t max_phys_devs = 6;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        VkPhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT};
        props2.pNext = &pci_bus_info;
        GetPhysDevProps2(physical_devices[dev], &props2);

        switch (dev) {
            case 0:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd3", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC001);
                ASSERT_EQ(pci_bus_info.pciBus, 1U);
                break;
            case 1:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd4", props.deviceName));
                ASSERT_EQ(props.vendorID, 75);
                ASSERT_EQ(props.deviceID, 0xCCCC002);
                ASSERT_EQ(pci_bus_info.pciBus, 4U);
                break;
            case 2:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                ASSERT_EQ(true, !strcmp("pd0", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA001);
                ASSERT_EQ(pci_bus_info.pciBus, 7);
                break;
            case 3:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
                ASSERT_EQ(true, !strcmp("pd1", props.deviceName));
                ASSERT_EQ(props.vendorID, 888);
                ASSERT_EQ(props.deviceID, 0xAAA002);
                ASSERT_EQ(pci_bus_info.pciBus, 3U);
                break;
            case 4:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
                ASSERT_EQ(true, !strcmp("pd5", props.deviceName));
                ASSERT_EQ(props.vendorID, 6940U);
                ASSERT_EQ(props.deviceID, 0xDDDD001);
                ASSERT_EQ(pci_bus_info.pciBus, 0U);
                break;
            case 5:
                ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
                ASSERT_EQ(true, !strcmp("pd2", props.deviceName));
                ASSERT_EQ(props.vendorID, 1U);
                ASSERT_EQ(props.deviceID, 0xBBBB001);
                ASSERT_EQ(pci_bus_info.pciBus, 0U);
                break;
            default:
                ASSERT_EQ(false, true);
        }
    }

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices_again;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices_again.data()));
    ASSERT_EQ(device_count, max_phys_devs);
    for (uint32_t dev = 0; dev < device_count; ++dev) {
        ASSERT_EQ(physical_devices[dev], physical_devices_again[dev]);
    }
}

TEST(SortedPhysicalDevices, DevicesSortedDisabled) {
    FrameworkEnvironment env{};

    EnvVarWrapper disable_select_env_var{"VK_LOADER_DISABLE_SELECT", "1"};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({"pd0"});
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 888, 0xAAA001);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.push_back({"pd1"});
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_0, 888, 0xAAA002);
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(1).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(1).physical_devices.push_back({"pd2"});
    FillInRandomDeviceProps(env.get_test_icd(1).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_0,
                            1, 0xBBBB001);
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(2).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(2).physical_devices.push_back({"pd3"});
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 75, 0xCCCC001);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    env.get_test_icd(2).physical_devices.push_back({"pd4"});
    FillInRandomDeviceProps(env.get_test_icd(2).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                            VK_API_VERSION_1_0, 75, 0xCCCC002);
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(3).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(3).physical_devices.push_back({"pd5"});
    FillInRandomDeviceProps(env.get_test_icd(3).physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
                            VK_API_VERSION_1_0, 6940, 0xDDDD001);
    env.get_test_icd(3).physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    // Just make sure we have the correct number of devices
    const uint32_t max_phys_devs = 6;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    // make sure the order is what we started with - but its a bit wonky due to the loader reading physical devices "backwards"
    VkPhysicalDeviceProperties props{};
    instance->vkGetPhysicalDeviceProperties(physical_devices[0], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
    ASSERT_STREQ(props.deviceName, "pd5");

    instance->vkGetPhysicalDeviceProperties(physical_devices[1], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd3");

    instance->vkGetPhysicalDeviceProperties(physical_devices[2], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd4");

    instance->vkGetPhysicalDeviceProperties(physical_devices[3], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
    ASSERT_STREQ(props.deviceName, "pd2");

    instance->vkGetPhysicalDeviceProperties(physical_devices[4], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd0");

    instance->vkGetPhysicalDeviceProperties(physical_devices[5], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    ASSERT_STREQ(props.deviceName, "pd1");

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices_again;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices_again.data()));
    ASSERT_EQ(device_count, max_phys_devs);
    for (uint32_t dev = 0; dev < device_count; ++dev) {
        ASSERT_EQ(physical_devices[dev], physical_devices_again[dev]);
    }
}

TEST(SortedPhysicalDevices, DeviceGroupsSortedEnabled) {
    FrameworkEnvironment env{};

    // ICD 0: Vulkan 1.1
    //   PhysDev 0: pd0, Discrete, Vulkan 1.1, Bus 7
    //   PhysDev 1: pd1, Integrated, Vulkan 1.1, Bus 3
    //   PhysDev 2: pd2, Discrete, Vulkan 1.1, Bus 6
    //   Group 0: PhysDev 0, PhysDev 2
    //   Group 1: PhysDev 1
    // ICD 1: Vulkan 1.1
    //   PhysDev 3: pd3, CPU, Vulkan 1.1, Bus 0
    // ICD 2: Vulkan 1.1
    //   PhysDev 4: pd4, Discrete, Vulkan 1.1, Bus 1
    //   PhysDev 5: pd5, Discrete, Vulkan 1.1, Bus 4
    //   PhysDev 6: pd6, Discrete, Vulkan 1.1, Bus 2
    //   Group 0: PhysDev 5, PhysDev 6
    //   Group 1: PhysDev 4
    // ICD 3: Vulkan 1.1
    //   PhysDev 7: pd7, Virtual, Vulkan 1.1, Bus 0
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_0 = env.get_test_icd(0);
    cur_icd_0.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_0.physical_devices.push_back({"pd0"});
    cur_icd_0.physical_devices.back().set_pci_bus(7);
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA001);
    cur_icd_0.physical_devices.push_back({"pd1"});
    cur_icd_0.physical_devices.back().set_pci_bus(3);
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA002);
    cur_icd_0.physical_devices.push_back({"pd2"});
    cur_icd_0.physical_devices.back().set_pci_bus(6);
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA003);
    cur_icd_0.physical_device_groups.push_back({});
    cur_icd_0.physical_device_groups.back()
        .use_physical_device(cur_icd_0.physical_devices[0])
        .use_physical_device(cur_icd_0.physical_devices[2]);
    cur_icd_0.physical_device_groups.push_back({cur_icd_0.physical_devices[1]});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_1 = env.get_test_icd(1);
    cur_icd_1.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_1.physical_devices.push_back({"pd3"});
    cur_icd_1.physical_devices.back().set_pci_bus(0);
    cur_icd_1.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_1.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_1, 1,
                            0xBBBB001);

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_2 = env.get_test_icd(2);
    cur_icd_2.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_2.physical_devices.push_back({"pd4"});
    cur_icd_2.physical_devices.back().set_pci_bus(1);
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC001);
    cur_icd_2.physical_devices.push_back({"pd5"});
    cur_icd_2.physical_devices.back().set_pci_bus(4);
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC002);
    cur_icd_2.physical_devices.push_back({"pd6"});
    cur_icd_2.physical_devices.back().set_pci_bus(2);
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC003);
    cur_icd_2.physical_device_groups.push_back({});
    cur_icd_2.physical_device_groups.back()
        .use_physical_device(cur_icd_2.physical_devices[1])
        .use_physical_device(cur_icd_2.physical_devices[2]);
    cur_icd_2.physical_device_groups.push_back({cur_icd_2.physical_devices[0]});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_3 = env.get_test_icd(3);
    cur_icd_3.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_3.physical_devices.push_back({"pd7"});
    cur_icd_3.physical_devices.back().set_pci_bus(0);
    cur_icd_3.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_3.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, VK_API_VERSION_1_1,
                            6940, 0xDDDD001);

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = inst.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    const uint32_t max_phys_devs = 8;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    const uint32_t max_phys_dev_groups = 6;
    uint32_t group_count = max_phys_dev_groups;
    std::array<VkPhysicalDeviceGroupProperties, max_phys_dev_groups> physical_device_groups{};
    for (auto& group : physical_device_groups) group.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &group_count, physical_device_groups.data()));
    ASSERT_EQ(group_count, max_phys_dev_groups);

    uint32_t cur_dev = 0;
    for (uint32_t group = 0; group < max_phys_dev_groups; ++group) {
        for (uint32_t dev = 0; dev < physical_device_groups[group].physicalDeviceCount; ++dev) {
            VkPhysicalDeviceProperties props{};
            inst->vkGetPhysicalDeviceProperties(physical_device_groups[group].physicalDevices[dev], &props);
            VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            VkPhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT};
            props2.pNext = &pci_bus_info;
            GetPhysDevProps2(physical_device_groups[group].physicalDevices[dev], &props2);
            switch (cur_dev++) {
                case 0:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                    ASSERT_EQ(true, !strcmp("pd4", props.deviceName));
                    ASSERT_EQ(props.vendorID, 75);
                    ASSERT_EQ(props.deviceID, 0xCCCC001);
                    ASSERT_EQ(pci_bus_info.pciBus, 1U);
                    break;
                case 1:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                    ASSERT_EQ(true, !strcmp("pd6", props.deviceName));
                    ASSERT_EQ(props.vendorID, 75);
                    ASSERT_EQ(props.deviceID, 0xCCCC003);
                    ASSERT_EQ(pci_bus_info.pciBus, 2U);
                    break;
                case 2:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                    ASSERT_EQ(true, !strcmp("pd5", props.deviceName));
                    ASSERT_EQ(props.vendorID, 75);
                    ASSERT_EQ(props.deviceID, 0xCCCC002);
                    ASSERT_EQ(pci_bus_info.pciBus, 4U);
                    break;
                case 3:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                    ASSERT_EQ(true, !strcmp("pd2", props.deviceName));
                    ASSERT_EQ(props.vendorID, 888);
                    ASSERT_EQ(props.deviceID, 0xAAA003);
                    ASSERT_EQ(pci_bus_info.pciBus, 6);
                    break;
                case 4:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
                    ASSERT_EQ(true, !strcmp("pd0", props.deviceName));
                    ASSERT_EQ(props.vendorID, 888);
                    ASSERT_EQ(props.deviceID, 0xAAA001);
                    ASSERT_EQ(pci_bus_info.pciBus, 7);
                    break;
                case 5:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
                    ASSERT_EQ(true, !strcmp("pd1", props.deviceName));
                    ASSERT_EQ(props.vendorID, 888);
                    ASSERT_EQ(props.deviceID, 0xAAA002);
                    ASSERT_EQ(pci_bus_info.pciBus, 3U);
                    break;
                case 6:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
                    ASSERT_EQ(true, !strcmp("pd7", props.deviceName));
                    ASSERT_EQ(props.vendorID, 6940U);
                    ASSERT_EQ(props.deviceID, 0xDDDD001);
                    ASSERT_EQ(pci_bus_info.pciBus, 0U);
                    break;
                case 7:
                    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
                    ASSERT_EQ(true, !strcmp("pd3", props.deviceName));
                    ASSERT_EQ(props.vendorID, 1U);
                    ASSERT_EQ(props.deviceID, 0xBBBB001);
                    ASSERT_EQ(pci_bus_info.pciBus, 0U);
                    break;
                default:
                    ASSERT_EQ(false, true);
            }
        }
    }

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDeviceGroupProperties, max_phys_dev_groups> physical_device_groups_again{};
    for (auto& group : physical_device_groups_again) group.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;

    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &group_count, physical_device_groups_again.data()));
    ASSERT_EQ(group_count, max_phys_dev_groups);
    for (uint32_t group = 0; group < max_phys_dev_groups; ++group) {
        ASSERT_EQ(physical_device_groups[group].physicalDeviceCount, physical_device_groups_again[group].physicalDeviceCount);
        for (uint32_t dev = 0; dev < physical_device_groups[group].physicalDeviceCount; ++dev) {
            ASSERT_EQ(physical_device_groups[group].physicalDevices[dev], physical_device_groups_again[group].physicalDevices[dev]);
        }
    }
}

TEST(SortedPhysicalDevices, DeviceGroupsSortedDisabled) {
    FrameworkEnvironment env{};

    EnvVarWrapper disable_select_env_var{"VK_LOADER_DISABLE_SELECT", "1"};

    // ICD 0: Vulkan 1.1
    //   PhysDev 0: pd0, Discrete, Vulkan 1.1, Bus 7
    //   PhysDev 1: pd1, Integrated, Vulkan 1.1, Bus 3
    //   PhysDev 2: pd2, Discrete, Vulkan 1.1, Bus 6
    //   Group 0: PhysDev 0, PhysDev 2
    //   Group 1: PhysDev 1
    // ICD 1: Vulkan 1.1
    //   PhysDev 3: pd3, CPU, Vulkan 1.1, Bus 0
    // ICD 2: Vulkan 1.1
    //   PhysDev 4: pd4, Discrete, Vulkan 1.1, Bus 1
    //   PhysDev 5: pd5, Discrete, Vulkan 1.1, Bus 4
    //   PhysDev 6: pd6, Discrete, Vulkan 1.1, Bus 2
    //   Group 0: PhysDev 5, PhysDev 6
    //   Group 1: PhysDev 4
    // ICD 3: Vulkan 1.1
    //   PhysDev 7: pd7, Virtual, Vulkan 1.1, Bus 0
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_0 = env.get_test_icd(0);
    cur_icd_0.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_0.physical_devices.push_back({"pd0"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA001);
    cur_icd_0.physical_devices.push_back({"pd1"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
                            VK_API_VERSION_1_1, 888, 0xAAA002);
    cur_icd_0.physical_devices.push_back({"pd2"});
    cur_icd_0.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_0.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            888, 0xAAA003);
    cur_icd_0.physical_device_groups.push_back({});
    cur_icd_0.physical_device_groups.back()
        .use_physical_device(cur_icd_0.physical_devices[0])
        .use_physical_device(cur_icd_0.physical_devices[2]);
    cur_icd_0.physical_device_groups.push_back({cur_icd_0.physical_devices[1]});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_1 = env.get_test_icd(1);
    cur_icd_1.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_1.physical_devices.push_back({"pd3"});
    cur_icd_1.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_1.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_CPU, VK_API_VERSION_1_1, 1,
                            0xBBBB001);

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_2 = env.get_test_icd(2);
    cur_icd_2.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_2.physical_devices.push_back({"pd4"});
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC001);
    cur_icd_2.physical_devices.push_back({"pd5"});
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC002);
    cur_icd_2.physical_devices.push_back({"pd6"});
    cur_icd_2.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_2.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_API_VERSION_1_1,
                            75, 0xCCCC003);
    cur_icd_2.physical_device_groups.push_back({});
    cur_icd_2.physical_device_groups.back()
        .use_physical_device(cur_icd_2.physical_devices[1])
        .use_physical_device(cur_icd_2.physical_devices[2]);
    cur_icd_2.physical_device_groups.push_back({cur_icd_2.physical_devices[0]});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    auto& cur_icd_3 = env.get_test_icd(3);
    cur_icd_3.set_icd_api_version(VK_API_VERSION_1_1);
    cur_icd_3.physical_devices.push_back({"pd7"});
    cur_icd_3.physical_devices.back().extensions.push_back({VK_EXT_PCI_BUS_INFO_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(cur_icd_3.physical_devices.back().properties, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, VK_API_VERSION_1_1,
                            6940, 0xDDDD001);

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = inst.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    const uint32_t max_phys_devs = 8;
    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    const uint32_t max_phys_dev_groups = 6;
    uint32_t group_count = max_phys_dev_groups;
    std::array<VkPhysicalDeviceGroupProperties, max_phys_dev_groups> physical_device_groups{};
    for (auto& group : physical_device_groups) group.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;

    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &group_count, physical_device_groups.data()));
    ASSERT_EQ(group_count, max_phys_dev_groups);

    // make sure the order is what we started with - but its a bit wonky due to the loader reading physical devices "backwards"
    VkPhysicalDeviceProperties props{};
    inst->vkGetPhysicalDeviceProperties(physical_devices[0], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
    ASSERT_STREQ(props.deviceName, "pd7");

    inst->vkGetPhysicalDeviceProperties(physical_devices[1], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd4");

    inst->vkGetPhysicalDeviceProperties(physical_devices[2], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd5");

    inst->vkGetPhysicalDeviceProperties(physical_devices[3], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd6");

    inst->vkGetPhysicalDeviceProperties(physical_devices[4], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
    ASSERT_STREQ(props.deviceName, "pd3");

    inst->vkGetPhysicalDeviceProperties(physical_devices[5], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd0");

    inst->vkGetPhysicalDeviceProperties(physical_devices[6], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    ASSERT_STREQ(props.deviceName, "pd1");

    inst->vkGetPhysicalDeviceProperties(physical_devices[7], &props);
    ASSERT_EQ(props.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_STREQ(props.deviceName, "pd2");

    // Make sure if we call enumerate again, the information is the same
    std::array<VkPhysicalDeviceGroupProperties, max_phys_dev_groups> physical_device_groups_again{};
    for (auto& group : physical_device_groups_again) group.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;

    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &group_count, physical_device_groups_again.data()));
    ASSERT_EQ(group_count, max_phys_dev_groups);
    for (uint32_t group = 0; group < max_phys_dev_groups; ++group) {
        ASSERT_EQ(physical_device_groups[group].physicalDeviceCount, physical_device_groups_again[group].physicalDeviceCount);
        for (uint32_t dev = 0; dev < physical_device_groups[group].physicalDeviceCount; ++dev) {
            ASSERT_EQ(physical_device_groups[group].physicalDevices[dev], physical_device_groups_again[group].physicalDevices[dev]);
        }
    }
}

#endif  // __linux__ || __FreeBSD__ || __OpenBSD__ || __GNU__

const char* portability_driver_warning =
    "vkCreateInstance: Found drivers that contain devices which support the portability subset, but "
    "the instance does not enumerate portability drivers! Applications that wish to enumerate portability "
    "drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo "
    "flags and enable the VK_KHR_portability_enumeration instance extension.";

const char* portability_flag_missing =
    "vkCreateInstance: Found drivers that contain devices which support the portability subset, but "
    "the instance does not enumerate portability drivers! Applications that wish to enumerate portability "
    "drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo "
    "flags.";

const char* portability_extension_missing =
    "VkInstanceCreateInfo: If flags has the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit set, the "
    "list of enabled extensions in ppEnabledExtensionNames must contain VK_KHR_portability_enumeration "
    "[VUID-VkInstanceCreateInfo-flags-06559 ]"
    "Applications that wish to enumerate portability drivers must enable the VK_KHR_portability_enumeration "
    "instance extension.";

TEST(PortabilityICDConfiguration, PortabilityICDOnly) {
    FrameworkEnvironment env{};
    env.add_icd(
           TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_is_portability_driver(true)))
        .add_physical_device("physical_device_0")
        .set_max_icd_interface_version(1);
    {  // enable portability extension and flag
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        inst.create_info.add_extension("VK_KHR_portability_enumeration");
        inst.create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

        DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        auto phys_dev = inst.GetPhysDev();
        handle_assert_has_value(phys_dev);

        DeviceWrapper dev_info{inst};
        dev_info.CheckCreate(phys_dev);
        ASSERT_FALSE(log.find(portability_driver_warning));
        ASSERT_FALSE(log.find(portability_flag_missing));
        ASSERT_FALSE(log.find(portability_extension_missing));
    }
    {  // enable portability flag but not extension - shouldn't be able to create an instance when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
        ASSERT_TRUE(env.debug_log.find(portability_extension_missing));
    }
    {  // enable portability extension but not flag - shouldn't be able to create an instance when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension("VK_KHR_portability_enumeration");
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
        ASSERT_TRUE(env.debug_log.find(portability_flag_missing));
    }
    {  // enable neither the portability extension or the flag - shouldn't be able to create an instance when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.flags = 0;  // make sure its 0 - no portability
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
        ASSERT_TRUE(env.debug_log.find(portability_driver_warning));
    }
}

TEST(PortabilityICDConfiguration, PortabilityAndRegularICD) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)));
    env.add_icd(
        TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_is_portability_driver(true)));

    auto& driver0 = env.get_test_icd(0);
    auto& driver1 = env.get_test_icd(1);

    driver0.physical_devices.emplace_back("physical_device_0");
    driver0.max_icd_interface_version = 1;

    driver1.physical_devices.emplace_back("portability_physical_device_1");
    driver1.max_icd_interface_version = 1;
    {  // enable portability extension and flag
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        inst.create_info.add_extension("VK_KHR_portability_enumeration");
        inst.create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

        DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        auto phys_devs = inst.GetPhysDevs(2);
        for (const auto& phys_dev : phys_devs) {
            handle_assert_has_value(phys_dev);
        }
        DeviceWrapper dev_info_0{inst};
        DeviceWrapper dev_info_1{inst};
        dev_info_0.CheckCreate(phys_devs[0]);
        dev_info_1.CheckCreate(phys_devs[1]);
    }
    {  // enable portability extension but not flag - should only enumerate 1 physical device when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        inst.create_info.add_extension("VK_KHR_portability_enumeration");
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

        DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        auto phys_dev = inst.GetPhysDev();
        handle_assert_has_value(phys_dev);
        DeviceWrapper dev_info_0{inst};
        dev_info_0.CheckCreate(phys_dev);
    }
    {  // enable portability flag but not extension - should only enumerate 1 physical device when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        inst.create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

        DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        auto phys_dev = inst.GetPhysDev();
        handle_assert_has_value(phys_dev);
        DeviceWrapper dev_info_0{inst};
        dev_info_0.CheckCreate(phys_dev);
    }
    {  // do not enable portability extension or flag - should only enumerate 1 physical device when filtering is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

        DebugUtilsWrapper log{inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        auto phys_dev = inst.GetPhysDev();
        handle_assert_has_value(phys_dev);
        DeviceWrapper dev_info_0{inst};
        dev_info_0.CheckCreate(phys_dev);
    }
}

// Check that the portability enumeration flag bit doesn't get passed down
TEST(PortabilityICDConfiguration, PortabilityAndRegularICDCheckFlagsPassedIntoICD) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2)));
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2).set_is_portability_driver(true)));

    auto& driver0 = env.get_test_icd(0);
    auto& driver1 = env.get_test_icd(1);

    driver0.physical_devices.emplace_back("physical_device_0");
    driver0.max_icd_interface_version = 1;

    driver1.physical_devices.emplace_back("portability_physical_device_1");
    driver1.add_instance_extension("VK_KHR_portability_enumeration");
    driver1.max_icd_interface_version = 1;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    inst.create_info.add_extension("VK_KHR_portability_enumeration");
    inst.create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR | 4;

    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_FALSE(env.debug_log.find(portability_driver_warning));

    ASSERT_EQ(static_cast<VkInstanceCreateFlags>(4), driver0.passed_in_instance_create_flags);
    ASSERT_EQ(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR | static_cast<VkInstanceCreateFlags>(4),
              driver1.passed_in_instance_create_flags);
}

TEST(PortabilityICDConfiguration, PortabilityAndRegularICDPreInstanceFunctions) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_EXT_validation_features"};  // known instance extensions
    Extension second_ext{"VK_EXT_headless_surface"};
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)))
        .add_physical_device("physical_device_0")
        .set_max_icd_interface_version(1)
        .add_instance_extensions({first_ext, second_ext});
    env.add_icd(
           TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_is_portability_driver(true)))
        .add_physical_device("portability_physical_device_1")
        .set_max_icd_interface_version(1);
    {
        // check that enumerating instance extensions work with a portability driver present
        auto extensions = env.GetInstanceExtensions(6);
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, first_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, second_ext.extensionName.c_str()));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(5).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();

    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    {  // LayersMatch
        auto layer_props = inst.GetActiveLayers(phys_dev, 1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
    }
    {  // Property count less than available
        VkLayerProperties layer_props;
        uint32_t layer_count = 0;
        ASSERT_EQ(VK_INCOMPLETE, env.vulkan_functions.vkEnumerateDeviceLayerProperties(phys_dev, &layer_count, &layer_props));
        ASSERT_EQ(layer_count, 0U);
    }
}

#if defined(_WIN32)
TEST(AppPackageDiscovery, AppPackageDrivers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}.set_discovery_type(ManifestDiscoveryType::windows_app_package))
        .add_physical_device({});

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}
TEST(AppPackageDiscovery, AppPackageLayers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2))).add_physical_device({});

    const char* layer_name = "VK_LAYER_test_package_layer";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("DISABLE_ME")),
                                            "test_package_layer.json")
                               .set_discovery_type(ManifestDiscoveryType::windows_app_package));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1U);
    ASSERT_EQ(layers.size(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
}

TEST(AppPackageDiscovery, AppPackageICDAndLayers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2}.set_discovery_type(ManifestDiscoveryType::windows_app_package))
        .add_physical_device({});

    const char* layer_name = "VK_LAYER_test_package_layer";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("DISABLE_ME")),
                                            "test_package_layer.json")
                               .set_discovery_type(ManifestDiscoveryType::windows_app_package));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1U);
    ASSERT_EQ(layers.size(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
}

// Make sure that stale layer manifests (path to nonexistant file) which have the same name as real manifests don't cause the real
// manifests to be skipped. Stale registry entries happen when a registry is written on layer/driver installation but not cleaned up
// when the corresponding manifest is removed from the file system.
TEST(DuplicateRegistryEntries, Layers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2)));

    auto null_path = env.get_folder(ManifestLocation::null).location() / "test_layer.json";

    env.platform_shim->add_manifest(ManifestCategory::explicit_layer, null_path);

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    inst.CheckCreate();
}

// Check that the de-duplication of drivers found in both PnP and generic Khronos/Vulkan/Drivers doesn't result in the same thing
// being added twice
TEST(DuplicateRegistryEntries, Drivers) {
    FrameworkEnvironment env{};
    auto null_path = env.get_folder(ManifestLocation::null).location() / "test_icd_0.json";
    env.platform_shim->add_manifest(ManifestCategory::icd, null_path);

    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA}.set_discovery_type(ManifestDiscoveryType::null_dir))
        .add_physical_device("physical_device_0")
        .set_adapterLUID(_LUID{10, 1000});
    env.platform_shim->add_d3dkmt_adapter(D3DKMT_Adapter{0, _LUID{10, 1000}}.add_driver_manifest_path(env.get_icd_manifest_path()));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(std::string("Skipping adding of json file \"") + null_path.string() +
                                   "\" from registry \"HKEY_LOCAL_MACHINE\\" VK_DRIVERS_INFO_REGISTRY_LOC
                                   "\" to the list due to duplication"));
}
#endif

TEST(LibraryLoading, SystemLocations) {
    FrameworkEnvironment env{};
    EnvVarWrapper ld_library_path("LD_LIBRARY_PATH", env.get_folder(ManifestLocation::driver).location().string());
    ld_library_path.add_to_list(env.get_folder(ManifestLocation::explicit_layer).location());

    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2).set_library_path_type(LibraryPathType::default_search_paths))
                       .add_physical_device({});
    const char* fake_ext_name = "VK_FAKE_extension";
    driver.physical_devices.back().add_extension(fake_ext_name);

    const char* layer_name = "TestLayer";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "test_layer.json"}
            .set_library_path_type(LibraryPathType::default_search_paths));

    auto props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(props.at(0).layerName, layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();

    auto phys_dev = inst.GetPhysDev();

    auto active_props = inst.GetActiveLayers(phys_dev, 1);
    ASSERT_TRUE(string_eq(active_props.at(0).layerName, layer_name));

    auto device_extensions = inst.EnumerateDeviceExtensions(phys_dev, 1);
    ASSERT_TRUE(string_eq(device_extensions.at(0).extensionName, fake_ext_name));
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
// Check that valid symlinks do not cause the loader to crash when directly in an XDG env-var
TEST(ManifestDiscovery, ValidSymlinkInXDGEnvVar) {
    FrameworkEnvironment env{FrameworkSettings{}.set_enable_default_search_paths(false)};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::override_folder))
        .add_physical_device({});

    auto symlink_path =
        env.get_folder(ManifestLocation::driver_env_var).add_symlink(env.get_icd_manifest_path(0), "symlink_to_driver.json");

    EnvVarWrapper xdg_config_dirs_env_var{"XDG_CONFIG_DIRS", symlink_path};

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
}

// Check that valid symlinks do not cause the loader to crash
TEST(ManifestDiscovery, ValidSymlink) {
    FrameworkEnvironment env{FrameworkSettings{}.set_enable_default_search_paths(false)};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::override_folder))
        .add_physical_device({});

    auto symlink_path =
        env.get_folder(ManifestLocation::driver_env_var).add_symlink(env.get_icd_manifest_path(0), "symlink_to_driver.json");

    env.platform_shim->set_fake_path(ManifestCategory::icd, env.get_folder(ManifestLocation::driver_env_var).location());

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
}

// Check that invalid symlinks do not cause the loader to crash when directly in an XDG env-var
TEST(ManifestDiscovery, InvalidSymlinkXDGEnvVar) {
    FrameworkEnvironment env{FrameworkSettings{}.set_enable_default_search_paths(false)};

    auto symlink_path =
        env.get_folder(ManifestLocation::driver)
            .add_symlink(env.get_folder(ManifestLocation::driver).location() / "nothing_here.json", "symlink_to_nothing.json");

    EnvVarWrapper xdg_config_dirs_env_var{symlink_path};

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
}

// Check that invalid symlinks do not cause the loader to crash
TEST(ManifestDiscovery, InvalidSymlink) {
    FrameworkEnvironment env{FrameworkSettings{}.set_enable_default_search_paths(false)};

    auto symlink_path =
        env.get_folder(ManifestLocation::driver_env_var)
            .add_symlink(env.get_folder(ManifestLocation::driver).location() / "nothing_here.json", "symlink_to_nothing.json");

    env.platform_shim->set_fake_path(ManifestCategory::icd, env.get_folder(ManifestLocation::driver_env_var).location());

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
}
#endif

#if defined(__APPLE__)
// Add two drivers, one to the bundle and one to the system locations
TEST(ManifestDiscovery, AppleBundles) {
    FrameworkEnvironment env{};
    env.setup_macos_bundle();
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::macos_bundle));
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.at(0).properties.deviceID = 1337;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(1).physical_devices.push_back({});
    env.get_test_icd(1).physical_devices.at(0).properties.deviceID = 9999;

    InstWrapper inst{env.vulkan_functions};
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
    auto physical_devices = inst.GetPhysDevs();
    ASSERT_EQ(1, physical_devices.size());

    // Verify that this is the 'right' GPU, aka the one from the bundle
    VkPhysicalDeviceProperties props{};
    inst->vkGetPhysicalDeviceProperties(physical_devices[0], &props);
    ASSERT_EQ(env.get_test_icd(0).physical_devices.at(0).properties.deviceID, props.deviceID);
}

// Add two drivers, one to the bundle and one using the driver env-var
TEST(ManifestDiscovery, AppleBundlesEnvVarActive) {
    FrameworkEnvironment env{};
    env.setup_macos_bundle();
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::macos_bundle));
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.at(0).properties.deviceID = 1337;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::env_var));
    env.get_test_icd(1).physical_devices.push_back({});
    env.get_test_icd(1).physical_devices.at(0).properties.deviceID = 9999;

    InstWrapper inst{env.vulkan_functions};
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
    auto physical_devices = inst.GetPhysDevs();
    ASSERT_EQ(1, physical_devices.size());

    // Verify that this is the 'right' GPU, aka the one from the env-var
    VkPhysicalDeviceProperties props{};
    inst->vkGetPhysicalDeviceProperties(physical_devices[0], &props);
    ASSERT_EQ(env.get_test_icd(1).physical_devices.at(0).properties.deviceID, props.deviceID);
}
#endif

TEST(LayerCreatesDevice, Basic) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");
    env.get_test_layer().set_call_create_device_while_create_device_is_called(true);
    env.get_test_layer().set_physical_device_index_to_use_during_create_device(0);

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name2")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer2.json");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());
}

TEST(LayerCreatesDevice, DifferentPhysicalDevice) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.get_test_icd(0).physical_devices.emplace_back("Device0");
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.get_test_icd(1).physical_devices.emplace_back("Device1");

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");
    env.get_test_layer().set_call_create_device_while_create_device_is_called(true);
    env.get_test_layer().set_physical_device_index_to_use_during_create_device(1);

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name2")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer2.json");

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto phys_devs = inst.GetPhysDevs();

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_devs.at(0));
}

TEST(Layer, pfnNextGetInstanceProcAddr_should_not_return_layers_own_functions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");
    env.get_test_layer(0).set_check_if_EnumDevExtProps_is_same_as_queried_function(true);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto phys_devs = inst.GetPhysDevs();

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_devs.at(0));
}

TEST(Layer, LLP_LAYER_21) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");
    env.get_test_layer(0).set_clobber_pInstance(true);

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
#if defined(WIN32)
#if defined(_WIN64)
    ASSERT_DEATH(
        inst.CheckCreate(),
        testing::ContainsRegex(
            R"(terminator_CreateInstance: Instance pointer \(................\) has invalid MAGIC value 0x00000000. Instance value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_21\))"));
#else
    ASSERT_DEATH(
        inst.CheckCreate(),
        testing::ContainsRegex(
            R"(terminator_CreateInstance: Instance pointer \(........\) has invalid MAGIC value 0x00000000. Instance value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_21\))"));
#endif
#else
    ASSERT_DEATH(
        inst.CheckCreate(),
        testing::ContainsRegex(
            R"(terminator_CreateInstance: Instance pointer \(0x[0-9A-Fa-f]+\) has invalid MAGIC value 0x00000000. Instance value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_21\))"));
#endif
}

TEST(Layer, LLP_LAYER_22) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("implicit_layer_name")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");
    env.get_test_layer(0).set_clobber_pDevice(true);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension("VK_EXT_debug_utils");
    inst.CheckCreate();

    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);

    DeviceWrapper dev{inst};
#if defined(WIN32)
#if defined(_WIN64)
    ASSERT_DEATH(
        dev.CheckCreate(inst.GetPhysDev()),
        testing::ContainsRegex(
            R"(terminator_CreateDevice: Device pointer \(................\) has invalid MAGIC value 0x00000000. The expected value is 0x10ADED040410ADED. Device value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_22\))"));
#else
    ASSERT_DEATH(
        dev.CheckCreate(inst.GetPhysDev()),
        testing::ContainsRegex(
            R"(terminator_CreateDevice: Device pointer \(........\) has invalid MAGIC value 0x00000000. The expected value is 0x10ADED040410ADED. Device value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_22\))"));
#endif
#else
    ASSERT_DEATH(
        dev.CheckCreate(inst.GetPhysDev()),
        testing::ContainsRegex(
            R"(terminator_CreateDevice: Device pointer \(0x[0-9A-Fa-f]+\) has invalid MAGIC value 0x00000000. The expected value is 0x10ADED040410ADED. Device value )"
            R"(possibly corrupted by active layer \(Policy #LLP_LAYER_22\))"));
#endif
}

TEST(InvalidManifest, ICD) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<std::string> invalid_jsons;
    invalid_jsons.push_back(",");
    invalid_jsons.push_back("{},[]");
    invalid_jsons.push_back("{ \"foo\":\"bar\", }");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [], },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [{},] },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\"} },");
    invalid_jsons.push_back("{\"\":\"bar\", \"baz\": {}");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\":1234, true, \"ab\":\"bc\"} },");

    for (size_t i = 0; i < invalid_jsons.size(); i++) {
        auto file_name = std::string("invalid_driver_") + std::to_string(i) + ".json";
        std::filesystem::path new_path = env.get_folder(ManifestLocation::driver).write_manifest(file_name, invalid_jsons[i]);
        env.platform_shim->add_manifest(ManifestCategory::icd, new_path);
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}

TEST(InvalidManifest, Layer) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    std::vector<std::string> invalid_jsons;
    invalid_jsons.push_back(",");
    invalid_jsons.push_back("{},[]");
    invalid_jsons.push_back("{ \"foo\":\"bar\", }");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [], },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [{},] },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\"} },");
    invalid_jsons.push_back("{\"\":\"bar\", \"baz\": {}");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\":1234, true, \"ab\":\"bc\"} },");

    for (size_t i = 0; i < invalid_jsons.size(); i++) {
        auto file_name = std::string("invalid_implicit_layer_") + std::to_string(i) + ".json";
        std::filesystem::path new_path =
            env.get_folder(ManifestLocation::implicit_layer).write_manifest(file_name, invalid_jsons[i]);
        env.platform_shim->add_manifest(ManifestCategory::implicit_layer, new_path);
    }

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}
#if defined(WIN32)
void add_dxgi_adapter(FrameworkEnvironment& env, std::filesystem::path const& name, LUID luid, uint32_t vendor_id) {
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_6).set_discovery_type(ManifestDiscoveryType::null_dir));
    driver.set_min_icd_interface_version(5);
    driver.set_max_icd_interface_version(6);
    driver.setup_WSI();
    driver.set_icd_api_version(VK_API_VERSION_1_1);
    driver.physical_devices.emplace_back(name.string());
    auto& pd0 = driver.physical_devices.back();
    pd0.properties.apiVersion = VK_API_VERSION_1_1;
    driver.set_adapterLUID(luid);

    // luid is unique per DXGI/D3DKMT adapters.Don't add extra DXGI device if one is already created.
    // Just add path icd path to matching d3dkmt_adapter.
    D3DKMT_Adapter* pAdapter = NULL;
    for (uint32_t i = 0; i < env.platform_shim->d3dkmt_adapters.size(); i++) {
        if (env.platform_shim->d3dkmt_adapters[i].adapter_luid.HighPart == luid.HighPart &&
            env.platform_shim->d3dkmt_adapters[i].adapter_luid.LowPart == luid.LowPart) {
            pAdapter = &env.platform_shim->d3dkmt_adapters[i];
            break;
        }
    }
    if (pAdapter == NULL) {
        DXGI_ADAPTER_DESC1 desc{};
        desc.VendorId = known_driver_list.at(vendor_id).vendor_id;
        desc.AdapterLuid = luid;
        wcsncpy_s(desc.Description, 128, name.c_str(), name.native().size());
        env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc);

        env.platform_shim->add_d3dkmt_adapter(
            D3DKMT_Adapter{static_cast<UINT>(env.icds.size()) - 1U, desc.AdapterLuid}.add_driver_manifest_path(
                env.get_icd_manifest_path(env.icds.size() - 1)));
    } else {
        pAdapter->add_driver_manifest_path(env.get_icd_manifest_path(env.icds.size() - 1));
    }
}

TEST(EnumerateAdapterPhysicalDevices, SameAdapterLUID_reordered) {
    FrameworkEnvironment env;

    uint32_t physical_count = 3;

    // Physical devices are enumerated:
    // a) first in the order of LUIDs showing up in DXGIAdapter list
    // b) then in the reverse order to the drivers insertion into the test framework
    add_dxgi_adapter(env, "physical_device_2", LUID{10, 100}, 2);
    add_dxgi_adapter(env, "physical_device_1", LUID{20, 200}, 1);
    add_dxgi_adapter(env, "physical_device_0", LUID{10, 100}, 2);

    {
        uint32_t returned_physical_count = 0;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.setup_WSI().set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);
        std::vector<VkPhysicalDevice> physical_device_handles{returned_physical_count};
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count,
                                                                              physical_device_handles.data()));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDeviceProperties phys_dev_props[3]{};
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[0], &(phys_dev_props[0]));
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[1], &(phys_dev_props[1]));
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[2], &(phys_dev_props[2]));

        EXPECT_TRUE(string_eq(phys_dev_props[0].deviceName, "physical_device_0"));
        EXPECT_TRUE(string_eq(phys_dev_props[1].deviceName, "physical_device_2"));
        // Because LUID{10,100} is encountered first, all physical devices which correspond to that LUID are enumerated before any
        // other physical devices.
        EXPECT_TRUE(string_eq(phys_dev_props[2].deviceName, "physical_device_1"));

        // Check that both devices do not report VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT
        VkPhysicalDeviceLayeredDriverPropertiesMSFT layered_driver_properties_msft{};
        layered_driver_properties_msft.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;
        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = (void*)&layered_driver_properties_msft;

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[0], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[1], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[2], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);
    }
    // Set the first physical device that is enumerated to be a 'layered' driver so it should be swapped with the first physical
    // device
    env.get_test_icd(2).physical_devices.back().layered_driver_underlying_api = VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT;
    {
        uint32_t returned_physical_count = 0;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.setup_WSI().set_api_version(VK_API_VERSION_1_1);
        inst.CheckCreate();

        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);
        std::vector<VkPhysicalDevice> physical_device_handles{returned_physical_count};
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count,
                                                                              physical_device_handles.data()));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDeviceProperties phys_dev_props[3]{};
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[0], &(phys_dev_props[0]));
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[1], &(phys_dev_props[1]));
        env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[2], &(phys_dev_props[2]));

        // Because the 'last' driver has the layered_driver set to D3D12, the order is modified
        EXPECT_TRUE(string_eq(phys_dev_props[0].deviceName, "physical_device_2"));
        EXPECT_TRUE(string_eq(phys_dev_props[1].deviceName, "physical_device_0"));
        // Because LUID{10,100} is encountered first, all physical devices which correspond to that LUID are enumerated before any
        // other physical devices.
        EXPECT_TRUE(string_eq(phys_dev_props[2].deviceName, "physical_device_1"));

        // Check that the correct physical device reports VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT
        VkPhysicalDeviceLayeredDriverPropertiesMSFT layered_driver_properties_msft{};
        layered_driver_properties_msft.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;
        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = (void*)&layered_driver_properties_msft;

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[0], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[1], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT);

        env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[2], &props2);
        EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);
    }
}

TEST(EnumerateAdapterPhysicalDevices, SameAdapterLUID_same_order) {
    FrameworkEnvironment env;

    uint32_t physical_count = 3;

    // Physical devices are enumerated:
    // a) first in the order of LUIDs showing up in DXGIAdapter list
    // b) then in the reverse order to the drivers insertion into the test framework
    add_dxgi_adapter(env, "physical_device_2", LUID{10, 100}, 2);
    add_dxgi_adapter(env, "physical_device_1", LUID{20, 200}, 1);
    add_dxgi_adapter(env, "physical_device_0", LUID{10, 100}, 2);

    // Set the last physical device that is enumerated last to be a 'layered'  physical device - no swapping should occur
    env.get_test_icd(0).physical_devices.back().layered_driver_underlying_api = VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT;

    uint32_t returned_physical_count = 0;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);
    std::vector<VkPhysicalDevice> physical_device_handles{returned_physical_count};
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    VkPhysicalDeviceProperties phys_dev_props[3]{};
    env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[0], &(phys_dev_props[0]));
    env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[1], &(phys_dev_props[1]));
    env.vulkan_functions.vkGetPhysicalDeviceProperties(physical_device_handles[2], &(phys_dev_props[2]));

    // Make sure that reordering doesn't occur if the MSFT layered driver appears second
    EXPECT_TRUE(string_eq(phys_dev_props[0].deviceName, "physical_device_0"));
    EXPECT_TRUE(string_eq(phys_dev_props[1].deviceName, "physical_device_2"));
    // Because LUID{10,100} is encountered first, all physical devices which correspond to that LUID are enumerated before any
    // other physical devices.
    EXPECT_TRUE(string_eq(phys_dev_props[2].deviceName, "physical_device_1"));

    // Check that the correct physical device reports VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT
    VkPhysicalDeviceLayeredDriverPropertiesMSFT layered_driver_properties_msft{};
    layered_driver_properties_msft.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;
    VkPhysicalDeviceProperties2 props2{};
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props2.pNext = (void*)&layered_driver_properties_msft;

    env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[0], &props2);
    EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);

    env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[1], &props2);
    EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT);

    env.vulkan_functions.vkGetPhysicalDeviceProperties2(physical_device_handles[2], &props2);
    EXPECT_EQ(layered_driver_properties_msft.underlyingAPI, VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT);
}

TEST(EnumerateAdapterPhysicalDevices, WrongErrorCodes) {
    FrameworkEnvironment env;

    add_dxgi_adapter(env, "physical_device_0", LUID{10, 100}, 2);
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();
    // TestICD only fails in EnumAdapters, so shouldn't fail to query VkPhysicalDevices
    {
        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_INITIALIZATION_FAILED);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }
    {
        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_INCOMPATIBLE_DRIVER);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }
    {
        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_SURFACE_LOST_KHR);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 1);
    }

    // TestICD fails in EnumPhysDevs, should return VK_ERROR_INCOMPATIBLE_DRIVER
    auto check_icds = [&env, &inst] {
        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_INITIALIZATION_FAILED);
        uint32_t returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);

        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_INCOMPATIBLE_DRIVER);
        returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);

        env.get_test_icd().set_enum_adapter_physical_devices_return_code(VK_ERROR_SURFACE_LOST_KHR);
        returned_physical_count = 0;
        EXPECT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                  env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        EXPECT_EQ(returned_physical_count, 0);
    };

    // TestICD fails in EnumPhysDevs, should return VK_ERROR_INCOMPATIBLE_DRIVER
    env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_INITIALIZATION_FAILED);
    check_icds();

    // TestICD fails in EnumPhysDevs, should return VK_ERROR_INCOMPATIBLE_DRIVER
    env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_INCOMPATIBLE_DRIVER);
    check_icds();

    // TestICD fails in EnumPhysDevs, should return VK_ERROR_INCOMPATIBLE_DRIVER
    env.get_test_icd().set_enum_physical_devices_return_code(VK_ERROR_SURFACE_LOST_KHR);
    check_icds();
}

TEST(EnumerateAdapterPhysicalDevices, ManyAdapters) {
    FrameworkEnvironment env;

    uint32_t icd_count = 10;
    for (uint32_t i = 0; i < icd_count; i++) {
        // Add 2 separate physical devices with the same luid
        LUID luid{10U + i, static_cast<LONG>(100U + i)};
        add_dxgi_adapter(env, std::string("physical_device_") + std::to_string(i), luid, 2);
        add_dxgi_adapter(env, std::string("physical_device_") + std::to_string(i + icd_count), luid, 2);
    }
    uint32_t device_count = icd_count * 2;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    auto physical_devices = inst.GetPhysDevs(device_count);
    for (auto physical_device : physical_devices) {
        DeviceWrapper dev{inst};
        dev.CheckCreate(physical_device);
    }
}
#endif  // defined(WIN32)

void try_create_swapchain(InstWrapper& inst, VkPhysicalDevice physical_device, DeviceWrapper& dev, VkSurfaceKHR const& surface) {
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR = inst.load("vkGetPhysicalDeviceSurfaceSupportKHR");
    PFN_vkCreateSwapchainKHR CreateSwapchainKHR = dev.load("vkCreateSwapchainKHR");
    PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = dev.load("vkGetSwapchainImagesKHR");
    PFN_vkDestroySwapchainKHR DestroySwapchainKHR = dev.load("vkDestroySwapchainKHR");
    ASSERT_TRUE(nullptr != GetPhysicalDeviceSurfaceSupportKHR);
    ASSERT_TRUE(nullptr != CreateSwapchainKHR);
    ASSERT_TRUE(nullptr != GetSwapchainImagesKHR);
    ASSERT_TRUE(nullptr != DestroySwapchainKHR);

    VkBool32 supported = false;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface, &supported));
    ASSERT_EQ(supported, VK_TRUE);

    VkSwapchainKHR swapchain{};
    VkSwapchainCreateInfoKHR swap_create_info{};
    swap_create_info.surface = surface;

    ASSERT_EQ(VK_SUCCESS, CreateSwapchainKHR(dev, &swap_create_info, nullptr, &swapchain));
    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, GetSwapchainImagesKHR(dev, swapchain, &count, nullptr));
    ASSERT_GT(count, 0U);
    std::array<VkImage, 16> images;
    ASSERT_EQ(VK_SUCCESS, GetSwapchainImagesKHR(dev, swapchain, &count, images.data()));
    DestroySwapchainKHR(dev, swapchain, nullptr);
}

void add_driver_for_unloading_testing(FrameworkEnvironment& env) {
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
        .setup_WSI()
        .add_physical_device(PhysicalDevice{}
                                 .add_extension("VK_KHR_swapchain")
                                 .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true})
                                 .finish());
}

void add_empty_driver_for_unloading_testing(FrameworkEnvironment& env) {
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME).setup_WSI();
}

TEST(DriverUnloadingFromZeroPhysDevs, InterspersedThroughout) {
    FrameworkEnvironment env{};
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().add_extension("VK_EXT_debug_report");
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.CheckCreate();
    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));

    PFN_vkSubmitDebugUtilsMessageEXT submit_message = inst.load("vkSubmitDebugUtilsMessageEXT");
    ASSERT_TRUE(submit_message != nullptr);

    VkSurfaceKHR pre_surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, pre_surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> pre_enum_phys_devs_surface{
        pre_surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR};

    VkDebugReportCallbackEXT debug_callback{};
    VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};
    ASSERT_EQ(VK_SUCCESS, create_debug_callback(inst, debug_report_create_info, debug_callback));
    WrappedHandle<VkDebugReportCallbackEXT, VkInstance, PFN_vkDestroyDebugReportCallbackEXT>
        pre_enum_phys_devs_debug_report_callback{debug_callback, inst.inst, env.vulkan_functions.vkDestroyDebugReportCallbackEXT};

    auto phys_devs = inst.GetPhysDevs();
    std::vector<WrappedHandle<VkDebugUtilsMessengerEXT, VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT>> messengers;
    std::vector<WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR>> surfaces;
    for (uint32_t i = 0; i < 35; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
    for (const auto& phys_dev : phys_devs) {
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");
        dev.CheckCreate(phys_dev);
        for (const auto& surface : surfaces) {
            try_create_swapchain(inst, phys_dev, dev, surface.handle);
        }
    }
}

TEST(DriverUnloadingFromZeroPhysDevs, InMiddleOfList) {
    FrameworkEnvironment env{};
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    inst.create_info.setup_WSI().add_extension("VK_EXT_debug_report");
    inst.CheckCreate();
    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));

    VkSurfaceKHR pre_surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, pre_surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> pre_enum_phys_devs_surface{
        pre_surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR};

    VkDebugReportCallbackEXT debug_callback{};
    VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};
    ASSERT_EQ(VK_SUCCESS, create_debug_callback(inst, debug_report_create_info, debug_callback));
    WrappedHandle<VkDebugReportCallbackEXT, VkInstance, PFN_vkDestroyDebugReportCallbackEXT>
        pre_enum_phys_devs_debug_report_callback{debug_callback, inst.inst, env.vulkan_functions.vkDestroyDebugReportCallbackEXT};

    auto phys_devs = inst.GetPhysDevs();
    std::vector<WrappedHandle<VkDebugUtilsMessengerEXT, VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT>> messengers;
    std::vector<WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR>> surfaces;
    for (uint32_t i = 0; i < 35; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
    for (const auto& phys_dev : phys_devs) {
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");
        dev.CheckCreate(phys_dev);
        for (const auto& surface : surfaces) {
            try_create_swapchain(inst, phys_dev, dev, surface.handle);
        }
    }
}

TEST(DriverUnloadingFromZeroPhysDevs, AtFrontAndBack) {
    FrameworkEnvironment env{};
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().add_extension("VK_EXT_debug_report");
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.CheckCreate();

    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));

    VkSurfaceKHR pre_surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, pre_surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> pre_enum_phys_devs_surface{
        pre_surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR};

    VkDebugReportCallbackEXT debug_callback{};
    VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};
    ASSERT_EQ(VK_SUCCESS, create_debug_callback(inst, debug_report_create_info, debug_callback));
    WrappedHandle<VkDebugReportCallbackEXT, VkInstance, PFN_vkDestroyDebugReportCallbackEXT>
        pre_enum_phys_devs_debug_report_callback{debug_callback, inst.inst, env.vulkan_functions.vkDestroyDebugReportCallbackEXT};

    auto phys_devs = inst.GetPhysDevs();
    std::vector<WrappedHandle<VkDebugUtilsMessengerEXT, VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT>> messengers;
    std::vector<WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR>> surfaces;
    for (uint32_t i = 0; i < 35; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
    for (const auto& phys_dev : phys_devs) {
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");
        dev.CheckCreate(phys_dev);

        for (const auto& surface : surfaces) {
            try_create_swapchain(inst, phys_dev, dev, surface.handle);
        }
    }
}

TEST(DriverUnloadingFromZeroPhysDevs, MultipleEnumerateCalls) {
    FrameworkEnvironment env{};
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);

    uint32_t extension_count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, 0));
    ASSERT_EQ(extension_count, 6U);  // default extensions + surface extensions
    std::array<VkExtensionProperties, 6> extensions;
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto phys_devs1 = inst.GetPhysDevs();
        auto phys_devs2 = inst.GetPhysDevs();
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto phys_devs1 = inst.GetPhysDevs();
        auto phys_devs2 = inst.GetPhysDevs();
    }
}
TEST(DriverUnloadingFromZeroPhysDevs, NoPhysicalDevices) {
    FrameworkEnvironment env{};
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().add_extension("VK_EXT_debug_report");
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.CheckCreate();
    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));

    VkSurfaceKHR pre_surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, pre_surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> pre_enum_phys_devs_surface{
        pre_surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR};

    VkDebugReportCallbackEXT debug_callback{};
    VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};
    ASSERT_EQ(VK_SUCCESS, create_debug_callback(inst, debug_report_create_info, debug_callback));
    WrappedHandle<VkDebugReportCallbackEXT, VkInstance, PFN_vkDestroyDebugReportCallbackEXT>
        pre_enum_phys_devs_debug_report_callback{debug_callback, inst.inst, env.vulkan_functions.vkDestroyDebugReportCallbackEXT};

    // No physical devices == VK_ERROR_INITIALIZATION_FAILED
    inst.GetPhysDevs(VK_ERROR_INITIALIZATION_FAILED);

    std::vector<WrappedHandle<VkDebugUtilsMessengerEXT, VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT>> messengers;
    std::vector<WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR>> surfaces;
    for (uint32_t i = 0; i < 35; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
}

TEST(DriverUnloadingFromZeroPhysDevs, HandleRecreation) {
    FrameworkEnvironment env{};
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);
    add_driver_for_unloading_testing(env);
    add_empty_driver_for_unloading_testing(env);

    DebugUtilsLogger debug_log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI().add_extension("VK_EXT_debug_report");
    FillDebugUtilsCreateDetails(inst.create_info, debug_log);
    inst.CheckCreate();
    DebugUtilsWrapper log{inst};
    ASSERT_EQ(VK_SUCCESS, CreateDebugUtilsMessenger(log));

    PFN_vkSubmitDebugUtilsMessageEXT submit_message = inst.load("vkSubmitDebugUtilsMessageEXT");
    ASSERT_TRUE(submit_message != nullptr);

    VkSurfaceKHR pre_surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, pre_surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> pre_enum_phys_devs_surface{
        pre_surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR};

    VkDebugReportCallbackEXT debug_callback{};
    VkDebugReportCallbackCreateInfoEXT debug_report_create_info{};
    ASSERT_EQ(VK_SUCCESS, create_debug_callback(inst, debug_report_create_info, debug_callback));
    WrappedHandle<VkDebugReportCallbackEXT, VkInstance, PFN_vkDestroyDebugReportCallbackEXT>
        pre_enum_phys_devs_debug_report_callback{debug_callback, inst.inst, env.vulkan_functions.vkDestroyDebugReportCallbackEXT};

    auto phys_devs = inst.GetPhysDevs();
    std::vector<WrappedHandle<VkDebugUtilsMessengerEXT, VkInstance, PFN_vkDestroyDebugUtilsMessengerEXT>> messengers;
    std::vector<WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR>> surfaces;
    for (uint32_t i = 0; i < 35; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
    // Remove some elements arbitrarily - remove 15 of each
    // Do it backwards so the indexes are 'corect'
    for (uint32_t i = 31; i > 2; i -= 2) {
        messengers.erase(messengers.begin() + i);
        surfaces.erase(surfaces.begin() + i);
    }
    // Add in another 100
    for (uint32_t i = 0; i < 100; i++) {
        VkDebugUtilsMessengerEXT messenger;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateDebugUtilsMessengerEXT(inst.inst, log.get(), nullptr, &messenger));
        messengers.emplace_back(messenger, inst.inst, env.vulkan_functions.vkDestroyDebugUtilsMessengerEXT);

        VkSurfaceKHR surface{};
        ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
        surfaces.emplace_back(surface, inst.inst, env.vulkan_functions.vkDestroySurfaceKHR);
    }
    for (const auto& phys_dev : phys_devs) {
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");
        dev.CheckCreate(phys_dev);
        for (const auto& surface : surfaces) {
            try_create_swapchain(inst, phys_dev, dev, surface.handle);
        }
    }
    VkDebugUtilsMessengerCallbackDataEXT data{};
    data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
    data.pMessage = "I'm a test message!";
    data.messageIdNumber = 1;
    submit_message(inst.inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &data);

    ASSERT_EQ(120U + 1U, log.count(data.pMessage));
}
