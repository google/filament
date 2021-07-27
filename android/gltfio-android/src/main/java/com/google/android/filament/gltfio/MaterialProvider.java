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

import com.google.android.filament.MaterialInstance;
import com.google.android.filament.Material;
import com.google.android.filament.VertexBuffer;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

public interface MaterialProvider {

    /**
     * MaterialKey specifies the requirements for a requested glTF material.
     * The provider creates Filament materials that fulfill these requirements.
     */
    public static class MaterialKey {
        public boolean doubleSided;
        public boolean unlit;
        public boolean hasVertexColors;
        public boolean hasBaseColorTexture;
        public boolean hasNormalTexture;
        public boolean hasOcclusionTexture;
        public boolean hasEmissiveTexture;
        public boolean useSpecularGlossiness;
        public int alphaMode;                       // 0 = OPAQUE, 1 = MASK, 2 = BLEND
        public boolean enableDiagnostics;
        public boolean hasMetallicRoughnessTexture; // piggybacks with specularRoughness
        public int metallicRoughnessUV;             // piggybacks with specularRoughness
        public int baseColorUV;
        public boolean hasClearCoatTexture;
        public int clearCoatUV;
        public boolean hasClearCoatRoughnessTexture;
        public int clearCoatRoughnessUV;
        public boolean hasClearCoatNormalTexture;
        public int clearCoatNormalUV;
        public boolean hasClearCoat;
        public boolean hasTransmission;
        public boolean hasTextureTransforms;
        public int emissiveUV;
        public int aoUV;
        public int normalUV;
        public boolean hasTransmissionTexture;
        public int transmissionUV;
        public boolean hasSheenColorTexture;
        public int sheenColorUV;
        public boolean hasSheenRoughnessTexture;
        public int sheenRoughnessUV;
        public boolean hasVolumeThicknessTexture;
        public int volumeThicknessUV;
        public boolean hasSheen;
        public boolean hasIOR;

        public MaterialKey() {}
        static {
            nGlobalInit();
        }
        private static native void nGlobalInit();
    };

    /**
     * Creates or fetches a compiled Filament material, then creates an instance from it.
     *
     * @param config Specifies requirements; might be mutated due to resource constraints.
     * @param uvmap Output argument that gets populated with a small table that maps from a glTF uv
     *              index to a Filament uv index (0 = UNUSED, 1 = UV0, 2 = UV1).
     * @param label Optional tag that is not a part of the cache key.
     */
    public @Nullable MaterialInstance createMaterialInstance(MaterialKey config,
            @NonNull @Size(min = 8) int[] uvmap, @Nullable String label);

    /**
     * Creates and returns an array containing all cached materials.
     */
    public @NonNull Material[] getMaterials();

    /**
     * Returns true if the presence of the given vertex attribute is required.
     *
     * Some types of providers (e.g. ubershader) require dummy attribute values
     * if the glTF model does not provide them.
     *
     * NOTE: The given attribute is the VertexAttribute enum casted to an integer.
     * This is done to streamline the JNI work between Java and Native layers.
     */
    public boolean needsDummyData(int attrib);

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    public void destroyMaterials();
}
