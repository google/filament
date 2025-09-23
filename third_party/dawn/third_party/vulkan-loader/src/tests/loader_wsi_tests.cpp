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

#include "test_environment.h"

class WsiTests : public ::testing::Test {};

#if defined(VK_USE_PLATFORM_WIN32_KHR)

// When ICD doesn't support the extension, create instance should fail
TEST(WsiTests, CreateSurfaceWin32NoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();

    ASSERT_EQ(nullptr, env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkCreateWin32SurfaceKHR"));
}

// When ICD doesn't support the surface creation, the loader should handle it
TEST(WsiTests, CreateSurfaceWin32NoICDCreateSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkWin32SurfaceCreateInfoKHR surf_create_info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWin32SurfaceKHR(inst, &surf_create_info, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// When ICD does support the surface creation, the loader should  delegat handle it to the ICD
TEST(WsiTests, CreateSurfaceWin32ICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkWin32SurfaceCreateInfoKHR surf_create_info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWin32SurfaceKHR(inst, &surf_create_info, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// Some drivers supporting vkCreateWin32SurfaceKHR, and at least one that doesn't
TEST(WsiTests, CreateSurfaceWin32MixedICDSupport) {
    FrameworkEnvironment env{};
    for (uint32_t icd = 0; icd < 3; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extensions({first_ext, second_ext});
        if (icd < 2) {
            // Only enable ICD for first two
            cur_icd.enable_icd_wsi = true;
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkWin32SurfaceCreateInfoKHR surf_create_info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWin32SurfaceKHR(instance.inst, &surf_create_info, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}

TEST(WsiTests, GetPhysicalDeviceWin32PresentNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);
    auto res = env.vulkan_functions.vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, 0);
    ASSERT_EQ(res, VK_FALSE);
    ASSERT_TRUE(log.find("ICD for selected physical device does not export vkGetPhysicalDeviceWin32PresentationSupportKHR!"));
}

TEST(WsiTests, GetPhysicalDeviceWin32PresentICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    ASSERT_EQ(VK_TRUE, env.vulkan_functions.vkGetPhysicalDeviceWin32PresentationSupportKHR(physical_device, 0));
}

TEST(WsiTests, Win32GetPhysicalDeviceSurfaceSupportKHR) {
    FrameworkEnvironment env{};
    const uint32_t max_device_count = 4;
    for (uint32_t icd = 0; icd < max_device_count; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.set_min_icd_interface_version(5);
        cur_icd.add_instance_extensions({first_ext, second_ext});
        std::string dev_name = "phys_dev_" + std::to_string(icd);
        cur_icd.physical_devices.emplace_back(dev_name.c_str());
        cur_icd.physical_devices.back().add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true});
        cur_icd.enable_icd_wsi = true;
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkWin32SurfaceCreateInfoKHR surf_create_info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWin32SurfaceKHR(instance.inst, &surf_create_info, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    uint32_t device_count = max_device_count;
    std::array<VkPhysicalDevice, max_device_count> phys_devs;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(instance.inst, &device_count, phys_devs.data()));
    ASSERT_EQ(device_count, max_device_count);

    for (uint32_t pd = 0; pd < max_device_count; ++pd) {
        VkBool32 supported = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(phys_devs[pd], 0, surface, &supported));
        ASSERT_EQ(VK_TRUE, supported);
    }

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
// When ICD doesn't support the extension, create instance should fail
TEST(WsiTests, CreateSurfaceXCBNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();

    ASSERT_EQ(nullptr, env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkCreateXcbSurfaceKHR"));
}

// When ICD doesn't support the surface creation, the loader should handle it
TEST(WsiTests, CreateSurfaceXCBNoICDCreateSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXcbSurfaceKHR(inst, &xcb_createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// When ICD does support the surface creation, the loader should  delegat handle it to the ICD
TEST(WsiTests, CreateSurfaceXCBICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXcbSurfaceKHR(inst, &xcb_createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// Some drivers supporting vkCreateXcbSurfaceKHR, and at least one that doesn't
TEST(WsiTests, CreateSurfaceXCBMixedICDSupport) {
    FrameworkEnvironment env{};
    for (uint32_t icd = 0; icd < 3; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_XCB_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extensions({first_ext, second_ext});
        if (icd < 2) {
            // Only enable ICD for first two
            cur_icd.enable_icd_wsi = true;
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXcbSurfaceKHR(instance.inst, &xcb_createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}

TEST(WsiTests, GetPhysicalDeviceXcbPresentNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);
    auto res = env.vulkan_functions.vkGetPhysicalDeviceXcbPresentationSupportKHR(physical_device, 0, nullptr, 0);
    ASSERT_EQ(res, VK_FALSE);
    ASSERT_TRUE(log.find("ICD for selected physical device does not export vkGetPhysicalDeviceXcbPresentationSupportKHR!"));
}

TEST(WsiTests, GetPhysicalDeviceXcbPresentICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    ASSERT_EQ(VK_TRUE, env.vulkan_functions.vkGetPhysicalDeviceXcbPresentationSupportKHR(physical_device, 0, nullptr, 0));
}

TEST(WsiTests, XcbGetPhysicalDeviceSurfaceSupportKHR) {
    FrameworkEnvironment env{};
    const uint32_t max_device_count = 4;
    for (uint32_t icd = 0; icd < max_device_count; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_XCB_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.set_min_icd_interface_version(5);
        cur_icd.add_instance_extensions({first_ext, second_ext});
        std::string dev_name = "phys_dev_" + std::to_string(icd);
        cur_icd.physical_devices.emplace_back(dev_name.c_str());
        cur_icd.physical_devices.back().add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true});
        cur_icd.enable_icd_wsi = true;
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXcbSurfaceKHR(instance.inst, &xcb_createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    uint32_t device_count = max_device_count;
    std::array<VkPhysicalDevice, max_device_count> phys_devs;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(instance.inst, &device_count, phys_devs.data()));
    ASSERT_EQ(device_count, max_device_count);

    for (uint32_t pd = 0; pd < max_device_count; ++pd) {
        VkBool32 supported = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(phys_devs[pd], 0, surface, &supported));
        ASSERT_EQ(VK_TRUE, supported);
    }

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
// When ICD doesn't support the extension, create instance should fail
TEST(WsiTests, CreateSurfaceXLIBNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();

    ASSERT_EQ(nullptr, env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkCreateXlibSurfaceKHR"));
}

// When ICD doesn't support the surface creation, the loader should handle it
TEST(WsiTests, CreateSurfaceXLIBNoICDCreateSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkXlibSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXlibSurfaceKHR(inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// When ICD does support the surface creation, the loader should  delegat handle it to the ICD
TEST(WsiTests, CreateSurfaceXLIBICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkXlibSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXlibSurfaceKHR(inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// Some drivers supporting vkCreateXlibSurfaceKHR, and at least one that doesn't
TEST(WsiTests, CreateSurfaceXLIBMixedICDSupport) {
    FrameworkEnvironment env{};
    for (uint32_t icd = 0; icd < 3; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extensions({first_ext, second_ext});
        if (icd < 2) {
            // Only enable ICD for first two
            cur_icd.enable_icd_wsi = true;
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkXlibSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXlibSurfaceKHR(instance.inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}

TEST(WsiTests, GetPhysicalDeviceXlibPresentNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);
    auto res = env.vulkan_functions.vkGetPhysicalDeviceXlibPresentationSupportKHR(physical_device, 0, nullptr, 0);
    ASSERT_EQ(res, VK_FALSE);
    ASSERT_TRUE(log.find("ICD for selected physical device does not export vkGetPhysicalDeviceXlibPresentationSupportKHR!"));
}

TEST(WsiTests, GetPhysicalDeviceXlibPresentICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    ASSERT_EQ(VK_TRUE, env.vulkan_functions.vkGetPhysicalDeviceXlibPresentationSupportKHR(physical_device, 0, nullptr, 0));
}

TEST(WsiTests, XlibGetPhysicalDeviceSurfaceSupportKHR) {
    FrameworkEnvironment env{};
    const uint32_t max_device_count = 4;
    for (uint32_t icd = 0; icd < max_device_count; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.set_min_icd_interface_version(5);
        cur_icd.add_instance_extensions({first_ext, second_ext});
        std::string dev_name = "phys_dev_" + std::to_string(icd);
        cur_icd.physical_devices.emplace_back(dev_name.c_str());
        cur_icd.physical_devices.back().add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true});
        cur_icd.enable_icd_wsi = true;
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkXlibSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXlibSurfaceKHR(instance.inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    uint32_t device_count = max_device_count;
    std::array<VkPhysicalDevice, max_device_count> phys_devs;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(instance.inst, &device_count, phys_devs.data()));
    ASSERT_EQ(device_count, max_device_count);

    for (uint32_t pd = 0; pd < max_device_count; ++pd) {
        VkBool32 supported = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(phys_devs[pd], 0, surface, &supported));
        ASSERT_EQ(VK_TRUE, supported);
    }

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
// When ICD doesn't support the extension, create instance should fail
TEST(WsiTests, CreateSurfaceWaylandNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    inst.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();

    ASSERT_EQ(nullptr, env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkCreateWaylandSurfaceKHR"));
}

// When ICD doesn't support the surface creation, the loader should handle it
TEST(WsiTests, CreateSurfaceWaylandNoICDCreateSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkWaylandSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWaylandSurfaceKHR(inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// When ICD does support the surface creation, the loader should  delegat handle it to the ICD
TEST(WsiTests, CreateSurfaceWaylandICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    VkWaylandSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWaylandSurfaceKHR(inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(inst, surface, nullptr);
}

// Some drivers supporting vkCreateWaylandSurfaceKHR, and at least one that doesn't
TEST(WsiTests, CreateSurfaceWaylandMixedICDSupport) {
    FrameworkEnvironment env{};
    for (uint32_t icd = 0; icd < 3; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.add_instance_extensions({first_ext, second_ext});
        if (icd < 2) {
            // Only enable ICD for first two
            cur_icd.enable_icd_wsi = true;
        }
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkWaylandSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};

    VkSurfaceKHR surface;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWaylandSurfaceKHR(instance.inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}

TEST(WsiTests, GetPhysicalDeviceWaylandPresentNoICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = false;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions(
        {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);
    auto res = env.vulkan_functions.vkGetPhysicalDeviceWaylandPresentationSupportKHR(physical_device, 0, nullptr);
    ASSERT_EQ(res, VK_FALSE);
    ASSERT_TRUE(log.find("ICD for selected physical device does not export vkGetPhysicalDeviceWaylandPresentationSupportKHR!"));
}

TEST(WsiTests, GetPhysicalDeviceWaylandPresentICDSupport) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    auto& cur_icd = env.get_test_icd(0);
    cur_icd.set_min_icd_interface_version(5);
    cur_icd.add_instance_extension({VK_KHR_SURFACE_EXTENSION_NAME});
    cur_icd.add_instance_extension({VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    cur_icd.physical_devices.emplace_back("physical_device_0");
    cur_icd.enable_icd_wsi = true;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    inst.CheckCreate();

    uint32_t driver_count = 1;
    VkPhysicalDevice physical_device;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &driver_count, &physical_device));
    ASSERT_EQ(driver_count, 1U);

    ASSERT_EQ(VK_TRUE, env.vulkan_functions.vkGetPhysicalDeviceWaylandPresentationSupportKHR(physical_device, 0, nullptr));
}

TEST(WsiTests, WaylandGetPhysicalDeviceSurfaceSupportKHR) {
    FrameworkEnvironment env{};
    const uint32_t max_device_count = 4;
    for (uint32_t icd = 0; icd < max_device_count; ++icd) {
        Extension first_ext{VK_KHR_SURFACE_EXTENSION_NAME};
        Extension second_ext{VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
        auto& cur_icd = env.get_test_icd(icd);
        cur_icd.icd_api_version = VK_API_VERSION_1_0;
        cur_icd.set_min_icd_interface_version(5);
        cur_icd.add_instance_extensions({first_ext, second_ext});
        std::string dev_name = "phys_dev_" + std::to_string(icd);
        cur_icd.physical_devices.emplace_back(dev_name.c_str());
        cur_icd.physical_devices.back().add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true});
        cur_icd.enable_icd_wsi = true;
    }

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
    instance.CheckCreate();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkWaylandSurfaceCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWaylandSurfaceKHR(instance.inst, &createInfo, nullptr, &surface));
    ASSERT_TRUE(surface != VK_NULL_HANDLE);

    uint32_t device_count = max_device_count;
    std::array<VkPhysicalDevice, max_device_count> phys_devs;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumeratePhysicalDevices(instance.inst, &device_count, phys_devs.data()));
    ASSERT_EQ(device_count, max_device_count);

    for (uint32_t pd = 0; pd < max_device_count; ++pd) {
        VkBool32 supported = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(phys_devs[pd], 0, surface, &supported));
        ASSERT_EQ(VK_TRUE, supported);
    }

    env.vulkan_functions.vkDestroySurfaceKHR(instance.inst, surface, nullptr);
}
#endif

TEST(WsiTests, GoogleSurfaceslessQuery) {
    std::vector<VkPresentModeKHR> present_modes{VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
    VkSurfaceFormatKHR surface_format = {VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI()
        .add_instance_extension("VK_GOOGLE_surfaceless_query")
        .add_instance_extension("VK_KHR_get_surface_capabilities2")
        .add_physical_device(PhysicalDevice{}
                                 .add_extension("VK_KHR_swapchain")
#if defined(WIN32)
                                 .add_extension("VK_EXT_full_screen_exclusive")
#endif
                                 .add_surface_format(surface_format)
                                 .add_surface_present_modes(present_modes)
                                 .finish());

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension("VK_KHR_surface");
    inst.create_info.add_extension("VK_GOOGLE_surfaceless_query");
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    VkPhysicalDevice physical_device = inst.GetPhysDev();

    uint32_t present_mode_count = 4;
    std::vector<VkPresentModeKHR> queried_present_modes{present_mode_count};
    inst->vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, VK_NULL_HANDLE, &present_mode_count,
                                                    queried_present_modes.data());
    ASSERT_EQ(present_modes, queried_present_modes);

    uint32_t surface_format_count = 1;
    std::vector<VkSurfaceFormatKHR> queried_surface_formats{surface_format_count};
    inst->vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, VK_NULL_HANDLE, &surface_format_count,
                                               queried_surface_formats.data());
    ASSERT_EQ(std::vector<VkSurfaceFormatKHR>{surface_format}, queried_surface_formats);

    uint32_t surface_format2_count = 1;
    std::vector<VkSurfaceFormat2KHR> queried_surface_formats2{surface_format2_count};
    VkPhysicalDeviceSurfaceInfo2KHR surface_info{};
    surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surface_info.surface = VK_NULL_HANDLE;
    inst->vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &surface_info, &surface_format_count,
                                                queried_surface_formats2.data());
    ASSERT_EQ(std::vector<VkSurfaceFormatKHR>{surface_format}, queried_surface_formats2);

    VkSurfaceCapabilities2KHR surface_caps2{};
    surface_caps2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;

    ASSERT_EQ(VK_SUCCESS,
              env.vulkan_functions.vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info, &surface_caps2));

#if defined(WIN32)
    PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT pfn_vkGetPhysicalDeviceSurfacePresentModes2EXT =
        inst.load("vkGetPhysicalDeviceSurfacePresentModes2EXT");
    ASSERT_EQ(VK_SUCCESS, pfn_vkGetPhysicalDeviceSurfacePresentModes2EXT(physical_device, &surface_info, &present_mode_count,
                                                                         queried_present_modes.data()));
    ASSERT_EQ(present_modes, queried_present_modes);

#endif
}

TEST(WsiTests, ForgetEnableSurfaceExtensions) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI()
        .add_physical_device(PhysicalDevice{}.add_extension("VK_KHR_swapchain").finish());

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension("VK_KHR_surface");
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_ERROR_EXTENSION_NOT_PRESENT, create_surface(inst, surface));
}

TEST(WsiTests, SwapchainFunctional) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI()
        .add_physical_device(PhysicalDevice{}.add_extension("VK_KHR_swapchain").finish());

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI();
    inst.CheckCreate();
    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    {  // Use GDPA to get functions
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");

        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));

        VkSwapchainKHR swapchain{};
        VkSwapchainCreateInfoKHR swap_create_info{};
        swap_create_info.surface = surface;
        DeviceFunctions funcs{*inst.functions, dev};
        ASSERT_EQ(VK_SUCCESS, funcs.vkCreateSwapchainKHR(dev, &swap_create_info, nullptr, &swapchain));
        uint32_t count = 0;
        ASSERT_EQ(VK_SUCCESS, funcs.vkGetSwapchainImagesKHR(dev, swapchain, &count, nullptr));
        ASSERT_GT(count, 0U);
        std::array<VkImage, 16> images;
        ASSERT_EQ(VK_SUCCESS, funcs.vkGetSwapchainImagesKHR(dev, swapchain, &count, images.data()));
        funcs.vkDestroySwapchainKHR(dev, swapchain, nullptr);
    }
    {  // Use GIPA gotten functions
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");

        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));

        PFN_vkCreateSwapchainKHR inst_CreateSwapchainKHR = inst.load("vkCreateSwapchainKHR");
        PFN_vkGetSwapchainImagesKHR inst_GetSwapchainImagesKHR = inst.load("vkGetSwapchainImagesKHR");
        PFN_vkDestroySwapchainKHR inst_DestroySwapchainKHR = inst.load("vkDestroySwapchainKHR");
        ASSERT_TRUE(nullptr != inst_CreateSwapchainKHR);
        ASSERT_TRUE(nullptr != inst_GetSwapchainImagesKHR);
        ASSERT_TRUE(nullptr != inst_DestroySwapchainKHR);

        VkSwapchainKHR swapchain{};
        VkSwapchainCreateInfoKHR swap_create_info{};
        swap_create_info.surface = surface;

        ASSERT_EQ(VK_SUCCESS, inst_CreateSwapchainKHR(dev, &swap_create_info, nullptr, &swapchain));
        uint32_t count = 0;
        ASSERT_EQ(VK_SUCCESS, inst_GetSwapchainImagesKHR(dev, swapchain, &count, nullptr));
        ASSERT_GT(count, 0U);
        std::array<VkImage, 16> images;
        ASSERT_EQ(VK_SUCCESS, inst_GetSwapchainImagesKHR(dev, swapchain, &count, images.data()));
        inst_DestroySwapchainKHR(dev, swapchain, nullptr);
    }
    {  // forget to enable the extension
        DeviceWrapper dev{inst};
        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));

        DeviceFunctions funcs{*inst.functions, dev};
        ASSERT_EQ(funcs.vkCreateSwapchainKHR, nullptr);
        ASSERT_EQ(funcs.vkGetSwapchainImagesKHR, nullptr);
        ASSERT_EQ(funcs.vkDestroySwapchainKHR, nullptr);
    }
    {  // forget to set the surface
        DeviceWrapper dev{inst};
        dev.create_info.add_extension("VK_KHR_swapchain");

        dev.CheckCreate(phys_dev);

        VkSwapchainKHR swapchain{};
        VkSwapchainCreateInfoKHR swap_create_info{};
        DeviceFunctions funcs{*inst.functions, dev};
        ASSERT_DEATH(funcs.vkCreateSwapchainKHR(dev, &swap_create_info, nullptr, &swapchain), "");
    }
    env.vulkan_functions.vkDestroySurfaceKHR(inst.inst, surface, nullptr);
}

TEST(WsiTests, EXTSurfaceMaintenance1) {
    FrameworkEnvironment env{};

    std::vector<VkPresentModeKHR> present_modes{VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
    VkSurfaceCapabilitiesKHR surface_caps{};
    surface_caps.maxImageExtent = VkExtent2D{300, 300};
    surface_caps.minImageExtent = VkExtent2D{100, 100};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI()
        .add_instance_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)
        .add_physical_device(PhysicalDevice{}
                                 .add_extension("VK_KHR_swapchain")
                                 .set_deviceName("no")
                                 .set_surface_capabilities(surface_caps)
                                 .add_surface_present_modes(present_modes)
                                 .finish());
    VkSurfacePresentScalingCapabilitiesEXT scaling_capabilities{};
    scaling_capabilities.supportedPresentScaling = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT;
    scaling_capabilities.supportedPresentGravityX = VK_PRESENT_SCALING_ASPECT_RATIO_STRETCH_BIT_EXT;
    scaling_capabilities.supportedPresentGravityY = VK_PRESENT_SCALING_STRETCH_BIT_EXT;
    scaling_capabilities.minScaledImageExtent = {60, 60};
    scaling_capabilities.maxScaledImageExtent = {1000, 1000};
    auto& icd2 = env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
                     .setup_WSI()
                     .add_instance_extension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME)
                     .add_instance_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)
                     .add_physical_device(PhysicalDevice{}
                                              .add_extension("VK_KHR_swapchain")
                                              .set_deviceName("yes")
                                              .set_surface_capabilities(surface_caps)
                                              .add_surface_present_modes(present_modes)
                                              .set_surface_present_scaling_capabilities(scaling_capabilities)
                                              .finish());
    std::vector<std::vector<VkPresentModeKHR>> compatible_present_modes{
        {VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR},
        {VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR},
        {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR},
        {VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_FIFO_KHR},
    };
    icd2.physical_devices[0].surface_present_mode_compatibility = compatible_present_modes;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI()
        .add_physical_device(PhysicalDevice{}
                                 .add_extension("VK_KHR_swapchain")
                                 .set_deviceName("no")
                                 .set_surface_capabilities(surface_caps)
                                 .add_surface_present_modes(present_modes)
                                 .finish());

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.setup_WSI();
    inst.create_info.add_extension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    inst.create_info.add_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    inst.CheckCreate();

    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));
    WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> wrapped_surface{surface, inst.inst,
                                                                                     env.vulkan_functions.vkDestroySurfaceKHR};
    auto physical_devices = inst.GetPhysDevs(3);

    for (auto physical_device : physical_devices) {
        VkPhysicalDeviceProperties phys_dev_props{};
        inst->vkGetPhysicalDeviceProperties(physical_device, &phys_dev_props);
        bool driver_support_surface_maintenance1 = string_eq(phys_dev_props.deviceName, "yes");

        uint32_t present_mode_count = 4;
        std::vector<VkPresentModeKHR> queried_present_modes{present_mode_count};
        inst->vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                        queried_present_modes.data());

        for (uint32_t i = 0; i < present_mode_count; i++) {
            VkSurfacePresentModeEXT present_mode{};
            present_mode.sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT;
            present_mode.presentMode = queried_present_modes[i];

            VkPhysicalDeviceSurfaceInfo2KHR surface_info{};
            surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
            surface_info.surface = surface;
            surface_info.pNext = &present_mode;

            VkSurfacePresentModeCompatibilityEXT SurfacePresentModeCompatibilityEXT{};
            SurfacePresentModeCompatibilityEXT.sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT;

            VkSurfacePresentScalingCapabilitiesEXT SurfacePresentScalingCapabilitiesEXT{};
            SurfacePresentScalingCapabilitiesEXT.sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT;
            SurfacePresentScalingCapabilitiesEXT.pNext = &SurfacePresentModeCompatibilityEXT;

            VkSurfaceCapabilities2KHR surface_caps2{};
            surface_caps2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
            surface_caps2.pNext = &SurfacePresentScalingCapabilitiesEXT;

            ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info,
                                                                                                  &surface_caps2));
            if (driver_support_surface_maintenance1) {
                ASSERT_EQ(SurfacePresentModeCompatibilityEXT.presentModeCount, 2U);
            } else {
                ASSERT_EQ(SurfacePresentModeCompatibilityEXT.presentModeCount, 1U);
            }
            std::vector<VkPresentModeKHR> queried_compatible_present_modes{SurfacePresentModeCompatibilityEXT.presentModeCount};
            SurfacePresentModeCompatibilityEXT.pPresentModes = queried_compatible_present_modes.data();

            ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &surface_info,
                                                                                                  &surface_caps2));

            if (driver_support_surface_maintenance1) {
                ASSERT_EQ(compatible_present_modes[i], queried_compatible_present_modes);
                ASSERT_EQ(scaling_capabilities, SurfacePresentScalingCapabilitiesEXT);
            } else {
                // Make sure the emulation returned the values we expect - 1 compatible present mode which is the mode we are
                // querying, Scaling capabilities is 0 (aka none) and the extent is just the surface caps extent
                ASSERT_EQ(SurfacePresentModeCompatibilityEXT.presentModeCount, 1U);
                ASSERT_EQ(SurfacePresentModeCompatibilityEXT.pPresentModes[0], queried_present_modes[i]);
                ASSERT_EQ(SurfacePresentScalingCapabilitiesEXT.supportedPresentScaling, 0U);
                ASSERT_EQ(SurfacePresentScalingCapabilitiesEXT.supportedPresentGravityX, 0u);
                ASSERT_EQ(SurfacePresentScalingCapabilitiesEXT.supportedPresentGravityY, 0U);
                ASSERT_EQ(SurfacePresentScalingCapabilitiesEXT.minScaledImageExtent, surface_caps.minImageExtent);
                ASSERT_EQ(SurfacePresentScalingCapabilitiesEXT.maxScaledImageExtent, surface_caps.maxImageExtent);
            }
        }
    }
}
#if defined(VK_USE_PLATFORM_WAYLAND_KHR) && defined(VK_USE_PLATFORM_XCB_KHR)
TEST(WsiTests, MultiPlatformGetPhysicalDeviceSurfaceSupportKHR) {
    FrameworkEnvironment env{};

    const char* xcb_device_name = "XCB";
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI("VK_USE_PLATFORM_XCB_KHR")
        .add_physical_device(PhysicalDevice{}
                                 .set_deviceName(xcb_device_name)
                                 .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true})
                                 .finish());
    const char* wayland_device_name = "WAYLAND";
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .setup_WSI("VK_USE_PLATFORM_WAYLAND_KHR")
        .add_physical_device(PhysicalDevice{}
                                 .set_deviceName(wayland_device_name)
                                 .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, true})
                                 .finish());

    {
        // Create instance with only XCB support
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_EXTENSION_NAME});
        inst.CheckCreate();

        auto phys_devs = inst.GetPhysDevs();
        // Physical devices are enumerated in reverse order to the ICD order
        VkPhysicalDevice xcb_physical_device = phys_devs[1];
        VkPhysicalDevice wayland_physical_device = phys_devs[0];
        VkPhysicalDeviceProperties props0{};
        inst->vkGetPhysicalDeviceProperties(wayland_physical_device, &props0);
        ASSERT_TRUE(string_eq(props0.deviceName, wayland_device_name));

        VkPhysicalDeviceProperties props1{};
        inst->vkGetPhysicalDeviceProperties(xcb_physical_device, &props1);
        ASSERT_TRUE(string_eq(props1.deviceName, xcb_device_name));

        VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};

        VkSurfaceKHR surface0{VK_NULL_HANDLE};
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateXcbSurfaceKHR(inst, &xcb_createInfo, nullptr, &surface0));
        ASSERT_TRUE(surface0 != VK_NULL_HANDLE);
        WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> wrapped_surface{surface0, inst.inst,
                                                                                         env.vulkan_functions.vkDestroySurfaceKHR};

        VkWaylandSurfaceCreateInfoKHR wayland_createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};

        VkSurfaceKHR surface1{VK_NULL_HANDLE};
        ASSERT_EQ(VK_ERROR_EXTENSION_NOT_PRESENT,
                  env.vulkan_functions.vkCreateWaylandSurfaceKHR(inst, &wayland_createInfo, nullptr, &surface1));

        // Use the successful surface

        VkBool32 supported0 = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS,
                  env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(xcb_physical_device, 0, surface0, &supported0));
        ASSERT_EQ(VK_TRUE, supported0);

        VkBool32 supported1 = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS,
                  env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(wayland_physical_device, 0, surface0, &supported1));
        ASSERT_EQ(VK_FALSE, supported1);
    }

    {
        // Create instance with only WAYLAND support

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_extensions({VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME});
        inst.CheckCreate();

        auto phys_devs = inst.GetPhysDevs();
        // Physical devices are enumerated in reverse order to the ICD order
        VkPhysicalDevice xcb_physical_device = phys_devs[1];
        VkPhysicalDevice wayland_physical_device = phys_devs[0];
        VkPhysicalDeviceProperties props0{};
        inst->vkGetPhysicalDeviceProperties(wayland_physical_device, &props0);
        ASSERT_TRUE(string_eq(props0.deviceName, wayland_device_name));

        VkPhysicalDeviceProperties props1{};
        inst->vkGetPhysicalDeviceProperties(xcb_physical_device, &props1);
        ASSERT_TRUE(string_eq(props1.deviceName, xcb_device_name));

        VkXcbSurfaceCreateInfoKHR xcb_createInfo{VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};

        VkSurfaceKHR surface0{VK_NULL_HANDLE};
        ASSERT_EQ(VK_ERROR_EXTENSION_NOT_PRESENT,
                  env.vulkan_functions.vkCreateXcbSurfaceKHR(inst, &xcb_createInfo, nullptr, &surface0));

        VkWaylandSurfaceCreateInfoKHR wayland_createInfo{VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR};

        VkSurfaceKHR surface1{VK_NULL_HANDLE};
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateWaylandSurfaceKHR(inst, &wayland_createInfo, nullptr, &surface1));
        ASSERT_TRUE(surface1 != VK_NULL_HANDLE);
        WrappedHandle<VkSurfaceKHR, VkInstance, PFN_vkDestroySurfaceKHR> wrapped_surface{surface1, inst.inst,
                                                                                         env.vulkan_functions.vkDestroySurfaceKHR};
        // Use the successful surface

        VkBool32 supported0 = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS,
                  env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(xcb_physical_device, 0, surface1, &supported0));
        ASSERT_EQ(VK_FALSE, supported0);

        VkBool32 supported1 = VK_FALSE;
        ASSERT_EQ(VK_SUCCESS,
                  env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(wayland_physical_device, 0, surface1, &supported1));
        ASSERT_EQ(VK_TRUE, supported1);
    }
}
#endif  // defined(VK_USE_PLATFORM_WAYLAND_KHR) && defined(VK_USE_PLATFORM_XCB_KHR)
