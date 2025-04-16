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

#include "test_util.h"

#include <fstream>

#if defined(WIN32)
#include <wchar.h>
#include <strsafe.h>
const char* win_api_error_str(LSTATUS status) {
    if (status == ERROR_INVALID_FUNCTION) return "ERROR_INVALID_FUNCTION";
    if (status == ERROR_FILE_NOT_FOUND) return "ERROR_FILE_NOT_FOUND";
    if (status == ERROR_PATH_NOT_FOUND) return "ERROR_PATH_NOT_FOUND";
    if (status == ERROR_TOO_MANY_OPEN_FILES) return "ERROR_TOO_MANY_OPEN_FILES";
    if (status == ERROR_ACCESS_DENIED) return "ERROR_ACCESS_DENIED";
    if (status == ERROR_INVALID_HANDLE) return "ERROR_INVALID_HANDLE";
    if (status == ERROR_ENVVAR_NOT_FOUND) return "ERROR_ENVVAR_NOT_FOUND";
    if (status == ERROR_SETENV_FAILED) return "ERROR_SETENV_FAILED";
    return "UNKNOWN ERROR";
}

void print_error_message(LSTATUS status, const char* function_name, std::string optional_message) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

    std::cerr << function_name << " failed with " << win_api_error_str(status) << ": "
              << std::string(reinterpret_cast<LPTSTR>(lpMsgBuf));
    if (optional_message != "") {
        std::cerr << " | " << optional_message;
    }
    std::cerr << "\n";
    LocalFree(lpMsgBuf);
}

void EnvVarWrapper::set_env_var() {
    BOOL ret = SetEnvironmentVariableW(widen(name).c_str(), widen(cur_value).c_str());
    if (ret == 0) {
        print_error_message(ERROR_SETENV_FAILED, "SetEnvironmentVariableW");
    }
}
void EnvVarWrapper::remove_env_var() const { SetEnvironmentVariableW(widen(name).c_str(), nullptr); }
std::string get_env_var(std::string const& name, bool report_failure) {
    std::wstring name_utf16 = widen(name);
    DWORD value_size = GetEnvironmentVariableW(name_utf16.c_str(), nullptr, 0);
    if (0 == value_size) {
        if (report_failure) print_error_message(ERROR_ENVVAR_NOT_FOUND, "GetEnvironmentVariableW");
        return {};
    }
    std::wstring value(value_size, L'\0');
    if (GetEnvironmentVariableW(name_utf16.c_str(), &value[0], value_size) != value_size - 1) {
        return {};
    }
    return narrow(value);
}
#elif COMMON_UNIX_PLATFORMS

void EnvVarWrapper::set_env_var() { setenv(name.c_str(), cur_value.c_str(), 1); }
void EnvVarWrapper::remove_env_var() const { unsetenv(name.c_str()); }
std::string get_env_var(std::string const& name, bool report_failure) {
    char* ret = getenv(name.c_str());
    if (ret == nullptr) {
        if (report_failure) std::cerr << "Failed to get environment variable:" << name << "\n";
        return std::string();
    }
    return ret;
}
#endif

template <typename T>
void print_object_of_t(JsonWriter& writer, const char* object_name, std::vector<T> const& vec) {
    if (vec.size() == 0) return;
    writer.StartKeyedObject(object_name);
    for (auto& element : vec) {
        element.get_manifest_str(writer);
    }
    writer.EndObject();
}

template <typename T>
void print_array_of_t(JsonWriter& writer, const char* object_name, std::vector<T> const& vec) {
    if (vec.size() == 0) return;
    writer.StartKeyedArray(object_name);
    for (auto& element : vec) {
        element.get_manifest_str(writer);
    }
    writer.EndArray();
}
void print_vector_of_strings(JsonWriter& writer, const char* object_name, std::vector<std::string> const& strings) {
    if (strings.size() == 0) return;
    writer.StartKeyedArray(object_name);
    for (auto const& str : strings) {
        writer.AddString(std::filesystem::path(str).native());
    }
    writer.EndArray();
}
void print_vector_of_strings(JsonWriter& writer, const char* object_name, std::vector<std::filesystem::path> const& paths) {
    if (paths.size() == 0) return;
    writer.StartKeyedArray(object_name);
    for (auto const& path : paths) {
        writer.AddString(path.native());
    }
    writer.EndArray();
}

std::string to_text(bool b) { return b ? std::string("true") : std::string("false"); }

std::string ManifestICD::get_manifest_str() const {
    JsonWriter writer;
    writer.StartObject();
    writer.AddKeyedString("file_format_version", file_format_version.get_version_str());
    writer.StartKeyedObject("ICD");
    writer.AddKeyedString("library_path", lib_path.native());
    writer.AddKeyedString("api_version", version_to_string(api_version));
    writer.AddKeyedBool("is_portability_driver", is_portability_driver);
    if (!library_arch.empty()) writer.AddKeyedString("library_arch", library_arch);
    writer.EndObject();
    writer.EndObject();
    return writer.output;
}

void ManifestLayer::LayerDescription::Extension::get_manifest_str(JsonWriter& writer) const {
    writer.StartObject();
    writer.AddKeyedString("name", name);
    writer.AddKeyedString("spec_version", std::to_string(spec_version));
    writer.AddKeyedString("spec_version", std::to_string(spec_version));
    print_vector_of_strings(writer, "entrypoints", entrypoints);
    writer.EndObject();
}

void ManifestLayer::LayerDescription::get_manifest_str(JsonWriter& writer) const {
    writer.AddKeyedString("name", name);
    writer.AddKeyedString("type", get_type_str(type));
    if (!lib_path.empty()) {
        writer.AddKeyedString("library_path", lib_path.native());
    }
    writer.AddKeyedString("api_version", version_to_string(api_version));
    writer.AddKeyedString("implementation_version", std::to_string(implementation_version));
    writer.AddKeyedString("description", description);
    print_object_of_t(writer, "functions", functions);
    print_array_of_t(writer, "instance_extensions", instance_extensions);
    print_array_of_t(writer, "device_extensions", device_extensions);
    if (!enable_environment.empty()) {
        writer.StartKeyedObject("enable_environment");
        writer.AddKeyedString(enable_environment, "1");
        writer.EndObject();
    }
    if (!disable_environment.empty()) {
        writer.StartKeyedObject("disable_environment");
        writer.AddKeyedString(disable_environment, "1");
        writer.EndObject();
    }
    print_vector_of_strings(writer, "component_layers", component_layers);
    print_vector_of_strings(writer, "blacklisted_layers", blacklisted_layers);
    print_vector_of_strings(writer, "override_paths", override_paths);
    print_vector_of_strings(writer, "app_keys", app_keys);
    print_object_of_t(writer, "pre_instance_functions", pre_instance_functions);
    if (!library_arch.empty()) {
        writer.AddKeyedString("library_arch", library_arch);
    }
}

VkLayerProperties ManifestLayer::LayerDescription::get_layer_properties() const {
    VkLayerProperties properties{};
    copy_string_to_char_array(name, properties.layerName, VK_MAX_EXTENSION_NAME_SIZE);
    copy_string_to_char_array(description, properties.description, VK_MAX_EXTENSION_NAME_SIZE);
    properties.implementationVersion = implementation_version;
    properties.specVersion = api_version;
    return properties;
}

std::string ManifestLayer::get_manifest_str() const {
    JsonWriter writer;
    writer.StartObject();
    writer.AddKeyedString("file_format_version", file_format_version.get_version_str());
    if (layers.size() == 1) {
        writer.StartKeyedObject("layer");
        layers.at(0).get_manifest_str(writer);
        writer.EndObject();
    } else {
        writer.StartKeyedArray("layers");
        for (size_t i = 0; i < layers.size(); i++) {
            writer.StartObject();
            layers.at(i).get_manifest_str(writer);
            writer.EndObject();
        }
        writer.EndArray();
    }
    writer.EndObject();
    return writer.output;
}

const char* get_platform_wsi_extension([[maybe_unused]] const char* api_selection) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    return "VK_KHR_android_surface";
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    return "VK_EXT_directfb_surface";
#elif defined(VK_USE_PLATFORM_FUCHSIA)
    return "VK_FUCHSIA_imagepipe_surface";
#elif defined(VK_USE_PLATFORM_GGP)
    return "VK_GGP_stream_descriptor_surface";
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    return "VK_MVK_ios_surface";
#elif defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (string_eq(api_selection, "VK_USE_PLATFORM_MACOS_MVK")) return "VK_MVK_macos_surface";
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (string_eq(api_selection, "VK_USE_PLATFORM_METAL_EXT")) return "VK_EXT_metal_surface";
    return "VK_EXT_metal_surface";
#endif
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    return "VK_QNX_screen_surface";
#elif defined(VK_USE_PLATFORM_VI_NN)
    return "VK_NN_vi_surface";
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR)
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_XCB_KHR")) return "VK_KHR_xcb_surface";
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_XLIB_KHR")) return "VK_KHR_xlib_surface";
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_WAYLAND_KHR")) return "VK_KHR_wayland_surface";
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    return "VK_KHR_xcb_surface";
#endif
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    return "VK_KHR_win32_surface";
#else
    return "VK_KHR_display";
#endif
}

bool string_eq(const char* a, const char* b) noexcept { return a && b && strcmp(a, b) == 0; }
bool string_eq(const char* a, const char* b, size_t len) noexcept { return a && b && strncmp(a, b, len) == 0; }

InstanceCreateInfo::InstanceCreateInfo() {
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
}

VkInstanceCreateInfo* InstanceCreateInfo::get() noexcept {
    if (fill_in_application_info) {
        application_info.pApplicationName = app_name.c_str();
        application_info.pEngineName = engine_name.c_str();
        application_info.applicationVersion = app_version;
        application_info.engineVersion = engine_version;
        application_info.apiVersion = api_version;
        instance_info.pApplicationInfo = &application_info;
    }
    instance_info.flags = flags;
    instance_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    instance_info.ppEnabledLayerNames = enabled_layers.data();
    instance_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    instance_info.ppEnabledExtensionNames = enabled_extensions.data();
    return &instance_info;
}
InstanceCreateInfo& InstanceCreateInfo::set_api_version(uint32_t major, uint32_t minor, uint32_t patch) {
    this->api_version = VK_MAKE_API_VERSION(0, major, minor, patch);
    return *this;
}
InstanceCreateInfo& InstanceCreateInfo::setup_WSI(const char* api_selection) {
    add_extensions({"VK_KHR_surface", get_platform_wsi_extension(api_selection)});
    return *this;
}

DeviceQueueCreateInfo::DeviceQueueCreateInfo() { queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; }
DeviceQueueCreateInfo::DeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* create_info) {
    queue_create_info = *create_info;
    for (uint32_t i = 0; i < create_info->queueCount; i++) {
        priorities.push_back(create_info->pQueuePriorities[i]);
    }
}

VkDeviceQueueCreateInfo DeviceQueueCreateInfo::get() noexcept {
    queue_create_info.pQueuePriorities = priorities.data();
    queue_create_info.queueCount = 1;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    return queue_create_info;
}

DeviceCreateInfo::DeviceCreateInfo(const VkDeviceCreateInfo* create_info) {
    dev = *create_info;
    for (uint32_t i = 0; i < create_info->enabledExtensionCount; i++) {
        enabled_extensions.push_back(create_info->ppEnabledExtensionNames[i]);
    }
    for (uint32_t i = 0; i < create_info->enabledLayerCount; i++) {
        enabled_layers.push_back(create_info->ppEnabledLayerNames[i]);
    }
    for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++) {
        device_queue_infos.push_back(create_info->pQueueCreateInfos[i]);
    }
}

VkDeviceCreateInfo* DeviceCreateInfo::get() noexcept {
    dev.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
    dev.ppEnabledLayerNames = enabled_layers.data();
    dev.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    dev.ppEnabledExtensionNames = enabled_extensions.data();
    uint32_t index = 0;
    for (auto& queue : queue_info_details) {
        queue.queue_create_info.queueFamilyIndex = index++;
        queue.queue_create_info.queueCount = 1;
        device_queue_infos.push_back(queue.get());
    }

    dev.queueCreateInfoCount = static_cast<uint32_t>(device_queue_infos.size());
    dev.pQueueCreateInfos = device_queue_infos.data();
    return &dev;
}

#if defined(WIN32)
std::string narrow(const std::wstring& utf16) {
    if (utf16.empty()) {
        return {};
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string utf8(size, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, utf16.data(), static_cast<int>(utf16.size()), &utf8[0], size, nullptr, nullptr) != size) {
        return {};
    }
    return utf8;
}

std::wstring widen(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (size <= 0) {
        return {};
    }
    std::wstring utf16(size, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), &utf16[0], size) != size) {
        return {};
    }
    return utf16;
}
#else
std::string narrow(const std::string& utf16) { return utf16; }
std::string widen(const std::string& utf8) { return utf8; }
#endif
