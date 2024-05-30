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

#ifndef CAMUTILS_FREE_FLIGHT_2_MANIPULATOR_H
#define CAMUTILS_FREE_FLIGHT_2_MANIPULATOR_H

#include <camutils/Manipulator.h>

#include <math/scalar.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <cmath>

namespace filament {
namespace camutils {

using namespace filament::math;

/**
 * [FreeFlight2Manipulator] is a combination of [FreeFlightManipulator] & [OribitManipulator] to support FreeFlight
 * navigation for one finger swipe(offered by FreeFlightManipulator) and PAN for two finger swipe, ZOOM for
 * two fingers pinch (offered by OrbitManipulator). Other than copying related code (mostly grabBegin, grabUpdate,
 * grabEnd, scroll & some private properties) no new changes are added.
 */
template<typename FLOAT>
class FreeFlight2Manipulator : public Manipulator<FLOAT> {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;
    using Bookmark = filament::camutils::Bookmark<FLOAT>;
    using Base = Manipulator<FLOAT>;
    using Config = typename Base::Config;

    enum GrabState { INACTIVE, FLYING, PANNING };

    FreeFlight2Manipulator(Mode mode, const Config& props) : Base(mode, props) {
        setProperties(props);
        Base::mEye = Base::mProps.flightStartPosition;
        const auto pitch = Base::mProps.flightStartPitch;
        const auto yaw = Base::mProps.flightStartYaw;
        mTargetEuler = {pitch, yaw};
        updateTarget(pitch, yaw);
    }

    void setProperties(const Config& props) override {
        Config resolved = props;

        if (resolved.flightPanSpeed == vec2(0, 0)) {
            resolved.flightPanSpeed = vec2(0.01, 0.01);
        }
        if (resolved.flightMaxSpeed == 0.0) {
            resolved.flightMaxSpeed = 10.0;
        }
        if (resolved.flightSpeedSteps == 0) {
            resolved.flightSpeedSteps = 80;
        }

        Base::setProperties(resolved);
    }

    void updateTarget(FLOAT pitch, FLOAT yaw) {
        Base::mTarget = Base::mEye + (mat3::eulerZYX(0, yaw, pitch) * vec3(0.0, 0.0, -1.0));
    }

    void grabBegin(int x, int y, bool strafe) override {
        mGrabState = strafe ? PANNING : FLYING;

        // For PANNING
        mGrabPivot = mPivot;
        mGrabEye = Base::mEye;
        mGrabTarget = Base::mTarget;
        mGrabFar = Base::raycastFarPlane(x, y);
        Base::raycast(x, y, &mGrabScene);

        // For FLYING
        mGrabWinX = x;
        mGrabWinY = y;
        mGrabEuler = mTargetEuler;
    }

    void grabUpdate(int x, int y) override {
        if (mGrabState == FLYING) {
            const auto& grabPitch = mGrabEuler.x;
            const auto& grabYaw = mGrabEuler.y;
            auto& pitch = mTargetEuler.x;
            auto& yaw = mTargetEuler.y;

            constexpr double EPSILON = 0.001;

            auto panSpeed = Base::mProps.flightPanSpeed;
            constexpr FLOAT minPitch = (-F_PI_2 + EPSILON);
            constexpr FLOAT maxPitch = ( F_PI_2 - EPSILON);
            const int delx = mGrabWinX - x;
            const int dely = mGrabWinY - y;

            pitch = clamp(grabPitch + dely * -panSpeed.y, minPitch, maxPitch);
            yaw = fmod(grabYaw + delx * panSpeed.x, 2.0 * F_PI);

            // Reverse the direction of the target to follow the same as the swipe direction
            updateTarget(-pitch, -yaw);
        }
        else if (mGrabState == PANNING) {
            const FLOAT ulen = distance(mGrabScene, mGrabEye);
            const FLOAT vlen = distance(mGrabFar, mGrabScene);
            const vec3 translation = (mGrabFar - Base::raycastFarPlane(x, y)) * ulen / vlen;
            mPivot = mGrabPivot + translation;
            Base::mEye = mGrabEye + translation;
            Base::mTarget = mGrabTarget + translation;
        }
    }

    void grabEnd() override {
        mGrabState = INACTIVE;
    }

    void scroll(int x, int y, FLOAT scrolldelta) override {
        const vec3 gaze = normalize(Base::mTarget - Base::mEye);
        const vec3 movement = gaze * Base::mProps.zoomSpeed * -scrolldelta;
        const vec3 v0 = mPivot - Base::mEye;
        Base::mEye += movement;
        Base::mTarget += movement;
    }

    Bookmark getCurrentBookmark() const override {
        Bookmark bookmark;
        bookmark.flight.position = Base::mEye;
        bookmark.flight.pitch = mTargetEuler.x;
        bookmark.flight.yaw = mTargetEuler.y;
        return bookmark;
    }

    Bookmark getHomeBookmark() const override {
        Bookmark bookmark;
        bookmark.flight.position = Base::mProps.flightStartPosition;
        bookmark.flight.pitch = Base::mProps.flightStartPitch;
        bookmark.flight.yaw = Base::mProps.flightStartYaw;
        return bookmark;
    }

    void jumpToBookmark(const Bookmark& bookmark) override {
        Base::mEye = bookmark.flight.position;
        updateTarget(bookmark.flight.pitch, bookmark.flight.yaw);
    }

private:
    GrabState mGrabState = INACTIVE;

    // PANNING and ZOOMING
    bool mFlipped = false;
    vec3 mGrabPivot;
    vec3 mGrabScene;
    vec3 mGrabFar;
    vec3 mGrabEye;
    vec3 mGrabTarget;

    // FLYING
    int mGrabWinX;
    int mGrabWinY;
    vec3 mPivot;
    vec2 mTargetEuler;  // (pitch, yaw)
    vec2 mGrabEuler;    // (pitch, yaw)
    FLOAT mMoveSpeed = 1.0f;
    vec3 mEyeVelocity;
};

} // namespace camutils
} // namespace filament

#endif /* CAMUTILS_FREE_FLIGHT_2_MANIPULATOR_H */
