/*
 * Copyright (c) 2022 The Khronos Group Inc.
 * Copyright (c) 2022 Valve Corporation
 * Copyright (c) 2022 LunarG, Inc.
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

//
// VK_EXT_debug_report specific tests
// =========================================
//

// Prototype declaration for callback so we can use it in class utility methods
VkBool32 VKAPI_CALL test_DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                             size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                             void* pUserData);

class DebugReportTest : public ::testing::Test {
   public:
    void VerifyExpected(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, const std::string& message) {
        if (object_type == expected_object_type && 0 < (flags | expected_flag)) {
            if (allow_any_message || (std::string::npos != message.find(expected_message))) {
                message_found = true;
            }
        }
    }

   protected:
    virtual void SetUp() {
        env = std::unique_ptr<FrameworkEnvironment>(new FrameworkEnvironment());
        for (uint32_t icd = 0; icd < 3; ++icd) {
            env->add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_0));
            env->get_test_icd(icd).physical_devices.push_back({});
            env->get_test_icd(icd).physical_devices.push_back({});
        }
        // Initialize the expected output
        allow_any_message = false;
        expected_message = "";
        expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        message_found = false;
    }

    VkResult CreateReportInstance(VkDebugReportFlagsEXT debug_report_flags, VkInstance* inst,
                                  VkApplicationInfo* app_info = nullptr) {
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        // Setup the debug report struct
        VkDebugReportCallbackCreateInfoEXT debug_report_info{VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT};
        debug_report_info.pNext = nullptr;
        debug_report_info.flags = debug_report_flags;
        debug_report_info.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(test_DebugReportCallback);
        debug_report_info.pUserData = reinterpret_cast<void*>(this);

        // Pass it into instance create struct
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = &debug_report_info;
        create_info.pApplicationInfo = app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();

        return env->vulkan_functions.vkCreateInstance(&create_info, nullptr, inst);
    }

    VkResult CreateReportCallback(VkInstance inst, VkDebugReportFlagsEXT debug_report_flags, VkDebugReportCallbackEXT* callback) {
        PFN_vkCreateDebugReportCallbackEXT create_debug_report = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT"));
        if (nullptr == create_debug_report) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        VkDebugReportCallbackCreateInfoEXT debug_report_info{VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT};
        debug_report_info.pNext = nullptr;
        debug_report_info.flags = debug_report_flags;
        debug_report_info.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(test_DebugReportCallback);
        debug_report_info.pUserData = reinterpret_cast<void*>(this);
        return create_debug_report(inst, &debug_report_info, nullptr, callback);
    }

    VkResult DestroyReportCallback(VkInstance inst, VkDebugReportCallbackEXT callback) {
        PFN_vkDestroyDebugReportCallbackEXT destroy_debug_report = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkDestroyDebugReportCallbackEXT"));
        if (nullptr == destroy_debug_report) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        destroy_debug_report(inst, callback, nullptr);
        return VK_SUCCESS;
    }

    virtual void TearDown() { env.reset(); }

    std::unique_ptr<FrameworkEnvironment> env;
    bool allow_any_message;
    std::string expected_message;
    VkDebugReportObjectTypeEXT expected_object_type;
    VkDebugReportFlagBitsEXT expected_flag;
    bool message_found;
};

// This is the actual callback prototyped above.
VkBool32 VKAPI_CALL test_DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                             [[maybe_unused]] uint64_t object, [[maybe_unused]] size_t location,
                                             [[maybe_unused]] int32_t messageCode, [[maybe_unused]] const char* pLayerPrefix,
                                             const char* pMessage, void* pUserData) {
    DebugReportTest* debug_report_test = reinterpret_cast<DebugReportTest*>(pUserData);
    debug_report_test->VerifyExpected(flags, objectType, pMessage);
    return VK_FALSE;
}

class CreateDestroyInstanceReport : public DebugReportTest {};
class SeparateReport : public DebugReportTest {};
class ManualReport : public DebugReportTest {};

// Test creating and destroying instance looking for errors, but none should occur.
TEST_F(CreateDestroyInstanceReport, NoCallback) {
    // Make sure we don't find any errors
    allow_any_message = true;
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(false, message_found);
}

// Test report (error) created in vkCreateInstance with warning in vkCreateInstance
TEST_F(CreateDestroyInstanceReport, WarnInCreateIgnored) {
    expected_message = "The API Variant specified";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkApplicationInfo app_info;
    app_info.apiVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst, &app_info));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should NOT be found (because we only have errors reported in create)
    ASSERT_EQ(false, message_found);
}

// Test creating and destroying instance looking for errors, but none should occur.
TEST_F(CreateDestroyInstanceReport, WarnInCreate) {
    expected_message = "The API Variant specified";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkApplicationInfo app_info;
    app_info.apiVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_WARNING_BIT_EXT, &inst, &app_info));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found (because we only have errors reported in create)
    ASSERT_EQ(true, message_found);
}

// Test report (error/warning) created in vkCreateInstance with error in vkEnumeratePhysicalDevices.
// This should not be logged because we have only defined the debug report logging for vkCreateInstance
// and vkDestroyInstance.
TEST_F(SeparateReport, ErrorInEnumDevsNoCallback) {
    // Look for the invaid count param message
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should NOT be found (because we don't have a report callback setup outside of the create/destroy instance chain)
    ASSERT_EQ(false, message_found);
}

// Test report created outside of vkCreateInstance with error in vkEnumeratePhysicalDevices.
// This should be logged now.
TEST_F(SeparateReport, ErrorInEnumDevs) {
    // Look for the invaid count param message
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_ERROR_BIT_EXT, &callback));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found
    ASSERT_EQ(true, message_found);
}

// Test report created outside of vkCreateInstance with info in vkEnumeratePhysicalDevices.
// This should not be logged because type is wrong.
TEST_F(SeparateReport, InfoInEnumDevsIgnored) {
    expected_message = "Trimming device count from 6 to 5";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_ERROR_BIT_EXT, &callback));

    uint32_t max_count = 5;
    std::array<VkPhysicalDevice, 5> devices;
    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, &max_count, devices.data()), VK_INCOMPLETE);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found (because it's info)
    ASSERT_EQ(false, message_found);
}

// Test report created outside of vkCreateInstance with info in vkEnumeratePhysicalDevices.
// This should be logged because type is correct.
TEST_F(SeparateReport, InfoInEnumDevs) {
    expected_message = "Trimming device count from 6 to 5";
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, &callback));

    uint32_t max_count = 5;
    std::array<VkPhysicalDevice, 5> devices;
    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, &max_count, devices.data()), VK_INCOMPLETE);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found
    ASSERT_EQ(true, message_found);
}

// Test report created outside of vkCreateInstance with a manual info message of the wrong message flag type to be logged.
TEST_F(ManualReport, InfoIgnoredWrongType) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_ERROR_BIT_EXT, &callback));

    PFN_vkDebugReportMessageEXT log_debug_report =
        reinterpret_cast<PFN_vkDebugReportMessageEXT>(env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkDebugReportMessageEXT"));
    ASSERT_NE(nullptr, log_debug_report);
    log_debug_report(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, (uint64_t)(inst), 0, 0,
                     nullptr, my_message);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found
    ASSERT_EQ(false, message_found);
}

// Test report created outside of vkCreateInstance with a manual info message of the wrong object type to be logged.
TEST_F(ManualReport, InfoIgnoredWrongObject) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, &callback));

    PFN_vkDebugReportMessageEXT log_debug_report =
        reinterpret_cast<PFN_vkDebugReportMessageEXT>(env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkDebugReportMessageEXT"));
    ASSERT_NE(nullptr, log_debug_report);
    log_debug_report(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, (uint64_t)(inst), 0, 0,
                     nullptr, my_message);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found
    ASSERT_EQ(false, message_found);
}

// Test report created outside of vkCreateInstance with a manual info message to be logged.
TEST_F(ManualReport, InfoMessage) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    expected_flag = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateReportInstance(VK_DEBUG_REPORT_ERROR_BIT_EXT, &inst));

    VkDebugReportCallbackEXT callback;
    ASSERT_EQ(VK_SUCCESS, CreateReportCallback(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, &callback));

    PFN_vkDebugReportMessageEXT log_debug_report =
        reinterpret_cast<PFN_vkDebugReportMessageEXT>(env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkDebugReportMessageEXT"));
    ASSERT_NE(nullptr, log_debug_report);
    log_debug_report(inst, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, (uint64_t)(inst), 0, 0,
                     nullptr, my_message);

    ASSERT_EQ(VK_SUCCESS, DestroyReportCallback(inst, callback));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found
    ASSERT_EQ(true, message_found);
}

//
// VK_EXT_debug_util specific tests
// =========================================
//

// Prototype declaration for callback so we can use it in class utility methods
VkBool32 VKAPI_CALL test_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                            VkDebugUtilsMessageTypeFlagsEXT message_types,
                                            const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

class DebugUtilTest : public ::testing::Test {
   public:
    void VerifyExpected(VkDebugUtilsMessageTypeFlagsEXT message_flags, VkDebugUtilsMessageSeverityFlagsEXT severity_flags,
                        const std::string& message, const VkDebugUtilsMessengerCallbackDataEXT* callback_data) {
        if ((0 < (severity_flags | expected_severity_flags)) && (0 < (message_flags | expected_message_flags))) {
            if (allow_any_message || (std::string::npos != message.find(expected_message))) {
                for (uint32_t obj = 0; obj < callback_data->objectCount; ++obj) {
                    if (callback_data->pObjects[obj].objectType == expected_object_type &&
                        (!check_object_handle || callback_data->pObjects[obj].objectHandle == expected_object_handle)) {
                        message_found = true;
                        break;
                    }
                }
            }
        }
    }

   protected:
    virtual void SetUp() {
        env = std::unique_ptr<FrameworkEnvironment>(new FrameworkEnvironment());
        for (uint32_t icd = 0; icd < 3; ++icd) {
            env->add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_0));
            env->get_test_icd(icd).physical_devices.push_back({});
            env->get_test_icd(icd).physical_devices.push_back({});
        }
        // Initialize the expected output
        allow_any_message = false;
        expected_message = "";
        expected_object_type = VK_OBJECT_TYPE_UNKNOWN;
        check_object_handle = false;
        expected_object_handle = 0;
        message_found = false;
    }

    VkResult CreateUtilsInstance(VkDebugUtilsMessageTypeFlagsEXT types, VkDebugUtilsMessageSeverityFlagsEXT severities,
                                 VkInstance* inst, VkApplicationInfo* app_info = nullptr) {
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Setup the debug utils struct
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug_utils_info.pNext = nullptr;
        debug_utils_info.messageSeverity = severities;
        debug_utils_info.messageType = types;
        debug_utils_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(test_DebugUtilsCallback);
        debug_utils_info.pUserData = reinterpret_cast<void*>(this);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = &debug_utils_info;
        create_info.pApplicationInfo = app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();
        return env->vulkan_functions.vkCreateInstance(&create_info, nullptr, inst);
    }

    VkResult CreateUtilsMessenger(VkInstance inst, VkDebugUtilsMessageTypeFlagsEXT types,
                                  VkDebugUtilsMessageSeverityFlagsEXT severities, VkDebugUtilsMessengerEXT* messenger) {
        PFN_vkCreateDebugUtilsMessengerEXT create_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT"));
        if (nullptr == create_messenger) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        debug_utils_info.pNext = nullptr;
        debug_utils_info.messageSeverity = severities;
        debug_utils_info.messageType = types;
        debug_utils_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(test_DebugUtilsCallback);
        debug_utils_info.pUserData = reinterpret_cast<void*>(this);
        return create_messenger(inst, &debug_utils_info, nullptr, messenger);
    }

    VkResult DestroyUtilsMessenger(VkInstance inst, VkDebugUtilsMessengerEXT messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT"));
        if (nullptr == destroy_messenger) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        destroy_messenger(inst, messenger, nullptr);
        return VK_SUCCESS;
    }

    virtual void TearDown() { env.reset(); }

    std::unique_ptr<FrameworkEnvironment> env;
    bool allow_any_message;
    std::string expected_message;
    VkObjectType expected_object_type;
    bool check_object_handle;
    uint64_t expected_object_handle;
    VkDebugUtilsMessageTypeFlagsEXT expected_message_flags;
    VkDebugUtilsMessageSeverityFlagsEXT expected_severity_flags;
    bool message_found;
};

// This is the actual callback prototyped above.

VkBool32 VKAPI_CALL test_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                            VkDebugUtilsMessageTypeFlagsEXT message_types,
                                            const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
    DebugUtilTest* debug_util_test = reinterpret_cast<DebugUtilTest*>(user_data);
    debug_util_test->VerifyExpected(message_types, message_severity, callback_data->pMessage, callback_data);
    return VK_FALSE;
}

class CreateDestroyInstanceMessenger : public DebugUtilTest {};
class SeparateMessenger : public DebugUtilTest {};
class ManualMessage : public DebugUtilTest {};

// Test creating and destroying instance looking for errors, but none should occur.
TEST_F(CreateDestroyInstanceMessenger, NoCallback) {
    // Make sure we don't find any errors
    allow_any_message = true;
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(false, message_found);
}

// Test debug utils (error) created in vkCreateInstance with warning in vkCreateInstance
TEST_F(CreateDestroyInstanceMessenger, WarnInCreateIgnored) {
    expected_message = "The API Variant specified";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    VkApplicationInfo app_info;
    app_info.apiVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst, &app_info));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(false, message_found);
}

// Test debug utils (error/warning) created in vkCreateInstance with warning in vkCreateInstance
TEST_F(CreateDestroyInstanceMessenger, WarnInCreate) {
    expected_message = "The API Variant specified";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    VkApplicationInfo app_info;
    app_info.apiVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, &inst, &app_info));
    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(true, message_found);
}

// Test debug utils error created in vkCreateInstance with error in vkEnumeratePhysicalDevices.
// This should not be logged because we have only defined the debug utils logging for vkCreateInstance
// and vkDestroyInstance.
TEST_F(SeparateMessenger, ErrorInEnumDevsNoMessenger) {
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should NOT be found (because we don't have a report callback setup outside of the create/destroy instance chain)
    ASSERT_EQ(false, message_found);
}

// Test debug utils created outside of vkCreateInstance with error in vkEnumeratePhysicalDevices, but, with the wrong
// message type so it still won't be logged.
TEST_F(SeparateMessenger, ErrorInEnumDevsWrongType) {
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect performance warnings and errors
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                   &messenger));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(false, message_found);
}

// Test debug utils created outside of vkCreateInstance with error in vkEnumeratePhysicalDevices, but, with the wrong
// message severity so it still won't be logged.
TEST_F(SeparateMessenger, ErrorInEnumDevsWrongSeverity) {
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect only validation warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, &messenger));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(false, message_found);
}

// Test debug utils created outside of vkCreateInstance with error in vkEnumeratePhysicalDevices with the correct type.
TEST_F(SeparateMessenger, ErrorInEnumDevs) {
    expected_message = "VUID-vkEnumeratePhysicalDevices-pPhysicalDeviceCount-parameter";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect validation errors
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &messenger));

    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, nullptr, nullptr), VK_ERROR_INITIALIZATION_FAILED);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    ASSERT_EQ(true, message_found);
}

// Test debug utils created outside of vkCreateInstance with info in vkEnumeratePhysicalDevices.
// This should not be logged because type is wrong.
TEST_F(SeparateMessenger, InfoInEnumDevsIgnoredType) {
    expected_message = "Trimming device count from 6 to 5";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect validation info
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, &messenger));

    uint32_t max_count = 5;
    std::array<VkPhysicalDevice, 5> devices;
    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, &max_count, devices.data()), VK_INCOMPLETE);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found (because it's info)
    ASSERT_EQ(false, message_found);
}

// Test debug utils created outside of vkCreateInstance with info in vkEnumeratePhysicalDevices.
// This should not be logged because severity is wrong.
TEST_F(SeparateMessenger, InfoInEnumDevsIgnoredSeverity) {
    expected_message = "Trimming device count from 6 to 5";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect general errors/warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                   &messenger));

    uint32_t max_count = 5;
    std::array<VkPhysicalDevice, 5> devices;
    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, &max_count, devices.data()), VK_INCOMPLETE);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found (because it's info)
    ASSERT_EQ(false, message_found);
}

// Test debug utils created outside of vkCreateInstance with info in vkEnumeratePhysicalDevices.
TEST_F(SeparateMessenger, InfoInEnumDevs) {
    expected_message = "Trimming device count from 6 to 5";
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));
    ASSERT_EQ(false, message_found);

    // Create the debug utils messenger to collect general errors/warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, &messenger));

    uint32_t max_count = 5;
    std::array<VkPhysicalDevice, 5> devices;
    ASSERT_EQ(env->vulkan_functions.vkEnumeratePhysicalDevices(inst, &max_count, devices.data()), VK_INCOMPLETE);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found
    ASSERT_EQ(true, message_found);
}

// Test messenger created outside of vkCreateInstance with a manual info message of the wrong message severity to be logged.
TEST_F(ManualMessage, InfoMessageIgnoredSeverity) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect general errors/warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS,
              CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                                   &messenger));

    // Trigger the message
    PFN_vkSubmitDebugUtilsMessageEXT submit_message = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(
        env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkSubmitDebugUtilsMessageEXT"));
    ASSERT_NE(nullptr, submit_message);

    VkDebugUtilsObjectNameInfoEXT object{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    object.objectType = VK_OBJECT_TYPE_INSTANCE;
    object.objectHandle = (uint64_t)inst;
    VkDebugUtilsMessengerCallbackDataEXT message_data{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT};
    message_data.pMessage = my_message;
    message_data.objectCount = 1;
    message_data.pObjects = &object;
    submit_message(inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &message_data);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found
    ASSERT_EQ(false, message_found);
}

// Test messenger created outside of vkCreateInstance with a manual info message of the wrong object type to be logged.
TEST_F(ManualMessage, InfoMessageIgnoredObject) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect general errors/warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, &messenger));

    // Trigger the message
    PFN_vkSubmitDebugUtilsMessageEXT submit_message = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(
        env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkSubmitDebugUtilsMessageEXT"));
    ASSERT_NE(nullptr, submit_message);

    VkDebugUtilsObjectNameInfoEXT object{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    object.objectType = VK_OBJECT_TYPE_COMMAND_POOL;
    object.objectHandle = (uint64_t)inst;
    VkDebugUtilsMessengerCallbackDataEXT message_data{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT};
    message_data.pMessage = my_message;
    message_data.objectCount = 1;
    message_data.pObjects = &object;
    submit_message(inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &message_data);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should not be found
    ASSERT_EQ(false, message_found);
}

// Test messenger created outside of vkCreateInstance with a manual info message.
TEST_F(ManualMessage, InfoMessage) {
    const char my_message[] = "This is my special message!";
    expected_message = my_message;
    expected_object_type = VK_OBJECT_TYPE_INSTANCE;
    expected_message_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    expected_severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkInstance inst = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsInstance(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, &inst));

    // Create the debug utils messenger to collect general errors/warnings
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, CreateUtilsMessenger(inst, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, &messenger));

    // Trigger the message
    PFN_vkSubmitDebugUtilsMessageEXT submit_message = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(
        env->vulkan_functions.vkGetInstanceProcAddr(inst, "vkSubmitDebugUtilsMessageEXT"));
    ASSERT_NE(nullptr, submit_message);

    VkDebugUtilsObjectNameInfoEXT object{VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    object.objectType = VK_OBJECT_TYPE_INSTANCE;
    object.objectHandle = (uint64_t)inst;
    VkDebugUtilsMessengerCallbackDataEXT message_data{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT};
    message_data.pMessage = my_message;
    message_data.objectCount = 1;
    message_data.pObjects = &object;
    submit_message(inst, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, &message_data);

    // Now that we should have gotten our message, destroy the messenger
    ASSERT_EQ(VK_SUCCESS, DestroyUtilsMessenger(inst, messenger));

    env->vulkan_functions.vkDestroyInstance(inst, nullptr);

    // Message should be found
    ASSERT_EQ(true, message_found);
}

void CheckDeviceFunctions(FrameworkEnvironment& env, bool use_GIPA, bool enable_debug_extensions,
                          bool hardware_supports_debug_exts) {
    InstWrapper inst(env.vulkan_functions);
    if (enable_debug_extensions) {
        inst.create_info.add_extension("VK_EXT_debug_utils");
        inst.create_info.add_extension("VK_EXT_debug_report");
    }
    inst.create_info.setup_WSI();
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    auto phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_extension("VK_KHR_swapchain");
    dev.create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));
    if (enable_debug_extensions) {
        dev.create_info.add_extension("VK_EXT_debug_marker");
    }

    if (enable_debug_extensions && !hardware_supports_debug_exts) {
        // if the hardware doesn't support VK_EXT_debug_marker and we are trying to enable it, then we should exit since that will
        // fail to create a device

        dev.CheckCreate(phys_dev, VK_ERROR_EXTENSION_NOT_PRESENT);
        return;
    } else {
        ASSERT_NO_FATAL_FAILURE(dev.CheckCreate(phys_dev));
    }
    DeviceFunctions dev_funcs{env.vulkan_functions, dev};

    VkSurfaceKHR surface{};
    ASSERT_EQ(VK_SUCCESS, create_surface(inst, surface));

    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = surface;

    VkSwapchainKHR swapchain{};
    ASSERT_EQ(VK_SUCCESS, dev_funcs.vkCreateSwapchainKHR(dev.dev, &info, nullptr, &swapchain));

    auto load_function = [&inst, &dev, use_GIPA](const char* func_name) {
        return use_GIPA ? inst.load(func_name) : dev.load(func_name);
    };

    // Debug marker
    PFN_vkDebugMarkerSetObjectTagEXT DebugMarkerSetObjectTagEXT = load_function("vkDebugMarkerSetObjectTagEXT");
    PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT = load_function("vkDebugMarkerSetObjectNameEXT");
    PFN_vkCmdDebugMarkerBeginEXT CmdDebugMarkerBeginEXT = load_function("vkCmdDebugMarkerBeginEXT");
    PFN_vkCmdDebugMarkerEndEXT CmdDebugMarkerEndEXT = load_function("vkCmdDebugMarkerEndEXT");
    PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT = load_function("vkCmdDebugMarkerInsertEXT");
    // Debug utils
    PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = load_function("vkSetDebugUtilsObjectNameEXT");
    PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT = load_function("vkSetDebugUtilsObjectTagEXT");
    PFN_vkQueueBeginDebugUtilsLabelEXT QueueBeginDebugUtilsLabelEXT = load_function("vkQueueBeginDebugUtilsLabelEXT");
    PFN_vkQueueEndDebugUtilsLabelEXT QueueEndDebugUtilsLabelEXT = load_function("vkQueueEndDebugUtilsLabelEXT");
    PFN_vkQueueInsertDebugUtilsLabelEXT QueueInsertDebugUtilsLabelEXT = load_function("vkQueueInsertDebugUtilsLabelEXT");
    PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT = load_function("vkCmdBeginDebugUtilsLabelEXT");
    PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT = load_function("vkCmdEndDebugUtilsLabelEXT");
    PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT = load_function("vkCmdInsertDebugUtilsLabelEXT");

    // Debug marker functions - should always be found when using GIPA but when using GDPA found only when the extension is enabled
    if (use_GIPA) {
        ASSERT_TRUE(nullptr != DebugMarkerSetObjectTagEXT);
        ASSERT_TRUE(nullptr != DebugMarkerSetObjectNameEXT);
        ASSERT_TRUE(nullptr != CmdDebugMarkerBeginEXT);
        ASSERT_TRUE(nullptr != CmdDebugMarkerEndEXT);
        ASSERT_TRUE(nullptr != CmdDebugMarkerInsertEXT);
    } else {
        ASSERT_EQ(enable_debug_extensions, nullptr != DebugMarkerSetObjectTagEXT);
        ASSERT_EQ(enable_debug_extensions, nullptr != DebugMarkerSetObjectNameEXT);
        ASSERT_EQ(enable_debug_extensions, nullptr != CmdDebugMarkerBeginEXT);
        ASSERT_EQ(enable_debug_extensions, nullptr != CmdDebugMarkerEndEXT);
        ASSERT_EQ(enable_debug_extensions, nullptr != CmdDebugMarkerInsertEXT);
    }

    // Debug utils functions - should only be found if the extension was enabled (because its instance level)
    ASSERT_EQ(enable_debug_extensions, nullptr != SetDebugUtilsObjectNameEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != SetDebugUtilsObjectTagEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != QueueBeginDebugUtilsLabelEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != QueueEndDebugUtilsLabelEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != QueueInsertDebugUtilsLabelEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != CmdBeginDebugUtilsLabelEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != CmdEndDebugUtilsLabelEXT);
    ASSERT_EQ(enable_debug_extensions, nullptr != CmdInsertDebugUtilsLabelEXT);

    if (SetDebugUtilsObjectNameEXT) {
        VkDebugUtilsObjectNameInfoEXT obj_name_info{};
        obj_name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        obj_name_info.objectHandle = (uint64_t)swapchain;
        obj_name_info.objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        obj_name_info.pObjectName = " Your mom!";
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(dev.dev, &obj_name_info));

        obj_name_info.objectHandle = (uint64_t)(uintptr_t)surface;
        obj_name_info.objectType = VK_OBJECT_TYPE_SURFACE_KHR;
        obj_name_info.pObjectName = " Your moms surface!";
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(dev.dev, &obj_name_info));

        obj_name_info.objectHandle = (uint64_t)(uintptr_t)phys_dev;
        obj_name_info.objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        obj_name_info.pObjectName = "Physical Device AAAAAAAAA";
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(dev.dev, &obj_name_info));

        obj_name_info.objectHandle = (uint64_t)(uintptr_t)inst.inst;
        obj_name_info.objectType = VK_OBJECT_TYPE_INSTANCE;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(dev.dev, &obj_name_info));
    }
    if (SetDebugUtilsObjectTagEXT) {
        VkDebugUtilsObjectTagInfoEXT utils_object_tag{};
        utils_object_tag.objectHandle = (uint64_t)(uintptr_t)inst.inst;
        utils_object_tag.objectType = VK_OBJECT_TYPE_INSTANCE;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(dev.dev, &utils_object_tag));

        utils_object_tag.objectHandle = (uint64_t)(uintptr_t)phys_dev;
        utils_object_tag.objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(dev.dev, &utils_object_tag));

        utils_object_tag.objectHandle = (uint64_t)surface;
        utils_object_tag.objectType = VK_OBJECT_TYPE_SURFACE_KHR;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(dev.dev, &utils_object_tag));
    }
    VkDebugMarkerObjectTagInfoEXT marker_object_tag{};
    VkDebugMarkerObjectNameInfoEXT marker_object_name{};
    if (use_GIPA && !enable_debug_extensions) {
        // These functions crash when the extension isn't enabled and the function was acquired with GIPA.
        ASSERT_DEATH(DebugMarkerSetObjectTagEXT(dev.dev, &marker_object_tag), "");
        ASSERT_DEATH(DebugMarkerSetObjectNameEXT(dev.dev, &marker_object_name), "");
    } else {
        if (DebugMarkerSetObjectTagEXT) {
            marker_object_tag.object = (uint64_t)(uintptr_t)swapchain;
            marker_object_tag.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(dev.dev, &marker_object_tag));

            marker_object_tag.object = (uint64_t)(uintptr_t)phys_dev;
            marker_object_tag.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(dev.dev, &marker_object_tag));

            marker_object_tag.object = (uint64_t)surface;
            marker_object_tag.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(dev.dev, &marker_object_tag));

            marker_object_tag.object = (uint64_t)(uintptr_t)inst.inst;
            marker_object_tag.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(dev.dev, &marker_object_tag));
        }
        if (DebugMarkerSetObjectNameEXT) {
            marker_object_name.object = (uint64_t)(uintptr_t)swapchain;
            marker_object_name.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(dev.dev, &marker_object_name));

            marker_object_name.object = (uint64_t)(uintptr_t)phys_dev;
            marker_object_name.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(dev.dev, &marker_object_name));

            marker_object_name.object = (uint64_t)surface;
            marker_object_name.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(dev.dev, &marker_object_name));

            marker_object_name.object = (uint64_t)(uintptr_t)inst.inst;
            marker_object_name.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
            ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(dev.dev, &marker_object_name));
        }
    }
    VkQueue queue{};
    dev.functions->vkGetDeviceQueue(dev.dev, 0, 0, &queue);
    VkDebugUtilsLabelEXT utils_label{};
    utils_label.pLabelName = "Testing testing 123";
    if (QueueBeginDebugUtilsLabelEXT) QueueBeginDebugUtilsLabelEXT(queue, &utils_label);
    if (QueueEndDebugUtilsLabelEXT) QueueEndDebugUtilsLabelEXT(queue);
    if (QueueInsertDebugUtilsLabelEXT) QueueInsertDebugUtilsLabelEXT(queue, &utils_label);
    VkCommandBuffer cmd_buf{};
    VkCommandPool cmd_pool;
    VkCommandPoolCreateInfo cmd_pool_info{};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ASSERT_EQ(VK_SUCCESS, dev_funcs.vkCreateCommandPool(dev.dev, &cmd_pool_info, nullptr, &cmd_pool));
    VkCommandBufferAllocateInfo cmd_buf_alloc_info{};
    cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buf_alloc_info.commandBufferCount = 1;
    cmd_buf_alloc_info.commandPool = cmd_pool;
    ASSERT_EQ(VK_SUCCESS, dev_funcs.vkAllocateCommandBuffers(dev.dev, &cmd_buf_alloc_info, &cmd_buf));
    if (CmdBeginDebugUtilsLabelEXT) CmdBeginDebugUtilsLabelEXT(cmd_buf, &utils_label);
    if (CmdEndDebugUtilsLabelEXT) CmdEndDebugUtilsLabelEXT(cmd_buf);
    if (CmdInsertDebugUtilsLabelEXT) CmdInsertDebugUtilsLabelEXT(cmd_buf, &utils_label);

    dev_funcs.vkDestroySwapchainKHR(dev.dev, swapchain, nullptr);
    env.vulkan_functions.vkDestroySurfaceKHR(inst.inst, surface, nullptr);
}

TEST(GetProcAddr, DebugFuncsWithTerminator) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).setup_WSI().add_physical_device("physical_device_0");
    driver.physical_devices.at(0).add_extensions({"VK_KHR_swapchain"});
    // Hardware doesn't support the debug extensions

    // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, false));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, false));

    // Now set the hardware to support the extensions and run the situations again
    driver.add_instance_extensions({"VK_EXT_debug_utils", "VK_EXT_debug_report"});
    driver.physical_devices.at(0).add_extensions({"VK_EXT_debug_marker"});

    // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, true));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, true));
}

TEST(GetProcAddr, DebugFuncsWithTrampoline) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).setup_WSI().add_physical_device("physical_device_0");
    driver.physical_devices.at(0).add_extensions({"VK_KHR_swapchain"});
    // Hardware doesn't support the debug extensions

    // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, false));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, false));

    // Now add a layer that supports the extensions and run the situations again
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_test_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_instance_extensions({{VK_EXT_DEBUG_REPORT_EXTENSION_NAME},
                                                                                   {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}})
                                                         .add_device_extension({VK_EXT_DEBUG_MARKER_EXTENSION_NAME})),
                           "test_layer.json");

    // // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, true));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, true));
}

TEST(GetProcAddr, DebugFuncsWithDebugExtsForceAdded) {
    FrameworkEnvironment env{};
    auto& driver =
        env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).setup_WSI().add_physical_device("physical_device_0");
    driver.physical_devices.at(0).add_extensions({"VK_KHR_swapchain"});
    // Hardware doesn't support the debug extensions

    // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, false));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, false));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, false));

    // Now add a layer that supports the extensions and run the situations again
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_test_layer")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")),
                           "test_layer.json");
    env.get_test_layer()
        .add_injected_instance_extensions({{VK_EXT_DEBUG_REPORT_EXTENSION_NAME}, {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}})
        .add_injected_device_extension({VK_EXT_DEBUG_MARKER_EXTENSION_NAME});

    // Use getDeviceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, false, true, true));

    // Use getInstanceProcAddr & vary enabling the debug extensions
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, false, true));
    ASSERT_NO_FATAL_FAILURE(CheckDeviceFunctions(env, true, true, true));
}

TEST(DebugUtils, WrappingLayer) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2))
        .set_min_icd_interface_version(5)
        .add_physical_device(PhysicalDevice{}.add_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME).finish())
        .add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const char* wrap_objects_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(wrap_objects_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .add_instance_extension({VK_EXT_DEBUG_UTILS_EXTENSION_NAME})),
                           "wrap_objects_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    inst.create_info.add_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    inst.create_info.add_layer(wrap_objects_name);
    inst.CheckCreate();
    DebugUtilsWrapper log{inst};
    CreateDebugUtilsMessenger(log);

    auto phys_dev = inst.GetPhysDev();
    DeviceWrapper device{inst};
    device.create_info.add_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    device.CheckCreate(phys_dev);
    {
        PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT = inst.load("vkSetDebugUtilsObjectNameEXT");

        VkDebugUtilsObjectNameInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        info.objectType = VK_OBJECT_TYPE_DEVICE;
        info.objectHandle = (uint64_t)device.dev;
        info.pObjectName = "Test Name";
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(device, &info));

        info.objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        info.objectHandle = (uint64_t)phys_dev;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(device, &info));

        info.objectType = VK_OBJECT_TYPE_INSTANCE;
        info.objectHandle = (uint64_t)inst.inst;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectNameEXT(device, &info));
    }
    {
        PFN_vkSetDebugUtilsObjectTagEXT SetDebugUtilsObjectTagEXT = inst.load("vkSetDebugUtilsObjectTagEXT");

        VkDebugUtilsObjectTagInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
        info.objectType = VK_OBJECT_TYPE_DEVICE;
        info.objectHandle = (uint64_t)device.dev;
        info.pTag = "Test Name";
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(device, &info));

        info.objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        info.objectHandle = (uint64_t)phys_dev;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(device, &info));

        info.objectType = VK_OBJECT_TYPE_INSTANCE;
        info.objectHandle = (uint64_t)inst.inst;
        ASSERT_EQ(VK_SUCCESS, SetDebugUtilsObjectTagEXT(device, &info));
    }
    // Debug marker
    {
        PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT = inst.load("vkDebugMarkerSetObjectNameEXT");

        VkDebugMarkerObjectNameInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
        info.object = (uint64_t)device.dev;
        info.pObjectName = "Test Name";
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(device, &info));

        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
        info.object = (uint64_t)phys_dev;
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(device, &info));

        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
        info.object = (uint64_t)inst.inst;
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectNameEXT(device, &info));
    }
    {
        PFN_vkDebugMarkerSetObjectTagEXT DebugMarkerSetObjectTagEXT = inst.load("vkDebugMarkerSetObjectTagEXT");

        VkDebugMarkerObjectTagInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
        info.object = (uint64_t)device.dev;
        info.pTag = "Test Name";
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(device, &info));

        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
        info.object = (uint64_t)phys_dev;
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(device, &info));

        info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
        info.object = (uint64_t)inst.inst;
        ASSERT_EQ(VK_SUCCESS, DebugMarkerSetObjectTagEXT(device, &info));
    }
}
