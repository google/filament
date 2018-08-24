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

#ifndef TNT_FILAMENT_MATERIAL_ENUM_H
#define TNT_FILAMENT_MATERIAL_ENUM_H

#include <stddef.h>
#include <stdint.h>

namespace filament {
    enum class Shading : uint8_t {
        UNLIT,                  // no lighting applied, emissive possible
        LIT,                    // default, standard lighting
        SUBSURFACE,             // subsurface lighting model
        CLOTH,                  // cloth lighting model
    };

    enum class Interpolation : uint8_t {
        SMOOTH,                 // default, smooth interpolation
        FLAT                    // flat interpolation
    };

    static constexpr size_t MATERIAL_PROPERTIES_COUNT = 16;
    enum class Property : uint8_t {
        BASE_COLOR,              // float4, all shading models
        ROUGHNESS,               // float,  lit shading models only
        METALLIC,                // float,  all shading models, except unlit and cloth
        REFLECTANCE,             // float,  all shading models, except unlit and cloth
        AMBIENT_OCCLUSION,       // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT,              // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_ROUGHNESS,    // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_NORMAL,       // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY,              // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY_DIRECTION,    // float3, lit shading models only, except subsurface and cloth
        THICKNESS,               // float,  subsurface shading model only
        SUBSURFACE_POWER,        // float,  subsurface shading model only
        SUBSURFACE_COLOR,        // float3, subsurface and cloth shading models only
        SHEEN_COLOR,             // float3, cloth shading model only
        EMISSIVE,                // float4, all shading models
        NORMAL,                  // float3, all shading models only, except unlit
        // when adding new Properties, make sure to update MATERIAL_PROPERTIES_COUNT
    };

    enum class BlendingMode : uint8_t {
        OPAQUE,                 // material is opaque
        TRANSPARENT,            // material is transparent and color is alpha-pre-multiplied,
                                // affects diffuse lighting only
        ADD,                    // material is additive (e.g.: hologram)
        MASKED,                 // material is masked (i.e. alpha tested)
        FADE                    // material is transparent and color is alpha-pre-multiplied,
                                // affects specular lighting
        // when adding more entries, change the size of FRenderer::CommandKey::blending
    };

    enum class TransparencyMode : uint8_t {
        DEFAULT,                // the transparent object is drawn honoring the raster state
        TWO_PASSES_ONE_SIDE,    // the transparent object is first drawn in the depth buffer,
                                // then in the color buffer, honoring the culling mode, but
                                // ignoring the depth test function
        TWO_PASSES_TWO_SIDES    // the transparent object is drawn twice in the color buffer,
                                // first with back faces only, then with front faces; the culling
                                // mode is ignored. Can be combined with two-sided lighting
    };

    static constexpr size_t VERTEX_DOMAIN_COUNT = 4;
    enum class VertexDomain : uint8_t {
        OBJECT,                 // vertices are in object space, default
        WORLD,                  // vertices are in world space
        VIEW,                   // vertices are in view space
        DEVICE                  // vertices are in normalized device space
        // when adding more entries, make sure to update VERTEX_DOMAIN_COUNT
    };

    static constexpr size_t POST_PROCESS_STAGES_COUNT = 4;
    enum class PostProcessStage : uint8_t {
        TONE_MAPPING_OPAQUE,           // Tone mapping post-process
        TONE_MAPPING_TRANSLUCENT,      // Tone mapping post-process
        ANTI_ALIASING_OPAQUE,          // Anti-aliasing stage
        ANTI_ALIASING_TRANSLUCENT,     // Anti-aliasing stage
    };

    static constexpr size_t MATERIAL_VARIABLES_COUNT = 4;
    enum class Variable : uint8_t {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3
        // when adding more variables, make sure to update MATERIAL_VARIABLES_COUNT
    };
}

#endif
