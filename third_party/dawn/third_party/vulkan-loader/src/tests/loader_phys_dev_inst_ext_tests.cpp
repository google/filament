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
 * Author: Mark Young <marky@lunarg.com>
 */

#include "test_environment.h"

// These tests are all instance extension tests that touch physical devices.  This was
// before the idea that physical device extensions were more appropriately found in the
// list of device extensions.  Because of that, all these tests need to support devices
// that don't support the extension and have a fallback path in the loader that needs
// validation.

// Fill in random but valid data into the device properties struct for the current physical device
void FillInRandomICDInfo(uint32_t& vendor_id, uint32_t& driver_vers) {
    vendor_id = VK_MAKE_API_VERSION(0, rand() % 64, rand() % 255, rand() % 255);
    driver_vers = VK_MAKE_API_VERSION(0, rand() % 64, rand() % 255, rand() % 255);
}

// Fill in random but valid data into the device properties struct for the current physical device
void FillInRandomDeviceProps(VkPhysicalDeviceProperties& props, uint32_t api_vers, uint32_t vendor, uint32_t driver_vers) {
    props.apiVersion = api_vers;
    props.driverVersion = driver_vers;
    props.vendorID = vendor;
    props.deviceID = (static_cast<uint32_t>(rand()) >> 4) + (static_cast<uint32_t>(rand()) << 2);
    props.deviceType = static_cast<VkPhysicalDeviceType>(rand() % 5);
    for (uint8_t idx = 0; idx < VK_UUID_SIZE; ++idx) {
        props.pipelineCacheUUID[idx] = static_cast<uint8_t>(rand() % 255);
    }
}

//
// VK_KHR_get_physical_device_properties2
//

// Test vkGetPhysicalDeviceProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_EQ(GetPhysDevProps2, nullptr);
}

// Test vkGetPhysicalDeviceProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevProps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_EQ(GetPhysDevProps2, nullptr);
}

// Test vkGetPhysicalDeviceProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_API_VERSION_1_0, 5, 123);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceProperties props{};
    instance->vkGetPhysicalDeviceProperties(physical_device, &props);
    VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR};
    GetPhysDevProps2(physical_device, &props2);

    // Both properties should match
    ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
    ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
    ASSERT_EQ(props.vendorID, props2.properties.vendorID);
    ASSERT_EQ(props.deviceID, props2.properties.deviceID);
    ASSERT_EQ(props.deviceType, props2.properties.deviceType);
    ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));
}

// Test vkGetPhysicalDeviceProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_1));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_API_VERSION_1_1, 5, 123);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
        ASSERT_NE(GetPhysDevProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_device, &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR};
        GetPhysDevProps2(physical_device, &props2);

        // Both properties should match
        ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
        ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
        ASSERT_EQ(props.vendorID, props2.properties.vendorID);
        ASSERT_EQ(props.deviceID, props2.properties.deviceID);
        ASSERT_EQ(props.deviceType, props2.properties.deviceType);
        ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));
    }

    {  // Do the same logic but have the application forget to use 1.1 and doesn't enable the extension - should emulate the call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
        ASSERT_NE(GetPhysDevProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_device, &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR};
        GetPhysDevProps2(physical_device, &props2);

        // Both properties should match
        ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
        ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
        ASSERT_EQ(props.vendorID, props2.properties.vendorID);
        ASSERT_EQ(props.deviceID, props2.properties.deviceID);
        ASSERT_EQ(props.deviceType, props2.properties.deviceType);
        ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
        ASSERT_NE(GetPhysDevProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_device, &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR};
        GetPhysDevProps2(physical_device, &props2);

        // Both properties should match
        ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
        ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
        ASSERT_EQ(props.vendorID, props2.properties.vendorID);
        ASSERT_EQ(props.deviceID, props2.properties.deviceID);
        ASSERT_EQ(props.deviceType, props2.properties.deviceType);
        ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceProperties2 and vkGetPhysicalDeviceProperties2KHR where ICD is 1.0 and supports
// extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_0));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomDeviceProps(env.get_test_icd(0).physical_devices.back().properties, VK_API_VERSION_1_0, 5, 123);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2KHR = instance.load("vkGetPhysicalDeviceProperties2KHR");
    ASSERT_NE(GetPhysDevProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceProperties props{};
    instance->vkGetPhysicalDeviceProperties(physical_device, &props);
    VkPhysicalDeviceProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    GetPhysDevProps2(physical_device, &props2);

    // Both VkPhysicalDeviceProperties2 properties should match
    ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
    ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
    ASSERT_EQ(props.vendorID, props2.properties.vendorID);
    ASSERT_EQ(props.deviceID, props2.properties.deviceID);
    ASSERT_EQ(props.deviceType, props2.properties.deviceType);
    ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));

    VkPhysicalDeviceProperties2KHR props2KHR{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR};
    GetPhysDevProps2(physical_device, &props2KHR);

    // Both VkPhysicalDeviceProperties2KHR properties should match
    ASSERT_EQ(props.apiVersion, props2KHR.properties.apiVersion);
    ASSERT_EQ(props.driverVersion, props2KHR.properties.driverVersion);
    ASSERT_EQ(props.vendorID, props2KHR.properties.vendorID);
    ASSERT_EQ(props.deviceID, props2KHR.properties.deviceID);
    ASSERT_EQ(props.deviceType, props2KHR.properties.deviceType);
    ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2KHR.properties.pipelineCacheUUID, VK_UUID_SIZE));

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevProps2Mixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceProperties2 GetPhysDevProps2 = instance.load("vkGetPhysicalDeviceProperties2");
    ASSERT_NE(GetPhysDevProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);
        VkPhysicalDeviceProperties2KHR props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        GetPhysDevProps2(physical_devices[dev], &props2);

        // Both properties should match
        ASSERT_EQ(props.apiVersion, props2.properties.apiVersion);
        ASSERT_EQ(props.driverVersion, props2.properties.driverVersion);
        ASSERT_EQ(props.vendorID, props2.properties.vendorID);
        ASSERT_EQ(props.deviceID, props2.properties.deviceID);
        ASSERT_EQ(props.deviceType, props2.properties.deviceType);
        ASSERT_EQ(0, memcmp(props.pipelineCacheUUID, props2.properties.pipelineCacheUUID, VK_UUID_SIZE));
    }
}

// Fill in random but valid data into the features struct for the current physical device
void FillInRandomFeatures(VkPhysicalDeviceFeatures& feats) {
    feats.robustBufferAccess = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.fullDrawIndexUint32 = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.imageCubeArray = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.independentBlend = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.geometryShader = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.tessellationShader = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sampleRateShading = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.dualSrcBlend = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.logicOp = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.multiDrawIndirect = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.drawIndirectFirstInstance = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.depthClamp = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.depthBiasClamp = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.fillModeNonSolid = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.depthBounds = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.wideLines = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.largePoints = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.alphaToOne = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.multiViewport = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.samplerAnisotropy = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.textureCompressionETC2 = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.textureCompressionASTC_LDR = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.textureCompressionBC = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.occlusionQueryPrecise = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.pipelineStatisticsQuery = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.vertexPipelineStoresAndAtomics = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.fragmentStoresAndAtomics = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderTessellationAndGeometryPointSize = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderImageGatherExtended = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageImageExtendedFormats = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageImageMultisample = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageImageReadWithoutFormat = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageImageWriteWithoutFormat = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderUniformBufferArrayDynamicIndexing = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderSampledImageArrayDynamicIndexing = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageBufferArrayDynamicIndexing = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderStorageImageArrayDynamicIndexing = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderClipDistance = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderCullDistance = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderFloat64 = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderInt64 = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderInt16 = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderResourceResidency = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.shaderResourceMinLod = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseBinding = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidencyBuffer = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidencyImage2D = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidencyImage3D = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidency2Samples = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidency4Samples = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidency8Samples = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidency16Samples = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.sparseResidencyAliased = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.variableMultisampleRate = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
    feats.inheritedQueries = (rand() % 2) == 0 ? VK_FALSE : VK_TRUE;
}

// Test vkGetPhysicalDeviceFeatures2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevFeats2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysDevFeats2KHR = instance.load("vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_EQ(GetPhysDevFeats2KHR, nullptr);
}

// Test vkGetPhysicalDeviceFeatures2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevFeatsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysDevFeats2KHR = instance.load("vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_EQ(GetPhysDevFeats2KHR, nullptr);
}

// Test vkGetPhysicalDeviceFeatures2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevFeats2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomFeatures(env.get_test_icd(0).physical_devices.back().features);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysDevFeats2KHR = instance.load("vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_NE(GetPhysDevFeats2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceFeatures feats{};
    instance->vkGetPhysicalDeviceFeatures(physical_device, &feats);
    VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR};
    GetPhysDevFeats2KHR(physical_device, &feats2);
    ASSERT_EQ(feats, feats2);
}

// Test vkGetPhysicalDeviceFeatures2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevFeats2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_1));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.back().set_api_version(VK_API_VERSION_1_1);
    FillInRandomFeatures(env.get_test_icd(0).physical_devices.back().features);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceFeatures2 GetPhysDevFeats2 = instance.load("vkGetPhysicalDeviceFeatures2");
        ASSERT_NE(GetPhysDevFeats2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceFeatures feats{};
        instance->vkGetPhysicalDeviceFeatures(physical_device, &feats);
        VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        GetPhysDevFeats2(physical_device, &feats2);
        ASSERT_EQ(feats, feats2);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceFeatures2 GetPhysDevFeats2 = instance.load("vkGetPhysicalDeviceFeatures2");
        ASSERT_NE(GetPhysDevFeats2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceFeatures feats{};
        instance->vkGetPhysicalDeviceFeatures(physical_device, &feats);
        VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        GetPhysDevFeats2(physical_device, &feats2);
        ASSERT_EQ(feats, feats2);

        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceFeatures2 GetPhysDevFeats2 = instance.load("vkGetPhysicalDeviceFeatures2");
        ASSERT_NE(GetPhysDevFeats2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceFeatures feats{};
        instance->vkGetPhysicalDeviceFeatures(physical_device, &feats);
        VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        GetPhysDevFeats2(physical_device, &feats2);
        ASSERT_EQ(feats, feats2);

        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceFeatures2 and vkGetPhysicalDeviceFeatures2KHR where ICD is 1.0 and supports
// extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevFeats2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_0));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomFeatures(env.get_test_icd(0).physical_devices.back().features);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceFeatures2KHR GetPhysDevFeats2KHR = instance.load("vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_NE(GetPhysDevFeats2KHR, nullptr);

    PFN_vkGetPhysicalDeviceFeatures2 GetPhysDevFeats2 = instance.load("vkGetPhysicalDeviceFeatures2");
    ASSERT_NE(GetPhysDevFeats2, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceFeatures feats{};
    instance->vkGetPhysicalDeviceFeatures(physical_device, &feats);

    VkPhysicalDeviceFeatures2KHR feats2KHR{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR};
    GetPhysDevFeats2KHR(physical_device, &feats2KHR);
    ASSERT_EQ(feats, feats2KHR);

    VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    GetPhysDevFeats2(physical_device, &feats2);
    ASSERT_EQ(feats, feats2);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceFeatures2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevFeatsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomFeatures(cur_dev.features);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceFeatures2 GetPhysDevFeats2 = instance.load("vkGetPhysicalDeviceFeatures2");
    ASSERT_NE(GetPhysDevFeats2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceFeatures feats{};
        instance->vkGetPhysicalDeviceFeatures(physical_devices[dev], &feats);
        VkPhysicalDeviceFeatures2 feats2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        GetPhysDevFeats2(physical_devices[dev], &feats2);
        ASSERT_EQ(feats, feats2);
    }
}

// Fill in random but valid data into the format properties struct for the current physical device
void FillInRandomFormatProperties(std::vector<VkFormatProperties>& props) {
    props.resize(5);
    for (uint8_t form = 0; form < 5; ++form) {
        props[form].bufferFeatures = static_cast<VkFormatFeatureFlags>(rand());
        props[form].linearTilingFeatures = static_cast<VkFormatFeatureFlags>(rand());
        props[form].optimalTilingFeatures = static_cast<VkFormatFeatureFlags>(rand());
    }
}

// Test vkGetPhysicalDeviceFormatProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevFormatProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceFormatProperties2KHR GetPhysDevFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevFormatProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceFormatProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevFormatPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceFormatProperties2KHR GetPhysDevFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevFormatProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceFormatProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevFormatProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomFormatProperties(env.get_test_icd(0).physical_devices.back().format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceFormatProperties2KHR GetPhysDevFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceFormatProperties2KHR");
    ASSERT_NE(GetPhysDevFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkFormatProperties props{};
    instance->vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props);
    VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    GetPhysDevFormatProps2KHR(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2);

    ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
    ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
    ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);
}

// Test vkGetPhysicalDeviceFormatProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevFormatProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_1));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    env.get_test_icd(0).physical_devices.back().set_api_version(VK_API_VERSION_1_1);
    FillInRandomFormatProperties(env.get_test_icd(0).physical_devices.back().format_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysDevFormatProps2 = instance.load("vkGetPhysicalDeviceFormatProperties2");
        ASSERT_NE(GetPhysDevFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkFormatProperties props{};
        instance->vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props);
        VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        GetPhysDevFormatProps2(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2);

        ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
        ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
        ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);
    }
    {  // Do the same logic but have the application forget to enable 1.1 and doesn't enable the extension
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysDevFormatProps2 = instance.load("vkGetPhysicalDeviceFormatProperties2");
        ASSERT_NE(GetPhysDevFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkFormatProperties props{};
        instance->vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props);
        VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        GetPhysDevFormatProps2(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2);

        ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
        ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
        ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysDevFormatProps2 = instance.load("vkGetPhysicalDeviceFormatProperties2");
        ASSERT_NE(GetPhysDevFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkFormatProperties props{};
        instance->vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props);
        VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        GetPhysDevFormatProps2(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2);

        ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
        ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
        ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}
// Test vkGetPhysicalDeviceFormatProperties2 and vkGetPhysicalDeviceFormatProperties2KHR where ICD is 1.0 and supports
// extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevFormatProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomFormatProperties(env.get_test_icd(0).physical_devices.back().format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysDevFormatProps2 = instance.load("vkGetPhysicalDeviceFormatProperties2");
    ASSERT_NE(GetPhysDevFormatProps2, nullptr);

    PFN_vkGetPhysicalDeviceFormatProperties2KHR GetPhysDevFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceFormatProperties2KHR");
    ASSERT_NE(GetPhysDevFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkFormatProperties props{};
    instance->vkGetPhysicalDeviceFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props);
    VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    GetPhysDevFormatProps2(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2);

    ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
    ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
    ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);

    VkFormatProperties2KHR props2KHR{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    GetPhysDevFormatProps2KHR(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, &props2KHR);

    ASSERT_EQ(props.bufferFeatures, props2KHR.formatProperties.bufferFeatures);
    ASSERT_EQ(props.linearTilingFeatures, props2KHR.formatProperties.linearTilingFeatures);
    ASSERT_EQ(props.optimalTilingFeatures, props2KHR.formatProperties.optimalTilingFeatures);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceFormatProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevFormatPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomFormatProperties(cur_dev.format_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysDevFormatProps2 = instance.load("vkGetPhysicalDeviceFormatProperties2");
    ASSERT_NE(GetPhysDevFormatProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkFormat format = static_cast<VkFormat>((dev + 1) % 5);
        VkFormatProperties props{};
        instance->vkGetPhysicalDeviceFormatProperties(physical_devices[dev], format, &props);
        VkFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        GetPhysDevFormatProps2(physical_devices[dev], format, &props2);

        ASSERT_EQ(props.bufferFeatures, props2.formatProperties.bufferFeatures);
        ASSERT_EQ(props.linearTilingFeatures, props2.formatProperties.linearTilingFeatures);
        ASSERT_EQ(props.optimalTilingFeatures, props2.formatProperties.optimalTilingFeatures);
    }
}

// Fill in random but valid data into the image format data struct for the current physical device
void FillInRandomImageFormatData(VkImageFormatProperties& props) {
    props.maxExtent = {static_cast<uint32_t>(rand() % 512), static_cast<uint32_t>(rand() % 512),
                       static_cast<uint32_t>(rand() % 512)};
    props.maxMipLevels = static_cast<uint32_t>(1 << (rand() % 16));
    props.maxArrayLayers = static_cast<uint32_t>(1 << (rand() % 16));
    props.sampleCounts = static_cast<VkSampleCountFlags>(1 << (rand() % 7));
    props.maxResourceSize = static_cast<uint64_t>(rand());
}

// Test vkGetPhysicalDeviceImageFormatProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevImageFormatProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR GetPhysDevImageFormatProps2 =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevImageFormatProps2, nullptr);
}

// Test vkGetPhysicalDeviceImageFormatProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevImageFormatPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR GetPhysDevImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevImageFormatProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceImageFormatProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevImageFormatProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomImageFormatData(env.get_test_icd(0).physical_devices.back().image_format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR GetPhysDevImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_NE(GetPhysDevImageFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkImageFormatProperties props{};
    ASSERT_EQ(VK_SUCCESS,
              instance->vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

    VkPhysicalDeviceImageFormatInfo2 info2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
        nullptr,                                                // pNext
        VK_FORMAT_R4G4_UNORM_PACK8,                             // format
        VK_IMAGE_TYPE_2D,                                       // type
        VK_IMAGE_TILING_OPTIMAL,                                // tiling
        0,                                                      // usage
        0,                                                      // flags
    };
    VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2KHR(physical_device, &info2, &props2));

    ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
    ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
    ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
    ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
    ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
    ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
    ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);
}

// Test vkGetPhysicalDeviceImageFormatProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevImageFormatProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomImageFormatData(env.get_test_icd(0).physical_devices.back().image_format_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysDevImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceImageFormatProperties2");
        ASSERT_NE(GetPhysDevImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkImageFormatProperties props{};
        ASSERT_EQ(VK_SUCCESS,
                  instance->vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                     VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

        VkPhysicalDeviceImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                             // format
            VK_IMAGE_TYPE_2D,                                       // type
            VK_IMAGE_TILING_OPTIMAL,                                // tiling
            0,                                                      // usage
            0,                                                      // flags
        };

        VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2(physical_device, &info2, &props2));

        ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
        ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
        ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
        ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
        ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
        ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
        ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysDevImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceImageFormatProperties2");
        ASSERT_NE(GetPhysDevImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkImageFormatProperties props{};
        ASSERT_EQ(VK_SUCCESS,
                  instance->vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                     VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

        VkPhysicalDeviceImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                             // format
            VK_IMAGE_TYPE_2D,                                       // type
            VK_IMAGE_TILING_OPTIMAL,                                // tiling
            0,                                                      // usage
            0,                                                      // flags
        };

        VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2(physical_device, &info2, &props2));

        ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
        ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
        ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
        ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
        ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
        ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
        ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysDevImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceImageFormatProperties2");
        ASSERT_NE(GetPhysDevImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkImageFormatProperties props{};
        ASSERT_EQ(VK_SUCCESS,
                  instance->vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                     VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

        VkPhysicalDeviceImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                             // format
            VK_IMAGE_TYPE_2D,                                       // type
            VK_IMAGE_TILING_OPTIMAL,                                // tiling
            0,                                                      // usage
            0,                                                      // flags
        };

        VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2(physical_device, &info2, &props2));

        ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
        ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
        ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
        ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
        ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
        ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
        ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceImageFormatProperties2 and vkGetPhysicalDeviceImageFormatProperties2KHR where instance supports, an ICD,
// and a device under that ICD also support, so everything should work and return properly.
TEST(LoaderInstPhysDevExts, PhysDevImageFormatProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomImageFormatData(env.get_test_icd(0).physical_devices.back().image_format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysDevImageFormatProps2 =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2");
    ASSERT_NE(GetPhysDevImageFormatProps2, nullptr);

    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR GetPhysDevImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_NE(GetPhysDevImageFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkImageFormatProperties props{};
    ASSERT_EQ(VK_SUCCESS,
              instance->vkGetPhysicalDeviceImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

    VkPhysicalDeviceImageFormatInfo2 info2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
        nullptr,                                                // pNext
        VK_FORMAT_R4G4_UNORM_PACK8,                             // format
        VK_IMAGE_TYPE_2D,                                       // type
        VK_IMAGE_TILING_OPTIMAL,                                // tiling
        0,                                                      // usage
        0,                                                      // flags
    };

    VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
    ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2(physical_device, &info2, &props2));

    ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
    ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
    ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
    ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
    ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
    ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
    ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);

    VkImageFormatProperties2KHR props2KHR{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR};
    ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2KHR(physical_device, &info2, &props2KHR));

    ASSERT_EQ(props.maxExtent.width, props2KHR.imageFormatProperties.maxExtent.width);
    ASSERT_EQ(props.maxExtent.height, props2KHR.imageFormatProperties.maxExtent.height);
    ASSERT_EQ(props.maxExtent.depth, props2KHR.imageFormatProperties.maxExtent.depth);
    ASSERT_EQ(props.maxMipLevels, props2KHR.imageFormatProperties.maxMipLevels);
    ASSERT_EQ(props.maxArrayLayers, props2KHR.imageFormatProperties.maxArrayLayers);
    ASSERT_EQ(props.sampleCounts, props2KHR.imageFormatProperties.sampleCounts);
    ASSERT_EQ(props.maxResourceSize, props2KHR.imageFormatProperties.maxResourceSize);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceImageFormatProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevImageFormatPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomImageFormatData(cur_dev.image_format_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysDevImageFormatProps2 =
        instance.load("vkGetPhysicalDeviceImageFormatProperties2");
    ASSERT_NE(GetPhysDevImageFormatProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkImageFormatProperties props{};
        ASSERT_EQ(VK_SUCCESS,
                  instance->vkGetPhysicalDeviceImageFormatProperties(physical_devices[dev], VK_FORMAT_R4G4_UNORM_PACK8,
                                                                     VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, 0, 0, &props));

        VkPhysicalDeviceImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                             // format
            VK_IMAGE_TYPE_2D,                                       // type
            VK_IMAGE_TILING_OPTIMAL,                                // tiling
            0,                                                      // usage
            0,                                                      // flags
        };
        VkImageFormatProperties2 props2{VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        ASSERT_EQ(VK_SUCCESS, GetPhysDevImageFormatProps2(physical_devices[dev], &info2, &props2));

        ASSERT_EQ(props.maxExtent.width, props2.imageFormatProperties.maxExtent.width);
        ASSERT_EQ(props.maxExtent.height, props2.imageFormatProperties.maxExtent.height);
        ASSERT_EQ(props.maxExtent.depth, props2.imageFormatProperties.maxExtent.depth);
        ASSERT_EQ(props.maxMipLevels, props2.imageFormatProperties.maxMipLevels);
        ASSERT_EQ(props.maxArrayLayers, props2.imageFormatProperties.maxArrayLayers);
        ASSERT_EQ(props.sampleCounts, props2.imageFormatProperties.sampleCounts);
        ASSERT_EQ(props.maxResourceSize, props2.imageFormatProperties.maxResourceSize);
    }
}

// Test vkGetPhysicalDeviceMemoryProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevMemoryProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysDevMemoryProps2KHR =
        instance.load("vkGetPhysicalDeviceMemoryProperties2KHR");
    ASSERT_EQ(GetPhysDevMemoryProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceMemoryProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevMemoryPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysDevMemoryProps2KHR =
        instance.load("vkGetPhysicalDeviceMemoryProperties2KHR");
    ASSERT_EQ(GetPhysDevMemoryProps2KHR, nullptr);
}

// Fill in random but valid data into the memory data struct for the current physical device
void FillInRandomMemoryData(VkPhysicalDeviceMemoryProperties& props) {
    props.memoryTypeCount = (rand() % 7) + 1;
    props.memoryHeapCount = (rand() % 7) + 1;
    for (uint32_t i = 0; i < props.memoryHeapCount; ++i) {
        props.memoryHeaps[i].size = (rand() % 728) + (rand() % 728) + 1;
        props.memoryHeaps[i].flags = (rand() % 2) + 1;
    }
    for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
        props.memoryTypes[i].propertyFlags = static_cast<VkMemoryPropertyFlags>((rand() % 2) + 1);
        props.memoryTypes[i].heapIndex = rand() % props.memoryHeapCount;
    }
}

// Test vkGetPhysicalDeviceMemoryProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevMemoryProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomMemoryData(env.get_test_icd(0).physical_devices.back().memory_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysDevMemoryProps2KHR =
        instance.load("vkGetPhysicalDeviceMemoryProperties2KHR");
    ASSERT_NE(GetPhysDevMemoryProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceMemoryProperties props{};
    instance->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

    VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
    GetPhysDevMemoryProps2KHR(physical_device, &props2);
    ASSERT_EQ(props, props2);
}

// Test vkGetPhysicalDeviceMemoryProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevMemoryProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomMemoryData(env.get_test_icd(0).physical_devices.back().memory_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysDevMemoryProps2 = instance.load("vkGetPhysicalDeviceMemoryProperties2");
        ASSERT_NE(GetPhysDevMemoryProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceMemoryProperties props{};
        instance->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

        VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        GetPhysDevMemoryProps2(physical_device, &props2);
        ASSERT_EQ(props, props2);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysDevMemoryProps2 = instance.load("vkGetPhysicalDeviceMemoryProperties2");
        ASSERT_NE(GetPhysDevMemoryProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceMemoryProperties props{};
        instance->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

        VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        GetPhysDevMemoryProps2(physical_device, &props2);
        ASSERT_EQ(props, props2);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysDevMemoryProps2 = instance.load("vkGetPhysicalDeviceMemoryProperties2");
        ASSERT_NE(GetPhysDevMemoryProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceMemoryProperties props{};
        instance->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

        VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        GetPhysDevMemoryProps2(physical_device, &props2);
        ASSERT_EQ(props, props2);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceMemoryProperties2 and vkGetPhysicalDeviceMemoryProperties2KHR where ICD is 1.0 and supports
// extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevMemoryProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomMemoryData(env.get_test_icd(0).physical_devices.back().memory_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysDevMemoryProps2 = instance.load("vkGetPhysicalDeviceMemoryProperties2");
    ASSERT_NE(GetPhysDevMemoryProps2, nullptr);

    PFN_vkGetPhysicalDeviceMemoryProperties2KHR GetPhysDevMemoryProps2KHR =
        instance.load("vkGetPhysicalDeviceMemoryProperties2KHR");
    ASSERT_NE(GetPhysDevMemoryProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceMemoryProperties props{};
    instance->vkGetPhysicalDeviceMemoryProperties(physical_device, &props);

    VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
    GetPhysDevMemoryProps2(physical_device, &props2);
    ASSERT_EQ(props, props2);

    VkPhysicalDeviceMemoryProperties2KHR props2KHR{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR};
    GetPhysDevMemoryProps2KHR(physical_device, &props2KHR);
    ASSERT_EQ(props, props2KHR);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}
// Test vkGetPhysicalDeviceMemoryProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevMemoryPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomMemoryData(cur_dev.memory_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysDevMemoryProps2 = instance.load("vkGetPhysicalDeviceMemoryProperties2");
    ASSERT_NE(GetPhysDevMemoryProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceMemoryProperties props{};
        instance->vkGetPhysicalDeviceMemoryProperties(physical_devices[dev], &props);

        VkPhysicalDeviceMemoryProperties2 props2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        GetPhysDevMemoryProps2(physical_devices[dev], &props2);
        ASSERT_EQ(props, props2);
    }
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR GetPhysDevQueueFamilyProps2KHR =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    ASSERT_EQ(GetPhysDevQueueFamilyProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR GetPhysDevQueueFamilyProps2KHR =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    ASSERT_EQ(GetPhysDevQueueFamilyProps2KHR, nullptr);
}

// Fill in random but valid data into the queue family data struct for the current physical device
uint32_t FillInRandomQueueFamilyData(std::vector<MockQueueFamilyProperties>& props) {
    props.resize((rand() % 4) + 1);
    for (uint32_t i = 0; i < props.size(); ++i) {
        props[i].properties.queueFlags = (rand() % 30) + 1;
        props[i].properties.queueCount = (rand() % 7) + 1;
        props[i].properties.timestampValidBits = (rand() % 30) + 7;
        props[i].properties.minImageTransferGranularity.width = (rand() % 30) + 1;
        props[i].properties.minImageTransferGranularity.height = (rand() % 30) + 1;
        props[i].properties.minImageTransferGranularity.depth = (rand() % 30) + 1;
        props[i].support_present = rand() % 2 == 0;
    }
    return static_cast<uint32_t>(props.size());
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    uint32_t num_fam = FillInRandomQueueFamilyData(env.get_test_icd(0).physical_devices.back().queue_family_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR GetPhysDevQueueFamilyProps2KHR =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    ASSERT_NE(GetPhysDevQueueFamilyProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    uint32_t ret_fam_1 = 0;
    std::vector<VkQueueFamilyProperties> props{};
    instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, nullptr);
    ASSERT_EQ(num_fam, ret_fam_1);
    props.resize(ret_fam_1);

    instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, props.data());

    std::vector<VkQueueFamilyProperties2> props2{};
    uint32_t ret_fam_2 = 0;
    GetPhysDevQueueFamilyProps2KHR(physical_device, &ret_fam_2, nullptr);
    ASSERT_EQ(ret_fam_1, ret_fam_2);
    props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
    GetPhysDevQueueFamilyProps2KHR(physical_device, &ret_fam_2, props2.data());
    ASSERT_EQ(props, props2);
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    uint32_t num_fam = FillInRandomQueueFamilyData(env.get_test_icd(0).physical_devices.back().queue_family_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysDevQueueFamilyProps2 =
            instance.load("vkGetPhysicalDeviceQueueFamilyProperties2");
        ASSERT_NE(GetPhysDevQueueFamilyProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        uint32_t ret_fam_1 = 0;
        std::vector<VkQueueFamilyProperties> props{};
        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, nullptr);
        ASSERT_EQ(num_fam, ret_fam_1);
        props.resize(ret_fam_1);

        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, props.data());

        std::vector<VkQueueFamilyProperties2> props2{};
        uint32_t ret_fam_2 = 0;
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, nullptr);
        ASSERT_EQ(ret_fam_1, ret_fam_2);
        props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, props2.data());
        ASSERT_EQ(props, props2);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysDevQueueFamilyProps2 =
            instance.load("vkGetPhysicalDeviceQueueFamilyProperties2");
        ASSERT_NE(GetPhysDevQueueFamilyProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        uint32_t ret_fam_1 = 0;
        std::vector<VkQueueFamilyProperties> props{};
        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, nullptr);
        ASSERT_EQ(num_fam, ret_fam_1);
        props.resize(ret_fam_1);

        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, props.data());

        std::vector<VkQueueFamilyProperties2> props2{};
        uint32_t ret_fam_2 = 0;
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, nullptr);
        ASSERT_EQ(ret_fam_1, ret_fam_2);
        props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, props2.data());
        ASSERT_EQ(props, props2);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysDevQueueFamilyProps2 =
            instance.load("vkGetPhysicalDeviceQueueFamilyProperties2");
        ASSERT_NE(GetPhysDevQueueFamilyProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        uint32_t ret_fam_1 = 0;
        std::vector<VkQueueFamilyProperties> props{};
        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, nullptr);
        ASSERT_EQ(num_fam, ret_fam_1);
        props.resize(ret_fam_1);

        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, props.data());

        std::vector<VkQueueFamilyProperties2> props2{};
        uint32_t ret_fam_2 = 0;
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, nullptr);
        ASSERT_EQ(ret_fam_1, ret_fam_2);
        props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, props2.data());
        ASSERT_EQ(props, props2);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2 and vkGetPhysicalDeviceQueueFamilyProperties2KHR where ICD is 1.0 and supports
// extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    uint32_t num_fam = FillInRandomQueueFamilyData(env.get_test_icd(0).physical_devices.back().queue_family_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysDevQueueFamilyProps2 =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2");
    ASSERT_NE(GetPhysDevQueueFamilyProps2, nullptr);

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR GetPhysDevQueueFamilyProps2KHR =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    ASSERT_NE(GetPhysDevQueueFamilyProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    uint32_t ret_fam_1 = 0;
    std::vector<VkQueueFamilyProperties> props{};
    instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, nullptr);
    ASSERT_EQ(num_fam, ret_fam_1);
    props.resize(ret_fam_1);

    instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &ret_fam_1, props.data());

    std::vector<VkQueueFamilyProperties2> props2{};
    uint32_t ret_fam_2 = 0;
    GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, nullptr);
    ASSERT_EQ(ret_fam_1, ret_fam_2);
    props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
    GetPhysDevQueueFamilyProps2(physical_device, &ret_fam_2, props2.data());
    ASSERT_EQ(props, props2);

    std::vector<VkQueueFamilyProperties2KHR> props2KHR{};
    uint32_t ret_fam_2_khr = 0;
    GetPhysDevQueueFamilyProps2KHR(physical_device, &ret_fam_2_khr, nullptr);
    ASSERT_EQ(ret_fam_1, ret_fam_2_khr);
    props2KHR.resize(ret_fam_2_khr, VkQueueFamilyProperties2KHR{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR});
    GetPhysDevQueueFamilyProps2KHR(physical_device, &ret_fam_2_khr, props2KHR.data());
    ASSERT_EQ(props, props2KHR);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceQueueFamilyProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevQueueFamilyPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomQueueFamilyData(cur_dev.queue_family_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysDevQueueFamilyProps2 =
        instance.load("vkGetPhysicalDeviceQueueFamilyProperties2");
    ASSERT_NE(GetPhysDevQueueFamilyProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        uint32_t ret_fam_1 = 0;
        std::vector<VkQueueFamilyProperties> props{};
        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[dev], &ret_fam_1, nullptr);
        props.resize(ret_fam_1);
        instance->vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[dev], &ret_fam_1, props.data());

        std::vector<VkQueueFamilyProperties2> props2{};
        uint32_t ret_fam_2 = 0;
        GetPhysDevQueueFamilyProps2(physical_devices[dev], &ret_fam_2, nullptr);
        ASSERT_EQ(ret_fam_1, ret_fam_2);
        props2.resize(ret_fam_2, VkQueueFamilyProperties2{VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        GetPhysDevQueueFamilyProps2(physical_devices[dev], &ret_fam_2, props2.data());
        ASSERT_EQ(props, props2);
    }
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysDevSparseImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevSparseImageFormatProps2KHR, nullptr);
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysDevSparseImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    ASSERT_EQ(GetPhysDevSparseImageFormatProps2KHR, nullptr);
}

// Fill in random but valid data into the sparse image format data struct for the current physical device
void FillInRandomSparseImageFormatData(std::vector<VkSparseImageFormatProperties>& props) {
    props.resize((rand() % 4) + 1);
    for (uint32_t i = 0; i < props.size(); ++i) {
        props[i].aspectMask = static_cast<VkImageAspectFlags>((rand() % 0x7FE) + 1);
        props[i].imageGranularity = {static_cast<uint32_t>(rand() % 512), static_cast<uint32_t>(rand() % 512),
                                     static_cast<uint32_t>(rand() % 512)};
        props[i].flags = static_cast<VkSparseImageFormatFlags>((rand() % 6) + 1);
    }
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomSparseImageFormatData(env.get_test_icd(0).physical_devices.back().sparse_image_format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysDevSparseImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    ASSERT_NE(GetPhysDevSparseImageFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkSparseImageFormatProperties> props{};
    uint32_t sparse_count_1 = 0;
    instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                             VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                             VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
    ASSERT_NE(sparse_count_1, 0U);
    props.resize(sparse_count_1);
    instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                             VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                             VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
    ASSERT_NE(sparse_count_1, 0U);

    VkPhysicalDeviceSparseImageFormatInfo2 info2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
        nullptr,                                                       // pNext
        VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
        VK_IMAGE_TYPE_2D,                                              // type
        VK_SAMPLE_COUNT_4_BIT,                                         // samples
        VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
        VK_IMAGE_TILING_OPTIMAL,                                       // tiling
    };
    std::vector<VkSparseImageFormatProperties2> props2{};
    uint32_t sparse_count_2 = 0;
    GetPhysDevSparseImageFormatProps2KHR(physical_device, &info2, &sparse_count_2, nullptr);
    ASSERT_EQ(sparse_count_1, sparse_count_2);
    props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
    GetPhysDevSparseImageFormatProps2KHR(physical_device, &info2, &sparse_count_2, props2.data());
    ASSERT_EQ(sparse_count_1, sparse_count_2);
    ASSERT_EQ(props, props2);
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2 where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomSparseImageFormatData(env.get_test_icd(0).physical_devices.back().sparse_image_format_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysDevSparseImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2");
        ASSERT_NE(GetPhysDevSparseImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        std::vector<VkSparseImageFormatProperties> props{};
        uint32_t sparse_count_1 = 0;
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
        ASSERT_NE(sparse_count_1, 0U);
        props.resize(sparse_count_1);
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
        ASSERT_NE(sparse_count_1, 0U);

        VkPhysicalDeviceSparseImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                       // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
            VK_IMAGE_TYPE_2D,                                              // type
            VK_SAMPLE_COUNT_4_BIT,                                         // samples
            VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
            VK_IMAGE_TILING_OPTIMAL,                                       // tiling
        };
        std::vector<VkSparseImageFormatProperties2> props2{};
        uint32_t sparse_count_2 = 0;
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, nullptr);
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, props2.data());
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        ASSERT_EQ(props, props2);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysDevSparseImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2");
        ASSERT_NE(GetPhysDevSparseImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        std::vector<VkSparseImageFormatProperties> props{};
        uint32_t sparse_count_1 = 0;
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
        ASSERT_NE(sparse_count_1, 0U);
        props.resize(sparse_count_1);
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
        ASSERT_NE(sparse_count_1, 0U);

        VkPhysicalDeviceSparseImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                       // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
            VK_IMAGE_TYPE_2D,                                              // type
            VK_SAMPLE_COUNT_4_BIT,                                         // samples
            VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
            VK_IMAGE_TILING_OPTIMAL,                                       // tiling
        };
        std::vector<VkSparseImageFormatProperties2> props2{};
        uint32_t sparse_count_2 = 0;
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, nullptr);
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, props2.data());
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        ASSERT_EQ(props, props2);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysDevSparseImageFormatProps2 =
            instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2");
        ASSERT_NE(GetPhysDevSparseImageFormatProps2, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        std::vector<VkSparseImageFormatProperties> props{};
        uint32_t sparse_count_1 = 0;
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
        ASSERT_NE(sparse_count_1, 0U);
        props.resize(sparse_count_1);
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                                 VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                                 VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
        ASSERT_NE(sparse_count_1, 0U);

        VkPhysicalDeviceSparseImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                       // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
            VK_IMAGE_TYPE_2D,                                              // type
            VK_SAMPLE_COUNT_4_BIT,                                         // samples
            VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
            VK_IMAGE_TILING_OPTIMAL,                                       // tiling
        };
        std::vector<VkSparseImageFormatProperties2> props2{};
        uint32_t sparse_count_2 = 0;
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, nullptr);
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
        GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, props2.data());
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        ASSERT_EQ(props, props2);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2 and vkGetPhysicalDeviceSparseImageFormatProperties2KHR where ICD is 1.0 and
// supports extension but the instance supports 1.1 and the extension
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatProps2KHRInstanceSupports11) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
    FillInRandomSparseImageFormatData(env.get_test_icd(0).physical_devices.back().sparse_image_format_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.create_info.add_extensions(
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    instance.CheckCreate();
    DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
    CreateDebugUtilsMessenger(log);

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysDevSparseImageFormatProps2 =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2");
    ASSERT_NE(GetPhysDevSparseImageFormatProps2, nullptr);

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysDevSparseImageFormatProps2KHR =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    ASSERT_NE(GetPhysDevSparseImageFormatProps2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkSparseImageFormatProperties> props{};
    uint32_t sparse_count_1 = 0;
    instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                             VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                             VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
    ASSERT_NE(sparse_count_1, 0U);
    props.resize(sparse_count_1);
    instance->vkGetPhysicalDeviceSparseImageFormatProperties(physical_device, VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D,
                                                             VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
                                                             VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
    ASSERT_NE(sparse_count_1, 0U);

    VkPhysicalDeviceSparseImageFormatInfo2 info2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
        nullptr,                                                       // pNext
        VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
        VK_IMAGE_TYPE_2D,                                              // type
        VK_SAMPLE_COUNT_4_BIT,                                         // samples
        VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
        VK_IMAGE_TILING_OPTIMAL,                                       // tiling
    };
    std::vector<VkSparseImageFormatProperties2> props2{};
    uint32_t sparse_count_2 = 0;
    GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, nullptr);
    ASSERT_EQ(sparse_count_1, sparse_count_2);
    props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
    GetPhysDevSparseImageFormatProps2(physical_device, &info2, &sparse_count_2, props2.data());
    ASSERT_EQ(sparse_count_1, sparse_count_2);
    ASSERT_EQ(props, props2);

    std::vector<VkSparseImageFormatProperties2KHR> props2KHR{};
    uint32_t sparse_count_2_khr = 0;
    GetPhysDevSparseImageFormatProps2KHR(physical_device, &info2, &sparse_count_2_khr, nullptr);
    ASSERT_EQ(sparse_count_1, sparse_count_2_khr);
    props2KHR.resize(sparse_count_2, VkSparseImageFormatProperties2KHR{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR});
    GetPhysDevSparseImageFormatProps2KHR(physical_device, &info2, &sparse_count_2_khr, props2KHR.data());
    ASSERT_EQ(sparse_count_1, sparse_count_2_khr);
    ASSERT_EQ(props, props2KHR);

    ASSERT_FALSE(log.find("Emulating call in ICD"));
}

// Test vkGetPhysicalDeviceSparseImageFormatProperties2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevSparseImageFormatPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomSparseImageFormatData(cur_dev.sparse_image_format_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysDevSparseImageFormatProps2 =
        instance.load("vkGetPhysicalDeviceSparseImageFormatProperties2");
    ASSERT_NE(GetPhysDevSparseImageFormatProps2, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        std::vector<VkSparseImageFormatProperties> props{};
        uint32_t sparse_count_1 = 0;
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(
            physical_devices[dev], VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, nullptr);
        ASSERT_NE(sparse_count_1, 0U);
        props.resize(sparse_count_1);
        instance->vkGetPhysicalDeviceSparseImageFormatProperties(
            physical_devices[dev], VK_FORMAT_R4G4_UNORM_PACK8, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT,
            VK_IMAGE_TILING_OPTIMAL, &sparse_count_1, props.data());
        ASSERT_NE(sparse_count_1, 0U);

        VkPhysicalDeviceSparseImageFormatInfo2 info2{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,  // sType
            nullptr,                                                       // pNext
            VK_FORMAT_R4G4_UNORM_PACK8,                                    // format
            VK_IMAGE_TYPE_2D,                                              // type
            VK_SAMPLE_COUNT_4_BIT,                                         // samples
            VK_IMAGE_USAGE_STORAGE_BIT,                                    // usage
            VK_IMAGE_TILING_OPTIMAL,                                       // tiling
        };
        std::vector<VkSparseImageFormatProperties2> props2{};
        uint32_t sparse_count_2 = 0;
        GetPhysDevSparseImageFormatProps2(physical_devices[dev], &info2, &sparse_count_2, nullptr);
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        props2.resize(sparse_count_2, VkSparseImageFormatProperties2{VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2});
        GetPhysDevSparseImageFormatProps2(physical_devices[dev], &info2, &sparse_count_2, props2.data());
        ASSERT_EQ(sparse_count_1, sparse_count_2);
        ASSERT_EQ(props, props2);
    }
}

//
// VK_KHR_external_memory_capabilities
//

// Test vkGetPhysicalDeviceExternalBufferPropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevExtBufPropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR GetPhysicalDeviceExternalBufferPropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalBufferPropertiesKHR, nullptr);
}

// Test vkGetPhysicalDeviceExternalBufferPropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevExtBufPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR GetPhysicalDeviceExternalBufferPropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalBufferPropertiesKHR, nullptr);
}

// Fill in random but valid data into the external memorydata struct for the current physical device
void FillInRandomExtMemoryData(VkExternalMemoryProperties& props) {
    props.externalMemoryFeatures = static_cast<VkExternalMemoryFeatureFlags>((rand() % 6) + 1);
    props.exportFromImportedHandleTypes = static_cast<VkExternalMemoryHandleTypeFlags>((rand() % 0x1FFE) + 1);
    props.compatibleHandleTypes = static_cast<VkExternalMemoryHandleTypeFlags>((rand() % 0x1FFE) + 1);
}

// Test vkGetPhysicalDeviceExternalBufferPropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevExtBufProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomExtMemoryData(env.get_test_icd(0).physical_devices.back().external_memory_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR GetPhysicalDeviceExternalBufferPropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceExternalBufferPropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceExternalBufferInfoKHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR};
    VkExternalBufferPropertiesKHR props{VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR};
    GetPhysicalDeviceExternalBufferPropertiesKHR(physical_device, &info, &props);
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_memory_properties, props.externalMemoryProperties);
}

// Test vkGetPhysicalDeviceExternalBufferProperties where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevExtBufProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, 0});
    FillInRandomExtMemoryData(env.get_test_icd(0).physical_devices.back().external_memory_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties =
            instance.load("vkGetPhysicalDeviceExternalBufferProperties");
        ASSERT_NE(GetPhysicalDeviceExternalBufferProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalBufferInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO};
        VkExternalBufferProperties props{VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES};
        GetPhysicalDeviceExternalBufferProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_memory_properties, props.externalMemoryProperties);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties =
            instance.load("vkGetPhysicalDeviceExternalBufferProperties");
        ASSERT_NE(GetPhysicalDeviceExternalBufferProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalBufferInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO};
        VkExternalBufferProperties props{VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES};
        GetPhysicalDeviceExternalBufferProperties(physical_device, &info, &props);
        // Compare against 'zeroed' out VkExternalMemoryProperties
        ASSERT_EQ(VkExternalMemoryProperties{}, props.externalMemoryProperties);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties =
            instance.load("vkGetPhysicalDeviceExternalBufferProperties");
        ASSERT_NE(GetPhysicalDeviceExternalBufferProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalBufferInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO};
        VkExternalBufferProperties props{VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES};
        GetPhysicalDeviceExternalBufferProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_memory_properties, props.externalMemoryProperties);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceExternalBufferProperties where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevExtBufPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomExtMemoryData(cur_dev.external_memory_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties =
        instance.load("vkGetPhysicalDeviceExternalBufferProperties");
    ASSERT_NE(GetPhysicalDeviceExternalBufferProperties, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkPhysicalDeviceExternalBufferInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO};
                    VkExternalBufferProperties props{VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES};
                    GetPhysicalDeviceExternalBufferProperties(physical_devices[dev], &info, &props);
                    // No driver support for extension or 1.1 for ICD 1, all others support
                    if (icd != 1) {
                        ASSERT_EQ(cur_dev.external_memory_properties, props.externalMemoryProperties);
                    } else {
                        ASSERT_EQ(props.externalMemoryProperties, VkExternalMemoryProperties{});
                    }

                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

//
// VK_KHR_external_semaphore_capabilities
//

// Test vkGetPhysicalDeviceExternalSemaphorePropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevExtSemPropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalSemaphorePropertiesKHR, nullptr);
}

// Test vkGetPhysicalDeviceExternalSemaphorePropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevExtSemPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalSemaphorePropertiesKHR, nullptr);
}

// Fill in random but valid data into the external semaphore data struct for the current physical device
void FillInRandomExtSemData(VkExternalSemaphoreProperties& props) {
    props.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;
    props.pNext = nullptr;
    props.exportFromImportedHandleTypes = static_cast<VkExternalSemaphoreHandleTypeFlags>((rand() % 0xFFF) + 1);
    props.compatibleHandleTypes = static_cast<VkExternalSemaphoreHandleTypeFlags>((rand() % 0xFFF) + 1);
    props.externalSemaphoreFeatures = static_cast<VkExternalSemaphoreFeatureFlags>((rand() % 0xFFF) + 1);
}

// Test vkGetPhysicalDeviceExternalSemaphorePropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevExtSemProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomExtSemData(env.get_test_icd(0).physical_devices.back().external_semaphore_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR GetPhysicalDeviceExternalSemaphorePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceExternalSemaphorePropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceExternalSemaphoreInfoKHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR};
    VkExternalSemaphorePropertiesKHR props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR};
    GetPhysicalDeviceExternalSemaphorePropertiesKHR(physical_device, &info, &props);
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_semaphore_properties, props);
}

// Test vkGetPhysicalDeviceExternalSemaphoreProperties where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevExtSemProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, 0});
    FillInRandomExtSemData(env.get_test_icd(0).physical_devices.back().external_semaphore_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties =
            instance.load("vkGetPhysicalDeviceExternalSemaphoreProperties");
        ASSERT_NE(GetPhysicalDeviceExternalSemaphoreProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalSemaphoreInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO};
        VkExternalSemaphoreProperties props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES};
        GetPhysicalDeviceExternalSemaphoreProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_semaphore_properties, props);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties =
            instance.load("vkGetPhysicalDeviceExternalSemaphoreProperties");
        ASSERT_NE(GetPhysicalDeviceExternalSemaphoreProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalSemaphoreInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO};
        VkExternalSemaphoreProperties props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES};
        GetPhysicalDeviceExternalSemaphoreProperties(physical_device, &info, &props);
        // Compare against 'zeroed' out VkExternalSemaphoreProperties
        ASSERT_EQ(VkExternalSemaphoreProperties{}, props);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties =
            instance.load("vkGetPhysicalDeviceExternalSemaphoreProperties");
        ASSERT_NE(GetPhysicalDeviceExternalSemaphoreProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalSemaphoreInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO};
        VkExternalSemaphoreProperties props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES};
        GetPhysicalDeviceExternalSemaphoreProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_semaphore_properties, props);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}

// Test vkGetPhysicalDeviceExternalSemaphoreProperties where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevExtSemPropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomExtSemData(cur_dev.external_semaphore_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties =
        instance.load("vkGetPhysicalDeviceExternalSemaphoreProperties");
    ASSERT_NE(GetPhysicalDeviceExternalSemaphoreProperties, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkPhysicalDeviceExternalSemaphoreInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO};
                    VkExternalSemaphoreProperties props{VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES};
                    GetPhysicalDeviceExternalSemaphoreProperties(physical_devices[dev], &info, &props);
                    // No driver support for extension or 1.1 for ICD 1, all others support
                    if (icd != 1) {
                        ASSERT_EQ(cur_dev.external_semaphore_properties, props);
                    } else {
                        ASSERT_EQ(props, VkExternalSemaphoreProperties{});
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

//
// VK_KHR_external_fence_capabilities
//

// Test vkGetPhysicalDeviceExternalFencePropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevExtFencePropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR GetPhysicalDeviceExternalFencePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalFencePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalFencePropertiesKHR, nullptr);
}

// Test vkGetPhysicalDeviceExternalFencePropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevExtFencePropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR GetPhysicalDeviceExternalFencePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalFencePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceExternalFencePropertiesKHR, nullptr);
}

// Fill in random but valid data into the external fence data struct for the current physical device
void FillInRandomExtFenceData(VkExternalFenceProperties& props) {
    props.sType = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES;
    props.pNext = nullptr;
    props.exportFromImportedHandleTypes = static_cast<VkExternalFenceHandleTypeFlags>((rand() % 0xFFF) + 1);
    props.compatibleHandleTypes = static_cast<VkExternalFenceHandleTypeFlags>((rand() % 0xFFF) + 1);
    props.externalFenceFeatures = static_cast<VkExternalFenceFeatureFlags>((rand() % 0xFFF) + 1);
}

// Test vkGetPhysicalDeviceExternalFencePropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevExtFenceProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomExtFenceData(env.get_test_icd(0).physical_devices.back().external_fence_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR GetPhysicalDeviceExternalFencePropertiesKHR =
        instance.load("vkGetPhysicalDeviceExternalFencePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceExternalFencePropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkPhysicalDeviceExternalFenceInfoKHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR};
    VkExternalFencePropertiesKHR props{VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES_KHR};
    GetPhysicalDeviceExternalFencePropertiesKHR(physical_device, &info, &props);
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_fence_properties, props);
}

// Test vkGetPhysicalDeviceExternalFenceProperties where instance supports, an ICD, and a device under that ICD
// also support, so everything should work and return properly.
// Also check if the application didn't enable 1.1 and when a layer 'upgrades' the api version to 1.1
TEST(LoaderInstPhysDevExts, PhysDevExtFenceProps2Simple) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).icd_api_version = VK_API_VERSION_1_1;
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME, 0});
    FillInRandomExtFenceData(env.get_test_icd(0).physical_devices.back().external_fence_properties);
    {
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.set_api_version(VK_API_VERSION_1_1);
        instance.CheckCreate();

        PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties =
            instance.load("vkGetPhysicalDeviceExternalFenceProperties");
        ASSERT_NE(GetPhysicalDeviceExternalFenceProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalFenceInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO};
        VkExternalFenceProperties props{VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES};
        GetPhysicalDeviceExternalFenceProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_fence_properties, props);
    }
    {  // Now do the same logic but the application didn't enable 1.0 or the extension so they get the emulated call
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);

        PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties =
            instance.load("vkGetPhysicalDeviceExternalFenceProperties");
        ASSERT_NE(GetPhysicalDeviceExternalFenceProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalFenceInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO};
        VkExternalFenceProperties props{VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES};
        GetPhysicalDeviceExternalFenceProperties(physical_device, &info, &props);
        // Compare against 'zeroed' out VkExternalFenceProperties
        ASSERT_EQ(VkExternalFenceProperties{}, props);
        ASSERT_TRUE(log.find("Emulating call in ICD"));
    }
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("modify_api_version_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DisableEnvVar")),
                           "modify_api_version_layer.json");
    env.get_test_layer().set_alter_api_version(VK_API_VERSION_1_1);
    {  // Now do the same as above but with a layer that updates the version to 1.1 on behalf of the application
        InstWrapper instance(env.vulkan_functions);
        instance.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instance.CheckCreate();
        DebugUtilsWrapper log{instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT};
        CreateDebugUtilsMessenger(log);
        PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties =
            instance.load("vkGetPhysicalDeviceExternalFenceProperties");
        ASSERT_NE(GetPhysicalDeviceExternalFenceProperties, nullptr);

        uint32_t driver_count = 1;
        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
        ASSERT_EQ(driver_count, 1U);

        VkPhysicalDeviceExternalFenceInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO};
        VkExternalFenceProperties props{VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES};
        GetPhysicalDeviceExternalFenceProperties(physical_device, &info, &props);
        ASSERT_EQ(env.get_test_icd(0).physical_devices.back().external_fence_properties, props);
        ASSERT_FALSE(log.find("Emulating call in ICD"));
    }
}
// Test vkGetPhysicalDeviceExternalFenceProperties where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevExtFencePropsMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomExtFenceData(cur_dev.external_fence_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(VK_API_VERSION_1_1);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties =
        instance.load("vkGetPhysicalDeviceExternalFenceProperties");
    ASSERT_NE(GetPhysicalDeviceExternalFenceProperties, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkPhysicalDeviceExternalFenceInfo info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO};
                    VkExternalFenceProperties props{VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES};
                    GetPhysicalDeviceExternalFenceProperties(physical_devices[dev], &info, &props);
                    // No driver support for extension or 1.1 for ICD 1, all others support
                    if (icd != 1) {
                        ASSERT_EQ(cur_dev.external_fence_properties, props);
                    } else {
                        ASSERT_EQ(props, VkExternalFenceProperties{});
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

//
// VK_KHR_get_surface_capabilities2
//

// Test vkGetPhysicalDeviceSurfaceCapabilities2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceCaps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR GetPhysicalDeviceSurfaceCapabilities2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_EQ(GetPhysicalDeviceSurfaceCapabilities2KHR, nullptr);
}

// Test vkGetPhysicalDeviceSurfaceCapabilities2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceCaps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR GetPhysicalDeviceSurfaceCapabilities2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_EQ(GetPhysicalDeviceSurfaceCapabilities2KHR, nullptr);
}

// Fill in random but valid data into the surface capability data struct for the current physical device
void FillInRandomSurfaceCapsData(VkSurfaceCapabilitiesKHR& props) {
    props.minImageCount = (rand() % 0xFFF) + 1;
    props.maxImageCount = (rand() % 0xFFF) + 1;
    props.currentExtent.width = (rand() % 0xFFF) + 1;
    props.currentExtent.height = (rand() % 0xFFF) + 1;
    props.minImageExtent.width = (rand() % 0xFFF) + 1;
    props.minImageExtent.height = (rand() % 0xFFF) + 1;
    props.maxImageExtent.width = (rand() % 0xFFF) + 1;
    props.maxImageExtent.height = (rand() % 0xFFF) + 1;
    props.maxImageArrayLayers = (rand() % 0xFFF) + 1;
    props.supportedTransforms = static_cast<VkSurfaceTransformFlagsKHR>((rand() % 0xFFF) + 1);
    props.currentTransform = static_cast<VkSurfaceTransformFlagBitsKHR>((rand() % 0xFFF) + 1);
    props.supportedCompositeAlpha = static_cast<VkCompositeAlphaFlagsKHR>((rand() % 0xFFF) + 1);
    props.supportedUsageFlags = static_cast<VkImageUsageFlags>((rand() % 0xFFF) + 1);
}

// Test vkGetPhysicalDeviceSurfaceCapabilities2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceCaps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
    Extension third_ext{VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME};
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.add_instance_extensions({first_ext, second_ext, third_ext});
    cur_icd.physical_devices.push_back({});
    cur_icd.min_icd_interface_version = 3;
    cur_icd.enable_icd_wsi = true;
    FillInRandomSurfaceCapsData(env.get_test_icd(0).physical_devices.back().surface_capabilities);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceCapabilitiesKHR, nullptr);
    PFN_vkCreateHeadlessSurfaceEXT CreateHeadlessSurfaceEXT = instance.load("vkCreateHeadlessSurfaceEXT");
    ASSERT_NE(CreateHeadlessSurfaceEXT, nullptr);
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(DestroySurfaceKHR, nullptr);
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR GetPhysicalDeviceSurfaceCapabilities2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceCapabilities2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkSurfaceKHR surface;
    VkHeadlessSurfaceCreateInfoEXT create_info{VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT};
    ASSERT_EQ(VK_SUCCESS, CreateHeadlessSurfaceEXT(instance.inst, &create_info, nullptr, &surface));

    VkSurfaceCapabilitiesKHR props{};
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &props));

    VkPhysicalDeviceSurfaceInfo2KHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, surface};
    VkSurfaceCapabilities2KHR props2{VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &info, &props2));
    ASSERT_EQ(props, props2.surfaceCapabilities);

    DestroySurfaceKHR(instance.inst, surface, nullptr);
}

// Test vkGetPhysicalDeviceSurfaceCapabilities2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevSurfaceCaps2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;
    Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
    Extension third_ext{VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME};

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.min_icd_interface_version = 3;
        cur_icd.enable_icd_wsi = true;
        cur_icd.add_instance_extensions({first_ext, third_ext});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension(second_ext);
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_SURFACE_EXTENSION_NAME, 0});
            cur_dev.extensions.push_back({VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomSurfaceCapsData(cur_dev.surface_capabilities);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceCapabilitiesKHR, nullptr);
    PFN_vkCreateHeadlessSurfaceEXT CreateHeadlessSurfaceEXT = instance.load("vkCreateHeadlessSurfaceEXT");
    ASSERT_NE(CreateHeadlessSurfaceEXT, nullptr);
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(DestroySurfaceKHR, nullptr);
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR GetPhysicalDeviceSurfaceCapabilities2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceCapabilities2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    VkSurfaceKHR surface;
    VkHeadlessSurfaceCreateInfoEXT create_info{VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT};
    ASSERT_EQ(VK_SUCCESS, CreateHeadlessSurfaceEXT(instance.inst, &create_info, nullptr, &surface));

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkSurfaceCapabilitiesKHR props{};
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices[dev], surface, &props));

        VkPhysicalDeviceSurfaceInfo2KHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, surface};
        VkSurfaceCapabilities2KHR props2{VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceCapabilities2KHR(physical_devices[dev], &info, &props2));
        ASSERT_EQ(props, props2.surfaceCapabilities);
    }

    DestroySurfaceKHR(instance.inst, surface, nullptr);
}

// Test vkGetPhysicalDeviceSurfaceFormats2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceFormats2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR GetPhysicalDeviceSurfaceFormats2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormats2KHR");
    ASSERT_EQ(GetPhysicalDeviceSurfaceFormats2KHR, nullptr);
}

// Test vkGetPhysicalDeviceSurfaceFormats2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceFormats2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR GetPhysicalDeviceSurfaceFormats2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormats2KHR");
    ASSERT_EQ(GetPhysicalDeviceSurfaceFormats2KHR, nullptr);
}

// Fill in random but valid data into the surface formats data struct for the current physical device
void FillInRandomSurfaceFormatsData(std::vector<VkSurfaceFormatKHR>& props) {
    props.resize((rand() % 5) + 1);
    for (uint32_t i = 0; i < props.size(); ++i) {
        props[i].format = static_cast<VkFormat>((rand() % 0xFFF) + 1);
        props[i].colorSpace = static_cast<VkColorSpaceKHR>((rand() % 0xFFF) + 1);
    }
}

// Test vkGetPhysicalDeviceSurfaceFormats2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevSurfaceFormats2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
    Extension third_ext{VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME};
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.add_instance_extensions({first_ext, second_ext, third_ext});
    cur_icd.physical_devices.push_back({});
    cur_icd.min_icd_interface_version = 3;
    cur_icd.enable_icd_wsi = true;
    FillInRandomSurfaceFormatsData(env.get_test_icd(0).physical_devices.back().surface_formats);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormatsKHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceFormatsKHR, nullptr);
    PFN_vkCreateHeadlessSurfaceEXT CreateHeadlessSurfaceEXT = instance.load("vkCreateHeadlessSurfaceEXT");
    ASSERT_NE(CreateHeadlessSurfaceEXT, nullptr);
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(DestroySurfaceKHR, nullptr);
    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR GetPhysicalDeviceSurfaceFormats2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormats2KHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceFormats2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkSurfaceKHR surface;
    VkHeadlessSurfaceCreateInfoEXT create_info{VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT};
    ASSERT_EQ(VK_SUCCESS, CreateHeadlessSurfaceEXT(instance.inst, &create_info, nullptr, &surface));

    std::vector<VkSurfaceFormatKHR> formats{};
    uint32_t count_1 = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count_1, nullptr));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().surface_formats.size(), count_1);
    formats.resize(count_1);
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count_1, formats.data()));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().surface_formats.size(), count_1);

    VkPhysicalDeviceSurfaceInfo2KHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, surface};
    std::vector<VkSurfaceFormat2KHR> formats2{};
    uint32_t count_2 = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormats2KHR(physical_device, &info, &count_2, nullptr));
    ASSERT_EQ(count_1, count_2);
    formats2.resize(count_2, VkSurfaceFormat2KHR{VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR});
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormats2KHR(physical_device, &info, &count_2, formats2.data()));
    ASSERT_EQ(formats, formats2);
    DestroySurfaceKHR(instance.inst, surface, nullptr);
}

// Test vkGetPhysicalDeviceSurfaceFormats2 where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevSurfaceFormats2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;
    Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME};
    Extension third_ext{VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME};

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.enable_icd_wsi = true;
        cur_icd.min_icd_interface_version = 3;
        cur_icd.add_instance_extensions({first_ext, third_ext});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension(second_ext);
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_SURFACE_EXTENSION_NAME, 0});
            cur_dev.extensions.push_back({VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomSurfaceFormatsData(cur_dev.surface_formats);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormatsKHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceFormatsKHR, nullptr);
    PFN_vkCreateHeadlessSurfaceEXT CreateHeadlessSurfaceEXT = instance.load("vkCreateHeadlessSurfaceEXT");
    ASSERT_NE(CreateHeadlessSurfaceEXT, nullptr);
    PFN_vkDestroySurfaceKHR DestroySurfaceKHR = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(DestroySurfaceKHR, nullptr);
    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR GetPhysicalDeviceSurfaceFormats2KHR =
        instance.load("vkGetPhysicalDeviceSurfaceFormats2KHR");
    ASSERT_NE(GetPhysicalDeviceSurfaceFormats2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    VkSurfaceKHR surface;
    VkHeadlessSurfaceCreateInfoEXT create_info{VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT};
    ASSERT_EQ(VK_SUCCESS, CreateHeadlessSurfaceEXT(instance.inst, &create_info, nullptr, &surface));

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        std::vector<VkSurfaceFormatKHR> formats{};
        uint32_t count_1 = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormatsKHR(physical_devices[dev], surface, &count_1, nullptr));
        ASSERT_NE(0U, count_1);
        formats.resize(count_1);
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormatsKHR(physical_devices[dev], surface, &count_1, formats.data()));
        ASSERT_NE(0U, count_1);

        VkPhysicalDeviceSurfaceInfo2KHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr, surface};
        std::vector<VkSurfaceFormat2KHR> formats2{};
        uint32_t count_2 = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormats2KHR(physical_devices[dev], &info, &count_2, nullptr));
        ASSERT_EQ(count_1, count_2);
        formats2.resize(count_2, VkSurfaceFormat2KHR{VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR});
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceSurfaceFormats2KHR(physical_devices[dev], &info, &count_2, formats2.data()));
        ASSERT_EQ(count_1, count_2);
        ASSERT_EQ(formats, formats2);
    }

    DestroySurfaceKHR(instance.inst, surface, nullptr);
}

//
// VK_KHR_display
//

// Test vkGetPhysicalDeviceDisplayPropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevDispPropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);
}

// Test vkGetPhysicalDeviceDisplayPropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevDispPropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);
}

VkDisplayKHR CreateRandomDisplay() { return (VkDisplayKHR)(((rand() % 0xFFFFFFFBull) << 12) * (rand() % 0xFFFFFFFull) + 1); }

VkDisplayModeKHR CreateRandomDisplayMode() {
    return (VkDisplayModeKHR)(((rand() % 0xFFFFFFFBull) << 12) * (rand() % 0xFFFFFFFull) + 1);
}

// Fill in random but valid data into the display property data struct for the current physical device
void FillInRandomDisplayPropData(std::vector<VkDisplayPropertiesKHR>& props) {
    props.resize((rand() % 5) + 1);
    for (uint32_t i = 0; i < props.size(); ++i) {
        props[i].display = CreateRandomDisplay();
        props[i].physicalDimensions.width = static_cast<uint32_t>((rand() % 0xFFF) + 1);
        props[i].physicalDimensions.height = static_cast<uint32_t>((rand() % 0xFFF) + 1);
        props[i].physicalResolution.width = static_cast<uint32_t>((rand() % 0xFFF) + 1);
        props[i].physicalResolution.height = static_cast<uint32_t>((rand() % 0xFFF) + 1);
        props[i].supportedTransforms = static_cast<VkSurfaceTransformFlagsKHR>((rand() % 0xFFE) + 1);
        props[i].planeReorderPossible = rand() % 2 > 0 ? VK_TRUE : VK_FALSE;
        props[i].persistentContent = rand() % 2 > 0 ? VK_TRUE : VK_FALSE;
    }
}

// Test vGetPhysicalDeviceDisplayPropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevDispPropsKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDisplayPropData(env.get_test_icd(0).physical_devices.back().display_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayPropertiesKHR> props{};
    uint32_t prop_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &prop_count, nullptr));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_properties.size(), prop_count);
    props.resize(prop_count);
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &prop_count, props.data()));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_properties.size(), prop_count);

    ASSERT_EQ(props, env.get_test_icd(0).physical_devices.back().display_properties);
}

// Test vkGetPhysicalDeviceDisplayPropertiesKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevDispPropsKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomDisplayPropData(cur_dev.display_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    std::vector<VkDisplayPropertiesKHR> props{};
                    uint32_t prop_count = 0;
                    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_devices[dev], &prop_count, nullptr));
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(0U, prop_count);
                    } else {
                        ASSERT_EQ(cur_dev.display_properties.size(), prop_count);
                        props.resize(prop_count);
                        ASSERT_EQ(VK_SUCCESS,
                                  GetPhysicalDeviceDisplayPropertiesKHR(physical_devices[dev], &prop_count, props.data()));
                        ASSERT_EQ(cur_dev.display_properties.size(), prop_count);

                        ASSERT_EQ(props, cur_dev.display_properties);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkGetPhysicalDeviceDisplayPlanePropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevDispPlanePropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);
}

// Test vkGetPhysicalDeviceDisplayPlanePropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevDispPlanePropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);
}

// Fill in random but valid data into the display plane property data struct for the current physical device
void FillInRandomDisplayPlanePropData(std::vector<VkDisplayPlanePropertiesKHR>& props) {
    props.resize((rand() % 5) + 1);
    for (uint32_t i = 0; i < props.size(); ++i) {
        props[i].currentDisplay = CreateRandomDisplay();
        props[i].currentStackIndex = static_cast<uint32_t>((rand() % 0xFFF) + (rand() % 0xFFF) + 1);
    }
}

// Test vGetPhysicalDeviceDisplayPlanePropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevDispPlanePropsKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDisplayPlanePropData(env.get_test_icd(0).physical_devices.back().display_plane_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayPlanePropertiesKHR> props{};
    uint32_t prop_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &prop_count, nullptr));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_plane_properties.size(), prop_count);
    props.resize(prop_count);
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &prop_count, props.data()));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_plane_properties.size(), prop_count);

    ASSERT_EQ(props, env.get_test_icd(0).physical_devices.back().display_plane_properties);
}

// Test vkGetPhysicalDeviceDisplayPlanePropertiesKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevDispPlanePropsKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomDisplayPlanePropData(cur_dev.display_plane_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    std::vector<VkDisplayPlanePropertiesKHR> props{};
                    uint32_t prop_count = 0;
                    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_devices[dev], &prop_count, nullptr));
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(0U, prop_count);
                    } else {
                        ASSERT_EQ(cur_dev.display_plane_properties.size(), prop_count);
                        props.resize(prop_count);
                        ASSERT_EQ(VK_SUCCESS,
                                  GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_devices[dev], &prop_count, props.data()));
                        ASSERT_EQ(cur_dev.display_plane_properties.size(), prop_count);

                        ASSERT_EQ(props, cur_dev.display_plane_properties);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkGetDisplayPlaneSupportedDisplaysKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispPlaneSupDispsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR =
        instance.load("vkGetDisplayPlaneSupportedDisplaysKHR");
    ASSERT_EQ(GetDisplayPlaneSupportedDisplaysKHR, nullptr);
}

// Test vkGetDisplayPlaneSupportedDisplaysKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispPlaneSupDispsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR =
        instance.load("vkGetDisplayPlaneSupportedDisplaysKHR");
    ASSERT_EQ(GetDisplayPlaneSupportedDisplaysKHR, nullptr);
}

// Fill in random but valid data into the display plane property data struct for the current physical device
void GenerateRandomDisplays(std::vector<VkDisplayKHR>& disps) {
    disps.resize((rand() % 5) + 1);
    for (uint32_t i = 0; i < disps.size(); ++i) {
        disps[i] = CreateRandomDisplay();
    }
}

// Test vGetDisplayPlaneSupportedDisplaysKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispPlaneSupDispsKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplays(env.get_test_icd(0).physical_devices.back().displays);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR =
        instance.load("vkGetDisplayPlaneSupportedDisplaysKHR");
    ASSERT_NE(GetDisplayPlaneSupportedDisplaysKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayKHR> disps{};
    uint32_t disp_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneSupportedDisplaysKHR(physical_device, 0, &disp_count, nullptr));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().displays.size(), disp_count);
    disps.resize(disp_count);
    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneSupportedDisplaysKHR(physical_device, 0, &disp_count, disps.data()));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().displays.size(), disp_count);

    ASSERT_EQ(disps, env.get_test_icd(0).physical_devices.back().displays);
}

// Test vkGetDisplayPlaneSupportedDisplaysKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispPlaneSupDispsKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplays(cur_dev.displays);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR =
        instance.load("vkGetDisplayPlaneSupportedDisplaysKHR");
    ASSERT_NE(GetDisplayPlaneSupportedDisplaysKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    std::vector<VkDisplayKHR> disps{};
                    uint32_t disp_count = 0;
                    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneSupportedDisplaysKHR(physical_devices[dev], 0, &disp_count, nullptr));
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(0U, disp_count);
                    } else {
                        ASSERT_EQ(cur_dev.displays.size(), disp_count);
                        disps.resize(disp_count);
                        ASSERT_EQ(VK_SUCCESS,
                                  GetDisplayPlaneSupportedDisplaysKHR(physical_devices[dev], 0, &disp_count, disps.data()));
                        ASSERT_EQ(cur_dev.displays.size(), disp_count);

                        ASSERT_EQ(disps, cur_dev.displays);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkGetDisplayModePropertiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispModePropsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_EQ(GetDisplayModePropertiesKHR, nullptr);
}

// Test vkGetDisplayModePropertiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispModePropsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_EQ(GetDisplayModePropertiesKHR, nullptr);
}

// Fill in random but valid data into the display mode properties data struct for the current physical device
void GenerateRandomDisplayModeProps(std::vector<VkDisplayModePropertiesKHR>& disps) {
    disps.resize((rand() % 5) + 1);
    for (uint32_t i = 0; i < disps.size(); ++i) {
        disps[i].displayMode = CreateRandomDisplayMode();
        disps[i].parameters.visibleRegion.width = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
        disps[i].parameters.visibleRegion.height = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
        disps[i].parameters.refreshRate = 1 << (rand() % 8);
    }
}

// Test vGetDisplayModePropertiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispModePropsKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplayModeProps(env.get_test_icd(0).physical_devices.back().display_mode_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_NE(GetDisplayModePropertiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayModePropertiesKHR> props{};
    uint32_t props_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_device, VK_NULL_HANDLE, &props_count, nullptr));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_mode_properties.size(), props_count);
    props.resize(props_count);
    ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_device, VK_NULL_HANDLE, &props_count, props.data()));
    ASSERT_EQ(env.get_test_icd(0).physical_devices.back().display_mode_properties.size(), props_count);

    ASSERT_EQ(props, env.get_test_icd(0).physical_devices.back().display_mode_properties);
}

// Test vkGetDisplayModePropertiesKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispModePropsKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplayModeProps(cur_dev.display_mode_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_NE(GetDisplayModePropertiesKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    uint32_t props_count = 0;
                    ASSERT_EQ(VK_SUCCESS,
                              GetDisplayModePropertiesKHR(physical_devices[dev], VK_NULL_HANDLE, &props_count, nullptr));
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(0U, props_count);
                    } else {
                        std::vector<VkDisplayModePropertiesKHR> props{};
                        ASSERT_EQ(cur_dev.display_mode_properties.size(), props_count);
                        props.resize(props_count);
                        ASSERT_EQ(VK_SUCCESS,
                                  GetDisplayModePropertiesKHR(physical_devices[dev], VK_NULL_HANDLE, &props_count, props.data()));
                        ASSERT_EQ(cur_dev.display_mode_properties.size(), props_count);

                        ASSERT_EQ(props, cur_dev.display_mode_properties);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkCreateDisplayModeKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispModesKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR = instance.load("vkCreateDisplayModeKHR");
    ASSERT_EQ(CreateDisplayModeKHR, nullptr);
}

// Test vkCreateDisplayModeKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispModesKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR = instance.load("vkCreateDisplayModeKHR");
    ASSERT_EQ(CreateDisplayModeKHR, nullptr);
}

// Test vkCreateDisplayModeKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispModesKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    env.get_test_icd(0).physical_devices.back().display_mode = CreateRandomDisplayMode();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR = instance.load("vkCreateDisplayModeKHR");
    ASSERT_NE(CreateDisplayModeKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkDisplayModeKHR mode{};
    VkDisplayModeCreateInfoKHR create_info{VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, CreateDisplayModeKHR(physical_device, VK_NULL_HANDLE, &create_info, nullptr, &mode));
    ASSERT_EQ(mode, env.get_test_icd(0).physical_devices.back().display_mode);
}

// Test vkCreateDisplayModeKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispModesKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            cur_dev.display_mode = CreateRandomDisplayMode();
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR = instance.load("vkCreateDisplayModeKHR");
    ASSERT_NE(CreateDisplayModeKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkDisplayModeKHR mode{};
                    VkDisplayModeCreateInfoKHR create_info{VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR};
                    if (icd == 1) {
                        // Unsupported ICD should return initialization failed (instead of crash)
                        ASSERT_EQ(VK_ERROR_INITIALIZATION_FAILED,
                                  CreateDisplayModeKHR(physical_devices[dev], VK_NULL_HANDLE, &create_info, nullptr, &mode));
                    } else {
                        ASSERT_EQ(VK_SUCCESS,
                                  CreateDisplayModeKHR(physical_devices[dev], VK_NULL_HANDLE, &create_info, nullptr, &mode));
                        ASSERT_EQ(mode, cur_dev.display_mode);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkGetDisplayPlaneCapabilitiesKHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispPlaneCapsKHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_EQ(GetDisplayPlaneCapabilitiesKHR, nullptr);
}

// Test vkGetDisplayPlaneCapabilitiesKHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispPlaneCapsKHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_EQ(GetDisplayPlaneCapabilitiesKHR, nullptr);
}

// Fill in random but valid data into the display plane caps for the current physical device
void GenerateRandomDisplayPlaneCaps(VkDisplayPlaneCapabilitiesKHR& caps) {
    caps.supportedAlpha = static_cast<VkDisplayPlaneAlphaFlagsKHR>((rand() % 0xFFFFFFF) + 1);
    caps.minSrcPosition.x = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minSrcPosition.y = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxSrcPosition.x = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxSrcPosition.y = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minSrcExtent.width = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minSrcExtent.height = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxSrcExtent.width = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxSrcExtent.height = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minDstPosition.x = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minDstPosition.y = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxDstPosition.x = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxDstPosition.y = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minDstExtent.width = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.minDstExtent.height = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxDstExtent.width = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
    caps.maxDstExtent.height = static_cast<uint32_t>((rand() % 0xFFFFFFF) + 1);
}

// Test vkGetDisplayPlaneCapabilitiesKHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispPlaneCapsKHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplayPlaneCaps(env.get_test_icd(0).physical_devices.back().display_plane_capabilities);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_NE(GetDisplayPlaneCapabilitiesKHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkDisplayPlaneCapabilitiesKHR caps{};
    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilitiesKHR(physical_device, 0, 0, &caps));
    ASSERT_EQ(caps, env.get_test_icd(0).physical_devices.back().display_plane_capabilities);
}

// Test vkGetDisplayPlaneCapabilitiesKHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispPlaneCapsKHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplayPlaneCaps(cur_dev.display_plane_capabilities);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_NE(GetDisplayPlaneCapabilitiesKHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkDisplayPlaneCapabilitiesKHR caps{};
                    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilitiesKHR(physical_devices[dev], 0, 0, &caps));
                    if (icd != 1) {
                        ASSERT_EQ(caps, cur_dev.display_plane_capabilities);
                    } else {
                        ASSERT_EQ(caps, VkDisplayPlaneCapabilitiesKHR{});
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

//
// VK_KHR_get_display_properties2
//

// Test vkGetPhysicalDeviceDisplayProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevDispProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayProperties2KHR GetPhysicalDeviceDisplayProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayProperties2KHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayProperties2KHR, nullptr);
}

// Test vkGetPhysicalDeviceDisplayProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevDispProps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceDisplayProperties2KHR GetPhysicalDeviceDisplayProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayProperties2KHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayProperties2KHR, nullptr);
}

// Test vGetPhysicalDeviceDisplayProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevDispProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDisplayPropData(env.get_test_icd(0).physical_devices.back().display_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);
    PFN_vkGetPhysicalDeviceDisplayProperties2KHR GetPhysicalDeviceDisplayProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayProperties2KHR");
    ASSERT_NE(GetPhysicalDeviceDisplayProperties2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayPropertiesKHR> props{};
    uint32_t prop_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &prop_count, nullptr));
    ASSERT_NE(0U, prop_count);
    props.resize(prop_count);
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_device, &prop_count, props.data()));

    std::vector<VkDisplayProperties2KHR> props2{};
    uint32_t prop_count_2 = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayProperties2KHR(physical_device, &prop_count_2, nullptr));
    ASSERT_EQ(prop_count, prop_count_2);
    props2.resize(prop_count_2, {VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR});
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayProperties2KHR(physical_device, &prop_count_2, props2.data()));
    ASSERT_EQ(prop_count, prop_count_2);
    ASSERT_EQ(props, props2);
}

// Test vkGetPhysicalDeviceDisplayProperties2KHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevDispProps2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomDisplayPropData(cur_dev.display_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPropertiesKHR, nullptr);
    PFN_vkGetPhysicalDeviceDisplayProperties2KHR GetPhysicalDeviceDisplayProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayProperties2KHR");
    ASSERT_NE(GetPhysicalDeviceDisplayProperties2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        std::vector<VkDisplayPropertiesKHR> props{};
        uint32_t prop_count = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_devices[dev], &prop_count, nullptr));
        ASSERT_NE(0U, prop_count);
        props.resize(prop_count);
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPropertiesKHR(physical_devices[dev], &prop_count, props.data()));

        std::vector<VkDisplayProperties2KHR> props2{};
        uint32_t prop_count_2 = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayProperties2KHR(physical_devices[dev], &prop_count_2, nullptr));
        ASSERT_EQ(prop_count, prop_count_2);
        props2.resize(prop_count_2, {VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR});
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayProperties2KHR(physical_devices[dev], &prop_count_2, props2.data()));
        ASSERT_EQ(prop_count, prop_count_2);
        ASSERT_EQ(props, props2);
    }
}

// Test vkGetPhysicalDeviceDisplayPlaneProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, PhysDevDispPlaneProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR GetPhysicalDeviceDisplayPlaneProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPlaneProperties2KHR, nullptr);
}

// Test vkGetPhysicalDeviceDisplayPlaneProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, PhysDevDispPlaneProps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR GetPhysicalDeviceDisplayPlaneProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    ASSERT_EQ(GetPhysicalDeviceDisplayPlaneProperties2KHR, nullptr);
}

// Test vGetPhysicalDeviceDisplayPlaneProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, PhysDevDispPlaneProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDisplayPlanePropData(env.get_test_icd(0).physical_devices.back().display_plane_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);
    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR GetPhysicalDeviceDisplayPlaneProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlaneProperties2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayPlanePropertiesKHR> props{};
    uint32_t prop_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &prop_count, nullptr));
    ASSERT_NE(0U, prop_count);
    props.resize(prop_count);
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_device, &prop_count, props.data()));

    std::vector<VkDisplayPlaneProperties2KHR> props2{};
    uint32_t prop_count2 = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlaneProperties2KHR(physical_device, &prop_count2, nullptr));
    ASSERT_EQ(prop_count, prop_count2);
    props2.resize(prop_count2, {VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR});
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlaneProperties2KHR(physical_device, &prop_count2, props2.data()));
    ASSERT_EQ(props, props2);
}

// Test vkGetPhysicalDeviceDisplayPlaneProperties2KHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, PhysDevDispPlaneProps2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            FillInRandomDisplayPlanePropData(cur_dev.display_plane_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR =
        instance.load("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlanePropertiesKHR, nullptr);
    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR GetPhysicalDeviceDisplayPlaneProperties2KHR =
        instance.load("vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    ASSERT_NE(GetPhysicalDeviceDisplayPlaneProperties2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        std::vector<VkDisplayPlanePropertiesKHR> props{};
        uint32_t prop_count = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_devices[dev], &prop_count, nullptr));
        ASSERT_NE(0U, prop_count);
        props.resize(prop_count);
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlanePropertiesKHR(physical_devices[dev], &prop_count, props.data()));

        std::vector<VkDisplayPlaneProperties2KHR> props2{};
        uint32_t prop_count2 = 0;
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlaneProperties2KHR(physical_devices[dev], &prop_count2, nullptr));
        ASSERT_EQ(prop_count, prop_count2);
        props2.resize(prop_count2, {VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR});
        ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceDisplayPlaneProperties2KHR(physical_devices[dev], &prop_count2, props2.data()));
        ASSERT_EQ(props, props2);
    }
}

// Test vkGetDisplayModeProperties2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispModeProps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDisplayModeProperties2KHR GetDisplayModeProperties2KHR = instance.load("vkGetDisplayModeProperties2KHR");
    ASSERT_EQ(GetDisplayModeProperties2KHR, nullptr);
}

// Test vkGetDisplayModeProperties2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispModeProps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDisplayModeProperties2KHR GetDisplayModeProperties2KHR = instance.load("vkGetDisplayModeProperties2KHR");
    ASSERT_EQ(GetDisplayModeProperties2KHR, nullptr);
}

// Test vGetDisplayModeProperties2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispModeProps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplayModeProps(env.get_test_icd(0).physical_devices.back().display_mode_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_NE(GetDisplayModePropertiesKHR, nullptr);
    PFN_vkGetDisplayModeProperties2KHR GetDisplayModeProperties2KHR = instance.load("vkGetDisplayModeProperties2KHR");
    ASSERT_NE(GetDisplayModeProperties2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    std::vector<VkDisplayModePropertiesKHR> props{};
    uint32_t props_count1 = 0;
    ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_device, VK_NULL_HANDLE, &props_count1, nullptr));
    ASSERT_NE(0U, props_count1);
    props.resize(props_count1);
    ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_device, VK_NULL_HANDLE, &props_count1, props.data()));

    std::vector<VkDisplayModeProperties2KHR> props2{};
    uint32_t props_count2 = 0;
    ASSERT_EQ(VK_SUCCESS, GetDisplayModeProperties2KHR(physical_device, VK_NULL_HANDLE, &props_count2, nullptr));
    ASSERT_EQ(props_count1, props_count2);
    props2.resize(props_count2, {VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR});
    ASSERT_EQ(VK_SUCCESS, GetDisplayModeProperties2KHR(physical_device, VK_NULL_HANDLE, &props_count2, props2.data()));
    ASSERT_EQ(props, props2);
}

// Test vkGetDisplayModeProperties2KHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispModeProps2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplayModeProps(cur_dev.display_mode_properties);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR = instance.load("vkGetDisplayModePropertiesKHR");
    ASSERT_NE(GetDisplayModePropertiesKHR, nullptr);
    PFN_vkGetDisplayModeProperties2KHR GetDisplayModeProperties2KHR = instance.load("vkGetDisplayModeProperties2KHR");
    ASSERT_NE(GetDisplayModeProperties2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        std::vector<VkDisplayModePropertiesKHR> props{};
        uint32_t props_count1 = 0;
        ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_devices[dev], VK_NULL_HANDLE, &props_count1, nullptr));
        ASSERT_NE(0U, props_count1);
        props.resize(props_count1);
        ASSERT_EQ(VK_SUCCESS, GetDisplayModePropertiesKHR(physical_devices[dev], VK_NULL_HANDLE, &props_count1, props.data()));

        std::vector<VkDisplayModeProperties2KHR> props2{};
        uint32_t props_count2 = 0;
        ASSERT_EQ(VK_SUCCESS, GetDisplayModeProperties2KHR(physical_devices[dev], VK_NULL_HANDLE, &props_count2, nullptr));
        ASSERT_EQ(props_count1, props_count2);
        props2.resize(props_count2, {VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR});
        ASSERT_EQ(VK_SUCCESS, GetDisplayModeProperties2KHR(physical_devices[dev], VK_NULL_HANDLE, &props_count2, props2.data()));
        ASSERT_EQ(props, props2);
    }
}

// Test vkGetDisplayPlaneCapabilities2KHR where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDispPlaneCaps2KHRNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_EQ(GetDisplayPlaneCapabilitiesKHR, nullptr);
}

// Test vkGetDisplayPlaneCapabilities2KHR where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDispPlaneCaps2KHRNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_EQ(GetDisplayPlaneCapabilitiesKHR, nullptr);
}

// Test vkGetDisplayPlaneCapabilities2KHR where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDispPlaneCaps2KHRInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    FillInRandomDisplayPropData(env.get_test_icd(0).physical_devices.back().display_properties);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_NE(GetDisplayPlaneCapabilitiesKHR, nullptr);
    PFN_vkGetDisplayPlaneCapabilities2KHR GetDisplayPlaneCapabilities2KHR = instance.load("vkGetDisplayPlaneCapabilities2KHR");
    ASSERT_NE(GetDisplayPlaneCapabilities2KHR, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkDisplayPlaneCapabilitiesKHR caps{};
    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilitiesKHR(physical_device, 0, 0, &caps));
    VkDisplayPlaneCapabilities2KHR caps2{};
    VkDisplayPlaneInfo2KHR info{VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR};
    ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilities2KHR(physical_device, &info, &caps2));
    ASSERT_EQ(caps, caps2);
}

// Test vkGetDisplayPlaneCapabilities2KHR where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDispPlaneCaps2KHRMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplayPlaneCaps(cur_dev.display_plane_capabilities);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR = instance.load("vkGetDisplayPlaneCapabilitiesKHR");
    ASSERT_NE(GetDisplayPlaneCapabilitiesKHR, nullptr);
    PFN_vkGetDisplayPlaneCapabilities2KHR GetDisplayPlaneCapabilities2KHR = instance.load("vkGetDisplayPlaneCapabilities2KHR");
    ASSERT_NE(GetDisplayPlaneCapabilities2KHR, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkDisplayPlaneCapabilitiesKHR caps{};
        ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilitiesKHR(physical_devices[dev], 0, 0, &caps));
        VkDisplayPlaneCapabilities2KHR caps2{};
        VkDisplayPlaneInfo2KHR info{VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR};
        ASSERT_EQ(VK_SUCCESS, GetDisplayPlaneCapabilities2KHR(physical_devices[dev], &info, &caps2));
        ASSERT_EQ(caps, caps2);
    }
}

//
// VK_EXT_acquire_drm_display
//

// Test vkAcquireDrmDisplayEXT where nothing supports it.
TEST(LoaderInstPhysDevExts, AcquireDrmDisplayEXTNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkAcquireDrmDisplayEXT AcquireDrmDisplayEXT = instance.load("vkAcquireDrmDisplayEXT");
    ASSERT_EQ(AcquireDrmDisplayEXT, nullptr);
}

// Test vkAcquireDrmDisplayEXT where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, AcquireDrmDisplayEXTNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkAcquireDrmDisplayEXT AcquireDrmDisplayEXT = instance.load("vkAcquireDrmDisplayEXT");
    ASSERT_EQ(AcquireDrmDisplayEXT, nullptr);
}

// Test vkAcquireDrmDisplayEXT where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, AcquireDrmDisplayEXTInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplays(env.get_test_icd(0).physical_devices.back().displays);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkAcquireDrmDisplayEXT AcquireDrmDisplayEXT = instance.load("vkAcquireDrmDisplayEXT");
    ASSERT_NE(AcquireDrmDisplayEXT, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkDisplayKHR display = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, AcquireDrmDisplayEXT(physical_device, 0, display));
}

// Test vkAcquireDrmDisplayEXT where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, AcquireDrmDisplayEXTMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplays(cur_dev.displays);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkAcquireDrmDisplayEXT AcquireDrmDisplayEXT = instance.load("vkAcquireDrmDisplayEXT");
    ASSERT_NE(AcquireDrmDisplayEXT, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkDisplayKHR display = VK_NULL_HANDLE;
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(VK_ERROR_EXTENSION_NOT_PRESENT, AcquireDrmDisplayEXT(physical_devices[dev], 0, display));
                    } else {
                        ASSERT_EQ(VK_SUCCESS, AcquireDrmDisplayEXT(physical_devices[dev], 0, display));
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

// Test vkGetDrmDisplayEXT where nothing supports it.
TEST(LoaderInstPhysDevExts, GetDrmDisplayEXTNoSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    PFN_vkGetDrmDisplayEXT GetDrmDisplayEXT = instance.load("vkGetDrmDisplayEXT");
    ASSERT_EQ(GetDrmDisplayEXT, nullptr);
}

// Test vkGetDrmDisplayEXT where instance supports it, but nothing else.
TEST(LoaderInstPhysDevExts, GetDrmDisplayEXTNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    env.get_test_icd(0).physical_devices.push_back({});

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME);
    instance.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    PFN_vkGetDrmDisplayEXT GetDrmDisplayEXT = instance.load("vkGetDrmDisplayEXT");
    ASSERT_EQ(GetDrmDisplayEXT, nullptr);
}

// Test vkGetDrmDisplayEXT where instance and ICD supports it, but device does not support it.
TEST(LoaderInstPhysDevExts, GetDrmDisplayEXTInstanceAndICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    Extension first_ext{VK_KHR_DISPLAY_EXTENSION_NAME};
    Extension second_ext{VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME};
    env.get_test_icd(0).add_instance_extensions({first_ext, second_ext});
    env.get_test_icd(0).physical_devices.push_back({});
    GenerateRandomDisplays(env.get_test_icd(0).physical_devices.back().displays);

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDrmDisplayEXT GetDrmDisplayEXT = instance.load("vkGetDrmDisplayEXT");
    ASSERT_NE(GetDrmDisplayEXT, nullptr);

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    VkDisplayKHR display = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, GetDrmDisplayEXT(physical_device, 0, 0, &display));
    ASSERT_EQ(display, env.get_test_icd(0).physical_devices.back().displays[0]);
}

// Test vkGetDrmDisplayEXT where instance supports it with some ICDs that both support
// and don't support it:
//    ICD 0 supports
//        Physical device 0 does not
//        Physical device 1 does
//        Physical device 2 does not
//    ICD 1 doesn't support
//        Physical device 3 does not
//    ICD 2 supports
//        Physical device 4 does not
//        Physical device 5 does not
//    ICD 3 supports
//        Physical device 6 does
TEST(LoaderInstPhysDevExts, GetDrmDisplayEXTMixed) {
    FrameworkEnvironment env{};
    const uint32_t max_icd_count = 4;
    const uint32_t dev_counts[max_icd_count] = {3, 1, 2, 1};
    const uint32_t max_phys_devs = 7;

    for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, icd != 1 ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extension({VK_KHR_DISPLAY_EXTENSION_NAME});

        // ICD 1 should not have 1.1
        if (icd != 1) {
            cur_icd.icd_api_version = VK_API_VERSION_1_1;
            cur_icd.add_instance_extension({VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
        }

        uint32_t rand_vendor_id;
        uint32_t rand_driver_vers;
        FillInRandomICDInfo(rand_vendor_id, rand_driver_vers);

        for (uint32_t dev = 0; dev < dev_counts[icd]; ++dev) {
            uint32_t device_version = VK_API_VERSION_1_0;
            cur_icd.physical_devices.push_back({});
            auto& cur_dev = cur_icd.physical_devices.back();
            cur_dev.extensions.push_back({VK_KHR_DISPLAY_EXTENSION_NAME, 0});

            // 2nd device in ICD 0 and the one device in ICD 3 support the extension and 1.1
            if ((icd == 0 && dev == 1) || icd == 3) {
                cur_dev.extensions.push_back({VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME, 0});
                device_version = VK_API_VERSION_1_1;
            }

            // Still set physical device properties (so we can determine if device is correct API version)
            FillInRandomDeviceProps(cur_dev.properties, device_version, rand_vendor_id, rand_driver_vers);
            GenerateRandomDisplays(cur_dev.displays);
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_DISPLAY_EXTENSION_NAME, VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME});
    instance.CheckCreate();

    PFN_vkGetDrmDisplayEXT GetDrmDisplayEXT = instance.load("vkGetDrmDisplayEXT");
    ASSERT_NE(GetDrmDisplayEXT, nullptr);

    uint32_t device_count = max_phys_devs;
    std::array<VkPhysicalDevice, max_phys_devs> physical_devices;
    ASSERT_EQ(VK_SUCCESS, instance->vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data()));
    ASSERT_EQ(device_count, max_phys_devs);

    for (uint32_t dev = 0; dev < device_count; ++dev) {
        VkPhysicalDeviceProperties pd_props{};
        instance->vkGetPhysicalDeviceProperties(physical_devices[dev], &pd_props);

        for (uint32_t icd = 0; icd < max_icd_count; ++icd) {
            auto& cur_icd = env.get_test_icd(icd);
            bool found = false;
            for (uint32_t pd = 0; pd < dev_counts[icd]; ++pd) {
                auto& cur_dev = cur_icd.physical_devices[pd];
                // Find the ICD device matching the physical device we're looking at info for so we can compare the
                // physical devices info with the returned info.
                if (cur_dev.properties.apiVersion == pd_props.apiVersion && cur_dev.properties.deviceID == pd_props.deviceID &&
                    cur_dev.properties.deviceType == pd_props.deviceType &&
                    cur_dev.properties.driverVersion == pd_props.driverVersion &&
                    cur_dev.properties.vendorID == pd_props.vendorID) {
                    VkDisplayKHR display = VK_NULL_HANDLE;
                    if (icd == 1) {
                        // For this extension, if no support exists (like for ICD 1), the value of 0 should be returned by the
                        // loader.
                        ASSERT_EQ(VK_ERROR_EXTENSION_NOT_PRESENT, GetDrmDisplayEXT(physical_devices[dev], 0, 0, &display));
                    } else {
                        ASSERT_EQ(VK_SUCCESS, GetDrmDisplayEXT(physical_devices[dev], 0, 0, &display));
                        ASSERT_EQ(display, cur_dev.displays[0]);
                    }
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
}

TEST(LoaderInstPhysDevExts, DifferentInstanceExtensions) {
    FrameworkEnvironment env{};

    // Add 3 drivers each of which supports a different instance extension
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(0).add_instance_extension({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(0).physical_devices.push_back({"pd0"});
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(1).add_instance_extension({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(1).physical_devices.push_back({"pd1"});
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(2).add_instance_extension({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME});
    env.get_test_icd(2).physical_devices.push_back({"pd2"});
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME, 0});

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
                                     VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
                                     VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME});
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();

    const uint32_t expected_device_count = 3;
    auto physical_devices = inst.GetPhysDevs(expected_device_count);

    PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR GetPhysicalDeviceExternalBufferProperties =
        inst.load("vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR GetPhysicalDeviceExternalSemaphoreProperties =
        inst.load("vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR GetPhysicalDeviceExternalFenceProperties =
        inst.load("vkGetPhysicalDeviceExternalFencePropertiesKHR");
    ASSERT_NE(nullptr, GetPhysicalDeviceExternalBufferProperties);
    ASSERT_NE(nullptr, GetPhysicalDeviceExternalSemaphoreProperties);
    ASSERT_NE(nullptr, GetPhysicalDeviceExternalFenceProperties);

    // The above are instance extensions, so shouldn't crash even if only one physical device supports each
    // extension.
    for (uint32_t dev = 0; dev < expected_device_count; ++dev) {
        VkPhysicalDeviceExternalBufferInfo ext_buf_info{};
        VkExternalBufferProperties ext_buf_props{};
        VkPhysicalDeviceExternalSemaphoreInfo ext_sem_info{};
        VkExternalSemaphoreProperties ext_sem_props{};
        VkPhysicalDeviceExternalFenceInfo ext_fence_info{};
        VkExternalFenceProperties ext_fence_props{};
        GetPhysicalDeviceExternalBufferProperties(physical_devices[dev], &ext_buf_info, &ext_buf_props);
        GetPhysicalDeviceExternalSemaphoreProperties(physical_devices[dev], &ext_sem_info, &ext_sem_props);
        GetPhysicalDeviceExternalFenceProperties(physical_devices[dev], &ext_fence_info, &ext_fence_props);
    }
}

TEST(LoaderInstPhysDevExts, DifferentPhysicalDeviceExtensions) {
    FrameworkEnvironment env{};

    // Add 3 drivers each of which supports a different physical device extension
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(0).physical_devices.push_back("pd0");
    env.get_test_icd(0).physical_devices.back().extensions.push_back({VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(1).physical_devices.push_back("pd1");
    env.get_test_icd(1).physical_devices.back().extensions.push_back({VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, 0});

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_0));
    env.get_test_icd(2).physical_devices.push_back("pd2");
    env.get_test_icd(2).physical_devices.back().extensions.push_back({VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, 0});

    DebugUtilsLogger log{VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT};
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();

    const uint32_t expected_device_count = 3;
    auto physical_devices = inst.GetPhysDevs(expected_device_count);

    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
        EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR =
            inst.load("vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT GetPhysicalDeviceMultisamplePropertiesEXT =
        inst.load("vkGetPhysicalDeviceMultisamplePropertiesEXT");
    PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT GetPhysicalDeviceCalibrateableTimeDomainsEXT =
        inst.load("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    ASSERT_NE(nullptr, EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
    ASSERT_NE(nullptr, GetPhysicalDeviceMultisamplePropertiesEXT);
    ASSERT_NE(nullptr, GetPhysicalDeviceCalibrateableTimeDomainsEXT);

    for (uint32_t dev = 0; dev < expected_device_count; ++dev) {
        uint32_t extension_count = 0;
        std::vector<VkExtensionProperties> device_extensions;
        bool supports_query = false;
        bool supports_samples = false;
        bool supports_timestamps = false;
        ASSERT_EQ(VK_SUCCESS,
                  inst->vkEnumerateDeviceExtensionProperties(physical_devices[dev], nullptr, &extension_count, nullptr));
        ASSERT_GT(extension_count, 0U);
        device_extensions.resize(extension_count);
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumerateDeviceExtensionProperties(physical_devices[dev], nullptr, &extension_count,
                                                                         device_extensions.data()));
        for (uint32_t ext = 0; ext < extension_count; ++ext) {
            if (string_eq(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME, &device_extensions[ext].extensionName[0])) {
                supports_query = true;
            }
            if (string_eq(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, &device_extensions[ext].extensionName[0])) {
                supports_samples = true;
            }
            if (string_eq(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME, &device_extensions[ext].extensionName[0])) {
                supports_timestamps = true;
            }
        }

        // For physical device extensions, they should work for devices that support it and crash for those that don't.
        if (supports_query) {
            ASSERT_EQ(VK_SUCCESS, EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_devices[dev], 0, nullptr,
                                                                                                nullptr, nullptr));
        } else {
            ASSERT_DEATH(
                EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_devices[dev], 0, nullptr, nullptr, nullptr),
                "");
            ASSERT_FALSE(
                log.find("ICD associated with VkPhysicalDevice does not support "
                         "EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR"));
        }
        if (supports_samples) {
            GetPhysicalDeviceMultisamplePropertiesEXT(physical_devices[dev], VK_SAMPLE_COUNT_2_BIT, nullptr);
        } else {
            ASSERT_DEATH(GetPhysicalDeviceMultisamplePropertiesEXT(physical_devices[dev], VK_SAMPLE_COUNT_2_BIT, nullptr), "");
            ASSERT_FALSE(
                log.find("ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceMultisamplePropertiesEXT"));
        }
        if (supports_timestamps) {
            ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_devices[dev], nullptr, nullptr));
        } else {
            ASSERT_DEATH(GetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_devices[dev], nullptr, nullptr), "");
            ASSERT_FALSE(
                log.find("ICD associated with VkPhysicalDevice does not support GetPhysicalDeviceCalibrateableTimeDomainsEXT"));
        }
    }
}
