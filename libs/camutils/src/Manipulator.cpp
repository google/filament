/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <camutils/Manipulator.h>

#include <math/scalar.h>

#include "FreeFlightManipulator.h"
#include "MapManipulator.h"
#include "OrbitManipulator.h"

using namespace filament::math;

namespace filament {
namespace camutils {

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::viewport(int width, int height) {
    details.viewport[0] = width;
    details.viewport[1] = height;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::targetPosition(FLOAT x, FLOAT y, FLOAT z) {
    details.targetPosition = {x, y, z};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::upVector(FLOAT x, FLOAT y, FLOAT z) {
    details.upVector = {x, y, z};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::zoomSpeed(FLOAT val) {
    details.zoomSpeed = val;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::orbitHomePosition(FLOAT x, FLOAT y, FLOAT z) {
    details.orbitHomePosition = {x, y, z};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::orbitSpeed(FLOAT x, FLOAT y) {
    details.orbitSpeed = {x, y};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::fovDirection(Fov fov) {
    details.fovDirection = fov;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::fovDegrees(FLOAT degrees) {
    details.fovDegrees = degrees;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::farPlane(FLOAT distance) {
    details.farPlane = distance;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::mapExtent(FLOAT worldWidth, FLOAT worldHeight) {
    details.mapExtent = {worldWidth, worldHeight};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::mapMinDistance(FLOAT mindist) {
    details.mapMinDistance = mindist;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightStartPosition(FLOAT x, FLOAT y, FLOAT z) {
    details.flightStartPosition = {x, y, z};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightStartOrientation(FLOAT pitch, FLOAT yaw) {
    details.flightStartPitch = pitch;
    details.flightStartYaw = yaw;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightMaxMoveSpeed(FLOAT maxSpeed) {
    details.flightMaxSpeed = maxSpeed;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightSpeedSteps(int steps) {
    details.flightSpeedSteps = steps;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightPanSpeed(FLOAT x, FLOAT y) {
    details.flightPanSpeed = {x, y};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::flightMoveDamping(FLOAT damping) {
    details.flightMoveDamping = damping;
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::groundPlane(FLOAT a, FLOAT b, FLOAT c, FLOAT d) {
    details.groundPlane = {a, b, c, d};
    return *this;
}

template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::raycastCallback(RayCallback cb, void* userdata) {
    details.raycastCallback = cb;
    details.raycastUserdata = userdata;
    return *this;
}



template <typename FLOAT> typename
Manipulator<FLOAT>::Builder& Manipulator<FLOAT>::Builder::panning(bool enabled) {
    details.panning = enabled;
    return *this;
}

template <typename FLOAT>
Manipulator<FLOAT>* Manipulator<FLOAT>::Builder::build(Mode mode) {
    switch (mode) {
        case Mode::FREE_FLIGHT:
            return new FreeFlightManipulator<FLOAT>(mode, details);
        case Mode::MAP:
            return new MapManipulator<FLOAT>(mode, details);
        case Mode::ORBIT:
            return new OrbitManipulator<FLOAT>(mode, details);
    }
}

template <typename FLOAT>
Manipulator<FLOAT>::Manipulator(Mode mode, const Config& props) : mMode(mode) {
    setProperties(props);
}

template <typename FLOAT>
void Manipulator<FLOAT>::setProperties(const Config& props) {
    mProps = props;

    if (mProps.zoomSpeed == FLOAT(0)) {
        mProps.zoomSpeed = 0.01;
    }

    if (mProps.upVector == vec3(0)) {
        mProps.upVector = vec3(0, 1, 0);
    }

    if (mProps.fovDegrees == FLOAT(0)) {
        mProps.fovDegrees = 33;
    }

    if (mProps.farPlane == FLOAT(0)) {
        mProps.farPlane = 5000;
    }

    if (mProps.mapExtent == vec2(0)) {
        mProps.mapExtent = vec2(512);
    }
}

template <typename FLOAT>
void Manipulator<FLOAT>::setViewport(int width, int height) {
    Config props = mProps;
    props.viewport[0] = width;
    props.viewport[1] = height;
    setProperties(props);
}

template <typename FLOAT>
void Manipulator<FLOAT>::getLookAt(vec3* eyePosition, vec3* targetPosition, vec3* upward) const {
    *targetPosition = mTarget;
    *eyePosition = mEye;
    const vec3 gaze = normalize(mTarget - mEye);
    const vec3 right = cross(gaze, mProps.upVector);
    *upward = cross(right, gaze);
}

template<typename FLOAT>
static bool raycastPlane(const filament::math::vec3<FLOAT>& origin,
        const filament::math::vec3<FLOAT>& dir, FLOAT* t, void* userdata) {
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;
    auto props = (const typename Manipulator<FLOAT>::Config*) userdata;
    const vec4 plane = props->groundPlane;
    const vec3 n = vec3(plane[0], plane[1], plane[2]);
    const vec3 p0 = n * plane[3];
    const FLOAT denom = -dot(n, dir);
    if (denom > 1e-6) {
        const vec3 p0l0 = p0 - origin;
        *t = dot(p0l0, n) / -denom;
        return *t >= 0;
    }
    return false;
}

template <typename FLOAT>
void Manipulator<FLOAT>::getRay(int x, int y, vec3* porigin, vec3* pdir) const {
    const vec3 gaze = normalize(mTarget - mEye);
    const vec3 right = normalize(cross(gaze, mProps.upVector));
    const vec3 upward = cross(right, gaze);
    const FLOAT width = mProps.viewport[0];
    const FLOAT height = mProps.viewport[1];
    const FLOAT fov = mProps.fovDegrees * F_PI / 180.0;

    // Remap the grid coordinate into [-1, +1] and shift it to the pixel center.
    const FLOAT u = 2.0 * (0.5 + x) / width - 1.0;
    const FLOAT v = 2.0 * (0.5 + y) / height - 1.0;

    // Compute the tangent of the field-of-view angle as well as the aspect ratio.
    const FLOAT tangent = tan(fov / 2.0);
    const FLOAT aspect = width / height;

    // Adjust the gaze so it goes through the pixel of interest rather than the grid center.
    vec3 dir = gaze;
    if (mProps.fovDirection == Fov::VERTICAL) {
        dir += right * tangent * u * aspect;
        dir += upward * tangent * v;
    } else {
        dir += right * tangent * u;
        dir += upward * tangent * v / aspect;
    }
    dir = normalize(dir);

    *porigin = mEye;
    *pdir = dir;
}

template <typename FLOAT>
bool Manipulator<FLOAT>::raycast(int x, int y, vec3* result) const {
    vec3 origin, dir;
    getRay(x, y, &origin, &dir);

    // Choose either the user's callback function or the plane intersector.
    auto callback = mProps.raycastCallback;
    auto fallback = raycastPlane<FLOAT>;
    void* userdata = mProps.raycastUserdata;
    if (!callback) {
        callback = fallback;
        userdata = (void*) &mProps;
    }

    // If the ray misses, then try the fallback function.
    FLOAT t;
    if (!callback(mEye, dir, &t, userdata)) {
        if (callback == fallback || !fallback(mEye, dir, &t, (void*) &mProps)) {
            return false;
        }
    }

    *result = mEye + dir * t;
    return true;
}

template <typename FLOAT>
filament::math::vec3<FLOAT> Manipulator<FLOAT>::raycastFarPlane(int x, int y) const {
    const filament::math::vec3<FLOAT> gaze = normalize(mTarget - mEye);
    const vec3 right = cross(gaze, mProps.upVector);
    const vec3 upward = cross(right, gaze);
    const FLOAT width = mProps.viewport[0];
    const FLOAT height = mProps.viewport[1];
    const FLOAT fov = mProps.fovDegrees * math::F_PI / 180.0;

    // Remap the grid coordinate into [-1, +1] and shift it to the pixel center.
    const FLOAT u = 2.0 * (0.5 + x) / width - 1.0;
    const FLOAT v = 2.0 * (0.5 + y) / height - 1.0;

    // Compute the tangent of the field-of-view angle as well as the aspect ratio.
    const FLOAT tangent = tan(fov / 2.0);
    const FLOAT aspect = width / height;

    // Adjust the gaze so it goes through the pixel of interest rather than the grid center.
    vec3 dir = gaze;
    if (mProps.fovDirection == Fov::VERTICAL) {
        dir += right * tangent * u * aspect;
        dir += upward * tangent * v;
    } else {
        dir += right * tangent * u;
        dir += upward * tangent * v / aspect;
    }
    return mEye + dir * mProps.farPlane;
}

template <typename FLOAT>
void Manipulator<FLOAT>::keyDown(Manipulator<FLOAT>::Key key) { }

template <typename FLOAT>
void Manipulator<FLOAT>::keyUp(Manipulator<FLOAT>::Key key) { }

template <typename FLOAT>
void Manipulator<FLOAT>::update(FLOAT deltaTime) { }

template class Manipulator<float>;

} // namespace camutils
} // namespace filament
