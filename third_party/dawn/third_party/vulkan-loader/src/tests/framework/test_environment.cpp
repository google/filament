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

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

#include "json_writer.h"
#include "test_defines.h"

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
    instance_functions = InstanceFunctions(functions->vkGetInstanceProcAddr, inst);
}

void InstWrapper::CheckCreateWithInfo(InstanceCreateInfo& create_info, VkResult result_to_check) {
    handle_assert_null(inst);
    ASSERT_EQ(result_to_check, functions->vkCreateInstance(create_info.get(), callbacks, &inst));
    instance_functions = InstanceFunctions(functions->vkGetInstanceProcAddr, inst);
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

PlatformShimWrapper::PlatformShimWrapper(fs::FileSystemManager& file_system_manager, const char* log_filter) noexcept
    : loader_logging{"VK_LOADER_DEBUG"} {
#if defined(WIN32) || defined(__APPLE__)
    shim_library = LibraryWrapper(SHIM_LIBRARY_NAME);
    PFN_get_platform_shim get_platform_shim = shim_library.get_symbol(GET_PLATFORM_SHIM_STR);
    assert(get_platform_shim != NULL && "Must be able to get \"platform_shim\"");
    platform_shim = get_platform_shim();
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GNU__)
    platform_shim = get_platform_shim();
#endif
    platform_shim->file_system_manager = &file_system_manager;

    if (log_filter) {
        loader_logging.set_new_value(log_filter);
    }
}

template <>
TestBinaryHandle<TestICD, GetTestICDFunc, GetNewTestICDFunc>::TestBinaryHandle(std::filesystem::path const& binary_path) noexcept
    : library(binary_path) {
    proc_addr_get_test_binary = library.get_symbol(GET_TEST_ICD_FUNC_STR);
    proc_addr_reset_binary = library.get_symbol(RESET_ICD_FUNC_STR);
}

template <>
TestBinaryHandle<TestLayer, GetTestLayerFunc, GetNewTestLayerFunc>::TestBinaryHandle(
    std::filesystem::path const& binary_path) noexcept
    : library(binary_path) {
    proc_addr_get_test_binary = library.get_symbol(GET_TEST_LAYER_FUNC_STR);
    proc_addr_reset_binary = library.get_symbol(RESET_LAYER_FUNC_STR);
}

FrameworkEnvironment::FrameworkEnvironment() noexcept : FrameworkEnvironment(FrameworkSettings{}) {}
FrameworkEnvironment::FrameworkEnvironment(FrameworkSettings const& settings) noexcept
    : settings(settings), platform_shim(file_system_manager, settings.log_filter) {
    // Setup default path redirections

    platform_shim->set_elevated_privilege(settings.run_as_if_with_elevated_privleges);

#if TESTING_COMMON_UNIX_PLATFORMS
    if (!settings.home_env_var.empty()) env_var_home.set_new_value(settings.home_env_var);
#if !defined(__APPLE__)
    if (!settings.xdg_config_home_env_var.empty()) env_var_xdg_config_home.set_new_value(settings.xdg_config_home_env_var);
    if (!settings.xdg_config_dirs_env_var.empty()) env_var_xdg_config_dirs.set_new_value(settings.xdg_config_dirs_env_var);
    if (!settings.xdg_data_home_env_var.empty()) env_var_xdg_data_home.set_new_value(settings.xdg_data_home_env_var);
    if (!settings.xdg_data_dirs_env_var.empty()) env_var_xdg_data_dirs.set_new_value(settings.xdg_data_dirs_env_var);
#endif
#endif

    const std::array<std::pair<const char*, ManifestLocation>, 4> secured_redirection_map = {
        std::pair{"icd.d", ManifestLocation::driver},
        std::pair{"implicit_layer.d", ManifestLocation::implicit_layer},
        std::pair{"explicit_layer.d", ManifestLocation::explicit_layer},
        std::pair{"loader_settings.d", ManifestLocation::settings_location},
    };

    const std::array<std::pair<const char*, ManifestLocation>, 4> unsecured_redirection_map = {
        std::pair{"icd.d", ManifestLocation::unsecured_driver},
        std::pair{"implicit_layer.d", ManifestLocation::unsecured_implicit_layer},
        std::pair{"explicit_layer.d", ManifestLocation::unsecured_explicit_layer},
        std::pair{"loader_settings.d", ManifestLocation::unsecured_settings},
    };

#if TESTING_COMMON_UNIX_PLATFORMS && !defined(__APPLE__)
    // Always are searching SYSCONFDIR on unix (but not apple, which is handled separately)
    secure_manifest_base_location = SYSCONFDIR;

    for (auto const& [redirect, location] : secured_redirection_map) {
        file_system_manager.add_path_redirect(secure_manifest_base_location + "/vulkan/" + redirect, location);
    }

    // Determines which unsecure path should be used
    if (!settings.run_as_if_with_elevated_privleges) {
        if (!settings.xdg_config_home_env_var.empty()) {
            auto env_var_list = split_env_var_as_list(settings.xdg_config_home_env_var);
            unsecure_manifest_base_location = env_var_list.at(0);
            if (!env_var_list.empty()) {
                for (auto const& [redirect, location] : unsecured_redirection_map) {
                    file_system_manager.add_path_redirect(env_var_list.at(0) + "/vulkan/" + redirect, location);
                }
            }
        } else if (!settings.xdg_data_home_env_var.empty()) {
            auto env_var_list = split_env_var_as_list(settings.xdg_data_home_env_var);
            if (!env_var_list.empty()) {
                unsecure_manifest_base_location = env_var_list.at(0);
                for (auto const& [redirect, location] : unsecured_redirection_map) {
                    file_system_manager.add_path_redirect(env_var_list.at(0) + "/vulkan/" + redirect, location);
                }
            }
        } else {
            std::string home = settings.home_env_var;
            for (auto const& [redirect, location] : unsecured_redirection_map) {
                unsecure_manifest_base_location = home + "/.config";
                file_system_manager.add_path_redirect(home + "/.config/vulkan/" + redirect, location);
            }
        }
    }
#endif

#if defined(__APPLE__)
    // Since XDG env-var shouldn't ever be defined on apple, FALLBACK_DATA_DIRS takes over as the 'global' location to search, eg
    // /usr/local/share
    auto env_var_list = split_env_var_as_list(FALLBACK_DATA_DIRS);
    assert(env_var_list.size() > 0 && "FALLBACK_DATA_DIRS was set to an empty path");
    secure_manifest_base_location = env_var_list.at(0);
    for (auto const& [redirect, location] : secured_redirection_map) {
        file_system_manager.add_path_redirect(secure_manifest_base_location + "/vulkan/" + redirect, location);
    }

    std::string home = settings.home_env_var;
    unsecure_manifest_base_location = home + "/.config";
    for (auto const& [redirect, location] : unsecured_redirection_map) {
        file_system_manager.add_path_redirect(home + "/.config/vulkan/" + redirect, location);
    }

    // Necessary since bundles look in sub folders for manifests, not the test framework folder itself
    auto bundle_location = get_folder(ManifestLocation::macos_bundle).location();
    file_system_manager.add_path_redirect(bundle_location / "vulkan/icd.d", ManifestLocation::macos_bundle);
    file_system_manager.add_path_redirect(bundle_location / "vulkan/explicit_layer.d", ManifestLocation::macos_bundle);
    file_system_manager.add_path_redirect(bundle_location / "vulkan/implicit_layer.d", ManifestLocation::macos_bundle);
#endif

    platform_shim->is_finished_setup = true;
}

FrameworkEnvironment::~FrameworkEnvironment() {
    // This is necessary to prevent the folder manager from using dead memory during destruction.
    // What happens is that each folder manager tries to cleanup itself. Except, folders that were never called did not have
    // their DirEntry array's filled out. So when that folder calls delete_folder, which calls readdir, the shim tries to order
    // the files. Except, the list of files is in a object that is currently being destroyed.
    platform_shim->is_during_destruction = true;
}

ManifestLocation FrameworkEnvironment::map_discovery_type_to_location(ManifestDiscoveryType discovery_type,
                                                                      ManifestCategory category) {
    switch (discovery_type) {
        case (ManifestDiscoveryType::env_var):
            switch (category) {
                case (ManifestCategory::explicit_layer):
                    return ManifestLocation::explicit_layer_env_var;
                case (ManifestCategory::implicit_layer):
                    return ManifestLocation::implicit_layer_env_var;
                case (ManifestCategory::icd):
                    return ManifestLocation::driver_env_var;
                default:
                    return ManifestLocation::null;
            }
        case (ManifestDiscoveryType::add_env_var):
            switch (category) {
                case (ManifestCategory::explicit_layer):
                    return ManifestLocation::explicit_layer_add_env_var;
                case (ManifestCategory::implicit_layer):
                    return ManifestLocation::implicit_layer_add_env_var;
                case (ManifestCategory::icd):
                    return ManifestLocation::driver_env_var;
                default:
                    return ManifestLocation::null;
            }
#if defined(WIN32)
        case (ManifestDiscoveryType::windows_app_package):
            return ManifestLocation::windows_app_package;
#endif
        case (ManifestDiscoveryType::override_folder):
            return ManifestLocation::override_layer;
#if defined(__APPLE__)
        case (ManifestDiscoveryType::macos_bundle):
            return ManifestLocation::macos_bundle;
#endif
        case (ManifestDiscoveryType::unsecured_generic):
            switch (category) {
                case (ManifestCategory::explicit_layer):
                    return ManifestLocation::unsecured_explicit_layer;
                case (ManifestCategory::implicit_layer):
                    return ManifestLocation::unsecured_implicit_layer;
                case (ManifestCategory::icd):
                    return ManifestLocation::unsecured_driver;
                case (ManifestCategory::settings):
                    return ManifestLocation::unsecured_settings;
                default:
                    return ManifestLocation::null;
            }
        case (ManifestDiscoveryType::null_dir):
        case (ManifestDiscoveryType::none):
            return ManifestLocation::null;
        default:
        case (ManifestDiscoveryType::generic):
            switch (category) {
                case (ManifestCategory::explicit_layer):
                    return ManifestLocation::explicit_layer;
                case (ManifestCategory::implicit_layer):
                    return ManifestLocation::implicit_layer;
                case (ManifestCategory::icd):
                    return ManifestLocation::driver;
                case (ManifestCategory::settings):
                    return ManifestLocation::settings_location;
                default:
                    return ManifestLocation::null;
            }
    }
}

TestICD& FrameworkEnvironment::add_icd(std::filesystem::path const& path, ManifestOptions args, ManifestICD manifest) noexcept {
    manifest.set_lib_path(path);

    if (args.json_name.empty()) {
        args.json_name = "test_icd_" + std::to_string(icds.size()) + ".json";
    }

    size_t cur_icd_index = icds.size();
    auto& folder = get_folder(map_discovery_type_to_location(args.discovery_type, ManifestCategory::icd));

    if (args.discovery_type == ManifestDiscoveryType::env_var) {
        if (args.is_dir) {
            env_var_vk_icd_filenames.add_to_list(folder.location());
        } else {
            env_var_vk_icd_filenames.add_to_list(folder.location() / args.json_name);
        }
    } else if (args.discovery_type == ManifestDiscoveryType::add_env_var) {
        if (args.is_dir) {
            add_env_var_vk_icd_filenames.add_to_list(folder.location());
        } else {
            add_env_var_vk_icd_filenames.add_to_list(folder.location() / args.json_name);
        }
    }
    if (!args.is_fake) {
        std::filesystem::path new_lib_name = manifest.lib_path.stem();
        new_lib_name += "_";
        new_lib_name += std::to_string(cur_icd_index);
        new_lib_name += manifest.lib_path.extension();
        auto new_driver_location = folder.copy_file(manifest.lib_path, new_lib_name);

#if TESTING_COMMON_UNIX_PLATFORMS
        if (args.library_path_type == LibraryPathType::default_search_paths) {
            platform_shim->add_system_library(new_lib_name, new_driver_location);
        }
#endif
#if defined(WIN32)
        if (args.library_path_type == LibraryPathType::default_search_paths) {
            SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
            AddDllDirectory(new_driver_location.parent_path().native().c_str());
        }
#endif
        icds.push_back(TestICDHandle(new_driver_location));
        icds.back().reset();
        if (args.library_path_type == LibraryPathType::relative) {
            manifest.lib_path = std::filesystem::path(".") / new_lib_name;
        } else if (args.library_path_type == LibraryPathType::default_search_paths) {
            manifest.lib_path = new_lib_name;
        } else {
            manifest.lib_path = new_driver_location;
        }
    }
    if (args.discovery_type != ManifestDiscoveryType::none) {
        icds.back().manifest_path = folder.write_manifest(args.json_name, manifest.get_manifest_str());
        icds.back().shimmed_manifest_path = icds.back().manifest_path;

#if defined(WIN32)
        // only add the manifest to the registry if its a generic location (as if it was installed) - both system and user local
        if (args.discovery_type == ManifestDiscoveryType::generic) {
            platform_shim->add_manifest_to_registry(ManifestCategory::icd, icds.back().manifest_path);
        }
        if (args.discovery_type == ManifestDiscoveryType::unsecured_generic) {
            platform_shim->add_unsecured_manifest_to_registry(ManifestCategory::icd, icds.back().manifest_path);
        }
        if (args.discovery_type == ManifestDiscoveryType::windows_app_package) {
            platform_shim->set_app_package_path(folder.location());
        }
#endif
#if TESTING_COMMON_UNIX_PLATFORMS
        if (args.discovery_type == ManifestDiscoveryType::generic ||
            args.discovery_type == ManifestDiscoveryType::unsecured_generic) {
            icds.back().shimmed_manifest_path =
                file_system_manager.get_path_redirect_by_manifest_location(
                    args.discovery_type == ManifestDiscoveryType::unsecured_generic ? ManifestLocation::driver
                                                                                    : ManifestLocation::unsecured_driver) /
                args.json_name;
        }
#endif
    }
    return icds.back().get_test_binary();
}

void FrameworkEnvironment::add_implicit_layer(ManifestOptions args, ManifestLayer layer_manifest) noexcept {
    add_layer_impl(args, layer_manifest, ManifestCategory::implicit_layer);
}
void FrameworkEnvironment::add_explicit_layer(ManifestOptions args, ManifestLayer layer_manifest) noexcept {
    add_layer_impl(args, layer_manifest, ManifestCategory::explicit_layer);
}

void FrameworkEnvironment::add_layer_impl(ManifestOptions args, ManifestLayer manifest, ManifestCategory category) {
    if (args.json_name.empty()) {
        args.json_name = "test_layer_" + std::to_string(created_layer_count) + ".json";
        created_layer_count++;
    }

    auto& folder = get_folder(map_discovery_type_to_location(args.discovery_type, category));

    if (args.discovery_type == ManifestDiscoveryType::env_var || args.discovery_type == ManifestDiscoveryType::add_env_var) {
        std::filesystem::path path_to_add = folder.location();
        if (!args.is_dir) {
            path_to_add /= args.json_name;
        }

        EnvVarWrapper* env_var_to_use = nullptr;
        if (args.discovery_type == ManifestDiscoveryType::env_var && category == ManifestCategory::explicit_layer) {
            env_var_vk_layer_paths.add_to_list(path_to_add);
        } else if (args.discovery_type == ManifestDiscoveryType::env_var && category == ManifestCategory::implicit_layer) {
            env_var_vk_implicit_layer_paths.add_to_list(path_to_add);
        } else if (args.discovery_type == ManifestDiscoveryType::add_env_var && category == ManifestCategory::explicit_layer) {
            add_env_var_vk_layer_paths.add_to_list(path_to_add);
        } else if (args.discovery_type == ManifestDiscoveryType::add_env_var && category == ManifestCategory::implicit_layer) {
            add_env_var_vk_implicit_layer_paths.add_to_list(path_to_add);
        }
    }

    size_t new_layers_start = layers.size();
    for (auto& layer : manifest.layers) {
        if (!layer.lib_path.empty()) {
            std::filesystem::path new_lib_path = layer.lib_path.stem();
            new_lib_path += "_";
            new_lib_path += std::to_string(layers.size());
            new_lib_path += layer.lib_path.extension();

            auto new_layer_location = folder.copy_file(layer.lib_path, new_lib_path);

#if TESTING_COMMON_UNIX_PLATFORMS
            if (args.library_path_type == LibraryPathType::default_search_paths) {
                platform_shim->add_system_library(new_lib_path, new_layer_location);
            }
#endif
#if defined(WIN32)
            if (args.library_path_type == LibraryPathType::default_search_paths) {
                SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
                AddDllDirectory(new_layer_location.parent_path().native().c_str());
            }
#endif

            // Don't load the layer binary if using any of the wrap objects layers, since it doesn't export the same interface
            // functions
            if (!args.is_fake && layer.lib_path.stem().string().find(
                                     std::filesystem::path(TEST_LAYER_WRAP_OBJECTS).stem().string()) == std::string::npos) {
                layers.push_back(TestLayerHandle(new_layer_location));
                layers.back().reset();
            }
            if (args.library_path_type == LibraryPathType::relative) {
                layer.lib_path = std::filesystem::path(".") / new_lib_path;
            } else if (args.library_path_type == LibraryPathType::default_search_paths) {
                layer.lib_path = new_lib_path;
            } else {
                layer.lib_path = new_layer_location;
            }
        }
    }
    if (args.discovery_type != ManifestDiscoveryType::none) {
        // Write a manifest file to a folder as long as the discovery type isn't none
        auto layer_manifest_loc = folder.write_manifest(args.json_name, manifest.get_manifest_str());
#if defined(WIN32)
        // only add the manifest to the registry if its a generic location (as if it was installed) - both system and user local
        if (args.discovery_type == ManifestDiscoveryType::generic) {
            platform_shim->add_manifest_to_registry(category, layer_manifest_loc);
        }
        if (args.discovery_type == ManifestDiscoveryType::unsecured_generic) {
            platform_shim->add_unsecured_manifest_to_registry(category, layer_manifest_loc);
        }
        if (args.discovery_type == ManifestDiscoveryType::windows_app_package) {
            platform_shim->set_app_package_path(folder.location());
        }
#endif
        for (size_t i = new_layers_start; i < layers.size(); i++) {
            layers.at(i).manifest_path = layer_manifest_loc;
            layers.at(i).shimmed_manifest_path = layer_manifest_loc;
#if TESTING_COMMON_UNIX_PLATFORMS
            if (args.discovery_type == ManifestDiscoveryType::generic ||
                args.discovery_type == ManifestDiscoveryType::unsecured_generic) {
                ManifestLocation location = ManifestLocation::null;
                if (args.discovery_type == ManifestDiscoveryType::generic) {
                    location = (category == ManifestCategory::explicit_layer ? ManifestLocation::explicit_layer
                                                                             : ManifestLocation::implicit_layer);
                } else if (args.discovery_type == ManifestDiscoveryType::unsecured_generic) {
                    location = (category == ManifestCategory::explicit_layer ? ManifestLocation::unsecured_explicit_layer
                                                                             : ManifestLocation::unsecured_implicit_layer);
                }
                layers.at(i).shimmed_manifest_path =
                    file_system_manager.get_path_redirect_by_manifest_location(location) / args.json_name;
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
        if (!setting.driver_configurations.empty()) {
            writer.AddKeyedBool("additional_drivers_use_exclusively", setting.additional_drivers_use_exclusively);
            writer.StartKeyedArray("additional_drivers");
            for (const auto& driver : setting.driver_configurations) {
                writer.StartObject();
                writer.AddKeyedString("path", driver.path);
                writer.EndObject();
            }
            writer.EndArray();
        }
        if (!setting.device_configurations.empty()) {
            writer.StartKeyedArray("device_configurations");
            for (const auto& device : setting.device_configurations) {
                writer.StartObject();
                if (!device.deviceName.empty()) {
                    writer.AddKeyedString("deviceName", device.deviceName);
                }
                if (!device.driverName.empty()) {
                    writer.AddKeyedString("driverName", device.driverName);
                }
                writer.StartKeyedArray("deviceUUID");
                for (const auto& u : device.deviceUUID) {
                    writer.AddInteger(u);
                }
                writer.EndArray();

                writer.StartKeyedArray("driverUUID");
                for (const auto& u : device.driverUUID) {
                    writer.AddInteger(u);
                }
                writer.EndArray();
                writer.AddKeyedInteger("driverVersion", device.driverVersion);
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
void FrameworkEnvironment::write_settings_file(std::string const& file_contents, bool write_to_secure_location) {
    auto location = write_to_secure_location ? ManifestLocation::settings_location : ManifestLocation::unsecured_settings;
    auto out_path = get_folder(location).write_manifest("vk_loader_settings.json", file_contents);
#if defined(WIN32)
    if (write_to_secure_location) {
        platform_shim->hkey_local_machine_settings.clear();
        platform_shim->add_manifest_to_registry(ManifestCategory::settings, out_path);
    } else {
        platform_shim->hkey_current_user_settings.clear();
        platform_shim->add_unsecured_manifest_to_registry(ManifestCategory::settings, out_path);
    }
#endif
}
void FrameworkEnvironment::update_loader_settings(const LoaderSettings& settings, bool write_to_secure_location) noexcept {
    write_settings_file(get_loader_settings_file_contents(settings), write_to_secure_location);
}
void FrameworkEnvironment::remove_loader_settings() {
    get_folder(ManifestLocation::settings_location).remove("vk_loader_settings.json");
}
void FrameworkEnvironment::write_file_from_string(std::string const& source_string, ManifestCategory category,
                                                  ManifestLocation location, std::string const& file_name) {
    auto out_path = get_folder(location).write_manifest(file_name, source_string);

#if defined(WIN32)
    // Only writes to the hkey_local_machine registries, doesn't support hkey_current_user registries
    platform_shim->add_manifest_to_registry(category, out_path);
#endif
}
void FrameworkEnvironment::write_file_from_source(const char* source_file, ManifestCategory category, ManifestLocation location,
                                                  std::string const& file_name) {
    std::fstream file{source_file, std::ios_base::in};
    ASSERT_TRUE(file.is_open());
    std::stringstream file_stream;
    file_stream << file.rdbuf();
    write_file_from_string(file_stream.str(), category, location, file_name);
}

TestICD& FrameworkEnvironment::get_test_icd(size_t index) noexcept { return icds[index].get_test_binary(); }
TestICD& FrameworkEnvironment::reset_icd(size_t index) noexcept { return icds[index].reset(); }
std::filesystem::path FrameworkEnvironment::get_test_icd_path(size_t index) noexcept { return icds[index].get_full_path(); }
std::filesystem::path FrameworkEnvironment::get_icd_manifest_path(size_t index) noexcept { return icds[index].get_manifest_path(); }
std::filesystem::path FrameworkEnvironment::get_shimmed_icd_manifest_path(size_t index) noexcept {
    return icds[index].get_shimmed_manifest_path();
}

TestLayer& FrameworkEnvironment::get_test_layer(size_t index) noexcept { return layers[index].get_test_binary(); }
TestLayer& FrameworkEnvironment::reset_layer(size_t index) noexcept { return layers[index].reset(); }
std::filesystem::path FrameworkEnvironment::get_test_layer_path(size_t index) noexcept { return layers[index].get_full_path(); }
std::filesystem::path FrameworkEnvironment::get_layer_manifest_path(size_t index) noexcept {
    return layers[index].get_manifest_path();
}
std::filesystem::path FrameworkEnvironment::get_shimmed_layer_manifest_path(size_t index) noexcept {
    return layers[index].get_shimmed_manifest_path();
}

fs::Folder& FrameworkEnvironment::get_folder(ManifestLocation location) noexcept {
    return file_system_manager.get_folder(location);
}
fs::Folder const& FrameworkEnvironment::get_folder(ManifestLocation location) const noexcept {
    return file_system_manager.get_folder(location);
}
#if defined(__APPLE__)
void FrameworkEnvironment::setup_macos_bundle() noexcept {
    platform_shim->bundle_contents = file_system_manager.get_folder(ManifestLocation::macos_bundle).location();
}
#endif

void FrameworkEnvironment::add_symlink(ManifestLocation location, std::filesystem::path const& target,
                                       std::filesystem::path const& link_name) {
    auto symlinked_path = get_folder(location).add_symlink(target, link_name);
    file_system_manager.add_path_redirect(symlinked_path, location);
}

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
    return inst.instance_functions.vkCreateDebugReportCallbackEXT(inst.inst, &create_info, nullptr, &callback);
}

extern "C" {
void __ubsan_on_report() { FAIL() << "Encountered an undefined behavior sanitizer error"; }
void __asan_on_error() { FAIL() << "Encountered an address sanitizer error"; }
void __tsan_on_report() { FAIL() << "Encountered a thread sanitizer error"; }
}  // extern "C"
