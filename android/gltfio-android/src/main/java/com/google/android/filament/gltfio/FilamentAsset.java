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

import com.google.android.filament.Box;
import com.google.android.filament.Engine;
import com.google.android.filament.Entity;

/**
 * Owns a bundle of Filament objects that have been created by <code>AssetLoader</code>.
 *
 * <p>For usage instructions, see the documentation for {@link AssetLoader}.</p>
 *
 * <p>This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity
 * has a <code>TransformManager</code> component, and some entities also have compnents managed by
 * <code>NameComponentManager</code>, <code>RenderableManager</code>, and others.</p>
 *
 * <p>In addition to the aforementioned entities, an asset has strong ownership over a list of
 * <code>VertexBuffer</code>, <code>IndexBuffer</code>, and <code>Texture</code>.</p>
 *
 * <p>Clients can use {@link ResourceLoader} to create textures, compute tangent quaternions, and
 * upload data into vertex buffers and index buffers.</p>
 *
 * @see ResourceLoader
 * @see FilamentInstance
 * @see AssetLoader
 */
public class FilamentAsset {
    private long mNativeObject;
    private FilamentInstance mPrimaryInstance;
    private Engine mEngine;

    FilamentAsset(Engine engine, long nativeObject) {
        mEngine = engine;
        mNativeObject = nativeObject;
    }

    public FilamentInstance getInstance() {
        if (mPrimaryInstance != null) {
            return mPrimaryInstance;
        }
        long nativeInstance = nGetInstance(getNativeObject());
        mPrimaryInstance = new FilamentInstance(this, nativeInstance);
        return mPrimaryInstance;
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
     * Pops a ready renderable off the queue, or returns 0 if no renderables have become ready.
     *
     * NOTE: To determine the progress percentage or completion status, please use
     * ResourceLoader#asyncGetLoadProgress.
     *
     * This helper method allows clients to progressively add renderables to the scene as textures
     * gradually become ready through asynchronous loading.
     *
     * See also ResourceLoader#asyncBeginLoad.
     */
    public @Entity int popRenderable() {
        return nPopRenderable(mNativeObject);
    }

    /**
     * Pops one or more renderables off the queue, or returns the available number.
     *
     * Returns the number of entities written into the given array. If the given array
     * is null, returns the number of available renderables.
     */
    public int popRenderables(@Nullable @Entity int[] entities) {
        return nPopRenderables(mNativeObject, entities);
    }

    /**
     * Gets the list of entities, one for each glTF node.
     *
     * <p>All of these have a transform component. Some of the returned entities may also have a
     * renderable or light component.</p>
     */
    public @NonNull @Entity int[] getEntities() {
        int[] result = new int[nGetEntityCount(mNativeObject)];
        nGetEntities(mNativeObject, result);
        return result;
    }

    /**
     * Gets only the entities that have light components.
     */
    public @NonNull @Entity int[] getLightEntities() {
        int[] result = new int[nGetLightEntityCount(mNativeObject)];
        nGetLightEntities(mNativeObject, result);
        return result;
    }

    /**
     * Gets only the entities that have renderable components.
     */
    public @NonNull @Entity int[] getRenderableEntities() {
        int[] result = new int[nGetRenderableEntityCount(mNativeObject)];
        nGetRenderableEntities(mNativeObject, result);
        return result;
    }

    /**
     * Gets only the entities that have camera components.
     *
     * <p>
     * Note about aspect ratios:<br>
     *
     * gltfio always uses an aspect ratio of 1.0 when setting the projection matrix for perspective
     * cameras. gltfio then sets the camera's scaling matrix with the aspect ratio specified in the
     * glTF file (if present).<br>
     *
     * The camera's scaling matrix allows clients to adjust the aspect ratio independently from the
     * camera's projection.
     * </p>
     *
     * @see com.google.android.filament.Camera#setScaling
     */
    public @NonNull @Entity int[] getCameraEntities() {
        int[] result = new int[nGetCameraEntityCount(mNativeObject)];
        nGetCameraEntities(mNativeObject, result);
        return result;
    }

    /**
     * Returns the first entity with the given name, or 0 if none exist.
     */
    public @Entity int getFirstEntityByName(String name) {
        return nGetFirstEntityByName(mNativeObject, name);
    }

    /**
     * Gets a list of entities with the given name.
     */
    public @NonNull @Entity int[] getEntitiesByName(String name) {
        int[] result = new int[nGetEntitiesByName(mNativeObject, name, null)];
        nGetEntitiesByName(mNativeObject, name, result);
        return result;
    }

    /**
     * Gets a list of entities whose names start with the given prefix.
     */
    public @NonNull @Entity int[] getEntitiesByPrefix(String prefix) {
        int[] result = new int[nGetEntitiesByPrefix(mNativeObject, prefix, null)];
        nGetEntitiesByPrefix(mNativeObject, prefix, result);
        return result;
    }

    /**
     * Gets the bounding box computed from the supplied min / max values in glTF accessors.
     *
     * This does not return a bounding box over all FilamentInstance, it's just a straightforward
     * AAAB that can be determined at load time from the asset data.
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
     * Gets the glTF extras string for the asset or a specific node.
     *
     * @param entity the entity corresponding to the glTF node, or 0 to get the asset-level string.
     * @return the requested extras string, or null if it does not exist.
     */
    public @Nullable String getExtras(@Entity int entity) {
        return nGetExtras(mNativeObject, entity);
    }

    /**
     * Gets the names of all morph targets in the given entity.
     */
    public @NonNull String[] getMorphTargetNames(@Entity int entity) {
        String[] names = new String[nGetMorphTargetCount(mNativeObject, entity)];
        nGetMorphTargetNames(mNativeObject, entity, names);
        return names;
    }

    /**
     * Gets resource URIs for all externally-referenced buffers.
     */
    public @NonNull String[] getResourceUris() {
        String[] uris = new String[nGetResourceUriCount(mNativeObject)];
        nGetResourceUris(mNativeObject, uris);
        return uris;
    }

    /**
     * Reclaims CPU-side memory for URI strings, binding lists, and raw animation data.
     *
     * This should only be called after ResourceLoader#loadResources() or
     * ResourceLoader#asyncBeginLoad(). If this is an instanced asset, this prevents creation of new
     * instances.
     */
    public void releaseSourceData() {
        nReleaseSourceData(mNativeObject);
    }

    public Engine getEngine() { return mEngine; }

    void clearNativeObject() {
        mPrimaryInstance = null;
        mNativeObject = 0;
    }

    private static native int nGetRoot(long nativeAsset);
    private static native int nPopRenderable(long nativeAsset);
    private static native int nPopRenderables(long nativeAsset, int[] result);

    private static native int nGetEntityCount(long nativeAsset);
    private static native void nGetEntities(long nativeAsset, int[] result);

    private static native int nGetFirstEntityByName(long nativeAsset, String name);
    private static native int nGetEntitiesByName(long nativeAsset, String name, int[] result);
    private static native int nGetEntitiesByPrefix(long nativeAsset, String prefix, int[] result);

    private static native int nGetLightEntityCount(long nativeAsset);
    private static native void nGetLightEntities(long nativeAsset, int[] result);

    private static native int nGetRenderableEntityCount(long nativeAsset);
    private static native void nGetRenderableEntities(long nativeAsset, int[] result);

    private static native int nGetCameraEntityCount(long nativeAsset);
    private static native void nGetCameraEntities(long nativeAsset, int[] result);

    private static native int nGetMorphTargetCount(long nativeAsset, int entity);
    private static native void nGetMorphTargetNames(long nativeAsset, int entity, String[] result);

    private static native void nGetBoundingBox(long nativeAsset, float[] box);
    private static native String nGetName(long nativeAsset, int entity);
    private static native String nGetExtras(long nativeAsset, int entity);

    private static native long nGetInstance(long nativeAsset);

    private static native int nGetResourceUriCount(long nativeAsset);
    private static native void nGetResourceUris(long nativeAsset, String[] result);

    private static native void nReleaseSourceData(long nativeAsset);
}
