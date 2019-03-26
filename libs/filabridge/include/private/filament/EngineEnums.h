/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_ENGINE_ENUM_H
#define TNT_FILAMENT_ENGINE_ENUM_H

#include <stddef.h>
#include <stdint.h>

namespace filament {

static constexpr size_t POST_PROCESS_STAGES_COUNT = 4;
enum class PostProcessStage : uint8_t {
    TONE_MAPPING_OPAQUE,           // Tone mapping post-process
    TONE_MAPPING_TRANSLUCENT,      // Tone mapping post-process
    ANTI_ALIASING_OPAQUE,          // Anti-aliasing stage
    ANTI_ALIASING_TRANSLUCENT,     // Anti-aliasing stage
};

// Binding points for uniform buffers and sampler buffers.
// Effectively, these are just names.
namespace BindingPoints {
    constexpr uint8_t PER_VIEW                = 0;    // uniforms/samplers updated per view
    constexpr uint8_t PER_RENDERABLE          = 1;    // uniforms/samplers updated per renderable
    constexpr uint8_t PER_RENDERABLE_BONES    = 2;    // bones data, per renderable
    constexpr uint8_t LIGHTS                  = 3;    // lights data array
    constexpr uint8_t POST_PROCESS            = 4;    // samplers for the post process pass
    constexpr uint8_t PER_MATERIAL_INSTANCE   = 5;    // uniforms/samplers updates per material
    constexpr uint8_t COUNT                   = 6;
    // These are limited by Program::UNIFORM_BINDING_COUNT (currently 6)
}

static_assert(BindingPoints::PER_MATERIAL_INSTANCE == BindingPoints::COUNT - 1,
        "Dynamically sized sampler buffer must be the last binding point.");

// This value is limited by UBO size, ES3.0 only guarantees 16 KiB.
// Values <= 256, use less CPU and GPU resources.
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_INDEX = CONFIG_MAX_LIGHT_COUNT - 1;

// This value is also limited by UBO size, ES3.0 only guarantees 16 KiB.
// We store 64 bytes per bone.
constexpr size_t CONFIG_MAX_BONE_COUNT = 256;

// TODO This should be injected by the engine as a define of the shader.
static constexpr bool   CONFIG_IBL_RGBM  = true;
static constexpr size_t CONFIG_IBL_SIZE  = 256;

} // namespace filament

#endif // TNT_FILAMENT_driver/EngineEnums.h
