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

import android.support.annotation.Nullable;

public class Scene {
    private long mNativeObject;
    private @Nullable Skybox mSkybox;
    private @Nullable IndirectLight mIndirectLight;

    Scene(long nativeScene) {
        mNativeObject = nativeScene;
    }

    @Nullable
    public Skybox getSkybox() {
        return mSkybox;
    }

    public void setSkybox(@Nullable Skybox skybox) {
        mSkybox = skybox;
        nSetSkybox(getNativeObject(), mSkybox != null ? mSkybox.getNativeObject() : 0);
    }

    @Nullable
    public IndirectLight getIndirectLight() {
        return mIndirectLight;
    }

    public void setIndirectLight(@Nullable IndirectLight ibl) {
        mIndirectLight = ibl;
        nSetIndirectLight(getNativeObject(),
                mIndirectLight != null ? mIndirectLight.getNativeObject() : 0);
    }

    public void addEntity(@Entity int entity) {
        nAddEntity(getNativeObject(), entity);
    }

    public void removeEntity(@Entity int entity) {
        nRemove(getNativeObject(), entity);
    }

    /**
     * @deprecated See {@link #removeEntity(int)}
     */
    public void remove(@Entity int entity) {
        removeEntity(entity);
    }

    public int getRenderableCount() {
        return nGetRenderableCount(getNativeObject());
    }

    public int getLightCount() {
        return nGetLightCount(getNativeObject());
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Scene");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetSkybox(long nativeScene, long nativeSkybox);
    private static native void nSetIndirectLight(long nativeScene, long nativeIndirectLight);
    private static native void nAddEntity(long nativeScene, int entity);
    private static native void nRemove(long nativeScene, int entity);
    private static native int nGetRenderableCount(long nativeScene);
    private static native int nGetLightCount(long nativeScene);
}
