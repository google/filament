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

#ifndef CAMUTILS_FREEFLIGHT_MANIPULATOR_H
#define CAMUTILS_FREEFLIGHT_MANIPULATOR_H

#include <camutils/Manipulator.h>

#include <math/scalar.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>

#include <utils/Log.h>

#include <cmath>

namespace filament {
namespace camutils {

using namespace filament::math;

template<typename FLOAT>
class FreeFlightManipulator : public Manipulator<FLOAT> {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;
    using Bookmark = filament::camutils::Bookmark<FLOAT>;
    using Base = Manipulator<FLOAT>;
    using Config = typename Base::Config;

    FreeFlightManipulator(Mode mode, const Config& props) : Base(mode, props) {
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
        mGrabWin = {x, y};
        mGrabbing = true;
        mGrabEuler = mTargetEuler;
    }

    void grabUpdate(int x, int y) override {
        if (!mGrabbing) {
            return;
        }

        const vec2 del = mGrabWin - vec2{x, y};

        const auto& grabPitch = mGrabEuler.x;
        const auto& grabYaw = mGrabEuler.y;
        auto& pitch = mTargetEuler.x;
        auto& yaw = mTargetEuler.y;

        constexpr double EPSILON = 0.001;

        auto panSpeed = Base::mProps.flightPanSpeed;
        constexpr FLOAT minPitch = (-F_PI_2 + EPSILON);
        constexpr FLOAT maxPitch = ( F_PI_2 - EPSILON);
        pitch = clamp(grabPitch + del.y * -panSpeed.y, minPitch, maxPitch);
        yaw = fmod(grabYaw + del.x * panSpeed.x, 2.0 * F_PI);

        updateTarget(pitch, yaw);
    }

    void grabEnd() override {
        mGrabbing = false;
    }

    void keyDown(typename Base::Key key) override {
        mKeyDown[(int) key] = true;
    }

    void keyUp(typename Base::Key key) override {
        mKeyDown[(int) key] = false;
    }

    void zoom(int x, int y, FLOAT scrolldelta) override {
        const FLOAT halfSpeedSteps = Base::mProps.flightSpeedSteps / 2;
        mScrollWheel = clamp(mScrollWheel + scrolldelta, -halfSpeedSteps, halfSpeedSteps);
        // Normalize the scroll position from -1 to 1 and calculate the move speed, in world
        // units per second.
        mScrollPositionNormalized = (mScrollWheel + halfSpeedSteps) / halfSpeedSteps - 1.0;
        mMoveSpeed = pow(Base::mProps.flightMaxSpeed, mScrollPositionNormalized);
    }

    void update(FLOAT deltaTime) override {
        vec3 forceLocal { 0.0, 0.0, 0.0 };

        if (mKeyDown[(int) Base::Key::UP]) {
            forceLocal += vec3{  0.0,  0.0, -1.0 };
        }
        if (mKeyDown[(int) Base::Key::LEFT]) {
            forceLocal += vec3{ -1.0,  0.0,  0.0 };
        }
        if (mKeyDown[(int) Base::Key::DOWN]) {
            forceLocal += vec3{  0.0,  0.0,  1.0 };
        }
        if (mKeyDown[(int) Base::Key::RIGHT]) {
            forceLocal += vec3{  1.0,  0.0,  0.0 };
        }

        forceLocal *= mMoveSpeed;

        const mat4 orientation = mat4::lookAt(Base::mEye, Base::mTarget, Base::mProps.upVector);
        const vec3 forceWorld = (orientation * vec4{ forceLocal, 0.0f }).xyz;

        const auto dampingFactor = Base::mProps.flightMoveDamping;
        if (dampingFactor == 0.0) {
            // Without damping, we simply treat the force as our velocity.
            mEyeVelocity = forceWorld;
        } else {
            // The dampingFactor acts as "friction", which acts upon the camera in the direction
            // opposite its velocity.
            // Force is also multiplied by the dampingFactor, to "make up" for the friction.
            // This ensures that the max velocity still approaches mMoveSpeed;
            vec3 velocityDelta = (forceWorld - mEyeVelocity) * dampingFactor;
            mEyeVelocity += velocityDelta * deltaTime;
        }

        const vec3 positionDelta = mEyeVelocity * deltaTime;

        Base::mEye += positionDelta;
        Base::mTarget += positionDelta;
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
        bookmark.flight.position = Base::mProps.flightStartPosition;;
        bookmark.flight.pitch = Base::mProps.flightStartPitch;
        bookmark.flight.yaw = Base::mProps.flightStartYaw;
        return bookmark;
    }

    void jumpToBookmark(const Bookmark& bookmark) override {
        Base::mEye = bookmark.flight.position;
        updateTarget(bookmark.flight.pitch, bookmark.flight.yaw);
    }

private:
    vec2 mGrabWin;
    vec2 mTargetEuler;  // (pitch, yaw)
    vec2 mGrabEuler;    // (pitch, yaw)
    bool mKeyDown[(int) Base::Key::COUNT] = {false};
    bool mGrabbing = false;
    FLOAT mScrollWheel = 0.0f;
    FLOAT mScrollPositionNormalized = 0.0f;
    FLOAT mMoveSpeed = 1.0f;
    vec3 mEyeVelocity;
};

} // namespace camutils
} // namespace filament

#endif /* CAMUTILS_FREEFLIGHT_MANIPULATOR_H */
