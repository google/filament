/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.google.android.filament.gltfio;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * Updates matrices according to glTF <code>animation</code> and <code>skin</code> definitions.
 *
 * <p>Animator can be used for two things:
 * <ul>
 * <li>Updating matrices in <code>TransformManager</code> components according to glTF <code>animation</code> definitions.</li>
 * <li>Updating bone matrices in <code>RenderableManager</code> components according to glTF <code>skin</code> definitions.</li>
 * </ul>
 * </p>
 *
 * @see AssetLoader
 * @see FilamentAsset
 * @see ResourceLoader
 */
public class Animator {
    private long mNativeObject;

    Animator(long nativeObject) {
        mNativeObject = nativeObject;
    }

    /**
     * Applies rotation, translation, and scale to entities that have been targeted by the given
     * animation definition. Uses <code>TransformManager</code>.
     *
     * @param animationIndex Zero-based index for the <code>animation</code> of interest.
     * @param time Elapsed time of interest in seconds.
     *
     * @see #getAnimationCount
     */
    public void applyAnimation(@IntRange(from = 0) int animationIndex, float time) {
        nApplyAnimation(mNativeObject, animationIndex, time);
    }

    /**
     * Computes root-to-node transforms for all bone nodes, then passes
     * the results into {@see RenderableManager#setBones}.
     * Uses <code>TransformManager</code> and <code>RenderableManager</code>.
     *
     * <p>NOTE: this operation is independent of <code>animation</code>.</p>
     */
    public void updateBoneMatrices() {
        nUpdateBoneMatrices(mNativeObject);
    }

    /**
     * Returns the number of <code>animation</code> definitions in the glTF asset.
     */
    public int getAnimationCount() {
        return nGetAnimationCount(mNativeObject);
    }

    /**
     * Returns the duration of the specified glTF <code>animation</code> in seconds.
     *
     * @param animationIndex Zero-based index for the <code>animation</code> of interest.
     *
     * @see #getAnimationCount
     * */
    public float getAnimationDuration(@IntRange(from = 0) int animationIndex) {
        return nGetAnimationDuration(mNativeObject, animationIndex);
    }

    /**
     * Returns a weak reference to the string name of the specified <code>animation</code>, or an
     * empty string if none was specified.
     *
     * @param animationIndex Zero-based index for the <code>animation</code> of interest.
     *
     * @see #getAnimationCount
     */
    public String getAnimationName(@IntRange(from = 0) int animationIndex) {
        return nGetAnimationName(mNativeObject, animationIndex);
    }

    private static native void nApplyAnimation(long nativeAnimator, int index, float time);
    private static native void nUpdateBoneMatrices(long nativeAnimator);
    private static native int nGetAnimationCount(long nativeAnimator);
    private static native float nGetAnimationDuration(long nativeAnimator, int index);
    private static native String nGetAnimationName(long nativeAnimator, int index);
}
