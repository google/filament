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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import com.google.android.filament.proguard.UsedByReflection;

public class EntityManager {
    private long mNativeObject = nGetEntityManager();

    private static class Holder {
        static final EntityManager INSTANCE = new EntityManager();
    }

    private EntityManager() {
    }

    EntityManager(long nativeEntityManager) {
        mNativeObject = nativeEntityManager;
    }

    @NonNull
    public static EntityManager get() {
        return Holder.INSTANCE;
    }

    @Entity
    public int create() {
        return nCreate(mNativeObject);
    }

    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    @Entity
    @NonNull
    public int[] create(@IntRange(from = 1) int n) {
        if (n < 1) throw new ArrayIndexOutOfBoundsException("n must be at least 1");
        int[] entities = new int[n];
        nCreateArray(mNativeObject, n, entities);
        return entities;
    }

    @NonNull
    public int[] create(@Entity @NonNull int[] entities) {
        nCreateArray(mNativeObject, entities.length, entities);
        return entities;
    }

    public void destroy(@Entity @NonNull int[] entities) {
        nDestroyArray(mNativeObject, entities.length, entities);
    }

    public boolean isAlive(@Entity int entity) {
        return nIsAlive(mNativeObject, entity);
    }

    @UsedByReflection("AssetLoader.java")
    public long getNativeObject() {
        return mNativeObject;
    }

    private static native long nGetEntityManager();
    private static native void nCreateArray(long nativeEntityManager, int n, int[] entities);
    private static native int nCreate(long nativeEntityManager);
    private static native void nDestroyArray(long nativeEntityManager, int n, int[] entities);
    private static native void nDestroy(long nativeEntityManager, int entity);
    private static native boolean nIsAlive(long nativeEntityManager, int entity);
}
