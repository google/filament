/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/Camera.h"

#include <math/mat4.h>

namespace filament {

using namespace math;

template<typename T>
details::TMat44<T> inverseProjection(const details::TMat44<T>& p) noexcept {
    details::TMat44<T> r;
    const T A = 1 / p[0][0];
    const T B = 1 / p[1][1];
    if (p[2][3] != T(0)) {
        // perspective projection
        // a 0 tx 0
        // 0 b ty 0
        // 0 0 tz c
        // 0 0 -1 0
        const T C = 1 / p[3][2];
        r[0][0] = A;
        r[1][1] = B;
        r[2][2] = 0;
        r[2][3] = C;
        r[3][0] = p[2][0] * A;    // not needed if symmetric
        r[3][1] = p[2][1] * B;    // not needed if symmetric
        r[3][2] = -1;
        r[3][3] = p[2][2] * C;
    } else {
        // orthographic projection
        // a 0 0 tx
        // 0 b 0 ty
        // 0 0 c tz
        // 0 0 0 1
        const T C = 1 / p[2][2];
        r[0][0] = A;
        r[1][1] = B;
        r[2][2] = C;
        r[3][3] = 1;
        r[3][0] = -p[3][0] * A;
        r[3][1] = -p[3][1] * B;
        r[3][2] = -p[3][2] * C;
    }
    return r;
}

mat4f Camera::inverseProjection(const mat4f& p) noexcept {
    return filament::inverseProjection(p);
}
mat4 Camera::inverseProjection(const mat4 & p) noexcept {
    return filament::inverseProjection(p);
}

void Camera::setProjection(Camera::Projection projection, double left, double right, double bottom,
        double top, double near, double far) noexcept {
    upcast(this)->setProjection(projection, left, right, bottom, top, near, far);
}

void Camera::setProjection(double fovInDegrees, double aspect, double near, double far,
        Camera::Fov direction) noexcept {
    upcast(this)->setProjection(fovInDegrees, aspect, near, far, direction);
}

void Camera::setLensProjection(double focalLengthInMillimeters,
        double aspect, double near, double far) noexcept {
    upcast(this)->setLensProjection(focalLengthInMillimeters, aspect, near, far);
}

void Camera::setCustomProjection(mat4 const& projection, double near, double far) noexcept {
    upcast(this)->setCustomProjection(projection, near, far);
}

void Camera::setCustomProjection(mat4 const& projection, mat4 const& projectionForCulling,
        double near, double far) noexcept {
    upcast(this)->setCustomProjection(projection, projectionForCulling, near, far);
}

void Camera::setScaling(double2 scaling) noexcept {
    upcast(this)->setScaling(scaling);
}

void Camera::setShift(double2 shift) noexcept {
    upcast(this)->setShift(shift);
}

mat4 Camera::getProjectionMatrix() const noexcept {
    return upcast(this)->getUserProjectionMatrix();
}

mat4 Camera::getCullingProjectionMatrix() const noexcept {
    return upcast(this)->getUserCullingProjectionMatrix();
}

double4 Camera::getScaling() const noexcept {
    return upcast(this)->getScaling();
}

double2 Camera::getShift() const noexcept {
    return upcast(this)->getShift();
}

float Camera::getNear() const noexcept {
    return upcast(this)->getNear();
}

float Camera::getCullingFar() const noexcept {
    return upcast(this)->getCullingFar();
}

void Camera::setModelMatrix(const mat4& modelMatrix) noexcept {
    upcast(this)->setModelMatrix(modelMatrix);
}

void Camera::setModelMatrix(const mat4f& modelMatrix) noexcept {
    upcast(this)->setModelMatrix(modelMatrix);
}

void Camera::lookAt(const float3& eye, const float3& center, float3 const& up) noexcept {
    upcast(this)->lookAt(eye, center, up);
}

void Camera::lookAt(const float3& eye, const float3& center) noexcept {
    upcast(this)->lookAt(eye, center, {0, 1, 0});
}

mat4 Camera::getModelMatrix() const noexcept {
    return upcast(this)->getModelMatrix();
}

mat4 Camera::getViewMatrix() const noexcept {
    return upcast(this)->getViewMatrix();
}

float3 Camera::getPosition() const noexcept {
    return upcast(this)->getPosition();
}

float3 Camera::getLeftVector() const noexcept {
    return upcast(this)->getLeftVector();
}

float3 Camera::getUpVector() const noexcept {
    return upcast(this)->getUpVector();
}

float3 Camera::getForwardVector() const noexcept {
    return upcast(this)->getForwardVector();
}

float Camera::getFieldOfViewInDegrees(Camera::Fov direction) const noexcept {
    return upcast(this)->getFieldOfViewInDegrees(direction);
}

Frustum Camera::getFrustum() const noexcept {
    return upcast(this)->getCullingFrustum();
}

utils::Entity Camera::getEntity() const noexcept {
    return upcast(this)->getEntity();
}

void Camera::setExposure(float aperture, float shutterSpeed, float ISO) noexcept {
    upcast(this)->setExposure(aperture, shutterSpeed, ISO);
}

float Camera::getAperture() const noexcept {
    return upcast(this)->getAperture();
}

float Camera::getShutterSpeed() const noexcept {
    return upcast(this)->getShutterSpeed();
}

float Camera::getSensitivity() const noexcept {
    return upcast(this)->getSensitivity();
}

void Camera::setFocusDistance(float distance) noexcept {
    upcast(this)->setFocusDistance(distance);
}

float Camera::getFocusDistance() const noexcept {
    return upcast(this)->getFocusDistance();
}

double Camera::getFocalLength() const noexcept {
    return upcast(this)->getFocalLength();
}

double Camera::computeEffectiveFocalLength(double focalLength, double focusDistance) noexcept {
    return FCamera::computeEffectiveFocalLength(focalLength, focusDistance);
}

double Camera::computeEffectiveFov(double fovInDegrees, double focusDistance) noexcept {
    return FCamera::computeEffectiveFov(fovInDegrees, focusDistance);
}

} // namespace filament
