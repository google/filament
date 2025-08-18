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

// Use the NDK's header on Android
#include "gtest/gtest.h"

#include "test_util.h"

#include "shim/shim.h"

#include "icd/test_icd.h"

#include "layer/test_layer.h"

#include FRAMEWORK_CONFIG_HEADER

// Useful defines
#if COMMON_UNIX_PLATFORMS
#define HOME_DIR "/home/fake_home"
#define USER_LOCAL_SHARE_DIR HOME_DIR "/.local/share"
#define ETC_DIR "/etc"
#endif

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

// VulkanFunctions - loads vulkan functions for tests to use

struct VulkanFunctions {
#if !defined(APPLE_STATIC_LOADER)
    LibraryWrapper loader;
#endif
    // Pre-Instance
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = nullptr;
    PFN_vkCreateInstance vkCreateInstance = nullptr;

    // Instance
    PFN_vkDestroyInstance vkDestroyInstance = nullptr;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
    PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups = nullptr;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties = nullptr;
    PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties = nullptr;
    PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = nullptr;
    PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties = nullptr;
    PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties = nullptr;
    PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties = nullptr;
    PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties = nullptr;

    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
    PFN_vkCreateDevice vkCreateDevice = nullptr;

    // WSI
    PFN_vkCreateHeadlessSurfaceEXT vkCreateHeadlessSurfaceEXT = nullptr;
    PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR = nullptr;
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR = nullptr;
    PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR = nullptr;
    PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR = nullptr;
    PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR = nullptr;
    PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR = nullptr;
    PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR = nullptr;
    PFN_vkGetPhysicalDeviceDisplayProperties2KHR vkGetPhysicalDeviceDisplayProperties2KHR = nullptr;
    PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR vkGetPhysicalDeviceDisplayPlaneProperties2KHR = nullptr;
    PFN_vkGetDisplayModeProperties2KHR vkGetDisplayModeProperties2KHR = nullptr;
    PFN_vkGetDisplayPlaneCapabilities2KHR vkGetDisplayPlaneCapabilities2KHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormats2KHR vkGetPhysicalDeviceSurfaceFormats2KHR = nullptr;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR = nullptr;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    PFN_vkCreateDirectFBSurfaceEXT vkCreateDirectFBSurfaceEXT = nullptr;
    PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT vkGetPhysicalDeviceDirectFBPresentationSupportEXT = nullptr;
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_FUCHSIA)
    PFN_vkCreateImagePipeSurfaceFUCHSIA vkCreateImagePipeSurfaceFUCHSIA = nullptr;
#endif  // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_GGP)
    PFN_vkCreateStreamDescriptorSurfaceGGP vkCreateStreamDescriptorSurfaceGGP = nullptr;
#endif  // VK_USE_PLATFORM_GGP
#if defined(VK_USE_PLATFORM_IOS_MVK)
    PFN_vkCreateIOSSurfaceMVK vkCreateIOSSurfaceMVK = nullptr;
#endif  // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK = nullptr;
#endif  // VK_USE_PLATFORM_MACOS_MVK
#if defined(VK_USE_PLATFORM_METAL_EXT)
    PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT = nullptr;
#endif  // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    PFN_vkCreateScreenSurfaceQNX vkCreateScreenSurfaceQNX = nullptr;
    PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX vkGetPhysicalDeviceScreenPresentationSupportQNX = nullptr;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR = nullptr;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR = nullptr;
#endif  // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR = nullptr;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = nullptr;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;

    // instance extensions functions (can only be loaded with a valid instance)
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;    // Null unless the extension is enabled
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;  // Null unless the extension is enabled
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;    // Null unless the extension is enabled
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;  // Null unless the extension is enabled

    // device functions
    PFN_vkDestroyDevice vkDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;

    VulkanFunctions();

    void load_instance_functions(VkInstance instance);

    FromVoidStarFunc load(VkInstance inst, const char* func_name) const {
        return FromVoidStarFunc(vkGetInstanceProcAddr(inst, func_name));
    }

    FromVoidStarFunc load(VkDevice device, const char* func_name) const {
        return FromVoidStarFunc(vkGetDeviceProcAddr(device, func_name));
    }
};

struct DeviceFunctions {
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
    PFN_vkDestroyDevice vkDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
    PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
    PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;

    DeviceFunctions() = default;
    DeviceFunctions(const VulkanFunctions& vulkan_functions, VkDevice device);

    FromVoidStarFunc load(VkDevice device, const char* func_name) const {
        return FromVoidStarFunc(vkGetDeviceProcAddr(device, func_name));
    }
};

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

namespace fs {

int create_folder(std::filesystem::path const& path);
int delete_folder(std::filesystem::path const& folder);

class FolderManager {
   public:
    explicit FolderManager(std::filesystem::path root_path, std::string name) noexcept;
    ~FolderManager() noexcept;
    FolderManager(FolderManager const&) = delete;
    FolderManager& operator=(FolderManager const&) = delete;
    FolderManager(FolderManager&& other) noexcept;
    FolderManager& operator=(FolderManager&& other) noexcept;

    // Add a manifest to the folder
    std::filesystem::path write_manifest(std::filesystem::path const& name, std::string const& contents);

    // close file handle, delete file, remove `name` from managed file list.
    void remove(std::filesystem::path const& name);

    // Remove all contents in the path
    void clear() const noexcept;

    // copy file into this folder with name `new_name`. Returns the full path of the file that was copied
    std::filesystem::path copy_file(std::filesystem::path const& file, std::filesystem::path const& new_name);

    // location of the managed folder
    std::filesystem::path location() const { return folder; }

    std::vector<std::filesystem::path> get_files() const;

    // Create a symlink in this folder to target with the filename set to link_name
    std::filesystem::path add_symlink(std::filesystem::path const& target, std::filesystem::path const& link_name);

   private:
    bool actually_created = false;
    std::filesystem::path folder;
    std::vector<std::filesystem::path> added_files;

    void insert_file_to_tracking(std::filesystem::path const& name);
    void check_if_first_use();
};
}  // namespace fs

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

struct LoaderSettingsDeviceConfiguration {
    BUILDER_VALUE(VulkanUUID, deviceUUID)
    BUILDER_VALUE(std::string, deviceName)
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

struct FrameworkEnvironment;  // forward declaration

struct PlatformShimWrapper {
    PlatformShimWrapper(GetFoldersFunc get_folders_by_name_function, const char* log_filter) noexcept;
    ~PlatformShimWrapper() noexcept;
    PlatformShimWrapper(PlatformShimWrapper const&) = delete;
    PlatformShimWrapper& operator=(PlatformShimWrapper const&) = delete;

    // Convenience
    PlatformShim* operator->() { return platform_shim; }

    LibraryWrapper shim_library;
    PlatformShim* platform_shim = nullptr;
    EnvVarWrapper loader_logging;
};

struct TestICDHandle {
    TestICDHandle() noexcept;
    TestICDHandle(std::filesystem::path const& icd_path) noexcept;
    TestICD& reset_icd() noexcept;
    TestICD& get_test_icd() noexcept;
    std::filesystem::path get_icd_full_path() noexcept;
    std::filesystem::path get_icd_manifest_path() noexcept;
    std::filesystem::path get_shimmed_manifest_path() noexcept;

    // Must use statically
    LibraryWrapper icd_library;
    GetTestICDFunc proc_addr_get_test_icd = nullptr;
    GetNewTestICDFunc proc_addr_reset_icd = nullptr;
    std::filesystem::path
        manifest_path;  // path to the manifest file is on the actual filesystem (aka <build_folder>/tests/framework/<...>)
    std::filesystem::path
        shimmed_manifest_path;  // path to where the loader will find the manifest file (eg /usr/local/share/vulkan/<...>)
};
struct TestLayerHandle {
    TestLayerHandle() noexcept;
    TestLayerHandle(std::filesystem::path const& layer_path) noexcept;
    TestLayer& reset_layer() noexcept;
    TestLayer& get_test_layer() noexcept;
    std::filesystem::path get_layer_full_path() noexcept;
    std::filesystem::path get_layer_manifest_path() noexcept;
    std::filesystem::path get_shimmed_manifest_path() noexcept;

    // Must use statically
    LibraryWrapper layer_library;
    GetTestLayerFunc proc_addr_get_test_layer = nullptr;
    GetNewTestLayerFunc proc_addr_reset_layer = nullptr;
    std::filesystem::path
        manifest_path;  // path to the manifest file is on the actual filesystem (aka <build_folder>/tests/framework/<...>)
    std::filesystem::path
        shimmed_manifest_path;  // path to where the loader will find the manifest file (eg /usr/local/share/vulkan/<...>)
};

// Controls whether to create a manifest and where to put it
enum class ManifestDiscoveryType {
    generic,              // put the manifest in the regular locations
    unsecured_generic,    // put the manifest in a user folder rather than system
    none,                 // Do not write the manifest anywhere (for Direct Driver Loading)
    null_dir,             // put the manifest in the 'null_dir' which the loader does not search in (D3DKMT for instance)
    env_var,              // use the corresponding env-var for it
    add_env_var,          // use the corresponding add-env-var for it
    override_folder,      // add to a special folder for the override layer to use
    windows_app_package,  // let the app package search find it
    macos_bundle,         // place it in a location only accessible to macos bundles
};

enum class LibraryPathType {
    absolute,              // default for testing - the exact path of the binary
    relative,              // Relative to the manifest file
    default_search_paths,  // Dont add any path information to the library_path - force the use of the default search paths
};

struct TestICDDetails {
    TestICDDetails(ManifestICD icd_manifest) noexcept : icd_manifest(icd_manifest) {}
    TestICDDetails(std::filesystem::path icd_binary_path, uint32_t api_version = VK_API_VERSION_1_0) noexcept {
        icd_manifest.set_lib_path(icd_binary_path).set_api_version(api_version);
    }
    BUILDER_VALUE(ManifestICD, icd_manifest);
    BUILDER_VALUE_WITH_DEFAULT(std::filesystem::path, json_name, "test_icd");
    // Uses the json_name without modification - default is to append _1 in the json file to distinguish drivers
    BUILDER_VALUE(bool, disable_icd_inc);
    BUILDER_VALUE_WITH_DEFAULT(ManifestDiscoveryType, discovery_type, ManifestDiscoveryType::generic);
    BUILDER_VALUE(bool, is_fake);
    // If discovery type is env-var, is_dir controls whether to use the path to the file or folder the manifest is in
    BUILDER_VALUE(bool, is_dir);
    BUILDER_VALUE_WITH_DEFAULT(LibraryPathType, library_path_type, LibraryPathType::absolute);
};

struct TestLayerDetails {
    TestLayerDetails(ManifestLayer layer_manifest, const std::string& json_name) noexcept
        : layer_manifest(layer_manifest), json_name(json_name) {}
    BUILDER_VALUE(ManifestLayer, layer_manifest);
    BUILDER_VALUE_WITH_DEFAULT(std::string, json_name, "test_layer");
    BUILDER_VALUE_WITH_DEFAULT(ManifestDiscoveryType, discovery_type, ManifestDiscoveryType::generic);
    BUILDER_VALUE(bool, is_fake);
    // If discovery type is env-var, is_dir controls whether to use the path to the file or folder the manifest is in
    BUILDER_VALUE_WITH_DEFAULT(bool, is_dir, true);
    BUILDER_VALUE_WITH_DEFAULT(LibraryPathType, library_path_type, LibraryPathType::absolute);
};

// Locations manifests can go in the test framework
// If this enum is added to - the contructor of FrameworkEnvironment also needs to be updated with the new enum value
enum class ManifestLocation {
    null = 0,
    driver = 1,
    driver_env_var = 2,
    explicit_layer = 3,
    explicit_layer_env_var = 4,
    explicit_layer_add_env_var = 5,
    implicit_layer = 6,
    implicit_layer_env_var = 7,
    implicit_layer_add_env_var = 8,
    override_layer = 9,
    windows_app_package = 10,
    macos_bundle = 11,
    unsecured_location = 12,
    settings_location = 13,
};

struct FrameworkSettings {
    BUILDER_VALUE_WITH_DEFAULT(const char*, log_filter, "all");
    BUILDER_VALUE_WITH_DEFAULT(bool, enable_default_search_paths, true);
    BUILDER_VALUE(LoaderSettings, loader_settings);
    BUILDER_VALUE(bool, secure_loader_settings);
};

struct FrameworkEnvironment {
    FrameworkEnvironment() noexcept;  // default is to enable VK_LOADER_DEBUG=all and enable the default search paths
    FrameworkEnvironment(const FrameworkSettings& settings) noexcept;
    ~FrameworkEnvironment();
    // Delete copy constructors - this class should never move after being created
    FrameworkEnvironment(const FrameworkEnvironment&) = delete;
    FrameworkEnvironment& operator=(const FrameworkEnvironment&) = delete;

    TestICD& add_icd(TestICDDetails icd_details) noexcept;
    void add_implicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept;
    void add_implicit_layer(TestLayerDetails layer_details) noexcept;
    void add_explicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept;
    void add_explicit_layer(TestLayerDetails layer_details) noexcept;
    void add_fake_implicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept;
    void add_fake_explicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept;

    // resets the current settings with the values contained in loader_settings
    void write_settings_file(std::string const& file_contents);
    // apply any changes made to FrameworkEnvironment's loader_settings member
    void update_loader_settings(const LoaderSettings& loader_settings) noexcept;
    void remove_loader_settings();

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

    fs::FolderManager& get_folder(ManifestLocation location) noexcept;
    fs::FolderManager const& get_folder(ManifestLocation location) const noexcept;
#if defined(__APPLE__)
    // Set the path of the app bundle to the appropriate test framework bundle
    void setup_macos_bundle() noexcept;
#endif

    FrameworkSettings settings;

    fs::FolderManager test_folder;

    // Query the global extensions
    // Optional: use layer_name to query the extensions of a specific layer
    std::vector<VkExtensionProperties> GetInstanceExtensions(uint32_t count, const char* layer_name = nullptr);
    // Query the available layers
    std::vector<VkLayerProperties> GetLayerProperties(uint32_t count);

    PlatformShimWrapper platform_shim;
    std::vector<fs::FolderManager> folders;

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

    LoaderSettings loader_settings;  // the current settings written to disk
   private:
    void add_layer_impl(TestLayerDetails layer_details, ManifestCategory category);
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
