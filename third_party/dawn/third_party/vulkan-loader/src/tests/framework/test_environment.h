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

/*
 * The test_environment is what combines the icd, layer, and shim library into a single object that
 * test fixtures can create and use. Responsible for loading the libraries and establishing the
 * channels for tests to talk with the icd's and layers.
 */
#pragma once

// Must include gtest first to guard against Xlib colliding due to redefinitions of "None" and "Bool"
#if defined(VK_USE_PLATFORM_XLIB_KHR)
#pragma push_macro("None")
#pragma push_macro("Bool")
#undef None
#undef Bool
#endif

#if defined(_WIN32)
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#endif

#include "gtest/gtest.h"

#include "util/builder_defines.h"
#include "util/dispatch_table.h"
#include "util/dynamic_library_wrapper.h"
#include "util/env_var_wrapper.h"
#include "util/equality_helpers.h"
#include "util/folder_manager.h"
#include "util/functions.h"
#include "util/manifest_builders.h"
#include "util/test_defines.h"
#include "util/vulkan_object_wrappers.h"

#include "shim/shim.h"

#include "icd/test_icd.h"

#include "layer/test_layer.h"

#include "generated/vk_result_to_string_helper.h"

// handle checking
template <typename T>
void handle_assert_has_value(T const& handle) {
    ASSERT_TRUE(handle != VK_NULL_HANDLE);
}
template <typename T>
void handle_assert_null(T const& handle) {
    ASSERT_TRUE(handle == VK_NULL_HANDLE);
}
template <typename T>
void handle_assert_has_values(std::vector<T> const& handles) {
    for (auto const& handle : handles) {
        ASSERT_TRUE(handle != VK_NULL_HANDLE);
    }
}
template <typename T>
void handle_assert_no_values(std::vector<T> const& handles) {
    for (auto const& handle : handles) {
        ASSERT_TRUE(handle == VK_NULL_HANDLE);
    }
}
template <typename T>
void handle_assert_no_values(size_t length, T handles[]) {
    for (size_t i = 0; i < length; i++) {
        ASSERT_TRUE(handles[i] == VK_NULL_HANDLE);
    }
}
template <typename T>
void handle_assert_equal(T const& left, T const& right) {
    ASSERT_EQ(left, right);
}
template <typename T>
void handle_assert_equal(std::vector<T> const& left, std::vector<T> const& right) {
    ASSERT_EQ(left.size(), right.size());
    for (size_t i = 0; i < left.size(); i++) {
        ASSERT_EQ(left[i], right[i]);
    }
}
template <typename T>
void handle_assert_equal(size_t count, T left[], T right[]) {
    for (size_t i = 0; i < count; i++) {
        ASSERT_EQ(left[i], right[i]);
    }
}

template <typename T, size_t U>
bool check_permutation(std::initializer_list<const char*> expected, std::array<T, U> const& returned) {
    if (expected.size() != returned.size()) return false;
    for (uint32_t i = 0; i < expected.size(); i++) {
        auto found = std::find_if(std::begin(returned), std::end(returned),
                                  [&](T elem) { return string_eq(*(expected.begin() + i), elem.layerName); });
        if (found == std::end(returned)) return false;
    }
    return true;
}
template <typename T>
bool check_permutation(std::initializer_list<const char*> expected, std::vector<T> const& returned) {
    if (expected.size() != returned.size()) return false;
    for (uint32_t i = 0; i < expected.size(); i++) {
        auto found = std::find_if(std::begin(returned), std::end(returned),
                                  [&](T elem) { return string_eq(*(expected.begin() + i), elem.layerName); });
        if (found == std::end(returned)) return false;
    }
    return true;
}

// InstWrapper & DeviceWrapper - used to make creating instances & devices easier when writing tests
struct InstWrapper {
    InstWrapper(VulkanFunctions& functions, VkAllocationCallbacks* callbacks = nullptr) noexcept;
    InstWrapper(VulkanFunctions& functions, VkInstance inst, VkAllocationCallbacks* callbacks = nullptr) noexcept;
    ~InstWrapper() noexcept;

    // Move-only object
    InstWrapper(InstWrapper const&) = delete;
    InstWrapper& operator=(InstWrapper const&) = delete;
    InstWrapper(InstWrapper&& other) noexcept;
    InstWrapper& operator=(InstWrapper&&) noexcept;

    // Construct this VkInstance using googletest to assert if it succeeded
    void CheckCreate(VkResult result_to_check = VK_SUCCESS);
    void CheckCreateWithInfo(InstanceCreateInfo& create_info, VkResult result_to_check = VK_SUCCESS);

    // Convenience
    operator VkInstance() { return inst; }
    VulkanFunctions* operator->() { return functions; }

    FromVoidStarFunc load(const char* func_name) { return FromVoidStarFunc(functions->vkGetInstanceProcAddr(inst, func_name)); }

    // Enumerate physical devices using googletest to assert if it succeeded
    std::vector<VkPhysicalDevice> GetPhysDevs(VkResult result_to_check = VK_SUCCESS);  // query all physical devices
    std::vector<VkPhysicalDevice> GetPhysDevs(uint32_t phys_dev_count,
                                              VkResult result_to_check = VK_SUCCESS);  // query only phys_dev_count devices
    // Enumerate a single physical device using googletest to assert if it succeeded
    VkPhysicalDevice GetPhysDev(VkResult result_to_check = VK_SUCCESS);

    // Get all the list of active layers through vkEnumerateDeviceLayerProperties
    // Use count to specify the expected count
    std::vector<VkLayerProperties> GetActiveLayers(VkPhysicalDevice phys_dev, uint32_t count);

    // Get list of device extensions associated with a VkPhysicalDevice
    // Use count to specify an expected count
    std::vector<VkExtensionProperties> EnumerateDeviceExtensions(VkPhysicalDevice physical_device, uint32_t count);
    // Same as EnumerateDeviceExtensions but for a specific layer
    std::vector<VkExtensionProperties> EnumerateLayerDeviceExtensions(VkPhysicalDevice physical_device, const char* layer_name,
                                                                      uint32_t expected_count);

    VulkanFunctions* functions = nullptr;
    VkInstance inst = VK_NULL_HANDLE;
    VkAllocationCallbacks* callbacks = nullptr;
    InstanceCreateInfo create_info{};
    InstanceFunctions instance_functions{};
};

struct DeviceWrapper {
    DeviceWrapper(InstWrapper& inst_wrapper, VkAllocationCallbacks* callbacks = nullptr) noexcept;
    DeviceWrapper(VulkanFunctions& functions, VkDevice device, VkAllocationCallbacks* callbacks = nullptr) noexcept;
    ~DeviceWrapper() noexcept;

    // Move-only object
    DeviceWrapper(DeviceWrapper const&) = delete;
    DeviceWrapper& operator=(DeviceWrapper const&) = delete;
    DeviceWrapper(DeviceWrapper&&) noexcept;
    DeviceWrapper& operator=(DeviceWrapper&&) noexcept;

    // Construct this VkDevice using googletest to assert if it succeeded
    void CheckCreate(VkPhysicalDevice physical_device, VkResult result_to_check = VK_SUCCESS);

    // Convenience
    operator VkDevice() { return dev; }
    operator VkDevice() const { return dev; }
    VulkanFunctions* operator->() { return functions; }

    FromVoidStarFunc load(const char* func_name) { return FromVoidStarFunc(functions->vkGetDeviceProcAddr(dev, func_name)); }

    VulkanFunctions* functions = nullptr;
    VkDevice dev = VK_NULL_HANDLE;
    VkAllocationCallbacks* callbacks = nullptr;
    DeviceCreateInfo create_info{};
};

template <typename HandleType, typename ParentType, typename DestroyFuncType>
struct WrappedHandle {
    WrappedHandle(HandleType in_handle, ParentType in_parent, DestroyFuncType in_destroy_func,
                  VkAllocationCallbacks* in_callbacks = nullptr)
        : handle(in_handle), parent(in_parent), destroy_func(in_destroy_func), callbacks(in_callbacks) {}
    ~WrappedHandle() {
        if (handle) {
            destroy_func(parent, handle, callbacks);
            handle = VK_NULL_HANDLE;
        }
    }
    WrappedHandle(WrappedHandle const&) = delete;
    WrappedHandle& operator=(WrappedHandle const&) = delete;
    WrappedHandle(WrappedHandle&& other) noexcept
        : handle(other.handle), parent(other.parent), destroy_func(other.destroy_func), callbacks(other.callbacks) {
        other.handle = VK_NULL_HANDLE;
    }
    WrappedHandle& operator=(WrappedHandle&& other) noexcept {
        if (handle != VK_NULL_HANDLE) {
            destroy_func(parent, handle, callbacks);
        }
        handle = other.handle;
        other.handle = VK_NULL_HANDLE;
        parent = other.parent;
        destroy_func = other.destroy_func;
        callbacks = other.callbacks;
        return *this;
    }

    HandleType handle = VK_NULL_HANDLE;
    ParentType parent = VK_NULL_HANDLE;
    DestroyFuncType destroy_func = nullptr;
    VkAllocationCallbacks* callbacks = nullptr;
};

struct DebugUtilsLogger {
    static VkBool32 VKAPI_PTR
    DebugUtilsMessengerLoggerCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                      [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        DebugUtilsLogger* debug = reinterpret_cast<DebugUtilsLogger*>(pUserData);
        debug->returned_output += pCallbackData->pMessage;
        debug->returned_output += '\n';
        return VK_FALSE;
    }
    DebugUtilsLogger(VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        returned_output.reserve(4096);  // output can be very noisy, reserving should help prevent many small allocations
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.pNext = nullptr;
        create_info.messageSeverity = severity;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = DebugUtilsMessengerLoggerCallback;
        create_info.pUserData = this;
    }

    // Immoveable object
    DebugUtilsLogger(DebugUtilsLogger const&) = delete;
    DebugUtilsLogger& operator=(DebugUtilsLogger const&) = delete;
    DebugUtilsLogger(DebugUtilsLogger&&) = delete;
    DebugUtilsLogger& operator=(DebugUtilsLogger&&) = delete;
    // Find a string in the log output
    bool find(std::string const& search_text) const { return returned_output.find(search_text) != std::string::npos; }
    // Find the number of times a string appears in the log output
    uint32_t count(std::string const& search_text) const {
        uint32_t occurrences = 0;
        std::string::size_type position = 0;
        while ((position = returned_output.find(search_text, position)) != std::string::npos) {
            ++occurrences;
            position += search_text.length();
        }
        return occurrences;
    }

    // Look through the event log. If you find a line containing the prefix we're interested in, look for the end of
    // line character, and then see if the postfix occurs in it as well.
    bool find_prefix_then_postfix(const char* prefix, const char* postfix) const;

    // Clear the log
    void clear() { returned_output.clear(); }
    VkDebugUtilsMessengerCreateInfoEXT* get() noexcept { return &create_info; }
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    std::string returned_output;
};

struct DebugUtilsWrapper {
    DebugUtilsWrapper() noexcept {}
    DebugUtilsWrapper(InstWrapper& inst_wrapper,
                      VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                      VkAllocationCallbacks* callbacks = nullptr)
        : logger(severity),
          inst(inst_wrapper.inst),
          callbacks(callbacks),
          local_vkCreateDebugUtilsMessengerEXT(
              FromVoidStarFunc(inst_wrapper.functions->vkGetInstanceProcAddr(inst_wrapper.inst, "vkCreateDebugUtilsMessengerEXT"))),
          local_vkDestroyDebugUtilsMessengerEXT(FromVoidStarFunc(
              inst_wrapper.functions->vkGetInstanceProcAddr(inst_wrapper.inst, "vkDestroyDebugUtilsMessengerEXT"))){};
    ~DebugUtilsWrapper() noexcept {
        if (messenger) {
            local_vkDestroyDebugUtilsMessengerEXT(inst, messenger, callbacks);
            messenger = VK_NULL_HANDLE;
        }
    }
    // Immoveable object
    DebugUtilsWrapper(DebugUtilsWrapper const&) = delete;
    DebugUtilsWrapper& operator=(DebugUtilsWrapper const&) = delete;
    DebugUtilsWrapper(DebugUtilsWrapper&&) = delete;
    DebugUtilsWrapper& operator=(DebugUtilsWrapper&&) = delete;

    bool find(std::string const& search_text) { return logger.find(search_text); }
    uint32_t count(std::string const& search_text) { return logger.count(search_text); }
    VkDebugUtilsMessengerCreateInfoEXT* get() noexcept { return logger.get(); }

    DebugUtilsLogger logger;
    VkInstance inst = VK_NULL_HANDLE;
    VkAllocationCallbacks* callbacks = nullptr;
    PFN_vkCreateDebugUtilsMessengerEXT local_vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT local_vkDestroyDebugUtilsMessengerEXT = nullptr;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
};

VkResult CreateDebugUtilsMessenger(DebugUtilsWrapper& debug_utils);

// Helper that adds the debug utils extension name and sets the pNext chain up
// NOTE: Ignores existing pNext chains
void FillDebugUtilsCreateDetails(InstanceCreateInfo& create_info, DebugUtilsLogger& logger);
void FillDebugUtilsCreateDetails(InstanceCreateInfo& create_info, DebugUtilsWrapper& wrapper);

struct LoaderSettingsLayerConfiguration {
    BUILDER_VALUE(std::string, name)
    BUILDER_VALUE(std::filesystem::path, path)
    BUILDER_VALUE(std::string, control)
    BUILDER_VALUE(bool, treat_as_implicit_manifest)
};
// Needed for next_permutation
inline bool operator==(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) {
    return a.name == b.name && a.path == b.path && a.control == b.control &&
           a.treat_as_implicit_manifest == b.treat_as_implicit_manifest;
}
inline bool operator!=(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) { return !(a == b); }
inline bool operator<(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) {
    return a.name < b.name;
}
inline bool operator>(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) { return (b < a); }
inline bool operator<=(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) { return !(b < a); }
inline bool operator>=(LoaderSettingsLayerConfiguration const& a, LoaderSettingsLayerConfiguration const& b) { return !(a < b); }

struct LoaderSettingsDriverConfiguration {
    BUILDER_VALUE(std::filesystem::path, path)
};

using VulkanUUID = std::array<uint8_t, VK_UUID_SIZE>;

struct LoaderSettingsDeviceConfiguration {
    BUILDER_VALUE(VulkanUUID, deviceUUID)
    BUILDER_VALUE(VulkanUUID, driverUUID)
    BUILDER_VALUE(uint32_t, driverVersion)
    BUILDER_VALUE(std::string, deviceName)
    BUILDER_VALUE(std::string, driverName)
};

// Log files and their associated filter
struct LoaderLogConfiguration {
    BUILDER_VECTOR(std::string, destinations, destination)
    BUILDER_VECTOR(std::string, filters, filter)
};
struct AppSpecificSettings {
    BUILDER_VECTOR(std::string, app_keys, app_key)
    BUILDER_VECTOR(LoaderSettingsLayerConfiguration, layer_configurations, layer_configuration)
    BUILDER_VECTOR(LoaderSettingsDriverConfiguration, driver_configurations, driver_configuration)
    BUILDER_VECTOR(LoaderSettingsDeviceConfiguration, device_configurations, device_configuration)
    BUILDER_VALUE(bool, additional_drivers_use_exclusively)
    BUILDER_VECTOR(std::string, stderr_log, stderr_log_filter)
    BUILDER_VECTOR(LoaderLogConfiguration, log_configurations, log_configuration)
};

struct LoaderSettings {
    BUILDER_VALUE(ManifestVersion, file_format_version)
    BUILDER_VECTOR(AppSpecificSettings, app_specific_settings, app_specific_setting);
};

struct PlatformShimWrapper {
    PlatformShimWrapper(fs::FileSystemManager& file_system_manager, const char* log_filter) noexcept;
    PlatformShimWrapper(PlatformShimWrapper const&) = delete;
    PlatformShimWrapper& operator=(PlatformShimWrapper const&) = delete;

    // Convenience
    PlatformShim* operator->() { return platform_shim; }

    LibraryWrapper shim_library;
    PlatformShim* platform_shim = nullptr;
    EnvVarWrapper loader_logging;
};

template <typename BinaryObject, typename GetTestBinaryFunc, typename GetNewTestBinaryFunc>
struct TestBinaryHandle {
    TestBinaryHandle() noexcept {}
    TestBinaryHandle(std::filesystem::path const& binary_path) noexcept;

    BinaryObject& get_test_binary() noexcept {
        assert(proc_addr_get_test_binary != NULL && "symbol must be loaded before use");
        return *proc_addr_get_test_binary();
    }
    BinaryObject& reset() noexcept {
        assert(proc_addr_reset_binary != NULL && "symbol must be loaded before use");
        return *proc_addr_reset_binary();
    }
    std::filesystem::path get_full_path() noexcept { return library.get_path(); }
    std::filesystem::path get_manifest_path() noexcept { return manifest_path; }
    std::filesystem::path get_shimmed_manifest_path() noexcept { return shimmed_manifest_path; }

    // Must use statically
    LibraryWrapper library;
    GetTestBinaryFunc proc_addr_get_test_binary = nullptr;
    GetNewTestBinaryFunc proc_addr_reset_binary = nullptr;
    std::filesystem::path
        manifest_path;  // path to the manifest file is on the actual filesystem (aka <build_folder>/tests/framework/<...>)
    std::filesystem::path
        shimmed_manifest_path;  // path to where the loader will find the manifest file (eg /usr/local/share/vulkan/<...>)
};

using TestICDHandle = TestBinaryHandle<TestICD, GetTestICDFunc, GetNewTestICDFunc>;
using TestLayerHandle = TestBinaryHandle<TestLayer, GetTestLayerFunc, GetNewTestLayerFunc>;

struct ManifestOptions {
    BUILDER_VALUE(std::filesystem::path, json_name);
    BUILDER_VALUE_WITH_DEFAULT(ManifestDiscoveryType, discovery_type, ManifestDiscoveryType::generic);
    BUILDER_VALUE(bool, is_fake);
    // If discovery type is env-var, is_dir controls whether to use the path to the file or folder the manifest is in
    BUILDER_VALUE(bool, is_dir);
    BUILDER_VALUE_WITH_DEFAULT(LibraryPathType, library_path_type, LibraryPathType::absolute);
};

struct FrameworkSettings {
    BUILDER_VALUE_WITH_DEFAULT(const char*, log_filter, "all");
    BUILDER_VALUE_WITH_DEFAULT(bool, run_as_if_with_elevated_privleges, false);

#if TESTING_COMMON_UNIX_PLATFORMS
    BUILDER_VALUE_WITH_DEFAULT(std::string, home_env_var, "/home/fake_home");
#if !defined(__APPLE__)
    BUILDER_VALUE(std::string, xdg_config_home_env_var);
    BUILDER_VALUE(std::string, xdg_config_dirs_env_var);
    BUILDER_VALUE(std::string, xdg_data_home_env_var);
    BUILDER_VALUE(std::string, xdg_data_dirs_env_var);
#endif
#endif
};

struct FrameworkEnvironment {
    FrameworkEnvironment() noexcept;  // default is to enable VK_LOADER_DEBUG=all and enable the default search paths
    FrameworkEnvironment(const FrameworkSettings& settings) noexcept;
    ~FrameworkEnvironment();
    // Delete copy constructors - this class should never move after being created
    FrameworkEnvironment(const FrameworkEnvironment&) = delete;
    FrameworkEnvironment& operator=(const FrameworkEnvironment&) = delete;

    TestICD& add_icd(std::filesystem::path const& path, ManifestOptions args = ManifestOptions{},
                     ManifestICD manifest = ManifestICD{}) noexcept;

    void add_implicit_layer(ManifestOptions args, ManifestLayer layer_manifest) noexcept;
    void add_explicit_layer(ManifestOptions args, ManifestLayer layer_manifest) noexcept;

    // Resets the current settings with the values contained in loader_settings.
    // Write_to_secure_location determines whether to write to the secure or unsecure settings folder.
    void write_settings_file(std::string const& file_contents, bool write_to_secure_location);

    // Apply any changes made to FrameworkEnvironment's loader_settings member.
    // By default writes to the secure settings location
    void update_loader_settings(const LoaderSettings& loader_settings, bool write_to_secure_location = true) noexcept;

    void remove_loader_settings();

    // Creates a file called `file_name` for the given `category` in the given `location` with `source_string` as the contents
    void write_file_from_string(std::string const& source_string, ManifestCategory category, ManifestLocation location,
                                std::string const& file_name);
    // Creates a file called `file_name` for the given `category` in the given `location` with contents copied from `source_file`
    void write_file_from_source(const char* source_file, ManifestCategory category, ManifestLocation location,
                                std::string const& file_name);

    TestICD& get_test_icd(size_t index = 0) noexcept;
    TestICD& reset_icd(size_t index = 0) noexcept;
    std::filesystem::path get_test_icd_path(size_t index = 0) noexcept;
    std::filesystem::path get_icd_manifest_path(size_t index = 0) noexcept;
    std::filesystem::path get_shimmed_icd_manifest_path(size_t index = 0) noexcept;

    TestLayer& get_test_layer(size_t index = 0) noexcept;
    TestLayer& reset_layer(size_t index = 0) noexcept;
    std::filesystem::path get_test_layer_path(size_t index = 0) noexcept;
    std::filesystem::path get_layer_manifest_path(size_t index = 0) noexcept;
    std::filesystem::path get_shimmed_layer_manifest_path(size_t index = 0) noexcept;

    fs::Folder& get_folder(ManifestLocation location) noexcept;
    fs::Folder const& get_folder(ManifestLocation location) const noexcept;
#if defined(__APPLE__)
    // Set the path of the app bundle to the appropriate test framework bundle
    void setup_macos_bundle() noexcept;
#endif

    void add_symlink(ManifestLocation location, std::filesystem::path const& target, std::filesystem::path const& link_name);

    FrameworkSettings settings;

    fs::FileSystemManager file_system_manager;

    // Query the global extensions
    // Optional: use layer_name to query the extensions of a specific layer
    std::vector<VkExtensionProperties> GetInstanceExtensions(uint32_t count, const char* layer_name = nullptr);
    // Query the available layers
    std::vector<VkLayerProperties> GetLayerProperties(uint32_t count);

    PlatformShimWrapper platform_shim;

    std::vector<TestICDHandle> icds;
    std::vector<TestLayerHandle> layers;

    DebugUtilsLogger debug_log;
    VulkanFunctions vulkan_functions;

    EnvVarWrapper env_var_vk_icd_filenames{"VK_DRIVER_FILES"};
    EnvVarWrapper add_env_var_vk_icd_filenames{"VK_ADD_DRIVER_FILES"};
    EnvVarWrapper env_var_vk_layer_paths{"VK_LAYER_PATH"};
    EnvVarWrapper add_env_var_vk_layer_paths{"VK_ADD_LAYER_PATH"};
    EnvVarWrapper env_var_vk_implicit_layer_paths{"VK_IMPLICIT_LAYER_PATH"};
    EnvVarWrapper add_env_var_vk_implicit_layer_paths{"VK_ADD_IMPLICIT_LAYER_PATH"};
    EnvVarWrapper env_var_vk_loader_device_id_filter{"VK_LOADER_DEVICE_ID_FILTER"};
    EnvVarWrapper env_var_vk_loader_vendor_id_filter{"VK_LOADER_VENDOR_ID_FILTER"};
    EnvVarWrapper env_var_vk_loader_driver_id_filter{"VK_LOADER_DRIVER_ID_FILTER"};

#if TESTING_COMMON_UNIX_PLATFORMS
    EnvVarWrapper env_var_home{"HOME", "/home/fake_home"};
#if !defined(__APPLE__)
    EnvVarWrapper env_var_xdg_config_home{"XDG_CONFIG_HOME"};
    EnvVarWrapper env_var_xdg_config_dirs{"XDG_CONFIG_DIRS"};
    EnvVarWrapper env_var_xdg_data_home{"XDG_DATA_HOME"};
    EnvVarWrapper env_var_xdg_data_dirs{"XDG_DATA_DIRS"};
#endif
    std::string secure_manifest_base_location;
    std::string unsecure_manifest_base_location;
#endif

    LoaderSettings loader_settings;  // the current settings written to disk
   private:
    uint32_t created_layer_count = 0;
    void add_layer_impl(ManifestOptions args, ManifestLayer manifest, ManifestCategory category);

    static ManifestLocation map_discovery_type_to_location(ManifestDiscoveryType type, ManifestCategory category);
};

// Create a surface using a platform specific API
// api_selection: optionally provide a VK_USE_PLATFORM_XXX string to select which API to create a surface with.
//    defaults to Metal on macOS and XCB on linux if not provided
// Returns an VkResult with the result of surface creation
// returns VK_ERROR_EXTENSION_NOT_PRESENT if it fails to load the surface creation function
VkResult create_surface(InstWrapper& inst, VkSurfaceKHR& out_surface, const char* api_selection = nullptr);
// Alternate parameter list for allocation callback tests
VkResult create_surface(VulkanFunctions* functions, VkInstance inst, VkSurfaceKHR& surface, const char* api_selection = nullptr);

VkResult create_debug_callback(InstWrapper& inst, const VkDebugReportCallbackCreateInfoEXT& create_info,
                               VkDebugReportCallbackEXT& callback);
