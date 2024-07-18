#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzTexture : VzResource
    {
        VzTexture(const VID vid, const std::string& originFrom, const std::string& typeName, const RES_COMPONENT_TYPE resType)
            : VzResource(vid, originFrom, typeName, resType) {}
        void LoadImage(const std::string& fileName);

        // sampler
        // 
    };
}
