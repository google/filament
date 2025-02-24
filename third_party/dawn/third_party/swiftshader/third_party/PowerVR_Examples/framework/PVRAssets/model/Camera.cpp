/*!
\brief Implementations of methods of the Camera class.
\file PVRAssets/model/Camera.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstring>

#include "PVRAssets/model/Camera.h"
#include "PVRCore/Errors.h"

namespace pvr {
namespace assets {
float Camera::getFOV(float timeInMs) const
{
	const float timeInSec = timeInMs * 0.001f;
	uint32_t f0 = 0;
	uint32_t f1 = 0;
	float t = 0.f;
	bool interpolate = true;

	if (_data.fovs.size() == 0) { return 0.7f; }

	if (timeInSec <= _data.fovs[0].timeInSec) { interpolate = false; }

	else if (timeInSec >= _data.fovs.back().timeInSec)
	{
		f0 = static_cast<uint32_t>(_data.fovs.size()) - 1;
		interpolate = false;
	}
	else
	{
		// find the bounding frame
		for (f1 = 0; f1 < _data.fovs.size() && _data.fovs[f1].timeInSec < timeInSec; ++f1)
			;
		f0 = f1 - 1;
		t = (timeInMs - _data.fovs[f0].timeInSec) / (_data.fovs[f1].timeInSec - _data.fovs[f0].timeInSec);
	}

	if (interpolate) { return _data.fovs[f0].fov * (1.f - t) + _data.fovs[f1].fov * t; }
	else
	{
		return _data.fovs[f0].fov;
	}
}

void Camera::setFOV(float fov)
{
	const float time = 0.0f;
	return setFOV(1, &fov, &time);
}

void Camera::setFOV(uint32_t frames, const float* const fovs, const float* timeInSec)
{
	_data.fovs.resize(frames);

	if (fovs == nullptr)
	{
		_data.fovs.resize(0);
		return;
	}

	for (uint32_t f = 0; f < frames; ++f)
	{
		_data.fovs[f].fov = fovs[f];
		_data.fovs[f].timeInSec = timeInSec[f];
	}
}
} // namespace assets
} // namespace pvr
//!\endcond
