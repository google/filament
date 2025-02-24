/*!
\brief Implementations of methods of the Light class.
\file PVRAssets/model/Light.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/model/Light.h"
namespace pvr {
namespace assets {
int32_t Light::getTargetIdx() const { return _data.spotTargetNodeIdx; }

const glm::vec3& Light::getColor() const { return _data.color; }

Light::LightType Light::getType() const { return _data.type; }

float Light::getConstantAttenuation() const { return _data.constantAttenuation; }

float Light::getLinearAttenuation() const { return _data.linearAttenuation; }

float Light::getQuadraticAttenuation() const { return _data.quadraticAttenuation; }

float Light::getFalloffAngle() const { return _data.falloffAngle; }

float Light::getFalloffExponent() const { return _data.falloffExponent; }

void Light::setTargetNodeIdx(int32_t index) { _data.spotTargetNodeIdx = index; }

void Light::setColor(float r, float g, float b)
{
	_data.color[0] = r;
	_data.color[1] = g;
	_data.color[2] = b;
}

void Light::setType(LightType t) { _data.type = t; }

void Light::setConstantAttenuation(float c) { _data.constantAttenuation = c; }

void Light::setLinearAttenuation(float l) { _data.linearAttenuation = l; }

void Light::setQuadraticAttenuation(float q) { _data.quadraticAttenuation = q; }

void Light::setFalloffAngle(float fa) { _data.falloffAngle = fa; }

void Light::setFalloffExponent(float fe) { _data.falloffExponent = fe; }

Light::InternalData& Light::getInternalData() { return _data; }
} // namespace assets
} // namespace pvr
//!\endcond