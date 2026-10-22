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

#include "manifest_builders.h"

#include "json_writer.h"

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

std::string version_to_string(uint32_t version) {
    std::string out = std::to_string(VK_API_VERSION_MAJOR(version)) + "." + std::to_string(VK_API_VERSION_MINOR(version)) + "." +
                      std::to_string(VK_API_VERSION_PATCH(version));
    if (VK_API_VERSION_VARIANT(version) != 0) out += std::to_string(VK_API_VERSION_VARIANT(version)) + "." + out;
    return out;
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

void ManifestLayer::LayerDescription::FunctionOverride::get_manifest_str(JsonWriter& writer) const {
    writer.AddKeyedString(vk_func, override_name);
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
    name.copy(properties.layerName, VK_MAX_EXTENSION_NAME_SIZE);
    description.copy(properties.description, VK_MAX_EXTENSION_NAME_SIZE);
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
