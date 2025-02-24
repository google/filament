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
 * Contains all the utilities needed to make the framework and tests work.
 * Contains:
 * All the standard library includes and main platform specific includes
 * Dll export macro
 * Manifest ICD & Layer structs
 * FolderManager - manages the contents of a folder, cleaning up when needed
 * per-platform library loading - mirrors the vk_loader_platform
 * LibraryWrapper - RAII wrapper for a library
 * DispatchableHandle - RAII wrapper for vulkan dispatchable handle objects
 * ostream overload for VkResult - prettifies googletest output
 * Instance & Device create info helpers
 * operator == overloads for many vulkan structs - more concise tests
 */
#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include <cassert>
#include <cstring>
#include <ctime>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

// Set of platforms with a common set of functionality which is queried throughout the program
#if defined(__linux__) || defined(__APPLE__) || defined(__Fuchsia__) || defined(__QNX__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__GNU__)
#define COMMON_UNIX_PLATFORMS 1
#else
#define COMMON_UNIX_PLATFORMS 0
#endif

#if defined(WIN32)
#include <direct.h>
#include <windows.h>
#include <strsafe.h>
#elif COMMON_UNIX_PLATFORMS
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

// Prevent macro collisions from <sys/types.h>
#undef major
#undef minor

#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <vulkan/vk_layer.h>

#include FRAMEWORK_CONFIG_HEADER

#if defined(__GNUC__) && __GNUC__ >= 4
#define FRAMEWORK_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define FRAMEWORK_EXPORT __attribute__((visibility("default")))
#elif defined(WIN32)
#define FRAMEWORK_EXPORT __declspec(dllexport)
#else
#define FRAMEWORK_EXPORT
#endif

// Define it here so that json_writer.h has access to these functions
#if defined(WIN32)
// Convert an UTF-16 wstring to an UTF-8 string
std::string narrow(const std::wstring& utf16);
// Convert an UTF-8 string to an UTF-16 wstring
std::wstring widen(const std::string& utf8);
#else
// Do nothing passthrough for the sake of Windows & UTF-16
std::string narrow(const std::string& utf16);
// Do nothing passthrough for the sake of Windows & UTF-16
std::string widen(const std::string& utf8);
#endif

#include "json_writer.h"

// get_env_var() - returns a std::string of `name`. if report_failure is true, then it will log to stderr that it didn't find the
//     env-var
// NOTE: This is only intended for test framework code, all test code MUST use EnvVarWrapper
std::string get_env_var(std::string const& name, bool report_failure = true);

/*
 * Wrapper around Environment Variables with common operations
 * Since Environment Variables leak between tests, there needs to be RAII code to remove them during test cleanup

 */

// Wrapper to set & remove env-vars automatically
struct EnvVarWrapper {
    // Constructor which unsets the env-var
    EnvVarWrapper(std::string const& name) noexcept : name(name) {
        initial_value = get_env_var(name, false);
        remove_env_var();
    }
    // Constructor which set the env-var to the specified value
    EnvVarWrapper(std::string const& name, std::string const& value) noexcept : name(name), cur_value(value) {
        initial_value = get_env_var(name, false);
        set_env_var();
    }
    ~EnvVarWrapper() noexcept {
        remove_env_var();
        if (!initial_value.empty()) {
            set_new_value(initial_value);
        }
    }

    // delete copy operators
    EnvVarWrapper(const EnvVarWrapper&) = delete;
    EnvVarWrapper& operator=(const EnvVarWrapper&) = delete;

    void set_new_value(std::string const& value) {
        cur_value = value;
        set_env_var();
    }
    void add_to_list(std::string const& list_item) {
        if (!cur_value.empty()) {
            cur_value += OS_ENV_VAR_LIST_SEPARATOR;
        }
        cur_value += list_item;
        set_env_var();
    }
#if defined(WIN32)
    void add_to_list(std::wstring const& list_item) {
        if (!cur_value.empty()) {
            cur_value += OS_ENV_VAR_LIST_SEPARATOR;
        }
        cur_value += narrow(list_item);
        set_env_var();
    }
#endif
    void remove_value() const { remove_env_var(); }
    const char* get() const { return name.c_str(); }
    const char* value() const { return cur_value.c_str(); }

   private:
    std::string name;
    std::string cur_value;
    std::string initial_value;

    void set_env_var();
    void remove_env_var() const;
#if defined(WIN32)
    // Environment variable list separator - not for filesystem paths
    const char OS_ENV_VAR_LIST_SEPARATOR = ';';
#elif COMMON_UNIX_PLATFORMS
    // Environment variable list separator - not for filesystem paths
    const char OS_ENV_VAR_LIST_SEPARATOR = ':';
#endif
};

// Windows specific error handling logic
#if defined(WIN32)
const long ERROR_SETENV_FAILED = 10543;           // chosen at random, attempts to not conflict
const long ERROR_REMOVEDIRECTORY_FAILED = 10544;  // chosen at random, attempts to not conflict
const char* win_api_error_str(LSTATUS status);
void print_error_message(LSTATUS status, const char* function_name, std::string optional_message = "");
#endif

struct ManifestICD;    // forward declaration for FolderManager::write
struct ManifestLayer;  // forward declaration for FolderManager::write

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

    // Add an already existing file to the manager, so it will be cleaned up automatically
    void add_existing_file(std::filesystem::path const& file_name);

    // close file handle, delete file, remove `name` from managed file list.
    void remove(std::filesystem::path const& name);

    // copy file into this folder with name `new_name`. Returns the full path of the file that was copied
    std::filesystem::path copy_file(std::filesystem::path const& file, std::filesystem::path const& new_name);

    // location of the managed folder
    std::filesystem::path location() const { return folder; }

    std::vector<std::filesystem::path> get_files() const { return files; }

   private:
    std::filesystem::path folder;
    std::vector<std::filesystem::path> files;
};
}  // namespace fs

// copy the contents of a std::string into a char array and add a null terminator at the end
// src - std::string to read from
// dst - char array to write to
// size_dst - number of characters in the dst array
inline void copy_string_to_char_array(std::string const& src, char* dst, size_t size_dst) { dst[src.copy(dst, size_dst - 1)] = 0; }

#if defined(WIN32)
typedef HMODULE test_platform_dl_handle;
inline test_platform_dl_handle test_platform_open_library(const wchar_t* lib_path) {
    // Try loading the library the original way first.
    test_platform_dl_handle lib_handle = LoadLibraryW(lib_path);
    if (lib_handle == nullptr && GetLastError() == ERROR_MOD_NOT_FOUND) {
        // If that failed, then try loading it with broader search folders.
        lib_handle = LoadLibraryExW(lib_path, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    }
    return lib_handle;
}
inline void test_platform_open_library_print_error(std::filesystem::path const& libPath) {
    std::wcerr << L"Unable to open library: " << libPath << L" due to: " << std::to_wstring(GetLastError()) << L"\n";
}
inline void test_platform_close_library(test_platform_dl_handle library) { FreeLibrary(library); }
inline void* test_platform_get_proc_address(test_platform_dl_handle library, const char* name) {
    assert(library);
    assert(name);
    return reinterpret_cast<void*>(GetProcAddress(library, name));
}
inline char* test_platform_get_proc_address_error(const char* name) {
    static char errorMsg[120];
    snprintf(errorMsg, 119, "Failed to find function \"%s\" in dynamic library", name);
    return errorMsg;
}

#elif COMMON_UNIX_PLATFORMS

typedef void* test_platform_dl_handle;
inline test_platform_dl_handle test_platform_open_library(const char* libPath) { return dlopen(libPath, RTLD_LAZY | RTLD_LOCAL); }
inline void test_platform_open_library_print_error(std::filesystem::path const& libPath) {
    std::wcerr << "Unable to open library: " << libPath << " due to: " << dlerror() << "\n";
}
inline void test_platform_close_library(test_platform_dl_handle library) {
    char* loader_disable_dynamic_library_unloading_env_var = getenv("VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING");
    if (NULL == loader_disable_dynamic_library_unloading_env_var ||
        0 != strncmp(loader_disable_dynamic_library_unloading_env_var, "1", 2)) {
        dlclose(library);
    }
}
inline void* test_platform_get_proc_address(test_platform_dl_handle library, const char* name) {
    assert(library);
    assert(name);
    return dlsym(library, name);
}
inline const char* test_platform_get_proc_address_error([[maybe_unused]] const char* name) { return dlerror(); }
#endif

class FromVoidStarFunc {
   private:
    void* function;

   public:
    FromVoidStarFunc(void* function) : function(function) {}
    FromVoidStarFunc(PFN_vkVoidFunction function) : function(reinterpret_cast<void*>(function)) {}

    template <typename T>
    operator T() {
        return reinterpret_cast<T>(function);
    }
};

struct LibraryWrapper {
    explicit LibraryWrapper() noexcept {}
    explicit LibraryWrapper(std::filesystem::path const& lib_path) noexcept : lib_path(lib_path) {
        lib_handle = test_platform_open_library(lib_path.native().c_str());
        if (lib_handle == nullptr) {
            test_platform_open_library_print_error(lib_path);
            assert(lib_handle != nullptr && "Must be able to open library");
        }
    }
    ~LibraryWrapper() noexcept {
        if (lib_handle != nullptr) {
            test_platform_close_library(lib_handle);
            lib_handle = nullptr;
        }
    }
    LibraryWrapper(LibraryWrapper const& wrapper) = delete;
    LibraryWrapper& operator=(LibraryWrapper const& wrapper) = delete;
    LibraryWrapper(LibraryWrapper&& wrapper) noexcept : lib_handle(wrapper.lib_handle), lib_path(wrapper.lib_path) {
        wrapper.lib_handle = nullptr;
    }
    LibraryWrapper& operator=(LibraryWrapper&& wrapper) noexcept {
        if (this != &wrapper) {
            if (lib_handle != nullptr) {
                test_platform_close_library(lib_handle);
            }
            lib_handle = wrapper.lib_handle;
            lib_path = wrapper.lib_path;
            wrapper.lib_handle = nullptr;
        }
        return *this;
    }
    FromVoidStarFunc get_symbol(const char* symbol_name) const {
        assert(lib_handle != nullptr && "Cannot get symbol with null library handle");
        void* symbol = test_platform_get_proc_address(lib_handle, symbol_name);
        if (symbol == nullptr) {
            fprintf(stderr, "Unable to open symbol %s: %s\n", symbol_name, test_platform_get_proc_address_error(symbol_name));
            assert(symbol != nullptr && "Must be able to get symbol");
        }
        return FromVoidStarFunc(symbol);
    }

    explicit operator bool() const noexcept { return lib_handle != nullptr; }

    test_platform_dl_handle lib_handle = nullptr;
    std::filesystem::path lib_path;
};

template <typename T>
PFN_vkVoidFunction to_vkVoidFunction(T func) {
    return reinterpret_cast<PFN_vkVoidFunction>(func);
}
template <typename T>
struct FRAMEWORK_EXPORT DispatchableHandle {
    DispatchableHandle() {
        auto ptr_handle = new VK_LOADER_DATA;
        set_loader_magic_value(ptr_handle);
        handle = reinterpret_cast<T>(ptr_handle);
    }
    ~DispatchableHandle() {
        if (handle) {
            delete reinterpret_cast<VK_LOADER_DATA*>(handle);
        }
        handle = nullptr;
    }
    DispatchableHandle(DispatchableHandle const&) = delete;
    DispatchableHandle& operator=(DispatchableHandle const&) = delete;
    DispatchableHandle(DispatchableHandle&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    DispatchableHandle& operator=(DispatchableHandle&& other) noexcept {
        if (handle) {
            delete reinterpret_cast<VK_LOADER_DATA*>(handle);
        }
        handle = other.handle;
        other.handle = nullptr;
        return *this;
    }
    bool operator==(T base_handle) { return base_handle == handle; }
    bool operator!=(T base_handle) { return base_handle != handle; }

    T handle = nullptr;
};

// Stream operator for VkResult so GTEST will print out error codes as strings (automatically)
inline std::ostream& operator<<(std::ostream& os, const VkResult& result) {
    switch (result) {
        case (VK_SUCCESS):
            return os << "VK_SUCCESS";
        case (VK_NOT_READY):
            return os << "VK_NOT_READY";
        case (VK_TIMEOUT):
            return os << "VK_TIMEOUT";
        case (VK_EVENT_SET):
            return os << "VK_EVENT_SET";
        case (VK_EVENT_RESET):
            return os << "VK_EVENT_RESET";
        case (VK_INCOMPLETE):
            return os << "VK_INCOMPLETE";
        case (VK_ERROR_OUT_OF_HOST_MEMORY):
            return os << "VK_ERROR_OUT_OF_HOST_MEMORY";
        case (VK_ERROR_OUT_OF_DEVICE_MEMORY):
            return os << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case (VK_ERROR_INITIALIZATION_FAILED):
            return os << "VK_ERROR_INITIALIZATION_FAILED";
        case (VK_ERROR_DEVICE_LOST):
            return os << "VK_ERROR_DEVICE_LOST";
        case (VK_ERROR_MEMORY_MAP_FAILED):
            return os << "VK_ERROR_MEMORY_MAP_FAILED";
        case (VK_ERROR_LAYER_NOT_PRESENT):
            return os << "VK_ERROR_LAYER_NOT_PRESENT";
        case (VK_ERROR_EXTENSION_NOT_PRESENT):
            return os << "VK_ERROR_EXTENSION_NOT_PRESENT";
        case (VK_ERROR_FEATURE_NOT_PRESENT):
            return os << "VK_ERROR_FEATURE_NOT_PRESENT";
        case (VK_ERROR_INCOMPATIBLE_DRIVER):
            return os << "VK_ERROR_INCOMPATIBLE_DRIVER";
        case (VK_ERROR_TOO_MANY_OBJECTS):
            return os << "VK_ERROR_TOO_MANY_OBJECTS";
        case (VK_ERROR_FORMAT_NOT_SUPPORTED):
            return os << "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case (VK_ERROR_FRAGMENTED_POOL):
            return os << "VK_ERROR_FRAGMENTED_POOL";
        case (VK_ERROR_UNKNOWN):
            return os << "VK_ERROR_UNKNOWN";
        case (VK_ERROR_OUT_OF_POOL_MEMORY):
            return os << "VK_ERROR_OUT_OF_POOL_MEMORY";
        case (VK_ERROR_INVALID_EXTERNAL_HANDLE):
            return os << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case (VK_ERROR_FRAGMENTATION):
            return os << "VK_ERROR_FRAGMENTATION";
        case (VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS):
            return os << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case (VK_ERROR_SURFACE_LOST_KHR):
            return os << "VK_ERROR_SURFACE_LOST_KHR";
        case (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR):
            return os << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case (VK_SUBOPTIMAL_KHR):
            return os << "VK_SUBOPTIMAL_KHR";
        case (VK_ERROR_OUT_OF_DATE_KHR):
            return os << "VK_ERROR_OUT_OF_DATE_KHR";
        case (VK_ERROR_INCOMPATIBLE_DISPLAY_KHR):
            return os << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case (VK_ERROR_VALIDATION_FAILED_EXT):
            return os << "VK_ERROR_VALIDATION_FAILED_EXT";
        case (VK_ERROR_INVALID_SHADER_NV):
            return os << "VK_ERROR_INVALID_SHADER_NV";
        case (VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT):
            return os << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case (VK_ERROR_NOT_PERMITTED_EXT):
            return os << "VK_ERROR_NOT_PERMITTED_EXT";
        case (VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT):
            return os << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case (VK_THREAD_IDLE_KHR):
            return os << "VK_THREAD_IDLE_KHR";
        case (VK_THREAD_DONE_KHR):
            return os << "VK_THREAD_DONE_KHR";
        case (VK_OPERATION_DEFERRED_KHR):
            return os << "VK_OPERATION_DEFERRED_KHR";
        case (VK_OPERATION_NOT_DEFERRED_KHR):
            return os << "VK_OPERATION_NOT_DEFERRED_KHR";
        case (VK_PIPELINE_COMPILE_REQUIRED_EXT):
            return os << "VK_PIPELINE_COMPILE_REQUIRED_EXT";
        case (VK_RESULT_MAX_ENUM):
            return os << "VK_RESULT_MAX_ENUM";
        case (VK_ERROR_COMPRESSION_EXHAUSTED_EXT):
            return os << "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        case (VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case (VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR):
            return os << "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case (VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR):
            return os << "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case (VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT):
            return os << "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
        case (VK_PIPELINE_BINARY_MISSING_KHR):
            return os << "VK_PIPELINE_BINARY_MISSING_KHR";
        case (VK_ERROR_NOT_ENOUGH_SPACE_KHR):
            return os << "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
    }
    return os << static_cast<int32_t>(result);
}

const char* get_platform_wsi_extension([[maybe_unused]] const char* api_selection);

bool string_eq(const char* a, const char* b) noexcept;
bool string_eq(const char* a, const char* b, size_t len) noexcept;

inline std::string version_to_string(uint32_t version) {
    std::string out = std::to_string(VK_API_VERSION_MAJOR(version)) + "." + std::to_string(VK_API_VERSION_MINOR(version)) + "." +
                      std::to_string(VK_API_VERSION_PATCH(version));
    if (VK_API_VERSION_VARIANT(version) != 0) out += std::to_string(VK_API_VERSION_VARIANT(version)) + "." + out;
    return out;
}

// Macro to ease the definition of variables with builder member functions
// class_name = class the member variable is apart of
// type = type of the variable
// name = name of the variable
// default_value = value to default initialize, use {} if nothing else makes sense
#define BUILDER_VALUE(class_name, type, name, default_value) \
    type name = default_value;                               \
    class_name& set_##name(type const& name) {               \
        this->name = name;                                   \
        return *this;                                        \
    }

// Macro to ease the definition of vectors with builder member functions
// class_name = class the member variable is apart of
// type = type of the variable
// name = name of the variable
// singular_name = used for the `add_singular_name` member function
#define BUILDER_VECTOR(class_name, type, name, singular_name)                    \
    std::vector<type> name;                                                      \
    class_name& add_##singular_name(type const& singular_name) {                 \
        this->name.push_back(singular_name);                                     \
        return *this;                                                            \
    }                                                                            \
    class_name& add_##singular_name##s(std::vector<type> const& singular_name) { \
        for (auto& elem : singular_name) this->name.push_back(elem);             \
        return *this;                                                            \
    }
// Like BUILDER_VECTOR but for move only types - where passing in means giving up ownership
#define BUILDER_VECTOR_MOVE_ONLY(class_name, type, name, singular_name) \
    std::vector<type> name;                                             \
    class_name& add_##singular_name(type&& singular_name) {             \
        this->name.push_back(std::move(singular_name));                 \
        return *this;                                                   \
    }

struct ManifestVersion {
    BUILDER_VALUE(ManifestVersion, uint32_t, major, 1)
    BUILDER_VALUE(ManifestVersion, uint32_t, minor, 0)
    BUILDER_VALUE(ManifestVersion, uint32_t, patch, 0)

    std::string get_version_str() const noexcept {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

// ManifestICD builder
struct ManifestICD {
    BUILDER_VALUE(ManifestICD, ManifestVersion, file_format_version, {})
    BUILDER_VALUE(ManifestICD, uint32_t, api_version, 0)
    BUILDER_VALUE(ManifestICD, std::filesystem::path, lib_path, {})
    BUILDER_VALUE(ManifestICD, bool, is_portability_driver, false)
    BUILDER_VALUE(ManifestICD, std::string, library_arch, "")
    std::string get_manifest_str() const;
};

// ManifestLayer builder
struct ManifestLayer {
    struct LayerDescription {
        enum class Type { INSTANCE, GLOBAL, DEVICE };
        std::string get_type_str(Type layer_type) const {
            if (layer_type == Type::GLOBAL)
                return "GLOBAL";
            else if (layer_type == Type::DEVICE)
                return "DEVICE";
            else  // default
                return "INSTANCE";
        }
        struct FunctionOverride {
            BUILDER_VALUE(FunctionOverride, std::string, vk_func, {})
            BUILDER_VALUE(FunctionOverride, std::string, override_name, {})

            void get_manifest_str(JsonWriter& writer) const { writer.AddKeyedString(vk_func, override_name); }
        };
        struct Extension {
            Extension() noexcept {}
            Extension(std::string name, uint32_t spec_version = 0, std::vector<std::string> entrypoints = {}) noexcept
                : name(name), spec_version(spec_version), entrypoints(entrypoints) {}
            std::string name;
            uint32_t spec_version = 0;
            std::vector<std::string> entrypoints;
            void get_manifest_str(JsonWriter& writer) const;
        };
        BUILDER_VALUE(LayerDescription, std::string, name, {})
        BUILDER_VALUE(LayerDescription, Type, type, Type::INSTANCE)
        BUILDER_VALUE(LayerDescription, std::filesystem::path, lib_path, {})
        BUILDER_VALUE(LayerDescription, uint32_t, api_version, VK_API_VERSION_1_0)
        BUILDER_VALUE(LayerDescription, uint32_t, implementation_version, 0)
        BUILDER_VALUE(LayerDescription, std::string, description, {})
        BUILDER_VECTOR(LayerDescription, FunctionOverride, functions, function)
        BUILDER_VECTOR(LayerDescription, Extension, instance_extensions, instance_extension)
        BUILDER_VECTOR(LayerDescription, Extension, device_extensions, device_extension)
        BUILDER_VALUE(LayerDescription, std::string, enable_environment, {})
        BUILDER_VALUE(LayerDescription, std::string, disable_environment, {})
        BUILDER_VECTOR(LayerDescription, std::string, component_layers, component_layer)
        BUILDER_VECTOR(LayerDescription, std::string, blacklisted_layers, blacklisted_layer)
        BUILDER_VECTOR(LayerDescription, std::filesystem::path, override_paths, override_path)
        BUILDER_VECTOR(LayerDescription, FunctionOverride, pre_instance_functions, pre_instance_function)
        BUILDER_VECTOR(LayerDescription, std::string, app_keys, app_key)
        BUILDER_VALUE(LayerDescription, std::string, library_arch, "")

        void get_manifest_str(JsonWriter& writer) const;
        VkLayerProperties get_layer_properties() const;
    };
    BUILDER_VALUE(ManifestLayer, ManifestVersion, file_format_version, {})
    BUILDER_VECTOR(ManifestLayer, LayerDescription, layers, layer)

    std::string get_manifest_str() const;
};

struct Extension {
    BUILDER_VALUE(Extension, std::string, extensionName, {})
    BUILDER_VALUE(Extension, uint32_t, specVersion, VK_API_VERSION_1_0)

    Extension(const char* name, uint32_t specVersion = VK_API_VERSION_1_0) noexcept
        : extensionName(name), specVersion(specVersion) {}
    Extension(std::string extensionName, uint32_t specVersion = VK_API_VERSION_1_0) noexcept
        : extensionName(extensionName), specVersion(specVersion) {}

    VkExtensionProperties get() const noexcept {
        VkExtensionProperties props{};
        copy_string_to_char_array(extensionName, &props.extensionName[0], VK_MAX_EXTENSION_NAME_SIZE);
        props.specVersion = specVersion;
        return props;
    }
};

struct MockQueueFamilyProperties {
    BUILDER_VALUE(MockQueueFamilyProperties, VkQueueFamilyProperties, properties, {})
    BUILDER_VALUE(MockQueueFamilyProperties, bool, support_present, false)

    VkQueueFamilyProperties get() const noexcept { return properties; }
};

struct InstanceCreateInfo {
    BUILDER_VALUE(InstanceCreateInfo, VkInstanceCreateInfo, instance_info, {})
    BUILDER_VALUE(InstanceCreateInfo, VkApplicationInfo, application_info, {})
    BUILDER_VALUE(InstanceCreateInfo, std::string, app_name, {})
    BUILDER_VALUE(InstanceCreateInfo, std::string, engine_name, {})
    BUILDER_VALUE(InstanceCreateInfo, uint32_t, flags, 0)
    BUILDER_VALUE(InstanceCreateInfo, uint32_t, app_version, 0)
    BUILDER_VALUE(InstanceCreateInfo, uint32_t, engine_version, 0)
    BUILDER_VALUE(InstanceCreateInfo, uint32_t, api_version, VK_API_VERSION_1_0)
    BUILDER_VECTOR(InstanceCreateInfo, const char*, enabled_layers, layer)
    BUILDER_VECTOR(InstanceCreateInfo, const char*, enabled_extensions, extension)
    // tell the get() function to not provide `application_info`
    BUILDER_VALUE(InstanceCreateInfo, bool, fill_in_application_info, true)

    InstanceCreateInfo();

    VkInstanceCreateInfo* get() noexcept;

    InstanceCreateInfo& set_api_version(uint32_t major, uint32_t minor, uint32_t patch);

    InstanceCreateInfo& setup_WSI(const char* api_selection = nullptr);
};

struct DeviceQueueCreateInfo {
    DeviceQueueCreateInfo();
    DeviceQueueCreateInfo(const VkDeviceQueueCreateInfo* create_info);

    BUILDER_VALUE(DeviceQueueCreateInfo, VkDeviceQueueCreateInfo, queue_create_info, {})
    BUILDER_VECTOR(DeviceQueueCreateInfo, float, priorities, priority)

    VkDeviceQueueCreateInfo get() noexcept;
};

struct DeviceCreateInfo {
    DeviceCreateInfo() = default;
    DeviceCreateInfo(const VkDeviceCreateInfo* create_info);

    BUILDER_VALUE(DeviceCreateInfo, VkDeviceCreateInfo, dev, {})
    BUILDER_VECTOR(DeviceCreateInfo, const char*, enabled_extensions, extension)
    BUILDER_VECTOR(DeviceCreateInfo, const char*, enabled_layers, layer)
    BUILDER_VECTOR(DeviceCreateInfo, DeviceQueueCreateInfo, queue_info_details, device_queue)

    VkDeviceCreateInfo* get() noexcept;

   private:
    std::vector<VkDeviceQueueCreateInfo> device_queue_infos;
};

inline bool operator==(const VkExtent3D& a, const VkExtent3D& b) {
    return a.width == b.width && a.height == b.height && a.depth == b.depth;
}
inline bool operator!=(const VkExtent3D& a, const VkExtent3D& b) { return !(a == b); }

inline bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b) {
    return a.minImageTransferGranularity == b.minImageTransferGranularity && a.queueCount == b.queueCount &&
           a.queueFlags == b.queueFlags && a.timestampValidBits == b.timestampValidBits;
}
inline bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties& b) { return !(a == b); }

inline bool operator==(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b) { return a == b.queueFamilyProperties; }
inline bool operator!=(const VkQueueFamilyProperties& a, const VkQueueFamilyProperties2& b) { return a != b.queueFamilyProperties; }

inline bool operator==(const VkLayerProperties& a, const VkLayerProperties& b) {
    return string_eq(a.layerName, b.layerName, 256) && string_eq(a.description, b.description, 256) &&
           a.implementationVersion == b.implementationVersion && a.specVersion == b.specVersion;
}
inline bool operator!=(const VkLayerProperties& a, const VkLayerProperties& b) { return !(a == b); }

inline bool operator==(const VkExtensionProperties& a, const VkExtensionProperties& b) {
    return string_eq(a.extensionName, b.extensionName, 256) && a.specVersion == b.specVersion;
}
inline bool operator!=(const VkExtensionProperties& a, const VkExtensionProperties& b) { return !(a == b); }

inline bool operator==(const VkPhysicalDeviceFeatures& feats1, const VkPhysicalDeviceFeatures2& feats2) {
    return feats1.robustBufferAccess == feats2.features.robustBufferAccess &&
           feats1.fullDrawIndexUint32 == feats2.features.fullDrawIndexUint32 &&
           feats1.imageCubeArray == feats2.features.imageCubeArray && feats1.independentBlend == feats2.features.independentBlend &&
           feats1.geometryShader == feats2.features.geometryShader &&
           feats1.tessellationShader == feats2.features.tessellationShader &&
           feats1.sampleRateShading == feats2.features.sampleRateShading && feats1.dualSrcBlend == feats2.features.dualSrcBlend &&
           feats1.logicOp == feats2.features.logicOp && feats1.multiDrawIndirect == feats2.features.multiDrawIndirect &&
           feats1.drawIndirectFirstInstance == feats2.features.drawIndirectFirstInstance &&
           feats1.depthClamp == feats2.features.depthClamp && feats1.depthBiasClamp == feats2.features.depthBiasClamp &&
           feats1.fillModeNonSolid == feats2.features.fillModeNonSolid && feats1.depthBounds == feats2.features.depthBounds &&
           feats1.wideLines == feats2.features.wideLines && feats1.largePoints == feats2.features.largePoints &&
           feats1.alphaToOne == feats2.features.alphaToOne && feats1.multiViewport == feats2.features.multiViewport &&
           feats1.samplerAnisotropy == feats2.features.samplerAnisotropy &&
           feats1.textureCompressionETC2 == feats2.features.textureCompressionETC2 &&
           feats1.textureCompressionASTC_LDR == feats2.features.textureCompressionASTC_LDR &&
           feats1.textureCompressionBC == feats2.features.textureCompressionBC &&
           feats1.occlusionQueryPrecise == feats2.features.occlusionQueryPrecise &&
           feats1.pipelineStatisticsQuery == feats2.features.pipelineStatisticsQuery &&
           feats1.vertexPipelineStoresAndAtomics == feats2.features.vertexPipelineStoresAndAtomics &&
           feats1.fragmentStoresAndAtomics == feats2.features.fragmentStoresAndAtomics &&
           feats1.shaderTessellationAndGeometryPointSize == feats2.features.shaderTessellationAndGeometryPointSize &&
           feats1.shaderImageGatherExtended == feats2.features.shaderImageGatherExtended &&
           feats1.shaderStorageImageExtendedFormats == feats2.features.shaderStorageImageExtendedFormats &&
           feats1.shaderStorageImageMultisample == feats2.features.shaderStorageImageMultisample &&
           feats1.shaderStorageImageReadWithoutFormat == feats2.features.shaderStorageImageReadWithoutFormat &&
           feats1.shaderStorageImageWriteWithoutFormat == feats2.features.shaderStorageImageWriteWithoutFormat &&
           feats1.shaderUniformBufferArrayDynamicIndexing == feats2.features.shaderUniformBufferArrayDynamicIndexing &&
           feats1.shaderSampledImageArrayDynamicIndexing == feats2.features.shaderSampledImageArrayDynamicIndexing &&
           feats1.shaderStorageBufferArrayDynamicIndexing == feats2.features.shaderStorageBufferArrayDynamicIndexing &&
           feats1.shaderStorageImageArrayDynamicIndexing == feats2.features.shaderStorageImageArrayDynamicIndexing &&
           feats1.shaderClipDistance == feats2.features.shaderClipDistance &&
           feats1.shaderCullDistance == feats2.features.shaderCullDistance &&
           feats1.shaderFloat64 == feats2.features.shaderFloat64 && feats1.shaderInt64 == feats2.features.shaderInt64 &&
           feats1.shaderInt16 == feats2.features.shaderInt16 &&
           feats1.shaderResourceResidency == feats2.features.shaderResourceResidency &&
           feats1.shaderResourceMinLod == feats2.features.shaderResourceMinLod &&
           feats1.sparseBinding == feats2.features.sparseBinding &&
           feats1.sparseResidencyBuffer == feats2.features.sparseResidencyBuffer &&
           feats1.sparseResidencyImage2D == feats2.features.sparseResidencyImage2D &&
           feats1.sparseResidencyImage3D == feats2.features.sparseResidencyImage3D &&
           feats1.sparseResidency2Samples == feats2.features.sparseResidency2Samples &&
           feats1.sparseResidency4Samples == feats2.features.sparseResidency4Samples &&
           feats1.sparseResidency8Samples == feats2.features.sparseResidency8Samples &&
           feats1.sparseResidency16Samples == feats2.features.sparseResidency16Samples &&
           feats1.sparseResidencyAliased == feats2.features.sparseResidencyAliased &&
           feats1.variableMultisampleRate == feats2.features.variableMultisampleRate &&
           feats1.inheritedQueries == feats2.features.inheritedQueries;
}

inline bool operator==(const VkPhysicalDeviceMemoryProperties& props1, const VkPhysicalDeviceMemoryProperties2& props2) {
    bool equal = true;
    equal = equal && props1.memoryTypeCount == props2.memoryProperties.memoryTypeCount;
    equal = equal && props1.memoryHeapCount == props2.memoryProperties.memoryHeapCount;
    for (uint32_t i = 0; i < props1.memoryHeapCount; ++i) {
        equal = equal && props1.memoryHeaps[i].size == props2.memoryProperties.memoryHeaps[i].size;
        equal = equal && props1.memoryHeaps[i].flags == props2.memoryProperties.memoryHeaps[i].flags;
    }
    for (uint32_t i = 0; i < props1.memoryTypeCount; ++i) {
        equal = equal && props1.memoryTypes[i].propertyFlags == props2.memoryProperties.memoryTypes[i].propertyFlags;
        equal = equal && props1.memoryTypes[i].heapIndex == props2.memoryProperties.memoryTypes[i].heapIndex;
    }
    return equal;
}
inline bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties& props2) {
    return props1.aspectMask == props2.aspectMask && props1.imageGranularity.width == props2.imageGranularity.width &&
           props1.imageGranularity.height == props2.imageGranularity.height &&
           props1.imageGranularity.depth == props2.imageGranularity.depth && props1.flags == props2.flags;
}
inline bool operator==(const VkSparseImageFormatProperties& props1, const VkSparseImageFormatProperties2& props2) {
    return props1 == props2.properties;
}
inline bool operator==(const VkExternalMemoryProperties& props1, const VkExternalMemoryProperties& props2) {
    return props1.externalMemoryFeatures == props2.externalMemoryFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
inline bool operator==(const VkExternalSemaphoreProperties& props1, const VkExternalSemaphoreProperties& props2) {
    return props1.externalSemaphoreFeatures == props2.externalSemaphoreFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
inline bool operator==(const VkExternalFenceProperties& props1, const VkExternalFenceProperties& props2) {
    return props1.externalFenceFeatures == props2.externalFenceFeatures &&
           props1.exportFromImportedHandleTypes == props2.exportFromImportedHandleTypes &&
           props1.compatibleHandleTypes == props2.compatibleHandleTypes;
}
inline bool operator==(const VkSurfaceCapabilitiesKHR& props1, const VkSurfaceCapabilitiesKHR& props2) {
    return props1.minImageCount == props2.minImageCount && props1.maxImageCount == props2.maxImageCount &&
           props1.currentExtent.width == props2.currentExtent.width && props1.currentExtent.height == props2.currentExtent.height &&
           props1.minImageExtent.width == props2.minImageExtent.width &&
           props1.minImageExtent.height == props2.minImageExtent.height &&
           props1.maxImageExtent.width == props2.maxImageExtent.width &&
           props1.maxImageExtent.height == props2.maxImageExtent.height &&
           props1.maxImageArrayLayers == props2.maxImageArrayLayers && props1.supportedTransforms == props2.supportedTransforms &&
           props1.currentTransform == props2.currentTransform && props1.supportedCompositeAlpha == props2.supportedCompositeAlpha &&
           props1.supportedUsageFlags == props2.supportedUsageFlags;
}
inline bool operator==(const VkSurfacePresentScalingCapabilitiesEXT& caps1, const VkSurfacePresentScalingCapabilitiesEXT& caps2) {
    return caps1.supportedPresentScaling == caps2.supportedPresentScaling &&
           caps1.supportedPresentGravityX == caps2.supportedPresentGravityX &&
           caps1.supportedPresentGravityY == caps2.supportedPresentGravityY &&
           caps1.minScaledImageExtent.width == caps2.minScaledImageExtent.width &&
           caps1.minScaledImageExtent.height == caps2.minScaledImageExtent.height &&
           caps1.maxScaledImageExtent.width == caps2.maxScaledImageExtent.width &&
           caps1.maxScaledImageExtent.height == caps2.maxScaledImageExtent.height;
}
inline bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormatKHR& format2) {
    return format1.format == format2.format && format1.colorSpace == format2.colorSpace;
}
inline bool operator==(const VkSurfaceFormatKHR& format1, const VkSurfaceFormat2KHR& format2) {
    return format1 == format2.surfaceFormat;
}
inline bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayPropertiesKHR& props2) {
    return props1.display == props2.display && props1.physicalDimensions.width == props2.physicalDimensions.width &&
           props1.physicalDimensions.height == props2.physicalDimensions.height &&
           props1.physicalResolution.width == props2.physicalResolution.width &&
           props1.physicalResolution.height == props2.physicalResolution.height &&
           props1.supportedTransforms == props2.supportedTransforms && props1.planeReorderPossible == props2.planeReorderPossible &&
           props1.persistentContent == props2.persistentContent;
}
inline bool operator==(const VkDisplayPropertiesKHR& props1, const VkDisplayProperties2KHR& props2) {
    return props1 == props2.displayProperties;
}
inline bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModePropertiesKHR& disp2) {
    return disp1.displayMode == disp2.displayMode && disp1.parameters.visibleRegion.width == disp2.parameters.visibleRegion.width &&
           disp1.parameters.visibleRegion.height == disp2.parameters.visibleRegion.height &&
           disp1.parameters.refreshRate == disp2.parameters.refreshRate;
}

inline bool operator==(const VkDisplayModePropertiesKHR& disp1, const VkDisplayModeProperties2KHR& disp2) {
    return disp1 == disp2.displayModeProperties;
}
inline bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilitiesKHR& caps2) {
    return caps1.supportedAlpha == caps2.supportedAlpha && caps1.minSrcPosition.x == caps2.minSrcPosition.x &&
           caps1.minSrcPosition.y == caps2.minSrcPosition.y && caps1.maxSrcPosition.x == caps2.maxSrcPosition.x &&
           caps1.maxSrcPosition.y == caps2.maxSrcPosition.y && caps1.minSrcExtent.width == caps2.minSrcExtent.width &&
           caps1.minSrcExtent.height == caps2.minSrcExtent.height && caps1.maxSrcExtent.width == caps2.maxSrcExtent.width &&
           caps1.maxSrcExtent.height == caps2.maxSrcExtent.height && caps1.minDstPosition.x == caps2.minDstPosition.x &&
           caps1.minDstPosition.y == caps2.minDstPosition.y && caps1.maxDstPosition.x == caps2.maxDstPosition.x &&
           caps1.maxDstPosition.y == caps2.maxDstPosition.y && caps1.minDstExtent.width == caps2.minDstExtent.width &&
           caps1.minDstExtent.height == caps2.minDstExtent.height && caps1.maxDstExtent.width == caps2.maxDstExtent.width &&
           caps1.maxDstExtent.height == caps2.maxDstExtent.height;
}

inline bool operator==(const VkDisplayPlaneCapabilitiesKHR& caps1, const VkDisplayPlaneCapabilities2KHR& caps2) {
    return caps1 == caps2.capabilities;
}
inline bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlanePropertiesKHR& props2) {
    return props1.currentDisplay == props2.currentDisplay && props1.currentStackIndex == props2.currentStackIndex;
}
inline bool operator==(const VkDisplayPlanePropertiesKHR& props1, const VkDisplayPlaneProperties2KHR& props2) {
    return props1 == props2.displayPlaneProperties;
}
inline bool operator==(const VkExtent2D& ext1, const VkExtent2D& ext2) {
    return ext1.height == ext2.height && ext1.width == ext2.width;
}
// Allow comparison of vectors of different types as long as their elements are comparable (just has to make sure to only apply when
// T != U)
template <typename T, typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
bool operator==(const std::vector<T>& a, const std::vector<U>& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](const auto& left, const auto& right) { return left == right; });
}

struct VulkanFunction {
    std::string name;
    PFN_vkVoidFunction function = nullptr;
};

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

inline bool contains(std::vector<VkExtensionProperties> const& vec, const char* name) {
    return std::any_of(std::begin(vec), std::end(vec),
                       [name](VkExtensionProperties const& elem) { return string_eq(name, elem.extensionName); });
}
inline bool contains(std::vector<VkLayerProperties> const& vec, const char* name) {
    return std::any_of(std::begin(vec), std::end(vec),
                       [name](VkLayerProperties const& elem) { return string_eq(name, elem.layerName); });
}

#if defined(__linux__) || defined(__GNU__)

// find application path + name. Path cannot be longer than 1024, returns NULL if it is greater than that.
inline std::string test_platform_executable_path() {
    std::string buffer;
    buffer.resize(1024);
    ssize_t count = readlink("/proc/self/exe", &buffer[0], buffer.size());
    if (count == -1) return NULL;
    if (count == 0) return NULL;
    buffer[count] = '\0';
    buffer.resize(count);
    return buffer;
}
#elif defined(__APPLE__)
#include <libproc.h>
inline std::string test_platform_executable_path() {
    std::string buffer;
    buffer.resize(1024);
    pid_t pid = getpid();
    int ret = proc_pidpath(pid, &buffer[0], static_cast<uint32_t>(buffer.size()));
    if (ret <= 0) return NULL;
    buffer[ret] = '\0';
    buffer.resize(ret);
    return buffer;
}
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/sysctl.h>
inline std::string test_platform_executable_path() {
    int mib[] = {
        CTL_KERN,
#if defined(__NetBSD__)
        KERN_PROC_ARGS,
        -1,
        KERN_PROC_PATHNAME,
#else
        KERN_PROC,
        KERN_PROC_PATHNAME,
        -1,
#endif
    };
    std::string buffer;
    buffer.resize(1024);
    size_t size = buffer.size();
    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &buffer[0], &size, NULL, 0) < 0) {
        return NULL;
    }
    buffer.resize(size);

    return buffer;
}
#elif defined(__Fuchsia__) || defined(__OpenBSD__)
inline std::string test_platform_executable_path() { return {}; }
#elif defined(__QNX__)

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

#include <fcntl.h>
#include <sys/stat.h>

inline std::string test_platform_executable_path() {
    std::string buffer;
    buffer.resize(1024);
    int fd = open("/proc/self/exefile", O_RDONLY);
    ssize_t rdsize;

    if (fd == -1) {
        return NULL;
    }

    rdsize = read(fd, &buffer[0], buffer.size());
    if (rdsize < 0) {
        return NULL;
    }
    buffer[rdsize] = 0x00;
    close(fd);
    buffer.resize(rdsize);

    return buffer;
}
#endif  // defined (__QNX__)
#if defined(WIN32)
inline std::string test_platform_executable_path() {
    std::string buffer;
    buffer.resize(1024);
    DWORD ret = GetModuleFileName(NULL, static_cast<LPSTR>(&buffer[0]), (DWORD)buffer.size());
    if (ret == 0) return NULL;
    if (ret > buffer.size()) return NULL;
    buffer.resize(ret);
    buffer[ret] = '\0';
    return narrow(std::filesystem::path(buffer).native());
}

#endif
