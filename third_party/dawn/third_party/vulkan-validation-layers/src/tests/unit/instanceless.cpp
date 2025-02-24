/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

//   Instanceless tests
// Tests of validation of vkCreateInstance and vkDestroyInstance via the pNext debug callback.
//
// This set of test should ideally be as complete as possible. Most of the VUs are Implicit (i.e. automatically generated), but any
// of the parameters could expose a bug or inadequacy in the Loader or the debug extension.
//
// Note: testing pCreateInfo pointer, the sType of a debug struct, the debug callback pointer, the ppEnabledLayerNames pointer, and
//       the ppEnabledExtensionNames would require extenally enabled debug layers, so this is currently not performed.
//
// TODO: VkDebugReportCallbackCreateInfoEXT::flags and VkDebugUtilsMessengerCreateInfoEXT various Flags could theoretically be
//       tested if the debug extensions are made robust enough

#include <memory>
#include <thread>
#include <vector>

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

static VkInstance dummy_instance;

class NegativeInstanceless : public VkLayerTest {};

TEST_F(NegativeInstanceless, InstanceExtensionDependencies) {
    TEST_DESCRIPTION("Test enabling instance extension without dependencies met.");

    if (!InstanceExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find required instance extension";
    }
    ASSERT_TRUE(InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME));  // Driver should always provide dependencies

    Monitor().SetDesiredError("VUID-vkCreateInstance-ppEnabledExtensionNames-01388");
    m_instance_extension_names.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    const auto ici = GetInstanceCreateInfo();
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(NegativeInstanceless, ExtensionNestedDependency) {
    TEST_DESCRIPTION("Make sure nested dependency extension logic is being checked");
    // VkSwapchainPresentModesCreateInfoEXT is part of VK_EXT_swapchain_maintenance1
    // VK_EXT_swapchain_maintenance1 requires VK_EXT_surface_maintenance1
    // VK_EXT_surface_maintenance1 requires VK_KHR_get_surface_capabilities2
    //
    // Don't enable VK_KHR_get_surface_capabilities2
    SetTargetApiVersion(VK_API_VERSION_1_0);
    if (!InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME) ||
        !InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find the required instance extensions";
    }
    m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    Monitor().SetDesiredError("VUID-vkCreateInstance-ppEnabledExtensionNames-01388");
    const auto ici = GetInstanceCreateInfo();
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(NegativeInstanceless, InstanceBadStype) {
    TEST_DESCRIPTION("Test creating instance with bad sType.");

    auto ici = GetInstanceCreateInfo();

    Monitor().SetDesiredError("VUID-VkInstanceCreateInfo-sType-sType");
    ici.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(NegativeInstanceless, InstanceDuplicatePnextStype) {
    TEST_DESCRIPTION("Test creating instance with duplicate sType in the pNext chain.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find required instance extension";
    }
    m_instance_extension_names.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    Monitor().SetDesiredError("VUID-VkInstanceCreateInfo-sType-unique");
    const VkValidationFeaturesEXT duplicate_pnext = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, ici.pNext};
    const VkValidationFeaturesEXT first_pnext = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, &duplicate_pnext};
    ici.pNext = &first_pnext;
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(NegativeInstanceless, InstanceAppInfoBadStype) {
    TEST_DESCRIPTION("Test creating instance with invalid sType in VkApplicationInfo.");

    auto ici = GetInstanceCreateInfo();

    VkApplicationInfo bad_app_info = {};
    if (ici.pApplicationInfo) bad_app_info = *ici.pApplicationInfo;
    bad_app_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &bad_app_info;

    Monitor().SetDesiredError("VUID-VkApplicationInfo-sType-sType");
    vk::CreateInstance(&ici, nullptr, &dummy_instance);
    Monitor().VerifyFound();
}

TEST_F(NegativeInstanceless, InstanceValidationFeaturesBadFlags) {
    TEST_DESCRIPTION("Test creating instance with invalid flags in VkValidationFeaturesEXT.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find required instance extension";
    }
    m_instance_extension_names.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    // the test framework should not be using VkValidationFeatureEnableEXT itself
    for (auto traversable_pnext = reinterpret_cast<const VkBaseInStructure*>(ici.pNext); traversable_pnext;
         traversable_pnext = traversable_pnext->pNext) {
        ASSERT_NE(traversable_pnext->sType, VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT);
    }

    VkValidationFeaturesEXT validation_features_template = {};
    validation_features_template.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features_template.pNext = ici.pNext;

    {
        const VkValidationFeatureEnableEXT bad_enable = (VkValidationFeatureEnableEXT)0x42;
        VkValidationFeaturesEXT validation_features = validation_features_template;
        validation_features.enabledValidationFeatureCount = 1;
        validation_features.pEnabledValidationFeatures = &bad_enable;
        ici.pNext = &validation_features;

        Monitor().SetDesiredError("VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        const VkValidationFeatureDisableEXT bad_disable = (VkValidationFeatureDisableEXT)0x42;
        VkValidationFeaturesEXT validation_features = validation_features_template;
        validation_features.disabledValidationFeatureCount = 1;
        validation_features.pDisabledValidationFeatures = &bad_disable;
        ici.pNext = &validation_features;

        Monitor().SetDesiredError("VUID-VkValidationFeaturesEXT-pDisabledValidationFeatures-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }
}

TEST_F(NegativeInstanceless, InstanceValidationFlags) {
    TEST_DESCRIPTION("Test creating instance with invalid VkValidationFlagsEXT.");

    if (!InstanceExtensionSupported(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME)) {
        GTEST_SKIP() << "Did not find required instance extension";
    }
    m_instance_extension_names.push_back(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME);

    auto ici = GetInstanceCreateInfo();

    // the test framework should not be using VkValidationFlagsEXT itself
    for (auto traversable_pnext = reinterpret_cast<const VkBaseInStructure*>(ici.pNext); traversable_pnext;
         traversable_pnext = traversable_pnext->pNext) {
        ASSERT_NE(traversable_pnext->sType, VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT);
    }

    VkValidationFlagsEXT validation_flags_template = {};
    validation_flags_template.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
    validation_flags_template.pNext = ici.pNext;

    {
        const VkValidationCheckEXT bad_disable = (VkValidationCheckEXT)0x42;
        VkValidationFlagsEXT validation_flags = validation_flags_template;
        validation_flags.disabledValidationCheckCount = 1;
        validation_flags.pDisabledValidationChecks = &bad_disable;
        ici.pNext = &validation_flags;

        Monitor().SetDesiredError("VUID-VkValidationFlagsEXT-pDisabledValidationChecks-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        VkValidationFlagsEXT validation_flags = validation_flags_template;
        validation_flags.disabledValidationCheckCount = 0;
        ici.pNext = &validation_flags;

        Monitor().SetDesiredError("VUID-VkValidationFlagsEXT-disabledValidationCheckCount-arraylength");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }
}

void* VKAPI_PTR DummyAlloc(void*, size_t size, size_t alignment, VkSystemAllocationScope) {
    size_t space = size + alignment - 1;
    void* mem_ptr = std::malloc(space);
    if (!mem_ptr) {
        printf("Dummy Alloc failed\n");
        return nullptr;
    }
    return std::align(alignment, size, mem_ptr, space);
}
void VKAPI_PTR DummyFree(void*, void* pMemory) {
// The test which uses DummyFree plays foul of the nature of memory allocation on Windows. Because it doesn't use
// a VkAllocationCallback during vkCreateInstance then passes one in during vkDestroyInstance, the memory that is
// allocated during vkCreateInstance lives in a different 'heap'. Trying to free it from the application with the
// DummyFree callbacks will crash the application. The goal of this change is to enable Leak Sanitizer to run on
// CI runs in linux, so leaking memory in the windows tests is okay.
#if !defined(WIN32)
    std::free(pMemory);
#endif
}
void* VKAPI_PTR DummyRealloc(void* pUserData, void* pOriginal, size_t size, size_t alignment,
                             VkSystemAllocationScope allocationScope) {
    DummyFree(pUserData, pOriginal);
    return DummyAlloc(pUserData, size, alignment, allocationScope);
}
void VKAPI_PTR DummyInfoAlloc(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope) {}
void VKAPI_PTR DummyInfoFree(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope) {}

TEST_F(NegativeInstanceless, DestroyInstanceAllocationCallbacksCompatibility) {
    TEST_DESCRIPTION("Test vkDestroyInstance with incompatible allocation callbacks.");

    const auto ici = GetInstanceCreateInfo();
    const VkAllocationCallbacks alloc_callbacks = {nullptr,    &DummyAlloc,     &DummyRealloc,
                                                   &DummyFree, &DummyInfoAlloc, &DummyInfoFree};

    {
        VkInstance instance;
        ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance));

        Monitor().SetDesiredError("VUID-vkDestroyInstance-instance-00631");
        vk::DestroyInstance(instance, &alloc_callbacks);
        Monitor().VerifyFound();
    }
}

// When address/tread sanitizer is on, this tends to fail tests after it
TEST_F(NegativeInstanceless, DISABLED_DestroyInstanceHandleLeak) {
    TEST_DESCRIPTION("Test vkDestroyInstance while leaking a VkDevice object.");
    RETURN_IF_SKIP(InitFramework());
    if (!IsPlatformMockICD()) {
        // This test leaks a device (on purpose) and should not be run on a real driver
        GTEST_SKIP() << "This test only runs on the mock ICD";
    }
    const auto ici = GetInstanceCreateInfo();

    VkInstance instance;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance));
    uint32_t physical_device_count = 1;
    VkPhysicalDevice physical_device;
    const VkResult err = vk::EnumeratePhysicalDevices(instance, &physical_device_count, &physical_device);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    ASSERT_EQ(physical_device_count, 1);

    float dqci_priorities[] = {1.0};
    VkDeviceQueueCreateInfo dqci = vku::InitStructHelper();
    dqci.queueFamilyIndex = 0;
    dqci.queueCount = 1;
    dqci.pQueuePriorities = dqci_priorities;

    VkDeviceCreateInfo dci = vku::InitStructHelper();
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;

    VkDevice leaked_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(physical_device, &dci, nullptr, &leaked_device));

    Monitor().SetDesiredError("VUID-vkDestroyInstance-instance-00629");
    vk::DestroyInstance(instance, nullptr);
    Monitor().VerifyFound();
}

VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*,
                                        const char*, void*) {
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_PTR utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                             const VkDebugUtilsMessengerCallbackDataEXT*, void*) {
    return VK_FALSE;
}

#ifndef VK_USE_PLATFORM_ANDROID_KHR
TEST_F(NegativeInstanceless, ExtensionStructsWithoutExtensions) {
    TEST_DESCRIPTION("Create instance with structures in pNext while not including required extensions");

#ifdef VK_USE_PLATFORM_METAL_EXT
    AddRequiredExtensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo ici = vku::InitStructHelper();
#if defined(VK_USE_PLATFORM_METAL_EXT)
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    ici.pApplicationInfo = &app_info_;
    ici.enabledLayerCount = size32(instance_layers_);
    ici.ppEnabledLayerNames = instance_layers_.data();
    ici.enabledExtensionCount = size32(m_instance_extension_names);
    ici.ppEnabledExtensionNames = m_instance_extension_names.data();

    VkInstance instance;
    VkDebugReportCallbackCreateInfoEXT debug_report_callback = vku::InitStructHelper();
    debug_report_callback.pfnCallback = callback;
    debug_report_callback.pNext = m_errorMonitor->GetDebugCreateInfo();
    ici.pNext = &debug_report_callback;
    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-pNext-04925");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();

    VkDirectDriverLoadingInfoLUNARG driver = vku::InitStructHelper();

    VkDirectDriverLoadingListLUNARG direct_driver_loading_list = vku::InitStructHelper();
    direct_driver_loading_list.pNext = const_cast<VkDebugUtilsMessengerCreateInfoEXT*>(m_errorMonitor->GetDebugCreateInfo());
    direct_driver_loading_list.driverCount = 1u;
    direct_driver_loading_list.pDrivers = &driver;
    ici.pNext = &direct_driver_loading_list;
    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-pNext-09400");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();

    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.pNext = m_errorMonitor->GetDebugCreateInfo();
    ici.pNext = &features;
    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-pNext-10243");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();

    // This must be last because it messes up the extension list
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger = vku::InitStructHelper();
    debug_utils_messenger.pNext = m_errorMonitor->GetDebugCreateInfo();
    debug_utils_messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_utils_messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debug_utils_messenger.pfnUserCallback = utils_callback;
    ici.pNext = &debug_utils_messenger;
    // Ignore the first extension which is VK_EXT_debug_utils
    ici.enabledExtensionCount = size32(m_instance_extension_names) - 1;
    if (ici.enabledExtensionCount > 0) {
        ici.ppEnabledExtensionNames = &m_instance_extension_names[1];
    }
    m_errorMonitor->SetDesiredError("VUID-VkInstanceCreateInfo-pNext-04926");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();
}
#endif

// The test works, you will see the errors, but the test framework is not setup to hook into the debug callback before the create
// instance so no way to detect it
TEST_F(NegativeInstanceless, DISABLED_VkLayerSettingEXT) {
    const char* ids[] = {"something"};
    VkLayerSettingEXT setting = {nullptr, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = vku::InitStructHelper();
    layer_settings_create_info.settingCount = 1;
    layer_settings_create_info.pSettings = &setting;

    auto ici = GetInstanceCreateInfo();
    ici.pNext = &layer_settings_create_info;

    {
        Monitor().SetDesiredError("VUID-VkLayerSettingEXT-pLayerName-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        setting = {OBJECT_LAYER_NAME, nullptr, VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
        Monitor().SetDesiredError("VUID-VkLayerSettingEXT-pSettingName-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, nullptr};
        Monitor().SetDesiredError("VUID-VkLayerSettingEXT-valueCount-10070");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }

    {
        layer_settings_create_info.pSettings = nullptr;
        Monitor().SetDesiredError("VUID-VkLayerSettingsCreateInfoEXT-pSettings-parameter");
        vk::CreateInstance(&ici, nullptr, &dummy_instance);
        Monitor().VerifyFound();
    }
}

// Test for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9181
TEST_F(NegativeInstanceless, DestroyDeviceRace) {
    TEST_DESCRIPTION("Test vkDestroyDevice from multiple threads to make sure handle lookup isn't accessing stale data");
    RETURN_IF_SKIP(InitFramework());
    const auto ici = GetInstanceCreateInfo();

    VkInstance instance;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance));
    uint32_t physical_device_count = 1;
    VkPhysicalDevice physical_device;
    const VkResult err = vk::EnumeratePhysicalDevices(instance, &physical_device_count, &physical_device);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    ASSERT_EQ(physical_device_count, 1);

    float dqci_priorities[] = {1.0};
    VkDeviceQueueCreateInfo dqci = vku::InitStructHelper();
    dqci.queueFamilyIndex = 0;
    dqci.queueCount = 1;
    dqci.pQueuePriorities = dqci_priorities;

    VkDeviceCreateInfo dci = vku::InitStructHelper();
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;

    VkDevice device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(physical_device, &dci, nullptr, &device));

    const auto& destroy_func = [device]() { vk::DestroyDevice(device, nullptr); };
    std::thread destroy_thread(destroy_func);
    destroy_thread.join();

    VkDevice device2;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(physical_device, &dci, nullptr, &device2));

    vk::DestroyDevice(device2, nullptr);
    vk::DestroyInstance(instance, nullptr);
}
