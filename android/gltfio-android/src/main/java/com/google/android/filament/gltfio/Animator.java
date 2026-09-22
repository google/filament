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
import java.nio.Buffer;

/**
 * Updates matrices according to glTF <code>animation</code> and <code>skin</code> definitions.
 *
 * <p>Animator is owned by <code>FilamentAsset</code> and can be used for two things:
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
    private boolean mIsOwner = false;

    Animator(long nativeObject) {
        mNativeObject = nativeObject;
    }

    /**
     * Creates an Animator that can manipulate animations in the provided FilamentAsset.
     * <p>
     * <strong>Important:</strong> This Animator manages native resources that must be 
     * explicitly freed by calling {@link #destroy()} when no longer needed. Failing to 
     * call {@link #destroy()} will result in native memory leaks.
     * </p>
     *
     * @param asset The FilamentAsset containing the animations
     * @param instance The FilamentInstance to animate
     */
    public Animator(FilamentAsset asset, FilamentInstance instance) {
        mNativeObject = nCreateAnimatorFromAssetAndInstance(asset.getNativeObject(), instance.getNativeObject());
        mIsOwner = true;
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
        nApplyAnimation(getNativeObject(), animationIndex, time);
    }

    /**
     * Computes root-to-node transforms for all bone nodes, then passes the results into
     * {@link com.google.android.filament.RenderableManager#setBonesAsMatrices(int, Buffer, int, int)}.
     * Uses <code>TransformManager</code> and <code>RenderableManager</code>.
     *
     * <p>NOTE: this operation is independent of <code>animation</code>.</p>
     */
    public void updateBoneMatrices() {
        nUpdateBoneMatrices(getNativeObject());
    }

    /**
     * Applies a blended transform to the union of nodes affected by two animations.
     * Used for cross-fading from a previous skinning-based animation or rigid body animation.
     *
     * First, this stashes the current transform hierarchy into a transient memory buffer.
     *
     * Next, this applies previousAnimIndex / previousAnimTime to the actual asset by internally
     * calling applyAnimation().
     *
     * Finally, the stashed local transforms are lerped (via the scale / translation / rotation
     * components) with their live counterparts, and the results are pushed to the asset.
     *
     * To achieve a cross fade effect with skinned models, clients will typically call animator
     * methods in this order: (1) applyAnimation (2) applyCrossFade (3) updateBoneMatrices. The
     * animation that clients pass to applyAnimation is the "current" animation corresponding to
     * alpha=1, while the "previous" animation passed to applyCrossFade corresponds to alpha=0.
     */
    public void applyCrossFade(int previousAnimIndex, float previousAnimTime, float alpha) {
        nApplyCrossFade(getNativeObject(), previousAnimIndex, previousAnimTime, alpha);
    }

    /**
     * Pass the identity matrix into all bone nodes, useful for returning to the T pose.
     *
     * <p>NOTE: this operation is independent of <code>animation</code>.</p>
     */
    public void resetBoneMatrices() {
        nResetBoneMatrices(getNativeObject());
    }

    /**
     * Returns the number of <code>animation</code> definitions in the glTF asset.
     */
    public int getAnimationCount() {
        return nGetAnimationCount(getNativeObject());
    }

    /**
     * Returns the duration of the specified glTF <code>animation</code> in seconds.
     *
     * @param animationIndex Zero-based index for the <code>animation</code> of interest.
     *
     * @see #getAnimationCount
     * */
    public float getAnimationDuration(@IntRange(from = 0) int animationIndex) {
        return nGetAnimationDuration(getNativeObject(), animationIndex);
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
        return nGetAnimationName(getNativeObject(), animationIndex);
    }

    /**
     * Explicitly destroys this Animator and frees all its associated native resources.
     * The object will be unusable after this call.
     */
    public void destroy() {
        if (mNativeObject != 0) {
            if (mIsOwner) {
                nDestroyAnimator(mNativeObject);
            }
            mNativeObject = 0;
        }
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Using Animator on destroyed asset");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateAnimatorFromAssetAndInstance(long nativeAsset, long nativeInstance);
    private static native void nDestroyAnimator(long nativeAnimator);
    private static native void nApplyAnimation(long nativeAnimator, int index, float time);
    private static native void nUpdateBoneMatrices(long nativeAnimator);
    private static native void nApplyCrossFade(long nativeAnimator, int animIndex, float animTime, float alpha);
    private static native void nResetBoneMatrices(long nativeAnimator);
    private static native int nGetAnimationCount(long nativeAnimator);
    private static native float nGetAnimationDuration(long nativeAnimator, int index);
    private static native String nGetAnimationName(long nativeAnimator, int index);
}
