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

#ifndef TNT_FILAMENT_DETAILS_FEATUREFLAGMANAGER_H
#define TNT_FILAMENT_DETAILS_FEATUREFLAGMANAGER_H

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <array>
#include <optional>
#include <string_view>

// We have added correctness assertions that breaks clients' projects. We add this define to allow
// for the client's to address these assertions at a more gradual pace.
#if defined(FILAMENT_RELAXED_CORRECTNESS_ASSERTIONS)
#define CORRECTNESS_ASSERTION_DEFAULT false
#else
#define CORRECTNESS_ASSERTION_DEFAULT true
#endif

namespace utils {

class FeatureFlagManager {
public:

    struct FeatureFlag {
        char const* UTILS_NONNULL name;
        char const* UTILS_NONNULL description;
        bool const* UTILS_NONNULL value;
        bool constant = true;
    };

    struct {
        struct {
            struct {
                bool use_1d_lut = false;
            } color_grading;
            struct {
                bool use_shadow_atlas = false;
            } shadows;
            struct {
                // TODO: clean-up the following flags (equivalent to setting them to true) when
                // clients have addressed their usages.
                bool assert_material_instance_in_use = CORRECTNESS_ASSERTION_DEFAULT;
                bool assert_destroy_material_before_material_instance = CORRECTNESS_ASSERTION_DEFAULT;
                bool assert_vertex_buffer_count_exceeds_8 = CORRECTNESS_ASSERTION_DEFAULT;
                bool assert_vertex_buffer_attribute_stride_mult_of_4 = CORRECTNESS_ASSERTION_DEFAULT;
                bool assert_material_instance_texture_descriptor_set_compatible = CORRECTNESS_ASSERTION_DEFAULT;
                bool assert_texture_can_generate_mipmap = CORRECTNESS_ASSERTION_DEFAULT;
            } debug;
            struct {
                bool disable_gpu_frame_complete_metric = true;
            } frame_info;
            // Automatically requests a frame skip when the CPU gets too much ahead of the display. This can prevent
            // stalls on Android, e.g. when running out buffers, but this also keeps the expected latency in check.
            bool skip_frame_when_cpu_ahead_of_display = false;
            bool enable_program_cache = false;
        } engine;
        struct {
            struct {
                bool assert_native_window_is_valid = false;
            } opengl;
            struct {
                // In certain GPU drivers, graphics pipelines are cached based on a subset of their
                // parameters. In those cases, we can create fake pipelines ahead of time to ensure
                // a cache hit when creating graphics pipelines at draw time, eliminating hitching.
                bool enable_pipeline_cache_prewarming = false;
                // On Unified Memory Architecture device, it is possible to bypass using the staging
                // buffer. This is an experimental feature that still needs to be implemented fully
                // before it can be fully enabled.
                bool enable_staging_buffer_bypass = false;
                // A client requires that swapchain be acquired in makeCurrent() (b/476144715).
                bool enable_acquire_swapchain_in_make_current = false;
            } vulkan;
            bool disable_parallel_shader_compile = false;
            bool disable_amortized_shader_compile = true;
            bool disable_handle_use_after_free_check = false;
            bool disable_heap_handle_tags = true; // FIXME: this should be false
            bool enable_asynchronous_operation = false;
        } backend;
        struct {
            bool check_crc32_after_loading = false;
            bool enable_material_instance_uniform_batching = false;
            bool enable_fog_as_postprocess = false;
        } material;
    } features;

public:
    FeatureFlagManager();
    ~FeatureFlagManager() noexcept;

    FeatureFlagManager(FeatureFlagManager const&) = default;
    FeatureFlagManager& operator=(FeatureFlagManager const&) = default;

    std::optional<bool> getFeatureFlag(char const* UTILS_NONNULL name) const noexcept;

protected:
    Slice<const FeatureFlag> getFeatureFlags() const noexcept {
        return { mFeatures.data(), mFeatures.size() };
    }
    bool setFeatureFlag(char const* UTILS_NONNULL name, bool value) noexcept;
    bool* UTILS_NULLABLE getFeatureFlagPtr(std::string_view name, bool allowConstant = false) const noexcept;

private:
    std::array<FeatureFlag, sizeof(features)> mFeatures;
};

} // namespace utils

#endif // TNT_FILAMENT_DETAILS_FEATUREFLAGMANAGER_H
