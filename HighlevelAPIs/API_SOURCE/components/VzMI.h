#pragma once
#include "../VizComponentAPIs.h"
#include "VzMaterial.h"

namespace vzm
{
    __dojostruct VzMI : VzResource
    {
        VzMI(const VID vid, const std::string& originFrom, const std::string& typeName, const RES_COMPONENT_TYPE resType)
            : VzResource(vid, originFrom, typeName, resType) {}

        enum class TransparencyMode : uint8_t {
            DEFAULT = 0, // following the rasterizer state
            TWO_PASSES_ONE_SIDE,
            TWO_PASSES_TWO_SIDES
        };

        void SetTransparencyMode(const TransparencyMode tMode);
        bool SetParameter(const std::string& name, const vzm::UniformType vType, const void* v);
        bool SetParameter(const std::string& name, const vzm::RgbType vType, const float* v);
        bool SetParameter(const std::string& name, const vzm::RgbaType vType, const float* v);
        void SetTexture(const std::string& uniformName, const VID vidTexture);
        VID GetMaterial();
    };
}
