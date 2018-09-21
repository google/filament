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
import android.support.annotation.Nullable;
import android.support.annotation.Size;

public class TransformManager {
    private long mNativeObject;

    TransformManager(long nativeTransformManager) {
        mNativeObject = nativeTransformManager;
    }

    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    @EntityInstance
    public int create(@Entity int entity) {
        return nCreate(mNativeObject, entity);
    }

    @EntityInstance
    public int create(@Entity int entity, @EntityInstance int parent,
            @Nullable @Size(min = 16) float[] localTransform) {
        return nCreateArray(mNativeObject, entity, parent, localTransform);
    }

    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    public void setParent(@EntityInstance int i, @EntityInstance int newParent) {
        nSetParent(mNativeObject, i, newParent);
    }

    public void setTransform(@EntityInstance int i,
            @NonNull @Size(min = 16) float[] localTransform) {
        if (localTransform.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
        nSetTransform(mNativeObject, i, localTransform);
    }

    @NonNull
    @Size(min = 16)
    public float[] getTransform(@EntityInstance int i,
            @Nullable @Size(min = 16) float[] outLocalTransform) {
        outLocalTransform = assertMat4f(outLocalTransform);
        nGetTransform(mNativeObject, i, outLocalTransform);
        return outLocalTransform;
    }

    @NonNull
    @Size(min = 16)
    public float[] getWorldTransform(@EntityInstance int i,
            @Nullable @Size(min = 16) float[] outWorldTransform) {
        outWorldTransform = assertMat4f(outWorldTransform);
        nGetWorldTransform(mNativeObject, i, outWorldTransform);
        return outWorldTransform;
    }

    public void openLocalTransformTransaction() {
        nOpenLocalTransformTransaction(mNativeObject);
    }

    public void commitLocalTransformTransaction() {
        nCommitLocalTransformTransaction(mNativeObject);
    }

    @NonNull @Size(min = 16)
    private static float[] assertMat4f(@Nullable float[] out) {
        if (out == null) out = new float[16];
        else if (out.length < 16) {
            throw new ArrayIndexOutOfBoundsException("Array length must be at least 16");
        }
        return out;
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
