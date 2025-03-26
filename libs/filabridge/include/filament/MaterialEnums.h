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

//! \file

#ifndef TNT_FILAMENT_MATERIAL_ENUM_H
#define TNT_FILAMENT_MATERIAL_ENUM_H

#include <utils/bitset.h>
#include <utils/BitmaskEnum.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

// update this when a new version of filament wouldn't work with older materials
static constexpr size_t MATERIAL_VERSION = 58;

/**
 * Supported shading models
 */
enum class Shading : uint8_t {
    UNLIT,                  //!< no lighting applied, emissive possible
    LIT,                    //!< default, standard lighting
    SUBSURFACE,             //!< subsurface lighting model
    CLOTH,                  //!< cloth lighting model
    SPECULAR_GLOSSINESS,    //!< legacy lighting model
};

/**
 * Attribute interpolation types in the fragment shader
 */
enum class Interpolation : uint8_t {
    SMOOTH,                 //!< default, smooth interpolation
    FLAT                    //!< flat interpolation
};

/**
 * Shader quality, affect some global quality parameters
 */
enum class ShaderQuality : int8_t {
    DEFAULT = -1,   // LOW on mobile, HIGH on desktop
    LOW     = 0,    // enable optimizations that can slightly affect correctness
    NORMAL  = 1,    // normal quality, correctness honored
    HIGH    = 2     // higher quality (e.g. better upscaling, etc...)
};

/**
 * Supported blending modes
 */
enum class BlendingMode : uint8_t {
    //! material is opaque
    OPAQUE,
    //! material is transparent and color is alpha-pre-multiplied, affects diffuse lighting only
    TRANSPARENT,
    //! material is additive (e.g.: hologram)
    ADD,
    //! material is masked (i.e. alpha tested)
    MASKED,
    /**
     * material is transparent and color is alpha-pre-multiplied, affects specular lighting
     * when adding more entries, change the size of FRenderer::CommandKey::blending
     */
    FADE,
    //! material darkens what's behind it
    MULTIPLY,
    //! material brightens what's behind it
    SCREEN,
    //! custom blending function
    CUSTOM,
};

/**
 * How transparent objects are handled
 */
enum class TransparencyMode : uint8_t {
    //! the transparent object is drawn honoring the raster state
    DEFAULT,
    /**
     * the transparent object is first drawn in the depth buffer,
     * then in the color buffer, honoring the culling mode, but ignoring the depth test function
     */
    TWO_PASSES_ONE_SIDE,

    /**
     * the transparent object is drawn twice in the color buffer,
     * first with back faces only, then with front faces; the culling
     * mode is ignored. Can be combined with two-sided lighting
     */
    TWO_PASSES_TWO_SIDES
};

/**
 * Supported types of vertex domains.
 */
enum class VertexDomain : uint8_t {
    OBJECT,                 //!< vertices are in object space, default
    WORLD,                  //!< vertices are in world space
    VIEW,                   //!< vertices are in view space
    DEVICE                  //!< vertices are in normalized device space
    // when adding more entries, make sure to update VERTEX_DOMAIN_COUNT
};

/**
 * Vertex attribute types
 */
enum VertexAttribute : uint8_t {
    // Update hasIntegerTarget() in VertexBuffer when adding an attribute that will
    // be read as integers in the shaders

    POSITION        = 0, //!< XYZ position (float3)
    TANGENTS        = 1, //!< tangent, bitangent and normal, encoded as a quaternion (float4)
    COLOR           = 2, //!< vertex color (float4)
    UV0             = 3, //!< texture coordinates (float2)
    UV1             = 4, //!< texture coordinates (float2)
    BONE_INDICES    = 5, //!< indices of 4 bones, as unsigned integers (uvec4)
    BONE_WEIGHTS    = 6, //!< weights of the 4 bones (normalized float4)
    // -- we have 1 unused slot here --
    CUSTOM0         = 8,
    CUSTOM1         = 9,
    CUSTOM2         = 10,
    CUSTOM3         = 11,
    CUSTOM4         = 12,
    CUSTOM5         = 13,
    CUSTOM6         = 14,
    CUSTOM7         = 15,

    // Aliases for legacy vertex morphing.
    // See RenderableManager::Builder::morphing().
    MORPH_POSITION_0 = CUSTOM0,
    MORPH_POSITION_1 = CUSTOM1,
    MORPH_POSITION_2 = CUSTOM2,
    MORPH_POSITION_3 = CUSTOM3,
    MORPH_TANGENTS_0 = CUSTOM4,
    MORPH_TANGENTS_1 = CUSTOM5,
    MORPH_TANGENTS_2 = CUSTOM6,
    MORPH_TANGENTS_3 = CUSTOM7,

    // this is limited by driver::MAX_VERTEX_ATTRIBUTE_COUNT
};

static constexpr size_t MAX_LEGACY_MORPH_TARGETS = 4;
static constexpr size_t MAX_MORPH_TARGETS = 256; // this is limited by filament::CONFIG_MAX_MORPH_TARGET_COUNT
static constexpr size_t MAX_CUSTOM_ATTRIBUTES = 8;

/**
 * Material domains
 */
enum class MaterialDomain : uint8_t {
    SURFACE         = 0, //!< shaders applied to renderables
    POST_PROCESS    = 1, //!< shaders applied to rendered buffers
    COMPUTE         = 2, //!< compute shader
};

/**
 * Specular occlusion
 */
enum class SpecularAmbientOcclusion : uint8_t {
    NONE            = 0, //!< no specular occlusion
    SIMPLE          = 1, //!< simple specular occlusion
    BENT_NORMALS    = 2, //!< more accurate specular occlusion, requires bent normals
};

/**
 * Refraction
 */
enum class RefractionMode : uint8_t {
    NONE            = 0, //!< no refraction
    CUBEMAP         = 1, //!< refracted rays go to the ibl cubemap
    SCREEN_SPACE    = 2, //!< refracted rays go to screen space
};

/**
 * Refraction type
 */
enum class RefractionType : uint8_t {
    SOLID           = 0, //!< refraction through solid objects (e.g. a sphere)
    THIN            = 1, //!< refraction through thin objects (e.g. window)
};

/**
 * Reflection mode
 */
enum class ReflectionMode : uint8_t {
    DEFAULT         = 0, //! reflections sample from the scene's IBL only
    SCREEN_SPACE    = 1, //! reflections sample from screen space, and fallback to the scene's IBL
};

// can't really use std::underlying_type<AttributeIndex>::type because the driver takes a uint32_t
using AttributeBitset = utils::bitset32;

static constexpr size_t MATERIAL_PROPERTIES_COUNT = 29;
enum class Property : uint8_t {
    BASE_COLOR,              //!< float4, all shading models
    ROUGHNESS,               //!< float,  lit shading models only
    METALLIC,                //!< float,  all shading models, except unlit and cloth
    REFLECTANCE,             //!< float,  all shading models, except unlit and cloth
    AMBIENT_OCCLUSION,       //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT,              //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_ROUGHNESS,    //!< float,  lit shading models only, except subsurface and cloth
    CLEAR_COAT_NORMAL,       //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY,              //!< float,  lit shading models only, except subsurface and cloth
    ANISOTROPY_DIRECTION,    //!< float3, lit shading models only, except subsurface and cloth
    THICKNESS,               //!< float,  subsurface shading model only
    SUBSURFACE_POWER,        //!< float,  subsurface shading model only
    SUBSURFACE_COLOR,        //!< float3, subsurface and cloth shading models only
    SHEEN_COLOR,             //!< float3, lit shading models only, except subsurface
    SHEEN_ROUGHNESS,         //!< float3, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR,          //!< float3, specular-glossiness shading model only
    GLOSSINESS,              //!< float,  specular-glossiness shading model only
    EMISSIVE,                //!< float4, all shading models
    NORMAL,                  //!< float3, all shading models only, except unlit
    POST_LIGHTING_COLOR,     //!< float4, all shading models
    POST_LIGHTING_MIX_FACTOR,//!< float, all shading models
    CLIP_SPACE_TRANSFORM,    //!< mat4,   vertex shader only
    ABSORPTION,              //!< float3, how much light is absorbed by the material
    TRANSMISSION,            //!< float,  how much light is refracted through the material
    IOR,                     //!< float,  material's index of refraction
    MICRO_THICKNESS,         //!< float, thickness of the thin layer
    BENT_NORMAL,             //!< float3, all shading models only, except unlit
    SPECULAR_FACTOR,         //!< float, lit shading models only, except subsurface and cloth
    SPECULAR_COLOR_FACTOR,   //!< float3, lit shading models only, except subsurface and cloth

    // when adding new Properties, make sure to update MATERIAL_PROPERTIES_COUNT
};

using UserVariantFilterMask = uint32_t;

enum class UserVariantFilterBit : UserVariantFilterMask {
    DIRECTIONAL_LIGHTING        = 0x01,         //!< Directional lighting
    DYNAMIC_LIGHTING            = 0x02,         //!< Dynamic lighting
    SHADOW_RECEIVER             = 0x04,         //!< Shadow receiver
    SKINNING                    = 0x08,         //!< Skinning
    FOG                         = 0x10,         //!< Fog
    VSM                         = 0x20,         //!< Variance shadow maps
    SSR                         = 0x40,         //!< Screen-space reflections
    STE                         = 0x80,         //!< Instanced stereo rendering
    ALL                         = 0xFF,
};

} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::UserVariantFilterBit>
        : public std::true_type {};

template<> struct utils::EnableIntegerOperators<filament::UserVariantFilterBit>
        : public std::true_type {};

#endif
