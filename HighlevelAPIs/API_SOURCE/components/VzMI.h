#pragma once
#include "../VizComponentAPIs.h"
#include "VzMaterial.h"

namespace vzm
{
    __dojostruct VzMI : VzResource
    {
        VzMI(const VID vid, const std::string& originFrom)
            : VzResource(vid, originFrom, "VzMI", RES_COMPONENT_TYPE::MATERIALINSTANCE) {}

        enum class TransparencyMode : uint8_t {
            DEFAULT = 0, // following the rasterizer state
            TWO_PASSES_ONE_SIDE,
            TWO_PASSES_TWO_SIDES
        };

        void SetDoubleSided(const bool doubleSided);
        bool IsDoubleSided() const;
        void SetTransparencyMode(const TransparencyMode tMode);
        TransparencyMode GetTransparencyMode() const;
        bool SetParameter(const std::string& name, const vzm::UniformType vType, const void* v);
        bool SetParameter(const std::string& name, const vzm::RgbType vType, const float* v);
        bool SetParameter(const std::string& name, const vzm::RgbaType vType, const float* v);
        bool GetParameter(const std::string& name, const vzm::UniformType vType, const void* v);
        bool SetTexture(const std::string& name, const VID vidTexture);
        VID GetTexture(const std::string& name);

        VID GetMaterial();
    };
}
