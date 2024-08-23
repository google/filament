#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    struct API_EXPORT VzFont : VzResource
    {
        VzFont(const VID vid, const std::string& originFrom)
            : VzResource(vid, originFrom, "VzFont", RES_COMPONENT_TYPE::FONT) {}
        bool ReadFont(const std::string& fileName, const uint32_t fontSize = 10);
        std::string GetFontFileName();
    };
}
