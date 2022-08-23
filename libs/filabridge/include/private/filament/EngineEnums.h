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

#include <utils/BitmaskEnum.h>

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
enum class BindingPoints : uint8_t {
    PER_VIEW                   = 0,    // uniforms/samplers updated per view
    PER_RENDERABLE             = 1,    // uniforms/samplers updated per renderable
    PER_RENDERABLE_BONES       = 2,    // bones data, per renderable
    PER_RENDERABLE_MORPHING    = 3,    // morphing uniform/sampler updated per render primitive
    LIGHTS                     = 4,    // lights data array
    SHADOW                     = 5,    // punctual shadow data
    FROXEL_RECORDS             = 6,
    PER_MATERIAL_INSTANCE      = 7,    // uniforms/samplers updates per material
    // Update utils::Enum::count<>() below when adding values here
    // These are limited by CONFIG_BINDING_COUNT (currently 12)
};

// This value is limited by UBO size, ES3.0 only guarantees 16 KiB.
// Values <= 256, use less CPU and GPU resources.
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_INDEX = CONFIG_MAX_LIGHT_COUNT - 1;

// The maximum number of spotlights in a scene that can cast shadows.
// There is currently a limit to 14 spot shadow due to how we store the culling result
// (see View.h).
constexpr size_t CONFIG_MAX_SHADOW_CASTING_SPOTS = 14;

// The maximum number of shadow cascades that can be used for directional lights.
constexpr size_t CONFIG_MAX_SHADOW_CASCADES = 4;

// The maximum UBO size, in bytes. This value is set to 16 KiB due to the ES3.0 spec.
// Note that this value constrains the maximum number of skinning bones, morph targets,
// instances, and shadow casting spotlights.
constexpr size_t CONFIG_MINSPEC_UBO_SIZE = 16384;

// The maximum number of instances that Filament automatically creates as an optimization.
constexpr size_t CONFIG_MAX_INSTANCES = 64;

// The maximum number of bones that can be associated with a single renderable.
// We store 32 bytes per bone. Must be a power-of-two, and must fit within CONFIG_MINSPEC_UBO_SIZE.
constexpr size_t CONFIG_MAX_BONE_COUNT = 256;

// The maximum number of morph targets associated with a single renderable.
// Note that ES3.0 only guarantees 256 layers in an array texture.
// Furthermore, this is constrained by CONFIG_MINSPEC_UBO_SIZE (16 bytes per morph target).
constexpr size_t CONFIG_MAX_MORPH_TARGET_COUNT = 256;

} // namespace filament

template<>
struct utils::EnableIntegerOperators<filament::BindingPoints> : public std::true_type {};

template<>
inline constexpr size_t utils::Enum::count<filament::BindingPoints>() { return 8; }

static_assert(utils::Enum::count<filament::BindingPoints>() <= filament::backend::CONFIG_BINDING_COUNT);

static_assert(
        filament::BindingPoints::PER_MATERIAL_INSTANCE == utils::Enum::count<filament::BindingPoints>()-1u,
        "Dynamically sized sampler buffer must be the last binding point.");

#endif // TNT_FILAMENT_ENGINE_ENUM_H
