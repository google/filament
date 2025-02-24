
/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
#pragma once
#include <cstdint>
// Default values for those settings should match layers/VkLayer_khronos_validation.json.in

struct GpuAVSettings {
    bool warn_on_robust_oob = true;
    uint32_t max_bda_in_use = 10000;
    bool select_instrumented_shaders = false;

    bool validate_indirect_draws_buffers = true;
    bool validate_indirect_dispatches_buffers = true;
    bool validate_indirect_trace_rays_buffers = true;
    bool validate_buffer_copies = true;
    bool validate_index_buffers = true;

    // Currently turned of due to some false positives still observed
    bool validate_image_layout = false;

    bool vma_linear_output = true;

    bool debug_validate_instrumented_shaders = false;
    bool debug_dump_instrumented_shaders = false;
    uint32_t debug_max_instrumentations_count = 0;  // zero is same as "unlimited"
    bool debug_print_instrumentation_info = false;

    // Note - even though DebugPrintf basically fits in here, from the user point of view they are different and that is reflected
    // in the settings (which are reflected in VkConfig). To make our lives easier, we just make these settings with the hierarchy
    // of the settings exposed
    struct ShaderInstrumentation {
        bool descriptor_checks = true;
        bool buffer_device_address = true;
        bool ray_query = true;
        bool post_process_descriptor_index = true;
        bool vertex_attribute_fetch_oob = true;
    } shader_instrumentation;

    bool IsShaderInstrumentationEnabled() const {
        return shader_instrumentation.descriptor_checks || shader_instrumentation.buffer_device_address ||
               shader_instrumentation.ray_query || shader_instrumentation.post_process_descriptor_index ||
               shader_instrumentation.vertex_attribute_fetch_oob;
    }
    bool IsSpirvModified() const { return IsShaderInstrumentationEnabled() || debug_printf_enabled; }

    // Also disables shader caching and select shader instrumentation
    void DisableShaderInstrumentationAndOptions() {
        shader_instrumentation.descriptor_checks = false;
        shader_instrumentation.buffer_device_address = false;
        shader_instrumentation.ray_query = false;
        shader_instrumentation.post_process_descriptor_index = false;
        shader_instrumentation.vertex_attribute_fetch_oob = false;
        // Because of this setting, cannot really have an "enabled" parameter to pass to this method
        select_instrumented_shaders = false;
    }
    bool IsBufferValidationEnabled() const {
        return validate_indirect_draws_buffers || validate_indirect_dispatches_buffers || validate_indirect_trace_rays_buffers ||
               validate_buffer_copies || validate_index_buffers;
    }
    void SetBufferValidationEnabled(bool enabled) {
        validate_indirect_draws_buffers = enabled;
        validate_indirect_dispatches_buffers = enabled;
        validate_indirect_trace_rays_buffers = enabled;
        validate_buffer_copies = enabled;
        validate_index_buffers = enabled;
    }

    // For people who are using VkValidationFeatureEnableEXT to set only DebugPrintf (and want the rest of GPU-AV off)
    bool debug_printf_only = false;
    void SetOnlyDebugPrintf() {
        DisableShaderInstrumentationAndOptions();
        SetBufferValidationEnabled(false);

        // Turn on the minmal settings for DebugPrintf
        debug_printf_enabled = true;
        debug_printf_only = true;
    }

    bool debug_printf_enabled = false;
    bool debug_printf_to_stdout = false;
    bool debug_printf_verbose = false;
    uint32_t debug_printf_buffer_size = 1024;
};