#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzMaterial : VzResource
    {
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
        void SetMaterialType(const MaterialType type);
        MaterialType GetMaterialType() const;

        void SetLightingModel(const LightingModel model);
        LightingModel GetLightingModel() const;
    };
}
