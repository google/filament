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

package com.google.android.filament.gltfio;

import androidx.annotation.NonNull;

import com.google.android.filament.Entity;

/**
 * Provides access to a hierarchy of entities that have been instanced from a glTF asset.
 *
 * @see FilamentAsset
 * @see Animator
 * @see AssetLoader
 */
public class FilamentInstance {
    private FilamentAsset mAsset;
    private long mNativeObject;
    private Animator mAnimator;

    FilamentInstance(FilamentAsset asset, long nativeObject) {
        mAsset = asset;
        mNativeObject = nativeObject;
        mAnimator = null;
    }

    @SuppressWarnings("unused")
    long getNativeObject() {
        return mNativeObject;
    }

    @SuppressWarnings("unused")
    void clearNativeObject() {
        mNativeObject = 0;
    }

    @SuppressWarnings("unused")
    public @NonNull FilamentAsset getAsset() {
        return mAsset;
    }

    /**
     * Gets the transform root for the asset, which has no matching glTF node.
     */
    @SuppressWarnings("unused")
    public @Entity int getRoot() {
        return nGetRoot(mNativeObject);
    }

    /**
     * Gets the list of entities for this instance, one for each glTF node.
     *
     * <p>All of these have a transform component. Some of the returned entities may also have a
     * renderable component.</p>
     */
    public @NonNull @Entity int[] getEntities() {
        int[] result = new int[nGetEntityCount(mNativeObject)];
        nGetEntities(mNativeObject, result);
        return result;
    }

    /**
     * Creates or retrieves the <code>Animator</code> for this instance.
     *
     * <p>When calling this for the first time, this must be called after
     * {@link ResourceLoader#loadResources}.</p>
     */
    public @NonNull Animator getAnimator() {
        if (mAnimator != null) {
            return mAnimator;
        }
        mAnimator = new Animator(nGetAnimator(mNativeObject));
        return mAnimator;
    }

    private static native int nGetRoot(long nativeAsset);
    private static native int nGetEntityCount(long nativeAsset);
    private static native void nGetEntities(long nativeAsset, int[] result);
    private static native long nGetAnimator(long nativeAsset);
}
