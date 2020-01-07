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

#ifndef CAMUTILS_MANIPULATOR_H
#define CAMUTILS_MANIPULATOR_H

#include <camutils/Bookmark.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <cstdint>

namespace filament {
namespace camutils {

enum class Fov { VERTICAL, HORIZONTAL };

/**
 * Helper that enables camera interaction similar to sketchfab or Google Maps.
 *
 * Clients notify the camera manipulator of various mouse or touch events, then periodically call
 * its getLookAt() method so that they can adjust their camera(s). Two modes are supported: ORBIT
 * and MAP. To construct a manipulator instance, the desired mode is passed into the create method.
 *
 * Usage example:
 *
 *     using CameraManipulator = camutils::Manipulator<float>;
 *     CameraManipulator* manip;
 *
 *     void init() {
 *         manip = CameraManipulator::create(camutils::Mode::ORBIT, {
 *             .viewport = {1024, 768}
 *         });
 *     }
 *
 *     void onMouseDown(int x, int y) {
 *         manip->grabBegin(x, y, false);
 *     }
 *
 *     void onMouseMove(int x, int y) {
 *         manip->grabUpdate(x, y);
 *     }
 *
 *     void onMouseUp(int x, int y) {
 *         manip->grabEnd();
 *     }
 *
 *     void gameLoop() {
 *         while (true) {
 *             filament::math::float3 eye, center, up;
 *             manip->getLookAt(&eye, &center, &up);
 *             camera->lookAt(eye, center, up);
 *             render();
 *         }
 *     }
 *
 * @see Bookmark
 */
template <typename FLOAT>
class Manipulator {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;

    /** Opaque handle to a viewing position and orientation to facilitate camera animation. */
    using Bookmark = filament::camutils::Bookmark<FLOAT>;

    /** Optional raycasting function to enable perspective-correct panning. */
    typedef bool (*RayCallback)(const vec3& origin, const vec3& dir, FLOAT* t, void* userdata);

    /**
     * User-controlled configuration that is never changed by the manipulator.
     *
     * This is a Builder expressed as a POD structure. Many of the fields fall back to reasonable
     * default values if they are zero-filled.
     */
    struct Config {
        int viewport[2];             //! Width and height of the viewing area
        vec3 targetPosition;         //! World-space position of interest, defaults to (0,0,0)
        vec3 upVector;               //! Orientation for the home position, defaults to (0,1,0)
        FLOAT zoomSpeed;             //! Multiplied with scroll delta, defaults to 0.01

        // Orbit mode properties
        vec3 orbitHomePosition;      //! Initial eye position in world space, defaults to (0,0,1)
        vec2 orbitSpeed;             //! Multiplied with viewport delta, defaults to 0.01

        // Map mode properties
        Fov fovDirection;           //! The FOV axis that's held constant when the viewport changes
        FLOAT fovDegrees;           //! The full FOV (not the half-angle)
        FLOAT farPlane;             //! The distance to the far plane
        vec2 mapExtent;             //! The ground plane size used to compute the home position
        FLOAT mapMinDistance;       //! Constrains the zoom-in level

        // Raycast properties
        vec4 groundPlane;            //! Plane equation used as a raycast fallback
        RayCallback raycastCallback; //! Raycast function for accurate grab-and-pan
        void* raycastUserdata;       //! Callback userdata for closures
    };

    /**
     * Creates a new camera manipulator, either ORBIT or MAP.
     *
     * Clients can simply use "delete" to destroy the manipulator.
     */
    static Manipulator* create(Mode mode, const Config& props);

    virtual ~Manipulator() = default;

    /**
     * Gets the immutable mode of the manipulator.
     */
    Mode getMode() const { return mMode; }

    /**
     * Sets the viewport dimensions. The manipulator uses this to process grab events and raycasts.
     */
    void setViewport(int width, int height);

    /**
     * Gets the current orthonormal basis; this is usually called once per frame.
     */
    void getLookAt(vec3* eyePosition, vec3* targetPosition, vec3* upward) const;

    /**
     * Given a viewport coordinate, picks a point in the ground plane, or in the actual scene if the
     * raycast callback was provided.
     */
    bool raycast(int x, int y, vec3* result) const;

    /**
     * Given a viewport coordinate, computes a picking ray (origin + direction).
     */
    void getRay(int x, int y, vec3* origin, vec3* dir) const;

    /**
     * Starts a grabbing session (i.e. the user is dragging around in the viewport).
     *
     * This starts a panning session in MAP mode, and starts either rotating or strafing in ORBIT.
     *
     * @param x X-coordinate for point of interest in viewport space
     * @param y Y-coordinate for point of interest in viewport space
     * @param strafe ORBIT mode only; if true, starts a translation rather than a rotation
     */
    virtual void grabBegin(int x, int y, bool strafe) = 0;

    /**
     * Updates a grabbing session.
     *
     * This must be called at least once between grabBegin / grabEnd to dirty the camera.
     */
    virtual void grabUpdate(int x, int y) = 0;

    /**
     * Ends a grabbing session.
     */
    virtual void grabEnd() = 0;

    /**
     * Dollys the camera along the viewing direction.
     *
     * @param x X-coordinate for point of interest in viewport space
     * @param y Y-coordinate for point of interest in viewport space
     * @param scrolldelta Negative means "zoom in", positive means "zoom out"
     */
    virtual void zoom(int x, int y, FLOAT scrolldelta) = 0;

    /**
     * Gets a handle that can be used to reset the manipulator back to its current position.
     *
     * @see jumpToBookmark
     */
    virtual Bookmark getCurrentBookmark() const = 0;

    /**
     * Gets a handle that can be used to reset the manipulator back to its home position.
     *
     * @see jumpToBookmark
     */
    virtual Bookmark getHomeBookmark() const = 0;

    /**
     * Sets the manipulator position and orientation back to a stashed state.
     *
     * @see getCurrentBookmark, getHomeBookmark
     */
    virtual void jumpToBookmark(const Bookmark& bookmark) = 0;

protected:
    Manipulator(Mode mode, const Config& props);

    virtual void setProperties(const Config& props);

    vec3 raycastFarPlane(int x, int y) const;

    const Mode mMode;
    Config mProps;
    vec3 mEye;
    vec3 mTarget;
};

} // namespace camutils
} // namespace filament

#endif /* CAMUTILS_MANIPULATOR_H */
