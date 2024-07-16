#include "VzLight.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzLight::SetIntensity(const float intensity)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setIntensity(ins, intensity);
        timeStamp = std::chrono::high_resolution_clock::now();
    }
    float VzLight::GetIntensity() const
    {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getIntensity(ins);
    }
}
