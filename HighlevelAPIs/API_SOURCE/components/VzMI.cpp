#include "VzMI.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

extern std::vector<std::string> gMProp;

namespace vzm
{
    void VzMI::SetTransparencyMode(const TransparencyMode tMode)
    {
        COMP_MI(mi, );
        mi->setTransparencyMode((filament::TransparencyMode)tMode);
        UpdateTimeStamp();
    }
    void VzMI::SetMaterialProperty(const MProp mProp, const std::vector<float>& v)
    {
        COMP_MI(mi, );
        if (mProp == MProp::BASE_COLOR)
        {
            mi->setParameter(gMProp[(uint32_t)mProp].c_str(), (filament::RgbaType)rgbType, *(filament::math::float4*)&v[0]);
        }
        UpdateTimeStamp();
    }
}
