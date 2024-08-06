#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzGeometry : VzResource
    {
        VzGeometry(const VID vid, const std::string& originFrom)
            : VzResource(vid, originFrom, "VzGeometry", RES_COMPONENT_TYPE::GEOMATRY) {}
    };
}
