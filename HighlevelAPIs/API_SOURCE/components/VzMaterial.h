#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzMaterial : VzResource
    {
        VzMaterial(const VID vid, const std::string& originFrom, const std::string& typeName, const RES_COMPONENT_TYPE resType)
            : VzResource(vid, originFrom, typeName, resType) {}
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
        void SetMaterialType(const MaterialType type);
        MaterialType GetMaterialType() const;

        void SetLightingModel(const LightingModel model);
        LightingModel GetLightingModel() const;

        size_t GetAllowedParameters(std::map<std::string, vzm::UniformType>& paramters);
    };
}
