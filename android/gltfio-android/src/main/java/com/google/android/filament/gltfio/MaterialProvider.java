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

import com.google.android.filament.Engine;

import java.lang.reflect.Method;

/**
 * Loads pre-generated ubershader materials that fulfill glTF requirements.
 *
 * <p>This class is used by {@link AssetLoader} to create Filament materials.
 * Client applications do not need to call methods on it.</p>
 */
public class MaterialProvider {
    private long mNativeObject;

    /**
     * Constructs an ubershader loader using the supplied {@link Engine}.
     *
     * @param engine the engine used to create materials
     */
    public MaterialProvider(Engine engine) {
        long nativeEngine = engine.getNativeObject();
        mNativeObject = nCreateMaterialProvider(nativeEngine);
    }

    /**
     * Frees memory associated with the native material provider.
     * */
    public void destroy() {
        nDestroyMaterialProvider(mNativeObject);
        mNativeObject = 0;
    }

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    public void destroyMaterials() {
        nDestroyMaterials(mNativeObject);
    }

    long getNativeObject() {
        return mNativeObject;
    }

    private static native long nCreateMaterialProvider(long nativeEngine);
    private static native void nDestroyMaterialProvider(long nativeProvider);
    private static native void nDestroyMaterials(long nativeProvider);
}
