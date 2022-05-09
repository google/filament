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
    constexpr uint8_t PER_VIEW                   = 0;    // uniforms/samplers updated per view
    constexpr uint8_t PER_RENDERABLE             = 1;    // uniforms/samplers updated per renderable
    constexpr uint8_t PER_RENDERABLE_BONES       = 2;    // bones data, per renderable
    constexpr uint8_t PER_RENDERABLE_MORPHING    = 3;    // morphing uniform/sampler updated per render primitive
    constexpr uint8_t LIGHTS                     = 4;    // lights data array
    constexpr uint8_t SHADOW                     = 5;    // punctual shadow data
    constexpr uint8_t FROXEL_RECORDS             = 6;
    constexpr uint8_t PER_MATERIAL_INSTANCE      = 7;    // uniforms/samplers updates per material
    constexpr uint8_t COUNT                      = 8;
    // These are limited by CONFIG_BINDING_COUNT (currently 12)
}

static_assert(BindingPoints::COUNT <= backend::CONFIG_BINDING_COUNT);

static_assert(BindingPoints::PER_MATERIAL_INSTANCE == BindingPoints::COUNT - 1,
        "Dynamically sized sampler buffer must be the last binding point.");

// This value is limited by UBO size, ES3.0 only guarantees 16 KiB.
// Values <= 256, use less CPU and GPU resources.
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_INDEX = CONFIG_MAX_LIGHT_COUNT - 1;

// The maximum number of spot lights in a scene that can cast shadows.
// There is currently a limit to 14 spot shadow due to how we store the culling result
// (see View.h).
constexpr size_t CONFIG_MAX_SHADOW_CASTING_SPOTS = 14;

// The maximum number of shadow cascades that can be used for directional lights.
constexpr size_t CONFIG_MAX_SHADOW_CASCADES = 4;

// This value is also limited by UBO size, ES3.0 only guarantees 16 KiB.
// We store 64 bytes per bone.
constexpr size_t CONFIG_MAX_BONE_COUNT = 256;

// The maximum number of morph target count.
// This value is limited by ES3.0, ES3.0 only guarantees 256 layers in an array texture.
constexpr size_t CONFIG_MAX_MORPH_TARGET_COUNT = 256;

} // namespace filament

#endif // TNT_FILAMENT_ENGINE_ENUM_H
