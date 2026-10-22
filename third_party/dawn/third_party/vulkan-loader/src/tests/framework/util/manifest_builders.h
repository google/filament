/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include <vulkan/vulkan_core.h>

#include "builder_defines.h"

// Forward declaration since we take it by reference only
class JsonWriter;

struct ManifestVersion {
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, major, 1)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, minor, 0)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, patch, 0)

    std::string get_version_str() const noexcept {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

// ManifestICD builder
struct ManifestICD {
    BUILDER_VALUE(ManifestVersion, file_format_version)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, api_version, VK_API_VERSION_1_0)
    BUILDER_VALUE(std::filesystem::path, lib_path)
    BUILDER_VALUE(bool, is_portability_driver)
    BUILDER_VALUE(std::string, library_arch)
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
            BUILDER_VALUE(std::string, vk_func)
            BUILDER_VALUE(std::string, override_name)

            void get_manifest_str(JsonWriter& writer) const;
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
        BUILDER_VALUE(std::string, name)
        BUILDER_VALUE_WITH_DEFAULT(Type, type, Type::INSTANCE)
        BUILDER_VALUE(std::filesystem::path, lib_path)
        BUILDER_VALUE_WITH_DEFAULT(uint32_t, api_version, VK_API_VERSION_1_0)
        BUILDER_VALUE(uint32_t, implementation_version)
        BUILDER_VALUE(std::string, description)
        BUILDER_VECTOR(FunctionOverride, functions, function)
        BUILDER_VECTOR(Extension, instance_extensions, instance_extension)
        BUILDER_VECTOR(Extension, device_extensions, device_extension)
        BUILDER_VALUE(std::string, enable_environment)
        BUILDER_VALUE(std::string, disable_environment)
        BUILDER_VECTOR(std::string, component_layers, component_layer)
        BUILDER_VECTOR(std::string, blacklisted_layers, blacklisted_layer)
        BUILDER_VECTOR(std::filesystem::path, override_paths, override_path)
        BUILDER_VECTOR(FunctionOverride, pre_instance_functions, pre_instance_function)
        BUILDER_VECTOR(std::string, app_keys, app_key)
        BUILDER_VALUE(std::string, library_arch)

        void get_manifest_str(JsonWriter& writer) const;
        VkLayerProperties get_layer_properties() const;
    };
    BUILDER_VALUE(ManifestVersion, file_format_version)
    BUILDER_VECTOR(LayerDescription, layers, layer)

    std::string get_manifest_str() const;
};
