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
        UpdateTimeStamp();
    }
    float VzLight::GetIntensity() const
    {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getIntensity(ins);
    }
    void VzLight::SetColor(const float color[3]) {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _color = *(float3*)color;
        lcm.setColor(ins, _color);
    }
    void VzLight::GetColor(float color[3]) {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _color = lcm.getColor(ins);
        if (color) *(float3*)color = float3(_color);
    }
    void VzLight::SetRange(const float range) {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setFalloff(ins, range);
        UpdateTimeStamp();

    }
    float VzLight::GetRange() const {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getIntensity(ins);
    }
    void VzLight::SetCone(const float inner, const float outer) {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setSpotLightCone(ins, inner, outer);
    }
    float VzLight::getInnerCone() const {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getSpotLightInnerCone(ins);
    }
    float VzLight::getOuterCone() const {
        COMP_LIGHT(lcm, ett, ins, -1.f);
        return lcm.getSpotLightOuterCone(ins);
    }
    }
