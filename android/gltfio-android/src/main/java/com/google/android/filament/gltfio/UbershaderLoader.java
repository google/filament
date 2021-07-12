/*
 * Copyright (C) 2021 The Android Open Source Project
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
import com.google.android.filament.MaterialInstance;
import com.google.android.filament.Material;
import com.google.android.filament.VertexBuffer;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

/**
 * Loads pre-generated ubershader materials that fulfill glTF requirements.
 *
 * <p>This class is used by {@link AssetLoader} to create Filament materials.
 * Client applications do not need to call methods on it.</p>
 */
public class UbershaderLoader implements MaterialProvider {
    private long mNativeObject;

    /**
     * Constructs an ubershader loader using the supplied {@link Engine}.
     *
     * @param engine the engine used to create materials
     */
    public UbershaderLoader(Engine engine) {
        long nativeEngine = engine.getNativeObject();
        mNativeObject = nCreateUbershaderLoader(nativeEngine);
    }

    /**
     * Frees memory associated with the native material provider.
     * */
    public void destroy() {
        nDestroyUbershaderLoader(mNativeObject);
        mNativeObject = 0;
    }

    public @Nullable MaterialInstance createMaterialInstance(MaterialKey config,
            @NonNull @Size(min = 8) int[] uvmap, @Nullable String label) {
        long nativeMaterialInstance = nCreateMaterialInstance(mNativeObject, config, uvmap, label);
        return nativeMaterialInstance == 0 ? null : new MaterialInstance(null, nativeMaterialInstance);
    }

    public @NonNull Material[] getMaterials() {
        final int count = nGetMaterialCount(mNativeObject);
        Material[] result = new Material[count];
        long[] natives = new long[count];
        nGetMaterials(mNativeObject, natives);
        for (int i = 0; i < count; i++) {
            result[i] = new Material(natives[i]);
        }
        return result;
    }

    public boolean needsDummyData(int attrib) {
        VertexBuffer.VertexAttribute vattrib = VertexBuffer.VertexAttribute.values()[attrib];
        switch (vattrib) {
            case UV0:
            case UV1:
            case COLOR:
                return true;
            default:
                return false;
        }
    }

    public void destroyMaterials() {
        nDestroyMaterials(mNativeObject);
    }

    public long getNativeObject() {
        return mNativeObject;
    }

    private static native long nCreateUbershaderLoader(long nativeEngine);
    private static native void nDestroyUbershaderLoader(long nativeProvider);
    private static native void nDestroyMaterials(long nativeProvider);
    private static native long nCreateMaterialInstance(long nativeProvider, MaterialKey config, int[] uvmap, String label);
    private static native int nGetMaterialCount(long nativeProvider);
    private static native void nGetMaterials(long nativeProvider, long[] result);
}
