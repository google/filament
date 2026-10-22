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

#include <algorithm>
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

    void scroll(int x, int y, FLOAT scrolldelta) override {
        const FLOAT halfSpeedSteps = Base::mProps.flightSpeedSteps / 2;
        mScrollWheel = clamp(mScrollWheel + scrolldelta, -halfSpeedSteps, halfSpeedSteps);
        // Normalize the scroll position from -1 to 1 and calculate the move speed, in world
        // units per second.
        mScrollPositionNormalized = (mScrollWheel + halfSpeedSteps) / halfSpeedSteps - 1.0;
        mMoveSpeed = pow(Base::mProps.flightMaxSpeed, mScrollPositionNormalized);
    }

    void update(FLOAT deltaTime) override {

        auto getLocalDirection = [this]() -> vec3 {
            vec3 directionLocal{ 0.0, 0.0, 0.0 };
            if (mKeyDown[(int)Base::Key::FORWARD]) {
                directionLocal += vec3{ 0.0, 0.0, -1.0 };
            }
            if (mKeyDown[(int)Base::Key::LEFT]) {
                directionLocal += vec3{ -1.0, 0.0, 0.0 };
            }
            if (mKeyDown[(int)Base::Key::BACKWARD]) {
                directionLocal += vec3{ 0.0, 0.0, 1.0 };
            }
            if (mKeyDown[(int)Base::Key::RIGHT]) {
                directionLocal += vec3{ 1.0, 0.0, 0.0 };
            }
            return directionLocal;
        };

        auto getWorldDirection = [this](vec3 directionLocal) -> vec3 {
            const mat4 orientation = mat4::lookAt(Base::mEye, Base::mTarget, Base::mProps.upVector);
            vec3 directionWorld = (orientation * vec4{ directionLocal, 0.0f }).xyz;
            if (mKeyDown[(int)Base::Key::UP]) {
                directionWorld += vec3{ 0.0, 1.0, 0.0 };
            }
            if (mKeyDown[(int)Base::Key::DOWN]) {
                directionWorld += vec3{ 0.0, -1.0, 0.0 };
            }
            return directionWorld;
        };

        vec3 const localDirection = getLocalDirection();
        vec3 const worldDirection = getWorldDirection(localDirection);

        // unit of dampingFactor is [1/s]
        FLOAT const dampingFactor = Base::mProps.flightMoveDamping;
        if (dampingFactor == 0.0) {
            // Without damping, we simply treat the force as our velocity.
            vec3 const speed = worldDirection * mMoveSpeed;
            mEyeVelocity = speed;
            vec3 const positionDelta = mEyeVelocity * deltaTime;
            Base::mEye += positionDelta;
            Base::mTarget += positionDelta;
        } else {
            auto dt = deltaTime / 16.0;
            for (size_t i = 0; i < 16; i++) {
                // Note: the algorithm below doesn't work well for large time steps because
                //       we're not using a closed form for updating the position, so we need
                //       to loop a few times. We could make this better by having a dynamic
                //       loop count. What we're really doing is evaluation the solution to
                //       a differential equation numerically.

                // Kinetic friction is a force opposing velocity and proportional to it.:
                //      F = -kv
                //      F = ma
                // ==>  ma = -kv
                //      a = -vk/m               [m.s^-2] = [m/s] * [Kg/s] / [Kg]
                // ==>  dampingFactor = k/m        [1/s] = [Kg/s] / [Kg]
                //
                // The velocity update for dt due to friction is then:
                // v = v + a.dt
                //   = v - v * dampingFactor * dt
                //   = v * (1.0 - dampingFactor * dt)
                mEyeVelocity = mEyeVelocity * saturate(1.0 - dampingFactor * dt);

                // We also undergo an acceleration proportional to the distance to the target speed
                // (the closer we are the less we accelerate, similar to a car).
                //       F = k * (target_v - v)
                //       F = ma
                //  ==> ma = k * (target_v - v)
                //       a = k/m * (target_v - v)       [m.s^-2] = [Kg/s] / [Kg] * [m/s]
                //
                // The velocity update for dt due to the acceleration (the gas basically) is then:
                // v = v + a.dt
                //   = v + k/m * (target_v - v).dt
                // We're using the same dampingFactor here, but we don't have to.
                auto const accelerationFactor = dampingFactor;
                vec3 const acceleration = worldDirection *
                        (accelerationFactor * std::max(mMoveSpeed - length(mEyeVelocity), FLOAT(0)));
                mEyeVelocity += acceleration * dt;
                vec3 const positionDelta = mEyeVelocity * dt;
                Base::mEye += positionDelta;
                Base::mTarget += positionDelta;
            }
        }
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
        mTargetEuler.x = bookmark.flight.pitch;
        mTargetEuler.y = bookmark.flight.yaw;
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
