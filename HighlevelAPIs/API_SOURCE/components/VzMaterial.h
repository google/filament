#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzMaterial : VzResource
    {
        VzMaterial(const VID vid, const std::string& originFrom)
            : VzResource(vid, originFrom, "VzMaterial", RES_COMPONENT_TYPE::MATERIAL) {}
        enum class MaterialType : uint8_t {
            STANDARD = 0,
            CUSTOM
        };
        enum class LightingModel : uint8_t {
            UNLIT,                  //!< no lighting applied, emissive possible
            LIT,                    //!< default, standard lighting
            SUBSURFACE,             //!< subsurface lighting model
            CLOTH,                  //!< cloth lighting model
            SPECULAR_GLOSSINESS,    //!< legacy lighting model
        };
        struct ParameterInfo {
            //! Name of the parameter.
            const char* name;
            //! Whether the parameter is a sampler (texture).
            bool isSampler;
            //! Whether the parameter is a subpass type.
            bool isSubpass;
            union {
                //! Type of the parameter if the parameter is not a sampler.
                UniformType type;
                //! Type of the parameter if the parameter is a sampler.
                SamplerType samplerType;
                //! Type of the parameter if the parameter is a subpass.
                SubpassType subpassType;
            };
            //! Size of the parameter when the parameter is an array.
            uint32_t count;
            //! Requested precision of the parameter.
            Precision precision;
        };
        enum class AlphaMode : uint8_t {
            OPAQUE,
            MASK,
            BLEND
        };
        struct alignas(4) MaterialKey {
          // -- 32 bit boundary --
          bool doubleSided : 1;
          bool unlit : 1;
          bool hasVertexColors : 1;
          bool hasBaseColorTexture : 1;
          bool hasNormalTexture : 1;
          bool hasOcclusionTexture : 1;
          bool hasEmissiveTexture : 1;
          bool useSpecularGlossiness : 1;
          AlphaMode alphaMode : 4;
          bool enableDiagnostics : 4;
          union {
            struct {
              bool hasMetallicRoughnessTexture : 1;
              uint8_t metallicRoughnessUV : 7;
            };
            struct {
              bool hasSpecularGlossinessTexture : 1;
              uint8_t specularGlossinessUV : 7;
            };
          };
          uint8_t baseColorUV;
          // -- 32 bit boundary --
          bool hasClearCoatTexture : 1;
          uint8_t clearCoatUV : 7;
          bool hasClearCoatRoughnessTexture : 1;
          uint8_t clearCoatRoughnessUV : 7;
          bool hasClearCoatNormalTexture : 1;
          uint8_t clearCoatNormalUV : 7;
          bool hasClearCoat : 1;
          bool hasTransmission : 1;
          bool hasTextureTransforms : 6;
          // -- 32 bit boundary --
          uint8_t emissiveUV;
          uint8_t aoUV;
          uint8_t normalUV;
          bool hasTransmissionTexture : 1;
          uint8_t transmissionUV : 7;
          // -- 32 bit boundary --
          bool hasSheenColorTexture : 1;
          uint8_t sheenColorUV : 7;
          bool hasSheenRoughnessTexture : 1;
          uint8_t sheenRoughnessUV : 7;
          bool hasVolumeThicknessTexture : 1;
          uint8_t volumeThicknessUV : 7;
          bool hasSheen : 1;
          bool hasIOR : 1;
          bool hasVolume : 1;
          bool hasSpecular : 1;
          bool hasSpecularTexture : 1;
          bool hasSpecularColorTexture : 1;
          bool padding : 2;
          // -- 32 bit boundary --
          uint8_t specularTextureUV;
          uint8_t specularColorTextureUV;
          uint16_t padding2;
        };
        void SetMaterialType(const MaterialType type);
        MaterialType GetMaterialType() const;

        void SetLightingModel(const LightingModel model);
        LightingModel GetLightingModel() const;

        // TODO : 'Default' Setters and Getters for Material Instances
        size_t GetAllowedParameters(std::map<std::string, ParameterInfo>& paramters);

        MaterialKey GetVzmMaterialKey();
        bool SetMaterialKey(MaterialKey materialKey);
    };
}
