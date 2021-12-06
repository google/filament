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

package com.google.android.filament.utils;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

/**
 * Helper that enables camera interaction similar to sketchfab or Google Maps.
 *
 * Clients notify the camera manipulator of various mouse or touch events, then periodically call
 * its getLookAt() method so that they can adjust their camera(s). Three modes are supported: ORBIT,
 * MAP, and FREE_FLIGHT. To construct a manipulator instance, the desired mode is passed into the
 * create method.
 *
 * @see Bookmark
 */
public class Manipulator {
    private static final Mode[] sModeValues = Mode.values();

    private final long mNativeObject;

    private Manipulator(long nativeIndexBuffer) {
        mNativeObject = nativeIndexBuffer;
    }

    public enum Mode { ORBIT, MAP, FREE_FLIGHT };

    public enum Fov { VERTICAL, HORIZONTAL };

    /**
     * Keys used to translate the camera in FREE_FLIGHT mode.
     * UP and DOWN dolly the camera forwards and backwards.
     * LEFT and RIGHT strafe the camera left and right.
     */
    public enum Key {
        FORWARD,
        LEFT,
        BACKWARD,
        RIGHT,
        UP,
        DOWN
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Width and height of the viewing area.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder viewport(@IntRange(from = 1) int width, @IntRange(from = 1) int height) {
            nBuilderViewport(mNativeBuilder, width, height);
            return this;
        }

        /**
         * Sets world-space position of interest, which defaults to (0,0,0).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder targetPosition(float x, float y, float z) {
            nBuilderTargetPosition(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets orientation for the home position, which defaults to (0,1,0).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder upVector(float x, float y, float z) {
            nBuilderUpVector(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets the scroll delta multiplier, which defaults to 0.01.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder zoomSpeed(float arg) {
            nBuilderZoomSpeed(mNativeBuilder, arg);
            return this;
        }

        /**
         * Sets initial eye position in world space for ORBIT mode.
         * This defaults to (0,0,1).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder orbitHomePosition(float x, float y, float z) {
            nBuilderOrbitHomePosition(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets the multiplier with viewport delta for ORBIT mode.
         * This defaults to 0.01
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder orbitSpeed(float x, float y) {
            nBuilderOrbitSpeed(mNativeBuilder, x, y);
            return this;
        }

        /**
         * Sets the FOV axis that's held constant when the viewport changes.
         * This defaults to Vertical.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder fovDirection(Fov fov) {
            nBuilderFovDirection(mNativeBuilder, fov.ordinal());
            return this;
        }

        /**
         * Sets the full FOV (not the half-angle) in the degrees.
         * This defaults to 33.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder fovDegrees(float arg) {
            nBuilderFovDegrees(mNativeBuilder, arg);
            return this;
        }

        /**
         * Sets the distance to the far plane, which defaults to 5000.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder farPlane(float arg) {
            nBuilderFarPlane(mNativeBuilder, arg);
            return this;
        }

        /**
         * Sets the ground plane size used to compute the home position for MAP mode.
         * This defaults to 512 x 512
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder mapExtent(float width, float height) {
            nBuilderMapExtent(mNativeBuilder, width, height);
            return this;
        }

        /**
         * Constrains the zoom-in level. Defaults to 0.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder mapMinDistance(float arg) {
            nBuilderMapMinDistance(mNativeBuilder, arg);
            return this;
        }

        /**
         * Sets the initial eye position in world space for FREE_FLIGHT mode. Defaults to (0,0,0).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        public Builder flightStartPosition(float x, float y, float z) {
            nBuilderFlightStartPosition(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets the initial orientation in pitch and yaw for FREE_FLIGHT mode. Defaults to (0,0).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        public Builder flightStartOrientation(float pitch, float yaw) {
            nBuilderFlightStartOrientation(mNativeBuilder, pitch, yaw);
            return this;
        }

        /**
         * Sets the maximum camera translation speed in world units per second for FREE_FLIGHT mode.
         * Defaults to 10.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        public Builder flightMaxMoveSpeed(float maxSpeed) {
            nBuilderFlightMaxMoveSpeed(mNativeBuilder, maxSpeed);
            return this;
        }

        /**
         * Sets the number of speed steps adjustable with scroll wheel for FREE_FLIGHT mode.
         * Defaults to 80.
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        public Builder flightSpeedSteps(int steps) {
            nBuilderFlightSpeedSteps(mNativeBuilder, steps);
            return this;
        }

       /**
        * Sets the multiplier with viewport delta for FREE_FLIGHT mode.
        * This defaults to 0.01.
        *
        * @return this <code>Builder</code> object for chaining calls
        */
        public Builder flightPanSpeed(float x, float y) {
            nBuilderFlightPanSpeed(mNativeBuilder, x, y);
            return this;
        }

       /**
        * Applies a deceleration to camera movement in FREE_FLIGHT mode. Defaults to 0 (no damping).
        *
        * Lower values give slower damping times. A good default is 15.0. Too high a value may lead
        * to instability.
        *
        * @return this <code>Builder</code> object for chaining calls
        */
        public Builder flightMoveDamping(float damping) {
            nBuilderFlightMoveDamping(mNativeBuilder, damping);
            return this;
        }

        /**
         * Sets the ground plane equation used for ray casts.
         * This is a plane equation as in Ax + By + Cz + D = 0.
         * Defaults to (0, 0, 1, 0).
         *
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder groundPlane(float a, float b, float c, float d) {
            nBuilderGroundPlane(mNativeBuilder, a, b, c, d);
            return this;
        }

        /**
         * Creates and returns the <code>Manipulator</code> object.
         *
         * @return the newly created <code>Manipulator</code> object
         *
         * @exception IllegalStateException if the Manipulator could not be created
         *
         */
        @NonNull
        public Manipulator build(Mode mode) {
            long nativeManipulator = nBuilderBuild(mNativeBuilder, mode.ordinal());
            if (nativeManipulator == 0)
                throw new IllegalStateException("Couldn't create Manipulator");
            return new Manipulator(nativeManipulator);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    };

    @Override
    public void finalize() {
        try {
            super.finalize();
        } catch (Throwable t) { // Ignore
        } finally {
            nDestroyManipulator(mNativeObject);
        }
    }

    /**
     * Gets the immutable mode of the manipulator.
     */
    public Mode getMode() { return sModeValues[nGetMode(mNativeObject)]; }

    /**
     * Sets the viewport dimensions in terms of pixels.
     *
     * The manipulator uses this only in the grab and raycast methods, since
     * those methods consume coordinates in viewport space.
     */
    public void setViewport(int width, int height) {
        nSetViewport(mNativeObject, width, height);
    }

    /**
     * Gets the current orthonormal basis. This is usually called once per frame.
     */
    public void getLookAt(
            @NonNull @Size(min = 3) float[] eyePosition,
            @NonNull @Size(min = 3) float[] targetPosition,
            @NonNull @Size(min = 3) float[] upward) {
        nGetLookAtFloat(mNativeObject, eyePosition, targetPosition, upward);
    }

    public void getLookAt(
            @NonNull @Size(min = 3) double[] eyePosition,
            @NonNull @Size(min = 3) double[] targetPosition,
            @NonNull @Size(min = 3) double[] upward) {
        nGetLookAtDouble(mNativeObject, eyePosition, targetPosition, upward);
    }

    /**
     * Given a viewport coordinate, picks a point in the ground plane.
     */
    @Nullable @Size(min = 3)
    public float[] raycast(int x, int y) {
        float[] result = new float[3];
        nRaycast(mNativeObject, x, y, result);
        return result;
    }

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
    public void grabBegin(int x, int y, boolean strafe) {
        nGrabBegin(mNativeObject, x, y, strafe);
    }

    /**
     * Updates a grabbing session.
     *
     * This must be called at least once between grabBegin / grabEnd to dirty the camera.
     */
    public void grabUpdate(int x, int y) {
        nGrabUpdate(mNativeObject, x, y);
    }

    /**
     * Ends a grabbing session.
     */
    public void grabEnd() {
        nGrabEnd(mNativeObject);
    }

    /**
     * Keys used to translate the camera in FREE_FLIGHT mode.
     * UP and DOWN dolly the camera forwards and backwards.
     * LEFT and RIGHT strafe the camera left and right.
     * UP and DOWN boom the camera upwards and downwards.
     */
    public void keyDown(Key key) {
        nKeyDown(mNativeObject, key.ordinal());
    }

    /**
     * Signals that a key is now in the up state.
     *
     * @see keyDown
     */
    public void keyUp(Key key) {
        nKeyUp(mNativeObject, key.ordinal());
    }

    /**
     * In MAP and ORBIT modes, dollys the camera along the viewing direction.
     * In FREE_FLIGHT mode, adjusts the move speed of the camera.
     *
     * @param x X-coordinate for point of interest in viewport space, ignored in FREE_FLIGHT mode
     * @param y Y-coordinate for point of interest in viewport space, ignored in FREE_FLIGHT mode
     * @param scrolldelta In MAP and ORBIT modes, negative means "zoom in", positive means "zoom out"
     *                    In FREE_FLIGHT mode, negative means "slower", positive means "faster"
     */
    public void scroll(int x, int y, float scrolldelta) {
        nScroll(mNativeObject, x, y, scrolldelta);
    }

    /**
     * Processes input and updates internal state.
     *
     * This must be called once every frame before getLookAt is valid.
     *
     * @param deltaTime The amount of time, in seconds, passed since the previous call to update.
     */
    public void update(float deltaTime) {
        nUpdate(mNativeObject, deltaTime);
    }

    /**
     * Gets a handle that can be used to reset the manipulator back to its current position.
     *
     * @see #jumpToBookmark(Bookmark)
     */
    public Bookmark getCurrentBookmark() {
        return new Bookmark(nGetCurrentBookmark(mNativeObject));
    }

    /**
     * Gets a handle that can be used to reset the manipulator back to its home position.
     *
     * see jumpToBookmark
     */
    public Bookmark getHomeBookmark() {
        return new Bookmark(nGetHomeBookmark(mNativeObject));
    }

    /**
     * Sets the manipulator position and orientation back to a stashed state.
     *
     * @see #getCurrentBookmark()
     * @see #getHomeBookmark()
     */
    public void jumpToBookmark(Bookmark bookmark) {
        nJumpToBookmark(mNativeObject, bookmark.getNativeObject());
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native void nBuilderViewport(long nativeBuilder, int width, int height);
    private static native void nBuilderTargetPosition(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderUpVector(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderZoomSpeed(long nativeBuilder, float arg);
    private static native void nBuilderOrbitHomePosition(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderOrbitSpeed(long nativeBuilder, float x, float y);
    private static native void nBuilderFovDirection(long nativeBuilder, int arg);
    private static native void nBuilderFovDegrees(long nativeBuilder, float arg);
    private static native void nBuilderFarPlane(long nativeBuilder, float distance);
    private static native void nBuilderMapExtent(long nativeBuilder, float width, float height);
    private static native void nBuilderMapMinDistance(long nativeBuilder, float arg);
    private static native void nBuilderFlightStartPosition(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderFlightStartOrientation(long nativeBuilder, float pitch, float yaw);
    private static native void nBuilderFlightMaxMoveSpeed(long nativeBuilder, float maxSpeed);
    private static native void nBuilderFlightSpeedSteps(long nativeBuilder, int steps);
    private static native void nBuilderFlightPanSpeed(long nativeBuilder, float x, float y);
    private static native void nBuilderFlightMoveDamping(long nativeBuilder, float damping);
    private static native void nBuilderGroundPlane(long nativeBuilder, float a, float b, float c, float d);
    private static native long nBuilderBuild(long nativeBuilder, int mode);

    private static native void nDestroyManipulator(long nativeManip);
    private static native int nGetMode(long nativeManip);
    private static native void nSetViewport(long nativeManip, int width, int height);
    private static native void nGetLookAtFloat(long nativeManip, float[] eyePosition, float[] targetPosition, float[] upward);
    private static native void nGetLookAtDouble(long nativeManip, double[] eyePosition, double[] targetPosition, double[] upward);
    private static native void nRaycast(long nativeManip, int x, int y, float[] result);
    private static native void nGrabBegin(long nativeManip, int x, int y, boolean strafe);
    private static native void nGrabUpdate(long nativeManip, int x, int y);
    private static native void nGrabEnd(long nativeManip);
    private static native void nKeyDown(long nativeManip, int key);
    private static native void nKeyUp(long nativeManip, int key);
    private static native void nScroll(long nativeManip, int x, int y, float scrolldelta);
    private static native void nUpdate(long nativeManip, float deltaTime);
    private static native long nGetCurrentBookmark(long nativeManip);
    private static native long nGetHomeBookmark(long nativeManip);
    private static native void nJumpToBookmark(long nativeManip, long nativeBookmark);
}
