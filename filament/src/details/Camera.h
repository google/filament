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

#ifndef TNT_FILAMENT_DETAILS_CAMERA_H
#define TNT_FILAMENT_DETAILS_CAMERA_H

#include <filament/Camera.h>

#include "downcast.h"

#include <filament/Frustum.h>

#include <private/filament/EngineEnums.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Panic.h>

#include <math/mat4.h>
#include <math/scalar.h>

namespace filament {

class FEngine;

/*
 * FCamera is used to easily compute the projection and view matrices
 */
class FCamera : public Camera {
public:
    // a 35mm camera has a 36x24mm wide frame size
    static constexpr const float SENSOR_SIZE = 0.024f;    // 24mm

    FCamera(FEngine& engine, utils::Entity e);

    void terminate(FEngine&) noexcept { }


    // Sets the projection matrices (viewing and culling). The viewing matrice has infinite far.
    void setProjection(Projection projection,
                       double left, double right, double bottom, double top,
                       double near, double far);

    // Sets custom projection matrices (sets both the viewing and culling projections).
    void setCustomProjection(math::mat4 const& projection,
            math::mat4 const& projectionForCulling, double near, double far) noexcept;

    inline void setCustomProjection(math::mat4 const& projection,
            double const near, double const far) noexcept {
        setCustomProjection(projection, projection, near, far);
    }

    void setCustomEyeProjection(math::mat4 const* projection, size_t count,
            math::mat4 const& projectionForCulling, double near, double far);


    void setScaling(math::double2 const scaling) noexcept { mScalingCS = scaling; }

    math::double4 getScaling() const noexcept { return math::double4{ mScalingCS, 1.0, 1.0 }; }

    void setShift(math::double2 const shift) noexcept { mShiftCS = shift * 2.0; }

    math::double2 getShift() const noexcept { return mShiftCS * 0.5; }

    // viewing the projection matrix to be used for rendering, contains scaling/shift and possibly
    // other transforms needed by the shaders
    math::mat4 getProjectionMatrix(uint8_t eye = 0) const noexcept;

    // culling the projection matrix to be used for culling, contains scaling/shift
    math::mat4 getCullingProjectionMatrix() const noexcept;

    math::mat4 getEyeFromViewMatrix(uint8_t const eye) const noexcept { return mEyeFromView[eye]; }

    // viewing projection matrix set by the user
    const math::mat4& getUserProjectionMatrix(uint8_t eyeId) const;

    // culling projection matrix set by the user
    math::mat4 getUserCullingProjectionMatrix() const noexcept { return mProjectionForCulling; }

    double getNear() const noexcept { return mNear; }

    double getCullingFar() const noexcept { return mFar; }

    // sets the camera's model matrix (must be a rigid transform)
    void setModelMatrix(const math::mat4& modelMatrix) noexcept;
    void setModelMatrix(const math::mat4f& modelMatrix) noexcept;
    void setEyeModelMatrix(uint8_t eyeId, math::mat4 const& model);

    // sets the camera's model matrix
    void lookAt(math::double3 const& eye, math::double3 const& center, math::double3 const& up) noexcept;

    // returns the model matrix
    math::mat4 getModelMatrix() const noexcept;

    // returns the view matrix (inverse of the model matrix)
    math::mat4 getViewMatrix() const noexcept;

    template<typename T>
    static math::details::TMat44<T> rigidTransformInverse(math::details::TMat44<T> const& v) noexcept {
        // The inverse of a rigid transform can be computed from the transpose
        //  | R T |^-1    | Rt -Rt*T |
        //  | 0 1 |     = |  0   1   |
        const auto rt(transpose(v.upperLeft()));
        const auto t(rt * v[3].xyz);
        return { rt, -t };
    }

    math::double3 getPosition() const noexcept {
        return getModelMatrix()[3].xyz;
    }

    math::float3 getLeftVector() const noexcept {
        return normalize(getModelMatrix()[0].xyz);
    }

    math::float3 getUpVector() const noexcept {
        return normalize(getModelMatrix()[1].xyz);
    }

    math::float3 getForwardVector() const noexcept {
        // the camera looks towards -z
        return normalize(-getModelMatrix()[2].xyz);
    }

    float getFieldOfView(Fov const direction) const noexcept {
        // note: this is meaningless for an orthographic projection
        auto const& p = getProjectionMatrix();
        switch (direction) {
            case Fov::VERTICAL:
                return std::abs(2.0f * std::atan(1.0f / float(p[1][1])));
            case Fov::HORIZONTAL:
                return std::abs(2.0f * std::atan(1.0f / float(p[0][0])));
        }
    }

    float getFieldOfViewInDegrees(Fov const direction) const noexcept {
        return getFieldOfView(direction) * math::f::RAD_TO_DEG;
    }

    // Returns the camera's culling Frustum in world space
    Frustum getCullingFrustum() const noexcept;

    // sets this camera's exposure (default is f/16, 1/125s, 100 ISO)
    void setExposure(float aperture, float shutterSpeed, float sensitivity) noexcept;

    // returns this camera's aperture in f-stops
    float getAperture() const noexcept {
        return mAperture;
    }

    // returns this camera's shutter speed in seconds
    float getShutterSpeed() const noexcept {
        return mShutterSpeed;
    }

    // returns this camera's sensitivity in ISO
    float getSensitivity() const noexcept {
        return mSensitivity;
    }

    void setFocusDistance(float const distance) noexcept {
        mFocusDistance = distance;
    }

    float getFocusDistance() const noexcept {
        return mFocusDistance;
    }

    double getFocalLength() const noexcept;

    static double computeEffectiveFocalLength(double focalLength, double focusDistance) noexcept;

    static double computeEffectiveFov(double fovInDegrees, double focusDistance) noexcept;

    uint8_t getStereoscopicEyeCount() const noexcept;

    utils::Entity getEntity() const noexcept {
        return mEntity;
    }

    static math::mat4 projection(Fov direction, double fovInDegrees,
            double aspect, double near, double far);

    static math::mat4 projection(double focalLengthInMillimeters,
            double aspect, double near, double far);

private:
    FEngine& mEngine;
    utils::Entity mEntity;

    // For monoscopic cameras, mEyeProjection[0] == mEyeProjection[1].
    math::mat4 mEyeProjection[CONFIG_MAX_STEREOSCOPIC_EYES]; // projection matrix per eye (infinite far)
    math::mat4 mProjectionForCulling;                        // projection matrix (with far plane)
    math::mat4 mEyeFromView[CONFIG_MAX_STEREOSCOPIC_EYES];   // transforms from the main view (head)
                                                             // space to each eye's unique view space
    math::double2 mScalingCS = {1.0};  // additional scaling applied to projection
    math::double2 mShiftCS = {0.0};    // additional translation applied to projection

    double mNear{};
    double mFar{};
    // exposure settings
    float mAperture = 16.0f;
    float mShutterSpeed = 1.0f / 125.0f;
    float mSensitivity = 100.0f;
    float mFocusDistance = 0.0f;
};

struct CameraInfo {
    CameraInfo() noexcept {}

    // Creates a CameraInfo relative to inWorldTransform (i.e. it's model matrix is
    // transformed by inWorldTransform and inWorldTransform is recorded).
    // This is typically used for the color pass camera.
    CameraInfo(FCamera const& camera, math::mat4 const& inWorldTransform) noexcept;

    // Creates a CameraInfo from a camera that is relative to mainCameraInfo.
    // This is typically used for the shadow pass cameras.
    CameraInfo(FCamera const& camera, CameraInfo const& mainCameraInfo) noexcept;

    // Creates a CameraInfo from the FCamera
    explicit CameraInfo(FCamera const& camera) noexcept;

    union {
        // projection matrix for drawing (infinite zfar)
        // for monoscopic rendering
        // equivalent to eyeProjection[0], but aliased here for convenience
        math::mat4f projection;

        // for stereo rendering, one matrix per eye
        math::mat4f eyeProjection[CONFIG_MAX_STEREOSCOPIC_EYES] = {};
    };

    math::mat4f cullingProjection;                          // projection matrix for culling
    math::mat4f model;                                      // camera model matrix
    math::mat4f view;                                       // camera view matrix (inverse(model))
    math::mat4f eyeFromView[CONFIG_MAX_STEREOSCOPIC_EYES];  // eye view matrix (only for stereoscopic)
    math::mat4 worldTransform;                              // world transform (already applied
                                                            // to model and view)
    math::float4 clipTransform{1, 1, 0, 0};  // clip-space transform, only for VERTEX_DOMAIN_DEVICE
    float zn{};                              // distance (positive) to the near plane
    float zf{};                              // distance (positive) to the far plane
    float ev100{};                           // exposure
    float f{};                               // focal length [m]
    float A{};                               // f-number or f / aperture diameter [m]
    float d{};                               // focus distance [m]
    math::float3 const& getPosition() const noexcept { return model[3].xyz; }
    math::float3 getForwardVector() const noexcept { return normalize(-model[2].xyz); }
    math::mat4 getUserViewMatrix() const noexcept { return view * worldTransform; }

private:
    CameraInfo(FCamera const& camera,
            math::mat4 const& inWorldTransform,
            math::mat4 const& modelMatrix) noexcept;
};

FILAMENT_DOWNCAST(Camera)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_CAMERA_H
