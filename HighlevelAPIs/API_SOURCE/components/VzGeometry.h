#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzGeometry : VzResource
    {
        VzGeometry(const VID vid, const std::string& originFrom, const std::string& typeName, const RES_COMPONENT_TYPE resType)
            : VzResource(vid, originFrom, typeName, resType) {}
    };
}
