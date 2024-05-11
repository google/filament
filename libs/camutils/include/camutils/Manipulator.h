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
#include <camutils/compiler.h>

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
 * its getLookAt() method so that they can adjust their camera(s). Three modes are supported: ORBIT,
 * MAP, and FREE_FLIGHT. To construct a manipulator instance, the desired mode is passed into the
 * create method.
 *
 * Usage example:
 *
 *     using CameraManipulator = camutils::Manipulator<float>;
 *     CameraManipulator* manip;
 *
 *     void init() {
 *         manip = CameraManipulator::Builder()
 *             .viewport(1024, 768)
 *             .build(camutils::Mode::ORBIT);
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
class CAMUTILS_PUBLIC Manipulator {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;

    /** Opaque handle to a viewing position and orientation to facilitate camera animation. */
    using Bookmark = filament::camutils::Bookmark<FLOAT>;

    /** Optional raycasting function to enable perspective-correct panning. */
    typedef bool (*RayCallback)(const vec3& origin, const vec3& dir, FLOAT* t, void* userdata);

    /** Builder state, direct access is allowed but Builder methods are preferred. **/
    struct Config {
        int viewport[2];
        vec3 targetPosition;
        vec3 upVector;
        FLOAT zoomSpeed;
        vec3 orbitHomePosition;
        vec2 orbitSpeed;
        Fov fovDirection;
        FLOAT fovDegrees;
        FLOAT farPlane;
        vec2 mapExtent;
        FLOAT mapMinDistance;
        vec3 flightStartPosition;
        FLOAT flightStartPitch;
        FLOAT flightStartYaw;
        FLOAT flightMaxSpeed;
        FLOAT flightSpeedSteps;
        vec2 flightPanSpeed;
        FLOAT flightMoveDamping;
        vec4 groundPlane;
        RayCallback raycastCallback;
        void* raycastUserdata;
    };

    struct Builder {
        // Common properties
        Builder& viewport(int width, int height);           //! Width and height of the viewing area
        Builder& targetPosition(FLOAT x, FLOAT y, FLOAT z); //! World-space position of interest, defaults to (0,0,0)
        Builder& upVector(FLOAT x, FLOAT y, FLOAT z);       //! Orientation for the home position, defaults to (0,1,0)
        Builder& zoomSpeed(FLOAT val);                      //! Multiplied with scroll delta, defaults to 0.01

        // Orbit mode properties
        Builder& orbitHomePosition(FLOAT x, FLOAT y, FLOAT z); //! Initial eye position in world space, defaults to (0,0,1)
        Builder& orbitSpeed(FLOAT x, FLOAT y);                 //! Multiplied with viewport delta, defaults to 0.01

        // Map mode properties
        Builder& fovDirection(Fov fov);                          //! The axis that's held constant when viewport changes
        Builder& fovDegrees(FLOAT degrees);                      //! The full FOV (not the half-angle)
        Builder& farPlane(FLOAT distance);                       //! The distance to the far plane
        Builder& mapExtent(FLOAT worldWidth, FLOAT worldHeight); //! The ground size for computing home position
        Builder& mapMinDistance(FLOAT mindist);                  //! Constrains the zoom-in level

        // Free flight properties
        Builder& flightStartPosition(FLOAT x, FLOAT y, FLOAT z);     //! Initial eye position in world space, defaults to (0,0,0)
        Builder& flightStartOrientation(FLOAT pitch, FLOAT yaw);     //! Initial orientation in pitch and yaw, defaults to (0,0)
        Builder& flightMaxMoveSpeed(FLOAT maxSpeed);                 //! The maximum camera speed in world units per second, defaults to 10
        Builder& flightSpeedSteps(int steps);                        //! The number of speed steps adjustable with scroll wheel, defaults to 80
        Builder& flightPanSpeed(FLOAT x, FLOAT y);                   //! Multiplied with viewport delta, defaults to 0.01,0.01
        Builder& flightMoveDamping(FLOAT damping);                   //! Applies a deceleration to camera movement, defaults to 0 (no damping)
                                                                     //! Lower values give slower damping times, a good default is 15
                                                                     //! Too high a value may lead to instability

        // Raycast properties
        Builder& groundPlane(FLOAT a, FLOAT b, FLOAT c, FLOAT d);  //! Plane equation used as a raycast fallback
        Builder& raycastCallback(RayCallback cb, void* userdata);  //! Raycast function for accurate grab-and-pan

        /**
         * Creates a new camera manipulator, either ORBIT, MAP, or FREE_FLIGHT.
         *
         * Clients can simply use "delete" to destroy the manipulator.
         */
        Manipulator* build(Mode mode);

        Config details = {};
    };

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
     * In MAP mode, this starts a panning session.
     * In ORBIT mode, this starts either rotating or strafing.
     * In FREE_FLIGHT mode, this starts a nodal panning session.
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
     * Keys used to translate the camera in FREE_FLIGHT mode.
     * FORWARD and BACKWARD dolly the camera forwards and backwards.
     * LEFT and RIGHT strafe the camera left and right.
     * UP and DOWN boom the camera upwards and downwards.
     */
    enum class Key {
        FORWARD,
        LEFT,
        BACKWARD,
        RIGHT,
        UP,
        DOWN,

        COUNT
    };

    /**
     * Signals that a key is now in the down state.
     *
     * In FREE_FLIGHT mode, the camera is translated forward and backward and strafed left and right
     * depending on the depressed keys. This allows WASD-style movement.
     */
    virtual void keyDown(Key key);

    /**
     * Signals that a key is now in the up state.
     *
     * @see keyDown
     */
    virtual void keyUp(Key key);

    /**
     * In MAP and ORBIT modes, dollys the camera along the viewing direction.
     * In FREE_FLIGHT mode, adjusts the move speed of the camera.
     *
     * @param x X-coordinate for point of interest in viewport space, ignored in FREE_FLIGHT mode
     * @param y Y-coordinate for point of interest in viewport space, ignored in FREE_FLIGHT mode
     * @param scrolldelta In MAP and ORBIT modes, negative means "zoom in", positive means "zoom out"
     *                    In FREE_FLIGHT mode, negative means "slower", positive means "faster"
     */
    virtual void scroll(int x, int y, FLOAT scrolldelta) = 0;

    /**
     * Processes input and updates internal state.
     *
     * This must be called once every frame before getLookAt is valid.
     *
     * @param deltaTime The amount of time, in seconds, passed since the previous call to update.
     */
    virtual void update(FLOAT deltaTime);

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
