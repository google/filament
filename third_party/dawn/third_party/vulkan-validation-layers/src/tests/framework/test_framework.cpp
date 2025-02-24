/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include "test_framework.h"
#include "vk_layer_config.h"
#include "generated/vk_function_pointers.h"
#include <glslang/Public/ShaderLang.h>
#include CONFIG_HEADER_FILE
#include <filesystem>
#include <cmath>
#include <cstdarg>

struct SwapchainBuffers {
    VkImage image;
    VkCommandBuffer cmd;
    VkImageView view;
};

#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
// NOTE: Android doesn't use environment variables like desktop does!
//
// Certain VK_* environment variables accept lists.
// Return a vector of std::string containing each member in the list.
//
// EX input:
//  export VK_DRIVER_FILES=/intel.json:/amd.json
//  set VK_DRIVER_FILES=\nvidia.json;\mesa.json
static std::vector<std::string> GetVkEnvironmentVariable(const char *env_var) {
    const std::string str = GetEnvironment(env_var);
    if (str.empty()) {
        return {};
    }

    // Loader uses standard OS path separators per platform
    constexpr char delimiter =
#ifdef _WIN32
        ';';
#else
        ':';
#endif

    std::vector<std::string> items;
    std::string::size_type start = 0;

    std::string::size_type pos = str.find_first_of(delimiter, start);
    std::string::size_type length = pos;
    while (pos != std::string::npos) {
        // emplace uses std::substr which takes length from start
        items.emplace_back(str, start, length);

        start = pos + 1;

        pos = str.find_first_of(delimiter, start);

        length = pos - start;
    }
    items.emplace_back(str, start);

    return items;
}

static void CheckAndSetEnvironmentVariables() {
    for (const char *env_var : {"VK_DRIVER_FILES", "VK_ICD_FILENAMES"}) {
        const std::vector<std::string> driver_files = GetVkEnvironmentVariable(env_var);
        for (const std::string &driver_file : driver_files) {
            const std::filesystem::path icd_file(driver_file);
            // TODO: Error check relative paths (platform dependent)
            if (icd_file.is_relative()) {
                continue;
            }
            std::string user_provided;
            user_provided += "\n\n";
            user_provided += env_var;
            user_provided += " = ";
            user_provided += driver_file;
            if (std::filesystem::is_directory(icd_file)) {
                if (!std::filesystem::exists(icd_file)) {
                    std::cerr << "Invalid " << env_var << "! Directory doesn't exist!" << user_provided << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                bool contains_json = false;
                for (auto const &dir_entry : std::filesystem::directory_iterator{icd_file}) {
                    if (dir_entry.path().extension() == ".json") {
                        contains_json = true;
                    }
                }
                if (!contains_json) {
                    std::cerr << "Invalid " << env_var << "! " << env_var << " must contain a json file!" << user_provided
                              << std::endl;
                    std::exit(EXIT_FAILURE);
                }

            } else {
                if (!std::filesystem::exists(icd_file)) {
                    std::cerr << "Invalid " << env_var << "! File doesn't exist!" << user_provided << std::endl;
                    std::exit(EXIT_FAILURE);
                }

                if (icd_file.extension() != ".json") {
                    std::cerr << "Invalid " << env_var << "! " << env_var << " must be a json file!" << user_provided << std::endl;
                    std::exit(EXIT_FAILURE);
                }
            }
        }
    }

    bool found_json = false;
    bool vk_layer_env_vars_present = false;
    std::stringstream error_log;  // Build up error log in case the validation json cannot be found
    for (const char *env_var : {"VK_LAYER_PATH", "VK_ADD_LAYER_PATH"}) {
        const std::vector<std::string> vk_layer_paths = GetVkEnvironmentVariable(env_var);
        if (!vk_layer_paths.empty()) {
            vk_layer_env_vars_present = true;
        }
        for (const std::string &vk_layer_path : vk_layer_paths) {
            const std::filesystem::path layer_path(vk_layer_path);

            if (!std::filesystem::exists(layer_path)) {
                error_log << "Invalid " << env_var << "! " << layer_path << " doesn't exist." << std::endl;
                continue;
            }

            if (std::filesystem::is_directory(layer_path)) {
                for (auto const &dir_entry : std::filesystem::directory_iterator{layer_path}) {
                    if (dir_entry.path().filename() == "VkLayer_khronos_validation.json") {
                        if (std::filesystem::exists(dir_entry)) {
                            found_json = true;
                            break;
                        } else {
                            error_log << "Invalid " << env_var << "! " << dir_entry << " doen not exist!" << std::endl;
                        }
                    }
                }
                if (!found_json) {
                    error_log << "Invalid " << env_var << "! " << layer_path
                              << " is a directory but doesn't contain a VkLayer_khronos_validation.json file!" << std::endl;
                }
            } else {
                if (layer_path.filename() == "VkLayer_khronos_validation.json") {
                    found_json = true;
                    break;
                } else {
                    error_log << "Invalid " << env_var << "! The filename of path" << layer_path
                              << " is not VkLayer_khronos_validation.json!" << std::endl;
                }
            }
            if (found_json) {
                break;
            }
        }
    }

    if (!found_json) {
        if (vk_layer_env_vars_present) {
            std::cerr << error_log.str() << std::endl;
            std::cerr << "Automatically setting VK_LAYER_PATH to " << VALIDATION_LAYERS_BUILD_PATH << std::endl;
        }
        SetEnvironment("VK_LAYER_PATH", VALIDATION_LAYERS_BUILD_PATH);
    }
}
#endif

// Set up environment for GLSL compiler
// Must be done once per process
void TestEnvironment::SetUp() {
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
    // Helps ensure common developer environment variables are set correctly
    CheckAndSetEnvironmentVariables();
#endif

    // Initialize GLSL to SPV compiler utility
    glslang::InitializeProcess();

    vk::InitCore("vulkan");
}

void TestEnvironment::TearDown() { glslang::FinalizeProcess(); }

void VkTestFramework::InitArgs(int *argc, char *argv[]) {
    for (int i = 1; i < *argc; ++i) {
        const std::string_view current_argument = argv[i];
        if (current_argument == "--print-vu") {
            m_print_vu = true;
        } else if (current_argument == "--syncval-disable-core") {
            m_syncval_disable_core = true;
        } else if (current_argument == "--gpuav-disable-core") {
            m_gpuav_disable_core = true;
        } else if (current_argument == "--device-index" && ((i + 1) < *argc)) {
            m_phys_device_index = std::atoi(argv[++i]);
        } else if ((current_argument == "--help") || (current_argument == "-h")) {
            printf("\nOther options:\n");
            printf(
                "\t--print-vu\n"
                "\t\tPrints all VUs - help see what new VU will look like.\n");
            printf(
                "\t--syncval-disable-core\n"
                "\t\tDisable core validation when running syncval tests.\n"
                "\t\tBy default both CoreChecks and syncval validation is enabled.\n");
            printf(
                "\t--gpuav-disable-core\n"
                "\t\tDisable core validation when running gpu-av tests.\n"
                "\t\tBy default both CoreChecks and gpu-av is enabled.\n");
            printf(
                "\t--device-index <physical device index>\n"
                "\t\tIndex into VkPhysicalDevice array returned from vkEnumeratePhysicalDevices.\n"
                "\t\tThe default behavior is to automatically choose \"the most reasonable device.\"\n"
                "\t\tAn invalid index (i.e., outside the range [0, *pPhysicalDeviceCount)) will result in the default "
                "behavior\n");
            exit(0);
        } else {
            printf("\nUnrecognized option: %s\n", argv[i]);
            printf("\nUse --help or -h for option list.\n");
            exit(0);
        }
    }
}

void VkTestFramework::Finish() {}
