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

#ifndef CAMUTILS_ORBIT_MANIPULATOR_H
#define CAMUTILS_ORBIT_MANIPULATOR_H

#include <camutils/Manipulator.h>

#include <math/scalar.h>

#define MAX_PHI (F_PI / 2.0 - 0.001)

namespace filament {
namespace camutils {

using namespace filament::math;

template<typename FLOAT>
class OrbitManipulator : public Manipulator<FLOAT> {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;
    using Bookmark = filament::camutils::Bookmark<FLOAT>;
    using Base = Manipulator<FLOAT>;
    using Config = typename Base::Config;

    enum GrabState { INACTIVE, ORBITING, PANNING };

    OrbitManipulator(Mode mode, const Config& props) : Base(mode, props) {
        setProperties(props);
        Base::mEye = Base::mProps.orbitHomePosition;
        mPivot = Base::mTarget = Base::mProps.targetPosition;
    }

    void setProperties(const Config& props) override {
        Config resolved = props;

        if (resolved.orbitHomePosition == vec3(0)) {
            resolved.orbitHomePosition = vec3(0, 0, 1);
        }

        if (resolved.orbitSpeed == vec2(0)) {
            resolved.orbitSpeed = vec2(0.01);
        }

        // By default, place the ground plane so that it aligns with the targetPosition position.
        // This is used only when PANNING.
        if (resolved.groundPlane == vec4(0)) {
            const FLOAT d = length(resolved.targetPosition);
            const vec3 n = normalize(resolved.orbitHomePosition - resolved.targetPosition);
            resolved.groundPlane = vec4(n, -d);
        }

        Base::setProperties(resolved);
    }

    void grabBegin(int x, int y, bool strafe) override {
        mGrabState = strafe && Base::mProps.panning ? PANNING : ORBITING;
        mGrabPivot = mPivot;
        mGrabEye = Base::mEye;
        mGrabTarget = Base::mTarget;
        mGrabBookmark = getCurrentBookmark();
        mGrabWinX = x;
        mGrabWinY = y;
        mGrabFar = Base::raycastFarPlane(x, y);
        Base::raycast(x, y, &mGrabScene);
    }

    void grabUpdate(int x, int y) override {
        const int delx = mGrabWinX - x;
        const int dely = mGrabWinY - y;

        if (mGrabState == ORBITING) {
            Bookmark bookmark = getCurrentBookmark();

            const FLOAT theta = delx * Base::mProps.orbitSpeed.x;
            const FLOAT phi = dely * Base::mProps.orbitSpeed.y;
            const FLOAT maxPhi = MAX_PHI;

            bookmark.orbit.phi = clamp(mGrabBookmark.orbit.phi + phi, -maxPhi, +maxPhi);
            bookmark.orbit.theta = mGrabBookmark.orbit.theta + theta;

            jumpToBookmark(bookmark);
        }

        if (mGrabState == PANNING) {
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
        const vec3 v1 = mPivot - Base::mEye;

        // Check if the camera has moved past the point of interest.
        if (dot(v0, v1) < 0) {
            mFlipped = !mFlipped;
        }
    }

    Bookmark getCurrentBookmark() const override {
        Bookmark bookmark;
        bookmark.mode = Mode::ORBIT;
        const vec3 pivotToEye = Base::mEye - mPivot;
        const FLOAT d = length(pivotToEye);
        const FLOAT x = pivotToEye.x / d;
        const FLOAT y = pivotToEye.y / d;
        const FLOAT z = pivotToEye.z / d;

        bookmark.orbit.phi = asin(y);
        bookmark.orbit.theta = atan2(x, z);
        bookmark.orbit.distance = mFlipped ? -d : d;
        bookmark.orbit.pivot = mPivot;

        const FLOAT fov = Base::mProps.fovDegrees * math::F_PI / 180.0;
        const FLOAT halfExtent = d * tan(fov / 2.0);
        const vec3 targetToEye = Base::mProps.groundPlane.xyz;
        const vec3 uvec = cross(Base::mProps.upVector, targetToEye);
        const vec3 vvec = cross(targetToEye, uvec);
        const vec3 centerToTarget = mPivot - Base::mProps.targetPosition;

        bookmark.map.extent = halfExtent * 2;
        bookmark.map.center.x = dot(uvec, centerToTarget);
        bookmark.map.center.y = dot(vvec, centerToTarget);

        return bookmark;
    }

    Bookmark getHomeBookmark() const override {
        Bookmark bookmark;
        bookmark.mode = Mode::ORBIT;
        bookmark.orbit.phi = FLOAT(0);
        bookmark.orbit.theta = FLOAT(0);
        bookmark.orbit.pivot = Base::mProps.targetPosition;
        bookmark.orbit.distance = distance(Base::mProps.targetPosition, Base::mProps.orbitHomePosition);

        const FLOAT fov = Base::mProps.fovDegrees * math::F_PI / 180.0;
        const FLOAT halfExtent = bookmark.orbit.distance * tan(fov / 2.0);

        bookmark.map.extent = halfExtent * 2;
        bookmark.map.center.x = 0;
        bookmark.map.center.y = 0;

        return bookmark;
    }

    void jumpToBookmark(const Bookmark& bookmark) override {
        mPivot = bookmark.orbit.pivot;
        const FLOAT x = sin(bookmark.orbit.theta) * cos(bookmark.orbit.phi);
        const FLOAT y = sin(bookmark.orbit.phi);
        const FLOAT z = cos(bookmark.orbit.theta) * cos(bookmark.orbit.phi);
        Base::mEye = mPivot + vec3(x, y, z) * abs(bookmark.orbit.distance);
        mFlipped = bookmark.orbit.distance < 0;
        Base::mTarget = Base::mEye + vec3(x, y, z) * (mFlipped ? 1.0 : -1.0);
    }

private:
    GrabState mGrabState = INACTIVE;
    bool mFlipped = false;
    vec3 mGrabPivot;
    vec3 mGrabScene;
    vec3 mGrabFar;
    vec3 mGrabEye;
    vec3 mGrabTarget;
    Bookmark mGrabBookmark;
    int mGrabWinX;
    int mGrabWinY;
    vec3 mPivot;
};

} // namespace camutils
} // namespace filament

#endif /* CAMUTILS_ORBIT_MANIPULATOR_H */
