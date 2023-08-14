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

#include "components/TransformManager.h"

#include "details/Engine.h"

#include <filament/Exposure.h>
#include <filament/Camera.h>

#include <utils/compiler.h>
#include <utils/Panic.h>

#include <math/scalar.h>

#include <math/vec2.h>

using namespace filament::math;
using namespace utils;

namespace filament {

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

void UTILS_NOINLINE FCamera::setProjection(double fovInDegrees, double aspect, double near, double far,
        Camera::Fov direction) {
    double w;
    double h;
    double s = std::tan(fovInDegrees * math::d::DEG_TO_RAD / 2.0) * near;
    if (direction == Fov::VERTICAL) {
        w = s * aspect;
        h = s;
    } else {
        w = s;
        h = s / aspect;
    }
    FCamera::setProjection(Projection::PERSPECTIVE, -w, w, -h, h, near, far);
}

void FCamera::setLensProjection(double focalLengthInMillimeters,
        double aspect, double near, double far) {
    // a 35mm camera has a 36x24mm wide frame size
    double h = (0.5 * near) * ((SENSOR_SIZE * 1000.0) / focalLengthInMillimeters);
    double w = h * aspect;
    FCamera::setProjection(Projection::PERSPECTIVE, -w, w, -h, h, near, far);
}

/*
 * All methods for setting the projection funnel through here
 */

void UTILS_NOINLINE FCamera::setCustomProjection(mat4 const& p, double near, double far) noexcept {
    setCustomProjection(p, p, near, far);
}

void UTILS_NOINLINE FCamera::setCustomProjection(mat4 const& p,
        mat4 const& c, double near, double far) noexcept {
    for (uint8_t i = 0; i < CONFIG_STEREOSCOPIC_EYES; i++) {
        mEyeProjection[i] = p;
    }
    mProjectionForCulling = c;
    mNear = near;
    mFar = far;
}

void UTILS_NOINLINE FCamera::setCustomEyeProjection(math::mat4 const* projection, size_t count,
        math::mat4 const& projectionForCulling, double near, double far) {
    ASSERT_PRECONDITION(count >= CONFIG_STEREOSCOPIC_EYES,
            "All eye projections must be supplied together, count must be >= "
            "CONFIG_STEREOSCOPIC_EYES(%d)",
            CONFIG_STEREOSCOPIC_EYES);
    for (uint8_t i = 0; i < CONFIG_STEREOSCOPIC_EYES; i++) {
        mEyeProjection[i] = projection[i];
    }
    mProjectionForCulling = projectionForCulling;
    mNear = near;
    mFar = far;
}

void UTILS_NOINLINE FCamera::setProjection(Camera::Projection projection,
        double left, double right,
        double bottom, double top,
        double near, double far) {

    ASSERT_PRECONDITION(!(
            left == right ||
            bottom == top ||
            (projection == Projection::PERSPECTIVE && (near <= 0 || far <= near)) ||
            (projection == Projection::ORTHO && (near == far))),
            "Camera preconditions not met in setProjection(%s, %f, %f, %f, %f, %f, %f)",
            projection == Camera::Projection::PERSPECTIVE ? "PERSPECTIVE" : "ORTHO",
            left, right, bottom, top, near, far);

    mat4 c, p;
    switch (projection) {
        case Projection::PERSPECTIVE:
            /*
             * The general perspective projection in GL convention looks like this:
             *
             * P =  2N/r-l    0      r+l/r-l        0
             *       0      2N/t-b   t+b/t-b        0
             *       0        0      F+N/N-F   2*F*N/N-F
             *       0        0        -1           0
             */
            c = mat4::frustum(left, right, bottom, top, near, far);
            p = c;

            /*
             * but we're using a far plane at infinity
             *
             * P =  2N/r-l      0    r+l/r-l        0
             *       0      2N/t-b   t+b/t-b        0
             *       0       0         -1        -2*N    <-- far at infinity
             *       0       0         -1           0
             */
            p[2][2] = -1.0f;           // lim(far->inf) = -1
            p[3][2] = -2.0f * near;    // lim(far->inf) = -2*near
            break;

        case Projection::ORTHO:
            /*
             * The general orthographic projection in GL convention looks like this:
             *
             * P =  2/r-l    0         0       - r+l/r-l
             *       0      2/t-b      0       - t+b/t-b
             *       0       0       -2/F-N    - F+N/F-N
             *       0       0         0            1
             */
            c = mat4::ortho(left, right, bottom, top, near, far);
            p = c;
            break;
    }
    FCamera::setCustomProjection(p, c, near, far);
}

math::mat4 FCamera::getProjectionMatrix(uint8_t eye) const noexcept {
    assert_invariant(eye < CONFIG_STEREOSCOPIC_EYES);
    // This is where we transform the user clip-space (GL convention) to our virtual clip-space
    // (inverted DX convention)
    // Note that this math ends up setting the projection matrix' p33 to 0, which is where we're
    // getting back a lot of precision in the depth buffer.
    const mat4 m{ mat4::row_major_init{
            mScalingCS.x, 0.0, 0.0, mShiftCS.x,
            0.0, mScalingCS.y, 0.0, mShiftCS.y,
            0.0, 0.0, -0.5, 0.5,    // GL to inverted DX convention
            0.0, 0.0, 0.0, 1.0
    }};
    return m * mEyeProjection[eye];
}

math::mat4 FCamera::getCullingProjectionMatrix() const noexcept {
    // The culling projection matrix stays in the GL convention
    const mat4 m{ mat4::row_major_init{
            mScalingCS.x, 0.0, 0.0, mShiftCS.x,
            0.0, mScalingCS.y, 0.0, mShiftCS.y,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
    }};
    return m * mProjectionForCulling;
}

void UTILS_NOINLINE FCamera::setModelMatrix(const mat4f& modelMatrix) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity), modelMatrix);
}

void UTILS_NOINLINE FCamera::setModelMatrix(const mat4& modelMatrix) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity), modelMatrix);
}

void UTILS_NOINLINE FCamera::setEyeModelMatrix(uint8_t eyeId, math::mat4 const& model) {
    ASSERT_PRECONDITION(eyeId < CONFIG_STEREOSCOPIC_EYES,
            "eyeId must be < CONFIG_STEREOSCOPIC_EYES(%d)", CONFIG_STEREOSCOPIC_EYES);
    mEyeFromView[eyeId] = inverse(model);
}

void FCamera::lookAt(double3 const& eye, double3 const& center, double3 const& up) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity),
            mat4::lookAt(eye, center, up));
}

mat4 FCamera::getModelMatrix() const noexcept {
    FTransformManager const& transformManager = mEngine.getTransformManager();
    return transformManager.getWorldTransformAccurate(transformManager.getInstance(mEntity));
}

mat4 UTILS_NOINLINE FCamera::getViewMatrix() const noexcept {
    return inverse(getModelMatrix());
}

Frustum FCamera::getCullingFrustum() const noexcept {
    // for culling purposes we keep the far plane where it is
    return Frustum(mat4f{ getCullingProjectionMatrix() * getViewMatrix() });
}

void FCamera::setExposure(float aperture, float shutterSpeed, float sensitivity) noexcept {
    mAperture = clamp(aperture, MIN_APERTURE, MAX_APERTURE);
    mShutterSpeed = clamp(shutterSpeed, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED);
    mSensitivity = clamp(sensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);
}

double FCamera::getFocalLength() const noexcept {
    auto const& monoscopicEyeProjection = mEyeProjection[0];
    return (FCamera::SENSOR_SIZE * monoscopicEyeProjection[1][1]) * 0.5;
}

double FCamera::computeEffectiveFocalLength(double focalLength, double focusDistance) noexcept {
    focusDistance = std::max(focalLength, focusDistance);
    return (focusDistance * focalLength) / (focusDistance - focalLength);
}

double FCamera::computeEffectiveFov(double fovInDegrees, double focusDistance) noexcept {
    double f = 0.5 * FCamera::SENSOR_SIZE / std::tan(fovInDegrees * math::d::DEG_TO_RAD * 0.5);
    focusDistance = std::max(f, focusDistance);
    double fov = 2.0 * std::atan(FCamera::SENSOR_SIZE * (focusDistance - f) / (2.0 * focusDistance * f));
    return fov * math::d::RAD_TO_DEG;
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

// ------------------------------------------------------------------------------------------------

CameraInfo::CameraInfo(FCamera const& camera) noexcept {
    for (uint8_t i = 0; i < CONFIG_STEREOSCOPIC_EYES; i++) {
        eyeProjection[i]   = mat4f{ camera.getProjectionMatrix(i) };
    }
    cullingProjection  = mat4f{ camera.getCullingProjectionMatrix() };
    model              = mat4f{ camera.getModelMatrix() };
    view               = mat4f{ camera.getViewMatrix() };
    zn                 = (float)camera.getNear();
    zf                 = (float)camera.getCullingFar();
    ev100              = Exposure::ev100(camera);
    f                  = (float)camera.getFocalLength();
    A                  = f / camera.getAperture();
    d                  = std::max(zn, camera.getFocusDistance());
}

CameraInfo::CameraInfo(FCamera const& camera, const math::mat4& worldOriginCamera) noexcept {
    const mat4 modelMatrix{ worldOriginCamera * camera.getModelMatrix() };
    for (uint8_t i = 0; i < CONFIG_STEREOSCOPIC_EYES; i++) {
        eyeProjection[i]   = mat4f{ camera.getProjectionMatrix(i) };
        eyeFromView[i]     = mat4f{ camera.getEyeFromViewMatrix(i) };
    }
    cullingProjection  = mat4f{ camera.getCullingProjectionMatrix() };
    model              = mat4f{ modelMatrix };
    view               = mat4f{ inverse(modelMatrix) };
    worldOrigin        = worldOriginCamera;
    zn                 = (float)camera.getNear();
    zf                 = (float)camera.getCullingFar();
    ev100              = Exposure::ev100(camera);
    f                  = (float)camera.getFocalLength();
    A                  = f / camera.getAperture();
    d                  = std::max(zn, camera.getFocusDistance());
}

} // namespace filament
