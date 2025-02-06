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

FCamera::FCamera(FEngine& engine, Entity const e)
        : mEngine(engine),
          mEntity(e) {
}

mat4 FCamera::projection(Fov const direction, double const fovInDegrees,
        double const aspect, double const near, double const far) {
    double w;
    double h;
    double const s = std::tan(fovInDegrees * d::DEG_TO_RAD / 2.0) * near;
    if (direction == Fov::VERTICAL) {
        w = s * aspect;
        h = s;
    } else {
        w = s;
        h = s / aspect;
    }
    mat4 p = mat4::frustum(-w, w, -h, h, near, far);
    if (far == std::numeric_limits<double>::infinity()) {
        p[2][2] = -1.0f;           // lim(far->inf) = -1
        p[3][2] = -2.0f * near;    // lim(far->inf) = -2*near
    }
    return p;
}

mat4 FCamera::projection(double const focalLengthInMillimeters,
        double const aspect, double const near, double const far) {
    // a 35mm camera has a 36x24mm wide frame size
    double const h = (0.5 * near) * ((SENSOR_SIZE * 1000.0) / focalLengthInMillimeters);
    double const w = h * aspect;
    mat4 p = mat4::frustum(-w, w, -h, h, near, far);
    if (far == std::numeric_limits<double>::infinity()) {
        p[2][2] = -1.0f;           // lim(far->inf) = -1
        p[3][2] = -2.0f * near;    // lim(far->inf) = -2*near
    }
    return p;
}

/*
 * All methods for setting the projection funnel through here
 */

void UTILS_NOINLINE FCamera::setCustomProjection(mat4 const& p,
        mat4 const& c, double const near, double const far) noexcept {
    for (auto& eyeProjection: mEyeProjection) {
        eyeProjection = p;
    }
    mProjectionForCulling = c;
    mNear = near;
    mFar = far;
}

void UTILS_NOINLINE FCamera::setCustomEyeProjection(mat4 const* projection, size_t const count,
        mat4 const& projectionForCulling, double const near, double const far) {
    const Engine::Config& config = mEngine.getConfig();
    FILAMENT_CHECK_PRECONDITION(count >= config.stereoscopicEyeCount)
            << "All eye projections must be supplied together, count must be >= "
               "config.stereoscopicEyeCount ("
            << config.stereoscopicEyeCount << ")";
    for (int i = 0; i < config.stereoscopicEyeCount; i++) {
        mEyeProjection[i] = projection[i];
    }
    mProjectionForCulling = projectionForCulling;
    mNear = near;
    mFar = far;
}

void UTILS_NOINLINE FCamera::setProjection(Projection const projection,
        double const left, double const right,
        double const bottom, double const top,
        double const near, double const far) {

    FILAMENT_CHECK_PRECONDITION(!(left == right || bottom == top ||
            (projection == Projection::PERSPECTIVE && (near <= 0 || far <= near)) ||
            (projection == Projection::ORTHO && (near == far))))
            << "Camera preconditions not met in setProjection("
            << (projection == Projection::PERSPECTIVE ? "PERSPECTIVE" : "ORTHO") << ", "
            << left << ", " << right << ", " << bottom << ", " << top << ", " << near << ", " << far
            << ")";

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
    setCustomProjection(p, c, near, far);
}

mat4 FCamera::getProjectionMatrix(uint8_t const eye) const noexcept {
    UTILS_UNUSED_IN_RELEASE const Engine::Config& config = mEngine.getConfig();
    assert_invariant(eye < config.stereoscopicEyeCount);
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

mat4 FCamera::getCullingProjectionMatrix() const noexcept {
    // The culling projection matrix stays in the GL convention
    const mat4 m{ mat4::row_major_init{
            mScalingCS.x, 0.0, 0.0, mShiftCS.x,
            0.0, mScalingCS.y, 0.0, mShiftCS.y,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
    }};
    return m * mProjectionForCulling;
}

const mat4& FCamera::getUserProjectionMatrix(uint8_t const eyeId) const {
    const Engine::Config& config = mEngine.getConfig();
    FILAMENT_CHECK_PRECONDITION(eyeId < config.stereoscopicEyeCount)
            << "eyeId must be < config.stereoscopicEyeCount (" << config.stereoscopicEyeCount
            << ")";
    return mEyeProjection[eyeId];
}

void UTILS_NOINLINE FCamera::setModelMatrix(const mat4f& modelMatrix) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity), modelMatrix);
}

void UTILS_NOINLINE FCamera::setModelMatrix(const mat4& modelMatrix) noexcept {
    FTransformManager& transformManager = mEngine.getTransformManager();
    transformManager.setTransform(transformManager.getInstance(mEntity), modelMatrix);
}

void UTILS_NOINLINE FCamera::setEyeModelMatrix(uint8_t const eyeId, mat4 const& model) {
    const Engine::Config& config = mEngine.getConfig();
    FILAMENT_CHECK_PRECONDITION(eyeId < config.stereoscopicEyeCount)
            << "eyeId must be < config.stereoscopicEyeCount (" << config.stereoscopicEyeCount
            << ")";
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

void FCamera::setExposure(float const aperture, float const shutterSpeed, float const sensitivity) noexcept {
    mAperture = clamp(aperture, MIN_APERTURE, MAX_APERTURE);
    mShutterSpeed = clamp(shutterSpeed, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED);
    mSensitivity = clamp(sensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);
}

double FCamera::getFocalLength() const noexcept {
    auto const& monoscopicEyeProjection = mEyeProjection[0];
    return (SENSOR_SIZE * monoscopicEyeProjection[1][1]) * 0.5;
}

double FCamera::computeEffectiveFocalLength(double const focalLength, double focusDistance) noexcept {
    focusDistance = std::max(focalLength, focusDistance);
    return (focusDistance * focalLength) / (focusDistance - focalLength);
}

double FCamera::computeEffectiveFov(double const fovInDegrees, double focusDistance) noexcept {
    double const f = 0.5 * SENSOR_SIZE / std::tan(fovInDegrees * d::DEG_TO_RAD * 0.5);
    focusDistance = std::max(f, focusDistance);
    double const fov = 2.0 * std::atan(SENSOR_SIZE * (focusDistance - f) / (2.0 * focusDistance * f));
    return fov * d::RAD_TO_DEG;
}

uint8_t FCamera::getStereoscopicEyeCount() const noexcept {
    const Engine::Config& config = mEngine.getConfig();
    return config.stereoscopicEyeCount;
}

// ------------------------------------------------------------------------------------------------

CameraInfo::CameraInfo(FCamera const& camera) noexcept
        : CameraInfo(camera, {}, camera.getModelMatrix()) {
}

CameraInfo::CameraInfo(FCamera const& camera, mat4 const& inWorldTransform) noexcept
        : CameraInfo(camera, inWorldTransform, inWorldTransform * camera.getModelMatrix()) {
}

CameraInfo::CameraInfo(FCamera const& camera, CameraInfo const& mainCameraInfo) noexcept
        : CameraInfo(camera, mainCameraInfo.worldTransform, camera.getModelMatrix()) {
}

CameraInfo::CameraInfo(FCamera const& camera,
        mat4 const& inWorldTransform,
        mat4 const& modelMatrix) noexcept {
    for (size_t i = 0; i < camera.getStereoscopicEyeCount(); i++) {
        eyeProjection[i]   = mat4f{ camera.getProjectionMatrix(i) };
        eyeFromView[i]     = mat4f{ camera.getEyeFromViewMatrix(i) };
    }
    cullingProjection  = mat4f{ camera.getCullingProjectionMatrix() };
    model              = mat4f{ modelMatrix };
    view               = mat4f{ inverse(modelMatrix) };
    worldTransform     = inWorldTransform;
    zn                 = (float)camera.getNear();
    zf                 = (float)camera.getCullingFar();
    ev100              = Exposure::ev100(camera);
    f                  = (float)camera.getFocalLength();
    A                  = f / camera.getAperture();
    d                  = std::max(zn, camera.getFocusDistance());
}

} // namespace filament
