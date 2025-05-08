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

#include <fstream>

std::filesystem::path get_loader_path() {
    auto loader_path = std::filesystem::path(FRAMEWORK_VULKAN_LIBRARY_PATH);
    auto env_var_res = get_env_var("VK_LOADER_TEST_LOADER_PATH", false);
    if (!env_var_res.empty()) {
        loader_path = std::filesystem::path(env_var_res);
    }
    return loader_path;
}

void init_vulkan_functions(VulkanFunctions& funcs) {
#if defined(APPLE_STATIC_LOADER)
#define GPA(name) name
#else
#define GPA(name) funcs.loader.get_symbol(#name)
#endif

    // clang-format off
    funcs.vkGetInstanceProcAddr = GPA(vkGetInstanceProcAddr);
    funcs.vkEnumerateInstanceExtensionProperties = GPA(vkEnumerateInstanceExtensionProperties);
    funcs.vkEnumerateInstanceLayerProperties = GPA(vkEnumerateInstanceLayerProperties);
    funcs.vkEnumerateInstanceVersion = GPA(vkEnumerateInstanceVersion);
    funcs.vkCreateInstance = GPA(vkCreateInstance);
    funcs.vkDestroyInstance = GPA(vkDestroyInstance);
    funcs.vkEnumeratePhysicalDevices = GPA(vkEnumeratePhysicalDevices);
    funcs.vkEnumeratePhysicalDeviceGroups = GPA(vkEnumeratePhysicalDeviceGroups);
    funcs.vkGetPhysicalDeviceFeatures = GPA(vkGetPhysicalDeviceFeatures);
    funcs.vkGetPhysicalDeviceFeatures2 = GPA(vkGetPhysicalDeviceFeatures2);
    funcs.vkGetPhysicalDeviceFormatProperties = GPA(vkGetPhysicalDeviceFormatProperties);
    funcs.vkGetPhysicalDeviceFormatProperties2 = GPA(vkGetPhysicalDeviceFormatProperties2);
    funcs.vkGetPhysicalDeviceImageFormatProperties = GPA(vkGetPhysicalDeviceImageFormatProperties);
    funcs.vkGetPhysicalDeviceImageFormatProperties2 = GPA(vkGetPhysicalDeviceImageFormatProperties2);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties = GPA(vkGetPhysicalDeviceSparseImageFormatProperties);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2 = GPA(vkGetPhysicalDeviceSparseImageFormatProperties2);
    funcs.vkGetPhysicalDeviceProperties = GPA(vkGetPhysicalDeviceProperties);
    funcs.vkGetPhysicalDeviceProperties2 = GPA(vkGetPhysicalDeviceProperties2);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties = GPA(vkGetPhysicalDeviceQueueFamilyProperties);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2 = GPA(vkGetPhysicalDeviceQueueFamilyProperties2);
    funcs.vkGetPhysicalDeviceMemoryProperties = GPA(vkGetPhysicalDeviceMemoryProperties);
    funcs.vkGetPhysicalDeviceMemoryProperties2 = GPA(vkGetPhysicalDeviceMemoryProperties2);
    funcs.vkGetPhysicalDeviceSurfaceSupportKHR = GPA(vkGetPhysicalDeviceSurfaceSupportKHR);
    funcs.vkGetPhysicalDeviceSurfaceFormatsKHR = GPA(vkGetPhysicalDeviceSurfaceFormatsKHR);
    funcs.vkGetPhysicalDeviceSurfacePresentModesKHR = GPA(vkGetPhysicalDeviceSurfacePresentModesKHR);
    funcs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = GPA(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    funcs.vkEnumerateDeviceExtensionProperties = GPA(vkEnumerateDeviceExtensionProperties);
    funcs.vkEnumerateDeviceLayerProperties = GPA(vkEnumerateDeviceLayerProperties);
    funcs.vkGetPhysicalDeviceExternalBufferProperties = GPA(vkGetPhysicalDeviceExternalBufferProperties);
    funcs.vkGetPhysicalDeviceExternalFenceProperties = GPA(vkGetPhysicalDeviceExternalFenceProperties);
    funcs.vkGetPhysicalDeviceExternalSemaphoreProperties = GPA(vkGetPhysicalDeviceExternalSemaphoreProperties);

    funcs.vkDestroySurfaceKHR = GPA(vkDestroySurfaceKHR);
    funcs.vkGetDeviceProcAddr = GPA(vkGetDeviceProcAddr);
    funcs.vkCreateDevice = GPA(vkCreateDevice);

    funcs.vkCreateHeadlessSurfaceEXT = GPA(vkCreateHeadlessSurfaceEXT);
    funcs.vkCreateDisplayPlaneSurfaceKHR = GPA(vkCreateDisplayPlaneSurfaceKHR);
    funcs.vkGetPhysicalDeviceDisplayPropertiesKHR = GPA(vkGetPhysicalDeviceDisplayPropertiesKHR);
    funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR = GPA(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
    funcs.vkGetDisplayPlaneSupportedDisplaysKHR = GPA(vkGetDisplayPlaneSupportedDisplaysKHR);
    funcs.vkGetDisplayModePropertiesKHR = GPA(vkGetDisplayModePropertiesKHR);
    funcs.vkCreateDisplayModeKHR = GPA(vkCreateDisplayModeKHR);
    funcs.vkGetDisplayPlaneCapabilitiesKHR = GPA(vkGetDisplayPlaneCapabilitiesKHR);
    funcs.vkGetPhysicalDevicePresentRectanglesKHR = GPA(vkGetPhysicalDevicePresentRectanglesKHR);
    funcs.vkGetPhysicalDeviceDisplayProperties2KHR = GPA(vkGetPhysicalDeviceDisplayProperties2KHR);
    funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR = GPA(vkGetPhysicalDeviceDisplayPlaneProperties2KHR);
    funcs.vkGetDisplayModeProperties2KHR = GPA(vkGetDisplayModeProperties2KHR);
    funcs.vkGetDisplayPlaneCapabilities2KHR = GPA(vkGetDisplayPlaneCapabilities2KHR);
    funcs.vkGetPhysicalDeviceSurfaceCapabilities2KHR = GPA(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
    funcs.vkGetPhysicalDeviceSurfaceFormats2KHR = GPA(vkGetPhysicalDeviceSurfaceFormats2KHR);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    funcs.vkCreateAndroidSurfaceKHR = GPA(vkCreateAndroidSurfaceKHR);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    funcs.vkCreateDirectFBSurfaceEXT = GPA(vkCreateDirectFBSurfaceEXT);
    funcs.vkGetPhysicalDeviceDirectFBPresentationSupportEXT = GPA(vkGetPhysicalDeviceDirectFBPresentationSupportEXT);
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_FUCHSIA)
    funcs.vkCreateImagePipeSurfaceFUCHSIA = GPA(vkCreateImagePipeSurfaceFUCHSIA);
#endif  // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_GGP)
    funcs.vkCreateStreamDescriptorSurfaceGGP = GPA(vkCreateStreamDescriptorSurfaceGGP);
#endif  // VK_USE_PLATFORM_GGP
#if defined(VK_USE_PLATFORM_IOS_MVK)
    funcs.vkCreateIOSSurfaceMVK = GPA(vkCreateIOSSurfaceMVK);
#endif  // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    funcs.vkCreateMacOSSurfaceMVK = GPA(vkCreateMacOSSurfaceMVK);
#endif  // VK_USE_PLATFORM_MACOS_MVK
#if defined(VK_USE_PLATFORM_METAL_EXT)
    funcs.vkCreateMetalSurfaceEXT = GPA(vkCreateMetalSurfaceEXT);
#endif  // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    funcs.vkCreateScreenSurfaceQNX = GPA(vkCreateScreenSurfaceQNX);
    funcs.vkGetPhysicalDeviceScreenPresentationSupportQNX = GPA(vkGetPhysicalDeviceScreenPresentationSupportQNX);
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    funcs.vkCreateWaylandSurfaceKHR = GPA(vkCreateWaylandSurfaceKHR);
    funcs.vkGetPhysicalDeviceWaylandPresentationSupportKHR = GPA(vkGetPhysicalDeviceWaylandPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    funcs.vkCreateXcbSurfaceKHR = GPA(vkCreateXcbSurfaceKHR);
    funcs.vkGetPhysicalDeviceXcbPresentationSupportKHR = GPA(vkGetPhysicalDeviceXcbPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    funcs.vkCreateXlibSurfaceKHR = GPA(vkCreateXlibSurfaceKHR);
    funcs.vkGetPhysicalDeviceXlibPresentationSupportKHR = GPA(vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif  // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkCreateWin32SurfaceKHR = GPA(vkCreateWin32SurfaceKHR);
    funcs.vkGetPhysicalDeviceWin32PresentationSupportKHR = GPA(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif  // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkDestroyDevice = GPA(vkDestroyDevice);
    funcs.vkGetDeviceQueue = GPA(vkGetDeviceQueue);
#undef GPA
    // clang-format on
}

#if defined(APPLE_STATIC_LOADER)
VulkanFunctions::VulkanFunctions() {
#else
VulkanFunctions::VulkanFunctions() : loader(get_loader_path()) {
#endif
    init_vulkan_functions(*this);
}

void VulkanFunctions::load_instance_functions(VkInstance instance) {
    vkCreateDebugReportCallbackEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    vkDestroyDebugReportCallbackEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    vkCreateDebugUtilsMessengerEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT = FromVoidStarFunc(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
}

DeviceFunctions::DeviceFunctions(const VulkanFunctions& vulkan_functions, VkDevice device) {
    vkGetDeviceProcAddr = vulkan_functions.vkGetDeviceProcAddr;
    vkDestroyDevice = load(device, "vkDestroyDevice");
    vkGetDeviceQueue = load(device, "vkGetDeviceQueue");
    vkCreateCommandPool = load(device, "vkCreateCommandPool");
    vkAllocateCommandBuffers = load(device, "vkAllocateCommandBuffers");
    vkDestroyCommandPool = load(device, "vkDestroyCommandPool");
    vkCreateSwapchainKHR = load(device, "vkCreateSwapchainKHR");
    vkGetSwapchainImagesKHR = load(device, "vkGetSwapchainImagesKHR");
    vkDestroySwapchainKHR = load(device, "vkDestroySwapchainKHR");
}

InstWrapper::InstWrapper(VulkanFunctions& functions, VkAllocationCallbacks* callbacks) noexcept
    : functions(&functions), callbacks(callbacks) {}
InstWrapper::InstWrapper(VulkanFunctions& functions, VkInstance inst, VkAllocationCallbacks* callbacks) noexcept
    : functions(&functions), inst(inst), callbacks(callbacks) {}
InstWrapper::~InstWrapper() noexcept {
    if (inst != VK_NULL_HANDLE) functions->vkDestroyInstance(inst, callbacks);
}

InstWrapper::InstWrapper(InstWrapper&& other) noexcept {
    functions = other.functions;
    inst = other.inst;
    callbacks = other.callbacks;
    create_info = other.create_info;
    other.inst = VK_NULL_HANDLE;
}
InstWrapper& InstWrapper::operator=(InstWrapper&& other) noexcept {
    functions->vkDestroyInstance(inst, callbacks);
    functions = other.functions;
    inst = other.inst;
    callbacks = other.callbacks;
    create_info = other.create_info;
    other.inst = VK_NULL_HANDLE;
    return *this;
}

void InstWrapper::CheckCreate(VkResult result_to_check) {
    handle_assert_null(inst);
    ASSERT_EQ(result_to_check, functions->vkCreateInstance(create_info.get(), callbacks, &inst));
    functions->load_instance_functions(inst);
}

void InstWrapper::CheckCreateWithInfo(InstanceCreateInfo& create_info, VkResult result_to_check) {
    handle_assert_null(inst);
    ASSERT_EQ(result_to_check, functions->vkCreateInstance(create_info.get(), callbacks, &inst));
    functions->load_instance_functions(inst);
}

std::vector<VkPhysicalDevice> InstWrapper::GetPhysDevs(uint32_t phys_dev_count, VkResult result_to_check) {
    uint32_t physical_count = phys_dev_count;
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(phys_dev_count);
    VkResult res = functions->vkEnumeratePhysicalDevices(inst, &physical_count, physical_devices.data());
    EXPECT_EQ(result_to_check, res);
    return physical_devices;
}

std::vector<VkPhysicalDevice> InstWrapper::GetPhysDevs(VkResult result_to_check) {
    uint32_t physical_count = 0;
    VkResult res = functions->vkEnumeratePhysicalDevices(inst, &physical_count, nullptr);
    EXPECT_EQ(result_to_check, res);
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(physical_count);
    res = functions->vkEnumeratePhysicalDevices(inst, &physical_count, physical_devices.data());
    EXPECT_EQ(result_to_check, res);
    return physical_devices;
}

VkPhysicalDevice InstWrapper::GetPhysDev(VkResult result_to_check) {
    uint32_t physical_count = 1;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkResult res = this->functions->vkEnumeratePhysicalDevices(inst, &physical_count, &physical_device);
    EXPECT_EQ(result_to_check, res);
    return physical_device;
}

std::vector<VkLayerProperties> InstWrapper::GetActiveLayers(VkPhysicalDevice phys_dev, uint32_t expected_count) {
    uint32_t count = 0;
    VkResult res = functions->vkEnumerateDeviceLayerProperties(phys_dev, &count, nullptr);
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    std::vector<VkLayerProperties> layer_props{count};
    res = functions->vkEnumerateDeviceLayerProperties(phys_dev, &count, layer_props.data());
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    return layer_props;
}

std::vector<VkExtensionProperties> InstWrapper::EnumerateDeviceExtensions(VkPhysicalDevice physical_device,
                                                                          uint32_t expected_count) {
    uint32_t count = 0;
    VkResult res = functions->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    std::vector<VkExtensionProperties> extensions{count};
    res = functions->vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    return extensions;
}

std::vector<VkExtensionProperties> InstWrapper::EnumerateLayerDeviceExtensions(VkPhysicalDevice physical_device,
                                                                               const char* layer_name, uint32_t expected_count) {
    uint32_t count = 0;
    VkResult res = functions->vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &count, nullptr);
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    std::vector<VkExtensionProperties> extensions{count};
    res = functions->vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &count, extensions.data());
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    return extensions;
}

DeviceWrapper::DeviceWrapper(InstWrapper& inst_wrapper, VkAllocationCallbacks* callbacks) noexcept
    : functions(inst_wrapper.functions), callbacks(callbacks){};
DeviceWrapper::DeviceWrapper(VulkanFunctions& functions, VkDevice device, VkAllocationCallbacks* callbacks) noexcept
    : functions(&functions), dev(device), callbacks(callbacks){};
DeviceWrapper::~DeviceWrapper() noexcept { functions->vkDestroyDevice(dev, callbacks); }

DeviceWrapper::DeviceWrapper(DeviceWrapper&& other) noexcept {
    functions = other.functions;
    dev = other.dev;
    callbacks = other.callbacks;
    create_info = other.create_info;
    other.dev = VK_NULL_HANDLE;
}
DeviceWrapper& DeviceWrapper::operator=(DeviceWrapper&& other) noexcept {
    functions->vkDestroyDevice(dev, callbacks);
    functions = other.functions;
    dev = other.dev;
    callbacks = other.callbacks;
    create_info = other.create_info;
    other.dev = VK_NULL_HANDLE;
    return *this;
}

void DeviceWrapper::CheckCreate(VkPhysicalDevice phys_dev, VkResult result_to_check) {
    handle_assert_null(dev);
    ASSERT_EQ(result_to_check, functions->vkCreateDevice(phys_dev, create_info.get(), callbacks, &dev));
}

VkResult CreateDebugUtilsMessenger(DebugUtilsWrapper& debug_utils) {
    handle_assert_null(debug_utils.messenger);
    return debug_utils.local_vkCreateDebugUtilsMessengerEXT(debug_utils.inst, debug_utils.get(), debug_utils.callbacks,
                                                            &debug_utils.messenger);
}

void FillDebugUtilsCreateDetails(InstanceCreateInfo& create_info, DebugUtilsLogger& logger) {
    create_info.add_extension("VK_EXT_debug_utils");
    create_info.instance_info.pNext = logger.get();
}
void FillDebugUtilsCreateDetails(InstanceCreateInfo& create_info, DebugUtilsWrapper& wrapper) {
    create_info.add_extension("VK_EXT_debug_utils");
    create_info.instance_info.pNext = wrapper.get();
}

// Look through the event log. If you find a line containing the prefix we're interested in, look for the end of
// line character, and then see if the postfix occurs in it as well.
bool DebugUtilsLogger::find_prefix_then_postfix(const char* prefix, const char* postfix) const {
    size_t new_start = 0;
    size_t postfix_index = 0;
    size_t next_eol = 0;
    while ((new_start = returned_output.find(prefix, new_start)) != std::string::npos) {
        next_eol = returned_output.find("\n", new_start);
        if ((postfix_index = returned_output.find(postfix, new_start)) != std::string::npos) {
            if (postfix_index < next_eol) {
                return true;
            }
        }
        new_start = next_eol + 1;
    }
    return false;
}

bool FindPrefixPostfixStringOnLine(DebugUtilsLogger const& env_log, const char* prefix, const char* postfix) {
    return env_log.find_prefix_then_postfix(prefix, postfix);
}

namespace fs {
FolderManager::FolderManager(std::filesystem::path root_path, std::string name) noexcept : folder(root_path / name) {
    clear();
    // Don't actually create the folder yet, as we will do it on demand
}
FolderManager::~FolderManager() noexcept { clear(); }
FolderManager::FolderManager(FolderManager&& other) noexcept : actually_created(other.actually_created), folder(other.folder) {
    other.folder.clear();
}
FolderManager& FolderManager::operator=(FolderManager&& other) noexcept {
    folder = other.folder;
    actually_created = other.actually_created;
    other.folder.clear();
    return *this;
}

void FolderManager::check_if_first_use() {
    if (!actually_created) {
        if (!::testing::internal::InDeathTestChild()) {
            std::error_code err;
            if (!std::filesystem::create_directories(folder, err)) {
                std::cerr << "Failed to create folder " << folder << " because " << err.message() << "\n";
            }
        }
        actually_created = true;
    }
}

std::filesystem::path FolderManager::write_manifest(std::filesystem::path const& name, std::string const& contents) {
    check_if_first_use();
    std::filesystem::path out_path = folder / name;
    if (!::testing::internal::InDeathTestChild()) {
        auto file = std::ofstream(out_path, std::ios_base::trunc | std::ios_base::out);
        if (!file) {
            std::cerr << "Failed to create manifest " << name << " at " << out_path << "\n";
            return out_path;
        }
        file << contents << std::endl;
    }
    insert_file_to_tracking(name);
    return out_path;
}

// close file handle, delete file, remove `name` from managed file list.
void FolderManager::remove(std::filesystem::path const& name) {
    check_if_first_use();
    std::filesystem::path out_path = folder / name;
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        if (!std::filesystem::remove(out_path, err)) {
            std::cerr << "Failed to remove file " << name << " at " << out_path << " because " << err.message() << "\n";
        }
    }

    auto found = std::find(added_files.begin(), added_files.end(), name);
    if (found != added_files.end()) {
        added_files.erase(found);
    } else {
        std::cout << "File " << name << " not in tracked files of folder " << folder << ".\n";
    }
}

// copy file into this folder
std::filesystem::path FolderManager::copy_file(std::filesystem::path const& file, std::filesystem::path const& new_name) {
    check_if_first_use();
    insert_file_to_tracking(new_name);

    auto new_filepath = folder / new_name;
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        if (!std::filesystem::copy_file(file, new_filepath, err)) {
            std::cerr << "Failed to copy file " << file << " to " << new_filepath << " because " << err.message() << "\n";
        }
    }
    return new_filepath;
}

std::vector<std::filesystem::path> FolderManager::get_files() const { return added_files; }

std::filesystem::path FolderManager::add_symlink(std::filesystem::path const& target, std::filesystem::path const& link_name) {
    check_if_first_use();

    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        std::filesystem::create_symlink(target, folder / link_name, err);
        if (err.value() != 0) {
            std::cerr << "Failed to create symlink with target" << target << " with name " << folder / link_name << " because "
                      << err.message() << "\n";
        }
    }
    insert_file_to_tracking(link_name);
    return folder / link_name;
}
void FolderManager::insert_file_to_tracking(std::filesystem::path const& name) {
    auto found = std::find(added_files.begin(), added_files.end(), name);
    if (found != added_files.end()) {
        std::cout << "Overwriting manifest " << name << ". Was this intended?\n";
    } else {
        added_files.emplace_back(name);
    }
}

void FolderManager::clear() const noexcept {
    if (!::testing::internal::InDeathTestChild()) {
        std::error_code err;
        std::filesystem::remove_all(folder, err);
        if (err.value() != 0) {
            std::cerr << "Failed to remove folder " << folder << " because " << err.message() << "\n";
        }
    }
}

}  // namespace fs

PlatformShimWrapper::PlatformShimWrapper(GetFoldersFunc get_folders_by_name_function, const char* log_filter) noexcept
    : loader_logging{"VK_LOADER_DEBUG"} {
#if defined(WIN32) || defined(__APPLE__)
    shim_library = LibraryWrapper(SHIM_LIBRARY_NAME);
    PFN_get_platform_shim get_platform_shim_func = shim_library.get_symbol(GET_PLATFORM_SHIM_STR);
    assert(get_platform_shim_func != NULL && "Must be able to get \"platform_shim\"");
    platform_shim = get_platform_shim_func(get_folders_by_name_function);
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
    platform_shim = get_platform_shim(get_folders_by_name_function);
#endif
    platform_shim->reset();

    if (log_filter) {
        loader_logging.set_new_value(log_filter);
    }
}

PlatformShimWrapper::~PlatformShimWrapper() noexcept { platform_shim->reset(); }

TestICDHandle::TestICDHandle() noexcept {}
TestICDHandle::TestICDHandle(std::filesystem::path const& icd_path) noexcept : icd_library(icd_path) {
    proc_addr_get_test_icd = icd_library.get_symbol(GET_TEST_ICD_FUNC_STR);
    proc_addr_reset_icd = icd_library.get_symbol(RESET_ICD_FUNC_STR);
}
TestICD& TestICDHandle::get_test_icd() noexcept {
    assert(proc_addr_get_test_icd != NULL && "symbol must be loaded before use");
    return *proc_addr_get_test_icd();
}
TestICD& TestICDHandle::reset_icd() noexcept {
    assert(proc_addr_reset_icd != NULL && "symbol must be loaded before use");
    return *proc_addr_reset_icd();
}
std::filesystem::path TestICDHandle::get_icd_full_path() noexcept { return icd_library.lib_path; }
std::filesystem::path TestICDHandle::get_icd_manifest_path() noexcept { return manifest_path; }
std::filesystem::path TestICDHandle::get_shimmed_manifest_path() noexcept { return shimmed_manifest_path; }

TestLayerHandle::TestLayerHandle() noexcept {}
TestLayerHandle::TestLayerHandle(std::filesystem::path const& layer_path) noexcept : layer_library(layer_path) {
    proc_addr_get_test_layer = layer_library.get_symbol(GET_TEST_LAYER_FUNC_STR);
    proc_addr_reset_layer = layer_library.get_symbol(RESET_LAYER_FUNC_STR);
}
TestLayer& TestLayerHandle::get_test_layer() noexcept {
    assert(proc_addr_get_test_layer != NULL && "symbol must be loaded before use");
    return *proc_addr_get_test_layer();
}
TestLayer& TestLayerHandle::reset_layer() noexcept {
    assert(proc_addr_reset_layer != NULL && "symbol must be loaded before use");
    return *proc_addr_reset_layer();
}
std::filesystem::path TestLayerHandle::get_layer_full_path() noexcept { return layer_library.lib_path; }
std::filesystem::path TestLayerHandle::get_layer_manifest_path() noexcept { return manifest_path; }
std::filesystem::path TestLayerHandle::get_shimmed_manifest_path() noexcept { return shimmed_manifest_path; }

FrameworkEnvironment::FrameworkEnvironment() noexcept : FrameworkEnvironment(FrameworkSettings{}) {}
FrameworkEnvironment::FrameworkEnvironment(FrameworkSettings const& settings) noexcept
    : settings(settings),
      test_folder(std::filesystem::path(FRAMEWORK_BUILD_DIRECTORY),
                  std::string(::testing::UnitTest::GetInstance()->GetInstance()->current_test_suite()->name()) + "_" +
                      ::testing::UnitTest::GetInstance()->current_test_info()->name()),
      platform_shim(
          [this](const char* folder_name) -> std::vector<std::filesystem::path> {
              for (auto& folder : folders) {
                  if (folder.location() == folder_name) {
                      return folder.get_files();
                  }
              }
              return {};
          },
          settings.log_filter) {
    // Clean out test folder in case a previous run's files are still around
    test_folder.clear();

    // This order is important, it matches the enum ManifestLocation, used to index the folders vector
    folders.emplace_back(test_folder.location(), std::string("null_dir"));
    folders.emplace_back(test_folder.location(), std::string("icd_manifests"));
    folders.emplace_back(test_folder.location(), std::string("icd_env_vars_manifests"));
    folders.emplace_back(test_folder.location(), std::string("explicit_layer_manifests"));
    folders.emplace_back(test_folder.location(), std::string("explicit_env_var_layer_folder"));
    folders.emplace_back(test_folder.location(), std::string("explicit_add_env_var_layer_folder"));
    folders.emplace_back(test_folder.location(), std::string("implicit_layer_manifests"));
    folders.emplace_back(test_folder.location(), std::string("implicit_env_var_layer_manifests"));
    folders.emplace_back(test_folder.location(), std::string("implicit_add_env_var_layer_manifests"));
    folders.emplace_back(test_folder.location(), std::string("override_layer_manifests"));
    folders.emplace_back(test_folder.location(), std::string("app_package_manifests"));
    folders.emplace_back(test_folder.location(), std::string("macos_bundle"));
    folders.emplace_back(test_folder.location(), std::string("unsecured_location"));
    folders.emplace_back(test_folder.location(), std::string("settings_location"));

    platform_shim->redirect_all_paths(get_folder(ManifestLocation::null).location());
    if (settings.enable_default_search_paths) {
        platform_shim->set_fake_path(ManifestCategory::icd, get_folder(ManifestLocation::driver).location());
        platform_shim->set_fake_path(ManifestCategory::explicit_layer, get_folder(ManifestLocation::explicit_layer).location());
        platform_shim->set_fake_path(ManifestCategory::implicit_layer, get_folder(ManifestLocation::implicit_layer).location());
#if COMMON_UNIX_PLATFORMS
        auto home = get_env_var("HOME");
        auto unsecured_location = get_folder(ManifestLocation::unsecured_location).location();
        platform_shim->redirect_path(home + "/.local/share/vulkan/icd.d", unsecured_location);
        platform_shim->redirect_path(home + "/.local/share/vulkan/implicit_layer.d", unsecured_location);
        platform_shim->redirect_path(home + "/.local/share/vulkan/explicit_layer.d", unsecured_location);
#endif
    }
#if COMMON_UNIX_PLATFORMS
    if (settings.secure_loader_settings) {
        platform_shim->redirect_path("/etc/vulkan/loader_settings.d", get_folder(ManifestLocation::settings_location).location());
    } else {
        platform_shim->redirect_path(get_env_var("HOME") + "/.local/share/vulkan/loader_settings.d",
                                     get_folder(ManifestLocation::settings_location).location());
    }
#endif

#if defined(__APPLE__)
    // Necessary since bundles look in sub folders for manifests, not the test framework folder itself
    auto bundle_location = get_folder(ManifestLocation::macos_bundle).location();
    platform_shim->redirect_path(bundle_location / "vulkan/icd.d", bundle_location);
    platform_shim->redirect_path(bundle_location / "vulkan/explicit_layer.d", bundle_location);
    platform_shim->redirect_path(bundle_location / "vulkan/implicit_layer.d", bundle_location);
#endif
    // only set the settings file if there are elements in the app_specific_settings vector
    if (!settings.loader_settings.app_specific_settings.empty()) {
        update_loader_settings(settings.loader_settings);
    }
}

FrameworkEnvironment::~FrameworkEnvironment() {
    // This is necessary to prevent the folder manager from using dead memory during destruction.
    // What happens is that each folder manager tries to cleanup itself. Except, folders that were never called did not have
    // their DirEntry array's filled out. So when that folder calls delete_folder, which calls readdir, the shim tries to order
    // the files. Except, the list of files is in a object that is currently being destroyed.
    platform_shim->is_during_destruction = true;
}

TestICD& FrameworkEnvironment::add_icd(TestICDDetails icd_details) noexcept {
    size_t cur_icd_index = icds.size();
    fs::FolderManager* fs_ptr = &get_folder(ManifestLocation::driver);
    switch (icd_details.discovery_type) {
        case (ManifestDiscoveryType::env_var):
        case (ManifestDiscoveryType::add_env_var):
            fs_ptr = &get_folder(ManifestLocation::driver_env_var);
            break;
        case (ManifestDiscoveryType::windows_app_package):
            fs_ptr = &get_folder(ManifestLocation::windows_app_package);
            break;
        case (ManifestDiscoveryType::override_folder):
            fs_ptr = &get_folder(ManifestLocation::override_layer);
            break;
        case (ManifestDiscoveryType::macos_bundle):
            fs_ptr = &get_folder(ManifestLocation::macos_bundle);
            break;
        case (ManifestDiscoveryType::unsecured_generic):
            fs_ptr = &get_folder(ManifestLocation::unsecured_location);
            break;
        case (ManifestDiscoveryType::null_dir):
        case (ManifestDiscoveryType::none):
            fs_ptr = &get_folder(ManifestLocation::null);
            break;
        case (ManifestDiscoveryType::generic):
            fs_ptr = &get_folder(ManifestLocation::driver);
            break;
    }
    auto& folder = *fs_ptr;

    if (!icd_details.is_fake) {
        std::filesystem::path new_lib_name = icd_details.icd_manifest.lib_path.stem();
        new_lib_name += "_";
        new_lib_name += std::to_string(cur_icd_index);
        new_lib_name += icd_details.icd_manifest.lib_path.extension();
        auto new_driver_location = folder.copy_file(icd_details.icd_manifest.lib_path, new_lib_name);

#if COMMON_UNIX_PLATFORMS
        if (icd_details.library_path_type == LibraryPathType::default_search_paths) {
            platform_shim->redirect_dlopen_name(new_lib_name, new_driver_location);
        } else if (icd_details.library_path_type == LibraryPathType::relative) {
            platform_shim->redirect_dlopen_name(std::filesystem::path(SYSCONFDIR) / "vulkan" / "icd.d" / "." / new_lib_name,
                                                new_driver_location);
        }
#endif
#if defined(WIN32)
        if (icd_details.library_path_type == LibraryPathType::default_search_paths) {
            SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
            AddDllDirectory(new_driver_location.parent_path().native().c_str());
        }
#endif
        icds.push_back(TestICDHandle(new_driver_location));
        icds.back().reset_icd();
        if (icd_details.library_path_type == LibraryPathType::relative) {
            icd_details.icd_manifest.lib_path = std::filesystem::path(".") / new_lib_name;
        } else if (icd_details.library_path_type == LibraryPathType::default_search_paths) {
            icd_details.icd_manifest.lib_path = new_lib_name;
        } else {
            icd_details.icd_manifest.lib_path = new_driver_location;
        }
    }
    if (icd_details.discovery_type != ManifestDiscoveryType::none) {
        std::filesystem::path new_manifest_path = icd_details.json_name.stem();
        if (!icd_details.disable_icd_inc) {
            new_manifest_path += "_";
            new_manifest_path += std::to_string(cur_icd_index);
        }
        new_manifest_path += ".json";
        icds.back().manifest_path = folder.write_manifest(new_manifest_path, icd_details.icd_manifest.get_manifest_str());
        icds.back().shimmed_manifest_path = icds.back().manifest_path;
        switch (icd_details.discovery_type) {
            case (ManifestDiscoveryType::generic):
                platform_shim->add_manifest(ManifestCategory::icd, icds.back().manifest_path);
#if COMMON_UNIX_PLATFORMS
                icds.back().shimmed_manifest_path =
                    platform_shim->query_default_redirect_path(ManifestCategory::icd) / new_manifest_path;
#endif
                break;
            case (ManifestDiscoveryType::env_var):
                if (icd_details.is_dir) {
                    env_var_vk_icd_filenames.add_to_list(folder.location());
                } else {
                    env_var_vk_icd_filenames.add_to_list(folder.location() / new_manifest_path);
                }
                platform_shim->add_known_path(folder.location());
                break;
            case (ManifestDiscoveryType::add_env_var):
                if (icd_details.is_dir) {
                    add_env_var_vk_icd_filenames.add_to_list(folder.location());
                } else {
                    add_env_var_vk_icd_filenames.add_to_list(folder.location() / new_manifest_path);
                }
                platform_shim->add_known_path(folder.location());
                break;
            case (ManifestDiscoveryType::override_folder):
            case (ManifestDiscoveryType::macos_bundle):
                platform_shim->add_manifest(ManifestCategory::icd, icds.back().manifest_path);
                break;
            case (ManifestDiscoveryType::unsecured_generic):
                platform_shim->add_unsecured_manifest(ManifestCategory::icd, icds.back().manifest_path);
                break;
            case (ManifestDiscoveryType::null_dir):
            case (ManifestDiscoveryType::none):
                break;
            case (ManifestDiscoveryType::windows_app_package):
                platform_shim->set_app_package_path(folder.location());
                break;
        }
    }
    return icds.back().get_test_icd();
}

void FrameworkEnvironment::add_implicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept {
    add_layer_impl(TestLayerDetails{layer_manifest, json_name}, ManifestCategory::implicit_layer);
}
void FrameworkEnvironment::add_explicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept {
    add_layer_impl(TestLayerDetails{layer_manifest, json_name}, ManifestCategory::explicit_layer);
}
void FrameworkEnvironment::add_fake_implicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept {
    add_layer_impl(TestLayerDetails{layer_manifest, json_name}.set_is_fake(true), ManifestCategory::implicit_layer);
}
void FrameworkEnvironment::add_fake_explicit_layer(ManifestLayer layer_manifest, const std::string& json_name) noexcept {
    add_layer_impl(TestLayerDetails{layer_manifest, json_name}.set_is_fake(true), ManifestCategory::explicit_layer);
}
void FrameworkEnvironment::add_implicit_layer(TestLayerDetails layer_details) noexcept {
    add_layer_impl(layer_details, ManifestCategory::implicit_layer);
}
void FrameworkEnvironment::add_explicit_layer(TestLayerDetails layer_details) noexcept {
    add_layer_impl(layer_details, ManifestCategory::explicit_layer);
}

void FrameworkEnvironment::add_layer_impl(TestLayerDetails layer_details, ManifestCategory category) {
    fs::FolderManager* fs_ptr = &get_folder(ManifestLocation::explicit_layer);
    switch (layer_details.discovery_type) {
        case (ManifestDiscoveryType::generic):
            if (category == ManifestCategory::implicit_layer) fs_ptr = &get_folder(ManifestLocation::implicit_layer);
            break;
        case (ManifestDiscoveryType::env_var):
            if (category == ManifestCategory::explicit_layer) {
                fs_ptr = &get_folder(ManifestLocation::explicit_layer_env_var);
                if (layer_details.is_dir) {
                    env_var_vk_layer_paths.add_to_list(fs_ptr->location());
                } else {
                    env_var_vk_layer_paths.add_to_list(fs_ptr->location() / layer_details.json_name);
                }
            }
            if (category == ManifestCategory::implicit_layer) {
                fs_ptr = &get_folder(ManifestLocation::implicit_layer_env_var);
                if (layer_details.is_dir) {
                    env_var_vk_implicit_layer_paths.add_to_list(fs_ptr->location());
                } else {
                    env_var_vk_implicit_layer_paths.add_to_list(fs_ptr->location() / layer_details.json_name);
                }
            }
            platform_shim->add_known_path(fs_ptr->location());
            break;
        case (ManifestDiscoveryType::add_env_var):
            if (category == ManifestCategory::explicit_layer) {
                fs_ptr = &get_folder(ManifestLocation::explicit_layer_add_env_var);
                if (layer_details.is_dir) {
                    add_env_var_vk_layer_paths.add_to_list(fs_ptr->location());
                } else {
                    add_env_var_vk_layer_paths.add_to_list(fs_ptr->location() / layer_details.json_name);
                }
            }
            if (category == ManifestCategory::implicit_layer) {
                fs_ptr = &get_folder(ManifestLocation::implicit_layer_add_env_var);
                if (layer_details.is_dir) {
                    add_env_var_vk_implicit_layer_paths.add_to_list(fs_ptr->location());
                } else {
                    add_env_var_vk_implicit_layer_paths.add_to_list(fs_ptr->location() / layer_details.json_name);
                }
            }
            platform_shim->add_known_path(fs_ptr->location());
            break;
        case (ManifestDiscoveryType::override_folder):
            fs_ptr = &get_folder(ManifestLocation::override_layer);
            break;
        case (ManifestDiscoveryType::macos_bundle):
            fs_ptr = &(get_folder(ManifestLocation::macos_bundle));
            break;
        case (ManifestDiscoveryType::unsecured_generic):
            fs_ptr = &(get_folder(ManifestLocation::unsecured_location));
            break;
        case (ManifestDiscoveryType::windows_app_package):
            fs_ptr = &(get_folder(ManifestLocation::windows_app_package));
            break;
        case (ManifestDiscoveryType::none):
        case (ManifestDiscoveryType::null_dir):
            fs_ptr = &(get_folder(ManifestLocation::null));
            break;
    }
    auto& folder = *fs_ptr;
    size_t new_layers_start = layers.size();
    for (auto& layer : layer_details.layer_manifest.layers) {
        if (!layer.lib_path.empty()) {
            std::filesystem::path new_lib_path = layer.lib_path.stem();
            new_lib_path += "_";
            new_lib_path += std::to_string(layers.size());
            new_lib_path += layer.lib_path.extension();

            auto new_layer_location = folder.copy_file(layer.lib_path, new_lib_path);

#if COMMON_UNIX_PLATFORMS
            if (layer_details.library_path_type == LibraryPathType::default_search_paths) {
                platform_shim->redirect_dlopen_name(new_lib_path, new_layer_location);
            }
            if (layer_details.library_path_type == LibraryPathType::relative) {
                platform_shim->redirect_dlopen_name(
                    std::filesystem::path(SYSCONFDIR) / "vulkan" / category_path_name(category) / "." / new_lib_path,
                    new_layer_location);
            }
#endif
#if defined(WIN32)
            if (layer_details.library_path_type == LibraryPathType::default_search_paths) {
                SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
                AddDllDirectory(new_layer_location.parent_path().native().c_str());
            }
#endif

            // Don't load the layer binary if using any of the wrap objects layers, since it doesn't export the same interface
            // functions
            if (!layer_details.is_fake &&
                layer.lib_path.stem().string().find(std::filesystem::path(TEST_LAYER_WRAP_OBJECTS).stem().string()) ==
                    std::string::npos) {
                layers.push_back(TestLayerHandle(new_layer_location));
                layers.back().reset_layer();
            }
            if (layer_details.library_path_type == LibraryPathType::relative) {
                layer.lib_path = std::filesystem::path(".") / new_lib_path;
            } else if (layer_details.library_path_type == LibraryPathType::default_search_paths) {
                layer.lib_path = new_lib_path;
            } else {
                layer.lib_path = new_layer_location;
            }
        }
    }
    if (layer_details.discovery_type != ManifestDiscoveryType::none) {
        // Write a manifest file to a folder as long as the discovery type isn't none
        auto layer_manifest_loc = folder.write_manifest(layer_details.json_name, layer_details.layer_manifest.get_manifest_str());
        // only add the manifest to the registry if its a generic location (as if it was installed) - both system and user local
        if (layer_details.discovery_type == ManifestDiscoveryType::generic) {
            platform_shim->add_manifest(category, layer_manifest_loc);
        }
        if (layer_details.discovery_type == ManifestDiscoveryType::unsecured_generic) {
            platform_shim->add_unsecured_manifest(category, layer_manifest_loc);
        }
        if (layer_details.discovery_type == ManifestDiscoveryType::windows_app_package) {
            platform_shim->set_app_package_path(folder.location());
        }
        for (size_t i = new_layers_start; i < layers.size(); i++) {
            layers.at(i).manifest_path = layer_manifest_loc;
            layers.at(i).shimmed_manifest_path = layer_manifest_loc;
#if COMMON_UNIX_PLATFORMS
            if (layer_details.discovery_type == ManifestDiscoveryType::generic) {
                layers.at(i).shimmed_manifest_path = platform_shim->query_default_redirect_path(category) / layer_details.json_name;
            }
#endif
        }
    }
}

std::string get_loader_settings_file_contents(const LoaderSettings& loader_settings) noexcept {
    JsonWriter writer;
    writer.StartObject();
    writer.AddKeyedString("file_format_version", loader_settings.file_format_version.get_version_str());
    bool one_setting_file = true;
    if (loader_settings.app_specific_settings.size() > 1) {
        writer.StartKeyedArray("settings_array");
        one_setting_file = false;
    }
    for (const auto& setting : loader_settings.app_specific_settings) {
        if (one_setting_file) {
            writer.StartKeyedObject("settings");
        } else {
            writer.StartObject();
        }
        if (!setting.app_keys.empty()) {
            writer.StartKeyedArray("app_keys");
            for (const auto& app_key : setting.app_keys) {
                writer.AddString(app_key);
            }
            writer.EndArray();
        }
        if (!setting.layer_configurations.empty()) {
            writer.StartKeyedArray("layers");
            for (const auto& config : setting.layer_configurations) {
                writer.StartObject();
                writer.AddKeyedString("name", config.name);
                writer.AddKeyedString("path", config.path.native());
                writer.AddKeyedString("control", config.control);
                writer.AddKeyedBool("treat_as_implicit_manifest", config.treat_as_implicit_manifest);
                writer.EndObject();
            }
            writer.EndArray();
        }
        if (!setting.stderr_log.empty()) {
            writer.StartKeyedArray("stderr_log");
            for (const auto& filter : setting.stderr_log) {
                writer.AddString(filter);
            }
            writer.EndArray();
        }
        if (!setting.log_configurations.empty()) {
            writer.StartKeyedArray("log_locations");
            for (const auto& config : setting.log_configurations) {
                writer.StartObject();
                writer.StartKeyedArray("destinations");
                for (const auto& dest : config.destinations) {
                    writer.AddString(dest);
                }
                writer.EndArray();
                writer.StartKeyedArray("filter");
                for (const auto& filter : config.filters) {
                    writer.AddString(filter);
                }
                writer.EndArray();
                writer.EndObject();
            }
            writer.EndArray();
        }
        writer.EndObject();
    }
    if (!one_setting_file) {
        writer.EndArray();
    }

    writer.EndObject();
    return writer.output;
}
void FrameworkEnvironment::write_settings_file(std::string const& file_contents) {
    auto out_path = get_folder(ManifestLocation::settings_location).write_manifest("vk_loader_settings.json", file_contents);
#if defined(WIN32)
    platform_shim->hkey_current_user_settings.clear();
    platform_shim->hkey_local_machine_settings.clear();
#endif
    if (settings.secure_loader_settings)
        platform_shim->add_manifest(ManifestCategory::settings, out_path);
    else
        platform_shim->add_unsecured_manifest(ManifestCategory::settings, out_path);
}
void FrameworkEnvironment::update_loader_settings(const LoaderSettings& settings) noexcept {
    write_settings_file(get_loader_settings_file_contents(settings));
}
void FrameworkEnvironment::remove_loader_settings() {
    get_folder(ManifestLocation::settings_location).remove("vk_loader_settings.json");
}
void FrameworkEnvironment::write_file_from_source(const char* source_file, ManifestCategory category, ManifestLocation location,
                                                  std::string const& file_name) {
    std::fstream file{source_file, std::ios_base::in};
    ASSERT_TRUE(file.is_open());
    std::stringstream file_stream;
    file_stream << file.rdbuf();

    auto out_path = get_folder(location).write_manifest(file_name, file_stream.str());

    if (settings.secure_loader_settings)
        platform_shim->add_manifest(category, out_path);
    else
        platform_shim->add_unsecured_manifest(category, out_path);
}

TestICD& FrameworkEnvironment::get_test_icd(size_t index) noexcept { return icds[index].get_test_icd(); }
TestICD& FrameworkEnvironment::reset_icd(size_t index) noexcept { return icds[index].reset_icd(); }
std::filesystem::path FrameworkEnvironment::get_test_icd_path(size_t index) noexcept { return icds[index].get_icd_full_path(); }
std::filesystem::path FrameworkEnvironment::get_icd_manifest_path(size_t index) noexcept {
    return icds[index].get_icd_manifest_path();
}
std::filesystem::path FrameworkEnvironment::get_shimmed_icd_manifest_path(size_t index) noexcept {
    return icds[index].get_shimmed_manifest_path();
}

TestLayer& FrameworkEnvironment::get_test_layer(size_t index) noexcept { return layers[index].get_test_layer(); }
TestLayer& FrameworkEnvironment::reset_layer(size_t index) noexcept { return layers[index].reset_layer(); }
std::filesystem::path FrameworkEnvironment::get_test_layer_path(size_t index) noexcept {
    return layers[index].get_layer_full_path();
}
std::filesystem::path FrameworkEnvironment::get_layer_manifest_path(size_t index) noexcept {
    return layers[index].get_layer_manifest_path();
}
std::filesystem::path FrameworkEnvironment::get_shimmed_layer_manifest_path(size_t index) noexcept {
    return layers[index].get_shimmed_manifest_path();
}

fs::FolderManager& FrameworkEnvironment::get_folder(ManifestLocation location) noexcept {
    // index it directly using the enum location since they will always be in that order
    return folders.at(static_cast<size_t>(location));
}
fs::FolderManager const& FrameworkEnvironment::get_folder(ManifestLocation location) const noexcept {
    return folders.at(static_cast<size_t>(location));
}
#if defined(__APPLE__)
void FrameworkEnvironment::setup_macos_bundle() noexcept {
    platform_shim->bundle_contents = get_folder(ManifestLocation::macos_bundle).location();
}
#endif

std::vector<VkExtensionProperties> FrameworkEnvironment::GetInstanceExtensions(uint32_t expected_count, const char* layer_name) {
    uint32_t count = 0;
    VkResult res = vulkan_functions.vkEnumerateInstanceExtensionProperties(layer_name, &count, nullptr);
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    std::vector<VkExtensionProperties> extension_props{count};
    res = vulkan_functions.vkEnumerateInstanceExtensionProperties(layer_name, &count, extension_props.data());
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    return extension_props;
}
std::vector<VkLayerProperties> FrameworkEnvironment::GetLayerProperties(uint32_t expected_count) {
    uint32_t count = 0;
    VkResult res = vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr);
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    std::vector<VkLayerProperties> layer_props{count};
    res = vulkan_functions.vkEnumerateInstanceLayerProperties(&count, layer_props.data());
    EXPECT_EQ(VK_SUCCESS, res);
    EXPECT_EQ(count, expected_count);
    return layer_props;
}

template <typename CreationFunc, typename CreateInfo>
VkResult create_surface_helper(VulkanFunctions* functions, VkInstance inst, VkSurfaceKHR& surface, const char* load_func_name,
                               VkStructureType stype) {
    CreationFunc pfn_CreateSurface = functions->load(inst, load_func_name);
    if (!pfn_CreateSurface) return VK_ERROR_EXTENSION_NOT_PRESENT;
    CreateInfo surf_create_info{stype};
    return pfn_CreateSurface(inst, &surf_create_info, nullptr, &surface);
}
VkResult create_surface(VulkanFunctions* functions, VkInstance inst, VkSurfaceKHR& surface,
                        [[maybe_unused]] const char* api_selection) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    return create_surface_helper<PFN_vkCreateAndroidSurfaceKHR, VkAndroidSurfaceCreateInfoKHR>(
        functions, inst, surface, "vkCreateAndroidSurfaceKHR", VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    return create_surface_helper<PFN_vkCreateDirectFBSurfaceEXT, VkDirectFBSurfaceCreateInfoEXT>(
        functions, inst, surface, "vkCreateDirectFBSurfaceEXT", VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT);
#elif defined(VK_USE_PLATFORM_FUCHSIA)
    return create_surface_helper<PFN_vkCreateImagePipeSurfaceFUCHSIA, VkImagePipeSurfaceCreateInfoFUCHSIA>(
        functions, inst, surface, "vkCreateImagePipeSurfaceFUCHSIA", VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA);
#elif defined(VK_USE_PLATFORM_GGP)
    return create_surface_helper<PFN__vkCreateStreamDescriptorSurfaceGGP, VkStreamDescriptorSurfaceCreateInfoGGP>(
        functions, inst, surface, "vkCreateStreamDescriptorSurfaceGGP",
        VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    return create_surface_helper<PFN_vkCreateIOSSurfaceMVK, VkIOSSurfaceCreateInfoMVK>(
        functions, inst, surface, "vkCreateIOSSurfaceMVK", VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK);
#elif defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (api_selection != nullptr && string_eq(api_selection, "VK_USE_PLATFORM_MACOS_MVK"))
        return create_surface_helper<PFN_vkCreateMacOSSurfaceMVK, VkMacOSSurfaceCreateInfoMVK>(
            functions, inst, surface, "vkCreateMacOSSurfaceMVK", VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK);
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (api_selection == nullptr || (api_selection != nullptr && string_eq(api_selection, "VK_USE_PLATFORM_METAL_EXT")))
        return create_surface_helper<PFN_vkCreateMetalSurfaceEXT, VkMetalSurfaceCreateInfoEXT>(
            functions, inst, surface, "vkCreateMetalSurfaceEXT", VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT);
#endif
    return VK_ERROR_NOT_PERMITTED_KHR;
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    return create_surface_helper<PFN_vkCreateScreenSurfaceQNX, VkScreenSurfaceCreateInfoQNX>(
        functions, inst, surface, "vkCreateScreenSurfaceQNX", VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX);
#elif defined(VK_USE_PLATFORM_VI_NN)
    return create_surface_helper<PFN_vkCreateViSurfaceNN, VkViSurfaceCreateInfoNN>(functions, inst, surface, "vkCreateViSurfaceNN",
                                                                                   VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    return create_surface_helper<PFN_vkCreateWin32SurfaceKHR, VkWin32SurfaceCreateInfoKHR>(
        functions, inst, surface, "vkCreateWin32SurfaceKHR", VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR);
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR)
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_XLIB_KHR"))
        return create_surface_helper<PFN_vkCreateXlibSurfaceKHR, VkXlibSurfaceCreateInfoKHR>(
            functions, inst, surface, "vkCreateXlibSurfaceKHR", VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (string_eq(api_selection, "VK_USE_PLATFORM_WAYLAND_KHR"))
        return create_surface_helper<PFN_vkCreateWaylandSurfaceKHR, VkWaylandSurfaceCreateInfoKHR>(
            functions, inst, surface, "vkCreateWaylandSurfaceKHR", VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR);
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (api_selection == nullptr || string_eq(api_selection, "VK_USE_PLATFORM_XCB_KHR"))
        return create_surface_helper<PFN_vkCreateXcbSurfaceKHR, VkXcbSurfaceCreateInfoKHR>(
            functions, inst, surface, "vkCreateXcbSurfaceKHR", VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR);
#endif
    return VK_ERROR_NOT_PERMITTED_KHR;
#else
    return create_surface_helper<PFN_vkCreateDisplayPlaneSurfaceKHR, VkDisplaySurfaceCreateInfoKHR>(
        functions, inst, surface, "vkCreateDisplayPlaneSurfaceKHR", VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR);
#endif
}
VkResult create_surface(InstWrapper& inst, VkSurfaceKHR& surface, const char* api_selection) {
    return create_surface(inst.functions, inst.inst, surface, api_selection);
}

VkResult create_debug_callback(InstWrapper& inst, const VkDebugReportCallbackCreateInfoEXT& create_info,
                               VkDebugReportCallbackEXT& callback) {
    return inst.functions->vkCreateDebugReportCallbackEXT(inst.inst, &create_info, nullptr, &callback);
}

extern "C" {
void __ubsan_on_report() { FAIL() << "Encountered an undefined behavior sanitizer error"; }
void __asan_on_error() { FAIL() << "Encountered an address sanitizer error"; }
void __tsan_on_report() { FAIL() << "Encountered a thread sanitizer error"; }
}  // extern "C"
