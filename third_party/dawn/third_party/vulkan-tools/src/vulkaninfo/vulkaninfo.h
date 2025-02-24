/*
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
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
 *
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: David Pinedo <david@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Rene Lindsay <rene@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: Bob Ellison <bob@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <fstream>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <utility>
#include <functional>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cstring>

#ifdef __GNUC__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#else
#define strndup(p, n) strdup(p)
#endif

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#if _MSC_VER == 1900
#pragma warning(disable : 4800)
#endif
#endif  // _WIN32

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_XCB_KHR)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)
#include "metal_view.h"
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#endif

#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include "volk.h"

static std::string VkResultString(VkResult err);

// General error: Get file + line and a short message
struct FileLineException : std::runtime_error {
    FileLineException(const std::string &arg, const char *file, int line) : runtime_error(arg) {
        msg = std::string(file) + ":" + std::to_string(line) + ": " + arg;
    }
    ~FileLineException() throw() {}
    const char *what() const throw() { return msg.c_str(); }

  private:
    std::string msg;
};
#define THROW_ERR(arg) throw FileLineException(arg, __FILE__, __LINE__);

// Vulkan function error: Get name of function, file, line, and the error code returned by the function
struct VulkanException : std::runtime_error {
    VulkanException(const std::string &function, const char *file, int line, VkResult err) : runtime_error(function) {
        msg = std::string(file) + ":" + std::to_string(line) + ":" + function + " failed with " + VkResultString(err);
    }
    ~VulkanException() throw() {}
    const char *what() const throw() { return msg.c_str(); }

  private:
    std::string msg;
};
#define THROW_VK_ERR(func_name, err) throw VulkanException(func_name, __FILE__, __LINE__, err);

#ifdef _WIN32

#define strdup _strdup

// Returns nonzero if the console is used only for this process. Will return
// zero if another process (such as cmd.exe) is also attached.
static int ConsoleIsExclusive(void) {
    DWORD pids[2];
    DWORD num_pids = GetConsoleProcessList(pids, ARRAYSIZE(pids));
    return num_pids <= 1;
}
void wait_for_console_destroy() {
    if (ConsoleIsExclusive()) Sleep(INFINITE);
}

// User32 function declarations
using PFN_AdjustWindowRect = WINUSERAPI BOOL(WINAPI *)(_Inout_ LPRECT, _In_ DWORD, _In_ BOOL);
using PFN_CreateWindowExA = WINUSERAPI HWND(WINAPI *)(_In_ DWORD, _In_opt_ LPCSTR, _In_opt_ LPCSTR, _In_ DWORD, _In_ int, _In_ int,
                                                      _In_ int, _In_ int, _In_opt_ HWND, _In_opt_ HMENU, _In_opt_ HINSTANCE,
                                                      _In_opt_ LPVOID);
using PFN_DefWindowProcA = WINUSERAPI LRESULT(WINAPI *)(_In_ HWND, _In_ UINT, _In_ WPARAM, _In_ LPARAM);
using PFN_DestroyWindow = WINUSERAPI BOOL(WINAPI *)(_In_ HWND);
using PFN_LoadIconA = WINUSERAPI HICON(WINAPI *)(_In_opt_ HINSTANCE, _In_ LPCSTR);
using PFN_RegisterClassExA = WINUSERAPI ATOM(WINAPI *)(_In_ CONST WNDCLASSEXA *);

struct User32Handles {
    // User32 dll handle
    HMODULE user32DllHandle = nullptr;

    // User32 function pointers
    PFN_AdjustWindowRect pfnAdjustWindowRect = nullptr;
    PFN_CreateWindowExA pfnCreateWindowExA = nullptr;
    PFN_DefWindowProcA pfnDefWindowProcA = nullptr;
    PFN_DestroyWindow pfnDestroyWindow = nullptr;
    PFN_LoadIconA pfnLoadIconA = nullptr;
    PFN_RegisterClassExA pfnRegisterClassExA = nullptr;

    User32Handles() noexcept {}
    ~User32Handles() noexcept {
        if (user32DllHandle != nullptr) {
            FreeLibrary(user32DllHandle);
        }
    }
    // Don't allow moving of this class
    User32Handles(User32Handles const &) = delete;
    User32Handles &operator=(User32Handles const &) = delete;
    User32Handles(User32Handles &&) = delete;
    User32Handles &operator=(User32Handles &&) = delete;

    bool load() {
        user32DllHandle = LoadLibraryExA("user32.dll", nullptr, 0);
        if (user32DllHandle == nullptr) return false;
        if (!load_function(pfnAdjustWindowRect, "AdjustWindowRect")) return false;
        if (!load_function(pfnCreateWindowExA, "CreateWindowExA")) return false;
        if (!load_function(pfnDefWindowProcA, "DefWindowProcA")) return false;
        if (!load_function(pfnDestroyWindow, "DestroyWindow")) return false;
        if (!load_function(pfnLoadIconA, "LoadIconA")) return false;
        if (!load_function(pfnRegisterClassExA, "RegisterClassExA")) return false;
        return true;
    }

  private:
    template <typename T>
    bool load_function(T &function_pointer, const char *function_name) {
        function_pointer = reinterpret_cast<T>(GetProcAddress(user32DllHandle, function_name));
        if (function_pointer == nullptr) {
            fprintf(stderr, "Failed to load function: %s\n", function_name);
            return false;
        }
        return true;
    }
};

// Global user handles function used in windows callback and code
User32Handles *user32_handles;
#endif  // _WIN32

#define APP_SHORT_NAME "vulkaninfo"
#define APP_UPPER_CASE_NAME "VULKANINFO"
#define API_NAME "Vulkan"

std::vector<const char *> get_c_str_array(std::vector<std::string> const &vec) {
    std::vector<const char *> ret;
    for (auto &str : vec) ret.push_back(str.c_str());
    return ret;
}

static const char *VkDebugReportFlagsEXTString(const VkDebugReportFlagsEXT flags) {
    switch (flags) {
        case VK_DEBUG_REPORT_ERROR_BIT_EXT:
            return "ERROR";
        case VK_DEBUG_REPORT_WARNING_BIT_EXT:
            return "WARNING";
        case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
            return "PERF";
        case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
            return "INFO";
        case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
            return "DEBUG";
        default:
            return "UNKNOWN";
    }
}
static VKAPI_ATTR VkBool32 VKAPI_CALL DbgCallback(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType,
                                                  uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix,
                                                  const char *pMsg, void *pUserData) {
    std::cerr << VkDebugReportFlagsEXTString(msgFlags) << ": [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << "\n";

    // True is reserved for layer developers, and MAY mean calls are not distributed down the layer chain after validation
    // error. False SHOULD always be returned by apps:
    return VK_FALSE;
}

// Helper for robustly executing the two-call pattern
template <typename T, typename F, typename... Ts>
auto GetVectorInit(const char *func_name, F &&f, T init, Ts &&...ts) -> std::vector<T> {
    uint32_t count = 32;  // Preallocate enough so that most calls only happen once
    std::vector<T> results;
    VkResult err;
    uint32_t iteration_count = 0;
    uint32_t max_iterations = 5;
    do {
        count *= 2;
        results.resize(count, init);
        err = f(ts..., &count, results.data());
        results.resize(count);
        iteration_count++;
    } while (err == VK_INCOMPLETE && iteration_count < max_iterations);
    if (err) THROW_VK_ERR(func_name, err);
    return results;
}

template <typename T, typename F, typename... Ts>
auto GetVector(const char *func_name, F &&f, Ts &&...ts) -> std::vector<T> {
    return GetVectorInit(func_name, f, T(), ts...);
}

// Forward declarations for pNext chains
struct phys_device_props2_chain;
struct phys_device_mem_props2_chain;
struct phys_device_features2_chain;
struct surface_capabilities2_chain;
struct format_properties2_chain;
struct queue_properties2_chain;
struct video_profile_info_chain;
struct video_capabilities_chain;
struct video_format_properties_chain;
struct AppInstance;
struct AppGpu;
struct AppVideoProfile;

void setup_phys_device_props2_chain(VkPhysicalDeviceProperties2 &start, std::unique_ptr<phys_device_props2_chain> &chain,
                                    AppInstance &inst, AppGpu &gpu, bool show_promoted_structs);
void setup_phys_device_mem_props2_chain(VkPhysicalDeviceMemoryProperties2 &start,
                                        std::unique_ptr<phys_device_mem_props2_chain> &chain, AppGpu &gpu);
void setup_phys_device_features2_chain(VkPhysicalDeviceFeatures2 &start, std::unique_ptr<phys_device_features2_chain> &chain,
                                       AppGpu &gpu, bool show_promoted_structs);
void setup_surface_capabilities2_chain(VkSurfaceCapabilities2KHR &start, std::unique_ptr<surface_capabilities2_chain> &chain,
                                       AppInstance &inst, AppGpu &gpu);
void setup_format_properties2_chain(VkFormatProperties2 &start, std::unique_ptr<format_properties2_chain> &chain, AppGpu &gpu);
void setup_queue_properties2_chain(VkQueueFamilyProperties2 &start, std::unique_ptr<queue_properties2_chain> &chain, AppGpu &gpu);

bool prepare_phys_device_props2_twocall_chain_vectors(std::unique_ptr<phys_device_props2_chain> &chain);

bool is_video_format_same(const VkVideoFormatPropertiesKHR &format_a, const VkVideoFormatPropertiesKHR &format_b);
std::vector<std::unique_ptr<AppVideoProfile>> enumerate_supported_video_profiles(AppGpu &gpu);

/* An ptional contains either a value or nothing. The optional asserts if a value is trying to be gotten but none exist.
 * The interface is taken from C++17's <optional> with many aspects removed.
 * This class assumes the template type is 'trivial'
 */
namespace util {
template <typename T>
struct vulkaninfo_optional {
    using value_type = T;

    bool _contains_value = false;
    value_type _value;

    vulkaninfo_optional() noexcept : _contains_value(false), _value({}) {}
    vulkaninfo_optional(T value) noexcept : _contains_value(true), _value(value) {}

    explicit operator bool() const noexcept { return _contains_value; }
    bool has_value() const noexcept { return _contains_value; }

    value_type value() const noexcept {
        assert(_contains_value);
        return _value;
    }
    // clang-format off
    const value_type* operator->() const { assert(_contains_value); return _value;}
    value_type* operator->() { assert(_contains_value); return &_value;}
    const value_type& operator*() const& { assert(_contains_value); return _value;}
    value_type& operator*() & { assert(_contains_value); return _value;}
    const value_type&& operator*() const&& { assert(_contains_value); return _value;}
    value_type&& operator*() && { assert(_contains_value); return _value;}
    // clang-format on
};  // namespace util
}  // namespace util
struct LayerExtensionList {
    VkLayerProperties layer_properties;
    std::vector<VkExtensionProperties> extension_properties;
};

struct AppInstance;

struct SurfaceExtension {
    std::string name;
    void (*create_window)(AppInstance &) = nullptr;
    VkSurfaceKHR (*create_surface)(AppInstance &) = nullptr;
    void (*destroy_window)(AppInstance &) = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    bool operator==(const SurfaceExtension &other) { return name == other.name && surface == other.surface; }
};

struct AppVideoProfile {
    bool supported;

    std::string name;

    VkVideoProfileInfoKHR profile_info;
    std::unique_ptr<video_profile_info_chain> profile_info_chain;

    VkVideoCapabilitiesKHR capabilities;
    std::unique_ptr<video_capabilities_chain> capabilities_chain;

    struct Format {
        VkVideoFormatPropertiesKHR properties;
        std::unique_ptr<video_format_properties_chain> properties_chain;
    };
    std::vector<Format> formats;
    std::unordered_map<std::string, std::vector<VkVideoFormatPropertiesKHR>> formats_by_category;

    using CreateProfileInfoChainCb = std::function<std::unique_ptr<video_profile_info_chain>(const void **)>;
    using CreateCapabilitiesChainCb = std::function<std::unique_ptr<video_capabilities_chain>(void **)>;
    struct CreateFormatPropertiesChainCb {
        std::string format_name;
        VkImageUsageFlags image_usage_flags;
        std::function<bool(const VkVideoCapabilitiesKHR &capabilities)> check_required_caps;
        std::function<std::unique_ptr<video_format_properties_chain>(void **)> callback;
    };
    using CreateFormatPropertiesChainCbList = std::vector<CreateFormatPropertiesChainCb>;
    using InitProfileCb = std::function<void(AppVideoProfile &)>;

    AppVideoProfile(AppGpu &gpu, VkPhysicalDevice phys_device, const std::string &in_name,
                    const VkVideoProfileInfoKHR &in_profile_info, CreateProfileInfoChainCb create_profile_info_chain,
                    CreateCapabilitiesChainCb create_capabilities_chain,
                    const CreateFormatPropertiesChainCbList &create_format_properties_chain_list, InitProfileCb init_profile)
        : supported(true), name(in_name), profile_info(in_profile_info) {
        profile_info_chain = create_profile_info_chain(&profile_info.pNext);
        if (profile_info_chain == nullptr) {
            supported = false;
            return;
        }

        capabilities.sType = VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR;
        capabilities.pNext = nullptr;
        capabilities_chain = create_capabilities_chain(&capabilities.pNext);
        if (capabilities_chain == nullptr) {
            supported = false;
            return;
        }

        init_profile(*this);

        VkResult result = vkGetPhysicalDeviceVideoCapabilitiesKHR(phys_device, &profile_info, &capabilities);
        if (result != VK_SUCCESS) {
            supported = false;
            return;
        }

        VkVideoProfileListInfoKHR profile_list = {VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR, nullptr, 1, &profile_info};

        std::vector<VkVideoFormatPropertiesKHR> video_format_props{};
        std::vector<std::unique_ptr<video_format_properties_chain>> video_format_props_chains{};
        for (const auto &create_format_properties_chain_info : create_format_properties_chain_list) {
            if (!create_format_properties_chain_info.check_required_caps(capabilities)) {
                continue;
            }

            VkPhysicalDeviceVideoFormatInfoKHR video_format_info = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR,
                                                                    &profile_list,
                                                                    create_format_properties_chain_info.image_usage_flags};

            uint32_t video_format_property_count = 0;
            result =
                vkGetPhysicalDeviceVideoFormatPropertiesKHR(phys_device, &video_format_info, &video_format_property_count, nullptr);
            if (result != VK_SUCCESS) {
                continue;
            }

            video_format_props.resize(video_format_property_count);
            video_format_props_chains.resize(video_format_property_count);

            for (uint32_t i = 0; i < video_format_property_count; ++i) {
                video_format_props[i].sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR;
                video_format_props_chains[i] = create_format_properties_chain_info.callback(&video_format_props[i].pNext);
            }

            result = vkGetPhysicalDeviceVideoFormatPropertiesKHR(phys_device, &video_format_info, &video_format_property_count,
                                                                 video_format_props.data());
            if (result == VK_SUCCESS) {
                for (uint32_t i = 0; i < video_format_property_count; ++i) {
                    const VkVideoFormatPropertiesKHR *existing_format = nullptr;
                    for (const auto &format : formats) {
                        if (is_video_format_same(format.properties, video_format_props[i])) {
                            existing_format = &format.properties;
                            break;
                        }
                    }
                    if (existing_format == nullptr) {
                        formats.push_back(Format{video_format_props[i], std::move(video_format_props_chains[i])});
                        formats_by_category[create_format_properties_chain_info.format_name].push_back(video_format_props[i]);
                    } else {
                        formats_by_category[create_format_properties_chain_info.format_name].push_back(*existing_format);
                    }
                }
            }

            video_format_props.clear();
            video_format_props_chains.clear();
        }
    }
};

class APIVersion {
  public:
    APIVersion() : api_version_(VK_API_VERSION_1_0) {}
    APIVersion(uint32_t api_version) : api_version_(api_version) {}
    void SetPatch(uint32_t patch) { api_version_ = api_version_ - Patch() + VK_API_VERSION_PATCH(patch); }
    uint32_t Major() const { return VK_API_VERSION_MAJOR(api_version_); }
    uint32_t Minor() const { return VK_API_VERSION_MINOR(api_version_); }
    uint32_t Patch() const { return VK_API_VERSION_PATCH(api_version_); }
    bool operator<(APIVersion api_version) const { return api_version_ < api_version.api_version_; }
    bool operator<=(APIVersion api_version) const { return api_version_ <= api_version.api_version_; }
    bool operator>(APIVersion api_version) const { return api_version_ > api_version.api_version_; }
    bool operator>=(APIVersion api_version) const { return api_version_ >= api_version.api_version_; }
    bool operator==(APIVersion api_version) const { return api_version_ == api_version.api_version_; }
    bool operator!=(APIVersion api_version) const { return api_version_ != api_version.api_version_; }
    std::string str() { return std::to_string(Major()) + "." + std::to_string(Minor()) + "." + std::to_string(Patch()); }
    operator std::string() { return str(); }

  private:
    uint32_t api_version_;
};

std::ostream &operator<<(std::ostream &out, const APIVersion &v) {
    return out << v.Major() << "." << v.Minor() << "." << v.Patch();
}

struct AppInstance {
    VkInstance instance;
    APIVersion api_version;

    VkDebugReportCallbackEXT debug_callback = VK_NULL_HANDLE;

    std::vector<LayerExtensionList> global_layers;

    std::vector<VkExtensionProperties> global_extensions;  // Instance Extensions

    std::vector<std::string> inst_extensions;

    std::vector<SurfaceExtension> surface_extensions;

    int width = 256, height = 256;

    VkSurfaceCapabilitiesKHR surface_capabilities;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    HINSTANCE h_instance;  // Windows Instance
    HWND h_wnd;            // window handle
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    xcb_connection_t *xcb_connection;
    xcb_screen_t *xcb_screen;
    xcb_window_t xcb_window;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    Display *xlib_display;
    Window xlib_window;
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    void *macos_window;
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
    void *metal_window;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    wl_display *wayland_display;
    wl_surface *wayland_surface;
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    IDirectFB *dfb;
    IDirectFBSurface *directfb_surface;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR  // TODO
    ANativeWindow *window;
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    struct _screen_context *context;
    struct _screen_window *window;
#endif
    AppInstance() {
        VkResult dllErr = volkInitialize();

        if (dllErr != VK_SUCCESS) {
            THROW_ERR("Failed to initialize: " API_NAME " loader is not installed, not found, or failed to load.");
        }

        uint32_t instance_version = VK_API_VERSION_1_0;
        if (vkEnumerateInstanceVersion) {
            const VkResult err = vkEnumerateInstanceVersion(&instance_version);
            if (err) THROW_VK_ERR("vkEnumerateInstanceVersion", err);
        }

        api_version = APIVersion(instance_version);
        // fallback to baked header version if loader returns 0 for the patch version
        if (api_version.Patch() == 0) api_version.SetPatch(VK_HEADER_VERSION);

        AppGetInstanceExtensions();

        const VkDebugReportCallbackCreateInfoEXT dbg_info = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, nullptr,
                                                             VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                                             DbgCallback};

        const VkApplicationInfo app_info = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, APP_SHORT_NAME, 1, nullptr, 0, instance_version};

        AppCompileInstanceExtensionsToEnable();

        std::vector<const char *> inst_exts;
        for (const auto &ext : inst_extensions) {
            inst_exts.push_back(ext.c_str());
        }

        const VkInstanceCreateInfo inst_info = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            &dbg_info,
            (CheckExtensionEnabled(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
                 ? static_cast<VkInstanceCreateFlags>(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR)
                 : 0),
            &app_info,
            0,
            nullptr,
            static_cast<uint32_t>(inst_exts.size()),
            inst_exts.data()};

        VkResult err = vkCreateInstance(&inst_info, nullptr, &instance);
        if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
            std::cerr << "Cannot create " API_NAME " instance.\n";
            std::cerr << "This problem is often caused by a faulty installation of the " API_NAME
                         " driver or attempting to use a GPU "
                         "that does not support " API_NAME ".\n";
            THROW_VK_ERR("vkCreateInstance", err);
        } else if (err) {
            THROW_VK_ERR("vkCreateInstance", err);
        }

        volkLoadInstance(instance);

        err = vkCreateDebugReportCallbackEXT(instance, &dbg_info, nullptr, &debug_callback);
        if (err != VK_SUCCESS) {
            THROW_VK_ERR("vkCreateDebugReportCallbackEXT", err);
        }
    }

    ~AppInstance() {
        if (debug_callback) vkDestroyDebugReportCallbackEXT(instance, debug_callback, nullptr);
        if (vkDestroyInstance) vkDestroyInstance(instance, nullptr);
        volkFinalize();
    }

    AppInstance(const AppInstance &) = delete;
    const AppInstance &operator=(const AppInstance &) = delete;

    bool CheckExtensionEnabled(std::string extension_to_check) const {
        return std::any_of(inst_extensions.begin(), inst_extensions.end(),
                           [extension_to_check](std::string str) { return str == extension_to_check; });
    }

    /* Gets a list of layer and instance extensions */
    void AppGetInstanceExtensions() {
        /* Scan layers */
        auto global_layer_properties =
            GetVector<VkLayerProperties>("vkEnumerateInstanceLayerProperties", vkEnumerateInstanceLayerProperties);

        for (const auto &layer : global_layer_properties) {
            global_layers.push_back(LayerExtensionList{layer, AppGetGlobalLayerExtensions(layer.layerName)});
        }

        // Collect global extensions
        // Gets instance extensions, if no layer was specified in the first paramteter
        global_extensions = AppGetGlobalLayerExtensions(nullptr);
    }
    void AppCompileInstanceExtensionsToEnable() {
#if defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_IOS_MVK)
        bool metal_surface_available = false;
        for (const auto &ext : global_extensions) {
            if (strcmp("VK_EXT_metal_surface", ext.extensionName) == 0) {
                metal_surface_available = true;
            }
        }
#endif

        for (const auto &ext : global_extensions) {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_KHR_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            if (strcmp(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
            if (strcmp(VK_FUCHSIA_IMAGEPIPE_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_IOS_MVK
            if (strcmp(VK_MVK_IOS_SURFACE_EXTENSION_NAME, ext.extensionName) == 0 && !metal_surface_available) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
            if (strcmp(VK_MVK_MACOS_SURFACE_EXTENSION_NAME, ext.extensionName) == 0 && !metal_surface_available) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
            if (strcmp(VK_EXT_METAL_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_VI_NN
            if (strcmp(VK_NN_VI_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            if (strcmp(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
            if (strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
            if (strcmp(VK_KHR_XCB_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
            if (strcmp(VK_KHR_XLIB_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
            if (strcmp(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_GGP
            if (strcmp(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            if (strcmp(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
#endif
            if (strcmp(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
            if (strcmp(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, ext.extensionName) == 0) {
                inst_extensions.push_back(ext.extensionName);
            }
        }
    }

    void AddSurfaceExtension(SurfaceExtension ext) { surface_extensions.push_back(ext); }

    std::vector<VkExtensionProperties> AppGetGlobalLayerExtensions(const char *layer_name) {
        return GetVector<VkExtensionProperties>("vkEnumerateInstanceExtensionProperties", vkEnumerateInstanceExtensionProperties,
                                                layer_name);
    }

    std::vector<VkPhysicalDevice> FindPhysicalDevices() {
        return GetVector<VkPhysicalDevice>("vkEnumeratePhysicalDevices", vkEnumeratePhysicalDevices, instance);
    }

    std::vector<VkExtensionProperties> AppGetPhysicalDeviceLayerExtensions(VkPhysicalDevice phys_device, const char *layer_name) {
        return GetVector<VkExtensionProperties>("vkEnumerateDeviceExtensionProperties", vkEnumerateDeviceExtensionProperties,
                                                phys_device, layer_name);
    }
};

// --------- Platform Specific Presentation Calls --------- //

#if defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WIN32_KHR) ||      \
    defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT) || defined(VK_USE_PLATFORM_WAYLAND_KHR) || \
    defined(VK_USE_PLATFORM_DIRECTFB_EXT) || defined(VK_USE_PLATFORM_GGP) || defined(VK_USE_PLATFORM_SCREEN_QNX)

#define VULKANINFO_WSI_ENABLED
#endif

//-----------------------------------------------------------
#if defined(VULKANINFO_WSI_ENABLED)
static void AppDestroySurface(AppInstance &inst, VkSurfaceKHR surface) {  // same for all platforms
    vkDestroySurfaceKHR(inst.instance, surface, nullptr);
}
#endif  // defined(VULKANINFO_WSI_ENABLED)
//-----------------------------------------------------------

//---------------------------Win32---------------------------
#ifdef VK_USE_PLATFORM_WIN32_KHR

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return user32_handles->pfnDefWindowProcA(hWnd, uMsg, wParam, lParam);
}

static void AppCreateWin32Window(AppInstance &inst) {
    inst.h_instance = GetModuleHandle(nullptr);

    WNDCLASSEX win_class;

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = inst.h_instance;
    win_class.hIcon = user32_handles->pfnLoadIconA(nullptr, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = nullptr;
    win_class.lpszClassName = APP_SHORT_NAME;
    win_class.hInstance = inst.h_instance;
    win_class.hIconSm = user32_handles->pfnLoadIconA(nullptr, IDI_WINLOGO);
    // Register window class:
    if (!user32_handles->pfnRegisterClassExA(&win_class)) {
        // It didn't work, so try to give a useful error:
        THROW_ERR("Failed to register the window class!");
    }
    // Create window with the registered class:
    RECT wr = {0, 0, inst.width, inst.height};
    user32_handles->pfnAdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    inst.h_wnd = user32_handles->pfnCreateWindowExA(0,
                                                    APP_SHORT_NAME,  // class name
                                                    APP_SHORT_NAME,  // app name
                                                    // WS_VISIBLE | WS_SYSMENU |
                                                    WS_OVERLAPPEDWINDOW,  // window style
                                                    100, 100,             // x/y coords
                                                    wr.right - wr.left,   // width
                                                    wr.bottom - wr.top,   // height
                                                    nullptr,              // handle to parent
                                                    nullptr,              // handle to menu
                                                    inst.h_instance,      // hInstance
                                                    nullptr);             // no extra parameters
    if (!inst.h_wnd) {
        // It didn't work, so try to give a useful error:
        THROW_ERR("Failed to create a window!");
    }
}

static VkSurfaceKHR AppCreateWin32Surface(AppInstance &inst) {
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = inst.h_instance;
    createInfo.hwnd = inst.h_wnd;

    VkSurfaceKHR surface;
    VkResult err = vkCreateWin32SurfaceKHR(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateWin32SurfaceKHR", err);
    return surface;
}

static void AppDestroyWin32Window(AppInstance &inst) { user32_handles->pfnDestroyWindow(inst.h_wnd); }
#endif  // VK_USE_PLATFORM_WIN32_KHR
//-----------------------------------------------------------

//----------------------------XCB----------------------------
#ifdef VK_USE_PLATFORM_XCB_KHR
static void AppCreateXcbWindow(AppInstance &inst) {
    //--Init Connection--
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    // API guarantees non-null xcb_connection
    inst.xcb_connection = xcb_connect(nullptr, &scr);
    int conn_error = xcb_connection_has_error(inst.xcb_connection);
    if (conn_error) {
        fprintf(stderr, "XCB failed to connect to the X server due to error:%d.\n", conn_error);
        fflush(stderr);
        xcb_disconnect(inst.xcb_connection);
        inst.xcb_connection = nullptr;
        return;
    }

    setup = xcb_get_setup(inst.xcb_connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) {
        xcb_screen_next(&iter);
    }

    inst.xcb_screen = iter.data;
    //-------------------

    inst.xcb_window = xcb_generate_id(inst.xcb_connection);
    xcb_create_window(inst.xcb_connection, XCB_COPY_FROM_PARENT, inst.xcb_window, inst.xcb_screen->root, 0, 0, inst.width,
                      inst.height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, inst.xcb_screen->root_visual, 0, nullptr);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(inst.xcb_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(inst.xcb_connection, cookie, 0);
    free(reply);
}

static VkSurfaceKHR AppCreateXcbSurface(AppInstance &inst) {
    if (!inst.xcb_connection) {
        THROW_ERR("AppCreateXcbSurface failed to establish connection");
    }

    VkXcbSurfaceCreateInfoKHR xcb_createInfo;
    xcb_createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcb_createInfo.pNext = nullptr;
    xcb_createInfo.flags = 0;
    xcb_createInfo.connection = inst.xcb_connection;
    xcb_createInfo.window = inst.xcb_window;

    VkSurfaceKHR surface;
    VkResult err = vkCreateXcbSurfaceKHR(inst.instance, &xcb_createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateXcbSurfaceKHR", err);
    return surface;
}

static void AppDestroyXcbWindow(AppInstance &inst) {
    if (!inst.xcb_connection) {
        return;  // Nothing to destroy
    }

    xcb_destroy_window(inst.xcb_connection, inst.xcb_window);
    xcb_disconnect(inst.xcb_connection);
}
#endif  // VK_USE_PLATFORM_XCB_KHR
//-----------------------------------------------------------

//----------------------------XLib---------------------------
#ifdef VK_USE_PLATFORM_XLIB_KHR
static void AppCreateXlibWindow(AppInstance &inst) {
    long visualMask = VisualScreenMask;
    int numberOfVisuals{};

    inst.xlib_display = XOpenDisplay(nullptr);
    if (inst.xlib_display == nullptr) {
        THROW_ERR("XLib failed to connect to the X server.\nExiting...");
    }

    XVisualInfo vInfoTemplate = {};
    vInfoTemplate.screen = DefaultScreen(inst.xlib_display);
    XVisualInfo *visualInfoBegin = XGetVisualInfo(inst.xlib_display, visualMask, &vInfoTemplate, &numberOfVisuals);
    XVisualInfo *visualInfoEnd = visualInfoBegin + numberOfVisuals;
    const Visual *rootVisual = DefaultVisual(inst.xlib_display, vInfoTemplate.screen);
    const XVisualInfo *foundVisualInfo =
        std::find_if(visualInfoBegin, visualInfoEnd, [rootVisual](const XVisualInfo &vi) { return vi.visual == rootVisual; });
    const XVisualInfo *visualInfo = foundVisualInfo == visualInfoEnd ? visualInfoBegin : foundVisualInfo;
    inst.xlib_window = XCreateWindow(inst.xlib_display, RootWindow(inst.xlib_display, vInfoTemplate.screen), 0, 0, inst.width,
                                     inst.height, 0, visualInfo->depth, InputOutput, visualInfo->visual, 0, nullptr);

    XSync(inst.xlib_display, false);
    XFree(visualInfoBegin);
}

static VkSurfaceKHR AppCreateXlibSurface(AppInstance &inst) {
    VkXlibSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.dpy = inst.xlib_display;
    createInfo.window = inst.xlib_window;

    VkSurfaceKHR surface;
    VkResult err = vkCreateXlibSurfaceKHR(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateXlibSurfaceKHR", err);
    return surface;
}

static void AppDestroyXlibWindow(AppInstance &inst) {
    XDestroyWindow(inst.xlib_display, inst.xlib_window);
    XCloseDisplay(inst.xlib_display);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR
//-----------------------------------------------------------

//------------------------MACOS_MVK--------------------------
#ifdef VK_USE_PLATFORM_MACOS_MVK
static void AppCreateMacOSWindow(AppInstance &inst) {
    inst.macos_window = CreateMetalView(inst.width, inst.height);
    if (inst.macos_window == nullptr) {
        THROW_ERR("Could not create a native Metal view.\nExiting...");
    }
}

static VkSurfaceKHR AppCreateMacOSSurface(AppInstance &inst) {
    VkMacOSSurfaceCreateInfoMVK createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pView = inst.macos_window;

    VkSurfaceKHR surface;
    VkResult err = vkCreateMacOSSurfaceMVK(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateMacOSSurfaceMVK", err);
    return surface;
}

static void AppDestroyMacOSWindow(AppInstance &inst) { DestroyMetalView(inst.macos_window); }
#endif  // VK_USE_PLATFORM_MACOS_MVK
//-----------------------------------------------------------

//------------------------METAL_EXT--------------------------
#ifdef VK_USE_PLATFORM_METAL_EXT
static void AppCreateMetalWindow(AppInstance &inst) {
    inst.metal_window = CreateMetalView(inst.width, inst.height);
    if (inst.metal_window == nullptr) {
        THROW_ERR("Could not create a native Metal view.\nExiting...");
    }
}

static VkSurfaceKHR AppCreateMetalSurface(AppInstance &inst) {
    VkMetalSurfaceCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pLayer = static_cast<CAMetalLayer *>(GetCAMetalLayerFromMetalView(inst.metal_window));

    VkSurfaceKHR surface;
    VkResult err = vkCreateMetalSurfaceEXT(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateMetalSurfaceEXT", err);
    return surface;
}

static void AppDestroyMetalWindow(AppInstance &inst) { DestroyMetalView(inst.metal_window); }
#endif  // VK_USE_PLATFORM_METAL_EXT
//-----------------------------------------------------------

//-------------------------WAYLAND---------------------------
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static void wayland_registry_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
                                    uint32_t version) {
    AppInstance &inst = *static_cast<AppInstance *>(data);
    if (strcmp(interface, "wl_compositor") == 0) {
        struct wl_compositor *compositor = (struct wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
        inst.wayland_surface = wl_compositor_create_surface(compositor);
    }
}
static void wayland_registry_global_remove(void *data, struct wl_registry *registry, uint32_t id) {}
static const struct wl_registry_listener wayland_registry_listener = {wayland_registry_global, wayland_registry_global_remove};

static void AppCreateWaylandWindow(AppInstance &inst) {
    inst.wayland_display = wl_display_connect(nullptr);
    struct wl_registry *registry = wl_display_get_registry(inst.wayland_display);
    wl_registry_add_listener(wl_display_get_registry(inst.wayland_display), &wayland_registry_listener, static_cast<void *>(&inst));
    wl_display_roundtrip(inst.wayland_display);
    wl_registry_destroy(registry);
}

static VkSurfaceKHR AppCreateWaylandSurface(AppInstance &inst) {
    VkWaylandSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.display = inst.wayland_display;
    createInfo.surface = inst.wayland_surface;

    VkSurfaceKHR surface;
    VkResult err = vkCreateWaylandSurfaceKHR(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateWaylandSurfaceKHR", err);
    return surface;
}

static void AppDestroyWaylandWindow(AppInstance &inst) { wl_display_disconnect(inst.wayland_display); }
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
//-----------------------------------------------------------

//-------------------------DIRECTFB--------------------------
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
static void AppCreateDirectFBWindow(AppInstance &inst) {
    DFBResult ret;

    ret = DirectFBInit(NULL, NULL);
    if (ret) {
        THROW_ERR("DirectFBInit failed to initialize DirectFB.\nExiting...");
    }

    ret = DirectFBCreate(&inst.dfb);
    if (ret) {
        THROW_ERR("DirectFBCreate failed to create main interface of DirectFB.\nExiting...");
    }

    DFBSurfaceDescription desc;
    desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
    desc.caps = DSCAPS_PRIMARY;
    desc.width = inst.width;
    desc.height = inst.height;
    ret = inst.dfb->CreateSurface(inst.dfb, &desc, &inst.directfb_surface);
    if (ret) {
        THROW_ERR("CreateSurface failed to create DirectFB surface interface.\nExiting...");
    }
}

static VkSurfaceKHR AppCreateDirectFBSurface(AppInstance &inst) {
    VkDirectFBSurfaceCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.dfb = inst.dfb;
    createInfo.surface = inst.directfb_surface;

    VkSurfaceKHR surface;
    VkResult err = vkCreateDirectFBSurfaceEXT(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateDirectFBSurfaceEXT", err);
    return surface;
}

static void AppDestroyDirectFBWindow(AppInstance &inst) {
    inst.directfb_surface->Release(inst.directfb_surface);
    inst.dfb->Release(inst.dfb);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
//-----------------------------------------------------------

//-------------------------ANDROID---------------------------
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static void AppCreateAndroidWindow(AppInstance &inst) {}
static VkSurfaceKHR AppCreateAndroidSurface(AppInstance &inst) {
    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.window = (struct ANativeWindow *)(inst.window);

    VkSurfaceKHR surface;
    VkResult err = vkCreateAndroidSurfaceKHR(inst.instance, &createInfo, NULL, &surface);
    if (err) THROW_VK_ERR("vkCreateAndroidSurfaceKHR", err);
    return surface;
}
static void AppDestroyAndroidWindow(AppInstance &inst) {}
#endif
//-----------------------------------------------------------
//---------------------------GGP-----------------------------
#ifdef VK_USE_PLATFORM_GGP
static void AppCreateGgpWindow(AppInstance &inst) {}
static VkSurfaceKHR AppCreateGgpSurface(AppInstance &inst) {
    VkStreamDescriptorSurfaceCreateInfoGGP createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.streamDescriptor = 1;

    VkSurfaceKHR surface;
    VkResult err = vkCreateStreamDescriptorSurfaceGGP(inst.instance, &createInfo, NULL, &surface);
    if (err) THROW_VK_ERR("vkCreateStreamDescriptorSurfaceGGP", err);
    return surface;
}
static void AppDestroyGgpWindow(AppInstance &inst) {}
#endif
//-----------------------------------------------------------
//----------------------QNX SCREEN---------------------------
#ifdef VK_USE_PLATFORM_SCREEN_QNX
static void AppCreateScreenWindow(AppInstance &inst) {
    int usage = SCREEN_USAGE_VULKAN;
    int rc;

    rc = screen_create_context(&inst.context, 0);
    if (rc) {
        THROW_ERR("Could not create a QNX Screen context.\nExiting...");
    }
    rc = screen_create_window(&inst.window, inst.context);
    if (rc) {
        THROW_ERR("Could not create a QNX Screen window.\nExiting...");
    }
    rc = screen_set_window_property_iv(inst.window, SCREEN_PROPERTY_USAGE, &usage);
    if (rc) {
        THROW_ERR("Could not set SCREEN_USAGE_VULKAN flag for QNX Screen window!\nExiting...");
    }
}

static VkSurfaceKHR AppCreateScreenSurface(AppInstance &inst) {
    VkScreenSurfaceCreateInfoQNX createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.context = inst.context;
    createInfo.window = inst.window;

    VkSurfaceKHR surface;
    VkResult err = vkCreateScreenSurfaceQNX(inst.instance, &createInfo, nullptr, &surface);
    if (err) THROW_VK_ERR("vkCreateScreenSurfaceQNX", err);
    return surface;
}

static void AppDestroyScreenWindow(AppInstance &inst) {
    screen_destroy_window(inst.window);
    screen_destroy_context(inst.context);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
//-----------------------------------------------------------
// ------------ Setup Windows ------------- //

void SetupWindowExtensions(AppInstance &inst) {
#if defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
    bool has_display = true;
    const char *display_var = getenv("DISPLAY");
    if (display_var == nullptr || strlen(display_var) == 0) {
        has_display = false;
        std::cerr << "'DISPLAY' environment variable not set... skipping surface info\n";
    }
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    wl_display *wayland_display = wl_display_connect(nullptr);
    bool has_wayland_display = false;
    if (wayland_display != nullptr) {
        wl_display_disconnect(wayland_display);
        has_wayland_display = true;
    }
#endif

//--WIN32--
#ifdef VK_USE_PLATFORM_WIN32_KHR
    SurfaceExtension surface_ext_win32;
    if (inst.CheckExtensionEnabled(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        surface_ext_win32.name = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        surface_ext_win32.create_window = AppCreateWin32Window;
        surface_ext_win32.create_surface = AppCreateWin32Surface;
        surface_ext_win32.destroy_window = AppDestroyWin32Window;

        inst.AddSurfaceExtension(surface_ext_win32);
    }
#endif
//--XCB--
#ifdef VK_USE_PLATFORM_XCB_KHR
    SurfaceExtension surface_ext_xcb;
    if (inst.CheckExtensionEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        surface_ext_xcb.name = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        surface_ext_xcb.create_window = AppCreateXcbWindow;
        surface_ext_xcb.create_surface = AppCreateXcbSurface;
        surface_ext_xcb.destroy_window = AppDestroyXcbWindow;
        if (has_display) {
            inst.AddSurfaceExtension(surface_ext_xcb);
        }
    }
#endif
//--XLIB--
#ifdef VK_USE_PLATFORM_XLIB_KHR
    SurfaceExtension surface_ext_xlib;
    if (inst.CheckExtensionEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        surface_ext_xlib.name = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
        surface_ext_xlib.create_window = AppCreateXlibWindow;
        surface_ext_xlib.create_surface = AppCreateXlibSurface;
        surface_ext_xlib.destroy_window = AppDestroyXlibWindow;
        if (has_display) {
            inst.AddSurfaceExtension(surface_ext_xlib);
        }
    }
#endif
//--MACOS--
#ifdef VK_USE_PLATFORM_MACOS_MVK
    SurfaceExtension surface_ext_macos;
    if (inst.CheckExtensionEnabled(VK_MVK_MACOS_SURFACE_EXTENSION_NAME) && !inst.CheckExtensionEnabled("VK_EXT_metal_surface")) {
        surface_ext_macos.name = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
        surface_ext_macos.create_window = AppCreateMacOSWindow;
        surface_ext_macos.create_surface = AppCreateMacOSSurface;
        surface_ext_macos.destroy_window = AppDestroyMacOSWindow;

        inst.AddSurfaceExtension(surface_ext_macos);
    }
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
    SurfaceExtension surface_ext_metal;
    if (inst.CheckExtensionEnabled(VK_EXT_METAL_SURFACE_EXTENSION_NAME)) {
        surface_ext_metal.name = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
        surface_ext_metal.create_window = AppCreateMetalWindow;
        surface_ext_metal.create_surface = AppCreateMetalSurface;
        surface_ext_metal.destroy_window = AppDestroyMetalWindow;

        inst.AddSurfaceExtension(surface_ext_metal);
    }
#endif
//--WAYLAND--
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    SurfaceExtension surface_ext_wayland;
    if (inst.CheckExtensionEnabled(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)) {
        surface_ext_wayland.name = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
        surface_ext_wayland.create_window = AppCreateWaylandWindow;
        surface_ext_wayland.create_surface = AppCreateWaylandSurface;
        surface_ext_wayland.destroy_window = AppDestroyWaylandWindow;
        if (has_wayland_display) {
            inst.AddSurfaceExtension(surface_ext_wayland);
        }
    }
#endif
//--DIRECTFB--
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
    SurfaceExtension surface_ext_directfb;
    if (inst.CheckExtensionEnabled(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME)) {
        surface_ext_directfb.name = VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;
        surface_ext_directfb.create_window = AppCreateDirectFBWindow;
        surface_ext_directfb.create_surface = AppCreateDirectFBSurface;
        surface_ext_directfb.destroy_window = AppDestroyDirectFBWindow;

        inst.AddSurfaceExtension(surface_ext_directfb);
    }
#endif
//--ANDROID--
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    SurfaceExtension surface_ext_android;
    if (inst.CheckExtensionEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        surface_ext_android.name = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
        surface_ext_android.create_window = AppCreateAndroidWindow;
        surface_ext_android.create_surface = AppCreateAndroidSurface;
        surface_ext_android.destroy_window = AppDestroyAndroidWindow;

        inst.AddSurfaceExtension(surface_ext_android);
    }
#endif
//--GGP--
#ifdef VK_USE_PLATFORM_GGP
    SurfaceExtension surface_ext_ggp;
    if (inst.CheckExtensionEnabled(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME)) {
        surface_ext_ggp.name = VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME;
        surface_ext_ggp.create_window = AppCreateGgpWindow;
        surface_ext_ggp.create_surface = AppCreateGgpSurface;
        surface_ext_ggp.destroy_window = AppDestroyGgpWindow;

        inst.AddSurfaceExtension(surface_ext_ggp);
    }
#endif
//--QNX_SCREEN--
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    SurfaceExtension surface_ext_qnx_screen;
    if (inst.CheckExtensionEnabled(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME)) {
        surface_ext_qnx_screen.name = VK_QNX_SCREEN_SURFACE_EXTENSION_NAME;
        surface_ext_qnx_screen.create_window = AppCreateScreenWindow;
        surface_ext_qnx_screen.create_surface = AppCreateScreenSurface;
        surface_ext_qnx_screen.destroy_window = AppDestroyScreenWindow;

        inst.AddSurfaceExtension(surface_ext_qnx_screen);
    }
#endif
}

// ---------- Surfaces -------------- //

class AppSurface {
  public:
    AppInstance &inst;
    VkPhysicalDevice phys_device;
    SurfaceExtension surface_extension;

    std::vector<VkPresentModeKHR> surf_present_modes;

    std::vector<VkSurfaceFormatKHR> surf_formats;
    std::vector<VkSurfaceFormat2KHR> surf_formats2;

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    VkSurfaceCapabilities2KHR surface_capabilities2_khr{};
    VkSurfaceCapabilities2EXT surface_capabilities2_ext{};

    std::unique_ptr<surface_capabilities2_chain> chain_for_surface_capabilities2;

    AppSurface(AppInstance &inst, AppGpu &gpu, VkPhysicalDevice phys_device, SurfaceExtension surface_extension)
        : inst(inst), phys_device(phys_device), surface_extension(surface_extension) {
        surf_present_modes =
            GetVector<VkPresentModeKHR>("vkGetPhysicalDeviceSurfacePresentModesKHR", vkGetPhysicalDeviceSurfacePresentModesKHR,
                                        phys_device, surface_extension.surface);

        if (inst.CheckExtensionEnabled(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
            const VkPhysicalDeviceSurfaceInfo2KHR surface_info2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr,
                                                                   surface_extension.surface};
            VkSurfaceFormat2KHR init{};
            init.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
            surf_formats2 = GetVectorInit<VkSurfaceFormat2KHR>(
                "vkGetPhysicalDeviceSurfaceFormats2KHR", vkGetPhysicalDeviceSurfaceFormats2KHR, init, phys_device, &surface_info2);
        } else {
            surf_formats =
                GetVector<VkSurfaceFormatKHR>("vkGetPhysicalDeviceSurfaceFormatsKHR", vkGetPhysicalDeviceSurfaceFormatsKHR,
                                              phys_device, surface_extension.surface);
        }

        if (inst.CheckExtensionEnabled(VK_KHR_SURFACE_EXTENSION_NAME)) {
            VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface_extension.surface, &surface_capabilities);
            if (err) THROW_VK_ERR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", err);
        }

        if (inst.CheckExtensionEnabled(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) {
            surface_capabilities2_khr.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
            setup_surface_capabilities2_chain(surface_capabilities2_khr, chain_for_surface_capabilities2, inst, gpu);

            VkPhysicalDeviceSurfaceInfo2KHR surface_info{};
            surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
            surface_info.surface = surface_extension.surface;
#if defined(WIN32)
            VkSurfaceFullScreenExclusiveWin32InfoEXT win32_fullscreen_exclusive_info{};
            win32_fullscreen_exclusive_info.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
            win32_fullscreen_exclusive_info.hmonitor = MonitorFromWindow(inst.h_wnd, MONITOR_DEFAULTTOPRIMARY);

            surface_info.pNext = static_cast<void *>(&win32_fullscreen_exclusive_info);
#endif  // defined(WIN32)
            VkResult err = vkGetPhysicalDeviceSurfaceCapabilities2KHR(phys_device, &surface_info, &surface_capabilities2_khr);
            if (err) THROW_VK_ERR("vkGetPhysicalDeviceSurfaceCapabilities2KHR", err);
        }

        if (inst.CheckExtensionEnabled(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME)) {
            surface_capabilities2_ext.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT;
            surface_capabilities2_ext.pNext = nullptr;
            VkResult err =
                vkGetPhysicalDeviceSurfaceCapabilities2EXT(phys_device, surface_extension.surface, &surface_capabilities2_ext);
            if (err) THROW_VK_ERR("vkGetPhysicalDeviceSurfaceCapabilities2EXT", err);
        }
    }

    AppSurface(const AppSurface &) = delete;
    const AppSurface &operator=(const AppSurface &) = delete;
};

// -------------------- Device Groups ------------------------//

std::vector<VkPhysicalDeviceGroupProperties> GetGroups(AppInstance &inst) {
    if (inst.CheckExtensionEnabled(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME)) {
        VkPhysicalDeviceGroupProperties group_props{};
        group_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
        return GetVectorInit<VkPhysicalDeviceGroupProperties>("vkEnumeratePhysicalDeviceGroupsKHR",
                                                              vkEnumeratePhysicalDeviceGroupsKHR, group_props, inst.instance);
    }
    return {};
}

std::vector<VkPhysicalDeviceProperties> GetGroupProps(AppInstance &inst, VkPhysicalDeviceGroupProperties group) {
    std::vector<VkPhysicalDeviceProperties> props(group.physicalDeviceCount);

    for (uint32_t i = 0; i < group.physicalDeviceCount; ++i) {
        vkGetPhysicalDeviceProperties(group.physicalDevices[i], &props[i]);
    }

    return props;
}

util::vulkaninfo_optional<VkDeviceGroupPresentCapabilitiesKHR> GetGroupCapabilities(AppInstance &inst,
                                                                                    VkPhysicalDeviceGroupProperties group) {
    // Build create info for logical device made from all physical devices in this group.
    std::vector<std::string> extensions_list = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DEVICE_GROUP_EXTENSION_NAME};

#ifdef VK_ENABLE_BETA_EXTENSIONS
    for (const auto &extension : inst.AppGetPhysicalDeviceLayerExtensions(group.physicalDevices[0], nullptr)) {
        if (std::string(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == extension.extensionName) {
            extensions_list.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }
    }
#endif

    VkDeviceGroupDeviceCreateInfoKHR dg_ci = {VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR, nullptr,
                                              group.physicalDeviceCount, group.physicalDevices};

    float queue_priority = 1.0f;

    auto ext_list = get_c_str_array(extensions_list);

    VkDeviceQueueCreateInfo q_ci = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queue_priority};
    VkDeviceCreateInfo device_ci = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,   &dg_ci,         0, 1, &q_ci, 0, nullptr,
                                    static_cast<uint32_t>(ext_list.size()), ext_list.data()};

    VkDevice logical_device = VK_NULL_HANDLE;

    VkResult err = vkCreateDevice(group.physicalDevices[0], &device_ci, nullptr, &logical_device);
    if (err != VK_SUCCESS && err != VK_ERROR_EXTENSION_NOT_PRESENT) THROW_VK_ERR("vkCreateDevice", err);

    if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
        vkDestroyDevice(logical_device, nullptr);
        return {};
    }

    VkDeviceGroupPresentCapabilitiesKHR group_capabilities = {VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR, nullptr};

    // If the KHR_device_group extension is present, write the capabilities of the logical device into a struct for later
    // output to user.
    err = vkGetDeviceGroupPresentCapabilitiesKHR(logical_device, &group_capabilities);
    if (err) THROW_VK_ERR("vkGetDeviceGroupPresentCapabilitiesKHR", err);

    vkDestroyDevice(logical_device, nullptr);

    return {group_capabilities};
}

// -------------------- Device Setup ------------------- //

const VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

struct ImageTypeSupport {
    enum class Type { regular, sparse, transient } type;
    uint32_t memoryTypeBits;

    bool Compatible(uint32_t memtype_bit) { return memoryTypeBits & memtype_bit; }
};

struct ImageTypeFormatInfo {
    VkFormat format;
    std::vector<ImageTypeSupport> type_support;
};

struct ImageTypeInfos {
    VkImageTiling tiling;
    std::vector<ImageTypeFormatInfo> formats;
};

VkImageCreateInfo GetImageCreateInfo(VkImageCreateFlags flags, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usages) {
    return {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            nullptr,
            flags,
            VK_IMAGE_TYPE_2D,
            format,
            {8, 8, 1},
            1,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            tiling,
            usages,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr,
            VK_IMAGE_LAYOUT_UNDEFINED};
}

util::vulkaninfo_optional<ImageTypeSupport> FillImageTypeSupport(AppInstance &inst, VkPhysicalDevice phys_device, VkDevice device,
                                                                 ImageTypeSupport::Type img_type, VkImageCreateInfo image_ci) {
    VkImageFormatProperties img_props;
    VkResult res = vkGetPhysicalDeviceImageFormatProperties(phys_device, image_ci.format, image_ci.imageType, image_ci.tiling,
                                                            image_ci.usage, image_ci.flags, &img_props);

    if (res == VK_SUCCESS) {
        ImageTypeSupport img_type_support{};
        img_type_support.type = img_type;

        VkImage dummy_img;
        res = vkCreateImage(device, &image_ci, nullptr, &dummy_img);
        if (res) THROW_VK_ERR("vkCreateImage", res);

        VkMemoryRequirements mem_req;
        vkGetImageMemoryRequirements(device, dummy_img, &mem_req);
        img_type_support.memoryTypeBits = mem_req.memoryTypeBits;

        vkDestroyImage(device, dummy_img, nullptr);
        return img_type_support;
    } else if (res == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        return {};  // return empty util::vulkaninfo_optional
    }
    THROW_VK_ERR("vkGetPhysicalDeviceImageFormatProperties", res);
    return {};
}

struct FormatRange {
    // the Vulkan standard version that supports this format range, or 0 if non-standard
    APIVersion minimum_instance_version;

    // The name of the extension that supports this format range, or NULL if the range
    // is only part of the standard
    const char *extension_name;

    // The first and last supported formats within this range.
    VkFormat first_format;
    VkFormat last_format;
};

struct AppQueueFamilyProperties {
    VkQueueFamilyProperties props;
    uint32_t queue_index;
    void *pNext = nullptr;  // assumes the lifetime of the pNext chain outlives this object, eg parent object must keep both alive
    bool can_present = false;
    bool can_always_present = true;
    std::vector<std::pair<std::string, VkBool32>> present_support;
    AppQueueFamilyProperties(AppInstance &inst, VkPhysicalDevice physical_device, VkQueueFamilyProperties family_properties,
                             uint32_t queue_index, void *pNext = nullptr)
        : props(family_properties), queue_index(queue_index), pNext(pNext) {
        for (const auto &surface_ext : inst.surface_extensions) {
            present_support.push_back({surface_ext.name, VK_FALSE});
            VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_index, surface_ext.surface,
                                                                &present_support.back().second);
            if (err) THROW_VK_ERR("vkGetPhysicalDeviceSurfaceSupportKHR", err);
            if (present_support.back().second) {
                can_present = true;
            } else {
                can_always_present = false;
            }
        }
    }
};

struct AppGpu {
    AppInstance &inst;
    uint32_t id{};
    VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    APIVersion api_version;

    VkPhysicalDeviceProperties props{};
    VkPhysicalDeviceProperties2KHR props2{};

    // VkPhysicalDeviceDriverProperties
    VkDriverId driverID;
    char driverName[VK_MAX_DRIVER_NAME_SIZE];
    char driverInfo[VK_MAX_DRIVER_INFO_SIZE];
    VkConformanceVersion conformanceVersion;
    // VkPhysicalDeviceIDProperties
    uint8_t deviceUUID[VK_UUID_SIZE];
    uint8_t driverUUID[VK_UUID_SIZE];
    uint8_t deviceLUID[VK_LUID_SIZE];
    uint32_t deviceNodeMask;
    VkBool32 deviceLUIDValid;

    bool found_driver_props = false;
    bool found_device_id_props = false;

    std::vector<VkQueueFamilyProperties> queue_props;
    std::vector<VkQueueFamilyProperties2KHR> queue_props2;
    std::vector<AppQueueFamilyProperties> extended_queue_props;

    VkPhysicalDeviceMemoryProperties memory_props{};
    VkPhysicalDeviceMemoryProperties2KHR memory_props2{};

    std::vector<ImageTypeInfos> memory_image_support_types;

    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceFeatures2KHR features2{};

    std::vector<VkExtensionProperties> device_extensions;

    VkDevice dev = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures enabled_features{};

    std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapBudget;
    std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapUsage;

    std::unique_ptr<phys_device_props2_chain> chain_for_phys_device_props2;
    std::unique_ptr<phys_device_mem_props2_chain> chain_for_phys_device_mem_props2;
    std::unique_ptr<phys_device_features2_chain> chain_for_phys_device_features2;
    std::vector<std::unique_ptr<queue_properties2_chain>> chain_for_queue_props2;

    std::vector<std::unique_ptr<AppVideoProfile>> video_profiles;

    AppGpu(AppInstance &inst, uint32_t id, VkPhysicalDevice phys_device, bool show_promoted_structs)
        : inst(inst), id(id), phys_device(phys_device) {
        vkGetPhysicalDeviceProperties(phys_device, &props);

        // needs to find the minimum of the instance and device version, and use that to print the device info
        api_version = APIVersion(props.apiVersion);

        device_extensions = inst.AppGetPhysicalDeviceLayerExtensions(phys_device, nullptr);

        vkGetPhysicalDeviceMemoryProperties(phys_device, &memory_props);

        vkGetPhysicalDeviceFeatures(phys_device, &features);

        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_count, nullptr);
        queue_props.resize(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_count, queue_props.data());

        if (inst.CheckExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            // VkPhysicalDeviceProperties2
            props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
            setup_phys_device_props2_chain(props2, chain_for_phys_device_props2, inst, *this, show_promoted_structs);

            vkGetPhysicalDeviceProperties2KHR(phys_device, &props2);
            prepare_phys_device_props2_twocall_chain_vectors(chain_for_phys_device_props2);
            vkGetPhysicalDeviceProperties2KHR(phys_device, &props2);

            // VkPhysicalDeviceMemoryProperties2
            memory_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
            setup_phys_device_mem_props2_chain(memory_props2, chain_for_phys_device_mem_props2, *this);

            vkGetPhysicalDeviceMemoryProperties2KHR(phys_device, &memory_props2);

            // VkPhysicalDeviceFeatures2
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
            setup_phys_device_features2_chain(features2, chain_for_phys_device_features2, *this, show_promoted_structs);

            vkGetPhysicalDeviceFeatures2KHR(phys_device, &features2);

            // std::vector<VkPhysicalDeviceQueueFamilyProperties2>
            uint32_t queue_prop2_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties2KHR(phys_device, &queue_prop2_count, nullptr);
            queue_props2.resize(queue_prop2_count);
            chain_for_queue_props2.resize(queue_prop2_count);
            for (size_t i = 0; i < queue_props2.size(); i++) {
                queue_props2[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR;
                setup_queue_properties2_chain(queue_props2[i], chain_for_queue_props2[i], *this);
            }
            vkGetPhysicalDeviceQueueFamilyProperties2KHR(phys_device, &queue_prop2_count, queue_props2.data());

            if (CheckPhysicalDeviceExtensionIncluded(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) ||
                api_version >= VK_API_VERSION_1_2) {
                void *place = props2.pNext;
                while (place) {
                    VkBaseOutStructure *structure = static_cast<VkBaseOutStructure *>(place);
                    if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES) {
                        VkPhysicalDeviceDriverProperties *driver_driver_properties =
                            reinterpret_cast<VkPhysicalDeviceDriverProperties *>(structure);
                        driverID = driver_driver_properties->driverID;
                        memcpy(driverName, driver_driver_properties->driverName, VK_MAX_DRIVER_NAME_SIZE);
                        memcpy(driverInfo, driver_driver_properties->driverInfo, VK_MAX_DRIVER_INFO_SIZE);
                        conformanceVersion = driver_driver_properties->conformanceVersion;
                        found_driver_props = true;
                    } else if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES) {
                        VkPhysicalDeviceIDProperties *device_id_props = reinterpret_cast<VkPhysicalDeviceIDProperties *>(structure);
                        memcpy(deviceUUID, device_id_props->deviceUUID, VK_UUID_SIZE);
                        memcpy(driverUUID, device_id_props->driverUUID, VK_UUID_SIZE);
                        memcpy(deviceLUID, device_id_props->deviceLUID, VK_LUID_SIZE);
                        deviceNodeMask = device_id_props->deviceNodeMask;
                        deviceLUIDValid = device_id_props->deviceLUIDValid;
                        found_device_id_props = true;
                    } else if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES) {
                        VkPhysicalDeviceVulkan11Properties *vulkan_11_props =
                            reinterpret_cast<VkPhysicalDeviceVulkan11Properties *>(structure);
                        memcpy(deviceUUID, vulkan_11_props->deviceUUID, VK_UUID_SIZE);
                        memcpy(driverUUID, vulkan_11_props->driverUUID, VK_UUID_SIZE);
                        memcpy(deviceLUID, vulkan_11_props->deviceLUID, VK_LUID_SIZE);
                        deviceNodeMask = vulkan_11_props->deviceNodeMask;
                        deviceLUIDValid = vulkan_11_props->deviceLUIDValid;
                        found_device_id_props = true;
                    } else if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES) {
                        VkPhysicalDeviceVulkan12Properties *vulkan_12_props =
                            reinterpret_cast<VkPhysicalDeviceVulkan12Properties *>(structure);
                        driverID = vulkan_12_props->driverID;
                        memcpy(driverName, vulkan_12_props->driverName, VK_MAX_DRIVER_NAME_SIZE);
                        memcpy(driverInfo, vulkan_12_props->driverInfo, VK_MAX_DRIVER_INFO_SIZE);
                        conformanceVersion = vulkan_12_props->conformanceVersion;
                        found_driver_props = true;
                    }
                    place = structure->pNext;
                }
            }
        }

        // Use the queue_props2 if they exist, else fallback on vulkan 1.0 queue_props
        int queue_index = 0;
        if (queue_props2.size() > 0) {
            for (auto &queue_prop : queue_props2) {
                extended_queue_props.push_back(
                    AppQueueFamilyProperties(inst, phys_device, queue_prop.queueFamilyProperties, queue_index++, queue_prop.pNext));
            }
        } else {
            for (auto &queue_prop : queue_props) {
                extended_queue_props.push_back(AppQueueFamilyProperties(inst, phys_device, queue_prop, queue_index++, nullptr));
            }
        }

        if (features.sparseBinding) {
            enabled_features.sparseBinding = VK_TRUE;
        }

        std::vector<const char *> extensions_to_enable;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        for (const auto &extension : device_extensions) {
            if (std::string(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == extension.extensionName) {
                extensions_to_enable.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
            }
        }
#endif

        const float queue_priority = 1.0f;
        // pick the first queue index and hope for the best
        const VkDeviceQueueCreateInfo q_ci = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queue_priority};
        VkDeviceCreateInfo device_ci{};
        device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_ci.queueCreateInfoCount = 1;
        device_ci.pQueueCreateInfos = &q_ci;
        device_ci.enabledExtensionCount = static_cast<uint32_t>(extensions_to_enable.size());
        device_ci.ppEnabledExtensionNames = extensions_to_enable.data();
        device_ci.pEnabledFeatures = &enabled_features;

        VkResult err = vkCreateDevice(phys_device, &device_ci, nullptr, &dev);
        if (err) THROW_VK_ERR("vkCreateDevice", err);

        const std::array<VkImageTiling, 2> tilings = {VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_TILING_LINEAR};
        const std::array<VkFormat, 8> formats = {
            color_format,      VK_FORMAT_D16_UNORM,         VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,   VK_FORMAT_D32_SFLOAT_S8_UINT};

        for (const VkImageTiling tiling : tilings) {
            ImageTypeInfos image_type_infos;
            image_type_infos.tiling = tiling;

            for (const VkFormat format : formats) {
                ImageTypeFormatInfo image_type_format_info;
                image_type_format_info.format = format;

                VkFormatProperties fmt_props;
                vkGetPhysicalDeviceFormatProperties(phys_device, format, &fmt_props);
                if ((tiling == VK_IMAGE_TILING_OPTIMAL && fmt_props.optimalTilingFeatures == 0) ||
                    (tiling == VK_IMAGE_TILING_LINEAR && fmt_props.linearTilingFeatures == 0)) {
                    continue;
                }

                VkImageCreateInfo image_ci_regular = GetImageCreateInfo(0, format, tiling, 0);
                VkImageCreateInfo image_ci_transient =
                    GetImageCreateInfo(0, format, tiling, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
                VkImageCreateInfo image_ci_sparse =
                    GetImageCreateInfo(VK_IMAGE_CREATE_SPARSE_BINDING_BIT, format, tiling, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

                if (tiling == VK_IMAGE_TILING_LINEAR) {
                    if (format == color_format) {
                        image_ci_regular.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                        image_ci_transient.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    } else {
                        // linear tiling is only applicable to color image types
                        continue;
                    }
                } else {
                    if (format == color_format) {
                        image_ci_regular.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                        image_ci_transient.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

                    } else {
                        image_ci_regular.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                        image_ci_transient.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    }
                }

                auto image_ts_regular_ret =
                    FillImageTypeSupport(inst, phys_device, dev, ImageTypeSupport::Type::regular, image_ci_regular);
                if (image_ts_regular_ret) {
                    image_type_format_info.type_support.push_back(image_ts_regular_ret.value());
                }
                auto image_ts_transient_ret =
                    FillImageTypeSupport(inst, phys_device, dev, ImageTypeSupport::Type::transient, image_ci_transient);
                if (image_ts_transient_ret) {
                    image_type_format_info.type_support.push_back(image_ts_transient_ret.value());
                }

                if (enabled_features.sparseBinding) {
                    auto image_ts_sparse_ret =
                        FillImageTypeSupport(inst, phys_device, dev, ImageTypeSupport::Type::sparse, image_ci_sparse);
                    if (image_ts_sparse_ret) {
                        image_type_format_info.type_support.push_back(image_ts_sparse_ret.value());
                    }
                }
                image_type_infos.formats.push_back(image_type_format_info);
            }
            memory_image_support_types.push_back(image_type_infos);
        }

        // Memory //

        struct VkBaseOutStructure *structure = NULL;
        if (inst.CheckExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            structure = (struct VkBaseOutStructure *)memory_props2.pNext;

            while (structure) {
                if (structure->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT &&
                    CheckPhysicalDeviceExtensionIncluded(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
                    VkPhysicalDeviceMemoryBudgetPropertiesEXT *mem_budget_props =
                        reinterpret_cast<VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(structure);
                    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; i++) {
                        heapBudget[i] = mem_budget_props->heapBudget[i];
                        heapUsage[i] = mem_budget_props->heapUsage[i];
                    }
                }

                structure = structure->pNext;
            }
        }
        // TODO buffer - memory type compatibility

        // Video //
        video_profiles = enumerate_supported_video_profiles(*this);
    }
    ~AppGpu() { vkDestroyDevice(dev, nullptr); }

    AppGpu(const AppGpu &) = delete;
    const AppGpu &operator=(const AppGpu &) = delete;

    bool CheckPhysicalDeviceExtensionIncluded(std::string extension_to_check) const {
        return std::any_of(
            device_extensions.begin(), device_extensions.end(),
            [extension_to_check](const VkExtensionProperties &prop) { return prop.extensionName == extension_to_check; });
    }

    // Helper function to determine whether a format range is currently supported.
    bool FormatRangeSupported(const FormatRange &format_range) const {
        // Formats from base vulkan spec
        if (format_range.minimum_instance_version == 0 && format_range.extension_name == nullptr) {
            return true;
        }

        // True if this extension is present
        if (format_range.extension_name != nullptr) {
            return inst.CheckExtensionEnabled(format_range.extension_name);
        }

        // True if standard and supported by both this instance and this GPU
        if (inst.api_version >= format_range.minimum_instance_version &&
            APIVersion(props.apiVersion) >= format_range.minimum_instance_version) {
            return true;
        }

        // Otherwise, not supported.
        return false;
    }

    VkPhysicalDeviceProperties GetDeviceProperties() {
        if (inst.CheckExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            return props2.properties;
        } else {
            return props;
        }
    }

    // Vendor specific driverVersion mapping scheme
    // If one isn't present, fall back to the standard Vulkan scheme
    std::string GetDriverVersionString() {
        uint32_t v = props.driverVersion;
        if ((found_driver_props && driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY) ||
            (!found_driver_props && props.deviceID == 4318)) {
            return std::to_string((v >> 22) & 0x3ff) + "." + std::to_string((v >> 14) & 0x0ff) + "." +
                   std::to_string((v >> 6) & 0x0ff) + "." + std::to_string(v & 0x003f);
        } else if ((found_driver_props && driverID == VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS)
#if defined(WIN32)
                   || (!found_driver_props && props.deviceID == 0x8086)  // only do the fallback check if running in windows
#endif
        ) {
            return std::to_string(v >> 14) + "." + std::to_string(v & 0x3fff);
        } else {
            // AMD uses the standard vulkan scheme
            return APIVersion(v).str();
        }
    }
};

std::vector<VkPhysicalDeviceToolPropertiesEXT> GetToolingInfo(AppGpu &gpu) {
    if (vkGetPhysicalDeviceToolPropertiesEXT == nullptr) return {};
    return GetVector<VkPhysicalDeviceToolPropertiesEXT>("vkGetPhysicalDeviceToolPropertiesEXT",
                                                        vkGetPhysicalDeviceToolPropertiesEXT, gpu.phys_device);
}

// --------- Format Properties ----------//
// can't use autogen because that is put in a header that we can't include because that header depends on stuff defined here
bool operator==(const VkFormatProperties &a, const VkFormatProperties b) {
    return a.linearTilingFeatures == b.linearTilingFeatures && a.optimalTilingFeatures == b.optimalTilingFeatures &&
           a.bufferFeatures == b.bufferFeatures;
}
bool operator==(const VkFormatProperties3 &a, const VkFormatProperties3 b) {
    return a.linearTilingFeatures == b.linearTilingFeatures && a.optimalTilingFeatures == b.optimalTilingFeatures &&
           a.bufferFeatures == b.bufferFeatures;
}

struct PropFlags {
    VkFormatProperties props;
    VkFormatProperties3 props3;

    bool operator==(const PropFlags &other) const { return props == other.props && props3 == other.props3; }
};

PropFlags get_format_properties(const AppGpu &gpu, VkFormat fmt) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(gpu.phys_device, fmt, &props);

    VkFormatProperties3 props3{};
    props3.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3;

    if (gpu.inst.CheckExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) &&
        gpu.CheckPhysicalDeviceExtensionIncluded(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
        props2.formatProperties = props;
        props2.pNext = static_cast<void *>(&props3);
        vkGetPhysicalDeviceFormatProperties2KHR(gpu.phys_device, fmt, &props2);
    }
    return {props, props3};
}

namespace std {
template <>
struct hash<VkFormatProperties> {
    std::size_t operator()(const VkFormatProperties &k) const {
        return ((std::hash<uint32_t>()(k.linearTilingFeatures) ^ (std::hash<uint32_t>()(k.optimalTilingFeatures) << 1)) >> 1) ^
               (std::hash<uint32_t>()(k.bufferFeatures) << 1);
    }
};
template <>
struct hash<VkFormatProperties3> {
    std::size_t operator()(const VkFormatProperties3 &k) const {
        return ((std::hash<uint64_t>()(k.linearTilingFeatures) ^ (std::hash<uint64_t>()(k.optimalTilingFeatures) << 1)) >> 1) ^
               (std::hash<uint64_t>()(k.bufferFeatures) << 1);
    }
};
template <>
struct hash<PropFlags> {
    std::size_t operator()(const PropFlags &k) const {
        return (std::hash<VkFormatProperties>()(k.props) ^ std::hash<VkFormatProperties3>()(k.props3)) >> 1;
    }
};
}  // namespace std
