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
 * its getLookAt() method so that they can adjust their camera(s). Two modes are supported: ORBIT
 * and MAP. To construct a manipulator instance, the desired mode is passed into the create method.
 *
 * @see Bookmark
 */
public class Manipulator {
    private long mNativeObject;

    private Manipulator(long nativeIndexBuffer) {
        mNativeObject = nativeIndexBuffer;
    }

    public enum Mode { ORBIT, MAP };

    public enum Fov { VERTICAL, HORIZONTAL };

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
    public Mode getMode() { return Mode.values()[nGetMode(mNativeObject)]; }

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
     * This starts a panning session in MAP mode, and start either rotating or strafing in ORBIT.
     *
     * @param x X-coordinate for point of interest in viewport space
     * @param y Y-coordinate for point of interest in viewport space
     * @param strafe ORBIT mode only; if true, starts a translation rather than a rotation.
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
     * Dollys the camera along the viewing direction.
     *
     * @param x X-coordinate for point of interest in viewport space
     * @param y Y-coordinate for point of interest in viewport space
     * @param scrolldelta Positive means "zoom in", negative means "zoom out"
     */
    public void zoom(int x, int y, float scrolldelta) {
        nZoom(mNativeObject, x, y, scrolldelta);
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
    private static native void nZoom(long nativeManip, int x, int y, float scrolldelta);
    private static native long nGetCurrentBookmark(long nativeManip);
    private static native long nGetHomeBookmark(long nativeManip);
    private static native void nJumpToBookmark(long nativeManip, long nativeBookmark);
}
