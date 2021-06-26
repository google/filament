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

#include <backend/DriverEnums.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

static constexpr size_t POST_PROCESS_VARIANT_COUNT = 2;
enum class PostProcessVariant : uint8_t {
    OPAQUE,
    TRANSLUCENT
};

// Binding points for uniform buffers and sampler buffers.
// Effectively, these are just names.
namespace BindingPoints {
    constexpr uint8_t PER_VIEW                = 0;    // uniforms/samplers updated per view
    constexpr uint8_t PER_RENDERABLE          = 1;    // uniforms/samplers updated per renderable
    constexpr uint8_t PER_RENDERABLE_BONES    = 2;    // bones data, per renderable
    constexpr uint8_t LIGHTS                  = 3;    // lights data array
    constexpr uint8_t SHADOW                  = 4;    // punctual shadow data
    constexpr uint8_t FROXEL_RECORDS          = 5;
    constexpr uint8_t PER_MATERIAL_INSTANCE   = 6;    // uniforms/samplers updates per material
    constexpr uint8_t COUNT                   = 7;
    // These are limited by Program::UNIFORM_BINDING_COUNT (currently 8)
}

static_assert(BindingPoints::COUNT <= backend::CONFIG_BINDING_COUNT);

static_assert(BindingPoints::PER_MATERIAL_INSTANCE == BindingPoints::COUNT - 1,
        "Dynamically sized sampler buffer must be the last binding point.");

// This value is limited by UBO size, ES3.0 only guarantees 16 KiB.
// Values <= 256, use less CPU and GPU resources.
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_INDEX = CONFIG_MAX_LIGHT_COUNT - 1;

// The maximum number of spot lights in a scene that can cast shadows.
// Light space coordinates are computed in the vertex shader and interpolated across fragments.
// Thus, each additional shadow-casting spot light adds 4 additional varying components. Higher
// values may cause the number of varyings to exceed the driver limit.
constexpr size_t CONFIG_MAX_SHADOW_CASTING_SPOTS = 2;

// The maximum number of shadow cascades that can be used for directional lights.
constexpr size_t CONFIG_MAX_SHADOW_CASCADES = 4;

// This value is also limited by UBO size, ES3.0 only guarantees 16 KiB.
// We store 64 bytes per bone.
constexpr size_t CONFIG_MAX_BONE_COUNT = 256;

} // namespace filament

#endif // TNT_FILAMENT_driver/EngineEnums.h
