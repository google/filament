#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzTexture : VzResource
    {
        VzTexture(const VID vid, const std::string& originFrom)
            : VzResource(vid, originFrom, "VzTexture", RES_COMPONENT_TYPE::TEXTURE) {}
        bool ReadImage(const std::string& fileName, const bool generateMIPs = true);
        // sampler
        void SetMinFilter(const SamplerMinFilter filter);
        void SetMagFilter(const SamplerMagFilter filter);
        void SetWrapModeS(const SamplerWrapMode mode);
        void SetWrapModeT(const SamplerWrapMode mode);

        bool GenerateMIPs();
    };
}