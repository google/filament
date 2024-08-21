#include "VzLight.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzLight::SetType(const Type type)
    {
        COMP_LIGHT(lcm, ett, ins, );
        LightManager::Builder builder((LightManager::Type) type);
        for (unsigned int channel = 0; channel < 8; channel++)
        {
            builder.lightChannel(channel, lcm.getLightChannel(ins, channel));
        }
        builder.castShadows(lcm.isShadowCaster(ins));
        builder.shadowOptions(lcm.getShadowOptions(ins));
        builder.position(lcm.getPosition(ins));
        builder.direction(lcm.getDirection(ins));
        builder.color(lcm.getColor(ins));
        builder.intensityCandela(lcm.getIntensity(ins));
        builder.falloff(lcm.getFalloff(ins));
        builder.spotLightCone(lcm.getSpotLightInnerCone(ins), lcm.getSpotLightOuterCone(ins));
        builder.sunAngularRadius(lcm.getSunAngularRadius(ins));
        builder.sunHaloSize(lcm.getSunHaloSize(ins));
        builder.sunHaloFalloff(lcm.getSunHaloFalloff(ins));
        builder.build(*gEngine, ett);
        UpdateTimeStamp();
    }
    VzLight::Type VzLight::GetType() const
    {
        COMP_LIGHT(lcm, ett, ins, Type::DIRECTIONAL);
        return (VzLight::Type) lcm.getType(ins);
    }
    void VzLight::SetLightChannel(unsigned int channel, bool enable)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setLightChannel(ins, channel, enable);
        UpdateTimeStamp();
    }
    bool VzLight::GetLightChannel(unsigned int channel) const
    {
        COMP_LIGHT(lcm, ett, ins, false);
        return lcm.getLightChannel(ins, channel);
    }
    void VzLight::SetPosition(const float position[3])
    {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _position = *(float3*)position;
        lcm.setPosition(ins, _position);
        UpdateTimeStamp();
    }
    void VzLight::GetPosition(float position[3])
    {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _position = lcm.getPosition(ins);
        if (position) *(float3*)position = float3(_position);
    }
    void VzLight::SetDirection(const float direction[3])
    {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _direction = *(float3*)direction;
        lcm.setDirection(ins, _direction);
        UpdateTimeStamp();
    }
    void VzLight::GetDirection(float direction[3])
    {
        COMP_LIGHT(lcm, ett, ins, );
        float3 _direction = lcm.getDirection(ins);
        if (direction) *(float3*)direction = float3(_direction);
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
    void VzLight::SetIntensity(const float intensity) {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setIntensityCandela(ins, intensity);
        UpdateTimeStamp();
    }
    float VzLight::GetIntensity() const
    {
        COMP_LIGHT(lcm, ett, ins, 100000.0f);
        return lcm.getIntensity(ins);
    }
    void VzLight::SetFalloff(const float radius)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setFalloff(ins, radius);
        UpdateTimeStamp();
    }
    float VzLight::GetFalloff() const
    {
        COMP_LIGHT(lcm, ett, ins, 1.0f);
        return lcm.getFalloff(ins);
    }
    void VzLight::SetSpotLightCone(const float inner, const float outer)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setSpotLightCone(ins, inner, outer);
        UpdateTimeStamp();
    }
    float VzLight::GetSpotLightOuterCone() const
    {
        COMP_LIGHT(lcm, ett, ins, VZ_PIDIV4);
        return lcm.getSpotLightOuterCone(ins);
    }
    float VzLight::GetSpotLightInnerCone() const
    {
        COMP_LIGHT(lcm, ett, ins, VZ_PIDIV4 * 0.75f);
        return lcm.getSpotLightInnerCone(ins);
    }
    void VzLight::SetSunAngularRadius(const float angularRadius)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setSunAngularRadius(ins, angularRadius);
        UpdateTimeStamp();
    }
    float VzLight::GetSunAngularRadius() const
    {
        COMP_LIGHT(lcm, ett, ins, 0.00951f);
        return lcm.getSunAngularRadius(ins);
    }
    void VzLight::SetSunHaloSize(const float haloSize)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setSunHaloSize(ins, haloSize);
        UpdateTimeStamp();
    }
    float VzLight::GetSunHaloSize() const
    {
        COMP_LIGHT(lcm, ett, ins, 10.0f);
        return lcm.getSunHaloSize(ins);
    }
    void VzLight::SetSunHaloFalloff(const float haloFalloff)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setSunHaloFalloff(ins, haloFalloff);
        UpdateTimeStamp();
    }
    float VzLight::GetSunHaloFalloff() const
    {
        COMP_LIGHT(lcm, ett, ins, 80.0f);
        return lcm.getSunHaloFalloff(ins);
    }
    void VzLight::SetShadowOptions(ShadowOptions const& options)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setShadowOptions(ins, (LightManager::ShadowOptions const&) options);
        UpdateTimeStamp();
    }
    VzLight::ShadowOptions const* VzLight::GetShadowOptions() const
    {
        COMP_LIGHT(lcm, ett, ins, nullptr);
        return (VzLight::ShadowOptions const*) &lcm.getShadowOptions(ins);
    }
    void VzLight::SetShadowCaster(bool shadowCaster)
    {
        COMP_LIGHT(lcm, ett, ins, );
        lcm.setShadowCaster(ins, shadowCaster);
        UpdateTimeStamp();
    }
    bool VzLight::IsShadowCaster() const
    {
        COMP_LIGHT(lcm, ett, ins, false);
        return lcm.isShadowCaster(ins);
    }
}
