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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * A <code>Scene</code> is a flat container of {@link RenderableManager} and {@link LightManager}
 * components.
 * <br>
 * <p>A <code>Scene</code> doesn't provide a hierarchy of objects, i.e.: it's not a scene-graph.
 * However, it manages the list of objects to render and the list of lights. These can
 * be added or removed from a <code>Scene</code> at any time.
 * Moreover clients can use {@link TransformManager} to create a graph of transforms.</p>
 * <br>
 * <p>A {@link RenderableManager} component <b>must</b> be added to a <code>Scene</code> in order
 * to be rendered, and the <code>Scene</code> must be provided to a {@link View}.</p>
 *
 * <h1>Creation and Destruction</h1>
 *
 * A <code>Scene</code> is created using {@link Engine#createScene} and destroyed using
 * {@link Engine#destroyScene(Scene)}.
 *
 * @see View
 * @see LightManager
 * @see RenderableManager
 * @see TransformManager
 */
public class Scene {
    private long mNativeObject;
    private @Nullable Skybox mSkybox;
    private @Nullable IndirectLight mIndirectLight;

    Scene(long nativeScene) {
        mNativeObject = nativeScene;
    }

    /**
     * @return the {@link Skybox} or <code>null</code> if none is set
     * @see #setSkybox(Skybox)
     */
    @Nullable
    public Skybox getSkybox() {
        return mSkybox;
    }

    /**
     * Sets the {@link Skybox}.
     *
     * The {@link Skybox} is drawn last and covers all pixels not touched by geometry.
     *
     * @param skybox the {@link Skybox} to use to fill untouched pixels,
     *               or <code>null</code> to unset the {@link Skybox}.
     */
    public void setSkybox(@Nullable Skybox skybox) {
        mSkybox = skybox;
        nSetSkybox(getNativeObject(), mSkybox != null ? mSkybox.getNativeObject() : 0);
    }

    /**
     * @return the {@link IndirectLight} or <code>null</code> if none is set
     * @see #setIndirectLight(IndirectLight)
     */
    @Nullable
    public IndirectLight getIndirectLight() {
        return mIndirectLight;
    }

    /**
     * Sets the {@link IndirectLight} to use when rendering the <code>Scene</code>.
     *
     * Currently, a <code>Scene</code> may only have a single {@link IndirectLight}.
     * This call replaces the current {@link IndirectLight}.
     *
     * @param ibl the {@link IndirectLight} to use when rendering the <code>Scene</code>
     *            or <code>null</code> to unset.
     */
    public void setIndirectLight(@Nullable IndirectLight ibl) {
        mIndirectLight = ibl;
        nSetIndirectLight(getNativeObject(),
                mIndirectLight != null ? mIndirectLight.getNativeObject() : 0);
    }

    /**
     * Adds an {@link Entity} to the <code>Scene</code>.
     *
     * @param entity the entity is ignored if it doesn't have a {@link RenderableManager} component
     *               or {@link LightManager} component.<br>
     *               A given {@link Entity} object can only be added once to a <code>Scene</code>.
     */
    public void addEntity(@Entity int entity) {
        nAddEntity(getNativeObject(), entity);
    }

    /**
     * Adds a list of entities to the <code>Scene</code>.
     *
     * @param entities array containing entities to add to the <code>Scene</code>.
     */
    public void addEntities(@Entity int[] entities) {
        nAddEntities(getNativeObject(), entities);
    }

    /**
     * Removes an {@link Entity} from the <code>Scene</code>.
     *
     * @param entity the {@link Entity} to remove from the <code>Scene</code>. If the specified
     *                   <code>entity</code> doesn't exist, this call is ignored.
     */
    public void removeEntity(@Entity int entity) {
        nRemove(getNativeObject(), entity);
    }

    /**
     * @deprecated See {@link #removeEntity(int)}
     */
    @Deprecated
    public void remove(@Entity int entity) {
        removeEntity(entity);
    }

    /**
     * Removes a list of entities from the <code>Scene</code>.
     *
     * This is equivalent to calling remove in a loop.
     * If any of the specified entities do not exist in the scene, they are skipped.
     *
     * @param entities array containing entities to remove from the <code>Scene</code>.
     */
    public void removeEntities(@Entity int[] entities) {
        nRemoveEntities(getNativeObject(), entities);
    }

    /**
     * Returns the total number of Entities in the <code>Scene</code>, whether alive or not.
     *
     * @return the total number of Entities in the <code>Scene</code>.
     */
    public int getEntityCount() {
        return nGetEntityCount(getNativeObject());
    }

    /**
     * Returns the number of active (alive) {@link RenderableManager} components in the
     * <code>Scene</code>.
     *
     * @return number of {@link RenderableManager} components in the <code>Scene</code>.
     */
    public int getRenderableCount() {
        return nGetRenderableCount(getNativeObject());
    }

    /**
     * Returns the number of active (alive) {@link LightManager} components in the
     * <code>Scene</code>.
     *
     * @return number of {@link LightManager} components in the <code>Scene</code>.
     */
    public int getLightCount() {
        return nGetLightCount(getNativeObject());
    }

    /**
     * Returns true if the given entity is in the Scene.
     *
     * @return Whether the given entity is in the Scene.
     */
    public boolean hasEntity(@Entity int entity) {
        return nHasEntity(getNativeObject(), entity);
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Scene");
        }
        return mNativeObject;
    }

    /**
     * Returns the list of all entities in the Scene. If outArray is provided and large enough,
     * it is used to store the list and returned, otherwise a new array is allocated and returned.
     * @param outArray an array to store the list of entities in the scene.
     * @return outArray if it was used or a newly allocated array.
     * @see #getEntityCount
     */
    public int[] getEntities(@Nullable int[] outArray) {
        int c = getEntityCount();
        if (outArray == null || outArray.length < c) {
            outArray = new int[c];
        }
        boolean success = nGetEntities(getNativeObject(), outArray, outArray.length);
        if (!success) {
            throw new IllegalStateException("Error retriving Scene's entities");
        }
        return outArray;
    }

    /**
     * Returns the list of all entities in the Scene in a newly allocated array.
     * @return an array containing the list of all entities in the scene.
     * @see #getEntityCount
     */
    public int[] getEntities() {
        return getEntities(null);
    }

    public interface EntityProcessor {
        void process(@Entity int entity);
    }

    /**
     * Invokes user functor on each entity in the scene.
     *
     * It is not allowed to add or remove an entity from the scene within the functor.
     *
     * @param entityProcessor User provided functor called for each entity in the scene
     */
    public void forEach(@NonNull EntityProcessor entityProcessor) {
        int[] entities = getEntities(null);
        for (int entity : entities) {
            entityProcessor.process(entity);
        }
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetSkybox(long nativeScene, long nativeSkybox);
    private static native void nSetIndirectLight(long nativeScene, long nativeIndirectLight);
    private static native void nAddEntity(long nativeScene, int entity);
    private static native void nAddEntities(long nativeScene, int[] entities);
    private static native void nRemove(long nativeScene, int entity);
    private static native void nRemoveEntities(long nativeScene, int[] entities);
    private static native int nGetEntityCount(long nativeScene);
    private static native int nGetRenderableCount(long nativeScene);
    private static native int nGetLightCount(long nativeScene);
    private static native boolean nHasEntity(long nativeScene, int entity);
    private static native boolean nGetEntities(long nativeScene, int[] outArray, int length);
}
