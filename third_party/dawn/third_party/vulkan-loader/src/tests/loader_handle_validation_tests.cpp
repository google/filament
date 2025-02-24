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
 * Author(s): Charles Giessen <charles@lunarg.com>
 *            Mark Young <marky@lunarg.com>
 */

#include "test_environment.h"

// ---- Invalid Instance tests

struct BadData {
    uint64_t bad_array[3] = {0x123456789AB, 0x23456789AB1, 0x9876543210AB};
};
template <typename T>
T get_bad_handle() {
    static BadData my_bad_data;
    return reinterpret_cast<T>(static_cast<void*>(&my_bad_data));
}

TEST(LoaderHandleValidTests, BadInstEnumPhysDevices) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();
    uint32_t returned_physical_count = 0;

    ASSERT_DEATH(env.vulkan_functions.vkEnumeratePhysicalDevices(bad_instance, &returned_physical_count, nullptr),
                 "vkEnumeratePhysicalDevices: Invalid instance \\[VUID-vkEnumeratePhysicalDevices-instance-parameter\\]");
}

TEST(LoaderHandleValidTests, BadInstGetInstProcAddr) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();
    ASSERT_DEATH(env.vulkan_functions.vkGetInstanceProcAddr(bad_instance, "vkGetBufferDeviceAddress"),
                 "vkGetInstanceProcAddr: Invalid instance \\[VUID-vkGetInstanceProcAddr-instance-parameter\\]");
}

TEST(LoaderHandleValidTests, BadInstDestroyInstance) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    ASSERT_DEATH(env.vulkan_functions.vkDestroyInstance(bad_instance, nullptr),
                 "vkDestroyInstance: Invalid instance \\[VUID-vkDestroyInstance-instance-parameter\\]");
}

TEST(LoaderHandleValidTests, BadInstDestroySurface) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_EXT_headless_surface"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(first_ext.extensionName.c_str());
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    ASSERT_DEATH(env.vulkan_functions.vkDestroySurfaceKHR(bad_instance, VK_NULL_HANDLE, nullptr),
                 "vkDestroySurfaceKHR: Invalid instance \\[VUID-vkDestroySurfaceKHR-instance-parameter\\]");
}

TEST(LoaderHandleValidTests, BadInstCreateHeadlessSurf) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_EXT_headless_surface"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({first_ext.extensionName.c_str(), second_ext.extensionName.c_str()});
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkHeadlessSurfaceCreateInfoEXT surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateHeadlessSurfaceEXT(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateHeadlessSurfaceEXT: Invalid instance \\[VUID-vkCreateHeadlessSurfaceEXT-instance-parameter\\]");
}

TEST(LoaderHandleValidTests, BadInstCreateDisplayPlaneSurf) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_KHR_display"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions({first_ext.extensionName.c_str(), second_ext.extensionName.c_str()});
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkDisplaySurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateDisplayPlaneSurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateDisplayPlaneSurfaceKHR: Invalid instance \\[VUID-vkCreateDisplayPlaneSurfaceKHR-instance-parameter\\]");
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
TEST(LoaderHandleValidTests, BadInstCreateAndroidSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkAndroidSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateAndroidSurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateAndroidSurfaceKHR: Invalid instance \\[VUID-vkCreateAndroidSurfaceKHR-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
TEST(LoaderHandleValidTests, BadInstCreateDirectFBSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkDirectFBSurfaceCreateInfoEXT surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateDirectFBSurfaceEXT(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateDirectFBSurfaceEXT: Invalid instance \\[VUID-vkCreateDirectFBSurfaceEXT-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_FUCHSIA)
TEST(LoaderHandleValidTests, BadInstCreateFuchsiaSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkImagePipeSurfaceCreateInfoFUCHSIA surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateImagePipeSurfaceFUCHSIA(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateImagePipeSurfaceFUCHSIA: Invalid instance \\[VUID-vkCreateImagePipeSurfaceFUCHSIA-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#if defined(VK_USE_PLATFORM_GGP)
TEST(LoaderHandleValidTests, BadInstCreateGGPSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkStreamDescriptorSurfaceCreateInfoGGP surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(
        env.vulkan_functions.vkCreateStreamDescriptorSurfaceGGP(bad_instance, &surf_create_info, nullptr, &created_surface),
        "vkCreateStreamDescriptorSurfaceGGP: Invalid instance \\[VUID-vkCreateStreamDescriptorSurfaceGGP-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_GGP

#if defined(VK_USE_PLATFORM_IOS_MVK)
TEST(LoaderHandleValidTests, BadInstCreateIOSSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkIOSSurfaceCreateInfoMVK surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateIOSSurfaceMVK(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateIOSSurfaceMVK: Invalid instance \\[VUID-vkCreateIOSSurfaceMVK-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_MACOS_MVK)
TEST(LoaderHandleValidTests, BadInstCreateMacOSSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkMacOSSurfaceCreateInfoMVK surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateMacOSSurfaceMVK(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateMacOSSurfaceMVK: Invalid instance \\[VUID-vkCreateMacOSSurfaceMVK-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_METAL_EXT)
TEST(LoaderHandleValidTests, BadInstCreateMetalSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkMetalSurfaceCreateInfoEXT surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateMetalSurfaceEXT(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateMetalSurfaceEXT: Invalid instance \\[VUID-vkCreateMetalSurfaceEXT-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
TEST(LoaderHandleValidTests, BadInstCreateQNXSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkScreenSurfaceCreateInfoQNX surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateScreenSurfaceQNX(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateScreenSurfaceQNX: Invalid instance \\[VUID-vkCreateScreenSurfaceQNX-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

#if defined(VK_USE_PLATFORM_VI_NN)
TEST(LoaderHandleValidTests, BadInstCreateViNNSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkViSurfaceCreateInfoNN surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateViSurfaceNN(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateViSurfaceNN: Invalid instance \\[VUID-vkCreateViSurfaceNN-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_VI_NN

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
TEST(LoaderHandleValidTests, BadInstCreateWaylandSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkWaylandSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateWaylandSurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateWaylandSurfaceKHR: Invalid instance \\[VUID-vkCreateWaylandSurfaceKHR-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST(LoaderHandleValidTests, BadInstCreateWin32Surf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkWin32SurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateWin32SurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateWin32SurfaceKHR: Invalid instance \\[VUID-vkCreateWin32SurfaceKHR-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
TEST(LoaderHandleValidTests, BadInstCreateXCBSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkXcbSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateXcbSurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateXcbSurfaceKHR: Invalid instance \\[VUID-vkCreateXcbSurfaceKHR-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
TEST(LoaderHandleValidTests, BadInstCreateXlibSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_instance = get_bad_handle<VkInstance>();

    VkXlibSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateXlibSurfaceKHR(bad_instance, &surf_create_info, nullptr, &created_surface),
                 "vkCreateXlibSurfaceKHR: Invalid instance \\[VUID-vkCreateXlibSurfaceKHR-instance-parameter\\]");
    // TODO: Look for "invalid instance" in stderr log to make sure correct error is thrown
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

// ---- Invalid Physical Device tests

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevFeature) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceFeatures features = {};
    ASSERT_DEATH(
        env.vulkan_functions.vkGetPhysicalDeviceFeatures(bad_physical_dev, &features),
        "vkGetPhysicalDeviceFeatures: Invalid physicalDevice \\[VUID-vkGetPhysicalDeviceFeatures-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevFormatProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkFormatProperties format_info = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceFormatProperties(bad_physical_dev, VK_FORMAT_R8G8B8A8_UNORM, &format_info),
                 "vkGetPhysicalDeviceFormatProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceFormatProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevImgFormatProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkImageFormatProperties format_info = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceImageFormatProperties(bad_physical_dev, VK_FORMAT_R8G8B8A8_UNORM,
                                                                               VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR,
                                                                               VK_IMAGE_USAGE_STORAGE_BIT, 0, &format_info),
                 "vkGetPhysicalDeviceImageFormatProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceImageFormatProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceProperties properties = {};
    ASSERT_DEATH(
        env.vulkan_functions.vkGetPhysicalDeviceProperties(bad_physical_dev, &properties),
        "vkGetPhysicalDeviceProperties: Invalid physicalDevice \\[VUID-vkGetPhysicalDeviceProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevQueueFamProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceQueueFamilyProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDevMemProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceMemoryProperties properties = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceMemoryProperties(bad_physical_dev, &properties),
                 "vkGetPhysicalDeviceMemoryProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceMemoryProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevCreateDevice) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    float queue_priorities[3] = {0.0f, 0.5f, 1.0f};
    VkDeviceQueueCreateInfo dev_queue_create_info = {};
    dev_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dev_queue_create_info.pNext = nullptr;
    dev_queue_create_info.flags = 0;
    dev_queue_create_info.queueFamilyIndex = 0;
    dev_queue_create_info.queueCount = 1;
    dev_queue_create_info.pQueuePriorities = queue_priorities;
    VkDeviceCreateInfo dev_create_info = {};
    dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_create_info.pNext = nullptr;
    dev_create_info.flags = 0;
    dev_create_info.queueCreateInfoCount = 1;
    dev_create_info.pQueueCreateInfos = &dev_queue_create_info;
    dev_create_info.enabledLayerCount = 0;
    dev_create_info.ppEnabledLayerNames = nullptr;
    dev_create_info.enabledExtensionCount = 0;
    dev_create_info.ppEnabledExtensionNames = nullptr;
    dev_create_info.pEnabledFeatures = nullptr;
    VkDevice created_dev = VK_NULL_HANDLE;
    ASSERT_DEATH(env.vulkan_functions.vkCreateDevice(bad_physical_dev, &dev_create_info, nullptr, &created_dev),
                 "vkCreateDevice: Invalid physicalDevice \\[VUID-vkCreateDevice-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevEnumDevExtProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkEnumerateDeviceExtensionProperties(bad_physical_dev, nullptr, &count, nullptr),
                 "vkEnumerateDeviceExtensionProperties: Invalid physicalDevice "
                 "\\[VUID-vkEnumerateDeviceExtensionProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevEnumDevLayerProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkEnumerateDeviceLayerProperties(bad_physical_dev, &count, nullptr),
                 "vkEnumerateDeviceLayerProperties: Invalid physicalDevice "
                 "\\[VUID-vkEnumerateDeviceLayerProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSparseImgFormatProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSparseImageFormatProperties(
                     bad_physical_dev, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT,
                     VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_LINEAR, &count, nullptr),
                 "vkGetPhysicalDeviceSparseImageFormatProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSparseImageFormatProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevFeature2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = nullptr;
    ASSERT_DEATH(
        env.vulkan_functions.vkGetPhysicalDeviceFeatures2(bad_physical_dev, &features),
        "vkGetPhysicalDeviceFeatures2: Invalid physicalDevice \\[VUID-vkGetPhysicalDeviceFeatures2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevFormatProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkFormatProperties2 properties = {};
    properties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    properties.pNext = nullptr;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceFormatProperties2(bad_physical_dev, VK_FORMAT_R8G8B8A8_UNORM, &properties),
                 "vkGetPhysicalDeviceFormatProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceFormatProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevImgFormatProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceImageFormatInfo2 format_info = {};
    format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    format_info.pNext = nullptr;
    VkImageFormatProperties2 properties = {};
    properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    properties.pNext = nullptr;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceImageFormatProperties2(bad_physical_dev, &format_info, &properties),
                 "vkGetPhysicalDeviceImageFormatProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceImageFormatProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceProperties2 properties = {};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties.pNext = nullptr;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceProperties2(bad_physical_dev, &properties),
                 "vkGetPhysicalDeviceProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevQueueFamProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties2(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceQueueFamilyProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDevMemProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceMemoryProperties2 properties = {};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    properties.pNext = nullptr;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceMemoryProperties2(bad_physical_dev, &properties),
                 "vkGetPhysicalDeviceMemoryProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceMemoryProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSparseImgFormatProps2) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceSparseImageFormatInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2;
    info.pNext = nullptr;
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSparseImageFormatProperties2(bad_physical_dev, &info, &count, nullptr),
                 "vkGetPhysicalDeviceSparseImageFormatProperties2: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSparseImageFormatProperties2-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevExternFenceProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceExternalFenceInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO;
    info.pNext = nullptr;
    VkExternalFenceProperties props = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceExternalFenceProperties(bad_physical_dev, &info, &props),
                 "vkGetPhysicalDeviceExternalFenceProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceExternalFenceProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevExternBufferProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceExternalBufferInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    info.pNext = nullptr;
    VkExternalBufferProperties props = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceExternalBufferProperties(bad_physical_dev, &info, &props),
                 "vkGetPhysicalDeviceExternalBufferProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceExternalBufferProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevExternSemaphoreProps) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0));

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.set_api_version(1, 1, 0);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VkPhysicalDeviceExternalSemaphoreInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
    info.pNext = nullptr;
    VkExternalSemaphoreProperties props = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceExternalSemaphoreProperties(bad_physical_dev, &info, &props),
                 "vkGetPhysicalDeviceExternalSemaphoreProperties: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceExternalSemaphoreProperties-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfaceSupportKHR) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_KHR_display"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(first_ext.extensionName.c_str());
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkBool32 supported = VK_FALSE;

    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfaceSupportKHR(bad_physical_dev, 0, VK_NULL_HANDLE, &supported),
                 "vkGetPhysicalDeviceSurfaceSupportKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfaceSupportKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfaceCapsKHR) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_KHR_display"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(first_ext.extensionName.c_str());
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkSurfaceCapabilitiesKHR caps = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(bad_physical_dev, VK_NULL_HANDLE, &caps),
                 "vkGetPhysicalDeviceSurfaceCapabilitiesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfaceCapabilitiesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfaceFormatsKHR) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_KHR_display"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(first_ext.extensionName.c_str());
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfaceFormatsKHR(bad_physical_dev, VK_NULL_HANDLE, &count, nullptr),
                 "vkGetPhysicalDeviceSurfaceFormatsKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfaceFormatsKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfacePresentModesKHR) {
    FrameworkEnvironment env{};
    Extension first_ext{"VK_KHR_surface"};
    Extension second_ext{"VK_KHR_display"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions({first_ext, second_ext})
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension(first_ext.extensionName.c_str());
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfacePresentModesKHR(bad_physical_dev, VK_NULL_HANDLE, &count, nullptr),
                 "vkGetPhysicalDeviceSurfacePresentModesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfacePresentModesKHR-physicalDevice-parameter\\]");
}

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
TEST(LoaderHandleValidTests, BadPhysDevGetDirectFBPresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    IDirectFB directfb;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceDirectFBPresentationSupportEXT(bad_physical_dev, 0, &directfb),
                 "vkGetPhysicalDeviceDirectFBPresentationSupportEXT: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceDirectFBPresentationSupportEXT-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
TEST(LoaderHandleValidTests, BadPhysDevGetQNXPresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    ASSERT_DEATH(env.vulkan_functions.PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX(bad_physical_dev, 0, nullptr),
                 "vkGetPhysicalDeviceScreenPresentationSupportQNX: Invalid instance "
                 "\\[VUID-vkGetPhysicalDeviceScreenPresentationSupportQNX-instance-parameter\\]");
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevWaylandPresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceWaylandPresentationSupportKHR(bad_physical_dev, 0, nullptr),
                 "vkGetPhysicalDeviceWaylandPresentationSupportKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceWaylandPresentationSupportKHR-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevWin32PresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceWin32PresentationSupportKHR(bad_physical_dev, 0),
                 "vkGetPhysicalDeviceWin32PresentationSupportKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceWin32PresentationSupportKHR-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
TEST(LoaderHandleValidTests, BadPhysDevGetXCBPresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();
    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    xcb_visualid_t visual = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceXcbPresentationSupportKHR(bad_physical_dev, 0, nullptr, visual),
                 "vkGetPhysicalDeviceXcbPresentationSupportKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceXcbPresentationSupportKHR-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
TEST(LoaderHandleValidTests, BadPhysDevGetXlibPresentSupportKHR) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();

    VisualID visual = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceXlibPresentationSupportKHR(bad_physical_dev, 0, nullptr, visual),
                 "vkGetPhysicalDeviceXlibPresentationSupportKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceXlibPresentationSupportKHR-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

InstWrapper setup_BadPhysDev_env(FrameworkEnvironment& env, std::vector<const char*> exts) {
    std::vector<Extension> ext_modified;
    for (const auto& ext : exts) {
        ext_modified.push_back(Extension{ext});
    }
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .add_instance_extensions(ext_modified)
        .setup_WSI()
        .add_physical_device("physical_device_0");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extensions(exts);
    instance.CheckCreate();
    return instance;
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDisplayPropsKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceDisplayPropertiesKHR(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceDisplayPropertiesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceDisplayPropertiesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDisplayPlanePropsKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceDisplayPlanePropertiesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceDisplayPlanePropertiesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDisplayPlaneSupportedDisplaysKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetDisplayPlaneSupportedDisplaysKHR(bad_physical_dev, 0, &count, nullptr),
                 "vkGetDisplayPlaneSupportedDisplaysKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetDisplayPlaneSupportedDisplaysKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDisplayModePropsKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(
        env.vulkan_functions.vkGetDisplayModePropertiesKHR(bad_physical_dev, VK_NULL_HANDLE, &count, nullptr),
        "vkGetDisplayModePropertiesKHR: Invalid physicalDevice \\[VUID-vkGetDisplayModePropertiesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevCreateDisplayModeKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkDisplayModeCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    VkDisplayModeKHR display_mode;
    ASSERT_DEATH(
        env.vulkan_functions.vkCreateDisplayModeKHR(bad_physical_dev, VK_NULL_HANDLE, &create_info, nullptr, &display_mode),
        "vkCreateDisplayModeKHR: Invalid physicalDevice \\[VUID-vkCreateDisplayModeKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDisplayPlaneCapsKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkDisplayPlaneCapabilitiesKHR caps = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetDisplayPlaneCapabilitiesKHR(bad_physical_dev, VK_NULL_HANDLE, 0, &caps),
                 "vkGetDisplayPlaneCapabilitiesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetDisplayPlaneCapabilitiesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevPresentRectsKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDevicePresentRectanglesKHR(bad_physical_dev, VK_NULL_HANDLE, &count, nullptr),
                 "vkGetPhysicalDevicePresentRectanglesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDevicePresentRectanglesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDisplayProps2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_display_properties2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceDisplayProperties2KHR(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceDisplayProperties2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceDisplayProperties2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevDisplayPlaneProps2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_display_properties2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceDisplayPlaneProperties2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceDisplayPlaneProperties2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDisplayModeProps2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_display_properties2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetDisplayModeProperties2KHR(bad_physical_dev, VK_NULL_HANDLE, &count, nullptr),
                 "vkGetDisplayModeProperties2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetDisplayModeProperties2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDisplayPlaneCaps2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_display_properties2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkDisplayPlaneInfo2KHR disp_plane_info = {};
    disp_plane_info.sType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR;
    disp_plane_info.pNext = nullptr;
    VkDisplayPlaneCapabilities2KHR caps = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetDisplayPlaneCapabilities2KHR(bad_physical_dev, &disp_plane_info, &caps),
                 "vkGetDisplayPlaneCapabilities2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetDisplayPlaneCapabilities2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfaceCaps2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_surface_capabilities2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkPhysicalDeviceSurfaceInfo2KHR phys_dev_surf_info = {};
    phys_dev_surf_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    phys_dev_surf_info.pNext = nullptr;
    VkSurfaceCapabilities2KHR caps = {};
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfaceCapabilities2KHR(bad_physical_dev, &phys_dev_surf_info, &caps),
                 "vkGetPhysicalDeviceSurfaceCapabilities2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfaceCapabilities2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfaceFormats2KHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_KHR_get_surface_capabilities2"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkPhysicalDeviceSurfaceInfo2KHR phys_dev_surf_info = {};
    phys_dev_surf_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    phys_dev_surf_info.pNext = nullptr;
    uint32_t count = 0;
    ASSERT_DEATH(env.vulkan_functions.vkGetPhysicalDeviceSurfaceFormats2KHR(bad_physical_dev, &phys_dev_surf_info, &count, nullptr),
                 "vkGetPhysicalDeviceSurfaceFormats2KHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfaceFormats2KHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevEnumPhysDevQueueFamilyPerfQueryCountersKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR pfn =
        instance.load("vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, 0, &count, nullptr, nullptr),
                 "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR: Invalid physicalDevice "
                 "\\[VUID-vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevQueueFamilyPerfQueryPassesKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkQueryPoolPerformanceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    uint32_t count = 0;
    PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR pfn =
        instance.load("vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, &create_info, &count),
                 "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevFragmentShadingRatesKHR) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    uint32_t count = 0;
    PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR pfn = instance.load("vkGetPhysicalDeviceFragmentShadingRatesKHR");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceFragmentShadingRatesKHR: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceFragmentShadingRatesKHR-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevMSPropsEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkMultisamplePropertiesEXT props = {};
    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT pfn = instance.load("vkGetPhysicalDeviceMultisamplePropertiesEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, VK_SAMPLE_COUNT_1_BIT, &props),
                 "vkGetPhysicalDeviceMultisamplePropertiesEXT: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceMultisamplePropertiesEXT-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevAcquireDrmDisplayEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_EXT_acquire_drm_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    PFN_vkAcquireDrmDisplayEXT pfn = instance.load("vkAcquireDrmDisplayEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, 0, VK_NULL_HANDLE),
                 "vkAcquireDrmDisplayEXT: Invalid physicalDevice \\[VUID-vkAcquireDrmDisplayEXT-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetDrmDisplayEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_EXT_acquire_drm_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    PFN_vkGetDrmDisplayEXT pfn = instance.load("vkGetDrmDisplayEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, 0, 0, VK_NULL_HANDLE),
                 "vkGetDrmDisplayEXT: Invalid physicalDevice "
                 "\\[VUID-vkGetDrmDisplayEXT-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevReleaseDisplayEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_EXT_direct_mode_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    PFN_vkReleaseDisplayEXT pfn = instance.load("vkReleaseDisplayEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, VK_NULL_HANDLE),
                 "vkReleaseDisplayEXT: Invalid physicalDevice \\[VUID-vkReleaseDisplayEXT-physicalDevice-parameter\\]");
}

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
TEST(LoaderHandleValidTests, BadPhysDevAcquireXlibDisplayEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_EXT_acquire_xlib_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    PFN_vkAcquireXlibDisplayEXT pfn = instance.load("vkAcquireXlibDisplayEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, nullptr, VK_NULL_HANDLE),
                 "vkAcquireXlibDisplayEXT: Invalid physicalDevice \\[VUID-vkAcquireXlibDisplayEXT-physicalDevice-parameter\\]");
}

TEST(LoaderHandleValidTests, BadPhysDevGetRandROutputDisplayEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {"VK_KHR_surface", "VK_EXT_acquire_xlib_display"});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    RROutput rrout = {};
    VkDisplayKHR disp;
    PFN_vkGetRandROutputDisplayEXT pfn = instance.load("vkGetRandROutputDisplayEXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(
        pfn(bad_physical_dev, nullptr, rrout, &disp),
        "vkGetRandROutputDisplayEXT: Invalid physicalDevice \\[VUID-vkGetRandROutputDisplayEXT-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevSurfacePresentModes2EXT) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    InstWrapper instance(env.vulkan_functions);
    instance.CheckCreate();

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    VkPhysicalDeviceSurfaceInfo2KHR phys_dev_surf_info = {};
    phys_dev_surf_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    phys_dev_surf_info.pNext = nullptr;
    uint32_t count = 0;
    PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT pfn = instance.load("vkGetPhysicalDeviceSurfacePresentModes2EXT");
    ASSERT_NE(pfn, nullptr);
    ASSERT_DEATH(pfn(bad_physical_dev, &phys_dev_surf_info, &count, nullptr),
                 "vkGetPhysicalDeviceSurfacePresentModes2EXT: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceSurfacePresentModes2EXT-physicalDevice-parameter\\]");
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

TEST(LoaderHandleValidTests, BadPhysDevGetPhysDevToolPropertiesEXT) {
    FrameworkEnvironment env{};
    auto instance = setup_BadPhysDev_env(env, {});

    auto bad_physical_dev = get_bad_handle<VkPhysicalDevice>();
    PFN_vkGetPhysicalDeviceToolPropertiesEXT pfn = instance.load("vkGetPhysicalDeviceToolPropertiesEXT");
    ASSERT_NE(pfn, nullptr);
    uint32_t count = 0;
    ASSERT_DEATH(pfn(bad_physical_dev, &count, nullptr),
                 "vkGetPhysicalDeviceToolPropertiesEXT: Invalid physicalDevice "
                 "\\[VUID-vkGetPhysicalDeviceToolPropertiesEXT-physicalDevice-parameter\\]");
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
TEST(LoaderHandleValidTests, VerifyHandleWrappingAndroidSurface) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkAndroidSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateAndroidSurfaceKHR pfn_CreateSurface = instance.load("vkCreateAndroidSurfaceKHR");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
TEST(LoaderHandleValidTests, VerifyHandleWrappingDirectFBSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkDirectFBSurfaceCreateInfoEXT surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateDirectFBSurfaceEXT pfn_CreateSurface = instance.load("vkCreateDirectFBSurfaceEXT");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_FUCHSIA)
TEST(LoaderHandleValidTests, VerifyHandleWrappingFuchsiaSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkImagePipeSurfaceCreateInfoFUCHSIA surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateImagePipeSurfaceFUCHSIA pfn_CreateSurface = instance.load("vkCreateImagePipeSurfaceFUCHSIA");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#if defined(VK_USE_PLATFORM_GGP)
TEST(LoaderHandleValidTests, VerifyHandleWrappingGGPSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkStreamDescriptorSurfaceCreateInfoGGP surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateStreamDescriptorSurfaceGGP pfn_CreateSurface = instance.load("vkCreateStreamDescriptorSurfaceGGP");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_GGP

#if defined(VK_USE_PLATFORM_IOS_MVK)
TEST(LoaderHandleValidTests, VerifyHandleWrappingIOSSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkIOSSurfaceCreateInfoMVK surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateIOSSurfaceMVK pfn_CreateSurface = instance.load("vkCreateIOSSurfaceMVK");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_MACOS_MVK)
TEST(LoaderHandleValidTests, VerifyHandleWrappingMacOSSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI("VK_USE_PLATFORM_MACOS_MVK");

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI("VK_USE_PLATFORM_MACOS_MVK");
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkMacOSSurfaceCreateInfoMVK surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateMacOSSurfaceMVK pfn_CreateSurface = instance.load("vkCreateMacOSSurfaceMVK");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_METAL_EXT)
TEST(LoaderHandleValidTests, VerifyHandleWrappingMetalSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI("VK_USE_PLATFORM_METAL_EXT");

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI("VK_USE_PLATFORM_METAL_EXT");
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkMetalSurfaceCreateInfoEXT surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateMetalSurfaceEXT pfn_CreateSurface = instance.load("vkCreateMetalSurfaceEXT");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
TEST(LoaderHandleValidTests, VerifyHandleWrappingQNXSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkScreenSurfaceCreateInfoQNX surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateScreenSurfaceQNX pfn_CreateSurface = instance.load("vkCreateScreenSurfaceQNX");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

#if defined(VK_USE_PLATFORM_VI_NN)
TEST(LoaderHandleValidTests, VerifyHandleWrappingViNNSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkViSurfaceCreateInfoNN surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateViSurfaceNN pfn_CreateSurface = instance.load("vkCreateViSurfaceNN");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_VI_NN

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
TEST(LoaderHandleValidTests, VerifyHandleWrappingWaylandSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI("VK_USE_PLATFORM_WAYLAND_KHR");

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI("VK_USE_PLATFORM_WAYLAND_KHR");
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkWaylandSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateWaylandSurfaceKHR pfn_CreateSurface = instance.load("vkCreateWaylandSurfaceKHR");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
TEST(LoaderHandleValidTests, VerifyHandleWrappingWin32Surf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkWin32SurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateWin32SurfaceKHR pfn_CreateSurface = instance.load("vkCreateWin32SurfaceKHR");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
TEST(LoaderHandleValidTests, VerifyHandleWrappingXCBSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI();

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI();
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkXcbSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateXcbSurfaceKHR pfn_CreateSurface = instance.load("vkCreateXcbSurfaceKHR");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
TEST(LoaderHandleValidTests, VerifyHandleWrappingXlibSurf) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).setup_WSI("VK_USE_PLATFORM_XLIB_KHR");

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.setup_WSI("VK_USE_PLATFORM_XLIB_KHR");
    //
    for (auto& ext : instance.create_info.enabled_extensions) {
        std::cout << ext << "\n";
    }
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkXlibSurfaceCreateInfoKHR surf_create_info = {};
    surf_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surf_create_info.pNext = nullptr;
    VkSurfaceKHR created_surface = VK_NULL_HANDLE;
    PFN_vkCreateXlibSurfaceKHR pfn_CreateSurface = instance.load("vkCreateXlibSurfaceKHR");
    ASSERT_NE(pfn_CreateSurface, nullptr);
    PFN_vkDestroySurfaceKHR pfn_DestroySurface = instance.load("vkDestroySurfaceKHR");
    ASSERT_NE(pfn_DestroySurface, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateSurface(instance, &surf_create_info, nullptr, &created_surface));
    pfn_DestroySurface(instance, created_surface, nullptr);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkBool32 VKAPI_CALL JunkDebugUtilsCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                      [[maybe_unused]] void* pUserData) {
    // This is just a stub callback in case the loader or any other layer triggers it.
    return VK_FALSE;
}

TEST(LoaderHandleValidTests, VerifyHandleWrappingDebugUtilsMessenger) {
    FrameworkEnvironment env{};
    Extension ext{"VK_EXT_debug_utils"};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_instance_extensions({ext});

    const char* wrap_objects_name = "WrapObjectsLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    InstWrapper instance(env.vulkan_functions);
    instance.create_info.add_extension("VK_EXT_debug_utils");
    instance.create_info.add_layer(wrap_objects_name);
    instance.CheckCreate();

    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};
    debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_create_info.pNext = nullptr;
    debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_messenger_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_messenger_create_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(JunkDebugUtilsCallback);
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    PFN_vkCreateDebugUtilsMessengerEXT pfn_CreateMessenger = instance.load("vkCreateDebugUtilsMessengerEXT");
    ASSERT_NE(pfn_CreateMessenger, nullptr);
    PFN_vkDestroyDebugUtilsMessengerEXT pfn_DestroyMessenger = instance.load("vkDestroyDebugUtilsMessengerEXT");
    ASSERT_NE(pfn_DestroyMessenger, nullptr);
    ASSERT_EQ(VK_SUCCESS, pfn_CreateMessenger(instance, &debug_messenger_create_info, nullptr, &messenger));
    pfn_DestroyMessenger(instance, messenger, nullptr);
}
