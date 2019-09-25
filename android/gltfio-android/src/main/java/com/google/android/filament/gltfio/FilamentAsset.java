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
 * Owns a bundle of Filament objects that have been created by AssetLoader.
 *
 * For usage instructions, see the documentation for AssetLoader.
 *
 * This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity has
 * a TransformManager component, and some entities also have a NameComponentManager and/or
 * RenderableManager components.
 *
 * In addition to the aforementioned entities, an asset has strong ownership over a list of
 * VertexBuffer, IndexBuffer, MaterialInstance, Texture, and, optionally, a simple animation engine
 * (Animator).
 *
 * Clients can ResourceLoader to create Texture objects, compute tangent quaternions, and upload
 * data into vertex buffers and index buffers.
 */
public class FilamentAsset {
    private long mNativeObject;

    FilamentAsset(long nativeObject) {
        mNativeObject = nativeObject;
    }

    long getNativeObject() {
        return mNativeObject;
    }

    /** Gets the transform root for the asset, which has no matching glTF node. */
    public @Entity int getRoot() {
        return nGetRoot(mNativeObject);
    }

    /**
     * Gets the list of entities, one for each glTF node. All of these have a Transform component.
     * Some of the returned entities may also have a Renderable component.
     */
    public @Entity int[] getEntities() {
        int[] result = new int[nGetEntityCount(mNativeObject)];
        nGetEntities(mNativeObject, result);
        return result;
    }

    /** Gets the bounding box computed from the supplied min / max values in glTF accessors. */
    public @NonNull Box getBoundingBox() {
        float[] box = new float[6];
        nGetBoundingBox(mNativeObject, box);
        return new Box(box[0], box[1], box[2], box[3], box[4], box[5]);
    }

    /** Gets the NameComponentManager label for the given entity, if it exists. */
    public String getName(@Entity int entity) {
        return nGetName(getNativeObject(), entity);
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native int nGetRoot(long nativeAsset);
    private static native int nGetEntityCount(long nativeAsset);
    private static native void nGetEntities(long nativeAsset, int[] result);
    private static native void nGetBoundingBox(long nativeAsset, float[] box);
    private static native String nGetName(long nativeAsset, int entity);
}
