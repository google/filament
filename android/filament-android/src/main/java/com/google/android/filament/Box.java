/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

import android.support.annotation.NonNull;
import android.support.annotation.Size;

/**
 * An axis-aligned 3D box represented by its center and half-extent.
 *
 * The half-extent is a vector representing the distance from the center to the edge of the box in
 * each dimension. For example, a box of size 2 units in X, 4 units in Y, and 10 units in Z would
 * have a half-extent of (1, 2, 5).
 */
public class Box {
    private final float[] mCenter = new float[3];
    private final float[] mHalfExtent = new float[3];

    /**
     * Default-initializes the 3D box to have a center and half-extent of (0,0,0).
     */
    public Box() { }

    /**
     * Initializes the 3D box from its center and half-extent.
     */
    public Box(float centerX, float centerY, float centerZ,
               float halfExtentX, float halfExtentY, float halfExtentZ) {
        mCenter[0] = centerX;
        mCenter[1] = centerY;
        mCenter[2] = centerZ;
        mHalfExtent[0] = halfExtentX;
        mHalfExtent[1] = halfExtentY;
        mHalfExtent[2] = halfExtentZ;
    }

    /**
     * Initializes the 3D box from its center and half-extent.
     *
     * @param center     a float array with XYZ coordinates representing the center of the box
     * @param halfExtent a float array with XYZ coordinates representing half the size of the box in
     *                   each dimension
     */
    public Box(@NonNull @Size(min = 3) float[] center, @NonNull @Size(min = 3) float[] halfExtent) {
        mCenter[0] = center[0];
        mCenter[1] = center[1];
        mCenter[2] = center[2];
        mHalfExtent[0] = halfExtent[0];
        mHalfExtent[1] = halfExtent[1];
        mHalfExtent[2] = halfExtent[2];
    }

    /**
     * Sets the center of of the 3D box.
     */
    public void setCenter(float centerX, float centerY, float centerZ) {
        mCenter[0] = centerX;
        mCenter[1] = centerY;
        mCenter[2] = centerZ;
    }

    /**
     * Sets the half-extent of the 3D box.
     */
    public void setHalfExtent(float halfExtentX, float halfExtentY, float halfExtentZ) {
        mHalfExtent[0] = halfExtentX;
        mHalfExtent[1] = halfExtentY;
        mHalfExtent[2] = halfExtentZ;
    }

    /**
     * Returns the center of the 3D box.
     *
     * @return an XYZ float array of size 3
     */
    @NonNull @Size(min = 3)
    public float[] getCenter() { return mCenter; }

    /**
     * Returns the half-extent from the center of the 3D box.
     *
     * @return an XYZ float array of size 3
     */
    @NonNull @Size(min = 3)
    public float[] getHalfExtent() { return mHalfExtent; }
}
