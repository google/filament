#pragma once
#include "../VizComponentAPIs.h"
#include "VzMaterial.h"

namespace vzm
{
    __dojostruct VzMI : VzMaterial
    {
        static constexpr size_t MATERIAL_PROPERTIES_COUNT = 29;
        enum class MProp : uint8_t {
            BASE_COLOR = 0,              //!< float4, all shading models
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

        enum class RgbType : uint8_t {
            /**
             * the color is defined in Rec.709-sRGB-D65 (sRGB) space and the RGB values
             * have not been pre-multiplied by the alpha (for instance, a 50%
             * transparent red is <1,0,0,0.5>)
             */
            sRGB,
            /**
             * the color is defined in Rec.709-Linear-D65 ("linear sRGB") space and the
             * RGB values have not been pre-multiplied by the alpha (for instance, a 50%
             * transparent red is <1,0,0,0.5>)
             */
            LINEAR,
            /**
             * the color is defined in Rec.709-sRGB-D65 (sRGB) space and the RGB values
             * have been pre-multiplied by the alpha (for instance, a 50%
             * transparent red is <0.5,0,0,0.5>)
             */
            PREMULTIPLIED_sRGB,
            /**
             * the color is defined in Rec.709-Linear-D65 ("linear sRGB") space and the
             * RGB values have been pre-multiplied by the alpha (for instance, a 50%
             * transparent red is <0.5,0,0,0.5>)
             */
            PREMULTIPLIED_LINEAR
        } rgbType = RgbType::LINEAR;

        enum class TransparencyMode : uint8_t {
            DEFAULT = 0, // following the rasterizer state
            TWO_PASSES_ONE_SIDE,
            TWO_PASSES_TWO_SIDES
        };

        void SetTransparencyMode(const TransparencyMode tMode);
        void SetMaterialProperty(const MProp mProp, const std::vector<float>& v);

        // use attributes
        //void Update();
    };
}
