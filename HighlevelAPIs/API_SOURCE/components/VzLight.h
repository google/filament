#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzLight : VzSceneComp
    {
        VzLight(const VID vid, const std::string& originFrom, const std::string& typeName, const SCENE_COMPONENT_TYPE scenecompType)
            : VzSceneComp(vid, originFrom, typeName, scenecompType) {}
        enum class Type : uint8_t {
            SUN,            //!< Directional light that also draws a sun's disk in the sky.
            DIRECTIONAL,    //!< Directional light, emits light in a given direction.
            POINT,          //!< Point light, emits light from a position, in all directions.
            FOCUSED_SPOT,   //!< Physically correct spot light.
            SPOT,           //!< Spot light with coupling of outer cone and illumination disabled.
        };
        void SetIntensity(const float intensity = 110000);
        float GetIntensity() const;
        void SetColor(const float color[3]);
        void GetColor(float color[3]);
        void SetRange(const float range);
        float GetRange() const;
        void SetCone(const float inner, const float outer);
        float getInnerCone() const;
        float getOuterCone() const;
    };
}
