/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <private/utils/FeatureFlagManager.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/Slice.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

#include <stdlib.h>
#include <stdio.h>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

namespace utils {

namespace {

#ifdef __ANDROID__
std::string_view getFeatureFlagFromEnvironment(char const* const name, std::array<char, 128>& storage) {
    // since API 26 there is no limit to the property name
    char propertyName[128];
    snprintf(propertyName, 128, "debug.%s", name);
    int const length = __system_property_get(propertyName, storage.data());
    if (length > 0) {
        return { storage.data(), size_t(length) };
    }
    return {};
}
#else
std::string_view getFeatureFlagFromEnvironment(char const* const name, std::array<char, 128>&) {
    char const* const env = getenv(name);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return env ? std::string_view{env} : std::string_view{};
}
#endif

void overrideFeatureDefaults(utils::Slice<FeatureFlagManager::FeatureFlag> const& features) {
    std::array<char, 128> storage;
    UTILS_NOUNROLL
    for (auto const& feature : features) {
        std::string_view const value = getFeatureFlagFromEnvironment(feature.name, storage);
        if (!value.empty()) {
            if (value == "1" || value == "true") {
                *const_cast<bool*>(feature.value) = true;
            } else if (value == "0" || value == "false") {
                *const_cast<bool*>(feature.value) = false;
            }
            DLOG(INFO) << "overriding " << feature.name << " to " << *feature.value;
        }
    }
}

} // anonymous

FeatureFlagManager::FeatureFlagManager() : mFeatures{{
        { "backend.disable_parallel_shader_compile",
          "Disable parallel shader compilation in GL and Metal backends.",
          &features.backend.disable_parallel_shader_compile },
        { "backend.disable_amortized_shader_compile",
          "Disable amortized shader compilation in GL backend.",
          &features.backend.disable_amortized_shader_compile },
        { "backend.disable_handle_use_after_free_check",
          "Disable Handle<> use-after-free checks.",
          &features.backend.disable_handle_use_after_free_check },
        { "backend.disable_heap_handle_tags",
          "Disable Handle<> tags for heap-allocated handles.",
          &features.backend.disable_heap_handle_tags },
        { "backend.enable_asynchronous_operation",
          "Enable asynchronous operation for resource management.",
          &features.backend.enable_asynchronous_operation },
        { "backend.opengl.assert_native_window_is_valid",
          "Asserts that the ANativeWindow is valid when rendering starts.",
          &features.backend.opengl.assert_native_window_is_valid },
        { "engine.color_grading.use_1d_lut",
          "Uses a 1D LUT for color grading.",
          &features.engine.color_grading.use_1d_lut, false },
        { "engine.shadows.use_shadow_atlas",
          "Uses an array of atlases to store shadow maps.",
          &features.engine.shadows.use_shadow_atlas, false },
        { "engine.debug.assert_material_instance_in_use",
          "Assert when a MaterialInstance is destroyed while it is in use by RenderableManager.",
          &features.engine.debug.assert_material_instance_in_use, false },
        { "engine.debug.assert_destroy_material_before_material_instance",
          "Assert when a Material is destroyed but its instances are still alive.",
          &features.engine.debug.assert_destroy_material_before_material_instance, false },
        { "engine.debug.assert_vertex_buffer_count_exceeds_8",
          "Assert when a client's number of buffers for a VertexBuffer exceeds 8.",
          &features.engine.debug.assert_vertex_buffer_count_exceeds_8, false },
        { "engine.debug.assert_vertex_buffer_attribute_stride_mult_of_4",
          "Assert that the attribute stride of a vertex buffer is a multiple of 4.",
          &features.engine.debug.assert_vertex_buffer_attribute_stride_mult_of_4, false },
        { "backend.vulkan.enable_pipeline_cache_prewarming",
          "Enables an experimental approach to parallel shader compilation on Vulkan.",
          &features.backend.vulkan.enable_pipeline_cache_prewarming, false },
        { "backend.vulkan.enable_staging_buffer_bypass",
          "vulkan: enable a staging bypass logic for unified memory architecture.",
          &features.backend.vulkan.enable_staging_buffer_bypass, false },
        { "engine.debug.assert_material_instance_texture_descriptor_set_compatible",
          "Assert that the textures in a material instance are compatible with descriptor set.",
          &features.engine.debug.assert_material_instance_texture_descriptor_set_compatible, false },
        { "engine.debug.assert_texture_can_generate_mipmap",
          "Assert if a texture has the correct usage set for generating mipmaps.",
          &features.engine.debug.assert_texture_can_generate_mipmap, false },
        { "material.check_crc32_after_loading",
          "Verify the checksum of package data when a material is loaded.",
          &features.material.check_crc32_after_loading, false },
        { "material.enable_material_instance_uniform_batching",
          "Make all MaterialInstances share a common large uniform buffer and use sub-allocations within it.",
          &features.material.enable_material_instance_uniform_batching },
        { "engine.frame_info.disable_gpu_complete_metric",
          "Disable Renderer::FrameInfo::gpuFrameComplete reporting",
          &features.engine.frame_info.disable_gpu_frame_complete_metric },
}} {
    overrideFeatureDefaults({ mFeatures.data(), mFeatures.size() });
}

FeatureFlagManager::~FeatureFlagManager() noexcept = default;

bool FeatureFlagManager::setFeatureFlag(char const* name, bool const value) noexcept {
    auto* const p = getFeatureFlagPtr(name);
    if (p) {
        *p = value;
    }
    return p != nullptr;
}

std::optional<bool> FeatureFlagManager::getFeatureFlag(char const* name) const noexcept {
    if (auto* const p = getFeatureFlagPtr(name, true)) {
        return *p;
    }
    return std::nullopt;
}

bool* FeatureFlagManager::getFeatureFlagPtr(std::string_view name, bool const allowConstant) const noexcept {
    auto const pos = std::find_if(mFeatures.begin(), mFeatures.end(),
            [name](FeatureFlag const& entry) {
                return name == entry.name;
            });

    return (pos != mFeatures.end() && (!pos->constant || allowConstant)) ?
           const_cast<bool*>(pos->value) : nullptr;
}

} // namespace filament
