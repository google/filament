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

import android.support.annotation.NonNull;

import com.google.android.filament.Box;
import com.google.android.filament.Entity;

/**
 * Owns a bundle of Filament objects that have been created by <code>AssetLoader</code>.
 *
 * <p>For usage instructions, see the documentation for {@link AssetLoader}.</p>
 *
 * <p>This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity has
 * a <code>TransformManager</code> component, and some entities also have
 * <code>NameComponentManager</code> and/or <code>RenderableManager</code> components.</p>
 *
 * <p>In addition to the aforementioned entities, an asset has strong ownership over a list of
 * <code>VertexBuffer</code>, <code>IndexBuffer</code>, <code>MaterialInstance</code>, and
 * <code>Texture</code>.</p>
 *
 * <p>Clients can use {@link ResourceLoader} to create textures, compute tangent quaternions, and
 * upload data into vertex buffers and index buffers.</p>
 *
 * @see ResourceLoader
 * @see Animator
 * @see AssetLoader
 */
public class FilamentAsset {
    private long mNativeObject;
    private Animator mAnimator;

    FilamentAsset(long nativeObject) {
        mNativeObject = nativeObject;
        mAnimator = null;
    }

    long getNativeObject() {
        return mNativeObject;
    }

    /**
     * Gets the transform root for the asset, which has no matching glTF node.
     */
    public @Entity int getRoot() {
        return nGetRoot(mNativeObject);
    }

    /**
     * Gets the list of entities, one for each glTF node.
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
     * Gets the bounding box computed from the supplied min / max values in glTF accessors.
     */
    public @NonNull Box getBoundingBox() {
        float[] box = new float[6];
        nGetBoundingBox(mNativeObject, box);
        return new Box(box[0], box[1], box[2], box[3], box[4], box[5]);
    }

    /**
     * Gets the <code>NameComponentManager</code> label for the given entity, if it exists.
     */
    public String getName(@Entity int entity) {
        return nGetName(getNativeObject(), entity);
    }

    /**
     * Creates or retrieves the <code>Animator</code> for this asset.
     *
     * <p>When calling this for the first time, this must be called after
     * {@see ResourceLoader#loadResources}.</p>
     */
    public @NonNull Animator getAnimator() {
        if (mAnimator != null) {
            return mAnimator;
        }
        mAnimator = new Animator(nGetAnimator(getNativeObject()));
        return mAnimator;
    }

    /**
     * Gets resource URIs for all externally-referenced buffers.
     */
    public @NonNull String[] getResourceUris() {
        String[] uris = new String[nGetResourceUriCount(mNativeObject)];
        nGetResourceUris(mNativeObject, uris);
        return uris;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native int nGetRoot(long nativeAsset);
    private static native int nGetEntityCount(long nativeAsset);
    private static native void nGetEntities(long nativeAsset, int[] result);
    private static native void nGetBoundingBox(long nativeAsset, float[] box);
    private static native String nGetName(long nativeAsset, int entity);
    private static native long nGetAnimator(long nativeAsset);
    private static native int nGetResourceUriCount(long nativeAsset);
    private static native void nGetResourceUris(long nativeAsset, String[] result);
}
