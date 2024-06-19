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
#include <utils/FixedCapacityVector.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

static constexpr size_t POST_PROCESS_VARIANT_BITS = 1;
static constexpr size_t POST_PROCESS_VARIANT_COUNT = (1u << POST_PROCESS_VARIANT_BITS);
static constexpr size_t POST_PROCESS_VARIANT_MASK = POST_PROCESS_VARIANT_COUNT - 1;
enum class PostProcessVariant : uint8_t {
    OPAQUE,
    TRANSLUCENT
};

// Binding points for uniform buffers
enum class UniformBindingPoints : uint8_t {
    PER_VIEW                   = 0,    // uniforms updated per view
    PER_RENDERABLE             = 1,    // uniforms updated per renderable
    PER_RENDERABLE_BONES       = 2,    // bones data, per renderable
    PER_RENDERABLE_MORPHING    = 3,    // morphing uniform/sampler updated per render primitive
    LIGHTS                     = 4,    // lights data array
    SHADOW                     = 5,    // punctual shadow data
    FROXEL_RECORDS             = 6,
    FROXELS                    = 7,
    PER_MATERIAL_INSTANCE      = 8,    // uniforms updates per material
    // Update utils::Enum::count<>() below when adding values here
    // These are limited by CONFIG_BINDING_COUNT (currently 10)
};

// Binding points for sampler buffers.
enum class SamplerBindingPoints : uint8_t {
    PER_VIEW                   = 0,    // samplers updated per view
    PER_RENDERABLE_MORPHING    = 1,    // morphing sampler updated per render primitive
    PER_MATERIAL_INSTANCE      = 2,    // samplers updates per material
    PER_RENDERABLE_SKINNING    = 3,    // bone indices and weights sampler updated per render primitive
    // Update utils::Enum::count<>() below when adding values here
    // These are limited by CONFIG_SAMPLER_BINDING_COUNT (currently 4)
};

enum class ReservedSpecializationConstants : uint8_t {
    BACKEND_FEATURE_LEVEL = 0,
    CONFIG_MAX_INSTANCES = 1,
    CONFIG_STATIC_TEXTURE_TARGET_WORKAROUND = 2,
    CONFIG_SRGB_SWAPCHAIN_EMULATION = 3, // don't change (hardcoded in OpenGLDriver.cpp)
    CONFIG_FROXEL_BUFFER_HEIGHT = 4,
    CONFIG_POWER_VR_SHADER_WORKAROUNDS = 5,
    CONFIG_DEBUG_DIRECTIONAL_SHADOWMAP = 6,
    CONFIG_DEBUG_FROXEL_VISUALIZATION = 7,
    CONFIG_STEREO_EYE_COUNT = 8, // don't change (hardcoded in ShaderCompilerService.cpp)
    CONFIG_SH_BANDS_COUNT = 9
};

enum class PushConstantIds : uint8_t  {
    MORPHING_BUFFER_OFFSET = 0,
};

// This value is limited by UBO size, ES3.0 only guarantees 16 KiB.
// It's also limited by the Froxelizer's record buffer data type (uint8_t).
constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;
constexpr size_t CONFIG_MAX_LIGHT_INDEX = CONFIG_MAX_LIGHT_COUNT - 1;

// The number of specialization constants that Filament reserves for its own use. These are always
// the first constants (from 0 to CONFIG_MAX_RESERVED_SPEC_CONSTANTS - 1).
// Updating this value necessitates a material version bump.
constexpr size_t CONFIG_MAX_RESERVED_SPEC_CONSTANTS = 16;

// The maximum number of shadowmaps.
// There is currently a maximum limit of 128 shadowmaps.
// Factors contributing to this limit:
// - minspec for UBOs is 16KiB, which currently can hold a maximum of 128 entries
constexpr size_t CONFIG_MAX_SHADOWMAPS = 64;

// The maximum number of shadow layers.
// There is currently a maximum limit of 255 layers.
// Several factors are contributing to this limit:
// - minspec for 2d texture arrays layer is 256
// - we're using uint8_t to store the number of layers (255 max)
// - nonsensical to be larger than the number of shadowmaps
constexpr size_t CONFIG_MAX_SHADOW_LAYERS = CONFIG_MAX_SHADOWMAPS;

// The maximum number of shadow cascades that can be used for directional lights.
constexpr size_t CONFIG_MAX_SHADOW_CASCADES = 4;

// The maximum UBO size, in bytes. This value is set to 16 KiB due to the ES3.0 spec.
// Note that this value constrains the maximum number of skinning bones, morph targets,
// instances, and shadow casting spotlights.
constexpr size_t CONFIG_MINSPEC_UBO_SIZE = 16384;

// The maximum number of instances that Filament automatically creates as an optimization.
// Use a much smaller number for WebGL as a workaround for the following Chrome issues:
//     https://crbug.com/1348017 Compiling GLSL is very slow with struct arrays
//     https://crbug.com/1348363 Lighting looks wrong with D3D11 but not OpenGL
// Note that __EMSCRIPTEN__ is not defined when running matc, but that's okay because we're
// actually using a specification constant.
#if defined(__EMSCRIPTEN__)
constexpr size_t CONFIG_MAX_INSTANCES = 8;
#else
constexpr size_t CONFIG_MAX_INSTANCES = 64;
#endif

// The maximum number of bones that can be associated with a single renderable.
// We store 32 bytes per bone. Must be a power-of-two, and must fit within CONFIG_MINSPEC_UBO_SIZE.
constexpr size_t CONFIG_MAX_BONE_COUNT = 256;

// The maximum number of morph targets associated with a single renderable.
// Note that ES3.0 only guarantees 256 layers in an array texture.
// Furthermore, this is constrained by CONFIG_MINSPEC_UBO_SIZE (16 bytes per morph target).
constexpr size_t CONFIG_MAX_MORPH_TARGET_COUNT = 256;

// The max number of eyes supported in stereoscopic mode.
// The number of eyes actually rendered is set at Engine creation time, see
// Engine::Config::stereoscopicEyeCount.
constexpr uint8_t CONFIG_MAX_STEREOSCOPIC_EYES = 4;

} // namespace filament

template<>
struct utils::EnableIntegerOperators<filament::UniformBindingPoints> : public std::true_type {};
template<>
struct utils::EnableIntegerOperators<filament::SamplerBindingPoints> : public std::true_type {};
template<>
struct utils::EnableIntegerOperators<filament::ReservedSpecializationConstants> : public std::true_type {};
template<>
struct utils::EnableIntegerOperators<filament::PushConstantIds> : public std::true_type {};
template<>
struct utils::EnableIntegerOperators<filament::PostProcessVariant> : public std::true_type {};

template<>
inline constexpr size_t utils::Enum::count<filament::UniformBindingPoints>() { return 9; }
template<>
inline constexpr size_t utils::Enum::count<filament::SamplerBindingPoints>() { return 4; }
template<>
inline constexpr size_t utils::Enum::count<filament::PostProcessVariant>() { return filament::POST_PROCESS_VARIANT_COUNT; }

static_assert(utils::Enum::count<filament::UniformBindingPoints>() <= filament::backend::CONFIG_UNIFORM_BINDING_COUNT);
static_assert(utils::Enum::count<filament::SamplerBindingPoints>() <= filament::backend::CONFIG_SAMPLER_BINDING_COUNT);

#endif // TNT_FILAMENT_ENGINE_ENUM_H
