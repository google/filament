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

#include "test_environment.h"

TEST(ICDInterfaceVersion2Plus, vk_icdNegotiateLoaderICDInterfaceVersion) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    for (uint32_t i = 0; i <= 6; i++) {
        for (uint32_t j = i; j <= 6; j++) {
            driver.set_min_icd_interface_version(i).set_max_icd_interface_version(j);
            InstWrapper inst{env.vulkan_functions};
            inst.CheckCreate();
        }
    }
}

TEST(ICDInterfaceVersion2Plus, version_3) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");
    {
        driver.set_min_icd_interface_version(2).set_enable_icd_wsi(true);
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(driver.is_using_icd_wsi);
    }
    {
        driver.set_min_icd_interface_version(3).set_enable_icd_wsi(false);
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(driver.is_using_icd_wsi);
    }
    {
        driver.set_min_icd_interface_version(3).set_enable_icd_wsi(true);
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_TRUE(driver.is_using_icd_wsi);
    }
}

TEST(ICDInterfaceVersion2Plus, version_4) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}

TEST(ICDInterfaceVersion2Plus, l4_icd4) {
    // TODO:
    // ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance calls with apiVersion set to > Vulkan 1.0
    // because both the loader and ICD support interface version <= 4. Otherwise, the ICD should behave as normal.
}
TEST(ICDInterfaceVersion2Plus, l4_icd5) {
    // TODO:
    // ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance calls with apiVersion set to > Vulkan 1.0
    // because the loader is still at interface version <= 4. Otherwise, the ICD should behave as normal.
}
TEST(ICDInterfaceVersion2Plus, l5_icd4) {
    // TODO:
    // Loader will fail with VK_ERROR_INCOMPATIBLE_DRIVER if it can't handle the apiVersion. ICD may pass for all apiVersions,
    // but since its interface is <= 4, it is best if it assumes it needs to do the work of rejecting anything > Vulkan 1.0 and
    // fail with VK_ERROR_INCOMPATIBLE_DRIVER. Otherwise, the ICD should behave as normal.
}
TEST(ICDInterfaceVersion2Plus, l5_icd5) {
    // TODO:
    // Loader will fail with VK_ERROR_INCOMPATIBLE_DRIVER if it can't handle the apiVersion, and ICDs should fail with
    // VK_ERROR_INCOMPATIBLE_DRIVER only if they can not support the specified apiVersion. Otherwise, the ICD should behave as
    // normal.
}

#if defined(WIN32)
// This test makes sure that EnumerateAdapterPhysicalDevices on drivers found in the Khronos/Vulkan/Drivers registry
TEST(ICDInterfaceVersion2PlusEnumerateAdapterPhysicalDevices, version_6_in_drivers_registry) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES));
    driver.physical_devices.emplace_back("physical_device_1");
    driver.physical_devices.emplace_back("physical_device_0");
    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    std::vector<VkPhysicalDevice> physical_device_handles = std::vector<VkPhysicalDevice>(physical_count);

    driver.min_icd_interface_version = 5;

    auto& known_driver = known_driver_list.at(2);  // which drive this test pretends to be
    DXGI_ADAPTER_DESC1 desc1{};
    desc1.AdapterLuid = _LUID{10, 1000};
    desc1.VendorId = known_driver.vendor_id;
    env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    driver.set_adapterLUID(desc1.AdapterLuid);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    ASSERT_TRUE(driver.called_enumerate_adapter_physical_devices);
}
// Make the version_6 driver found through the D3DKMT driver discovery mechanism of the loader
TEST(ICDInterfaceVersion2PlusEnumerateAdapterPhysicalDevices, version_6) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_6, VK_API_VERSION_1_3}.set_discovery_type(ManifestDiscoveryType::null_dir));
    // Version 6 provides a mechanism to allow the loader to sort physical devices.
    // The loader will only attempt to sort physical devices on an ICD if version 6 of the interface is supported.
    // This version provides the vk_icdEnumerateAdapterPhysicalDevices function.
    auto& driver = env.get_test_icd(0);
    driver.physical_devices.emplace_back("physical_device_1");
    driver.physical_devices.emplace_back("physical_device_0");
    uint32_t physical_count = 2;
    uint32_t returned_physical_count = physical_count;
    std::vector<VkPhysicalDevice> physical_device_handles{physical_count};

    driver.min_icd_interface_version = 6;

    auto& known_driver = known_driver_list.at(2);  // which drive this test pretends to be
    DXGI_ADAPTER_DESC1 desc1{};
    desc1.AdapterLuid = _LUID{10, 1000};
    desc1.VendorId = known_driver.vendor_id;
    env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    driver.set_adapterLUID(desc1.AdapterLuid);

    env.platform_shim->add_d3dkmt_adapter(
        D3DKMT_Adapter{0, desc1.AdapterLuid}.add_driver_manifest_path(env.get_icd_manifest_path(0)));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    ASSERT_TRUE(driver.called_enumerate_adapter_physical_devices);

    // Make sure that the loader doesn't write past the the end of the pointer
    auto temp_ptr = std::unique_ptr<int>(new int());
    for (auto& phys_dev : physical_device_handles) {
        phys_dev = reinterpret_cast<VkPhysicalDevice>(temp_ptr.get());
    }

    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    returned_physical_count = 0;
    ASSERT_EQ(VK_INCOMPLETE,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(0U, returned_physical_count);
    for (auto& phys_dev : physical_device_handles) {
        ASSERT_EQ(phys_dev, reinterpret_cast<VkPhysicalDevice>(temp_ptr.get()));
    }
}

// Declare drivers using the D3DKMT driver interface and make sure the loader can find them - but don't export
// EnumerateAdapterPhysicalDevices
TEST(ICDInterfaceVersion2, EnumAdapters2) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA}.set_discovery_type(ManifestDiscoveryType::null_dir));
    InstWrapper inst{env.vulkan_functions};
    driver.physical_devices.emplace_back("physical_device_1");
    driver.physical_devices.emplace_back("physical_device_0");
    uint32_t physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    uint32_t returned_physical_count = static_cast<uint32_t>(driver.physical_devices.size());
    std::vector<VkPhysicalDevice> physical_device_handles = std::vector<VkPhysicalDevice>(physical_count);
    driver.adapterLUID = _LUID{10, 1000};
    env.platform_shim->add_d3dkmt_adapter(D3DKMT_Adapter{0, _LUID{10, 1000}}.add_driver_manifest_path(env.get_icd_manifest_path()));

    inst.CheckCreate();

    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);
    ASSERT_FALSE(driver.called_enumerate_adapter_physical_devices);
}

// Make sure that physical devices are found through EnumerateAdapterPhysicalDevices
// Verify that the handles are correct by calling vkGetPhysicalDeviceProperties with them
TEST(ICDInterfaceVersion2PlusEnumerateAdapterPhysicalDevices, VerifyPhysDevResults) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES, VK_API_VERSION_1_1}
                        .set_discovery_type(ManifestDiscoveryType::null_dir))
            .set_min_icd_interface_version(6)
            .set_icd_api_version(VK_API_VERSION_1_1);
    const std::vector<std::string> physical_device_names = {"physical_device_4", "physical_device_3", "physical_device_2",
                                                            "physical_device_1", "physical_device_0"};
    for (const auto& dev_name : physical_device_names) driver.physical_devices.push_back(dev_name);

    auto& known_driver = known_driver_list.at(2);  // which drive this test pretends to be
    DXGI_ADAPTER_DESC1 desc1{};
    desc1.VendorId = known_driver.vendor_id;
    desc1.AdapterLuid = _LUID{10, 1000};
    env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    driver.set_adapterLUID(desc1.AdapterLuid);

    env.platform_shim->add_d3dkmt_adapter(D3DKMT_Adapter{0, _LUID{10, 1000}}.add_driver_manifest_path(env.get_icd_manifest_path()));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    const size_t phys_dev_count = physical_device_names.size();

    // The test ICD should completely swap the order of devices.
    // Since we can't compare VkPhysicalDevice handles because they will be different per VkInstance, we will
    // compare the property names returned, which should still be equal.

    std::vector<VkPhysicalDevice> adapter_pds{phys_dev_count};
    uint32_t count = static_cast<uint32_t>(adapter_pds.size());
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst, &count, adapter_pds.data()));
    ASSERT_EQ(phys_dev_count, count);

    for (uint32_t dev = 0; dev < phys_dev_count; ++dev) {
        VkPhysicalDeviceProperties props;
        env.vulkan_functions.vkGetPhysicalDeviceProperties(adapter_pds[dev], &props);
        std::string dev_name = props.deviceName;
        // index in reverse
        ASSERT_EQ(dev_name, physical_device_names[physical_device_names.size() - 1 - dev]);
    }
}

// Make sure physical device groups enumerated through EnumerateAdapterPhysicalDevices are properly found
TEST(ICDInterfaceVersion2PlusEnumerateAdapterPhysicalDevices, VerifyGroupResults) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails{TEST_ICD_PATH_VERSION_2_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES, VK_API_VERSION_1_1}
                        .set_discovery_type(ManifestDiscoveryType::null_dir))
            .set_min_icd_interface_version(6)
            .set_icd_api_version(VK_API_VERSION_1_1);
    const std::vector<std::string> physical_device_names = {"physical_device_4", "physical_device_3", "physical_device_2",
                                                            "physical_device_1", "physical_device_0"};
    for (const auto& dev_name : physical_device_names) {
        driver.physical_devices.push_back(dev_name);
    }

    driver.physical_device_groups.emplace_back(driver.physical_devices[0]).use_physical_device(driver.physical_devices[1]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[2]);
    driver.physical_device_groups.emplace_back(driver.physical_devices[3]).use_physical_device(driver.physical_devices[4]);

    auto& known_driver = known_driver_list.at(2);  // which driver this test pretends to be
    DXGI_ADAPTER_DESC1 desc1{};
    desc1.VendorId = known_driver.vendor_id;
    desc1.AdapterLuid = _LUID{10, 1000};
    env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    driver.set_adapterLUID(desc1.AdapterLuid);

    env.platform_shim->add_d3dkmt_adapter(D3DKMT_Adapter{0, _LUID{10, 1000}}.add_driver_manifest_path(env.get_icd_manifest_path()));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // The test ICD should completely swap the order of devices.
    // Since we can't compare VkPhysicalDevice handles because they will be different per VkInstance, we will
    // compare the property names returned, which should still be equal.
    // And, since this is device groups, the groups themselves should also be in reverse order with the devices
    // inside each group in revers order.

    const uint32_t actual_group_count = 3;
    uint32_t count = actual_group_count;
    std::array<VkPhysicalDeviceGroupProperties, actual_group_count> groups{};
    for (uint32_t group = 0; group < actual_group_count; ++group) {
        groups[group].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
    }
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &count, groups.data()));
    ASSERT_EQ(actual_group_count, count);

    size_t cur_device_name_index = physical_device_names.size() - 1;  // start at last index and reverse through it
    for (uint32_t group = 0; group < actual_group_count; ++group) {
        for (uint32_t dev = 0; dev < groups[group].physicalDeviceCount; ++dev) {
            VkPhysicalDeviceProperties props;
            env.vulkan_functions.vkGetPhysicalDeviceProperties(groups[group].physicalDevices[dev], &props);
            std::string dev_name = props.deviceName;
            ASSERT_EQ(dev_name, physical_device_names[cur_device_name_index]);
            cur_device_name_index--;
        }
    }
}

#endif  // defined(WIN32)
TEST(ICDInterfaceVersion7, SingleDriver) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7)).add_physical_device({});
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());
    ASSERT_EQ(driver.interface_version_check, InterfaceVersionCheck::version_is_supported);
}

TEST(ICDInterfaceVersion7, SingleDriverWithoutExportedFunctions) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7_WIHTOUT_EXPORTS)).add_physical_device({});
    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    DeviceWrapper dev{inst};
    dev.CheckCreate(inst.GetPhysDev());
    ASSERT_EQ(driver.interface_version_check, InterfaceVersionCheck::version_is_supported);
}

TEST(MultipleICDConfig, Basic) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    env.get_test_icd(0).physical_devices.emplace_back("physical_device_0");
    env.get_test_icd(1).physical_devices.emplace_back("physical_device_1");
    env.get_test_icd(2).physical_devices.emplace_back("physical_device_2");

    env.get_test_icd(0).physical_devices.at(0).properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    env.get_test_icd(1).physical_devices.at(0).properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    env.get_test_icd(2).physical_devices.at(0).properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_CPU;

    copy_string_to_char_array("dev0", env.get_test_icd(0).physical_devices.at(0).properties.deviceName, VK_MAX_EXTENSION_NAME_SIZE);
    copy_string_to_char_array("dev1", env.get_test_icd(1).physical_devices.at(0).properties.deviceName, VK_MAX_EXTENSION_NAME_SIZE);
    copy_string_to_char_array("dev2", env.get_test_icd(2).physical_devices.at(0).properties.deviceName, VK_MAX_EXTENSION_NAME_SIZE);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    std::array<VkPhysicalDevice, 3> phys_devs_array;
    uint32_t phys_dev_count = 3;
    ASSERT_EQ(env.vulkan_functions.vkEnumeratePhysicalDevices(inst, &phys_dev_count, phys_devs_array.data()), VK_SUCCESS);
    ASSERT_EQ(phys_dev_count, 3U);
    ASSERT_EQ(env.get_test_icd(0).physical_devices.at(0).properties.deviceType, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    ASSERT_EQ(env.get_test_icd(1).physical_devices.at(0).properties.deviceType, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    ASSERT_EQ(env.get_test_icd(2).physical_devices.at(0).properties.deviceType, VK_PHYSICAL_DEVICE_TYPE_CPU);
}

TEST(MultipleDriverConfig, DifferentICDInterfaceVersions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_EXPORT_ICD_GIPA));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));

    TestICD& icd0 = env.get_test_icd(0);
    icd0.physical_devices.emplace_back("physical_device_0");
    icd0.max_icd_interface_version = 1;

    TestICD& icd1 = env.get_test_icd(1);
    icd1.physical_devices.emplace_back("physical_device_1");
    icd1.min_icd_interface_version = 2;
    icd1.max_icd_interface_version = 5;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    std::array<VkPhysicalDevice, 2> phys_devs_array;
    uint32_t phys_dev_count = 2;
    ASSERT_EQ(env.vulkan_functions.vkEnumeratePhysicalDevices(inst, &phys_dev_count, phys_devs_array.data()), VK_SUCCESS);
    ASSERT_EQ(phys_dev_count, 2U);
}

TEST(MultipleDriverConfig, DifferentICDsWithDevices) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_EXPORT_ICD_GIPA));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));

    // Make sure the loader returns all devices from all active ICDs.  Many of the other
    // tests add multiple devices to a single ICD, this just makes sure the loader combines
    // device info across multiple drivers properly.
    TestICD& icd0 = env.get_test_icd(0);
    icd0.physical_devices.emplace_back("physical_device_0");
    icd0.min_icd_interface_version = 5;
    icd0.max_icd_interface_version = 5;

    TestICD& icd1 = env.get_test_icd(1);
    icd1.physical_devices.emplace_back("physical_device_1");
    icd1.physical_devices.emplace_back("physical_device_2");
    icd1.min_icd_interface_version = 5;
    icd1.max_icd_interface_version = 5;

    TestICD& icd2 = env.get_test_icd(2);
    icd2.physical_devices.emplace_back("physical_device_3");
    icd2.min_icd_interface_version = 5;
    icd2.max_icd_interface_version = 5;

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    std::array<VkPhysicalDevice, 4> phys_devs_array;
    uint32_t phys_dev_count = 4;
    ASSERT_EQ(env.vulkan_functions.vkEnumeratePhysicalDevices(inst, &phys_dev_count, phys_devs_array.data()), VK_SUCCESS);
    ASSERT_EQ(phys_dev_count, 4U);
}

TEST(MultipleDriverConfig, DifferentICDsWithDevicesAndGroups) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_EXPORT_ICD_GIPA));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));

    // The loader has to be able to handle drivers that support device groups in combination
    // with drivers that don't support device groups.  When this is the case, the loader needs
    // to take every driver that doesn't support device groups and put each of its devices in
    // a separate group.  Then it combines that information with the drivers that support
    // device groups returned info.

    // ICD 0 :  No 1.1 support (so 1 device will become 1 group in loader)
    TestICD& icd0 = env.get_test_icd(0);
    icd0.physical_devices.emplace_back("physical_device_0");
    icd0.min_icd_interface_version = 5;
    icd0.max_icd_interface_version = 5;
    icd0.set_icd_api_version(VK_API_VERSION_1_0);

    // ICD 1 :  1.1 support (with 1 group with 2 devices)
    TestICD& icd1 = env.get_test_icd(1);
    icd1.physical_devices.emplace_back("physical_device_1").set_api_version(VK_API_VERSION_1_1);
    icd1.physical_devices.emplace_back("physical_device_2").set_api_version(VK_API_VERSION_1_1);
    icd1.physical_device_groups.emplace_back(icd1.physical_devices[0]);
    icd1.physical_device_groups.back().use_physical_device(icd1.physical_devices[1]);
    icd1.min_icd_interface_version = 5;
    icd1.max_icd_interface_version = 5;
    icd1.set_icd_api_version(VK_API_VERSION_1_1);

    // ICD 2 :  No 1.1 support (so 3 devices will become 3 groups in loader)
    TestICD& icd2 = env.get_test_icd(2);
    icd2.physical_devices.emplace_back("physical_device_3");
    icd2.physical_devices.emplace_back("physical_device_4");
    icd2.physical_devices.emplace_back("physical_device_5");
    icd2.min_icd_interface_version = 5;
    icd2.max_icd_interface_version = 5;
    icd2.set_icd_api_version(VK_API_VERSION_1_0);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();

    uint32_t group_count = static_cast<uint32_t>(5);
    uint32_t returned_group_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, nullptr));
    ASSERT_EQ(group_count, returned_group_count);

    std::vector<VkPhysicalDeviceGroupProperties> group_props{};
    group_props.resize(group_count, VkPhysicalDeviceGroupProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, group_props.data()));
    ASSERT_EQ(group_count, returned_group_count);
}

#if defined(WIN32)
// This is testing when there are drivers that support the Windows device adapter sorting mechanism by exporting
// EnumerateAdapterPhysicalDevices and drivers that do not expose that functionality
TEST(MultipleICDConfig, version_5_and_version_6) {
    FrameworkEnvironment env;

    const char* regular_layer_name = "VK_LAYER_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                         .set_disable_environment("DisableMeIfYouCan")),
                           "regular_test_layer.json");

    MockQueueFamilyProperties family_props{{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true};

    uint32_t physical_count = 0;
    for (uint32_t i = 0; i < 3; i++) {
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_ENUMERATE_ADAPTER_PHYSICAL_DEVICES));
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& driver_5 = env.get_test_icd(i * 2 + 1);
        driver_5.set_max_icd_interface_version(5);
        driver_5.set_min_icd_interface_version(5);
        driver_5.setup_WSI();
        driver_5.physical_devices.push_back({});
        driver_5.physical_devices.back().queue_family_properties.push_back(family_props);
        driver_5.physical_devices.push_back({});
        driver_5.physical_devices.back().queue_family_properties.push_back(family_props);
        driver_5.physical_devices.push_back({});
        driver_5.physical_devices.back().queue_family_properties.push_back(family_props);
        physical_count += static_cast<uint32_t>(driver_5.physical_devices.size());

        auto& driver_6 = env.get_test_icd(i * 2);
        driver_6.setup_WSI();
        driver_6.physical_devices.emplace_back("physical_device_0");
        driver_6.physical_devices.back().queue_family_properties.push_back(family_props);
        driver_6.physical_devices.emplace_back("physical_device_1");
        driver_6.physical_devices.back().queue_family_properties.push_back(family_props);
        physical_count += static_cast<uint32_t>(driver_6.physical_devices.size());

        driver_6.set_max_icd_interface_version(6);
        driver_6.set_min_icd_interface_version(5);

        uint32_t driver_index = i % 4;  // which drive this test pretends to be, must stay below 4
        auto& known_driver = known_driver_list.at(driver_index);
        DXGI_ADAPTER_DESC1 desc1{};
        desc1.VendorId = known_driver.vendor_id;
        desc1.AdapterLuid = LUID{100 + i, static_cast<LONG>(100 + i)};
        driver_6.set_adapterLUID(desc1.AdapterLuid);
        env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    }
    uint32_t returned_physical_count = 0;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI();
    inst.CheckCreate();

    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);
    std::vector<VkPhysicalDevice> physical_device_handles{returned_physical_count};
    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_device_handles.data()));
    ASSERT_EQ(physical_count, returned_physical_count);

    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
    for (const auto& handle : physical_device_handles) {
        handle_assert_has_value(handle);

        VkBool32 supported = false;
        EXPECT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(handle, 0, surface, &supported));
    }
    for (uint32_t i = 0; i < 3; i++) {
        auto& driver_6 = env.get_test_icd(i * 2);
        EXPECT_EQ(driver_6.called_enumerate_adapter_physical_devices, true);
    }
}
#endif  // defined(WIN32)

// shim function pointers for 1.3
// Should use autogen for this - it generates 'shim' functions for validation layers, maybe that could be used here.
void test_vkCmdBeginRendering(VkCommandBuffer, const VkRenderingInfo*) {}
void test_vkCmdBindVertexBuffers2(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*, const VkDeviceSize*,
                                  const VkDeviceSize*) {}
void test_vkCmdBlitImage2(VkCommandBuffer, const VkBlitImageInfo2*) {}
void test_vkCmdCopyBuffer2(VkCommandBuffer, const VkCopyBufferInfo2*) {}
void test_vkCmdCopyBufferToImage2(VkCommandBuffer, const VkCopyBufferToImageInfo2*) {}
void test_vkCmdCopyImage2(VkCommandBuffer, const VkCopyImageInfo2*) {}
void test_vkCmdCopyImageToBuffer2(VkCommandBuffer, const VkCopyImageToBufferInfo2*) {}
void test_vkCmdEndRendering(VkCommandBuffer) {}
void test_vkCmdPipelineBarrier2(VkCommandBuffer, const VkDependencyInfo*) {}
void test_vkCmdResetEvent2(VkCommandBuffer, VkEvent, VkPipelineStageFlags2) {}
void test_vkCmdResolveImage2(VkCommandBuffer, const VkResolveImageInfo2*) {}
void test_vkCmdSetCullMode(VkCommandBuffer, VkCullModeFlags) {}
void test_vkCmdSetDepthBiasEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetDepthBoundsTestEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetDepthCompareOp(VkCommandBuffer, VkCompareOp) {}
void test_vkCmdSetDepthTestEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetDepthWriteEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetEvent2(VkCommandBuffer, VkEvent, const VkDependencyInfo*) {}
void test_vkCmdSetFrontFace(VkCommandBuffer, VkFrontFace) {}
void test_vkCmdSetPrimitiveRestartEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetPrimitiveTopology(VkCommandBuffer, VkPrimitiveTopology) {}
void test_vkCmdSetRasterizerDiscardEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetScissorWithCount(VkCommandBuffer, uint32_t, const VkRect2D*) {}
void test_vkCmdSetStencilOp(VkCommandBuffer, VkStencilFaceFlags, VkStencilOp, VkStencilOp, VkStencilOp, VkCompareOp) {}
void test_vkCmdSetStencilTestEnable(VkCommandBuffer, VkBool32) {}
void test_vkCmdSetViewportWithCount(VkCommandBuffer, uint32_t, const VkViewport*) {}
void test_vkCmdWaitEvents2(VkCommandBuffer, uint32_t, const VkEvent*, const VkDependencyInfo*) {}
void test_vkCmdWriteTimestamp2(VkCommandBuffer, VkPipelineStageFlags2, VkQueryPool, uint32_t) {}
VkResult test_vkCreatePrivateDataSlot(VkDevice, const VkPrivateDataSlotCreateInfo*, const VkAllocationCallbacks*,
                                      VkPrivateDataSlot*) {
    return VK_SUCCESS;
}
void test_vkDestroyPrivateDataSlot(VkDevice, VkPrivateDataSlot, const VkAllocationCallbacks*) {}
void test_vkGetDeviceBufferMemoryRequirements(VkDevice, const VkDeviceBufferMemoryRequirements*, VkMemoryRequirements2*) {}
void test_vkGetDeviceImageMemoryRequirements(VkDevice, const VkDeviceImageMemoryRequirements*, VkMemoryRequirements2*) {}
void test_vkGetDeviceImageSparseMemoryRequirements(VkDevice, const VkDeviceImageMemoryRequirements*, uint32_t*,
                                                   VkSparseImageMemoryRequirements2*) {}
void test_vkGetPrivateData(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t*) {}
VkResult test_vkQueueSubmit2(VkQueue, uint32_t, const VkSubmitInfo2*, VkFence) { return VK_SUCCESS; }
VkResult test_vkSetPrivateData(VkDevice, VkObjectType, uint64_t, VkPrivateDataSlot, uint64_t) { return VK_SUCCESS; }

TEST(MinorVersionUpdate, Version1_3) {
    FrameworkEnvironment env{};
    auto& driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    driver.physical_devices.back().known_device_functions = {
        VulkanFunction{"vkCmdBeginRendering", to_vkVoidFunction(test_vkCmdBeginRendering)},
        VulkanFunction{"vkCmdBindVertexBuffers2", to_vkVoidFunction(test_vkCmdBindVertexBuffers2)},
        VulkanFunction{"vkCmdBlitImage2", to_vkVoidFunction(test_vkCmdBlitImage2)},
        VulkanFunction{"vkCmdCopyBuffer2", to_vkVoidFunction(test_vkCmdCopyBuffer2)},
        VulkanFunction{"vkCmdCopyBufferToImage2", to_vkVoidFunction(test_vkCmdCopyBufferToImage2)},
        VulkanFunction{"vkCmdCopyImage2", to_vkVoidFunction(test_vkCmdCopyImage2)},
        VulkanFunction{"vkCmdCopyImageToBuffer2", to_vkVoidFunction(test_vkCmdCopyImageToBuffer2)},
        VulkanFunction{"vkCmdEndRendering", to_vkVoidFunction(test_vkCmdEndRendering)},
        VulkanFunction{"vkCmdPipelineBarrier2", to_vkVoidFunction(test_vkCmdPipelineBarrier2)},
        VulkanFunction{"vkCmdResetEvent2", to_vkVoidFunction(test_vkCmdResetEvent2)},
        VulkanFunction{"vkCmdResolveImage2", to_vkVoidFunction(test_vkCmdResolveImage2)},
        VulkanFunction{"vkCmdSetCullMode", to_vkVoidFunction(test_vkCmdSetCullMode)},
        VulkanFunction{"vkCmdSetDepthBiasEnable", to_vkVoidFunction(test_vkCmdSetDepthBiasEnable)},
        VulkanFunction{"vkCmdSetDepthBoundsTestEnable", to_vkVoidFunction(test_vkCmdSetDepthBoundsTestEnable)},
        VulkanFunction{"vkCmdSetDepthCompareOp", to_vkVoidFunction(test_vkCmdSetDepthCompareOp)},
        VulkanFunction{"vkCmdSetDepthTestEnable", to_vkVoidFunction(test_vkCmdSetDepthTestEnable)},
        VulkanFunction{"vkCmdSetDepthWriteEnable", to_vkVoidFunction(test_vkCmdSetDepthWriteEnable)},
        VulkanFunction{"vkCmdSetEvent2", to_vkVoidFunction(test_vkCmdSetEvent2)},
        VulkanFunction{"vkCmdSetFrontFace", to_vkVoidFunction(test_vkCmdSetFrontFace)},
        VulkanFunction{"vkCmdSetPrimitiveRestartEnable", to_vkVoidFunction(test_vkCmdSetPrimitiveRestartEnable)},
        VulkanFunction{"vkCmdSetPrimitiveTopology", to_vkVoidFunction(test_vkCmdSetPrimitiveTopology)},
        VulkanFunction{"vkCmdSetRasterizerDiscardEnable", to_vkVoidFunction(test_vkCmdSetRasterizerDiscardEnable)},
        VulkanFunction{"vkCmdSetScissorWithCount", to_vkVoidFunction(test_vkCmdSetScissorWithCount)},
        VulkanFunction{"vkCmdSetStencilOp", to_vkVoidFunction(test_vkCmdSetStencilOp)},
        VulkanFunction{"vkCmdSetStencilTestEnable", to_vkVoidFunction(test_vkCmdSetStencilTestEnable)},
        VulkanFunction{"vkCmdSetViewportWithCount", to_vkVoidFunction(test_vkCmdSetViewportWithCount)},
        VulkanFunction{"vkCmdWaitEvents2", to_vkVoidFunction(test_vkCmdWaitEvents2)},
        VulkanFunction{"vkCmdWriteTimestamp2", to_vkVoidFunction(test_vkCmdWriteTimestamp2)},
        VulkanFunction{"vkCreatePrivateDataSlot", to_vkVoidFunction(test_vkCreatePrivateDataSlot)},
        VulkanFunction{"vkDestroyPrivateDataSlot", to_vkVoidFunction(test_vkDestroyPrivateDataSlot)},
        VulkanFunction{"vkGetDeviceBufferMemoryRequirements", to_vkVoidFunction(test_vkGetDeviceBufferMemoryRequirements)},
        VulkanFunction{"vkGetDeviceImageMemoryRequirements", to_vkVoidFunction(test_vkGetDeviceImageMemoryRequirements)},
        VulkanFunction{"vkGetDeviceImageSparseMemoryRequirements",
                       to_vkVoidFunction(test_vkGetDeviceImageSparseMemoryRequirements)},
        VulkanFunction{"vkGetPrivateData", to_vkVoidFunction(test_vkGetPrivateData)},
        VulkanFunction{"vkQueueSubmit2", to_vkVoidFunction(test_vkQueueSubmit2)},
        VulkanFunction{"vkSetPrivateData", to_vkVoidFunction(test_vkSetPrivateData)},
    };
    driver.physical_devices.back().add_extension({"VK_SOME_EXT_haha"});
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 3, 0);
    inst.CheckCreate();

    auto phys_dev = inst.GetPhysDev();

    PFN_vkGetPhysicalDeviceToolProperties GetPhysicalDeviceToolProperties = inst.load("vkGetPhysicalDeviceToolProperties");
    uint32_t tool_count = 0;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceToolProperties(phys_dev, &tool_count, nullptr));
    ASSERT_EQ(tool_count, 0U);
    VkPhysicalDeviceToolProperties props;
    ASSERT_EQ(VK_SUCCESS, GetPhysicalDeviceToolProperties(phys_dev, &tool_count, &props));

    DeviceWrapper device{inst};
    device.CheckCreate(phys_dev);

    PFN_vkCreateCommandPool CreateCommandPool = device.load("vkCreateCommandPool");
    PFN_vkAllocateCommandBuffers AllocateCommandBuffers = device.load("vkAllocateCommandBuffers");
    PFN_vkDestroyCommandPool DestroyCommandPool = device.load("vkDestroyCommandPool");
    VkCommandPool command_pool{};
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ASSERT_EQ(VK_SUCCESS, CreateCommandPool(device, &pool_create_info, nullptr, &command_pool));
    VkCommandBufferAllocateInfo buffer_allocate_info{};
    buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_allocate_info.commandPool = command_pool;
    buffer_allocate_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer{};
    ASSERT_EQ(VK_SUCCESS, AllocateCommandBuffers(device, &buffer_allocate_info, &command_buffer));
    DestroyCommandPool(device, command_pool, nullptr);

    PFN_vkCmdBeginRendering CmdBeginRendering = device.load("vkCmdBeginRendering");
    VkRenderingInfoKHR rendering_info{};
    CmdBeginRendering(command_buffer, &rendering_info);

    PFN_vkCmdBindVertexBuffers2 CmdBindVertexBuffers2 = device.load("vkCmdBindVertexBuffers2");
    CmdBindVertexBuffers2(command_buffer, 0, 0, nullptr, nullptr, nullptr, nullptr);

    PFN_vkCmdBlitImage2 CmdBlitImage2 = device.load("vkCmdBlitImage2");
    VkBlitImageInfo2 image_info{};
    CmdBlitImage2(command_buffer, &image_info);

    PFN_vkCmdCopyBuffer2 CmdCopyBuffer2 = device.load("vkCmdCopyBuffer2");
    VkCopyBufferInfo2 copy_info{};
    CmdCopyBuffer2(command_buffer, &copy_info);

    PFN_vkCmdCopyBufferToImage2 CmdCopyBufferToImage2 = device.load("vkCmdCopyBufferToImage2");
    VkCopyBufferToImageInfo2 copy_buf_image{};
    CmdCopyBufferToImage2(command_buffer, &copy_buf_image);

    PFN_vkCmdCopyImage2 CmdCopyImage2 = device.load("vkCmdCopyImage2");
    VkCopyImageInfo2 copy_image_info{};
    CmdCopyImage2(command_buffer, &copy_image_info);

    PFN_vkCmdCopyImageToBuffer2 CmdCopyImageToBuffer2 = device.load("vkCmdCopyImageToBuffer2");
    VkCopyImageToBufferInfo2 copy_image_buf;
    CmdCopyImageToBuffer2(command_buffer, &copy_image_buf);

    PFN_vkCmdEndRendering CmdEndRendering = device.load("vkCmdEndRendering");
    CmdEndRendering(command_buffer);

    PFN_vkCmdPipelineBarrier2 CmdPipelineBarrier2 = device.load("vkCmdPipelineBarrier2");
    VkDependencyInfo deps_info;
    CmdPipelineBarrier2(command_buffer, &deps_info);

    PFN_vkCmdResetEvent2 CmdResetEvent2 = device.load("vkCmdResetEvent2");
    CmdResetEvent2(command_buffer, {}, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    PFN_vkCmdResolveImage2 CmdResolveImage2 = device.load("vkCmdResolveImage2");
    VkResolveImageInfo2 resolve_image{};
    CmdResolveImage2(command_buffer, &resolve_image);

    PFN_vkCmdSetCullMode CmdSetCullMode = device.load("vkCmdSetCullMode");
    CmdSetCullMode(command_buffer, VK_CULL_MODE_BACK_BIT);

    PFN_vkCmdSetDepthBiasEnable CmdSetDepthBiasEnable = device.load("vkCmdSetDepthBiasEnable");
    CmdSetDepthBiasEnable(command_buffer, true);

    PFN_vkCmdSetDepthBoundsTestEnable CmdSetDepthBoundsTestEnable = device.load("vkCmdSetDepthBoundsTestEnable");
    CmdSetDepthBoundsTestEnable(command_buffer, true);

    PFN_vkCmdSetDepthCompareOp CmdSetDepthCompareOp = device.load("vkCmdSetDepthCompareOp");
    CmdSetDepthCompareOp(command_buffer, VK_COMPARE_OP_ALWAYS);

    PFN_vkCmdSetDepthTestEnable CmdSetDepthTestEnable = device.load("vkCmdSetDepthTestEnable");
    CmdSetDepthTestEnable(command_buffer, true);

    PFN_vkCmdSetDepthWriteEnable CmdSetDepthWriteEnable = device.load("vkCmdSetDepthWriteEnable");
    CmdSetDepthWriteEnable(command_buffer, true);

    PFN_vkCmdSetEvent2 CmdSetEvent2 = device.load("vkCmdSetEvent2");
    CmdSetEvent2(command_buffer, {}, &deps_info);

    PFN_vkCmdSetFrontFace CmdSetFrontFace = device.load("vkCmdSetFrontFace");
    CmdSetFrontFace(command_buffer, VK_FRONT_FACE_CLOCKWISE);

    PFN_vkCmdSetPrimitiveRestartEnable CmdSetPrimitiveRestartEnable = device.load("vkCmdSetPrimitiveRestartEnable");
    CmdSetPrimitiveRestartEnable(command_buffer, true);

    PFN_vkCmdSetPrimitiveTopology CmdSetPrimitiveTopology = device.load("vkCmdSetPrimitiveTopology");
    CmdSetPrimitiveTopology(command_buffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

    PFN_vkCmdSetRasterizerDiscardEnable CmdSetRasterizerDiscardEnable = device.load("vkCmdSetRasterizerDiscardEnable");
    CmdSetRasterizerDiscardEnable(command_buffer, true);

    PFN_vkCmdSetScissorWithCount CmdSetScissorWithCount = device.load("vkCmdSetScissorWithCount");
    CmdSetScissorWithCount(command_buffer, 0, nullptr);

    PFN_vkCmdSetStencilOp CmdSetStencilOp = device.load("vkCmdSetStencilOp");
    CmdSetStencilOp(command_buffer, VK_STENCIL_FACE_BACK_BIT, VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_STENCIL_OP_DECREMENT_AND_CLAMP,
                    VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_COMPARE_OP_ALWAYS);

    PFN_vkCmdSetStencilTestEnable CmdSetStencilTestEnable = device.load("vkCmdSetStencilTestEnable");
    CmdSetStencilTestEnable(command_buffer, true);

    PFN_vkCmdSetViewportWithCount CmdSetViewportWithCount = device.load("vkCmdSetViewportWithCount");
    CmdSetViewportWithCount(command_buffer, 0, nullptr);

    PFN_vkCmdWaitEvents2 CmdWaitEvents2 = device.load("vkCmdWaitEvents2");
    CmdWaitEvents2(command_buffer, 0, nullptr, &deps_info);

    PFN_vkCmdWriteTimestamp2 CmdWriteTimestamp2 = device.load("vkCmdWriteTimestamp2");
    CmdWriteTimestamp2(command_buffer, VK_PIPELINE_STAGE_2_BLIT_BIT, {}, 0);

    PFN_vkCreatePrivateDataSlot CreatePrivateDataSlot = device.load("vkCreatePrivateDataSlot");
    CreatePrivateDataSlot(device, nullptr, nullptr, nullptr);
    PFN_vkDestroyPrivateDataSlot DestroyPrivateDataSlot = device.load("vkDestroyPrivateDataSlot");
    DestroyPrivateDataSlot(device, VK_NULL_HANDLE, nullptr);
    PFN_vkGetDeviceBufferMemoryRequirements GetDeviceBufferMemoryRequirements = device.load("vkGetDeviceBufferMemoryRequirements");
    GetDeviceBufferMemoryRequirements(device, nullptr, nullptr);
    PFN_vkGetDeviceImageMemoryRequirements GetDeviceImageMemoryRequirements = device.load("vkGetDeviceImageMemoryRequirements");
    GetDeviceImageMemoryRequirements(device, nullptr, nullptr);
    PFN_vkGetDeviceImageSparseMemoryRequirements GetDeviceImageSparseMemoryRequirements =
        device.load("vkGetDeviceImageSparseMemoryRequirements");
    GetDeviceImageSparseMemoryRequirements(device, nullptr, nullptr, nullptr);
    PFN_vkGetPrivateData GetPrivateData = device.load("vkGetPrivateData");
    GetPrivateData(device, VK_OBJECT_TYPE_UNKNOWN, 0, {}, nullptr);
    PFN_vkQueueSubmit2 QueueSubmit2 = device.load("vkQueueSubmit2");
    QueueSubmit2(nullptr, 0, nullptr, VK_NULL_HANDLE);
    PFN_vkSetPrivateData SetPrivateData = device.load("vkSetPrivateData");
    SetPrivateData(device, VK_OBJECT_TYPE_UNKNOWN, 0, {}, 0);
}

TEST(ApplicationInfoVersion, NonVulkanVariant) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(1, 0, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find(
        std::string("vkCreateInstance: The API Variant specified in pCreateInfo->pApplicationInfo.apiVersion is 1 instead of "
                    "the expected value of 0.")));
}

TEST(DriverManifest, NonVulkanVariant) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_MAKE_API_VERSION(1, 1, 0, 0))).add_physical_device({});

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER);
    ASSERT_TRUE(log.find("loader_parse_icd_manifest: Driver's ICD JSON "));
    // log prints the path to the file, don't look for it since it is hard to determine inside the test what the path should be.
    ASSERT_TRUE(log.find("\'api_version\' field contains a non-zero variant value of 1.  Skipping ICD JSON."));
}

TEST(LayerManifest, ImplicitNonVulkanVariant) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_MAKE_API_VERSION(0, 1, 0, 0))).add_physical_device({});

    const char* implicit_layer_name = "ImplicitTestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_api_version(VK_MAKE_API_VERSION(1, 1, 0, 0))
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "implicit_test_layer.json");

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find(std::string("Layer \"") + implicit_layer_name +
                         "\" has an \'api_version\' field which contains a non-zero variant value of 1.  Skipping Layer."));
}

TEST(LayerManifest, ExplicitNonVulkanVariant) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_MAKE_API_VERSION(0, 1, 0, 0))).add_physical_device({});

    const char* explicit_layer_name = "ExplicitTestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name)
                                                         .set_api_version(VK_MAKE_API_VERSION(1, 1, 0, 0))
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                           "explicit_test_layer.json");

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0)).add_layer(explicit_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_TRUE(log.find(std::string("Layer \"") + explicit_layer_name +
                         "\" has an \'api_version\' field which contains a non-zero variant value of 1.  Skipping Layer."));
}

TEST(DriverManifest, UnknownManifestVersion) {
    FrameworkEnvironment env{};
    env.add_icd(
           TestICDDetails(ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_file_format_version({3, 2, 1})))
        .add_physical_device({});

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find("loader_parse_icd_manifest: "));
    // log prints the path to the file, don't look for it since it is hard to determine inside the test what the path should be.
    ASSERT_TRUE(log.find("has unknown icd manifest file version 3.2.1. May cause errors."));
}

TEST(DriverManifest, LargeUnknownManifestVersion) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(
                    ManifestICD{}.set_lib_path(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_file_format_version({100, 222, 111})))
        .add_physical_device({});

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find("loader_parse_icd_manifest: "));
    // log prints the path to the file, don't look for it since it is hard to determine inside the test what the path should be.
    ASSERT_TRUE(log.find("has unknown icd manifest file version 100.222.111. May cause errors."));
}

TEST(LayerManifest, UnknownManifestVersion) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "ImplicitTestLayer";
    env.add_implicit_layer(ManifestLayer{}
                               .add_layer(ManifestLayer::LayerDescription{}
                                              .set_name(implicit_layer_name)
                                              .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                                              .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                              .set_disable_environment("DISABLE_ME"))
                               .set_file_format_version({3, 2, 1}),
                           "implicit_test_layer.json");

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find("loader_add_layer_properties: "));
    // log prints the path to the file, don't look for it since it is hard to determine inside the test what the path should be.
    ASSERT_TRUE(log.find("has unknown layer manifest file version 3.2.1.  May cause errors."));
}

TEST(LayerManifest, LargeUnknownManifestVersion) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "ImplicitTestLayer";
    env.add_implicit_layer(ManifestLayer{}
                               .add_layer(ManifestLayer::LayerDescription{}
                                              .set_name(implicit_layer_name)
                                              .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                                              .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                              .set_disable_environment("DISABLE_ME"))
                               .set_file_format_version({100, 222, 111}),
                           "implicit_test_layer.json");

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.CheckCreate();
    ASSERT_TRUE(log.find("loader_add_layer_properties: "));
    // log prints the path to the file, don't look for it since it is hard to determine inside the test what the path should be.
    ASSERT_TRUE(log.find("has unknown layer manifest file version 100.222.111.  May cause errors."));
}

struct DriverInfo {
    DriverInfo(TestICDDetails icd_details, uint32_t driver_version, bool expect_to_find) noexcept
        : icd_details(icd_details), driver_version(driver_version), expect_to_find(expect_to_find) {}
    TestICDDetails icd_details;
    uint32_t driver_version = 0;
    bool expect_to_find = false;
};

void CheckDirectDriverLoading(FrameworkEnvironment& env, std::vector<DriverInfo> const& normal_drivers,
                              std::vector<DriverInfo> const& direct_drivers, bool exclusive) {
    std::vector<VkDirectDriverLoadingInfoLUNARG> ddl_infos;
    uint32_t expected_driver_count = 0;

    for (auto const& driver : direct_drivers) {
        auto& direct_driver_icd = env.add_icd(driver.icd_details);
        direct_driver_icd.physical_devices.push_back({});
        direct_driver_icd.physical_devices.at(0).properties.driverVersion = driver.driver_version;
        VkDirectDriverLoadingInfoLUNARG ddl_info{};
        ddl_info.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
        ddl_info.pfnGetInstanceProcAddr = env.icds.back().icd_library.get_symbol("vk_icdGetInstanceProcAddr");
        ddl_infos.push_back(ddl_info);
        if (driver.expect_to_find) {
            expected_driver_count++;
        }
    }

    for (auto const& driver : normal_drivers) {
        auto& direct_driver_icd = env.add_icd(driver.icd_details);
        direct_driver_icd.physical_devices.push_back({});
        direct_driver_icd.physical_devices.at(0).properties.driverVersion = driver.driver_version;
        if (!exclusive && driver.expect_to_find) {
            expected_driver_count++;
        }
    }

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = exclusive ? VK_DIRECT_DRIVER_LOADING_MODE_EXCLUSIVE_LUNARG : VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = static_cast<uint32_t>(ddl_infos.size());
    ddl_list.pDrivers = ddl_infos.data();

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
    inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    if (exclusive) {
        ASSERT_TRUE(
            log.find("loader_scan_for_direct_drivers: The VK_LUNARG_direct_driver_loading extension is active and specified "
                     "VK_DIRECT_DRIVER_LOADING_MODE_EXCLUSIVE_LUNARG, skipping system and environment "
                     "variable driver search mechanisms."));
    }

    // Make sure all drivers we expect to load were found - including checking that the pfn matches exactly.
    for (uint32_t i = 0; i < direct_drivers.size(); i++) {
        if (direct_drivers.at(i).expect_to_find) {
            std::stringstream ss;
            ss << "loader_add_direct_driver: Adding driver found in index " << i
               << " of VkDirectDriverLoadingListLUNARG::pDrivers structure. pfnGetInstanceProcAddr was set to "
               << reinterpret_cast<const void*>(ddl_infos.at(i).pfnGetInstanceProcAddr);
            std::string log_message = ss.str();
            ASSERT_TRUE(log.find(log_message));
        }
    }

    auto phys_devs = inst.GetPhysDevs();
    ASSERT_EQ(phys_devs.size(), expected_driver_count);

    // We have to iterate through the driver lists backwards because the loader *prepends* icd's, so the last found ICD is found
    // first in the driver list
    uint32_t driver_index = 0;
    for (size_t i = normal_drivers.size() - 1; i == 0; i--) {
        if (normal_drivers.at(i).expect_to_find) {
            VkPhysicalDeviceProperties props{};
            inst.functions->vkGetPhysicalDeviceProperties(phys_devs.at(driver_index), &props);
            ASSERT_EQ(props.driverVersion, normal_drivers.at(i).driver_version);
            driver_index++;
        }
    }
    for (size_t i = direct_drivers.size() - 1; i == 0; i--) {
        if (direct_drivers.at(i).expect_to_find) {
            VkPhysicalDeviceProperties props{};
            inst.functions->vkGetPhysicalDeviceProperties(phys_devs.at(driver_index), &props);
            ASSERT_EQ(props.driverVersion, direct_drivers.at(i).driver_version);
            driver_index++;
        }
    }
}

// Only 1 direct driver
TEST(DirectDriverLoading, Individual) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 10, true);

    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, false));
}

// 2 direct drivers
TEST(DirectDriverLoading, MultipleDirectDrivers) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 13, true);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 7, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, false));
}

// Multiple direct drivers with a normal driver in the middle
TEST(DirectDriverLoading, MultipleDirectDriversAndNormalDrivers) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    normal_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA), 90, true);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 80, true);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 70, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, false));
}

// Normal driver and direct driver with direct driver exclusivity
TEST(DirectDriverLoading, ExclusiveWithNormalDriver) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 33, true);
    normal_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_2), 44, false);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

TEST(DirectDriverLoading, ExclusiveWithMultipleNormalDriver) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    normal_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_2), 55, true);
    normal_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_2), 66, true);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 77, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

TEST(DirectDriverLoading, ExclusiveWithDriverEnvVar) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;
    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::env_var), 4, false);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 5, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

TEST(DirectDriverLoading, ExclusiveWithAddDriverEnvVar) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;

    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_discovery_type(ManifestDiscoveryType::add_env_var), 6, false);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 7, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

TEST(DirectDriverLoading, InclusiveWithFilterSelect) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;

    EnvVarWrapper driver_filter_select_env_var{"VK_LOADER_DRIVERS_SELECT", "normal_driver.json"};

    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_disable_icd_inc(true).set_json_name("normal_driver"), 8, true);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 9, true);

    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, false));
}

TEST(DirectDriverLoading, ExclusiveWithFilterSelect) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;

    EnvVarWrapper driver_filter_select_env_var{"VK_LOADER_DRIVERS_SELECT", "normal_driver.json"};

    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_disable_icd_inc(true).set_json_name("normal_driver"), 10,
        false);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 11, true);

    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

TEST(DirectDriverLoading, InclusiveWithFilterDisable) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;

    EnvVarWrapper driver_filter_disable_env_var{"VK_LOADER_DRIVERS_DISABLE", "normal_driver.json"};

    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_disable_icd_inc(true).set_json_name("normal_driver"), 12,
        false);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 13, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, false));
}

TEST(DirectDriverLoading, ExclusiveWithFilterDisable) {
    FrameworkEnvironment env{};
    std::vector<DriverInfo> normal_drivers;
    std::vector<DriverInfo> direct_drivers;

    EnvVarWrapper driver_filter_disable_env_var{"VK_LOADER_DRIVERS_DISABLE", "normal_driver.json"};

    normal_drivers.emplace_back(
        TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA).set_disable_icd_inc(true).set_json_name("normal_driver"), 14,
        false);
    direct_drivers.emplace_back(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none), 15, true);
    ASSERT_NO_FATAL_FAILURE(CheckDirectDriverLoading(env, normal_drivers, direct_drivers, true));
}

// The VK_LUNARG_direct_driver_loading extension is not enabled
TEST(DirectDriverLoading, ExtensionNotEnabled) {
    FrameworkEnvironment env{};

    auto& direct_driver_icd = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none));
    direct_driver_icd.physical_devices.push_back({});

    VkDirectDriverLoadingInfoLUNARG ddl_info{};
    ddl_info.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_info.pfnGetInstanceProcAddr = env.icds.back().icd_library.get_symbol("vk_icdGetInstanceProcAddr");

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = 1U;
    ddl_list.pDrivers = &ddl_info;

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

    ASSERT_TRUE(
        log.find("loader_scan_for_direct_drivers: The pNext chain of VkInstanceCreateInfo contained the "
                 "VkDirectDriverLoadingListLUNARG structure, but the VK_LUNARG_direct_driver_loading extension was "
                 "not enabled."));
}

// VkDirectDriverLoadingListLUNARG is not in the pNext chain of VkInstanceCreateInfo
TEST(DirectDriverLoading, DriverListNotInPnextChain) {
    FrameworkEnvironment env{};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none)).add_physical_device({});

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

    ASSERT_TRUE(
        log.find("loader_scan_for_direct_drivers: The VK_LUNARG_direct_driver_loading extension was enabled but the pNext chain of "
                 "VkInstanceCreateInfo did not contain the "
                 "VkDirectDriverLoadingListLUNARG structure."));
}

// User sets the pDrivers pointer in VkDirectDriverLoadingListLUNARG to nullptr
TEST(DirectDriverLoading, DriverListHasNullDriverPointer) {
    FrameworkEnvironment env{};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none)).add_physical_device({});

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = 1U;
    ddl_list.pDrivers = nullptr;  // user forgot to set the pDrivers

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
    inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

    ASSERT_TRUE(
        log.find("loader_scan_for_direct_drivers: The VkDirectDriverLoadingListLUNARG structure in the pNext chain of "
                 "VkInstanceCreateInfo has a NULL pDrivers member."));
}

// User sets the driverCount in VkDirectDriverLoadingListLUNARG to zero
TEST(DirectDriverLoading, DriverListHasZeroInfoCount) {
    FrameworkEnvironment env{};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none)).add_physical_device({});

    VkDirectDriverLoadingInfoLUNARG ddl_info{};
    ddl_info.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_info.pfnGetInstanceProcAddr = env.icds.back().icd_library.get_symbol("vk_icdGetInstanceProcAddr");

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = 0;  // user set 0 for the info list
    ddl_list.pDrivers = &ddl_info;

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
    inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

    ASSERT_TRUE(
        log.find("loader_scan_for_direct_drivers: The VkDirectDriverLoadingListLUNARG structure in the pNext chain of "
                 "VkInstanceCreateInfo has a non-null pDrivers member but a driverCount member with a value "
                 "of zero."));
}

// pfnGetInstanceProcAddr in VkDirectDriverLoadingInfoLUNARG is nullptr
TEST(DirectDriverLoading, DriverInfoMissingGetInstanceProcAddr) {
    FrameworkEnvironment env{};

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none)).add_physical_device({});

    std::array<VkDirectDriverLoadingInfoLUNARG, 2> ddl_infos{};
    ddl_infos[0].sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_infos[0].pfnGetInstanceProcAddr = nullptr;  // user didn't set the pfnGetInstanceProcAddr to the driver's handle

    ddl_infos[1].sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_infos[1].pfnGetInstanceProcAddr = nullptr;  // user didn't set the pfnGetInstanceProcAddr to the driver's handle

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = static_cast<uint32_t>(ddl_infos.size());
    ddl_list.pDrivers = ddl_infos.data();

    DebugUtilsLogger log;
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, log);
    log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
    inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
    inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

    ASSERT_TRUE(
        log.find("loader_add_direct_driver: VkDirectDriverLoadingInfoLUNARG structure at index 0 contains a NULL pointer for the "
                 "pfnGetInstanceProcAddr member, skipping."));
    ASSERT_TRUE(
        log.find("loader_add_direct_driver: VkDirectDriverLoadingInfoLUNARG structure at index 1 contains a NULL pointer for the "
                 "pfnGetInstanceProcAddr member, skipping."));
}

// test the various error paths in loader_add_direct_driver
TEST(DirectDriverLoading, DriverDoesNotExportNegotiateFunction) {
    FrameworkEnvironment env{};

    auto& direct_driver = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_7).set_discovery_type(ManifestDiscoveryType::none))
                              .add_physical_device({})
                              .set_exposes_vk_icdNegotiateLoaderICDInterfaceVersion(false)
                              .set_exposes_vkCreateInstance(false)
                              .set_exposes_vkEnumerateInstanceExtensionProperties(false);

    VkDirectDriverLoadingInfoLUNARG ddl_info{};
    ddl_info.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_info.pfnGetInstanceProcAddr = env.icds.back().icd_library.get_symbol("vk_icdGetInstanceProcAddr");

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = 1;
    ddl_list.pDrivers = &ddl_info;

    {
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, log);
        log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
        inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
        inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

        ASSERT_TRUE(
            log.find("loader_add_direct_driver: Could not get 'vk_icdNegotiateLoaderICDInterfaceVersion' from "
                     "VkDirectDriverLoadingInfoLUNARG structure at "
                     "index 0, skipping."));
    }

    // Allow the negotiate function to be found, now it should fail to find instance creation function
    direct_driver.set_exposes_vk_icdNegotiateLoaderICDInterfaceVersion(true);
    direct_driver.set_max_icd_interface_version(4);

    {
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, log);
        log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
        inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
        inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

        ASSERT_TRUE(log.find(
            "loader_add_direct_driver: VkDirectDriverLoadingInfoLUNARG structure at index 0 supports interface version 4, "
            "which is incompatible with the Loader Driver Interface version that supports the VK_LUNARG_direct_driver_loading "
            "extension, skipping."));
    }
    direct_driver.set_max_icd_interface_version(7);

    {
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, log);
        log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
        inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
        inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

        ASSERT_TRUE(
            log.find("loader_add_direct_driver: Could not get 'vkEnumerateInstanceExtensionProperties' from "
                     "VkDirectDriverLoadingInfoLUNARG structure at index 0, skipping."));
    }

    // Allow the instance creation function to be found, now it should fail to find EnumInstExtProps
    direct_driver.set_exposes_vkCreateInstance(true);

    {
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, log);
        log.get()->pNext = reinterpret_cast<const void*>(&ddl_list);
        inst.create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
        inst.create_info.set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0));
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_INCOMPATIBLE_DRIVER));

        ASSERT_TRUE(
            log.find("loader_add_direct_driver: Could not get 'vkEnumerateInstanceExtensionProperties' from "
                     "VkDirectDriverLoadingInfoLUNARG structure at index 0, skipping."));
    }
}

TEST(DriverManifest, VersionMismatchWithEnumerateInstanceVersion) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
        .set_icd_api_version(VK_API_VERSION_1_0)
        .add_physical_device({});

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();

    ASSERT_TRUE(env.debug_log.find(std::string("terminator_CreateInstance: Manifest ICD for \"") +
                                   env.get_test_icd_path().string() +
                                   "\" contained a 1.1 or greater API version, but "
                                   "vkEnumerateInstanceVersion returned 1.0, treating as a 1.0 ICD"));
}

TEST(DriverManifest, EnumerateInstanceVersionNotSupported) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_1))
        .set_icd_api_version(VK_API_VERSION_1_0)
        .set_can_query_vkEnumerateInstanceVersion(false)
        .add_physical_device({});

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();

    ASSERT_TRUE(env.debug_log.find(std::string("terminator_CreateInstance: Manifest ICD for \"") +
                                   env.get_test_icd_path().string() +
                                   "\" contained a 1.1 or greater API version, but does "
                                   "not support vkEnumerateInstanceVersion, treating as a 1.0 ICD"));
}
