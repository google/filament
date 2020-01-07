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
import androidx.annotation.Size;

/**
 * <code>TransformManager</code> is used to add transform components to entities.
 *
 * <p>A transform component gives an entity a position and orientation in space in the coordinate
 * space of its parent transform. The <code>TransformManager</code> takes care of computing the
 * world-space transform of each component (i.e. its transform relative to the root).</p>
 *
 * <h1>Creation and destruction</h1>
 *
 * A transform component is created using {@link TransformManager#create} and destroyed by calling
 * {@link TransformManager#destroy}.
 *
 * <pre>
 *  Engine engine = Engine.create();
 *  EntityManager entityManager = EntityManager().get();
 *  int object = entityManager.create();
 *
 *  TransformManager tcm = engine.getTransformManager();
 *
 *  // create the transform component
 *  tcm.create(object);
 *
 *  // set its transform
 *  float[] transform = ...; // transform to set
 *  EntityInstance i = tcm.getInstance(object);
 *  tcm.setTransform(i, transform));
 *
 *  // destroy the transform component
 *  tcm.destroy(object);
 * </pre>
 *
 */public class TransformManager {
    private long mNativeObject;

    TransformManager(long nativeTransformManager) {
        mNativeObject = nativeTransformManager;
    }

    /**
     * Returns whether a particular {@link Entity} is associated with a component of this
     * <code>TransformManager</code>
     *
     * @param entity an {@link Entity}
     * @return true if this {@link Entity} has a component associated with this manager
     */
    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    /**
     * Gets an {@link EntityInstance} representing the transform component associated with the
     * given {@link Entity}.
     *
     * @param entity an {@link Entity}
     * @return an {@link EntityInstance}, which represents the transform component associated with
     * the {@link Entity} <code>entity</code>
     * @see #hasComponent
     */
    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    /**
     * Creates a transform component and associates it with the given entity. The component is
     * initialized with the identity transform.
     * If this component already exists on the given entity, it is first
     * destroyed as if {@link #destroy} was called.
     *
     * @param entity an {@link Entity} to associate a transform component to.
     * @see #destroy
     */
    @EntityInstance
    public int create(@Entity int entity) {
        return nCreate(mNativeObject, entity);
    }

    /**
     * Creates a transform component with a parent and associates it with the given entity.
     * If this component already exists on the given entity, it is first
     * destroyed as if {@link #destroy} was called.
     *
     * @param entity         an {@link Entity} to associate a transform component to.
     * @param parent         the  {@link EntityInstance} of the parent transform
     * @param localTransform the transform, relative to the parent, to initialize the transform
     *                       component with.
     * @see #destroy
     */
    @EntityInstance
    public int create(@Entity int entity, @EntityInstance int parent,
            @Nullable @Size(min = 16) float[] localTransform) {
        return nCreateArray(mNativeObject, entity, parent, localTransform);
    }

    /**
     * Destroys this component from the given entity, children are orphaned.
     *
     * @param entity an {@link Entity}.
     *               If this transform had children, these are orphaned, which means their local
     *               transform becomes a world transform. Usually it's nonsensical.
     *               It's recommended to make sure that a destroyed transform doesn't have children.
     * @see #create
     */
    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    /**
     * Re-parents an entity to a new one.
     *
     * @param i         the {@link EntityInstance} of the transform component to re-parent
     * @param newParent the {@link EntityInstance} of the new parent transform.
     *                  It is an error to re-parent an entity to a descendant and will cause
     *                  undefined behaviour.
     * @see #getInstance
     */
    public void setParent(@EntityInstance int i, @EntityInstance int newParent) {
        nSetParent(mNativeObject, i, newParent);
    }

    /**
     * Sets a local transform of a transform component.
     * <p>This operation can be slow if the hierarchy of transform is too deep, and this
     * will be particularly bad when updating a lot of transforms. In that case,
     * consider using {@link #openLocalTransformTransaction} / {@link #commitLocalTransformTransaction}.</p>
     *
     * @param i              the {@link EntityInstance} of the transform component to set the local
     *                       transform to.
     * @param localTransform the local transform (i.e. relative to the parent).
     * @see #getTransform
     */
    public void setTransform(@EntityInstance int i,
            @NonNull @Size(min = 16) float[] localTransform) {
        Asserts.assertMat4fIn(localTransform);
        nSetTransform(mNativeObject, i, localTransform);
    }

    /**
     * Returns the local transform of a transform component.
     *
     * @param i                 the {@link EntityInstance} of the transform component to query the
     *                          local transform from.
     * @param outLocalTransform a 16 <code>float</code> array to receive the result.
     *                          If <code>null</code> is given,  a new suitable array is allocated.
     * @return the local transform of the component (i.e. relative to the parent). This always
     * returns the value set by setTransform().
     * @see #setTransform
     */
    @NonNull
    @Size(min = 16)
    public float[] getTransform(@EntityInstance int i,
            @Nullable @Size(min = 16) float[] outLocalTransform) {
        outLocalTransform = Asserts.assertMat4f(outLocalTransform);
        nGetTransform(mNativeObject, i, outLocalTransform);
        return outLocalTransform;
    }

    /**
     * Returns the world transform of a transform component.
     *
     * @param i                 the {@link EntityInstance} of the transform component to query the
     *                          world transform from.
     * @param outWorldTransform a 16 <code>float</code> array to receive the result.
     *                          If <code>null</code> is given,  a new suitable array is allocated
     * @return The world transform of the component (i.e. relative to the root). This is the
     * composition of this component's local transform with its parent's world transform.
     * @see #setTransform
     */
    @NonNull
    @Size(min = 16)
    public float[] getWorldTransform(@EntityInstance int i,
            @Nullable @Size(min = 16) float[] outWorldTransform) {
        outWorldTransform = Asserts.assertMat4f(outWorldTransform);
        nGetWorldTransform(mNativeObject, i, outWorldTransform);
        return outWorldTransform;
    }

    /**
     * Opens a local transform transaction. During a transaction, {@link #getWorldTransform} can
     * return an invalid transform until {@link #commitLocalTransformTransaction} is called.
     * However, {@link #setTransform} will perform significantly better and in constant time.
     *
     * <p>This is useful when updating many transforms and the transform hierarchy is deep (say more
     * than 4 or 5 levels).</p>
     *
     * <p>If the local transform transaction is already open, this is a no-op.</p>
     *
     * @see #commitLocalTransformTransaction
     * @see #setTransform
     */
    public void openLocalTransformTransaction() {
        nOpenLocalTransformTransaction(mNativeObject);
    }

    /**
     * Commits the currently open local transform transaction. When this returns, calls
     * to {@link #getWorldTransform} will return the proper value.
     *
     * <p>Failing to call this method when done updating the local transform will cause
     * a lot of rendering problems. The system never closes the transaction
     * automatically.</p>
     *
     * <p>If the local transform transaction is not open, this is a no-op.</p>
     *
     * @see #openLocalTransformTransaction
     * @see #setTransform
     */
    public void commitLocalTransformTransaction() {
        nCommitLocalTransformTransaction(mNativeObject);
    }

    public long getNativeObject() {
        return mNativeObject;
    }

    private static native boolean nHasComponent(long nativeTransformManager, int entity);
    private static native int nGetInstance(long nativeTransformManager, int entity);
    private static native int nCreate(long nativeTransformManager, int entity);
    private static native int nCreateArray(long mNativeObject, int entity, int parent, float[] localTransform);
    private static native void nDestroy(long nativeTransformManager, int entity);
    private static native void nSetParent(long nativeTransformManager, int i, int newParent);
    private static native void nSetTransform(long nativeTransformManager, int i, float[] localTransform);
    private static native void nGetTransform(long nativeTransformManager, int i, float[] outLocalTransform);
    private static native void nGetWorldTransform(long nativeTransformManager, int i, float[] outWorldTransform);
    private static native void nOpenLocalTransformTransaction(long nativeTransformManager);
    private static native void nCommitLocalTransformTransaction(long nativeTransformManager);
}
