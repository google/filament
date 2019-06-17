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
import android.support.annotation.Nullable;

import com.google.android.filament.Engine;
import com.google.android.filament.EntityManager;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.Skybox;
import com.google.android.filament.Texture;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.nio.Buffer;

public class AssetLoader {
    private long mNativeObject;

    static Method sEngineGetNativeObject;
    static Method sEntityManagerGetNativeObject;
    static Constructor<Texture> sTextureConstructor;
    static Constructor<IndirectLight> sIndirectLightConstructor;
    static Constructor<Skybox> sSkyboxConstructor;

    public static void init() {
        System.loadLibrary("gltfio-jni");
        try {
            sEngineGetNativeObject = Engine.class.getDeclaredMethod("getNativeObject");
            sEngineGetNativeObject.setAccessible(true);

            sEntityManagerGetNativeObject = EntityManager.class.getDeclaredMethod("getNativeObject");
            sEntityManagerGetNativeObject.setAccessible(true);

            sTextureConstructor = Texture.class.getDeclaredConstructor(long.class);
            sTextureConstructor.setAccessible(true);

            sIndirectLightConstructor = IndirectLight.class.getDeclaredConstructor(long.class);
            sIndirectLightConstructor.setAccessible(true);

            sSkyboxConstructor = Skybox.class.getDeclaredConstructor(long.class);
            sSkyboxConstructor.setAccessible(true);
        } catch (NoSuchMethodException e) {
            // Cannot happen
        }
    }

    public AssetLoader(@NonNull Engine engine, @NonNull MaterialProvider generator,
            @NonNull EntityManager entities) {
        try {
            long nativeEngine = (long) sEngineGetNativeObject.invoke(engine);
            long nativeMaterials = generator.getNativeObject();
            long nativeEntities = (long) sEntityManagerGetNativeObject.invoke(entities);
            mNativeObject = nCreateAssetLoader(nativeEngine, nativeMaterials, nativeEntities);
        } catch (Exception e) {
            // Ignored
        }
        if (mNativeObject == 0) {
            throw new IllegalStateException("Unable to parse glTF asset.");
        }
    }

    public void destroy() {
        nDestroyAssetLoader(mNativeObject);
        mNativeObject = 0;
    }

    @Nullable
    public FilamentAsset createAssetFromBinary(@NonNull Buffer buffer) {
        long nativeAsset = nCreateAssetFromBinary(mNativeObject, buffer, buffer.remaining());
        return new FilamentAsset(nativeAsset);
    }

    public void enableDiagnostics(boolean enable) {
        nEnableDiagnostics(mNativeObject, enable);
    }

    public void destroyAsset(@Nullable FilamentAsset asset) {
        nDestroyAsset(mNativeObject, asset.getNativeObject());
        asset.clearNativeObject();
    }

    private static native long nCreateAssetLoader(long nativeEngine, long nativeGenerator, long nativeEntities);
    private static native void nDestroyAssetLoader(long nativeLoader);
    private static native long nCreateAssetFromBinary(long nativeLoader, Buffer buffer, int remaining);
    private static native void nEnableDiagnostics(long nativeLoader, boolean enable);
    private static native void nDestroyAsset(long nativeLoader, long nativeAsset);
}
