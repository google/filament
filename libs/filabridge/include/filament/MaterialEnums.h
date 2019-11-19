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

#include <stddef.h>
#include <stdint.h>

namespace filament {

static constexpr size_t MATERIAL_VERSION = 4;

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

    // Aliases for vertex morphing.
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

static constexpr size_t MAX_MORPH_TARGETS = 4;
static constexpr size_t MAX_CUSTOM_ATTRIBUTES = 8;

/**
 * Material domains
 */
enum MaterialDomain : uint8_t {
    SURFACE         = 0, //!< shaders applied to renderables
    POST_PROCESS    = 1, //!< shaders applied to rendered buffers
};

// can't really use std::underlying_type<AttributeIndex>::type because the driver takes a uint32_t
using AttributeBitset = utils::bitset32;


} // namespace filament

#endif
