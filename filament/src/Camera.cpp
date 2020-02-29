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

#include "components/CameraManager.h"
#include "components/TransformManager.h"

#include "details/Engine.h"

#include <utils/compiler.h>
#include <utils/Panic.h>

#include <math/scalar.h>

using namespace filament::math;
using namespace utils;

namespace filament {
namespace details {

static constexpr const float MIN_APERTURE = 0.5f;
static constexpr const float MAX_APERTURE = 64.0f;
static constexpr const float MIN_SHUTTER_SPEED = 1.0f / 25000.0f;
static constexpr const float MAX_SHUTTER_SPEED = 60.0f;
static constexpr const float MIN_SENSITIVITY = 10.0f;
static constexpr const float MAX_SENSITIVITY = 204800.0f;

FCamera::FCamera(FEngine& engine, Entity e)
        : mEngine(engine),
          mEntity(e) {
}

void UTILS_NOINLINE FCamera::setProjection(double fov, double aspect, double near, double far,
        Camera::Fov direction) noexcept {
    double w, h;
    double s = std::tan(fov * (F_PI / 360.0)) * near;
    if (direction == Fov::VERTICAL) {
        w = s * aspect;
        h = s;
    } else {
        w = s;
        h = s / aspect;
    }
    FCamera::setProjection(Projection::PERSPECTIVE, -w, w, -h, h, near, far);
}

void FCamera::setLensProjection(double focalLength, double aspect, double near, double far) noexcept {
    // a 35mm camera has a 36x24mm wide frame size
    double theta = 2.0 * std::atan(24.0 / (2.0 * focalLength));
    theta *= 180.0 / math::F_PI;
    FCamera::setProjection(theta, aspect, near, far,Fov::VERTICAL);
}

/*
 * All methods for setting the projection funnel through here
 */

void UTILS_NOINLINE FCamera::setCustomProjection(mat4 const& p, double near, double far) noexcept {
    mProjectionForCulling = p;
    mProjection = p;
    mNear = (float)near;
    mFar = (float)far;
}

void UTILS_NOINLINE FCamera::setProjection(Camera::Projection projection,
        double left, double right,
        double bottom, double top,
        double near, double far) noexcept {

    // we make sure our preconditions are verified, using default values,
    // to avoid inconsistent states in the renderer later.
    if (UTILS_UNLIKELY(left == right ||
                       bottom == top ||
                       (projection == Projection::PERSPECTIVE && (near <= 0 || far <= near)) ||
                       (projection == Projection::ORTHO && (near == far)))) {
        PANIC_LOG("Camera preconditions not met. Using default projection.");
        left = -0.1;
        right = 0.1;
        bottom = -0.1;
        top = 0.1;
        near = 0.1;
        far = 100.0;
    }

    mat4 p;
    switch (projection) {
        case Projection::PERSPECTIVE:
            /*
             * The general perspective projection looks like this:
             *
             * P =  2N/r-l    0      r+l/r-l        0
             *       0      2N/t-b   t+b/t-b        0
             *       0        0      F+N/N-F   2*F*N/N-F
             *       0        0        -1           0
             */
            p = mat4::frustum(left, right, bottom, top, near, far);
            mProjectionForCulling = p;

            /*
             * we're using a far plane at infinity
             *
             * P =  2N/r-l      0    r+l/r-l        0
             *       0      2N/t-b   t+b/t-b        0
             *       0       0         -1        -2*N    <-- far at infinity
             *       0       0         -1           0
             */
            p[2][2] = -1;           // lim(far->inf) = -1
            p[3][2] = -2 * near;    // lim(far->inf) = -2*near

            /*
             * e.g.: A symmetrical frustum with far plane at infinity
             *
             * P =  N/r      0       0      0
             *       0      N/t      0      0
             *       0       0      -1    -2*N
             *       0       0      -1      0
             *
             * v(CC) = P*v
             * v(NDC) = v(CC) * (1 / v(CC).w)
             *
             * for v in the frustum, P generates v(CC).xyz in [-1, 1]
             *
             * v(WC).z = v(NDC).z * (f-n)*0.5 + (n+f)*0.5
             *         = v(NDC).z * 0.5 + 0.5
             */
            break;

        case Projection::ORTHO:
            /*
             * The general orthographic projection looks like this:
             *
             * P =  2/r-l    0         0       - r+l/r-l
             *       0      2/t-b      0       - t+b/t-b
             *       0       0       -2/F-N    - F+N/F-N
             *       0       0         0            1
             */
            p = mat4::ortho(left, right, bottom, top, near, far);
            mProjectionForCulling = p;
            break;
    }
    mProjection = p;
    mNear = float(near);
    mFar = float(far);
}

void UTILS_NOINLINE FCamera::setModelMatrix(const mat4f& modelMatrix) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity), modelMatrix);
}

void FCamera::lookAt(const float3& eye, const float3& center, const float3& up) noexcept {
    setModelMatrix(mat4f::lookAt(eye, center, up));
}

mat4f const& FCamera::getModelMatrix() const noexcept {
    FTransformManager const& transformManager = mEngine.getTransformManager();
    return transformManager.getWorldTransform(transformManager.getInstance(mEntity));
}

mat4f UTILS_NOINLINE FCamera::getViewMatrix() const noexcept {
    return FCamera::getViewMatrix(getModelMatrix());
}

Frustum FCamera::getFrustum() const noexcept {
    // for culling purposes we keep the far plane where it is
    return FCamera::getFrustum(mProjectionForCulling, getViewMatrix());
}

void FCamera::setExposure(float aperture, float shutterSpeed, float sensitivity) noexcept {
    mAperture = clamp(aperture, MIN_APERTURE, MAX_APERTURE);
    mShutterSpeed = clamp(shutterSpeed, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED);
    mSensitivity = clamp(sensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);
}

template<typename T>
math::details::TMat44<T> inverseProjection(const math::details::TMat44<T>& p) noexcept {
    math::details::TMat44<T> r;
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


UTILS_NOINLINE
mat4f FCamera::getViewMatrix(mat4f const& model) noexcept {
    return FCamera::rigidTransformInverse(model);
}

Frustum FCamera::getFrustum(mat4 const& projection, mat4f const& viewMatrix) noexcept {
    return Frustum(mat4f{ projection * viewMatrix });
}


} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

mat4f Camera::inverseProjection(const mat4f& p) noexcept {
    return details::inverseProjection(p);
}
mat4 Camera::inverseProjection(const mat4 & p) noexcept {
    return details::inverseProjection(p);
}

void Camera::setProjection(Camera::Projection projection, double left, double right, double bottom,
        double top, double near, double far) noexcept {
    upcast(this)->setProjection(projection, left, right, bottom, top, near, far);
}

void Camera::setProjection(double fov, double aspect, double near, double far,
        Camera::Fov direction) noexcept {
    upcast(this)->setProjection(fov, aspect, near, far, direction);
}

void Camera::setLensProjection(double focalLength, double aspect, double near, double far) noexcept {
    upcast(this)->setLensProjection(focalLength, aspect, near, far);
}

void Camera::setCustomProjection(mat4 const& projection, double near, double far) noexcept {
    upcast(this)->setCustomProjection(projection, near, far);
}

const mat4& Camera::getProjectionMatrix() const noexcept {
    return upcast(this)->getProjectionMatrix();
}

const mat4& Camera::getCullingProjectionMatrix() const noexcept {
    return upcast(this)->getCullingProjectionMatrix();
}

float Camera::getNear() const noexcept {
    return upcast(this)->getNear();
}

float Camera::getCullingFar() const noexcept {
    return upcast(this)->getCullingFar();
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

mat4f Camera::getModelMatrix() const noexcept {
    return upcast(this)->getModelMatrix();
}

mat4f Camera::getViewMatrix() const noexcept {
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
    return upcast(this)->getFrustum();
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

} // namespace filament
