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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import com.google.android.filament.Engine;
import com.google.android.filament.Entity;
import com.google.android.filament.MaterialInstance;

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
     * Retrieves the <code>Animator</code> for this instance.
     */
    public @NonNull Animator getAnimator() {
        if (mAnimator != null) {
            return mAnimator;
        }
        mAnimator = new Animator(nGetAnimator(mNativeObject));
        return mAnimator;
    }

    /**
     * Gets the skin count of this instance.
     */
    public int getSkinCount() {
        return nGetSkinCount(getNativeObject());
    }

    /**
     * Gets the skin name at skin index in this instance.
     */
    public @NonNull String[] getSkinNames() {
        String[] result = new String[getSkinCount()];
        nGetSkinNames(getNativeObject(), result);
        return result;
    }

    /**
     * Attaches the given skin to the given node, which must have an associated mesh with
     * BONE_INDICES and BONE_WEIGHTS attributes.
     *
     * This is a no-op if the given skin index or target is invalid.
     */
    public void attachSkin(@IntRange(from = 0) int skinIndex, @Entity int target) {
        nAttachSkin(getNativeObject(), skinIndex, target);
    }

    /**
     * Attaches the given skin to the given node, which must have an associated mesh with
     * BONE_INDICES and BONE_WEIGHTS attributes.
     *
     * This is a no-op if the given skin index or target is invalid.
     */
    public void detachSkin(@IntRange(from = 0) int skinIndex, @Entity int target) {
        nDetachSkin(getNativeObject(), skinIndex, target);
    }

    /**
     * Gets the joint count at skin index in this instance.
     */
    public int getJointCountAt(@IntRange(from = 0) int skinIndex) {
        return nGetJointCountAt(getNativeObject(), skinIndex);
    }

    /**
     * Gets joints at skin index in this instance.
     */
    public @NonNull @Entity int[] getJointsAt(@IntRange(from = 0) int skinIndex) {
        int[] result = new int[getJointCountAt(skinIndex)];
        nGetJointsAt(getNativeObject(), skinIndex, result);
        return result;
    }

    /**
     * Applies the given material variant to all primitives in this instance.
     *
     * Ignored if variantIndex is out of bounds.
     */
    public void applyMaterialVariant(@IntRange(from = 0) int variantIndex) {
        nApplyMaterialVariant(mNativeObject, variantIndex);
    }

    public @NonNull MaterialInstance[] getMaterialInstances() {
        final int count = nGetMaterialInstanceCount(mNativeObject);
        MaterialInstance[] result = new MaterialInstance[count];
        long[] natives = new long[count];
        nGetMaterialInstances(mNativeObject, natives);
        Engine engine = mAsset.getEngine();
        for (int i = 0; i < count; i++) {
            result[i] = new MaterialInstance(engine, natives[i]);
        }
        return result;
    }

    /**
     * Returns the names of all material variants.
     */
    public @NonNull String[] getMaterialVariantNames() {
        String[] names = new String[nGetMaterialVariantCount(mNativeObject)];
        nGetMaterialVariantNames(mNativeObject, names);
        return names;
    }

    private static native int nGetRoot(long nativeInstance);
    private static native int nGetEntityCount(long nativeInstance);
    private static native void nGetEntities(long nativeInstance, int[] result);
    private static native long nGetAnimator(long nativeInstance);

    private static native int nGetMaterialInstanceCount(long nativeAsset);
    private static native void nGetMaterialInstances(long nativeAsset, long[] nativeResults);

    private static native void nApplyMaterialVariant(long nativeInstance, int variantIndex);
    private static native int nGetMaterialVariantCount(long nativeAsset);
    private static native void nGetMaterialVariantNames(long nativeAsset, String[] result);

    private static native void nGetJointsAt(long nativeInstance, int skinIndex, int[] result);
    private static native int nGetSkinCount(long nativeInstance);
    private static native void nGetSkinNames(long nativeInstance, String[] result);
    private static native int nGetJointCountAt(long nativeInstance, int skinIndex);
    private static native void nAttachSkin(long nativeInstance, int skinIndex, int entity);
    private static native void nDetachSkin(long nativeInstance, int skinIndex, int entity);
}
